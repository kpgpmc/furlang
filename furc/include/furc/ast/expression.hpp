#ifndef FURC_AST_EXPRESSION_HPP
#define FURC_AST_EXPRESSION_HPP

#include "furc/ast/node.hpp"
#include "furc/ast/statement.hpp"

namespace furc {
namespace ast {

/**
 * @brief Expression node type.
 */
enum class expression_node_t {
    Literal,   /**< Literal */
    VarRead,   /**< Variable read expression */
    Unaryop,   /**< Unary operation expression */
    Binop,     /**< Binary operation expression */
    VarAssign, /**< Variable assignment expression */
};

/**
 * @brief Expression AST node.
 */
class expression_node : public statement_node, public abstract_node {
public:
    /**
     * @brief Construct a new expression AST node.
     *
     * @param location Node location.
     */
    expression_node(struct location location)
      : abstract_node(location) {}
public:
    /**
     * @brief Returns this node's category.
     *
     * @return node_t::Expression.
     */
    node_t category() const override { return node_t::Expression; }

    /**
     * @brief Returns this node's statement type.
     *
     * @return statement_node_t::Expression.
     */
    statement_node_t statement_type() const override { return statement_node_t::Expression; }

    /**
     * @brief Returns this node's expression type.
     *
     * @return The expression type.
     */
    virtual expression_node_t expression_type() const = 0;
protected:
    bool equal(const node& rhs) const override;
};

/**
 * @brief Var read expression AST node.
 */
class var_read_expression_node final : public expression_node {
public:
    /**
     * @brief Construct a new var read expression node object from a name handle.
     *
     * @param location Node location.
     * @param name Handle to the name.
     */
    template <typename T>
    var_read_expression_node(struct location location, T&& name)
      : expression_node(location), m_name(std::forward<T>(name)) {}

    /**
     * @brief Returns the variable's name.
     *
     * @return Name of the variable.
     */
    const std::string& get_name() const { return m_name; }

    /**
     * @brief Returns the variable's name.
     *
     * @return Name of the variable.
     */
    std::string&& move_name() { return std::move(m_name); }
public:
    /**
     * @brief Returns this node's expression type.
     *
     * @return expression_node_t::VarRead.
     */
    expression_node_t expression_type() const override { return expression_node_t::VarRead; }
public:
    void accept(visitor& visitor) const override;

    std::ostream& print(std::ostream& os) const override;
protected:
    bool equal(const node& rhs) const override;
private:
    std::string m_name;
};

/**
 * @brief Unary operation node type.
 */
enum class unaryop_expression_node_t {
    Positive,         /**< Positive (unary plus) */
    Negative,         /**< Negative (unary minus) */
    PrefixIncrement,  /**< Prefix increment */
    PostfixIncrement, /**< Postfix increment */
    PrefixDecrement,  /**< Prefix decrement */
    PostfixDecrement, /**< Postfix decrement */
};

/**
 * @brief Unary operation expression AST node.
 */
class unary_op_expression_node final : public expression_node {
public:
    /**
     * @brief Construct a new unaryop expression node object from type and expression node handle.
     *
     * @param location Node location.
     * @param type Operation type.
     * @param node Handle to the inner expression node.
     */
    unary_op_expression_node(struct location location, unaryop_expression_node_t type, expression_node_p&& node)
      : expression_node(location), m_type(type), m_node(std::move(node)) {}

    /**
     * @brief Sets this node's inner expression.
     *
     * @param node New node handle.
     */
    void set_node(expression_node_p&& node) { m_node = std::move(node); }

    /**
     * @brief Returns the type of this node's operation.
     *
     * @return The operation type.
     */
    unaryop_expression_node_t type() const { return m_type; }

    /**
     * @brief Returns this node's inner expression.
     *
     * @return The inner expression.
     */
    const expression_node_p& get_node() const { return m_node; }

    /**
     * @brief Returns this node's inner expression.
     *
     * @return The inner expression.
     */
    expression_node_p& get_node() { return m_node; }

    /**
     * @brief Moves this node's inner expression.
     *
     * @return The moved inner expression.
     */
    expression_node_p&& move_node() { return std::move(m_node); }
public:
    /**
     * @brief Returns this node's expression type.
     *
     * @return expression_node_t::Unaryop.
     */
    expression_node_t expression_type() const override { return expression_node_t::Unaryop; }
public:
    void accept(visitor& visitor) const override;

    std::ostream& print(std::ostream& os) const override;
protected:
    bool equal(const node& rhs) const override;
private:
    unaryop_expression_node_t m_type;
    expression_node_p         m_node; /**< The inner expression. */
};

/**
 * @brief Binary operation expression node type.
 */
enum class binop_expression_node_t {
    None = 0, /**< None */
    Add,      /**< Addition */
    Sub,      /**< Subtraction */
    Mul,      /**< Multiplication */
    Div,      /**< Division */
    Mod,      /**< Modulo */

