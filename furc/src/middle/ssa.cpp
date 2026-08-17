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
    std::vector<block_info> blocks(func.blocks.size());

    // 1. Compute CFG
    for (std::size_t i = 0; i < func.blocks.size(); ++i) {
        const auto& block = func.blocks[i];
        if (block.instructions.empty()) continue;

        const auto& termInstr = block.instructions.back();
        switch (termInstr.type) {
        case ir_instruction::Branch: {
            const auto& src = termInstr.sources.front();
            if (src.type != ir_operand::Block) throw std::runtime_error("invalid operand");
            blocks[src.value.block].preds.insert(i);
            blocks[i].sucs.insert(src.value.block);
        } break;
        case ir_instruction::BranchCond: {
            const auto& src = termInstr.sources.front();
            if (src.type != ir_operand::Block) throw std::runtime_error("invalid operand");
            blocks[src.value.blockPair.first].preds.insert(i);
            blocks[src.value.blockPair.second].preds.insert(i);
            blocks[i].preds.insert(src.value.blockPair.first);
            blocks[i].preds.insert(src.value.blockPair.second);
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
}

} // namespace

void ssa::process(ir_module& mod) {
    for (auto* func : mod.functions)
        process_function(*func);
}

void ssa::destruct(ir_module& mod) {}

} // namespace furc
