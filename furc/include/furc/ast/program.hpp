#ifndef FURC_AST_PROGRAM_HPP
#define FURC_AST_PROGRAM_HPP

#include "furc/ast/node.hpp"

#include <vector>

namespace furc {
namespace ast {

/**
 * @brief Program AST node.
 */
class program_node final : public abstract_node {
public:
    /**
     * @brief Construct a new program AST node.
     *
     * @param location Node location.
     */
    program_node(struct location location)
      : abstract_node(location) {}

    node_t category() const override { return node_t::Program; }
public:
    /**
     * @brief Adds a declaration to this program.
     *
     * @param declaration Declaration to add.
     */
    void push(declaration_node_p&& declaration) { m_declarations.push_back(std::move(declaration)); }

    /**
     * @brief Returns a list of declarations of this program.
     *
     * @return The list of this program's declarations.
     */
    const std::vector<declaration_node_p>& declarations() const { return m_declarations; }
public:
    void accept(visitor& visitor) const override;

    std::ostream& print(std::ostream& os) const override;
protected:
    bool equal(const node& rhs) const override;
private:
    std::vector<declaration_node_p> m_declarations;
};

} // namespace ast
} // namespace furc

#endif // FURC_AST_PROGRAM_HPP