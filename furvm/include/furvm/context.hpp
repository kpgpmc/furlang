#ifndef FURVM_CONTEXT_HPP
#define FURVM_CONTEXT_HPP

#include "furvm/fwd.hpp"
#include "furvm/module.hpp"

#include <memory>
#include <type_traits>
#include <vector>

namespace furvm {

class context {
public:
    using value_type = std::unique_ptr<mod>; /**< An alias to unique pointer of module. */
public:
    context()  = default;
    ~context() = default;

    /**
     * @brief Move constructor.
     */
    context(context&&) = default;

    /**
     * @brief Move constructor.
     */
    context& operator=(context&&) = default;

    context(const context&)            = delete;
    context& operator=(const context&) = delete;
public:
    /**
     * @brief Adds a module to this context.
     *
     * @param args Arguments to forward to module's constructor.
     * @return An index to the emplaced module.
     */
    template <typename... Args, typename = std::enable_if_t<std::is_constructible_v<mod, Args...>>>
    constexpr module_index emplace(Args&&... args) {
        module_index index = m_modules.size();
        m_modules.emplace_back(std::make_unique<mod>(std::forward<Args>(args)...));
        return index;
    }

    /**
     * @brief Erases a module from this context.
     *
     * @param index Index to the module.
     * @return Old value.
     */
    value_type erase(module_index index) {
        if (index >= m_modules.size()) return nullptr;
        return std::move(m_modules[index]);
    }

    /**
     * @brief Returns a module of this context.
     *
     * @param index Position of the module.
     * @return The module.
     */
    constexpr value_type& operator[](module_index index) { return m_modules[index]; }

    /**
     * @brief Returns a module of this context.
     *
     * @param index Position of the module.
     * @return The module.
     */
    constexpr const value_type& operator[](module_index index) const { return m_modules[index]; }

    /**
     * @brief Returns a module of this context.
     *
     * @param index Position of the module.
     * @return The module.
     */
    constexpr value_type& at(module_index index) { return m_modules.at(index); }

    /**
     * @brief Returns a module of this context.
     *
     * @param index Position of the module.
     * @return The module.
     */
    constexpr const value_type& at(module_index index) const { return m_modules.at(index); }

    /**
     * @brief Returns how many does this context have modules.
     *
     * @return The module count.
     */
    constexpr size_t size() const { return m_modules.size(); }
private:
    std::vector<std::unique_ptr<mod>> m_modules;
};

} // namespace furvm

#endif // FURVM_CONTEXT_HPP