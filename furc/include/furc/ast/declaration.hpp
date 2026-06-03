#ifndef FURC_AST_DECLARATION_HPP
#define FURC_AST_DECLARATION_HPP

#include "furc/ast/node.hpp"
#include "furc/ast/statement.hpp"

#include <string>

namespace furc {
namespace ast {

/**
 * @brief Declaration node type.
 */
enum class declaration_node_t {
    Func,    /**< Function declaration. */
    FuncDef, /**< Function definition. */
};

/**
 * @brief Declaration AST node interface.
 */
class declaration_node : public statement_node, public abstract_node {
public:
    /**
     * @brief Construct a new declaration AST node.
     *
     * @param location Node location.
     */
    declaration_node(struct location location)
      : abstract_node(location) {}
public:
    /**
     * @brief Returns this node's category.
     *
     * @return node_t::Declaration.
     */
    node_t category() const override { return node_t::Declaration; }

    /**
     * @brief Returns this node's statement type.
     *
     * @return statement_node_t::Declaration.
     */
    statement_node_t statement_type() const final { return statement_node_t::Declaration; }

    /**
     * @brief Returns this node's declaration type.
     *
     * @return The declaration type.
     */
    virtual declaration_node_t declaration_type() const = 0;
protected:
    bool equal(const node& rhs) const override;
};

/**
 * @brief Function declaration AST node.
 */
class function_declaration_node : public declaration_node {
public:
    /**
     * @brief Construct a new function declaration node object from name token.
     *
     * @param location Node location.
     * @param name Name of the function.
     */
    template <typename T>
    function_declaration_node(struct location location, T&& name)
      : declaration_node(location), p_name(std::forward<T>(name)) {}
public:
    /**
     * @brief Returns this node's declaration type.
     *
     * @return declaration_node_t::FunctionDeclaration.
     */
    declaration_node_t declaration_type() const override { return declaration_node_t::Func; }

    /**
     * @brief Returns function's name.
     *
     * @return Name of the function.
     */
    std::string name() const { return p_name; }
public:
    void accept(visitor& visitor) const override;

    std::ostream& print(std::ostream& os) const override;
protected:
    bool equal(const node& rhs) const override;
protected:
    /**
     * @brief Name of the function.
     */
    std::string p_name;
};

/**
 * @brief Function definition AST node.
 */
class function_definition_node final : public function_declaration_node {
public:
    /**
     * @brief Construct a new function definition node object from name and body.
     *
     * @param location Node location.
     * @param name Name of the function.
     * @param body Body of the function.
     */
    template <typename T>
    function_definition_node(struct location location, T&& name, body_r&& body)
      : function_declaration_node(location, std::forward<T>(name)), m_body(std::move(body)) {}
public:
    /**
     * @brief Returns this node's declaration type.
     *
     * @return declaration_node_t::FunctionDefinition.
     */
    declaration_node_t declaration_type() const override { return declaration_node_t::FuncDef; }

    /**
     * @brief Returns function's body.
     *
     * @return Body of the function.
     */
    const body_r& body() const { return m_body; }
public:
    void accept(visitor& visitor) const override;

    std::ostream& print(std::ostream& os) const override;
protected:
    bool equal(const node& rhs) const override;
private:
    body_r m_body;
};

} // namespace ast
} // namespace furc

#endif // FURC_AST_DECLARATION_HPP
