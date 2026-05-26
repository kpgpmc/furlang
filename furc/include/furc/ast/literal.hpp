#ifndef FURC_AST_LITERAL_HPP
#define FURC_AST_LITERAL_HPP

#include "furc/ast/expression.hpp"
#include "furc/ast/node.hpp"
#include "furc/front/token.hpp"

namespace furc {
namespace ast {

enum class literal_node_t {
    String,
    Integer
};

class literal_node : public expression_node {
public:
    node_t category() const override { return node_t::Literal; }

    expression_node_t expression_type() const override { return expression_node_t::Literal; }

    virtual literal_node_t literal_type() const = 0;
};

class string_literal_node : public literal_node {
public:
    string_literal_node(handle<std::string_view>&& value)
      : m_value(std::move(value)) {}
public:
    literal_node_t literal_type() const override { return literal_node_t::String; }

    const handle<std::string_view>& value() const { return m_value; }
public:
    std::ostream& print(std::ostream& os) const override {
        if (m_value.has_error()) return os << (std::string)m_value;
        return os << "string literal (" << *m_value << ")";
    }
private:
    handle<std::string_view> m_value;
};

class integer_literal_node : public literal_node {
public:
    integer_literal_node(handle<front::integer_token>&& value)
      : m_value(std::move(value)) {}
public:
    literal_node_t literal_type() const override { return literal_node_t::Integer; }

    const handle<front::integer_token>& value() const { return m_value; }
public:
    std::ostream& print(std::ostream& os) const override;
private:
    handle<front::integer_token> m_value;
};

} // namespace ast
} // namespace furc

#endif // FURC_AST_LITERAL_HPP