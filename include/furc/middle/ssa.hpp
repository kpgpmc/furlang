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
        registers.resize(func.regCount);
        compute_cfg(func.blocks, cfgBlocks);
        collect_registers(func.blocks, registers, globals);

        std::vector<std::uint64_t> order;
        compute_rpo(cfgBlocks, ssaBlocks, order);

        build_dtree(cfgBlocks, ssaBlocks, order);
        compute_dfrontiers(cfgBlocks, ssaBlocks);

        ssaification(func.blocks, cfgBlocks, ssaBlocks, registers, globals);
        rename(func.blocks, func.regCount, cfgBlocks, ssaBlocks, order);
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
public:
    std::vector<cfg_block>            cfgBlocks;
    std::vector<ssa_block>            ssaBlocks;
    std::vector<register_info>        registers;
    std::unordered_set<std::uint64_t> globals;
};

} // namespace furc

#endif // FURC_MIDDLE_SSA_HPP
