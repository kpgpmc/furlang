#ifndef FURC_AST_STATEMENT_HPP
#define FURC_AST_STATEMENT_HPP

#include "furc/ast/node.hpp"

#include <ostream>

namespace furc {
namespace ast {

enum class statement_node_t {
    Declaration,
    Return,
};

static inline std::ostream& operator<<(std::ostream& os, statement_node_t type) {
    switch (type) {
    case statement_node_t::Declaration: return os << "declaration";
    case statement_node_t::Return: return os << "return";
    }
}

class statement_node : public abstract_node<node_t::Statement> {
public:
    virtual statement_node_t type() const = 0;
public:
    virtual std::ostream& print(std::ostream& os) const = 0;

    friend std::ostream& operator<<(std::ostream& os, const statement_node& node) {
        os << node.type() << " statement";
        return node.print(os);
    }
};

class declaration_node;
class declaration_statement_node : public statement_node {
public:
    declaration_statement_node(declaration_node* declaration)
      : m_declaration(declaration) {}
public:
    statement_node_t type() const override { return statement_node_t::Declaration; }

    std::ostream& print(std::ostream& os) const override { return os; }
private:
    declaration_node* m_declaration = nullptr;
};

class return_statement_node : public statement_node {
public:
    return_statement_node() = default;
public:
    statement_node_t type() const override { return statement_node_t::Return; }

    std::ostream& print(std::ostream& os) const override { return os; }
};

} // namespace ast
} // namespace furc

#endif // FURC_AST_STATEMENT_HPP