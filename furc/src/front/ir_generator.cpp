#include "furc/front/ir_generator.hpp"

#include "furc/ast/declaration.hpp" // IWYU pragma: keep
#include "furc/ast/expression.hpp"  // IWYU pragma: keep
#include "furc/ast/literal.hpp"     // IWYU pragma: keep
#include "furc/ast/statement.hpp"   // IWYU pragma: keep
#include "furlang/ir/function.hpp"
#include "furlang/ir/instruction.hpp"
#include "furlang/ir/operand.hpp"

#include <cassert>
#include <memory>
#include <stdexcept>
#include <vector>

namespace furc::front {

namespace {
namespace ir = furlang::ir;
}

void ir_generator::visit(const ast::function_definition_node& funcDef) {
    furlang::ir::function_access_t access = (funcDef.access() == ast::declaration_access_t::Public)
                                                ? furlang::ir::function_access_t::Public
                                                : furlang::ir::function_access_t::Private;

    m_currentFunction = std::make_unique<furlang::ir::function>(std::string(funcDef.name()), access, 0);

    push_block();
    for (const auto& stmt : funcDef.body().statements) {
        stmt.value()->accept(*this);
    }

    m_currentBlock->emplace<ir::return_instruction>();

    m_module.push(std::move(m_currentFunction));
}

void ir_generator::visit(const ast::function_declaration_node& funcDecl) {
    if (funcDecl.type() == ast::function_declaration_node_t::Normal) return;

    furlang::ir::function_t type = funcDecl.type() == ast::function_declaration_node_t::Import
                                       ? furlang::ir::function_t::Import
                                       : furlang::ir::function_t::Native;

    furlang::ir::function_access_t access = (funcDecl.access() == ast::declaration_access_t::Public)
                                                ? furlang::ir::function_access_t::Public
                                                : furlang::ir::function_access_t::Private;

    m_module.push(
        std::make_unique<furlang::ir::function>(std::string(funcDecl.name()), access, funcDecl.params().size(), type));
}

void ir_generator::visit(const ast::return_statement_node& returnStmt) {
    if (returnStmt.value().has_value()) {
        returnStmt.value().value()->accept(*this);
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
    if (node.elze().has_value()) {
        m_currentBlock->emplace<ir::branch_instruction>(m_currentFunction->blocks().size() + 1);

        push_block(); // else block
        node.elze().value()->accept(*this);
    }
    m_currentBlock->emplace<ir::branch_instruction>(m_currentFunction->blocks().size());

    push_block(); // merge block
}

void ir_generator::visit(const ast::while_statement_node& node) {
    node.condition()->accept(*this);
    ir_register                cond      = m_registerCounter - 1;
    std::shared_ptr<ir::block> entry     = m_currentBlock;
    ir::block_index            headerIdx = m_currentFunction->blocks().size();

    push_block(false); // loop header
    push<ir::branch_instruction>(m_currentFunction->blocks().size());

    push_block(); // loop condition
    node.condition()->accept(*this);
    std::shared_ptr<ir::block> condBlock = m_currentBlock;
    ir_register                cond2     = m_registerCounter - 1;

    push_block(false); // loop body
    node.body()->accept(*this);
    push<ir::branch_instruction>(headerIdx);

    entry->emplace<ir::branch_cond_instruction>(ir::operand::new_reg(cond),
        headerIdx,
        m_currentFunction->blocks().size());
    condBlock->emplace<ir::branch_cond_instruction>(ir::operand::new_reg(cond2),
        m_currentFunction->blocks().size() - 1,
        m_currentFunction->blocks().size());
    push_block(); // merge block
}

void ir_generator::visit(const ast::compound_statement_node& node) {
    for (const auto& stmt : node.body().statements) {
        stmt.value()->accept(*this);
    }
}

void ir_generator::visit(const ast::string_literal_node& node) {
    push<furlang::ir::assign_instruction>(ir::operand::new_string(node.value()),
        ir::operand::new_reg(m_registerCounter++));
}

void ir_generator::visit(const ast::integer_literal_node& node) {
    push<furlang::ir::assign_instruction>(ir::operand::new_integer(node.value()),
        ir::operand::new_reg(m_registerCounter++));
}

void ir_generator::visit(const ast::var_read_expression_node& node) {
    if (auto it = m_variables.find(node.get_name()); it != m_variables.end()) {
        push<furlang::ir::assign_instruction>(ir::operand::new_reg(it->second),
            ir::operand::new_reg(m_registerCounter++));
    } else {
        throw std::runtime_error("unknown variable");
    }
}

static inline furlang::ir::instruction_t unary_op_instruction_t(ast::unaryop_expression_node_t type) {
    switch (type) {
    case ast::unaryop_expression_node_t::Pointerof: return furlang::ir::instruction_t::Pointerof;
    case ast::unaryop_expression_node_t::Sizeof: return furlang::ir::instruction_t::Sizeof;
    default: throw std::runtime_error("unimplemented");
    }
}

void ir_generator::visit(const ast::unary_op_expression_node& node) {
    node.get_node()->accept(*this);
    ir_register src = m_registerCounter - 1;
    ir_register dst = m_registerCounter++;
    push<furlang::ir::unary_instruction>(unary_op_instruction_t(node.type()),
        ir::operand::new_reg(src),
        ir::operand::new_reg(dst));
}

static inline furlang::ir::instruction_t binary_op_instruction_t(ast::binop_expression_node_t type) {
    switch (type) {
    case ast::binop_expression_node_t::Add: return furlang::ir::instruction_t::Add;
    case ast::binop_expression_node_t::Sub: return furlang::ir::instruction_t::Sub;
    case ast::binop_expression_node_t::Mul: return furlang::ir::instruction_t::Mul;
    case ast::binop_expression_node_t::Div: return furlang::ir::instruction_t::Div;
    case ast::binop_expression_node_t::Mod: return furlang::ir::instruction_t::Mod;
    case ast::binop_expression_node_t::Equal: return furlang::ir::instruction_t::Eq;
    case ast::binop_expression_node_t::NotEqual: return furlang::ir::instruction_t::NotEq;
    case ast::binop_expression_node_t::LessThan: return furlang::ir::instruction_t::LessThan;
    case ast::binop_expression_node_t::GreaterThan: return furlang::ir::instruction_t::GreaterThan;
    case ast::binop_expression_node_t::LessEqual: return furlang::ir::instruction_t::LessEq;
    case ast::binop_expression_node_t::GreaterEqual: return furlang::ir::instruction_t::GreaterEq;
    case ast::binop_expression_node_t::None:
    default: throw std::runtime_error("unreachable");
    }
}

void ir_generator::visit(const ast::binary_op_expression_node& node) {
    node.lhs()->accept(*this);
    ir_register lhs = m_registerCounter - 1;
    node.rhs()->accept(*this);
    ir_register rhs = m_registerCounter - 1;
    ir_register dst = m_registerCounter++;
    push<furlang::ir::binary_instruction>(binary_op_instruction_t(node.type()),
        ir::operand::new_reg(lhs),
        ir::operand::new_reg(rhs),
        ir::operand::new_reg(dst));
}

void ir_generator::visit(const ast::var_assign_expression_node& node) {
    node.rhs()->accept(*this);
    ir_register rhs = m_registerCounter - 1;
    assert(node.lhs()->expression_type() == ast::expression_node_t::VarRead);
    auto lhs = std::dynamic_pointer_cast<ast::var_read_expression_node>(node.lhs());

    ir_register reg = m_registerCounter++;

    auto compound = node.compound();
    if (compound != ast::binop_expression_node_t::None) {
        push<ir::binary_instruction>(binary_op_instruction_t(compound),
            ir::operand::new_reg(reg),
            ir::operand::new_reg(rhs),
            ir::operand::new_reg(reg));
    } else {
        push<ir::assign_instruction>(ir::operand::new_reg(rhs), ir::operand::new_reg(reg));
    }

    if (auto it = m_variables.find(lhs->get_name()); it != m_variables.end()) {
        push<ir::assign_instruction>(ir::operand::new_reg(reg), ir::operand::new_reg(it->second));
    } else {
        m_variables[lhs->get_name()] = reg;
    }
}

void ir_generator::visit(const ast::function_call_expression_node& node) {
    std::vector<ir::operand> args;
    args.reserve(node.args().size());
    for (const auto& arg : node.args()) {
        arg->accept(*this);
        args.push_back(ir::operand::new_reg(m_registerCounter - 1));
    }
    if (node.func()->expression_type() != ast::expression_node_t::VarRead)
        throw std::runtime_error("invalid function call left-hand-side expression");

    push<ir::call_instruction>(dynamic_cast<const ast::var_read_expression_node&>(*node.func()).get_name(),
        ir::operand::new_reg(m_registerCounter++),
        std::move(args));
}

furlang::ir::block_index ir_generator::push_block(bool validate) {
    if (validate && !m_currentFunction->blocks().empty() && !m_currentFunction->blocks().back()->has_exit()) {
        throw std::runtime_error(
            "block " + std::to_string(m_currentFunction->blocks().size() - 1) + " is lacking an exit");
    }
    ir::block_index index = m_currentFunction->blocks().size();
    m_currentBlock        = m_currentFunction->push();
    return index;
}

} // namespace furc::front
