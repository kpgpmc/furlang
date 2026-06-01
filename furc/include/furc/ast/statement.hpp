#ifndef FURC_AST_STATEMENT_HPP
#define FURC_AST_STATEMENT_HPP

#include "furc/ast/node.hpp"

namespace furc {
namespace ast {

enum class statement_node_t {
    Expression,
    Declaration,
    Return,
    If,
    Compound,
};

class statement_node : public node {
public:
    node_t category() const override { return node_t::Statement; }

    virtual statement_node_t statement_type() const = 0;
protected:
    bool equal(const node& rhs) const override;
};

class return_statement_node final : public statement_node {
public:
    return_statement_node() = default;

    return_statement_node(expression_node_h&& value)
      : m_value(std::move(value)) {}
public:
    expression_node_h value() const { return m_value; }
public:
    statement_node_t statement_type() const override { return statement_node_t::Return; }
public:
    void accept(visitor& visitor) const override;

    std::ostream& print(std::ostream& os) const override;
protected:
    bool equal(const node& rhs) const override;
private:
    expression_node_h m_value;
};

class if_statement_node final : public statement_node {
public:
    if_statement_node(expression_node_h&& cond, statement_node_h&& then)
      : m_cond(std::move(cond)), m_then(std::move(then)) {}

    if_statement_node(expression_node_h&& cond, statement_node_h&& then, statement_node_h&& elze)
      : m_cond(std::move(cond)), m_then(std::move(then)), m_else(std::move(elze)) {}
public:
    expression_node_h       cond() const { return m_cond; }
    const statement_node_h& then() const { return m_then; }
    const statement_node_h& elze() const { return m_else; }
public:
    statement_node_t statement_type() const override { return statement_node_t::If; }
public:
    void accept(visitor& visitor) const override;

    std::ostream& print(std::ostream& os) const override;
protected:
    bool equal(const node& rhs) const override;
private:
    expression_node_h m_cond;
    statement_node_h  m_then;
    statement_node_h  m_else;
};

class compound_statement_node final : public statement_node {
public:
    compound_statement_node(body_h&& body)
      : m_body(std::move(body)) {}
public:
    const body_h& body() const { return m_body; }
public:
    statement_node_t statement_type() const override { return statement_node_t::Compound; }
public:
    void accept(visitor& visitor) const override;

    std::ostream& print(std::ostream& os) const override;
protected:
    bool equal(const node& rhs) const override;
private:
    body_h m_body;
};

} // namespace ast
} // namespace furc

#endif // FURC_AST_STATEMENT_HPP