#ifndef FURC_AST_LITERAL_HPP
#define FURC_AST_LITERAL_HPP

#include "furc/ast/expression.hpp"
#include "furc/ast/node.hpp"
#include "furc/front/token.hpp"

namespace furc {
namespace ast {

/**
 * @brief Literal node type.
 */
enum class literal_node_t {
    String,  /**< String literal. */
    Integer, /**< Integer literal. */
};

/**
 * @brief Literal AST node.
 */
class literal_node : public expression_node {
public:
    /**
     * @brief Construct a new literal AST node.
     *
     * @param location Node location.
     */
    literal_node(struct location location)
      : expression_node(location) {}
public:
    /**
     * @brief Returns this node's category.
     *
     * @return node_t::Literal.
     */
    node_t category() const override { return node_t::Literal; }

    /**
     * @brief Returns this node's expression type.
     *
     * @return expression_node_t::Literal.
     */
    expression_node_t expression_type() const override { return expression_node_t::Literal; }

    /**
     * @brief Returns this node's literal type.
     *
     * @return The literal type.
     */
    virtual literal_node_t literal_type() const = 0;
protected:
    bool equal(const node& rhs) const override;
};

/**
 * @brief String literal AST node.
 */
class string_literal_node final : public literal_node {
public:
    using value_type = furlang::result<std::string_view, error>; /**< Value type. */
public:
    /**
     * @brief Construct a new string literal node object from a handle.
     *
     * @param location Node location.
     * @param value A string view result.
     */
    string_literal_node(struct location location, value_type&& value)
      : literal_node(location), m_value(std::move(value)) {}
public:
    /**
     * @brief Returns this node's literal type.
     *
     * @return literal_node_t::String.
     */
    literal_node_t literal_type() const override { return literal_node_t::String; }

    /**
     * @brief Returns this node's value.
     *
     * @return A string view result.
     */
    const value_type& value() const { return m_value; }
public:
    void accept(visitor& visitor) const override;

    std::ostream& print(std::ostream& os) const override;
protected:
    bool equal(const node& rhs) const override;
private:
    value_type m_value;
};

/**
 * @brief Integer literal AST node.
 */
class integer_literal_node final : public literal_node {
public:
    using value_type = furlang::result<front::integer_token, error>; /**< Value type. */
public:
    /**
     * @brief Construct a new integer literal node object from a handle.
     *
     * @param location Node location.
     * @param value An integer result.
     */
    integer_literal_node(struct location location, value_type&& value)
      : literal_node(location), m_value(std::move(value)) {}
public:
    /**
     * @brief Returns this node's literal type.
     *
     * @return literal_node_t::Integer.
     */
    literal_node_t literal_type() const override { return literal_node_t::Integer; }

    /**
     * @brief Returns this node's value.
     *
     * @return An integer result.
     */
    const value_type& value() const { return m_value; }

    /**
     * @brief Compares this node with an integer token for equality.
     *
     * @param integer Integer token to compare against.
     * @return true if the integer token is equal to this node.
     */
    bool operator==(front::integer_token integer) const { return m_value == integer; }
public:
    void accept(visitor& visitor) const override;

    std::ostream& print(std::ostream& os) const override;
protected:
    bool equal(const node& rhs) const override;
private:
    value_type m_value;
};

} // namespace ast
} // namespace furc

#endif // FURC_AST_LITERAL_HPP