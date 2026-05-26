#ifndef FURC_AST_DECLARATION_HPP
#define FURC_AST_DECLARATION_HPP

#include "furc/ast/node.hpp"
#include "furc/ast/statement.hpp"
#include "furc/front/token.hpp"

#include <ostream>
#include <vector>

namespace furc {
namespace ast {

enum class declaration_node_t {
    FunctionDeclaration,
    FunctionDefinition,
    Variable,
};

class declaration_node : public statement_node {
public:
    node_t category() const override { return node_t::Declaration; }

    statement_node_t statement_type() const override { return statement_node_t::Declaration; }

    virtual declaration_node_t declaration_type() const = 0;
};

struct function_body {
    location                                 begin, end;
    std::vector<node_handle<statement_node>> statements;
};

using function_body_handle = handle<ast::function_body>;

class function_declarartion_node : public declaration_node {
public:
    function_declarartion_node(front::token name)
      : m_name(name) {}
public:
    declaration_node_t declaration_type() const override { return declaration_node_t::FunctionDeclaration; }

    front::token name() const { return m_name; }
public:
    std::ostream& print(std::ostream& os) const override {
        return os << "function " << m_name->string << " declaration";
    }
protected:
    front::token m_name;
};

class function_definition_node : public function_declarartion_node {
public:
    function_definition_node(front::token name, function_body_handle&& body)
      : function_declarartion_node(name), m_body(std::move(body)) {}

    ~function_definition_node() override = default;

    function_definition_node(function_definition_node&& other) noexcept
      : function_declarartion_node(std::move(other)), m_body(std::move(other.m_body)) {}

    function_definition_node(const function_definition_node&) = delete;

    function_definition_node& operator=(function_definition_node&& other) noexcept {
        if (this == &other) return *this;
        function_declarartion_node::operator=(std::move(other));
        m_body = std::move(other.m_body);
        return *this;
    }

    function_definition_node& operator=(const function_definition_node&) = delete;
public:
    declaration_node_t declaration_type() const override { return declaration_node_t::FunctionDefinition; }

    const function_body_handle& body() const { return m_body; }
public:
    std::ostream& print(std::ostream& os) const override {
        function_declarartion_node::print(os);
        os << ':';
        if (m_body.present()) {
            for (const auto& entry : m_body->statements)
                os << '\n' << entry;
            return os << '\n' << m_body->end << ": " << m_name->string << " end";
        }
        return os << m_body.error(); // error
    }
private:
    function_body_handle m_body;
};

} // namespace ast
} // namespace furc

#endif // FURC_AST_DECLARATION_HPP