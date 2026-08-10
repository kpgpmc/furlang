#include "furc/middle/ir.hpp"

#include "furc/front/ast.hpp"

#include <stdexcept>
#include <utility>

namespace furc {

static ir_type ast_type_to_ir(const ast_type& type) {
    switch (type.type) {
    case ast_type::Void: return { ir_type::Void };
    case ast_type::S8: return { ir_type::S8 };
    case ast_type::U8: return { ir_type::U8 };
    case ast_type::S16: return { ir_type::S16 };
    case ast_type::U16: return { ir_type::U16 };
    case ast_type::S32: return { ir_type::S32 };
    case ast_type::U32: return { ir_type::U32 };
    case ast_type::S64: return { ir_type::S64 };
    case ast_type::U64: return { ir_type::U64 };
    }
    throw std::runtime_error("unreachable");
}

void ir_generator::visit_comp_stmt_node(const comp_stmt_node& node) {
    // TODO: Introduce scopes for statements to naturally allow variable shadowing.
    for (const auto& stmt : node.stmts) {
        stmt->accept(*this);
    }
}

void ir_generator::visit_if_stmt_node(const if_stmt_node& node) {
    node.cond->accept(*this);
    auto* branch = context().terminate(context().last_register(), context().blockIdx + 1, 0);

    context().new_next();
    node.thenBranch->accept(*this);
    context().new_next();
    branch->destination->value.blockPair.second = context().blockIdx; // NOLINT
    if (node.elseBranch != nullptr) {
        node.elseBranch->accept(*this);
        context().new_next();
    }
}

void ir_generator::visit_while_stmt_node(const while_stmt_node& node) {
    context().new_next();
    auto header = context().blockIdx;
    node.cond->accept(*this);
    auto* branch = context().terminate(context().last_register(), context().blockIdx + 1, 0);

    context().new_next();
    auto body = context().blockIdx;
    node.body->accept(*this);
    if (!context().blockPtr->is_terminated()) context().terminate(header);

    context().new_next();
    branch->destination->value.blockPair.second = context().blockIdx; // NOLINT
}

void ir_generator::visit_return_stmt_node(const return_stmt_node& node) {
    if (node.value == nullptr) {
        context().terminate();
    } else {
        node.value->accept(*this);
        context().terminate(context().last_register());
    }
}

void ir_generator::visit_var_decl_node(const var_decl_node& node) {
    const auto* var = m_scope->allocate(m_module.arena, node.name, ast_type_to_ir(node.type));

    if (node.init != nullptr) {
        if (m_context.empty()) {
            m_initContext.new_next();
            m_context.push(std::move(m_initContext));
            node.init->accept(*this);
            context().add_instr(ir_instruction{ ir_instruction::Move, var->operand(), { context().last_register() } });
            m_initContext = std::move(m_context.top());
            m_context.pop();
        } else {
            node.init->accept(*this);
            context().add_instr(ir_instruction{ ir_instruction::Move, var->operand(), { context().last_register() } });
        }
    }
}

void ir_generator::visit_func_decl_node(const func_decl_node& node) {
    ir_function function;
    function.previous = m_scope;
    function.name     = node.name;
    for (const auto& param : node.params) {
        function.params.push_back(ast_type_to_ir(param.type));
        function.allocate(m_module.arena, param.name, ast_type_to_ir(param.type));
    }
    function.retType = ast_type_to_ir(node.type);

    if (node.def.has_value()) {
        auto* func = m_module.functions.emplace_back(m_module.arena.allocate<ir_function>(std::move(function)));
        m_context.emplace(func);
        m_scope = func;
        node.def->body.accept(*this);
        m_scope = m_scope->previous;
        m_context.pop();
    }
}

void ir_generator::visit_var_read_expr_node(const var_read_expr_node& node) {
    const auto* var = m_scope->variable(node.name);
    if (var == nullptr) throw std::runtime_error("unknown variable");
    context().add_instr(ir_instruction{ ir_instruction::Move, context().next_register(), { var->operand() } });
}

void ir_generator::visit_func_call_expr_node(const func_call_expr_node& node) {
    throw std::runtime_error("not implemented");
}

void ir_generator::visit_group_expr_node(const group_expr_node& node) {
    node.inner->accept(*this);
}

void ir_generator::visit_binary_op_expr_node(const binary_op_expr_node& node) {
    node.lhs->accept(*this);
    auto lhs = context().last_register();
    node.rhs->accept(*this);
    auto rhs = context().last_register();

    switch (node.type) {
    case binary_op_expr_node::Add:
        context().add_instr(ir_instruction{ ir_instruction::Add, context().next_register(), { lhs, rhs } });
        return;
    case binary_op_expr_node::Sub:
        context().add_instr(ir_instruction{ ir_instruction::Sub, context().next_register(), { lhs, rhs } });
        return;
    case binary_op_expr_node::Mul:
        context().add_instr(ir_instruction{ ir_instruction::Mul, context().next_register(), { lhs, rhs } });
        return;
    case binary_op_expr_node::Div:
        context().add_instr(ir_instruction{ ir_instruction::Div, context().next_register(), { lhs, rhs } });
        return;
    case binary_op_expr_node::Mod:
        context().add_instr(ir_instruction{ ir_instruction::Mod, context().next_register(), { lhs, rhs } });
        return;
    case binary_op_expr_node::Shl:
        context().add_instr(ir_instruction{ ir_instruction::Shl, context().next_register(), { lhs, rhs } });
        return;
    case binary_op_expr_node::Shr:
        context().add_instr(ir_instruction{ ir_instruction::Shr, context().next_register(), { lhs, rhs } });
        return;
    case binary_op_expr_node::BinAnd:
        context().add_instr(ir_instruction{ ir_instruction::BinAnd, context().next_register(), { lhs, rhs } });
        return;
    case binary_op_expr_node::BinOr:
        context().add_instr(ir_instruction{ ir_instruction::BinOr, context().next_register(), { lhs, rhs } });
        return;
    case binary_op_expr_node::BinXor:
        context().add_instr(ir_instruction{ ir_instruction::BinXor, context().next_register(), { lhs, rhs } });
        return;
    case binary_op_expr_node::And:
        context().add_instr(ir_instruction{ ir_instruction::And, context().next_register(), { lhs, rhs } });
        return;
    case binary_op_expr_node::Or:
        context().add_instr(ir_instruction{ ir_instruction::Or, context().next_register(), { lhs, rhs } });
        return;
    case binary_op_expr_node::Equals:
        context().add_instr(ir_instruction{ ir_instruction::Eq, context().next_register(), { lhs, rhs } });
        return;
    case binary_op_expr_node::NotEquals:
        context().add_instr(ir_instruction{ ir_instruction::NotEq, context().next_register(), { lhs, rhs } });
        return;
    case binary_op_expr_node::LessThan:
        context().add_instr(ir_instruction{ ir_instruction::LessThan, context().next_register(), { lhs, rhs } });
        return;
    case binary_op_expr_node::LessEquals:
        context().add_instr(ir_instruction{ ir_instruction::LessEq, context().next_register(), { lhs, rhs } });
        return;
    case binary_op_expr_node::GreaterThan:
        context().add_instr(ir_instruction{ ir_instruction::GreaterThan, context().next_register(), { lhs, rhs } });
        return;
    case binary_op_expr_node::GreaterEquals:
        context().add_instr(ir_instruction{ ir_instruction::GreaterEq, context().next_register(), { lhs, rhs } });
        return;
    }
    throw std::runtime_error("unreachable");
}

void ir_generator::visit_unary_op_expr_node(const unary_op_expr_node& node) {
    if (node.type == unary_op_expr_node::PostInc || node.type == unary_op_expr_node::PostDec) {
        node.lhs->accept(*this);
        auto lhs = context().last_register();
        node.lhs->accept(*this);
        context().add_instr(node.type == unary_op_expr_node::PostInc ? ir_instruction::Increment
                                                                     : ir_instruction::Decrement,
            context().last_register());
        return;
    }

    node.lhs->accept(*this);
    auto lhs = context().last_register();

    switch (node.type) {
    case unary_op_expr_node::Positive: context().add_instr(ir_instruction::Positive, lhs);
    case unary_op_expr_node::Negative: context().add_instr(ir_instruction::Negative, lhs);
    case unary_op_expr_node::PreInc: context().add_instr(ir_instruction::Increment, lhs);
    case unary_op_expr_node::PreDec: context().add_instr(ir_instruction::Decrement, lhs);
    case unary_op_expr_node::BinNot: context().add_instr(ir_instruction::BinNot, lhs);
    case unary_op_expr_node::Not: context().add_instr(ir_instruction::Not, lhs);
    case unary_op_expr_node::Sizeof: context().add_instr(ir_instruction::Sizeof, lhs);
    case unary_op_expr_node::Pointerof: context().add_instr(ir_instruction::Pointerof, lhs);
    case unary_op_expr_node::Lengthof: context().add_instr(ir_instruction::Lenof, lhs);
    case unary_op_expr_node::PostInc:
    case unary_op_expr_node::PostDec: return;
    }
    throw std::runtime_error("unreachable");
}

void ir_generator::visit_if_expr_node(const if_expr_node& node) {
    node.cond->accept(*this);
    auto* branch = context().terminate(context().last_register(), context().blockIdx + 1, 0);

    context().new_next();
    auto thenBranch = context().blockIdx;
    node.thenExpr->accept(*this);
    auto thenReg = context().last_register();

    context().new_next();
    branch->destination->value.blockPair.second = context().blockIdx; // NOLINT
    node.elseExpr->accept(*this);
    auto elseReg = context().last_register();
    auto resReg  = context().next_register();
    context().add_instr(ir_instruction{ ir_instruction::Move, resReg, { elseReg } });

    context().new_next();
    auto epilogue = context().blockIdx;

    context().go(thenBranch);
    context().add_instr(ir_instruction{ ir_instruction::Move, resReg, { thenReg } });
    context().terminate(epilogue);
    context().go(epilogue);
}

void ir_generator::visit_int_lit_node(const int_lit_node& node) {
    context().add_instr(ir_instruction{ ir_instruction::Move,
        context().next_register(),
        { ir_operand{ ir_operand::Integer, node.value } } });
}

void ir_generator::visit_char_lit_node(const char_lit_node& node) {
    context().add_instr(ir_instruction{ ir_instruction::Move,
        context().next_register(),
        { ir_operand{ ir_operand::Integer, static_cast<std::uint64_t>(node.value) } } });
}

} // namespace furc
