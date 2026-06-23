#include "furc/front/post_process.hpp"

#include "furlang/ir/function.hpp"
#include "furlang/ir/instruction.hpp"
#include "furlang/ir/operand.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>
#include <queue>
#include <set>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace furc::front {

using block_idx   = furlang::ir::block_index;
using register_t  = furlang::ir::register_t;
using register_op = furlang::ir::register_operand;

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

struct sccp_lattice {
    enum lattice_t { // NOLINT
        Top,
        Constant,
        Bottom,
    } type                 = Top;
    std::uint64_t constant = 0;

    bool operator==(const sccp_lattice& other) const {
        return type == other.type && (type != Constant || constant == other.constant);
    }

    bool operator!=(const sccp_lattice& other) const { return !this->operator==(other); }
};

sccp_lattice sccp_stage_get_lattice(std::unordered_map<register_op, sccp_lattice>& latticeValues,
    const furlang::ir::operand&                                                    op) {
    if (op.type() == furlang::ir::operand_t::Integer) {
        sccp_lattice lat;
        lat.type     = sccp_lattice::Constant;
        lat.constant = op.integer();
        return lat;
    }
    if (op.type() == furlang::ir::operand_t::Register) {
        auto reg = op.reg();
        if (auto it = latticeValues.find(reg); it != latticeValues.end()) return it->second;
        return { sccp_lattice::Top };
    }
    return { sccp_lattice::Bottom };
};

