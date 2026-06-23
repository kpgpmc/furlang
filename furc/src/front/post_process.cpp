#include "furc/front/post_process.hpp"

#include "furlang/ir/function.hpp"
#include "furlang/ir/instruction.hpp"
#include "furlang/ir/operand.hpp"

#include <algorithm>
#include <limits>
#include <memory>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace furc::front {

using block_idx   = furlang::ir::block_index;
using reigster_t  = furlang::ir::register_t;
using reigster_op = furlang::ir::register_operand;

static constexpr block_idx INVALID_BLOCK = std::numeric_limits<block_idx>::max();

struct block_info {
    std::size_t rpoIndex{ 0 };

    std::vector<block_idx> predecessors;
    std::vector<block_idx> successors;

    block_idx                     idom = INVALID_BLOCK;
    std::vector<block_idx>        doms;
    std::unordered_set<block_idx> domFrontiers;
};

struct register_info {
    std::unordered_set<block_idx> defSites;
    std::stack<register_t>        renameStack;
    std::uint32_t                 nextVersion{ 0 };
};

struct function_context {
    explicit function_context(furlang::ir::function* function)
      : function(function) {
        build_cfg();
        compute_rpo();
    }

    void build_cfg() {
        for (block_idx idx = 0; idx < function->blocks().size(); ++idx) {
            const auto& block = function->blocks()[idx];

            for (const auto& instr : block->instructions()) {
                for (const auto& operand : instr->sources()) {
                    if (operand->type() != furlang::ir::operand_t::Register) continue;
                    auto reg = operand->reg();
                    if (registers[reg].defSites.find(idx) != registers[reg].defSites.end()) continue;
                    globalRegisters.insert(reg);
                }
            }

            for (const auto& instr : block->instructions()) {
                if (!instr->has_destination() || instr->destination().type() != furlang::ir::operand_t::Register)
                    continue;
                auto reg = instr->destination().reg();
                registers[reg].defSites.insert(idx);
            }

            for (const auto& operand : block->exit()->sources()) {
                if (operand->type() != furlang::ir::operand_t::Register) continue;
                auto reg = operand->reg();
                if (registers[reg].defSites.find(idx) != registers[reg].defSites.end()) continue;
                globalRegisters.insert(reg);
            }

            const auto& exit = block->exit();
            switch (exit->type()) {
            case furlang::ir::instruction_t::Branch: {
                const auto& br = dynamic_cast<const furlang::ir::branch_instruction&>(*exit);
                blocks[br.block()].predecessors.push_back(idx);
                blocks[idx].successors.push_back(br.block());
            } break;
            case furlang::ir::instruction_t::BranchCond: {
                const auto& br = dynamic_cast<const furlang::ir::branch_cond_instruction&>(*exit);
                blocks[br.if_block()].predecessors.push_back(idx);
                blocks[br.else_block()].predecessors.push_back(idx);
                blocks[idx].successors.push_back(br.if_block());
                blocks[idx].successors.push_back(br.else_block());
            } break;
            default: break;
            }
        }
    }

    void compute_rpo() {
        std::unordered_set<block_idx> visited;

        auto dfs = [&](auto& self, block_idx block) -> void {
            visited.insert(block);
            for (auto succ : blocks[block].successors) {
                if (visited.find(succ) != visited.end()) continue;
                self(self, succ);
            }
            rpoOrder.push_back(block);
        };

        if (!function->blocks().empty()) dfs(dfs, 0);

        std::reverse(rpoOrder.begin(), rpoOrder.end());
        for (std::size_t i = 0; i < rpoOrder.size(); ++i) {
            blocks[rpoOrder[i]].rpoIndex = i;
        }
    }

    void compute_dominance() {
        if (rpoOrder.empty()) return;

        const block_idx entry = rpoOrder.front();
        blocks[entry].idom    = entry;

        bool changed = true;
        while (changed) {
            changed = false;

            for (block_idx idx : rpoOrder) {
                if (idx == entry) continue;

                block_idx newIdom = INVALID_BLOCK;
                bool      found   = false;
                for (auto pred : blocks[idx].predecessors) {
                    if (blocks[pred].idom == INVALID_BLOCK) continue;
                    if (found) {
                        newIdom = intersect(pred, newIdom);
                    } else {
                        newIdom = pred;
                        found   = true;
                    }
                }

                if (blocks[idx].idom != newIdom) {
                    blocks[idx].idom = newIdom;
                    changed          = true;
                }
            }
        }

        for (auto idx : rpoOrder) {
            if (idx == entry) continue;
            const block_idx parent = blocks[idx].idom;
            if (parent != INVALID_BLOCK) blocks[parent].doms.push_back(idx);
        }

        for (auto idx : rpoOrder) {
            if (blocks[idx].predecessors.size() < 2) continue;
            for (auto cur : blocks[idx].predecessors) {
                while (cur != blocks[idx].idom) {
                    blocks[cur].domFrontiers.insert(idx);
                    cur = blocks[cur].idom;
                }
            }
        }
    }

    furlang::ir::function*                        function;
    std::vector<block_idx>                        rpoOrder;
    std::unordered_map<block_idx, block_info>     blocks;
    std::unordered_map<register_t, register_info> registers;
    std::unordered_set<register_t>                globalRegisters;
private:
    block_idx intersect(block_idx block1, block_idx block2) {
        while (block1 != block2) {
            while (blocks[block1].rpoIndex > blocks[block2].rpoIndex)
                block1 = blocks[block1].idom;
            while (blocks[block2].rpoIndex > blocks[block1].rpoIndex)
                block2 = blocks[block2].idom;
        }
        return block1;
    }
};

