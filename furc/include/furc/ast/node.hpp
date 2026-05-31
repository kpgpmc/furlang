#ifndef FURC_AST_NODE_HPP
#define FURC_AST_NODE_HPP

#include "furc/ast/fwd.hpp"
#include "furc/ast/visitor.hpp"

namespace furc {
namespace ast {

enum class node_t {
    Literal,
    Expression,
    Statement,
    Declaration,
    Program,
};

static inline std::ostream& operator<<(std::ostream& os, node_t type) {
    switch (type) {
    case node_t::Literal: return os << "literal";
    case node_t::Expression: return os << "expression";
    case node_t::Statement: return os << "statement";
    case node_t::Declaration: return os << "declaration";
    case node_t::Program: return os << "program";
    }
}

class node {
public:
    node()          = default;
    virtual ~node() = default;

    node(node&&)                 = default;
    node(const node&)            = delete;
    node& operator=(node&&)      = default;
    node& operator=(const node&) = delete;
public:
    virtual node_t category() const = 0;
public:
    bool operator==(const node& rhs) const { return category() == rhs.category() && equal(rhs); }
    bool operator!=(const node& rhs) const { return !this->operator==(rhs); }
public:
    virtual void accept(visitor& visitor) const = 0;

    virtual std::ostream& print(std::ostream& os) const = 0;

    friend std::ostream& operator<<(std::ostream& os, const node& node) { return node.print(os); }
protected:
    virtual bool equal(const node& rhs) const = 0;
};

} // namespace ast
} // namespace furc

#endif // FURC_AST_NODE_HPP