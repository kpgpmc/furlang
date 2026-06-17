#ifndef FURVM_CONTEXT_HPP
#define FURVM_CONTEXT_HPP

#include "furlang/arena.hpp"
#include "furvm/executor.hpp"
#include "furvm/fwd.hpp"
#include "furvm/module.hpp"

#include <memory>
#include <queue>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
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
     * @brief Emplaces a new module in this context.
     *
     * @param args Arguments to forward to module's constructor.
     * @return A handle to the module.
     */
    template <typename... Args, typename = std::enable_if_t<std::is_constructible_v<mod, Args...>>>
    mod_h emplace_module(Args&&... args) {
        mod_p       mod  = std::make_shared<class mod>(std::forward<Args>(args)...);
        std::string name = mod->name();
        return { name, m_modules[name] = std::move(mod) };
    }

    /**
     * @brief Erases a module from this context.
     *
     * @param name Name of the module to erase.
     * @return A shared pointer to the erased module.
     */
    template <typename Name>
    mod_p erase_module(Name&& name) {
        return std::move(m_modules.erase(std::forward<Name>(name))->second);
    }

    /**
     * @brief Returns a module of this context.
     *
     * @param name Name of the module.
     * @return The module.
     */
    template <typename Name>
    constexpr mod_p& module_at(Name&& name) {
        return m_modules.at(std::forward<Name>(name));
    }

    /**
     * @brief Returns a module of this context.
     *
     * @param name Name of the module.
     * @return The module.
     */
    template <typename Name>
    constexpr const mod_p& module_at(Name&& name) const {
        return m_modules.at(std::forward<Name>(name));
    }

    /**
     * @brief Returns how many does this context have modules.
     *
     * @return The module count.
     */
    constexpr size_t module_count() const { return m_modules.size(); }
public:
    template <typename... Args, typename = std::enable_if_t<std::is_constructible_v<executor, Args...>>>
    executor_h emplace_executor(Args&&... args) {
        executor    executor(std::forward<Args>(args)...);
        executor_id id = m_executors.size();
        m_executors.emplace_back(std::move(executor));
        return { id, m_executors[id] };
    }
public:
    /**
     * @brief Removes unreferenced things from the thing list.
     */
    void collect();
private:
    std::unordered_map<std::string, mod_p> m_modules;

    std::vector<thing_p>  m_things;
    std::vector<executor> m_executors;

    std::queue<thing_id> m_deadThings;
    std::vector<void*>   m_deadThingData;
    furlang::arena       m_thingArena;
};

} // namespace furvm

#endif // FURVM_CONTEXT_HPP
