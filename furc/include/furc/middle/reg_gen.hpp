/**
 * SSA destruction (out-of-SSA phase) for register-based targets based on "Mechanizing Conventional SSA for a
 * Verified Destruction with Coalescing" by Delphine Demange and Yon Fernandez de Retana
 * (https://dl.acm.org/doi/pdf/10.1145/2892208.2892222 09/11/2026).
 */
#ifndef FURC_MIDDLE_REG_GEN_HPP
#define FURC_MIDDLE_REG_GEN_HPP

#include "furc/middle/ir.hpp"
#include "furc/middle/ssa.hpp"

#include <cassert>
#include <cstdint>
#include <unordered_set>
#include <vector>

namespace furc {

class reg_gen {
public:
    class disjoint_set {
    public:
        std::uint64_t find(std::uint64_t var) {
            if (m_parents.find(var) == m_parents.end()) m_parents.emplace(var, var);
            if (m_parents[var] == var) return var;
            return find(m_parents[var]);
        }

        void unite(std::uint64_t var1, std::uint64_t var2) {
            std::uint64_t rep1 = find(var1);
            std::uint64_t rep2 = find(var2);
            if (rep1 != rep2) m_parents[rep1] = rep2;
        }
    private:
        std::unordered_map<std::uint64_t, std::uint64_t> m_parents;
    };
public:
    struct block_info {
        std::unordered_set<std::uint64_t> defs;
        std::unordered_set<std::uint64_t> uses;
        std::unordered_set<std::uint64_t> liveIn;
        std::unordered_set<std::uint64_t> liveOut;
    };
public:
    reg_gen(ir_function& func, ssa& ssa) {
        std::vector<block_info> lifeBlocks;
        live_analysis(lifeBlocks, func.blocks, ssa.cfgBlocks);
        remove_interference(func.blocks, lifeBlocks);
        merge(func.blocks);
    }
public:
    static void live_analysis(std::vector<block_info>& lifeBlocks,
        const std::vector<ir_basic_block>&             irBlocks,
        const std::vector<ssa::cfg_block>&             cfgBlocks) {
        lifeBlocks.resize(irBlocks.size());
        for (std::uint64_t i = 0; i < irBlocks.size(); ++i) {
            const auto& irBlock = irBlocks[i];
            auto&       block   = lifeBlocks[i];

            for (const auto& instr : irBlock.instructions) {
                for (const auto& op : instr.sources) {
                    if (op.type != ir_operand::Register) continue;
                    if (block.defs.find(op.value.reg) != block.defs.end()) continue;
                    block.uses.insert(op.value.reg);
                }

                if (!instr.destination.has_value() || instr.destination->type != ir_operand::Register) continue;
                block.defs.insert(instr.destination->value.reg);
            }
        }

        bool changed = true;
        while (changed) {
            changed = false;

            for (std::uint64_t i = 0; i < irBlocks.size(); ++i) {
                const auto& irBlock = irBlocks[i];
                auto&       block   = lifeBlocks[i];

                std::unordered_set<std::uint64_t> newSet;
                for (auto succ : cfgBlocks[i].sucs) {
                    newSet.insert(lifeBlocks[succ].liveIn.begin(), lifeBlocks[succ].liveIn.end());
                }

                if (newSet != block.liveOut) {
                    block.liveOut = newSet;
                    changed       = true;
                }

                newSet.clear();
                newSet.insert(block.uses.begin(), block.uses.end());
                for (const auto& var : block.liveOut) {
                    if (block.defs.find(var) != block.defs.end()) continue;
                    newSet.insert(var);
                }

                if (newSet != block.liveIn) {
                    block.liveIn = newSet;
                    changed      = true;
                }
            }
        }
    }

    static void remove_interference(std::vector<ir_basic_block>& irBlocks, const std::vector<block_info>& lifeBlocks) {
        for (std::uint64_t i = 0; i < irBlocks.size(); ++i) {
            auto& irBlock = irBlocks[i];

            for (auto it = irBlock.instructions.begin(), end = irBlock.instructions.end();
                it != end && it->type == ir_instruction::Phi;
                ++it) {
                assert(it->destination.has_value() && it->destination->type == ir_operand::Register);
                const auto& phiDst = it->destination->value.reg;

                for (auto& op : it->sources) {
                    assert(op.type == ir_operand::PhiPair);
                    const auto& predBlock = lifeBlocks[op.value.phiPair.block];
                    auto&       phiArg    = op.value.phiPair.reg;
                    if (predBlock.liveOut.count(phiArg) == 0 || phiArg == phiDst) continue;

                    auto oldArg = phiArg;
                    phiArg.ver  = 0; // TODO: Allocate temporary registers

                    auto& irBlock = irBlocks[op.value.phiPair.block];
                    assert(!irBlock.instructions.empty());
                    auto it = irBlock.instructions.end() - 1;
                    if (ir_instruction::is_terminating(it->type)) --it;
                    irBlock.instructions.emplace(it,
                        ir_instruction{ ir_instruction::Move,
                            ir_operand::reg(phiArg.name, phiArg.ver),
                            { ir_operand::reg(oldArg.name, oldArg.ver) } });
                }
            }
        }
    }

    static void merge(std::vector<ir_basic_block>& irBlocks) {
        disjoint_set dj;

        for (const auto& block : irBlocks) {
            for (const auto& instr : block.instructions) {
                if (instr.type != ir_instruction::Phi) break;

                assert(instr.destination.has_value() && instr.destination->type == ir_operand::Register);
                const auto& phiDst = instr.destination->value.reg;
                dj.find(phiDst);

                for (const auto& op : instr.sources) {
                    assert(op.type == ir_operand::PhiPair);
                    const auto& predBlock = op.value.phiPair.block;
                    const auto& phiArg    = op.value.phiPair.reg;
                    dj.unite(phiDst, phiArg);
                }
            }
        }

        for (auto& block : irBlocks) {
            auto it = block.instructions.begin();
            while (it != block.instructions.end() && it->type == ir_instruction::Phi) {
                it = block.instructions.erase(it);
            }
            for (; it != block.instructions.end(); ++it) {
                for (auto& op : it->sources) {
                    if (op.type != ir_operand::Register) continue;
                    op.value.reg = dj.find(op.value.reg);
                }

                if (!it->destination.has_value() || it->destination->type != ir_operand::Register) continue;
                it->destination->value.reg = dj.find(it->destination->value.reg);
            }
        }
    }
};

} // namespace furc

#endif // FURC_MIDDLE_REG_GEN_HPP
