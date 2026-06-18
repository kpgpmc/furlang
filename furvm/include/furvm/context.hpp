#ifndef FURVM_CONTEXT_HPP
#define FURVM_CONTEXT_HPP

#include "furlang/arena.hpp"
#include "furvm/executor.hpp"
#include "furvm/fwd.hpp"
#include "furvm/handle.hpp"
#include "furvm/module.hpp" // IWYU pragma: keep
#include "furvm/thing.hpp"  // IWYU pragma: keep

#include <cstddef>
#include <utility>

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
    context(context&&) noexcept = default;

    /**
     * @brief Move constructor.
     */
    context& operator=(context&&) noexcept = default;

    context(const context&)            = delete;
    context& operator=(const context&) = delete;
public:
    template <typename... Args>
    auto emplace_module(Args&&... args) {
        return m_modules.emplace(std::forward<Args>(args)...);
    }

    template <typename... Args>
    auto module_at(Args&&... args) {
        return m_modules.at(std::forward<Args>(args)...);
    }

    template <typename... Args>
    auto module_at(Args&&... args) const {
        return m_modules.at(std::forward<Args>(args)...);
    }

    template <typename... Args>
    auto erase_module(Args&&... args) {
        return m_modules.erase(std::forward<Args>(args)...);
    }
public:
    template <typename... Args>
    auto emplace_executor(Args&&... args) {
        return m_executors.emplace_back(std::forward<Args>(args)...);
    }

    template <typename... Args>
    auto executor_at(Args&&... args) {
        return m_executors.at(std::forward<Args>(args)...);
    }

    template <typename... Args>
    auto executor_at(Args&&... args) const {
        return m_executors.at(std::forward<Args>(args)...);
    }

    template <typename... Args>
    auto erase_executor(Args&&... args) {
        return m_executors.erase(std::forward<Args>(args)...);
    }
public:
    template <typename... Args>
    auto emplace_thing(Args&&... args) {
        return m_things.emplace_back(std::forward<Args>(args)...);
    }

    template <typename... Args>
    auto thing_at(Args&&... args) {
        return m_things.at(std::forward<Args>(args)...);
    }

    template <typename... Args>
    auto thing_at(Args&&... args) const {
        return m_things.at(std::forward<Args>(args)...);
    }

    template <typename... Args>
    auto erase_thing(Args&&... args) {
        return m_things.erase(std::forward<Args>(args)...);
    }
public:
    /**
     * @brief Removes unreferenced things from the thing list.
     */
    void collect();
private:
    handle_container<mod_h>      m_modules;
    handle_container<thing_h>    m_things;
    handle_container<executor_h> m_executors;

    furlang::arena             m_thingArena;
    thing_allocator<std::byte> m_thingAllocator;
};

} // namespace furvm

#endif // FURVM_CONTEXT_HPP