    Equal,        /**< Equality */
    NotEqual,     /**< Inequality */
    LessThan,     /**< Less */
    GreaterThan,  /**< Greater */
    LessEqual,    /**< Less or equal */
    GreaterEqual, /**< Greater or equal */
};

/**
 * @brief Binary operation expression AST node.
 */
class binary_op_expression_node final : public expression_node {
public:
    /**
     * @brief Construct a new binary operation expression AST node.
     *
     * @param location Node location.
     * @param type Binary operation type.
     * @param lhs Left-hand-side expression.
     * @param rhs Right-hand-side expression.
     */
    binary_op_expression_node(struct location location,
        binop_expression_node_t               type,
        expression_node_p&&                   lhs,
        expression_node_p&&                   rhs)
      : expression_node(location), m_type(type), m_lhs(std::move(lhs)), m_rhs(std::move(rhs)) {}

    /**
     * @brief Returns this node's binary operation type.
     *
     * @return The binary operation type.
     */
    binop_expression_node_t type() const { return m_type; };

    /**
     * @brief Returns this node's left-hand-side expression.
     *
     * @return The left-hand-side expression.
     */
    const expression_node_p& lhs() const { return m_lhs; };

    /**
     * @brief Returns this node's left-hand-side expression.
     *
     * @return The left-hand-side expression.
     */
    expression_node_p& lhs() { return m_lhs; };

    /**
     * @brief Moves this node's left-hand-side expression.
     *
     * @return The moved left-hand-side expression.
     */
    expression_node_p&& move_lhs() { return std::move(m_lhs); };

    /**
     * @brief Returns this node's right-hand-side expression.
     *
     * @return The right-hand-side expression.
     */
    const expression_node_p& rhs() const { return m_rhs; };

    /**
     * @brief Returns this node's right-hand-side expression.
     *
     * @return The right-hand-side expression.
     */
    expression_node_p& rhs() { return m_rhs; };

    /**
     * @brief Moves this node's right-hand-side expression.
     *
     * @return The moved right-hand-side expression.
     */
    expression_node_p&& move_rhs() { return std::move(m_rhs); };
public:
    expression_node_t expression_type() const override { return expression_node_t::Binop; }
public:
    void accept(visitor& visitor) const override;

    std::ostream& print(std::ostream& os) const override;
protected:
    bool equal(const node& rhs) const override;
private:
    binop_expression_node_t m_type;
    expression_node_p       m_lhs;
    expression_node_p       m_rhs;
};

/**
 * @brief Variable assignment expression AST node.
 */
class var_assign_expression_node final : public expression_node {
public:
    /**
     * @brief Construct a new variable assignment expression AST node.
     *
     * @param location Node location.
     * @param lhs Left-hand-side expression handle.
     * @param rhs Right-hand-side expression handle.
     */
    var_assign_expression_node(struct location location, expression_node_p&& lhs, expression_node_p&& rhs)
      : expression_node(location),
        m_compound(binop_expression_node_t::None),
        m_lhs(std::move(lhs)),
        m_rhs(std::move(rhs)) {}

    /**
     * @brief Construct a new compound variable assignment expression AST node.
     *
     * @param location Node location.
     * @param compound Compound operation type.
     * @param lhs Left-hand-side expression handle.
     * @param rhs Right-hand-side expression handle.
     */
    var_assign_expression_node(struct location location,
        binop_expression_node_t                compound,
        expression_node_p&&                    lhs,
        expression_node_p&&                    rhs)
      : expression_node(location), m_compound(compound), m_lhs(std::move(lhs)), m_rhs(std::move(rhs)) {}

    /**
     * @brief Returns this node's compound operation type.
     *
     * @return The compound operation type.
     */
    binop_expression_node_t compound() const { return m_compound; }

    /**
     * @brief Returns this node's left-hand-side expression.
     *
     * @return The left-hand-side expression.
     */
    const expression_node_p& lhs() const { return m_lhs; }

    /**
     * @brief Returns this node's right-hand-side expression.
     *
     * @return The right-hand-side expression.
     */
    const expression_node_p& rhs() const { return m_rhs; }
public:
    /**
     * @brief Returns this node's expression type.
     *
     * @return expression_node_t::VarAssign.
     */
    expression_node_t expression_type() const override { return expression_node_t::VarAssign; }
public:
    void accept(visitor& visitor) const override;

    std::ostream& print(std::ostream& os) const override;
protected:
    bool equal(const node& rhs) const override;
private:
    binop_expression_node_t m_compound;
    expression_node_p       m_lhs;
    expression_node_p       m_rhs;
};

} // namespace ast
} // namespace furc

#endif // FURC_AST_EXPRESSION_HPP