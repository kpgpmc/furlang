#ifndef FURC_AST_VISITOR_HPP
#define FURC_AST_VISITOR_HPP

#include "furc/ast/fwd.hpp"

namespace furc {
namespace ast {

/**
 * @brief Visitor pattern class for AST nodes.
 */
class visitor {
public:
    visitor()          = default;
    virtual ~visitor() = default;

    /**
     * @brief Move constructor.
     */
    visitor(visitor&&) = default;

    /**
     * @brief Move constructor.
     */
    visitor& operator=(visitor&&) = default;

    /**
     * @brief Copy constructor.
     */
    visitor(const visitor&) = default;

    /**
     * @brief Copy constructor.
     */
    visitor& operator=(const visitor&) = default;
public:
    /**
     * @brief Visit a string_literal_node.
     * @see string_literal_node
     *
     * @param node Node.
     */
    virtual void visit(const string_literal_node& node) {}

    /**
     * @brief Visit a integer_literal_node.
     * @see integer_literal_node
     *
     * @param node Node.
     */
    virtual void visit(const integer_literal_node& node) {}

    /**
     * @brief Visit a var_read_expression_node.
     * @see var_read_expression_node
     *
     * @param node Node.
     */
    virtual void visit(const var_read_expression_node& node) {}

    /**
     * @brief Visit a unaryop_expression_node.
     * @see unaryop_expression_node
     *
     * @param node Node.
     */
    virtual void visit(const unary_op_expression_node& node) {}

    /**
     * @brief Visit a binop_expression_node.
     * @see binop_expression_node
     *
     * @param node Node.
     */
    virtual void visit(const binary_op_expression_node& node) {}

    /**
     * @brief Visit a var_assign_expression_node.
     * @see var_assign_expression_node
     *
     * @param node Node.
     */
    virtual void visit(const var_assign_expression_node& node) {}

    /**
     * @brief Visit a function_declaration_node.
     * @see function_declaration_node
     *
     * @param node Node.
     */
    virtual void visit(const function_declaration_node& node) {}

    /**
     * @brief Visit a function_definition_node.
     * @see function_definition_node
     *
     * @param node Node.
     */
    virtual void visit(const function_definition_node& node) {}

    /**
     * @brief Visit a return_statement_node.
     * @see return_statement_node
     *
     * @param node Node.
     */
    virtual void visit(const return_statement_node& node) {}

    /**
     * @brief Visit a if_statement_node.
     * @see if_statement_node
     *
     * @param node Node.
     */
    virtual void visit(const if_statement_node& node) {}

    /**
     * @brief Visit a compound_statement_node.
     * @see compound_statement_node
     *
     * @param node Node.
     */
    virtual void visit(const compound_statement_node& node) {}

    /**
     * @brief Visit a while_statement_node.
     * @see while_statement_node
     *
     * @param node Node.
     */
    virtual void visit(const while_statement_node& node) {}

    /**
     * @brief Visit an AST error.
     *
     * @param error AST error.
     */
    virtual void visit_error(const ast::error& error) {}
};

} // namespace ast
} // namespace furc

#endif // FURC_AST_VISITOR_HPP
