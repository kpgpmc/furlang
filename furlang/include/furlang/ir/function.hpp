#ifndef FURLANG_IR_FUNCTION_HPP
#define FURLANG_IR_FUNCTION_HPP

#include "furlang/ir/block.hpp"

#include <cstdint>
#include <memory>
#include <type_traits>
#include <vector>

namespace furlang {
namespace ir {

enum class function_t : std::uint8_t {
    Normal = 0,
    Import,
    Native,
};

enum class function_access_t : std::uint8_t {
    Public = 0,
    Private,
};

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
     * @param name Name to forward.
     */
    template <typename StringFwd, typename = std::enable_if_t<std::is_constructible_v<std::string, StringFwd>>>
    function(StringFwd&& name, function_access_t access, std::uint32_t paramCount, function_t type = function_t::Normal)
      : m_name(std::forward<StringFwd>(name)), m_access(access), m_paramCount(paramCount), m_type(type) {}
public:
    /**
     * @brief Returns this function's name.
     *
     * @return The name.
     */
    const std::string& name() const { return m_name; }

    /**
     * @brief Returns the function's access.
     *
     * @return The access.
     */
    function_access_t access() const { return m_access; }

    /**
     * @brief Returns this function's parameter count.
     *
     * @return The parameter count.
     */
    std::uint32_t param_count() const { return m_paramCount; }

    /**
     * @brief Returns this function's type.
     *
     * @return The type.
     */
    function_t type() const { return m_type; }

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
    function_access_t       m_access;
    function_t              m_type;
    std::uint32_t           m_paramCount;
    std::vector<value_type> m_blocks;
};

} // namespace ir
} // namespace furlang

#endif // FURLANG_IR_FUNCTION_HPP
