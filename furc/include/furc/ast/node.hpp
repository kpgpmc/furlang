#ifndef FURC_AST_NODE_HPP
#define FURC_AST_NODE_HPP

#include "furc/ast/fwd.hpp"
#include "furc/ast/visitor.hpp"

namespace furc {
namespace ast {

/**
 * @brief Node category.
 */
enum class node_t {
    Literal,     /**< Literal. */
    Expression,  /**< Expression. */
    Statement,   /**< Statement. */
    Declaration, /**< Declaration. */
    Program,     /**< Program. */
};

/**
 * @brief Prints a node type (category) to an output stream.
 *
 * @param os Output stream.
 * @param type Type to print.
 * @return The output stream.
 */
static inline std::ostream& operator<<(std::ostream& os, node_t type) {
    switch (type) {
    case node_t::Literal: return os << "literal";
    case node_t::Expression: return os << "expression";
    case node_t::Statement: return os << "statement";
    case node_t::Declaration: return os << "declaration";
    case node_t::Program: return os << "program";
    }
    return os;
}

/**
 * @brief AST node interface.
 */
class node {
public:
    node()          = default;
    virtual ~node() = default;

    /**
     * @brief Move constructor.
     *
     * Constructs a node by transferring the state of another node.
     *
     * @param other Node to move from.
     */
    node(node&& other) = default;
    node(const node&)  = delete;

    /**
     * @brief Move constructor.
     *
     * Constructs a node by transferring the state of another node.
     *
     * @param other Node to move from.
     */
    node& operator=(node&& other) = default;
    node& operator=(const node&)  = delete;
public:
    /**
     * @brief Returns the category of this AST node.
     * @see node_t
     *
     * @return The node category.
     */
    virtual node_t category() const = 0;

    /**
     * @brief Returns the location of this AST node.
     * @see locaiton
     *
     * @return The location.
     */
    virtual location location() const = 0;
public:
    /**
     * @brief Compares two nodes for equality.
     *
     * Nodes are equal if they have the same category and
     * their derived-class-specific contents are equal.
     *
     * @param rhs Node to compare against.
     * @return true if the nodes are equal.
     */
    bool operator==(const node& rhs) const { return category() == rhs.category() && equal(rhs); }

    /**
     * @brief Compares two nodes for inequality.
     *
     * @param rhs Node to compare against.
     * @return true if the nodes are not equal.
     */
    bool operator!=(const node& rhs) const { return !this->operator==(rhs); }
public:
    /**
     * @brief Accepts a visitor.
     *
     * Dispatches to the visitor overload corresponding to the concrete node type.
     *
     * @param visitor Visitor instance.
     */
    virtual void accept(visitor& visitor) const = 0;

    /**
     * @brief Prints a node to an output stream.
     *
     * @param os Output stream.
     * @return The output stream.
     */
    virtual std::ostream& print(std::ostream& os) const = 0;

    /**
     * @brief Prints a node to an output stream.
     *
     * Equivalent to calling node.print(os).
     *
     * @param os Output stream.
     * @param node Node to print.
     * @return The output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const node& node) { return node.print(os); }
protected:
    /**
     * @brief Compares two nodes for equality.
     *
     * @param rhs Node to compare against.
     * @return true if nodes are equal.
     */
    virtual bool equal(const node& rhs) const = 0;
};

/**
 * @brief An abstract AST node.
 * @see node
 *
 * Implements location().
 */
class abstract_node : public virtual node {
public:
    abstract_node(struct location location)
      : p_location(location) {}
public:
    /**
     * @brief Returns the location of this AST node.
     * @see locaiton
     *
     * @return The location.
     */
    struct location location() const override { return p_location; }
protected:
    struct location p_location; /**< Node location. */
};

} // namespace ast
} // namespace furc

#endif // FURC_AST_NODE_HPP