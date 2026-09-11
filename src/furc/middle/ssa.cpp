/**
 * Sources:
 *  - Practical Improvements to the Construction and Deconstruction of Static Single Assignment Form:
 * https://web.archive.org/web/20100607003509/http://www.cs.rice.edu/~harv/my_papers/ssa.pdf
 *  - A Simple, Fast Dominance Algorithm:
 * https://www.researchgate.net/publication/2569680_A_Simple_Fast_Dominance_Algorithm
 */

#include "furc/middle/ssa.hpp"

#include "furc/middle/ir.hpp"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <stack>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace furc {

void ssa::compute_cfg(const std::vector<ir_basic_block>& irBlocks, std::vector<cfg_block>& cfgBlocks) {
    cfgBlocks.resize(irBlocks.size());

    for (std::size_t i = 0; i < irBlocks.size(); ++i) {
        const auto& block = irBlocks[i];
        if (block.instructions.empty()) continue;

        const auto& termInstr = block.instructions.back();
        switch (termInstr.type) {
        case ir_instruction::Branch: {
            const auto& dst = termInstr.destination.value();
            assert(dst.type == ir_operand::Block);
            cfgBlocks[dst.value.block].preds.insert(i);
            cfgBlocks[i].sucs.insert(dst.value.block);
        } break;
        case ir_instruction::BranchCond: {
            const auto& dst = termInstr.destination.value();
            assert(dst.type == ir_operand::BlockPair);
            cfgBlocks[dst.value.blockPair.first].preds.insert(i);
            cfgBlocks[dst.value.blockPair.second].preds.insert(i);
            cfgBlocks[i].sucs.insert(dst.value.blockPair.first);
            cfgBlocks[i].sucs.insert(dst.value.blockPair.second);
        } break;
        default: break;
        }
    }
}

void ssa::collect_registers(const std::vector<ir_basic_block>& irBlocks,
    std::vector<register_info>&                                registers,
    std::unordered_set<std::uint64_t>&                         globals) {
    for (std::size_t i = 0; i < irBlocks.size(); ++i) {
        const auto& block = irBlocks[i];
        for (const auto& instr : block.instructions) {
            for (const auto& src : instr.sources) {
                if (src.type != ir_operand::Register) continue;
                const auto& reg = registers.at(src.value.reg.name);
                if (reg.sites.find(i) != reg.sites.end()) continue;
                globals.insert(src.value.reg.name);
            }
            if (!instr.destination.has_value() || instr.destination->type != ir_operand::Register) continue;
            registers[instr.destination->value.reg.name].sites.insert(i);
        }
    }
}

void ssa::build_dtree(const std::vector<cfg_block>& cfgBlocks,
    std::vector<ssa_block>&                         ssaBlocks,
    const std::vector<std::size_t>&                 order) {
    ssaBlocks[order.front()].idom = order.front();

    bool changed = true;
    while (changed) {
        changed = false;
        for (auto it = order.begin() + 1; it != order.end(); ++it) {
            static constexpr std::uint64_t INVALID = std::numeric_limits<std::uint64_t>::max();

            std::uint64_t newIdom = -1;
            bool          found   = false;

            for (std::uint64_t pred : cfgBlocks[*it].preds) {
                if (ssaBlocks[pred].idom == INVALID) continue;
                newIdom = found ? intersect(ssaBlocks, pred, newIdom) : pred;
                found   = true;
            }
            if (ssaBlocks[*it].idom != newIdom) {
                ssaBlocks[*it].idom = newIdom;
                changed             = true;
            }
        }
    }
}

void ssa::compute_dfrontiers(const std::vector<cfg_block>& cfgBlocks, std::vector<ssa_block>& ssaBlocks) {
    for (std::uint64_t i = 0; i < ssaBlocks.size(); ++i) {
        if (cfgBlocks[i].preds.size() < 2) continue;
        const auto& cfgBlock = cfgBlocks[i];
        auto&       ssaBlock = ssaBlocks[i];

        for (std::uint64_t worker : cfgBlock.preds) {
            while (worker != ssaBlock.idom) {
                ssaBlocks[worker].df.insert(i);
                worker = ssaBlocks[worker].idom;
            }
        }
    }
}

