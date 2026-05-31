#include "furc/front/ir_generator.hpp"

#include "furc/ast/declaration.hpp" // IWYU pragma: keep
#include "furc/ast/expression.hpp"  // IWYU pragma: keep
#include "furc/ast/literal.hpp"     // IWYU pragma: keep
#include "furc/ast/statement.hpp"   // IWYU pragma: keep

#include <iostream>

namespace furc::front {

namespace {
namespace ir = furlang::ir;
}

void ir_generator::visit_function_definition_node(const ast::function_definition_node& funcDef) {
    auto func = std::make_unique<furlang::ir::function>(std::string(funcDef.name()->string));

    m_currentBlock = func->push();
    if (funcDef.body().has_error()) {
        std::cerr << funcDef.body().error() << '\n';
        return;
    }
    for (const auto& stmt : funcDef.body()->statements) {
        stmt->accept(*this);
    }

    m_module.push(std::move(func));
}

void ir_generator::visit_return_statement_node(const ast::return_statement_node& returnStmt) {
    if (returnStmt.value().has_error()) {
        std::cerr << returnStmt.value() << '\n';
    }

    if (returnStmt.value().present()) {
        returnStmt.value()->accept(*this);
        m_currentBlock->emplace<ir::return_instruction>(ir::operand::new_reg(m_registerCounter - 1));
    } else {
        m_currentBlock->emplace<ir::return_instruction>();
    }
}

void ir_generator::visit_string_literal_node(const ast::string_literal_node& node) {
    m_currentBlock->emplace<furlang::ir::assign_instruction>(ir::operand::new_string(std::string(*node.value())),
        ir::operand::new_reg(m_registerCounter++));
}

void ir_generator::visit_integer_literal_node(const ast::integer_literal_node& node) {
    m_currentBlock->emplace<furlang::ir::assign_instruction>(ir::operand::new_integer(*node.value()),
        ir::operand::new_reg(m_registerCounter++));
}

void ir_generator::visit_var_read_expression_node(const ast::var_read_expression_node& node) {
    throw std::runtime_error("unimplemented");
}

void ir_generator::visit_unaryop_expression_node(const ast::unaryop_expression_node& node) {
    throw std::runtime_error("unimplemented");
}

static inline furlang::ir::binary_op_instruction_t binary_op_instruction_t(ast::binop_expression_node_t type) {
    switch (type) {
    case ast::binop_expression_node_t::Add: return furlang::ir::binary_op_instruction_t::Add;
    case ast::binop_expression_node_t::Sub: return furlang::ir::binary_op_instruction_t::Sub;
    case ast::binop_expression_node_t::Mul: return furlang::ir::binary_op_instruction_t::Mul;
    case ast::binop_expression_node_t::Div: return furlang::ir::binary_op_instruction_t::Div;
    case ast::binop_expression_node_t::Mod: return furlang::ir::binary_op_instruction_t::Mod;
    case ast::binop_expression_node_t::Equal: return furlang::ir::binary_op_instruction_t::Eq;
    case ast::binop_expression_node_t::NotEqual: return furlang::ir::binary_op_instruction_t::NotEq;
    case ast::binop_expression_node_t::LessThan: return furlang::ir::binary_op_instruction_t::LessThan;
    case ast::binop_expression_node_t::GreaterThan: return furlang::ir::binary_op_instruction_t::GreaterThan;
    case ast::binop_expression_node_t::LessEqual: return furlang::ir::binary_op_instruction_t::LessEq;
    case ast::binop_expression_node_t::GreaterEqual: return furlang::ir::binary_op_instruction_t::GreaterEq;
    case ast::binop_expression_node_t::None:
    default: throw std::runtime_error("unreachable");
    }
}

void ir_generator::visit_binop_expression_node(const ast::binop_expression_node& node) {
    node.lhs()->accept(*this);
    std::uint32_t lhs = m_registerCounter - 1;
    node.rhs()->accept(*this);
    std::uint32_t rhs = m_registerCounter - 1;
    std::uint32_t dst = m_registerCounter++;
    m_currentBlock->emplace<furlang::ir::binary_op_instruction>(binary_op_instruction_t(node.type()),
        ir::operand::new_reg(lhs),
        ir::operand::new_reg(rhs),
        ir::operand::new_reg(dst));
}

void ir_generator::visit_var_assign_expression_node(const ast::var_assign_expression_node& node) {
    throw std::runtime_error("unimplemented");
}

} // namespace furc::front