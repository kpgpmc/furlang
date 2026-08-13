#ifndef FURVM_CONTEXT_HPP
#define FURVM_CONTEXT_HPP

#include "furlang/arena.hpp"
#include "furvm/executor.hpp"
#include "furvm/fwd.hpp"
#include "furvm/handle.hpp"
#include "furvm/module.hpp" // IWYU pragma: keep
#include "furvm/thing.hpp"  // IWYU pragma: keep
#include "furvm/thing_allocator.hpp"

#include <cstddef>
#include <utility>
#include <vector>

namespace furvm {

class context : public handle_container<mod_h> {
public:
    friend class executor;
public:
    /**
     * @brief Constructs a context.
     */
    context()
      : m_thingAllocator(m_thingArena) {}

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
    auto& allocate_executor() {
        executor executor(this);
        return m_executors.emplace_back(std::move(executor));
    }

    /**
     * @brief Returns an executor from the context.
     *
     * @param args Id of the executor.
     * @return A handle to the executor.
     */
    template <typename... Args>
    auto& executor_at(Args&&... args) {
        return m_executors.at(std::forward<Args>(args)...);
    }

    /**
     * @brief Returns an executor from the context.
     *
     * @param args Id of the executor.
     * @return A handle to the executor.
     */
    template <typename... Args>
    const auto& executor_at(Args&&... args) const {
        return m_executors.at(std::forward<Args>(args)...);
    }

    const std::vector<executor>& executors() const { return m_executors; }
public:
    /**
     * @brief Returns context's thing allocator.
     *
     * @return The thing allocator.
     */
    thing_allocator<std::byte> thing_alloc() const { return m_thingAllocator; }

    thing_type_store& tt_store() { return m_thingTypeStore; }
private:
    handle_container<mod_h> m_modules;
    std::vector<executor>   m_executors;

    furlang::arena             m_thingArena;
    thing_allocator<std::byte> m_thingAllocator;
    class thing_type_store     m_thingTypeStore;
};

} // namespace furvm

#endif // FURVM_CONTEXT_HPP
