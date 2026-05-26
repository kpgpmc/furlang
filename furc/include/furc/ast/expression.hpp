#ifndef FURC_AST_EXPRESSION_HPP
#define FURC_AST_EXPRESSION_HPP

#include "furc/ast/node.hpp"
#include "furc/ast/statement.hpp"

namespace furc {
namespace ast {

enum class expression_node_t {
    Literal
};

class expression_node : public statement_node {
public:
    node_t category() const override { return node_t::Expression; }

    statement_node_t statement_type() const override { return statement_node_t::Expression; }

    virtual expression_node_t expression_type() const = 0;
protected:
    bool equal(const node& rhs) const override;
};

} // namespace ast
} // namespace furc

#endif // FURC_AST_EXPRESSION_HPP