void ssa::compute_rpo(std::vector<cfg_block>& cfgBlocks,
    std::vector<ssa_block>&                   ssaBlocks,
    std::vector<std::size_t>&                 order) {
    std::unordered_set<std::size_t> visited;
    if (!cfgBlocks.empty()) rpo_dfs(visited, order, 0, cfgBlocks);
    std::reverse(order.begin(), order.end());
    ssaBlocks.resize(cfgBlocks.size());
    for (std::size_t i = 0; i < order.size(); ++i) {
        ssaBlocks[order[i]].order = i;
    }
}

void ssa::ssaification(std::vector<ir_basic_block>& irBlocks,
    const std::vector<cfg_block>&                   cfgBlocks,
    const std::vector<ssa_block>&                   ssaBlocks,
    const std::vector<register_info>&               registers,
    const std::unordered_set<std::uint64_t>&        globals) {
    std::vector<std::uint64_t> worklist;

    for (std::uint64_t i = 0; i < registers.size(); ++i) {
        const auto& reg = registers[i];
        if (reg.sites.size() < 2 || globals.find(i) == globals.end()) continue;
        worklist.insert(worklist.end(), reg.sites.begin(), reg.sites.end());

        std::unordered_set<std::uint64_t> done;
        while (!worklist.empty()) {
            const auto blockIdx = worklist.back();
            worklist.pop_back();

            for (auto frontier : ssaBlocks[blockIdx].df) {
                if (done.find(frontier) != done.end()) continue;
                done.insert(frontier);

                auto&          target = irBlocks[frontier];
                ir_instruction instr  = { ir_instruction::Phi, ir_operand{ ir_operand::Register, i } };
                for (const auto& pred : cfgBlocks[frontier].preds)
                    instr.sources.emplace_back(ir_operand::PhiPair, i, pred);
                target.instructions.emplace(target.instructions.begin(), std::move(instr));

                if (reg.sites.find(frontier) == reg.sites.end()) worklist.push_back(frontier);
            }
        }
    }
}

void ssa::rename(std::vector<ir_basic_block>& irBlocks,
    std::size_t                               regCount,
    const std::vector<cfg_block>&             cfgBlocks,
    std::vector<ssa_block>&                   ssaBlocks,
    const std::vector<std::uint64_t>&         order) {
    std::vector<std::uint64_t>             counters;
    std::vector<std::stack<std::uint64_t>> stacks;

    counters.resize(regCount);
    stacks.resize(regCount);

    for (auto it = order.begin() + 1; it != order.end(); ++it) {
        std::uint64_t parent = ssaBlocks[*it].idom;
        if (parent != std::numeric_limits<std::uint64_t>::max()) ssaBlocks[parent].children.emplace(*it);
    }

    rename_rec(counters, stacks, irBlocks, cfgBlocks, ssaBlocks, order.front());
}

void ssa::rename_rec(std::vector<std::uint64_t>& counters,
    std::vector<std::stack<std::uint64_t>>&      stacks,
    std::vector<ir_basic_block>&                 irBlocks,
    const std::vector<cfg_block>&                cfgBlocks,
    const std::vector<ssa_block>&                ssaBlocks,
    std::size_t                                  blockIdx) {
    std::unordered_map<std::uint64_t, std::size_t> pushed;

    auto& block = irBlocks[blockIdx];
    for (auto& instr : block.instructions) {
        if (instr.type == ir_instruction::Phi) {
            auto reg = instr.destination->value.reg.name;
            stacks[reg].push(instr.destination->value.reg.ver = counters[reg]++);
            ++pushed[reg];
            continue;
        }

        for (auto& op : instr.sources) {
            if (op.type != ir_operand::Register) continue;
            auto reg         = op.value.reg.name;
            op.value.reg.ver = stacks[reg].top();
        }

        if (!instr.destination.has_value() || instr.destination->type != ir_operand::Register) continue;
        auto reg = instr.destination->value.reg.name;
        stacks[reg].push(instr.destination->value.reg.ver = counters[reg]++);
        ++pushed[reg];
    }

    for (auto succIdx : cfgBlocks[blockIdx].sucs) {
        auto& succ = irBlocks[succIdx];
        for (auto& instr : succ.instructions) {
            if (instr.type != ir_instruction::Phi) break;
            for (auto& op : instr.sources) {
                if (op.value.phiPair.block != blockIdx) continue;
                op.value.phiPair.reg.ver = stacks[op.value.phiPair.reg.name].top();
            }
        }
    }

    for (std::uint64_t child : ssaBlocks[blockIdx].children)
        rename_rec(counters, stacks, irBlocks, cfgBlocks, ssaBlocks, child);

    for (auto [reg, count] : pushed)
        while ((count--) > 0)
            stacks[reg].pop();
}

