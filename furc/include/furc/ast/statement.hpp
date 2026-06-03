#ifndef FURC_AST_STATEMENT_HPP
#define FURC_AST_STATEMENT_HPP

#include "furc/ast/node.hpp"

namespace furc {
namespace ast {

/**
 * @brief Statement node type.
 */
enum class statement_node_t {
    Expression,  /**< Expression */
    Declaration, /**< Declaration */
    Return,      /**< Return statement */
    If,          /**< If statement */
    Compound,    /**< Compound statement */
};

/**
 * @brief Statement AST node.
 */
class statement_node : public node {
public:
    /**
     * @brief Returns this node's category.
     *
     * @return node_t::Statement.
     */
    node_t category() const override { return node_t::Statement; }

    /**
     * @brief Returns this node's statement type.
     *
     * @return The statement type.
     */
    virtual statement_node_t statement_type() const = 0;
protected:
    bool equal(const node& rhs) const override;
};

/**
 * @brief Return statement AST node.
 */
class return_statement_node final : public statement_node {
public:
    return_statement_node() = default;

    /**
     * @brief Construct a new return statement AST node.
     *
     * @param value Return value handle.
     */
    return_statement_node(expression_node_h&& value)
      : m_value(std::move(value)) {}
public:
    /**
     * @brief Returns this node's return value handle.
     *
     * @return The return value handle.
     */
    expression_node_h value() const { return m_value; }
public:
    /**
     * @brief Returns this node's statement type.
     *
     * @return statement_node_t::Return.
     */
    statement_node_t statement_type() const override { return statement_node_t::Return; }
public:
    void accept(visitor& visitor) const override;

    std::ostream& print(std::ostream& os) const override;
protected:
    bool equal(const node& rhs) const override;
private:
    expression_node_h m_value; /**< Return value handle. */
};

/**
 * @brief If statement AST node.
 */
class if_statement_node final : public statement_node {
public:
    /**
     * @brief Construct a new if statement AST node.
     *
     * @param cond Condition expression handle.
     * @param then Then statement handle.
     */
    if_statement_node(expression_node_h&& cond, statement_node_h&& then)
      : m_cond(std::move(cond)), m_then(std::move(then)) {}

    /**
     * @brief Construct a new if statement AST node.
     *
     * @param cond Condition expression handle.
     * @param then Then statement handle.
     * @param elze Else statement handle.
     */
    if_statement_node(expression_node_h&& cond, statement_node_h&& then, statement_node_h&& elze)
      : m_cond(std::move(cond)), m_then(std::move(then)), m_else(std::move(elze)) {}
public:
    /**
     * @brief Returns this node's condition expression handle.
     *
     * @return The condition expression handle.
     */
    expression_node_h cond() const { return m_cond; }

    /**
     * @brief Returns this node's then statement handle.
     *
     * @return The then statement handle.
     */
    const statement_node_h& then() const { return m_then; }

    /**
     * @brief Returns this node's else statement handle.
     *
     * @return The else statement handle.
     */
    const statement_node_h& elze() const { return m_else; }
public:
    /**
     * @brief Returns this node's statement type.
     *
     * @return statement_node_t::If.
     */
    statement_node_t statement_type() const override { return statement_node_t::If; }
public:
    void accept(visitor& visitor) const override;

    std::ostream& print(std::ostream& os) const override;
protected:
    bool equal(const node& rhs) const override;
private:
    expression_node_h m_cond; /**< The condition expression handle */
    statement_node_h  m_then; /**< The then statement handle */
    statement_node_h  m_else; /**< The else statement handle */
};

/**
 * @brief Compound statement AST node.
 */
class compound_statement_node final : public statement_node {
public:
    /**
     * @brief Construct a new compound statement AST node.
     *
     * @param body Body handle.
     */
    compound_statement_node(body_r&& body)
      : m_body(std::move(body)) {}
public:
    /**
     * @brief Returns this node's body handle.
     *
     * @return The body handle.
     */
    const body_r& body() const { return m_body; }
public:
    /**
     * @brief Returns this node's statement type.
     *
     * @return statement_node_t::Compound.
     */
    statement_node_t statement_type() const override { return statement_node_t::Compound; }
public:
    void accept(visitor& visitor) const override;

    std::ostream& print(std::ostream& os) const override;
protected:
    bool equal(const node& rhs) const override;
private:
    body_r m_body; /**< The body handle. */
};

} // namespace ast
} // namespace furc

#endif // FURC_AST_STATEMENT_HPP