static void ssa_stage_rename_block(function_context&           ctx,
    block_idx                                                  idx,
    std::unordered_map<register_t, std::uint32_t>&             regVers,
    std::unordered_map<register_t, std::stack<std::uint32_t>>& regVerStacks) {
    std::unordered_map<register_t, std::uint32_t> pushed;

    const auto& block = ctx.function->blocks()[idx];
    auto        it    = block->instructions().begin();
    for (; it != block->instructions().end(); ++it) {
        auto& instr = *it;
        if (instr->type() != furlang::ir::instruction_t::Phi) break;

        const register_t    orig   = instr->destination().reg();
        const std::uint32_t newVer = regVers[orig]++;

        instr->destination().reg().ver = newVer;
        regVerStacks[orig].push(newVer);
        ++pushed[orig];
    }

    for (; it != block->instructions().end(); ++it) {
        auto& instr = *it;
        for (auto& operand : instr->sources()) {
            if (operand->type() != furlang::ir::operand_t::Register) continue;

            const register_t orig = operand->reg();
            if (regVerStacks[orig].empty()) continue;
            operand->reg().ver = regVerStacks[orig].top();
        }

        if (instr->has_destination() && instr->destination().type() == furlang::ir::operand_t::Register) {
            const register_t    orig   = instr->destination().reg();
            const std::uint32_t newVer = regVers[orig]++;

            instr->destination().reg().ver = newVer;
            regVerStacks[orig].push(newVer);
            ++pushed[orig];
        }
    }

    for (auto& operand : block->exit()->sources()) {
        if (operand->type() != furlang::ir::operand_t::Register) continue;

        const auto orig = operand->reg();
        if (regVerStacks[orig].empty()) continue;
        operand->reg().ver = regVerStacks[orig].top();
    }

    for (auto succIdx : ctx.blocks[idx].successors) {
        const auto& succ = ctx.function->blocks()[succIdx];
        for (auto& instr : succ->instructions()) {
            if (instr->type() != furlang::ir::instruction_t::Phi) break;

            auto& phi = dynamic_cast<furlang::ir::phi_instruction&>(*instr);
            for (auto& pair : phi.labels()) {
                if (pair.second != idx) continue;

                auto orig = pair.first.reg();
                if (auto it = regVerStacks.find(orig); it != regVerStacks.end())
                    pair.first.reg().ver = it->second.top();
            }
        }
    }

    for (const auto& child : ctx.blocks[idx].doms) {
        ssa_stage_rename_block(ctx, child, regVers, regVerStacks);
    }

    for (const auto& [reg, count] : pushed) {
        for (std::size_t i = 0; i < count; ++i)
            regVerStacks[reg].pop();
    }
}

static void ssa_stage(function_context& ctx) {
    ctx.compute_dominance();

    std::vector<block_idx> worklist;
    for (const auto& [reg, info] : ctx.registers) {
        if (info.defSites.size() < 2 || ctx.globalRegisters.find(reg) == ctx.globalRegisters.end()) continue;

        worklist.clear();
        worklist.insert(worklist.end(), info.defSites.begin(), info.defSites.end());

        std::unordered_set<block_idx> added;
        while (!worklist.empty()) {
            const auto idx = worklist.back();
            worklist.pop_back();
            for (auto frontier : ctx.blocks[idx].domFrontiers) {
                if (added.find(frontier) != added.end()) continue;
                added.insert(frontier);

                const auto& target = ctx.function->blocks()[frontier];
                const auto& preds  = ctx.blocks[frontier].predecessors;

                auto instr = std::make_unique<furlang::ir::phi_instruction>(reg);
                for (auto pred : preds) {
                    instr->labels().emplace_back(furlang::ir::operand::new_reg(reg), pred);
                }
                target->instructions().emplace(target->instructions().begin(), std::move(instr));

                if (info.defSites.find(frontier) == info.defSites.end()) worklist.push_back(frontier);
            }
        }
    }

    std::unordered_map<register_t, std::uint32_t>             regVers;
    std::unordered_map<register_t, std::stack<std::uint32_t>> regVerStacks;
    ssa_stage_rename_block(ctx, ctx.rpoOrder.front(), regVers, regVerStacks);
}

static void dessa_stage(function_context& ctx) {
    for (block_idx idx = 0; idx < ctx.function->blocks().size(); ++idx) {
        const auto& block  = ctx.function->blocks()[idx];
        auto&       instrs = block->instructions();
        for (auto it = instrs.begin(); it != instrs.end() && (*it)->type() == furlang::ir::instruction_t::Phi;
            it       = instrs.erase(it)) {
            auto& phi    = dynamic_cast<furlang::ir::phi_instruction&>(**it);
            auto  dstReg = phi.destination().reg();
            for (auto& [srcOp, label] : phi.labels()) {
                ctx.function->blocks()[label]->instructions().push_back(
                    std::make_unique<furlang::ir::assign_instruction>(furlang::ir::operand::new_reg(srcOp.reg()),
                        furlang::ir::operand::new_reg(dstReg)));
            }
        }
    }
}

void post_process::process(furlang::ir::mod& mod) {
    for (const auto& func : mod.functions()) {
        if (!func || func->blocks().empty()) continue;
        function_context ctx{ func.get() };

        for (const auto& stage : m_stages) {
            switch (stage) {
            case Ssa: ssa_stage(ctx); break;
            case DeSsa: dessa_stage(ctx); break;
            }
        }
    }
}

} // namespace furc::front
