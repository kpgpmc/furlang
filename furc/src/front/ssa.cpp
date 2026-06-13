#include "furc/front/ssa.hpp"

#include "furlang/ir/instruction.hpp"
#include "furlang/ir/operand.hpp"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <memory>
#include <queue>
#include <set>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace furc::front {

void ssa::optimize(furlang::ir::module& mod) {
    for (const auto& func : mod.functions()) {
        ssa::optimize(func);
        ssa::constant_propagation(func);
        ssa::dead_code_elimination(func);
    }
}

void ssa::optimize(const std::unique_ptr<furlang::ir::function>& func) {
    block_map_t predecessors;
    block_map_t successors;

    std::unordered_map<furlang::ir::register_t, std::unordered_set<furlang::ir::block_index>> regSites;

    std::unordered_map<furlang::ir::block_index, furlang::ir::block_index> idoms;

    std::unordered_map<furlang::ir::register_t, std::uint32_t>                          regVers;
    std::unordered_map<furlang::ir::register_t, std::stack<std::uint32_t>>              regVerStacks;
    std::unordered_map<furlang::ir::block_index, std::vector<furlang::ir::block_index>> domTree;

    for (furlang::ir::block_index i = 0; i < func->blocks().size(); ++i) {
        const auto& block = func->blocks()[i];

        for (const auto& instr : block->instructions()) {
            if (instr->has_destination()) {
                if (instr->destination().type() == furlang::ir::operand_t::Register) {
                    regSites[instr->destination().reg()].insert(i);
                }
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

    std::unordered_set<furlang::ir::register_t> globalRegs;
    for (furlang::ir::block_index i = 0; i < func->blocks().size(); ++i) {
        const auto& block = func->blocks()[i];
        for (const auto& instr : block->instructions()) {
            for (const auto& operand : instr->sources()) {
                if (operand->type() == furlang::ir::operand_t::Register) {
                    auto reg = operand->reg();
                    if (regSites[reg].find(i) == regSites[reg].end()) {
                        globalRegs.insert(reg);
                    }
                }
            }
        }
        for (const auto& operand : block->exit()->sources()) {
            if (operand->type() == furlang::ir::operand_t::Register) {
                auto reg = operand->reg();
                if (regSites[reg].find(i) == regSites[reg].end()) {
                    globalRegs.insert(reg);
                }
            }
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

    if (rpoOrder.empty()) return;
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

    std::unordered_map<furlang::ir::block_index, std::unordered_set<furlang::ir::register_t>> phis;

    for (const auto& [reg, blocks] : regSites) {
        if (globalRegs.find(reg) == globalRegs.end()) continue;

        std::vector<furlang::ir::block_index>        worklist(blocks.begin(), blocks.end());
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
            auto phiInstr = std::make_unique<furlang::ir::phi_instruction>(reg);
            for (auto pred : preds) {
                phiInstr->labels().emplace_back(furlang::ir::operand::new_reg(reg), pred);
            }
            block->instructions().emplace(block->instructions().begin(), std::move(phiInstr));
        }
    }

    for (const auto& [block, idom] : idoms) {
        if (block != idom) domTree[idom].push_back(block);
    }

    std::function<void(furlang::ir::block_index)> renameBlock = [&](furlang::ir::block_index blockIndex) -> void {
        std::unordered_map<furlang::ir::register_t, std::uint32_t> pushed;

        const auto& block = func->blocks()[blockIndex];
        for (auto& instr : block->instructions()) {
            if (instr->type() != furlang::ir::instruction_t::Phi) continue;
            auto          orig             = instr->destination().reg();
            std::uint32_t newVer           = regVers[orig]++;
            instr->destination().reg().ver = newVer;
            regVerStacks[orig].push(newVer);
            ++pushed[orig];
        }

        for (auto& instr : block->instructions()) {
            if (instr->type() == furlang::ir::instruction_t::Phi) continue;

            for (auto& operand : instr->sources()) {
                if (operand->type() != furlang::ir::operand_t::Register) continue;
                auto orig = operand->reg();
                if (!regVerStacks[orig].empty()) {
                    operand->reg().ver = regVerStacks[orig].top();
                }
            }

            if (instr->has_destination() && instr->destination().type() == furlang::ir::operand_t::Register) {
                auto          orig             = instr->destination().reg();
                std::uint32_t newVer           = regVers[orig]++;
                instr->destination().reg().ver = newVer;
                regVerStacks[orig].push(newVer);
                ++pushed[orig];
            }
        }

        for (auto& operand : block->exit()->sources()) {
            if (operand->type() != furlang::ir::operand_t::Register) continue;
            auto orig = operand->reg();
            if (!regVerStacks[orig].empty()) {
                operand->reg().ver = regVerStacks[orig].top();
            }
        }

        for (auto succIndex : successors[blockIndex]) {
            const auto& succ = func->blocks()[succIndex];
            for (auto& instr : succ->instructions()) {
                if (instr->type() != furlang::ir::instruction_t::Phi) break;
                auto& phi = dynamic_cast<furlang::ir::phi_instruction&>(*instr);
                for (auto& [op, bl] : phi.labels()) {
                    if (bl != blockIndex) continue;
                    if (regVerStacks[op.reg()].empty()) continue;
                    op.reg().ver = regVerStacks[op.reg()].top();
                }
            }
        }

        for (const auto& child : domTree[blockIndex]) {
            renameBlock(child);
        }

        for (const auto& [reg, count] : pushed) {
            for (std::uint32_t i = 0; i < count; ++i) {
                regVerStacks[reg].pop();
            }
        }
    };

    renameBlock(entry);
}

enum class lattice_t {
    Top,
    Constant,
    Bottom,
};

struct lattice {
    lattice_t     type  = lattice_t::Top;
    std::uint64_t value = 0;

    bool operator==(const lattice& rhs) const {
        if (type != rhs.type) return false;
        return type != lattice_t::Constant || value == rhs.value;
    }
};

void ssa::constant_propagation(const std::unique_ptr<furlang::ir::function>& func) {
    using block_idx = furlang::ir::block_index;
    using reg_t     = furlang::ir::register_operand;
    using reg2_t    = furlang::ir::register_t;

    std::unordered_map<reg_t, lattice>                                 latVals;
    std::unordered_map<reg2_t, std::vector<furlang::ir::instruction*>> edges;
    std::unordered_set<block_idx>                                      executableBlocks;
    std::set<std::pair<block_idx, block_idx>>                          executedEdges;

    std::queue<std::pair<block_idx, block_idx>> cfgWorklist;
    std::queue<furlang::ir::instruction*>       ssaWorklist;

    std::unordered_map<furlang::ir::instruction*, block_idx> blockMap;

    auto getOperandLattice = [&](const furlang::ir::operand& op) -> lattice {
        if (op.type() == furlang::ir::operand_t::Integer) {
            lattice lat;
            lat.type  = lattice_t::Constant;
            lat.value = op.integer();
            return lat;
        }
        if (op.type() == furlang::ir::operand_t::Register) {
            auto reg = op.reg();
            if (latVals.find(reg) == latVals.end()) return { lattice_t::Top };
            return latVals[reg];
        }
        return { lattice_t::Bottom };
    };

    for (block_idx blockIdx = 0; blockIdx < func->blocks().size(); ++blockIdx) {
        const auto& block  = func->blocks()[blockIdx];
        auto&       instrs = block->instructions();
        for (auto it = instrs.begin(); it != instrs.end(); ++it) {
            const auto& instr     = *it;
            blockMap[instr.get()] = blockIdx;
            for (const auto& op : instr->sources()) {
                if (op->type() != furlang::ir::operand_t::Register) continue;
                edges[op->reg()].push_back(instr.get());
            }
        }

        blockMap[block->exit().get()] = blockIdx;
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

            if (executedEdges.count(edge) != 0) continue;
            executedEdges.insert(edge);

            bool firstVisit = (executableBlocks.find(to) == executableBlocks.end());
            executableBlocks.insert(to);

            const auto& block = func->blocks()[to];
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
            if (executableBlocks.find(blockIdx) == executableBlocks.end()) continue;

            lattice newLat = { lattice_t::Top };

            switch (instr->type()) {
            case furlang::ir::instruction_t::Phi: {
                auto& phi = dynamic_cast<furlang::ir::phi_instruction&>(*instr);
                for (const auto& [op, label] : phi.labels()) {
                    if (executedEdges.count({ label, blockIdx }) == 0) continue;
                    lattice opLat = getOperandLattice(op);
                    if (opLat.type == lattice_t::Bottom) newLat.type = lattice_t::Bottom;
                    if (opLat.type == lattice_t::Constant) {
                        if (newLat.type == lattice_t::Top) {
                            newLat = opLat;
                        } else if (newLat.type == lattice_t::Constant && newLat.value != opLat.value) {
                            newLat.type = lattice_t::Bottom;
                        }
                    }
                }
            } break;
            case furlang::ir::instruction_t::Assign: {
                newLat = getOperandLattice(*instr->sources().front());
            } break;
            case furlang::ir::instruction_t::BinaryOp: {
                lattice lhs = getOperandLattice(*instr->sources()[0]);
                lattice rhs = getOperandLattice(*instr->sources()[1]);

                if (lhs.type == lattice_t::Bottom || rhs.type == lattice_t::Bottom) {
                    newLat.type = lattice_t::Bottom;
                } else if (lhs.type == lattice_t::Constant && rhs.type == lattice_t::Constant) {
                    newLat.type = lattice_t::Constant;
                    switch (dynamic_cast<const furlang::ir::binary_op_instruction&>(*instr).op_type()) {
                    case furlang::ir::binary_op_instruction_t::Add: newLat.value = lhs.value + rhs.value; break;
                    case furlang::ir::binary_op_instruction_t::Sub: newLat.value = lhs.value - rhs.value; break;
                    case furlang::ir::binary_op_instruction_t::Mul: newLat.value = lhs.value * rhs.value; break;
                    case furlang::ir::binary_op_instruction_t::Div: newLat.value = lhs.value / rhs.value; break;
                    case furlang::ir::binary_op_instruction_t::Mod: newLat.value = lhs.value % rhs.value; break;
                    case furlang::ir::binary_op_instruction_t::Eq:
                        newLat.value = (lhs.value == rhs.value) ? 1 : 0;
                        break;
                    case furlang::ir::binary_op_instruction_t::NotEq:
                        newLat.value = (lhs.value != rhs.value) ? 1 : 0;
                        break;
                    case furlang::ir::binary_op_instruction_t::LessThan:
                        newLat.value = (lhs.value < rhs.value) ? 1 : 0;
                        break;
                    case furlang::ir::binary_op_instruction_t::GreaterThan:
                        newLat.value = (lhs.value > rhs.value) ? 1 : 0;
                        break;
                    case furlang::ir::binary_op_instruction_t::LessEq:
                        newLat.value = (lhs.value <= rhs.value) ? 1 : 0;
                        break;
                    case furlang::ir::binary_op_instruction_t::GreaterEq:
                        newLat.value = (lhs.value >= rhs.value) ? 1 : 0;
                        break;
                    }
                }
            } break;
            default: break;
            }

            if (instr->has_destination() && instr->destination().type() == furlang::ir::operand_t::Register) {
                auto dst = instr->destination().reg();
                if (!(latVals[dst] == newLat)) {
                    latVals[dst] = newLat;
                    for (auto* uInstr : edges[dst])
                        ssaWorklist.push(uInstr);
                }
            }

            if (instr == func->blocks()[blockIdx]->exit().get()) {
                auto* exit = func->blocks()[blockIdx]->exit().get();
                if (exit->type() == furlang::ir::instruction_t::Branch) {
                    auto& br = dynamic_cast<furlang::ir::branch_instruction&>(*exit);
                    cfgWorklist.push({ blockIdx, br.block() });
                } else if (exit->type() == furlang::ir::instruction_t::BranchCond) {
                    auto&   br   = dynamic_cast<furlang::ir::branch_cond_instruction&>(*exit);
                    lattice cond = getOperandLattice(*exit->sources()[0]);

                    if (cond.type == lattice_t::Constant) {
                        if (cond.value != 0)
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

    for (block_idx i = 0; i < func->blocks().size(); ++i) {
        if (executableBlocks.find(i) == executableBlocks.end()) {
            func->blocks()[i]->instructions().clear();
            continue;
        }

        const auto& block = func->blocks()[i];

        for (auto& instr : block->instructions()) {
            for (auto& op : instr->sources()) {
                if (op->type() != furlang::ir::operand_t::Register) continue;
                auto reg = op->reg();
                if (latVals[reg].type != lattice_t::Constant) continue;
                *op = furlang::ir::operand::new_integer(latVals[reg].value);
            }
        }

        auto* exit = block->exit().get();
        if (exit->type() != furlang::ir::instruction_t::BranchCond) continue;
        auto&   br   = dynamic_cast<furlang::ir::branch_cond_instruction&>(*exit);
        lattice cond = getOperandLattice(*exit->sources()[0]);
        if (cond.type != lattice_t::Constant) continue;
        block_idx target = (cond.value != 0) ? br.if_block() : br.else_block();
        block->exit()    = std::make_unique<furlang::ir::branch_instruction>(target);
    }
}

void ssa::dead_code_elimination(const std::unique_ptr<furlang::ir::function>& func) {
    using block_idx = furlang::ir::block_index;
    using reg_t     = furlang::ir::register_operand;

    std::unordered_map<reg_t, furlang::ir::instruction*> defMap;
    std::unordered_set<furlang::ir::instruction*>        alive;
    std::queue<furlang::ir::instruction*>                worklist;

    for (block_idx blockIdx = 0; blockIdx < func->blocks().size(); ++blockIdx) {
        const auto& block = func->blocks()[blockIdx];

        for (auto& instr : block->instructions()) {
            if (instr->has_destination() && instr->destination().type() == furlang::ir::operand_t::Register) {
                defMap[instr->destination().reg()] = instr.get();
            }
        }

        auto* exit = block->exit().get();
        alive.insert(exit);
        worklist.push(exit);
    }

    while (!worklist.empty()) {
        const auto* instr = worklist.front();
        worklist.pop();

        for (const auto& op : instr->sources()) {
            if (op->type() != furlang::ir::operand_t::Register) continue;
            auto src = op->reg();

            if (defMap.find(src) == defMap.end()) continue;
            auto* defInstr = defMap[src];
            if (alive.insert(defInstr).second) {
                worklist.push(defInstr);
            }
        }
    }

    for (block_idx blockIdx = 0; blockIdx < func->blocks().size(); ++blockIdx) {
        const auto& block  = func->blocks()[blockIdx];
        auto&       instrs = block->instructions();

        for (auto it = instrs.begin(); it != instrs.end();) {
            if (alive.find(it->get()) == alive.end()) {
                it = instrs.erase(it);
            } else {
                ++it;
            }
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
