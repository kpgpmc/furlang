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

class context : public handle_container<mod_h> {
public:
    friend class executor;
public:
    /**
     * @brief Constructs a context.
     */
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
     * @brief Emplaces an executor in the context.
     *
     * @param args Arguments forwarded to executor's constructor.
     * @return A handle to the emplaced executor.
     */
    template <typename... Args>
    auto emplace_executor(Args&&... args) {
        return m_executors.emplace_back(std::forward<Args>(args)...);
    }

    /**
     * @brief Returns an executor from the context.
     *
     * @param args Id of the executor.
     * @return A handle to the executor.
     */
    template <typename... Args>
    auto executor_at(Args&&... args) {
        return m_executors.at(std::forward<Args>(args)...);
    }

    /**
     * @brief Returns an executor from the context.
     *
     * @param args Id of the executor.
     * @return A handle to the executor.
     */
    template <typename... Args>
    auto executor_at(Args&&... args) const {
        return m_executors.at(std::forward<Args>(args)...);
    }

    /**
     * @brief Erases an executor from the context.
     *
     * @param args Id of the executor.
     */
    template <typename... Args>
    void erase_executor(Args&&... args) {
        m_executors.erase(std::forward<Args>(args)...);
    }
public:
    template <typename... Args>
    auto emplace_thing(Args&&... args) {
        return m_things.emplace_back(std::forward<Args>(args)...);
    }

    /**
     * @brief Returns a thing from the context.
     *
     * @param args Id of the thing.
     * @return A handle to the thing.
     */
    template <typename... Args>
    auto thing_at(Args&&... args) {
        return m_things.at(std::forward<Args>(args)...);
    }

    /**
     * @brief Returns a thing from the context.
     *
     * @param args Id of the thing.
     * @return A handle to the thing.
     */
    template <typename... Args>
    auto thing_at(Args&&... args) const {
        return m_things.at(std::forward<Args>(args)...);
    }

    /**
     * @brief Erases a thing from the context.
     *
     * @param args Id of the thing.
     */
    template <typename... Args>
    void erase_thing(Args&&... args) {
        m_things.erase(std::forward<Args>(args)...);
    }

    /**
     * @brief Returns context's thing allocator.
     *
     * @return The thing allocator.
     */
    thing_allocator<std::byte> thing_alloc() const { return m_thingAllocator; }
private:
    handle_container<mod_h>      m_modules;
    handle_container<thing_h>    m_things;
    handle_container<executor_h> m_executors;

    furlang::arena             m_thingArena;
    thing_allocator<std::byte> m_thingAllocator;
};

} // namespace furvm

#endif // FURVM_CONTEXT_HPP
