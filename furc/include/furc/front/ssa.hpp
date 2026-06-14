#ifndef FURC_FRONT_SSA_HPP
#define FURC_FRONT_SSA_HPP

#include "furlang/ir/function.hpp"
#include "furlang/ir/instruction.hpp"
#include "furlang/ir/module.hpp"

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace furc {
namespace front {

class ssa {
    using block_map_t = std::unordered_map<furlang::ir::block_index, std::vector<furlang::ir::block_index>>;
public:
    static void optimize(furlang::ir::module& mod);
private:
    static void optimize(const std::unique_ptr<furlang::ir::function>& func);

    static void de_ssa(const std::unique_ptr<furlang::ir::function>& func);

    static void constant_propagation(const std::unique_ptr<furlang::ir::function>& func);

    static void dead_code_elimination(const std::unique_ptr<furlang::ir::function>& func);

    static void copy_propagation(const std::unique_ptr<furlang::ir::function>& func);

    static void dfs_rpo(furlang::ir::block_index      block,
        const block_map_t&                            successors,
        std::unordered_set<furlang::ir::block_index>& visited,
        std::vector<furlang::ir::block_index>&        rpo);
};

} // namespace front
} // namespace furc

#endif // FURC_FRONT_SSA_HPP
