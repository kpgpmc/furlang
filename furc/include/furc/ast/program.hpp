#ifndef FURC_AST_PROGRAM_HPP
#define FURC_AST_PROGRAM_HPP

#include "furc/ast/declaration.hpp"
#include "furc/ast/node.hpp"

#include <vector>

namespace furc {
namespace ast {

class program_node : public node {
public:
    program_node() = default;

    node_t category() const override { return node_t::Program; }
public:
    void push(node_handle<declaration_node>&& declaration) { m_declarations.push_back(std::move(declaration)); }

    const std::vector<node_handle<declaration_node>>& declarations() const { return m_declarations; }
public:
    std::ostream& print(std::ostream& os) const override;
protected:
    bool equal(const node& rhs) const override;
private:
    std::vector<node_handle<declaration_node>> m_declarations;
};

using program_node_h = node_handle<program_node>;

} // namespace ast
} // namespace furc

#endif // FURC_AST_PROGRAM_HPP