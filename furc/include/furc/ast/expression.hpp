#ifndef FURC_AST_EXPRESSION_HPP
#define FURC_AST_EXPRESSION_HPP

#include "furc/ast/node.hpp"
#include "furc/ast/statement.hpp"

namespace furc {
namespace ast {

enum class expression_node_t {
    Literal,
    VarRead,
    VarAssign,
    Unaryop,
    Binop,
    Paren,
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

class var_read_expression_node : public expression_node {
public:
    var_read_expression_node(handle<std::string_view>&& name)
      : m_name(std::move(name)) {}

    const handle<std::string_view>& get_name() const { return m_name; }
    handle<std::string_view>&&      move_name() { return std::move(m_name); }
public:
    expression_node_t expression_type() const override { return expression_node_t::VarRead; }

    std::ostream& print(std::ostream& os) const override;
protected:
    bool equal(const node& rhs) const override;
private:
    handle<std::string_view> m_name;
};

using var_read_expression_node_h = node_handle<var_read_expression_node>;

enum class unaryop_expression_node_t {
    Positive,
    Negative,
    PrefixIncrement,
    PostfixIncrement,
    PrefixDecrement,
    PostfixDecrement,
};

class unaryop_expression_node : public expression_node {
public:
    unaryop_expression_node(unaryop_expression_node_t type, expression_node_h&& node)
      : m_type(type), m_node(std::move(node)) {}

    void set_node(expression_node_h&& node) { m_node = std::move(node); }

    unaryop_expression_node_t type() const { return m_type; }
    const expression_node_h&  get_node() const { return m_node; }
    expression_node_h&        get_node() { return m_node; }
    expression_node_h&&       move_node() { return std::move(m_node); }
public:
    expression_node_t expression_type() const override { return expression_node_t::Unaryop; }

    std::ostream& print(std::ostream& os) const override;
protected:
    bool equal(const node& rhs) const override;
private:
    unaryop_expression_node_t m_type;
    expression_node_h         m_node;
};

using unaryop_expression_node_h = node_handle<unaryop_expression_node>;

enum class binop_expression_node_t {
    None = 0,
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
    expression_node_h&       lhs() { return m_lhs; };
    expression_node_h&&      move_lhs() { return std::move(m_lhs); };
    const expression_node_h& rhs() const { return m_rhs; };
    expression_node_h&       rhs() { return m_rhs; };
    expression_node_h&&      move_rhs() { return std::move(m_rhs); };
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

class var_assign_expression_node : public expression_node {
public:
    var_assign_expression_node(expression_node_h&& lhs, expression_node_h&& rhs)
      : m_compound(binop_expression_node_t::None), m_lhs(std::move(lhs)), m_rhs(std::move(rhs)) {}

    var_assign_expression_node(binop_expression_node_t compound, expression_node_h&& lhs, expression_node_h&& rhs)
      : m_compound(compound), m_lhs(std::move(lhs)), m_rhs(std::move(rhs)) {}

    binop_expression_node_t  compound() const { return m_compound; }
    const expression_node_h& lhs() const { return m_lhs; }
    const expression_node_h& rhs() const { return m_rhs; }
public:
    expression_node_t expression_type() const override { return expression_node_t::VarAssign; }

    std::ostream& print(std::ostream& os) const override;
protected:
    bool equal(const node& rhs) const override;
private:
    binop_expression_node_t m_compound;
    expression_node_h       m_lhs;
    expression_node_h       m_rhs;
};

using var_assign_expression_node_h = node_handle<var_assign_expression_node>;

} // namespace ast
} // namespace furc

#endif // FURC_AST_EXPRESSION_HPP