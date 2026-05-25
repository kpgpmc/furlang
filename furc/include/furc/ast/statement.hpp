#ifndef FURC_AST_STATEMENT_HPP
#define FURC_AST_STATEMENT_HPP

#include "furc/ast/node.hpp"

#include <ostream>

namespace furc {
namespace ast {

enum class statement_node_t {
    Expression,
    Declaration,
    Return,
};

class statement_node : public node {
public:
    node_t category() const override { return node_t::Statement; }

    virtual statement_node_t statement_type() const = 0;
};

class return_statement_node : public statement_node {
public:
    return_statement_node() = default;
public:
    statement_node_t statement_type() const override { return statement_node_t::Return; }

    std::ostream& print(std::ostream& os) const override { return os << "return statement"; }
};

} // namespace ast
} // namespace furc

#endif // FURC_AST_STATEMENT_HPP