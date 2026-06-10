#ifndef FURVM_CONTEXT_HPP
#define FURVM_CONTEXT_HPP

#include "furlang/arena.hpp"
#include "furvm/fwd.hpp"
#include "furvm/module.hpp"

#include <queue>
#include <type_traits>
#include <vector>

namespace furvm {

class context {
public:
    friend class executor;
    friend class thing;
public:
    context();
    ~context() = default;

    /**
     * @brief Move constructor.
     */
    context(context&&) noexcept = default;

    /**
     * @brief Move constructor.
     */
    context& operator=(context&&) noexcept = default;

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
    mod_p erase(module_handle index) {
        if (index >= m_modules.size()) return nullptr;
        return std::move(m_modules[index]);
    }

    /**
     * @brief Returns a module of this context.
     *
     * @param index Position of the module.
     * @return The module.
     */
    constexpr mod_p& operator[](module_handle index) { return m_modules[index]; }

    /**
     * @brief Returns a module of this context.
     *
     * @param index Position of the module.
     * @return The module.
     */
    constexpr const mod_p& operator[](module_handle index) const { return m_modules[index]; }

    /**
     * @brief Returns a module of this context.
     *
     * @param index Position of the module.
     * @return The module.
     */
    constexpr mod_p& at(module_handle index) { return m_modules.at(index); }

    /**
     * @brief Returns a module of this context.
     *
     * @param index Position of the module.
     * @return The module.
     */
    constexpr const mod_p& at(module_handle index) const { return m_modules.at(index); }

    /**
     * @brief Returns how many does this context have modules.
     *
     * @return The module count.
     */
    constexpr size_t size() const { return m_modules.size(); }
public:
    /**
     * @brief Removes unreferenced things from the thing list.
     */
    void collect();
private:
    std::vector<mod_p>      m_modules;
    std::vector<thing_p>    m_things;
    std::vector<executor_p> m_executors;

    std::queue<thing_handle> m_deadThings;
    std::vector<void*>       m_deadThingData;
    furlang::arena           m_thingArena;
};

} // namespace furvm

#endif // FURVM_CONTEXT_HPP