// NOLINTBEGIN(portability-template-virtual-member-function)

#ifndef FURC_AST_LITERAL_HPP
#define FURC_AST_LITERAL_HPP

#include "furc/ast/expression.hpp"
#include "furc/ast/node.hpp"

namespace furc {
namespace ast {

/**
 * @brief Literal AST node.
 */
template <typename ValueType, literal_node_t LiteralType>
class literal_node : public expression_node {
public:
    using value_type = std::remove_reference_t<ValueType>; /**< Value type. */
public:
    /**
     * @brief Construct a new literal AST node.
     *
     * @param location Node location.
     */
    template <typename = std::enable_if_t<std::is_default_constructible_v<ValueType>>>
    literal_node(struct location location)
      : expression_node(location) {}

    /**
     * @brief Construct a new literal AST node.
     *
     * @param location Node location.
     * @param value Node value to copy.
     */
    literal_node(struct location location, const value_type& value)
      : expression_node(location), p_value(value) {}

    /**
     * @brief Construct a new literal AST node.
     *
     * @param location Node location.
     * @param value Node value to move.
     */
    literal_node(struct location location, value_type&& value)
      : expression_node(location), p_value(std::move(value)) {}

    /**
     * @brief Construct a new literal AST node.
     *
     * @param location Node location.
     * @param args Arguments to call value constructor with.
     */
    template <typename... Args, typename = std::enable_if_t<std::is_constructible_v<ValueType, Args...>>>
    literal_node(struct location location, Args&&... args)
      : expression_node(location), p_value(std::forward<Args>(args)...) {}
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
    literal_node_t literal_type() const { return LiteralType; }

    /**
     * @brief Returns this node's value.
     *
     * @return A string view result.
     */
    const value_type& value() const { return p_value; }
public:
    void accept(visitor& visitor) const override { visitor.visit(*this); }

    std::ostream& print(std::ostream& os) const override { return os << p_value; }
protected:
    bool equal(const node& rhsNode) const override {
        const auto& rhs = dynamic_cast<const literal_node&>(rhsNode);
        return literal_type() == rhs.literal_type() && p_value == rhs.p_value;
    }
protected:
    value_type p_value; /**< Node value. */
};

} // namespace ast
} // namespace furc

#endif // FURC_AST_LITERAL_HPP

// NOLINTEND(portability-template-virtual-member-function)