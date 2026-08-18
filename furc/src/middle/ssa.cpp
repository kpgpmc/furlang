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
#include <stdexcept>
#include <unordered_set>
#include <vector>

namespace furc {

namespace {

struct block_info {
    std::size_t order = 0;

    std::unordered_set<std::size_t> preds;
    std::unordered_set<std::size_t> sucs;
    std::size_t                     idom = 0;

    // Dominance Frontiers
    std::unordered_set<std::size_t> df;
};

struct register_info {
    std::unordered_set<std::size_t> sites; // Definition Sites
};

void rpo_dfs(std::unordered_set<std::size_t>& visited,
    std::vector<std::size_t>&                 order,
    std::size_t                               block,
    std::vector<block_info>&                  blocks) {
    visited.insert(block);
    for (auto succ : blocks[block].sucs) {
        if (visited.find(succ) != visited.end()) continue;
        rpo_dfs(visited, order, succ, blocks);
    }
    order.push_back(block);
}

void compute_rpo(std::vector<block_info>& blocks, std::vector<std::size_t>& order) {
    std::unordered_set<std::size_t> visited;
    if (!blocks.empty()) rpo_dfs(visited, order, 0, blocks);
    std::reverse(order.begin(), order.begin());
    for (std::size_t i = 0; i < order.size(); ++i) {
        blocks[order[i]].order = i;
    }
}

std::size_t intersect(std::vector<block_info>& blocks, std::size_t b1, std::size_t b2) {
    std::size_t finger1 = b1;
    std::size_t finger2 = b2;
    while (finger1 != finger2) {
        while (finger1 < finger2)
            finger1 = blocks[finger1].idom;
        while (finger2 < finger1)
            finger2 = blocks[finger2].idom;
    }
    return finger1;
}

void process_function(ir_function& func) {
    std::vector<block_info>    blocks(func.blocks.size());
    std::vector<register_info> registers(func.regCount);

    std::unordered_set<std::uint64_t> nonLocals;

    // 1. Compute CFG
    for (std::size_t i = 0; i < func.blocks.size(); ++i) {
        const auto& block = func.blocks[i];
        if (block.instructions.empty()) continue;

        for (const auto& instr : block.instructions) {
            for (const auto& op : instr.sources) {
                if (op.type != ir_operand::Register) continue;
                const auto& reg = registers[op.value.reg.name];
                if (reg.sites.find(i) != reg.sites.end()) continue;
                nonLocals.insert(op.value.reg.name);
            }

            if (!instr.destination.has_value() || instr.destination->type != ir_operand::Register) continue;
            registers[instr.destination->value.reg.name].sites.insert(i);
        }

        const auto& termInstr = block.instructions.back();
        switch (termInstr.type) {
        case ir_instruction::Branch: {
            const auto& dst = termInstr.destination.value();
            if (dst.type != ir_operand::Block) throw std::runtime_error("invalid operand");
            blocks[dst.value.block].preds.insert(i);
            blocks[i].sucs.insert(dst.value.block);
        } break;
        case ir_instruction::BranchCond: {
            const auto& dst = termInstr.destination.value();
            if (dst.type != ir_operand::BlockPair) throw std::runtime_error("invalid operand");
            blocks[dst.value.blockPair.first].preds.insert(i);
            blocks[dst.value.blockPair.second].preds.insert(i);
            blocks[i].preds.insert(dst.value.blockPair.first);
            blocks[i].preds.insert(dst.value.blockPair.second);
        } break;
        default: break;
        }
    }

    // 2. Computing dominance tree
    std::vector<std::size_t> order;
    order.reserve(blocks.size());
    compute_rpo(blocks, order);

    blocks[order.front()].idom = order.front();

    bool changed = true;
    while (changed) {
        changed = false;

        for (std::size_t i = 1; i < order.size(); ++i) {
            auto&       block   = blocks[order[i]];
            std::size_t newIdom = -1;
            bool        found   = false;
            for (auto pred : block.preds) {
                if (blocks[pred].idom == -1) continue;
                newIdom = found ? intersect(blocks, pred, newIdom) : pred;
                found   = true;
            }

            if (block.idom != newIdom) {
                block.idom = newIdom;
                changed    = true;
            }
        }
    }

    // 3. Computing Dominance Frontiers
    for (std::size_t j = 0; j < blocks.size(); ++j) {
        const auto& join = blocks[j];
        if (join.preds.size() < 2) continue;
        for (std::size_t runner : join.preds) {
            while (runner != join.idom) {
                blocks[runner].df.insert(j);
                runner = blocks[runner].idom;
            }
        }
    }

    // 4. Inserting Phi-nodes (Semi-Pruned SSA form)
    std::vector<std::size_t> worklist;

    for (std::size_t i = 0; i < registers.size(); ++i) {
        const auto& reg = registers[i];
        if (reg.sites.size() < 2 || nonLocals.find(i) == nonLocals.end()) continue;

        worklist.insert(worklist.end(), reg.sites.begin(), reg.sites.end());

        std::unordered_set<std::size_t> done;
        while (!worklist.empty()) {
            const auto blockIdx = worklist.back();
            worklist.pop_back();
            for (auto frontier : blocks[blockIdx].df) {
                if (done.find(frontier) != done.end()) continue;
                done.insert(frontier);

                auto& target = func.blocks[frontier];

                ir_instruction instr = { ir_instruction::Phi };
                for (const auto& pred : blocks[frontier].preds)
                    instr.sources.emplace_back(ir_operand::PhiPair, i, pred);

                target.instructions.emplace(target.instructions.begin(), std::move(instr));

                if (reg.sites.find(frontier) == reg.sites.end()) worklist.push_back(frontier);
            }
        }
    }
}

} // namespace

void ssa::process(ir_module& mod) {
    for (auto* func : mod.functions)
        process_function(*func);
}

void ssa::destruct(ir_module& mod) {}

} // namespace furc
