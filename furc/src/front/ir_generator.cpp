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

void ir_generator::visit(const ast::function_definition_node& funcDef) {
    m_currentFunction = std::make_unique<furlang::ir::function>(std::string(funcDef.name()->string));

    push_block();
    if (funcDef.body().has_error()) {
        std::cerr << funcDef.body().error() << '\n';
        return;
    }
    for (const auto& stmt : funcDef.body()->statements) {
        stmt->accept(*this);
    }
    m_currentBlock->emplace<ir::return_instruction>();

    m_module.push(std::move(m_currentFunction));
}

void ir_generator::visit(const ast::return_statement_node& returnStmt) {
    if (returnStmt.value().has_error()) {
        std::cerr << returnStmt.value() << '\n';
    }

    if (returnStmt.value().present()) {
        returnStmt.value()->accept(*this);
        push<ir::return_instruction>(ir::operand::new_reg(m_registerCounter - 1));
    } else {
        push<ir::return_instruction>();
    }
}

void ir_generator::visit(const ast::if_statement_node& node) {
    node.cond()->accept(*this);
    ir_register cond = m_registerCounter - 1;
    push<ir::branch_cond_instruction>(ir::operand::new_reg(cond),
        m_currentFunction->blocks().size(),
        m_currentFunction->blocks().size() + 1);

    push_block(); // then block
    node.then()->accept(*this);
    if (node.elze().present()) {
        m_currentBlock->emplace<ir::branch_instruction>(m_currentFunction->blocks().size() + 1);

        push_block(); // else block
        node.elze()->accept(*this);
    }
    m_currentBlock->emplace<ir::branch_instruction>(m_currentFunction->blocks().size());

    push_block(); // merge block
}

void ir_generator::visit(const ast::compound_statement_node& node) {
    for (const auto& stmt : node.body()->statements) {
        stmt->accept(*this);
    }
}

void ir_generator::visit(const ast::string_literal_node& node) {
    push<furlang::ir::assign_instruction>(ir::operand::new_string(std::string(*node.value())),
        ir::operand::new_reg(m_registerCounter++));
}

void ir_generator::visit(const ast::integer_literal_node& node) {
    push<furlang::ir::assign_instruction>(ir::operand::new_integer(*node.value()),
        ir::operand::new_reg(m_registerCounter++));
}

void ir_generator::visit(const ast::var_read_expression_node& node) {
    if (auto it = m_variables.find(*node.get_name()); it != m_variables.end()) {
        push<furlang::ir::assign_instruction>(ir::operand::new_reg(it->second),
            ir::operand::new_reg(m_registerCounter++));
    } else {
        throw std::runtime_error("unknown variable");
    }
}

void ir_generator::visit(const ast::unaryop_expression_node& node) {
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

void ir_generator::visit(const ast::binop_expression_node& node) {
    node.lhs()->accept(*this);
    ir_register lhs = m_registerCounter - 1;
    node.rhs()->accept(*this);
    ir_register rhs = m_registerCounter - 1;
    ir_register dst = m_registerCounter++;
    push<furlang::ir::binary_op_instruction>(binary_op_instruction_t(node.type()),
        ir::operand::new_reg(lhs),
        ir::operand::new_reg(rhs),
        ir::operand::new_reg(dst));
}

void ir_generator::visit(const ast::var_assign_expression_node& node) {
    node.rhs()->accept(*this);
    ir_register rhs = m_registerCounter - 1;
    assert(node.lhs()->expression_type() == ast::expression_node_t::VarRead);
    ast::var_read_expression_node_h lhs = node.lhs();

    ir_register reg = 0;
    if (auto it = m_variables.find(*lhs->get_name()); it != m_variables.end()) {
        reg = it->second;
    } else {
        m_variables[*lhs->get_name()] = reg = m_registerCounter++;
    }

    auto compound = node.compound();
    if (compound != ast::binop_expression_node_t::None) {
        push<ir::binary_op_instruction>(binary_op_instruction_t(compound),
            ir::operand::new_reg(reg),
            ir::operand::new_reg(rhs),
            ir::operand::new_reg(reg));
    } else {
        push<ir::assign_instruction>(ir::operand::new_reg(rhs), ir::operand::new_reg(reg));
    }
}

furlang::ir::block_index ir_generator::push_block() {
    if (!m_currentFunction->blocks().empty() && !m_currentFunction->blocks().back()->has_exit()) {
        throw std::runtime_error(
            "block " + std::to_string(m_currentFunction->blocks().size() - 1) + " is lacking an exit");
    }
    ir::block_index index = m_currentFunction->blocks().size();
    m_currentBlock        = m_currentFunction->push();
    return index;
}

} // namespace furc::front