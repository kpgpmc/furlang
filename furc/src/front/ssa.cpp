#include "furc/front/ssa.hpp"

#include "furlang/ir/instruction.hpp"
#include "furlang/ir/operand.hpp"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace furc::front {

void ssa::optimize(furlang::ir::module& mod) {
    for (const auto& func : mod.functions()) {
        ssa::optimize(func);
    }
}

void ssa::optimize(const std::unique_ptr<furlang::ir::function>& func) {
    block_map_t predecessors;
    block_map_t successors;

    std::unordered_map<furlang::ir::register_operand, std::unordered_set<furlang::ir::block_index>> regSites;
    std::unordered_map<furlang::ir::block_index, std::unordered_set<furlang::ir::register_operand>> regUses;

    std::unordered_map<furlang::ir::block_index, furlang::ir::block_index> idoms;

    for (furlang::ir::block_index i = 0; i < func->blocks().size(); ++i) {
        const auto& block = func->blocks()[i];

        for (const auto& instr : block->instructions()) {
            switch (instr->type()) {
            case furlang::ir::instruction_t::Assign: {
                const auto& assign = dynamic_cast<const furlang::ir::assign_instruction&>(*instr);
                regSites[assign.destination().reg()].insert(i);
                if (assign.source().type() == furlang::ir::operand_t::Register) {
                    regUses[i].insert(assign.source().reg());
                }
            } break;
            case furlang::ir::instruction_t::BinaryOp: {
                const auto& binOp = dynamic_cast<const furlang::ir::binary_op_instruction&>(*instr);
                regSites[binOp.dst().reg()].insert(i);
                if (binOp.lhs().type() == furlang::ir::operand_t::Register) {
                    regUses[i].insert(binOp.lhs().reg());
                }
                if (binOp.rhs().type() == furlang::ir::operand_t::Register) {
                    regUses[i].insert(binOp.rhs().reg());
                }
            } break;
            default: break;
            }
        }

        const auto& exit = block->exit();
        switch (exit->type()) {
        case furlang::ir::instruction_t::Branch: {
            const auto& br = dynamic_cast<const furlang::ir::branch_instruction&>(*exit);
            predecessors[br.block()].push_back(i);
            successors[i].push_back(br.block());
        } break;
        case furlang::ir::instruction_t::BranchCond: {
            const auto& br = dynamic_cast<const furlang::ir::branch_cond_instruction&>(*exit);
            predecessors[br.if_block()].push_back(i);
            predecessors[br.else_block()].push_back(i);
            successors[i].push_back(br.if_block());
            successors[i].push_back(br.else_block());
        } break;
        default: break;
        }
    }

    std::unordered_set<furlang::ir::block_index> visited;
    std::vector<furlang::ir::block_index>        rpoOrder;
    dfs_rpo(0, successors, visited, rpoOrder);
    std::reverse(rpoOrder.begin(), rpoOrder.end());

    std::unordered_map<furlang::ir::block_index, std::size_t> rpoIndex;
    for (std::size_t i = 0; i < rpoOrder.size(); ++i) {
        rpoIndex[rpoOrder[i]] = i;
    }

    auto intersect = [&](furlang::ir::block_index block1, furlang::ir::block_index block2) {
        while (block1 != block2) {
            while (rpoIndex[block1] > rpoIndex[block2]) {
                block1 = idoms[block1];
            }
            while (rpoIndex[block2] > rpoIndex[block1]) {
                block2 = idoms[block2];
            }
        }
        return block1;
    };

    auto entry   = rpoOrder.front();
    idoms[entry] = entry;

    bool changed = true;
    while (changed) {
        changed = false;

        for (std::size_t i = 1; i < rpoOrder.size(); ++i) {
            auto block = rpoOrder[i];

            furlang::ir::block_index newIdom = 0;
            bool                     found   = false;
            for (auto pred : predecessors[block]) {
                if (idoms.find(pred) == idoms.end()) continue;
                if (!found) {
                    newIdom = pred;
                    found   = true;
                } else {
                    newIdom = intersect(pred, newIdom);
                }
            }

            if (idoms.find(block) == idoms.end() || idoms[block] != newIdom) {
                idoms[block] = newIdom;
                changed      = true;
            }
        }
    }

    std::unordered_map<furlang::ir::block_index, std::unordered_set<furlang::ir::block_index>> df;
    for (auto block : rpoOrder) {
        df[block] = std::unordered_set<furlang::ir::block_index>{};
    }

    for (auto block : rpoOrder) {
        if (predecessors[block].size() < 2) continue;

        for (auto pred : predecessors[block]) {
            auto runner = pred;
            while (runner != idoms[block]) {
                df[runner].insert(block);
                runner = idoms[runner];
            }
        }
    }

    std::unordered_map<furlang::ir::block_index, std::unordered_set<furlang::ir::register_operand>> phis;

    for (const auto& [reg, blocks] : regSites) {
        std::vector<furlang::ir::block_index> worklist(blocks.begin(), blocks.end());

        std::unordered_set<furlang::ir::block_index> added;
        while (!worklist.empty()) {
            auto block = worklist.back();
            worklist.pop_back();
            for (auto frontier : df[block]) {
                if (added.find(frontier) != added.end()) continue;
                phis[frontier].insert(reg);
                added.insert(frontier);
                if (blocks.find(frontier) == blocks.end()) {
                    worklist.push_back(frontier);
                }
            }
        }
    }

    for (furlang::ir::block_index i = 0; i < func->blocks().size(); ++i) {
        if (phis.find(i) == phis.end()) continue;
        const auto& block = func->blocks()[i];
        const auto& preds = predecessors[i];

        for (auto reg : phis[i]) {
            if (regUses[i].find(reg) == regUses[i].end()) continue;

            auto phiInstr = std::make_unique<furlang::ir::phi_instruction>(reg);
            for (auto pred : preds) {
                phiInstr->labels().emplace_back(furlang::ir::operand::new_reg(reg), pred);
            }
            block->instructions().emplace(block->instructions().begin(), std::move(phiInstr));
        }
    }
}

void ssa::dfs_rpo(furlang::ir::block_index        block,
    const block_map_t&                            successors,
    std::unordered_set<furlang::ir::block_index>& visited,
    std::vector<furlang::ir::block_index>&        rpo) {
    visited.insert(block);
    if (auto it = successors.find(block); it != successors.end()) {
        for (auto successor : it->second) {
            if (visited.find(successor) == visited.end()) {
                dfs_rpo(successor, successors, visited, rpo);
            }
        }
    }
    rpo.push_back(block);
}

} // namespace furc::front
