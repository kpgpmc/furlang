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

static inline std::ostream& operator<<(std::ostream& os, declaration_node_t type) {
    switch (type) {
    case declaration_node_t::FunctionDeclaration: return os << "function declaration";
    case declaration_node_t::FunctionDefinition: return os << "function definition";
    case declaration_node_t::Variable: return os << "variable";
    }
}

class declaration_node : public abstract_node<node_t::Declaration> {
public:
    virtual declaration_node_t type() const = 0;
public:
    virtual std::ostream& print(std::ostream& os) const = 0;

    friend std::ostream& operator<<(std::ostream& os, const declaration_node& node) {
        os << node.type();
        return node.print(os);
    }
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
    declaration_node_t type() const override { return declaration_node_t::FunctionDeclaration; }

    front::token name() const { return m_name; }
public:
    std::ostream& print(std::ostream& os) const override { return os << ": " << m_name; }
protected:
    front::token m_name;
};

class function_definition_node : public function_declarartion_node {
public:
    function_definition_node(front::token name, function_body_handle&& body)
      : function_declarartion_node(name), m_body(std::move(body)) {}

    ~function_definition_node() override = default;

    function_definition_node(function_definition_node&& other) noexcept
      : function_declarartion_node(std::move(other)), m_name(other.m_name), m_body(std::move(other.m_body)) {}

    function_definition_node(const function_definition_node&) = delete;

    function_definition_node& operator=(function_definition_node&& other) noexcept {
        if (this == &other) return *this;
        function_declarartion_node::operator=(std::move(other));
        m_name = other.m_name;
        m_body = std::move(other.m_body);
        return *this;
    }

    function_definition_node& operator=(const function_definition_node&) = delete;
public:
    declaration_node_t type() const override { return declaration_node_t::FunctionDefinition; }

    const function_body_handle& body() const { return m_body; }
public:
    std::ostream& print(std::ostream& os) const override {
        os << ": " << m_name.value;
        if (m_body.present()) {
            os << '\n' << m_body->begin << ": begin:";
            for (const auto& entry : m_body->statements) {
                os << '\n' << entry;
            }
            os << '\n' << m_body->end << ": end";
        }
        return os;
    }
private:
    front::token         m_name;
    function_body_handle m_body;
};

} // namespace ast
} // namespace furc

#endif // FURC_AST_DECLARATION_HPP