void ssa::rpo_dfs(std::unordered_set<std::size_t>& visited,
    std::vector<std::size_t>&                      order,
    std::size_t                                    block,
    const std::vector<cfg_block>&                  blocks) {
    visited.insert(block);
    for (auto succ : blocks[block].sucs) {
        if (visited.find(succ) != visited.end()) continue;
        rpo_dfs(visited, order, succ, blocks);
    }
    order.push_back(block);
}

std::size_t ssa::intersect(std::vector<ssa_block>& m_blocks, std::size_t b1, std::size_t b2) {
    while (b1 != b2) {
        while (m_blocks[b1].order > m_blocks[b2].order)
            b1 = m_blocks[b1].idom;
        while (m_blocks[b2].order > m_blocks[b1].order)
            b2 = m_blocks[b2].idom;
    }
    return b1;
}

// // 5. Renaming
// std::vector<std::uint64_t>             counters;
// std::vector<std::stack<std::uint64_t>> stacks;
//
// counters.resize(func.regCount);
// stacks.resize(func.regCount);
//
// for (std::size_t i = 1; i < order.size(); ++i) {
// std::size_t parent = blocks[order[i]].idom;
// if (parent != std::numeric_limits<std::size_t>::max()) blocks[parent].children.emplace(order[i]);
// }
//
// auto rename = [&counters, &stacks, &blocks, &func](auto& self, std::size_t blockIdx) -> void {
// std::unordered_map<std::size_t, std::size_t> pushed;
//
// auto& block = func.blocks[blockIdx];
// for (auto& instr : block.instructions) {
// if (instr.type == ir_instruction::Phi) {
// auto reg                         = instr.destination->value.reg.name;
// auto idx                         = counters[reg]++;
// instr.destination->value.reg.ver = idx;
// stacks[reg].push(idx);
// ++pushed[reg];
// continue;
// }
//
// for (auto& op : instr.sources) {
// if (op.type != ir_operand::Register) continue;
// auto reg         = op.value.reg.name;
// op.value.reg.ver = stacks[reg].top();
// }
//
// if (!instr.destination.has_value() || instr.destination->type != ir_operand::Register) continue;
// auto reg                         = instr.destination->value.reg.name;
// auto idx                         = counters[reg]++;
// instr.destination->value.reg.ver = idx;
// stacks[reg].push(idx);
// ++pushed[reg];
// }
//
// for (auto succIdx : blocks[blockIdx].sucs) {
// auto& succ = func.blocks[succIdx];
// for (auto& instr : succ.instructions) {
// if (instr.type != ir_instruction::Phi) break;
// for (auto& op : instr.sources) {
// if (op.value.phiPair.block != blockIdx) continue;
// op.value.phiPair.reg.ver = stacks[op.value.phiPair.reg.name].top();
// }
// }
// }
//
// for (std::size_t child : blocks[blockIdx].children)
// self(self, child);
//
// for (auto [reg, count] : pushed)
// while (count--)
// stacks[reg].pop();
// };
// rename(rename, order.front());
// }

} // namespace furc