static void sccp_stage(function_context& ctx) {
    using lattice = sccp_lattice;

    std::unordered_map<register_op, lattice>                               latticeValues;
    std::unordered_map<register_t, std::vector<furlang::ir::instruction*>> edges;
    std::unordered_set<block_idx>                                          execBlocks;
    std::set<std::pair<block_idx, block_idx>>                              execEdges;

    std::queue<std::pair<block_idx, block_idx>> cfgWorklist;
    std::queue<furlang::ir::instruction*>       ssaWorklist;

    std::unordered_map<furlang::ir::instruction*, block_idx> blockMap;

    for (block_idx idx = 0; idx < ctx.function->blocks().size(); ++idx) {
        const auto& block  = ctx.function->blocks()[idx];
        auto&       instrs = block->instructions();
        for (auto it = instrs.begin(); it != instrs.end(); ++it) {
            const auto& instr     = *it;
            blockMap[instr.get()] = idx;
            for (const auto& op : instr->sources()) {
                if (op->type() != furlang::ir::operand_t::Register) continue;
                edges[op->reg()].push_back(instr.get());
            }
        }

        blockMap[block->exit().get()] = idx;
        for (const auto& op : block->exit()->sources()) {
            if (op->type() != furlang::ir::operand_t::Register) continue;
            edges[op->reg()].push_back(block->exit().get());
        }
    }

    cfgWorklist.push({ 0, 0 });
    while (!cfgWorklist.empty() || !ssaWorklist.empty()) {
        if (!cfgWorklist.empty()) {
            auto edge = cfgWorklist.front();
            cfgWorklist.pop();
            block_idx from = edge.first;
            block_idx to   = edge.second;

            if (execEdges.count(edge) != 0) continue;
            execEdges.insert(edge);

            bool firstVisit = (execBlocks.find(to) == execBlocks.end());
            execBlocks.insert(to);

            const auto& block = ctx.function->blocks()[to];
            if (firstVisit) {
                for (auto& instr : block->instructions()) {
                    ssaWorklist.push(instr.get());
                }
                ssaWorklist.push(block->exit().get());
            } else {
                for (auto& instr : block->instructions()) {
                    if (instr->type() != furlang::ir::instruction_t::Phi) break;
                    ssaWorklist.push(instr.get());
                }
            }
        }

        if (!ssaWorklist.empty()) {
            auto* instr = ssaWorklist.front();
            ssaWorklist.pop();

            block_idx blockIdx = blockMap[instr];
            if (execBlocks.find(blockIdx) == execBlocks.end()) continue;

            lattice newLat = { lattice::Top };

            switch (instr->type()) {
            case furlang::ir::instruction_t::Phi: {
                auto& phi = dynamic_cast<furlang::ir::phi_instruction&>(*instr);
                for (const auto& [op, label] : phi.labels()) {
                    if (execEdges.count({ label, blockIdx }) == 0) continue;
                    lattice opLat = sccp_stage_get_lattice(latticeValues, op);
                    if (opLat.type == lattice::Bottom) newLat.type = lattice::Bottom;
                    if (opLat.type == lattice::Constant) {
                        if (newLat.type == lattice::Top) {
                            newLat = opLat;
                        } else if (newLat.type == lattice::Constant && newLat.constant != opLat.constant) {
                            newLat.type = lattice::Bottom;
                        }
                    }
                }
            } break;
            case furlang::ir::instruction_t::Assign: {
                newLat = sccp_stage_get_lattice(latticeValues, *instr->sources().front());
            } break;
            case furlang::ir::instruction_t::BinaryOp: {
                lattice lhs = sccp_stage_get_lattice(latticeValues, *instr->sources()[0]);
                lattice rhs = sccp_stage_get_lattice(latticeValues, *instr->sources()[1]);

                if (lhs.type == lattice::Bottom || rhs.type == lattice::Bottom) {
                    newLat.type = lattice::Bottom;
                } else if (lhs.type == lattice::Constant && rhs.type == lattice::Constant) {
                    newLat.type = lattice::Constant;
                    switch (dynamic_cast<const furlang::ir::binary_op_instruction&>(*instr).op_type()) {
                    case furlang::ir::binary_op_instruction_t::Add:
                        newLat.constant = lhs.constant + rhs.constant;
                        break;
                    case furlang::ir::binary_op_instruction_t::Sub:
                        newLat.constant = lhs.constant - rhs.constant;
                        break;
                    case furlang::ir::binary_op_instruction_t::Mul:
                        newLat.constant = lhs.constant * rhs.constant;
                        break;
                    case furlang::ir::binary_op_instruction_t::Div:
                        newLat.constant = lhs.constant / rhs.constant;
                        break;
                    case furlang::ir::binary_op_instruction_t::Mod:
                        newLat.constant = lhs.constant % rhs.constant;
                        break;
                    case furlang::ir::binary_op_instruction_t::Eq:
                        newLat.constant = (lhs.constant == rhs.constant) ? 1 : 0;
                        break;
                    case furlang::ir::binary_op_instruction_t::NotEq:
                        newLat.constant = (lhs.constant != rhs.constant) ? 1 : 0;
                        break;
                    case furlang::ir::binary_op_instruction_t::LessThan:
                        newLat.constant = (lhs.constant < rhs.constant) ? 1 : 0;
                        break;
                    case furlang::ir::binary_op_instruction_t::GreaterThan:
                        newLat.constant = (lhs.constant > rhs.constant) ? 1 : 0;
                        break;
                    case furlang::ir::binary_op_instruction_t::LessEq:
                        newLat.constant = (lhs.constant <= rhs.constant) ? 1 : 0;
                        break;
                    case furlang::ir::binary_op_instruction_t::GreaterEq:
                        newLat.constant = (lhs.constant >= rhs.constant) ? 1 : 0;
                        break;
                    }
                }
            } break;
            default: break;
            }

            if (instr->has_destination() && instr->destination().type() == furlang::ir::operand_t::Register) {
                auto dst = instr->destination().reg();
                if (!(latticeValues[dst] == newLat)) {
                    latticeValues[dst] = newLat;
                    for (auto* uInstr : edges[dst])
                        ssaWorklist.push(uInstr);
                }
            }

            if (instr == ctx.function->blocks()[blockIdx]->exit().get()) {
                auto* exit = ctx.function->blocks()[blockIdx]->exit().get();
                if (exit->type() == furlang::ir::instruction_t::Branch) {
                    auto& br = dynamic_cast<furlang::ir::branch_instruction&>(*exit);
                    cfgWorklist.push({ blockIdx, br.block() });
                } else if (exit->type() == furlang::ir::instruction_t::BranchCond) {
                    auto&   br   = dynamic_cast<furlang::ir::branch_cond_instruction&>(*exit);
                    lattice cond = sccp_stage_get_lattice(latticeValues, *exit->sources()[0]);

                    if (cond.type == lattice::Constant) {
                        if (cond.constant != 0)
                            cfgWorklist.push({ blockIdx, br.if_block() });
                        else
                            cfgWorklist.push({ blockIdx, br.else_block() });
                    } else {
                        cfgWorklist.push({ blockIdx, br.if_block() });
                        cfgWorklist.push({ blockIdx, br.else_block() });
                    }
                }
            }
        }
    }

    for (block_idx i = 0; i < ctx.function->blocks().size(); ++i) {
        if (execBlocks.find(i) == execBlocks.end()) {
            ctx.function->blocks()[i]->instructions().clear();
            continue;
        }

        const auto& block = ctx.function->blocks()[i];

        for (auto& instr : block->instructions()) {
            for (auto& op : instr->sources()) {
                if (op->type() != furlang::ir::operand_t::Register) continue;
                auto reg = op->reg();
                if (latticeValues[reg].type != lattice::Constant) continue;
                *op = furlang::ir::operand::new_integer(latticeValues[reg].constant);
            }
        }

        auto* exit = block->exit().get();
        if (exit->type() != furlang::ir::instruction_t::BranchCond) continue;
        auto&   br   = dynamic_cast<furlang::ir::branch_cond_instruction&>(*exit);
        lattice cond = sccp_stage_get_lattice(latticeValues, *exit->sources()[0]);
        if (cond.type != lattice::Constant) continue;
        block_idx target = (cond.constant != 0) ? br.if_block() : br.else_block();
        block->exit()    = std::make_unique<furlang::ir::branch_instruction>(target);
    }
}

void post_process::process(furlang::ir::mod& mod) {
    for (const auto& func : mod.functions()) {
        if (!func || func->blocks().empty()) continue;
        function_context ctx{ func.get() };

        for (const auto& stage : m_stages) {
            switch (stage) {
            case Ssa: ssa_stage(ctx); break;
            case Sccp: sccp_stage(ctx); break;
            case DeSsa: dessa_stage(ctx); break;
            }
        }
    }
}

} // namespace furc::front
