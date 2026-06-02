#ifndef FURLANG_IR_FUNCTION_HPP
#define FURLANG_IR_FUNCTION_HPP

#include "furlang/ir/block.hpp"

#include <memory>
#include <vector>

namespace furlang {
namespace ir {

/**
 * @brief IR function.
 *
 * Consists of a name and blocks.
 * @see block
 */
class function {
public:
    using value_type = std::shared_ptr<block>; /**< Value type. */
public:
    /**
     * @brief Construct a new IR function.
     *
     * @param name Name to copy.
     */
    function(const std::string& name)
      : m_name(name) {}

    /**
     * @brief Construct a new IR function.
     *
     * @param name Name to move.
     */
    function(std::string&& name)
      : m_name(std::move(name)) {}
public:
    /**
     * @brief Returns this function's name.
     *
     * @return The name.
     */
    const std::string& name() const { return m_name; }

    /**
     * @brief Pushes and returns a new IR block.
     *
     * @return The new IR block.
     */
    value_type push() { return m_blocks.emplace_back(std::make_shared<block>()); }

    /**
     * @brief Returns this function's IR blocks.
     *
     * @return The IR blocks.
     */
    const std::vector<value_type>& blocks() const { return m_blocks; }
private:
    std::string             m_name;
    std::vector<value_type> m_blocks;
};

} // namespace ir
} // namespace furlang

#endif // FURLANG_IR_FUNCTION_HPP