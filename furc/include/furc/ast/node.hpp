#ifndef FURC_AST_NODE_HPP
#define FURC_AST_NODE_HPP

#include "furc/handle.hpp"

#include <string>

namespace furc {
namespace ast {

enum class node_t {
    Literal,
    Expression,
    Statement,
    Declaration,
    Program,
};

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
};

template <node_t Category>
class abstract_node : public node {
public:
    node_t category() const override { return Category; }
};

template <typename T, typename Error = std::string>
using node_handle = handle<T*, Error>;

} // namespace ast
} // namespace furc

#endif // FURC_AST_NODE_HPP