#ifndef FURC_AST_STATEMENT_HPP
#define FURC_AST_STATEMENT_HPP

#include "furc/ast/node.hpp"

namespace furc {
namespace ast {

enum class statement_node_t {
    Expression,
    Declaration,
    Return,
};

class statement_node : public node {
public:
    node_t category() const override { return node_t::Statement; }

    virtual statement_node_t statement_type() const = 0;
protected:
    bool equal(const node& rhs) const override;
};

class return_statement_node : public statement_node {
public:
    return_statement_node() = default;

    return_statement_node(node_handle<expression_node>&& value)
      : m_value(std::move(value)) {}
public:
    node_handle<expression_node> value() const { return m_value; }
public:
    statement_node_t statement_type() const override { return statement_node_t::Return; }
public:
    void accept(visitor& visitor) const override;

    std::ostream& print(std::ostream& os) const override;
protected:
    bool equal(const node& rhs) const override;
private:
    node_handle<expression_node> m_value;
};

} // namespace ast
} // namespace furc

#endif // FURC_AST_STATEMENT_HPP