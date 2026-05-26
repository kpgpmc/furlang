#ifndef FURC_AST_EXPRESSION_HPP
#define FURC_AST_EXPRESSION_HPP

#include "furc/ast/node.hpp"
#include "furc/ast/statement.hpp"

namespace furc {
namespace ast {

enum class expression_node_t {
    Literal,
    Binop
};

class expression_node : public statement_node {
public:
    node_t category() const override { return node_t::Expression; }

    statement_node_t statement_type() const override { return statement_node_t::Expression; }

    virtual expression_node_t expression_type() const = 0;
protected:
    bool equal(const node& rhs) const override;
};

using expression_node_h = node_handle<expression_node>;

enum class binop_expression_node_t {
    Add,
    Sub,
    Mul,
    Div,
    Mod,
};

class binop_expression_node : public expression_node {
public:
    binop_expression_node(binop_expression_node_t type, expression_node_h&& lhs, expression_node_h&& rhs)
      : m_type(type), m_lhs(std::move(lhs)), m_rhs(std::move(rhs)) {}

    binop_expression_node_t  type() const { return m_type; };
    const expression_node_h& lhs() const { return m_lhs; };
    const expression_node_h& rhs() const { return m_rhs; };
public:
    expression_node_t expression_type() const override { return expression_node_t::Binop; }

    std::ostream& print(std::ostream& os) const override;
protected:
    bool equal(const node& rhs) const override;
private:
    binop_expression_node_t m_type;
    expression_node_h       m_lhs;
    expression_node_h       m_rhs;
};

using binop_expression_node_h = node_handle<binop_expression_node>;

} // namespace ast
} // namespace furc

#endif // FURC_AST_EXPRESSION_HPP