#ifndef FURC_AST_VISITOR_HPP
#define FURC_AST_VISITOR_HPP

#include "furc/ast/fwd.hpp"

namespace furc {
namespace ast {

class visitor {
public:
    visitor()          = default;
    virtual ~visitor() = default;

    visitor(visitor&&)                 = default;
    visitor& operator=(visitor&&)      = default;
    visitor(const visitor&)            = default;
    visitor& operator=(const visitor&) = default;
public:
    virtual void visit(const string_literal_node&) {}
    virtual void visit(const integer_literal_node&) {}
    virtual void visit(const var_read_expression_node&) {}
    virtual void visit(const unaryop_expression_node&) {}
    virtual void visit(const binop_expression_node&) {}
    virtual void visit(const var_assign_expression_node&) {}
    virtual void visit(const function_declaration_node&) {}
    virtual void visit(const function_definition_node&) {}
    virtual void visit(const return_statement_node&) {}
    virtual void visit(const if_statement_node&) {}
    virtual void visit(const compound_statement_node&) {}

    virtual void visit_error(const node_handle<node>& handle) {}
};

} // namespace ast
} // namespace furc

#endif // FURC_AST_VISITOR_HPP