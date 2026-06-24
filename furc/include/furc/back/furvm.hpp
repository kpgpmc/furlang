#ifndef FURC_BACK_FURVM_HPP
#define FURC_BACK_FURVM_HPP

#include "furlang/ir/operand.hpp"

#include <cstddef>
#include <furlang/ir/function.hpp>
#include <furlang/ir/instruction.hpp>
#include <furlang/ir/module.hpp>
#include <furvm/fwd.hpp>
#include <furvm/module.hpp>
#include <unordered_map>
#include <vector>

namespace furc {
namespace back {

class furvm_generator {
public:
    furvm_generator() = default;
public:
    static furvm::mod generate(furlang::ir::mod& mod);
private:
    static void generate_function(furvm::mod& mod, const furlang::ir::function& function);

    struct function_context {
        std::unordered_map<furlang::ir::block_index, std::size_t>              blockOffsets;
        std::unordered_map<furlang::ir::block_index, std::vector<std::size_t>> incompleteJumps;

        std::unordered_map<furlang::ir::register_operand, furvm::variable_t> variables;
        furvm::variable_t                                                    variableCounter{ 0 };
    };

    static void generate_instruction(furvm::mod& mod, function_context& ctx, const furlang::ir::instruction& instr);
    static void generate_operand(furvm::mod& mod, function_context& ctx, const furlang::ir::operand& operand);

    static void generate_jump(furvm::mod& mod, function_context& ctx, furlang::ir::block_index block, bool conditional);
};

} // namespace back
} // namespace furc

#endif // FURC_BACK_FURVM_HPP
