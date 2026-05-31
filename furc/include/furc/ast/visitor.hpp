#ifndef FURC_AST_VISITOR_HPP
#define FURC_AST_VISITOR_HPP

#include "furc/ast/fwd.hpp"

namespace furc {
namespace ast {

class visitor {
public:
    virtual ~visitor() = default;

    visitor(visitor&&)                 = default;
    visitor& operator=(visitor&&)      = default;
    visitor(const visitor&)            = default;
    visitor& operator=(const visitor&) = default;
public:
    virtual void visit_string_literal_node(const string_literal_node&) {}
    virtual void visit_integer_literal_node(const integer_literal_node&) {}
    virtual void visit_var_read_expression_node(const var_read_expression_node&) {}
    virtual void visit_unaryop_expression_node(const unaryop_expression_node&) {}
    virtual void visit_binop_expression_node(const binop_expression_node&) {}
    virtual void visit_var_assign_expression_node(const var_assign_expression_node&) {}
    virtual void visit_function_declaration_node(const function_declaration_node&) {}
    virtual void visit_function_definition_node(const function_definition_node&) {}
    virtual void visit_return_statement_node(const return_statement_node&) {}

    virtual void visit_error(const node_handle<node>& handle) {}
};

} // namespace ast
} // namespace furc

#endif // FURC_AST_VISITOR_HPP