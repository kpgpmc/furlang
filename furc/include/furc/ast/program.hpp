#ifndef FURC_AST_PROGRAM_HPP
#define FURC_AST_PROGRAM_HPP

#include "furc/ast/declaration.hpp"
#include "furc/ast/node.hpp"

#include <ostream>
#include <vector>

namespace furc {
namespace ast {

class program_node : public abstract_node<node_t::Program> {
public:
    program_node() {}
public:
    void push(node_handle<declaration_node>&& declaration) { m_declarations.push_back(std::move(declaration)); }

    const std::vector<node_handle<declaration_node>>& declarations() const { return m_declarations; }
public:
    friend std::ostream& operator<<(std::ostream& os, const program_node& node) {
        os << "program";
        for (const auto& handle : node.m_declarations) {
            os << '\n' << handle;
        }
        return os;
    }
private:
    std::vector<node_handle<declaration_node>> m_declarations;
};

} // namespace ast
} // namespace furc

#endif // FURC_AST_PROGRAM_HPP