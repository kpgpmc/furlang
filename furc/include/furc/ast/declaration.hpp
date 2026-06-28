#ifndef FURC_AST_DECLARATION_HPP
#define FURC_AST_DECLARATION_HPP

#include "furc/ast/node.hpp"
#include "furc/ast/statement.hpp"

#include <optional>
#include <string>
#include <type_traits>
#include <vector>

namespace furc {
namespace ast {

/**
 * @brief Type class for AST declarations.
 */
class type {
public:
    template <typename NameFwd, typename = std::enable_if_t<std::is_constructible_v<std::string, NameFwd>>>
    type(NameFwd&& name)
      : m_name(std::forward<NameFwd>(name)) {}
public:
    const std::string& name() const { return m_name; }
private:
    std::string m_name;
};

enum class declaration_access_t {
    Implicit = 0, /**< Implicit access. */
    Public,       /**< Public access. */
    Private,      /**< Private access. */
};

static inline bool same_access(declaration_access_t lhs, declaration_access_t rhs) {
    return lhs == declaration_access_t::Implicit || lhs == rhs;
}

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
     * @param access Declaration access.
     */
    declaration_node(struct location location, declaration_access_t access)
      : abstract_node(location), p_access(access) {}
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

    /**
     * @brief Returns the declaration's access.
     *
     * @return Access.
     */
    declaration_access_t access() const { return p_access; }
protected:
    bool equal(const node& rhs) const override;
protected:
    declaration_access_t p_access;
};

/**
 * @brief Parameter of function declaration AST node.
 */
struct function_declaration_param {
    std::string name;
    type        type;
};

enum class function_declaration_node_t {
    Normal = 0,
    Import,
    Native,
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
     * @param type Return type of the function.
     */
    template <typename T, typename ParamsFwd>
    function_declaration_node(struct location location,
        declaration_access_t                  access,
        T&&                                   name,
        std::optional<type>&&                 returnType,
        ParamsFwd&&                           params,
        function_declaration_node_t           type = function_declaration_node_t::Normal)
      : declaration_node(location, access),
        p_name(std::forward<T>(name)),
        p_returnType(std::move(returnType)),
        p_params(std::forward<ParamsFwd>(params)),
        p_type(type) {}
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

    /**
     * @brief Returns function's return type.
     *
     * @return Function's return type.
     */
    const std::optional<type>& return_type() const { return p_returnType; }

    /**
     * @brief Returns function's parameters.
     *
     * @return Function's parameters.
     */
    const std::vector<function_declaration_param>& params() const { return p_params; }

    /**
     * @brief Returns function's type.
     *
     * @return Function's type.
     */
    function_declaration_node_t type() const { return p_type; }
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

    /**
     * @brief Return type of the function.
     */
    std::optional<class type> p_returnType;

    /**
     * @brief Parameters of the function.
     */
    std::vector<function_declaration_param> p_params;

    /**
     * @brief Type of the function declaration.
     */
    function_declaration_node_t p_type;
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
     * @param type Return type of the function.
     * @param body Body of the function.
     */
    template <typename T, typename ParamsFwd>
    function_definition_node(struct location location,
        declaration_access_t                 access,
        T&&                                  name,
        std::optional<class type>&&          type,
        ParamsFwd&&                          params,
        body&&                               body)
      : function_declaration_node(location,
            access,
            std::forward<T>(name),
            std::move(type),
            std::forward<ParamsFwd>(params)),
        m_body(std::move(body)) {}
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
    const body& body() const { return m_body; }
public:
    void accept(visitor& visitor) const override;

    std::ostream& print(std::ostream& os) const override;
protected:
    bool equal(const node& rhs) const override;
private:
    struct body m_body;
};

} // namespace ast
} // namespace furc

#endif // FURC_AST_DECLARATION_HPP
