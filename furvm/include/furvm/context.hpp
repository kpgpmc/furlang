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
    friend class executor;
public:
    context();
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
    template <typename... Args, typename = std::enable_if_t<std::is_constructible_v<mod, module_handle, Args...>>>
    constexpr const auto& emplace(Args&&... args) {
        module_handle id = static_cast<module_handle>(m_modules.size());
        return m_modules.emplace_back(std::make_unique<mod>(id, std::forward<Args>(args)...));
    }

    /**
     * @brief Erases a module from this context.
     *
     * @param index Index to the module.
     * @return Old value.
     */
    std::shared_ptr<mod> erase(module_handle index) {
        if (index >= m_modules.size()) return nullptr;
        return std::move(m_modules[index]);
    }

    /**
     * @brief Returns a module of this context.
     *
     * @param index Position of the module.
     * @return The module.
     */
    constexpr std::shared_ptr<mod>& operator[](module_handle index) { return m_modules[index]; }

    /**
     * @brief Returns a module of this context.
     *
     * @param index Position of the module.
     * @return The module.
     */
    constexpr const std::shared_ptr<mod>& operator[](module_handle index) const { return m_modules[index]; }

    /**
     * @brief Returns a module of this context.
     *
     * @param index Position of the module.
     * @return The module.
     */
    constexpr std::shared_ptr<mod>& at(module_handle index) { return m_modules.at(index); }

    /**
     * @brief Returns a module of this context.
     *
     * @param index Position of the module.
     * @return The module.
     */
    constexpr const std::shared_ptr<mod>& at(module_handle index) const { return m_modules.at(index); }

    /**
     * @brief Returns how many does this context have modules.
     *
     * @return The module count.
     */
    constexpr size_t size() const { return m_modules.size(); }
private:
    std::vector<std::shared_ptr<mod>>      m_modules;
    std::vector<std::shared_ptr<thing>>    m_things;
    std::vector<std::shared_ptr<executor>> m_executors;
};

} // namespace furvm

#endif // FURVM_CONTEXT_HPP