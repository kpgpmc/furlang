#ifndef FURLANG_IR_MODULE_HPP
#define FURLANG_IR_MODULE_HPP

#include "furlang/ir/function.hpp"

#include <memory>
#include <vector>

namespace furlang {
namespace ir {

/**
 * @brief IR module
 */
class mod {
public:
    using value_type = std::unique_ptr<function>; /**< Value type. */
public:
    mod() = default;
public:
    /**
     * @brief Pushes and returns a new IR function.
     *
     * @param args Arguments to call the constructor with.
     * @return The new IR function.
     */
    template <typename... Args>
    value_type& push(Args&&... args) {
        return m_functions.emplace_back(std::forward<Args>(args)...);
    }

    /**
     * @brief Returns this module's functions.
     *
     * @return The functions.
     */
    std::vector<value_type>& functions() { return m_functions; }

    /**
     * @brief Returns this module's functions.
     *
     * @return The functions.
     */
    const std::vector<value_type>& functions() const { return m_functions; }
private:
    std::vector<value_type> m_functions;
};

} // namespace ir
} // namespace furlang

#endif // FURLANG_IR_MODULE_HPP
