#ifndef FURC_AST_DECLARATION_HPP
#define FURC_AST_DECLARATION_HPP

#include "furc/ast/node.hpp"
#include "furc/ast/statement.hpp"
#include "furc/front/token.hpp"

namespace furc {
namespace ast {

enum class declaration_node_t {
    FunctionDeclaration,
    FunctionDefinition,
};

class declaration_node : public statement_node {
public:
    node_t category() const override { return node_t::Declaration; }

    statement_node_t statement_type() const override { return statement_node_t::Declaration; }

    virtual declaration_node_t declaration_type() const = 0;
protected:
    bool equal(const node& rhs) const override;
};

class function_declaration_node : public declaration_node {
public:
    function_declaration_node(front::token name)
      : m_name(name) {}
public:
    declaration_node_t declaration_type() const override { return declaration_node_t::FunctionDeclaration; }

    front::token name() const { return m_name; }
public:
    void accept(visitor& visitor) const override;

    std::ostream& print(std::ostream& os) const override;
protected:
    bool equal(const node& rhs) const override;
protected:
    front::token m_name;
};

class function_definition_node : public function_declaration_node {
public:
    function_definition_node(front::token name, body_h&& body)
      : function_declaration_node(name), m_body(std::move(body)) {}
public:
    declaration_node_t declaration_type() const override { return declaration_node_t::FunctionDefinition; }

    const body_h& body() const { return m_body; }
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

#endif // FURC_AST_DECLARATION_HPP
