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
    Function,
    Variable,
};

static inline std::ostream& operator<<(std::ostream& os, declaration_node_t type) {
    switch (type) {
    case declaration_node_t::Function: return os << "function";
    case declaration_node_t::Variable: return os << "variable";
    }
}

class declaration_node : public abstract_node<node_t::Declaration> {
public:
    virtual declaration_node_t type() const = 0;
public:
    virtual std::ostream& print(std::ostream& os) const = 0;

    friend std::ostream& operator<<(std::ostream& os, const declaration_node& node) {
        os << node.type() << " declaration";
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

    function_declarartion_node(front::token name, function_body&& body)
      : m_name(name), m_body(std::move(body)) {}
public:
    declaration_node_t type() const override { return declaration_node_t::Function; }
public:
    std::ostream& print(std::ostream& os) const override {
        os << ": " << m_name;
        if (m_body.has_value()) {
            os << '\n' << m_body->begin << ": begin:";
            for (const auto& entry : m_body->statements) {
                os << '\n' << entry;
            }
            os << '\n' << m_body->end << ": end";
        }
        return os;
    }
private:
    front::token                 m_name;
    std::optional<function_body> m_body;
};

} // namespace ast
} // namespace furc

#endif // FURC_AST_DECLARATION_HPP