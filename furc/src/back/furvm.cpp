#include "furc/back/furvm.hpp"

#include "furlang/ir/function.hpp"
#include "furlang/ir/instruction.hpp"
#include "furvm/fwd.hpp"

#include <furvm/instruction.hpp>
#include <stdexcept>

namespace furc::back {

furvm::mod furvm_generator::generate(furlang::ir::mod& mod) {
    furvm::mod vmMod;

    for (const auto& function : mod.functions()) {
        generate_function(vmMod, *function);
    }

    return vmMod;
}

void furvm_generator::generate_function(furvm::mod& mod, const furlang::ir::function& function) {
    switch (function.type()) {
    case furlang::ir::function_t::Normal: {
        if (function.access() == furlang::ir::function_access_t::Public)
            mod.emplace_function(function.name(), function.param_count(), mod.bytecode().size()).dispatch();
        else
            mod.emplace_function_private(function.name(), function.param_count(), mod.bytecode().size()).dispatch();

        function_context ctx;
        for (furlang::ir::block_index idx = 0; idx < function.blocks().size(); ++idx) {
            if (auto it = ctx.incompleteJumps.find(idx); it != ctx.incompleteJumps.end()) {
                for (std::size_t offset : it->second) {
                    mod.bytecode()[offset] = mod.bytecode().size() - offset - 1;
                }
                ctx.incompleteJumps.erase(it);
            }

            ctx.blockOffsets[idx] = mod.bytecode().size();
            for (const auto& instr : function.blocks()[idx]->instructions())
                generate_instruction(mod, ctx, *instr);
            generate_instruction(mod, ctx, *function.blocks()[idx]->exit());
        }
    } break;
    case furlang::ir::function_t::Import: {
        throw std::runtime_error("unimplemented");
        // mod.emplace_function_private(function.name(), function.param_count(), mod.bytecode().size()).dispatch();
    } break;
    case furlang::ir::function_t::Native: {
        if (function.access() == furlang::ir::function_access_t::Public)
            mod.emplace_function(function.name(), function.param_count(), function.name()).dispatch();
        else
            mod.emplace_function_private(function.name(), function.param_count(), function.name()).dispatch();
    } break;
    }
}

static inline furvm::instruction_t op_type(furlang::ir::instruction_t type) {
    switch (type) {
    // Unary
    case furlang::ir::instruction_t::Pointerof: return furvm::instruction_t::Pointerof;

    // Binary
    case furlang::ir::instruction_t::Add: return furvm::instruction_t::Add;
    case furlang::ir::instruction_t::Sub: return furvm::instruction_t::Sub;
    case furlang::ir::instruction_t::Mul: return furvm::instruction_t::Mul;
    case furlang::ir::instruction_t::Div: return furvm::instruction_t::Div;
    case furlang::ir::instruction_t::Mod: return furvm::instruction_t::Mod;
    case furlang::ir::instruction_t::Eq: return furvm::instruction_t::Equals;
    case furlang::ir::instruction_t::NotEq: return furvm::instruction_t::NotEquals;
    case furlang::ir::instruction_t::LessThan: return furvm::instruction_t::LessThan;
    case furlang::ir::instruction_t::GreaterThan: return furvm::instruction_t::GreaterThan;
    case furlang::ir::instruction_t::LessEq: return furvm::instruction_t::LessEqual;
    case furlang::ir::instruction_t::GreaterEq: return furvm::instruction_t::GreaterEqual;

    default: throw std::runtime_error("unreachable");
    }
}

void furvm_generator::generate_instruction(furvm::mod& mod,
    function_context&                                  ctx,
    const furlang::ir::instruction&                    instr) {
    for (const auto& operand : instr.sources())
        generate_operand(mod, ctx, *operand);

    switch (instr.type()) {
    case furlang::ir::instruction_t::Assign: {
        if (ctx.variables.find(instr.destination().reg()) == ctx.variables.end()) {
            ctx.variables[instr.destination().reg()] = ctx.variableCounter++;
        }
        auto var = ctx.variables[instr.destination().reg()];
        mod.bytecode().push_back(static_cast<furvm::byte>(furvm::instruction_t::Store));
        mod.bytecode().push_back((var >> 0) & 0xFF);
        mod.bytecode().push_back((var >> 8) & 0xFF);
        static_assert(sizeof(var) == 2, "sizeof(furvm::variable_t) has changed");
    } break;
    case furlang::ir::instruction_t::Add:
    case furlang::ir::instruction_t::Sub:
    case furlang::ir::instruction_t::Mul:
    case furlang::ir::instruction_t::Div:
    case furlang::ir::instruction_t::Mod:
    case furlang::ir::instruction_t::Eq:
    case furlang::ir::instruction_t::NotEq:
    case furlang::ir::instruction_t::LessThan:
    case furlang::ir::instruction_t::GreaterThan:
    case furlang::ir::instruction_t::LessEq:
    case furlang::ir::instruction_t::GreaterEq: {
        mod.bytecode().push_back(static_cast<furvm::byte>(op_type(instr.type())));

        if (ctx.variables.find(instr.destination().reg()) == ctx.variables.end()) {
            ctx.variables[instr.destination().reg()] = ctx.variableCounter++;
        }
        auto var = ctx.variables[instr.destination().reg()];
        mod.bytecode().push_back(static_cast<furvm::byte>(furvm::instruction_t::Store));
        mod.bytecode().push_back((var >> 0) & 0xFF);
        mod.bytecode().push_back((var >> 8) & 0xFF);
        static_assert(sizeof(var) == 2, "sizeof(furvm::variable_t) has changed");
    } break;
    case furlang::ir::instruction_t::Pointerof: {
        mod.bytecode().push_back(static_cast<furvm::byte>(op_type(instr.type())));

        if (ctx.variables.find(instr.destination().reg()) == ctx.variables.end()) {
            ctx.variables[instr.destination().reg()] = ctx.variableCounter++;
        }
        auto var = ctx.variables[instr.destination().reg()];
        mod.bytecode().push_back(static_cast<furvm::byte>(furvm::instruction_t::Store));
        mod.bytecode().push_back((var >> 0) & 0xFF);
        mod.bytecode().push_back((var >> 8) & 0xFF);
        static_assert(sizeof(var) == 2, "sizeof(furvm::variable_t) has changed");
    } break;
    case furlang::ir::instruction_t::Branch: {
        const auto& branch = dynamic_cast<const furlang::ir::branch_instruction&>(instr);
        generate_jump(mod, ctx, branch.block(), false);
    } break;
    case furlang::ir::instruction_t::BranchCond: {
        const auto& branch = dynamic_cast<const furlang::ir::branch_cond_instruction&>(instr);
        generate_jump(mod, ctx, branch.if_block(), true);
        generate_jump(mod, ctx, branch.else_block(), false);
    } break;
    case furlang::ir::instruction_t::Return: {
        if (instr.sources().empty()) {
            mod.bytecode().push_back(static_cast<furvm::byte>(furvm::instruction_t::Return));
        } else {
            mod.bytecode().push_back(static_cast<furvm::byte>(furvm::instruction_t::ReturnValue));
        }
    } break;
    case furlang::ir::instruction_t::Call: {
        const auto& call = dynamic_cast<const furlang::ir::call_instruction&>(instr);

        // TODO: Implement a queue for unknown functions
        furvm::function_id func = mod.get_function_id(call.name());
        mod.bytecode().push_back(static_cast<furvm::byte>(furvm::instruction_t::Call));
        mod.bytecode().push_back((func >> 0) & 0xFF);
        mod.bytecode().push_back((func >> 8) & 0xFF);
    } break;
    case furlang::ir::instruction_t::Alloca: throw std::runtime_error("unimplemented instruction");
    case furlang::ir::instruction_t::Phi: throw std::runtime_error("unreachable");
    }
}

void furvm_generator::generate_operand(furvm::mod& mod, function_context& ctx, const furlang::ir::operand& operand) {
    switch (operand.type()) {
    case furlang::ir::operand_t::Register: {
        if (ctx.variables.find(operand.reg()) == ctx.variables.end()) throw std::runtime_error("unregistered register");
        auto var = ctx.variables[operand.reg()];
        mod.bytecode().push_back(static_cast<furvm::byte>(furvm::instruction_t::Load));
        mod.bytecode().push_back((var >> 0) & 0xFF);
        mod.bytecode().push_back((var >> 8) & 0xFF);
        static_assert(sizeof(var) == 2, "sizeof(furvm::variable_t) has changed");
    } break;
    case furlang::ir::operand_t::Integer: {
        mod.bytecode().push_back(static_cast<furvm::byte>(furvm::instruction_t::PushB2I));
        mod.bytecode().push_back(operand.integer());
    } break;
    case furlang::ir::operand_t::Variable:
    case furlang::ir::operand_t::String: throw std::runtime_error("unimplemented operand");
    case furlang::ir::operand_t::None: throw std::runtime_error("unreachable");
    }
}

void furvm_generator::generate_jump(furvm::mod& mod,
    function_context&                           ctx,
    furlang::ir::block_index                    block,
    bool                                        conditional) {
    mod.bytecode().push_back(
        static_cast<furvm::byte>(conditional ? furvm::instruction_t::JumpNotZero : furvm::instruction_t::Jump));
    if (auto it = ctx.blockOffsets.find(block); it != ctx.blockOffsets.end()) {
        mod.bytecode().push_back(it->second - mod.bytecode().size() - 1);
    } else {
        ctx.incompleteJumps[block].push_back(mod.bytecode().size());
        mod.bytecode().push_back(0);
    }
}

} // namespace furc::back
