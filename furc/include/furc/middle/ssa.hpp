#ifndef FURC_MIDDLE_SSA_HPP
#define FURC_MIDDLE_SSA_HPP

#include "furc/middle/ir.hpp"

#include <cassert>
#include <limits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace furc {

class ssa {
public:
    struct cfg_block {
        std::unordered_set<std::uint64_t> preds;
        std::unordered_set<std::uint64_t> sucs;
    };

    struct ssa_block {
        std::size_t order = 0;

        std::uint64_t idom = -1;

        std::unordered_set<std::uint64_t> children; // Children of the block in dominator tree

        // Dominance Frontiers
        std::unordered_set<std::uint64_t> df;
    };

    struct register_info {
        std::unordered_set<std::uint64_t> sites; // Definition Sites
    };
public:
    ssa(ir_function& func) {
        m_registers.resize(func.regCount);
        compute_cfg(func.blocks, m_cfgBlocks);
        collect_registers(func.blocks, m_registers, m_globals);

        std::vector<std::uint64_t> order;
        compute_rpo(m_cfgBlocks, m_ssaBlocks, order);

        build_dtree(m_cfgBlocks, m_ssaBlocks, order);
        compute_dfrontiers(m_cfgBlocks, m_ssaBlocks);

        ssaification(func.blocks, m_cfgBlocks, m_ssaBlocks, m_registers, m_globals);
        rename(func.blocks, func.regCount, m_cfgBlocks, m_ssaBlocks, order);
    }
public:
    static void compute_cfg(const std::vector<ir_basic_block>& irBlocks, std::vector<cfg_block>& cfgBlocks);

    static void collect_registers(const std::vector<ir_basic_block>& irBlocks,
        std::vector<register_info>&                                  registers,
        std::unordered_set<std::uint64_t>&                           globals);

    static void build_dtree(const std::vector<cfg_block>& cfgBlocks,
        std::vector<ssa_block>&                           ssaBlocks,
        const std::vector<std::size_t>&                   order);

    static void compute_dfrontiers(const std::vector<cfg_block>& cfgBlocks, std::vector<ssa_block>& ssaBlocks);

    static void compute_rpo(std::vector<cfg_block>& cfgBlocks,
        std::vector<ssa_block>&                     ssaBlocks,
        std::vector<std::size_t>&                   order);

    static void ssaification(std::vector<ir_basic_block>& irBlocks,
        const std::vector<cfg_block>&                     cfgBlocks,
        const std::vector<ssa_block>&                     ssaBlocks,
        const std::vector<register_info>&                 registers,
        const std::unordered_set<std::uint64_t>&          globals);

    static void rename(std::vector<ir_basic_block>& irBlocks,
        std::size_t                                 regCount,
        const std::vector<cfg_block>&               cfgBlocks,
        std::vector<ssa_block>&                     ssaBlocks,
        const std::vector<std::uint64_t>&           order);
private:
    static void rename_rec(std::vector<std::uint64_t>& counters,
        std::vector<std::stack<std::uint64_t>>&        stacks,
        std::vector<ir_basic_block>&                   irBlocks,
        const std::vector<cfg_block>&                  cfgBlocks,
        const std::vector<ssa_block>&                  ssaBlocks,
        std::size_t                                    blockIdx);
private:
    static void rpo_dfs(std::unordered_set<std::size_t>& visited,
        std::vector<std::size_t>&                        order,
        std::size_t                                      block,
        const std::vector<cfg_block>&                    blocks);

    static std::size_t intersect(std::vector<ssa_block>& m_blocks, std::size_t b1, std::size_t b2);
private:
    std::vector<cfg_block>            m_cfgBlocks;
    std::vector<ssa_block>            m_ssaBlocks;
    std::vector<register_info>        m_registers;
    std::unordered_set<std::uint64_t> m_globals;
};

} // namespace furc

#endif // FURC_MIDDLE_SSA_HPP
