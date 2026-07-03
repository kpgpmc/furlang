#ifndef FURVM_THING_ALLOCATOR_HPP
#define FURVM_THING_ALLOCATOR_HPP

#include "furlang/arena.hpp"

#include <vector>

namespace furvm {

template <typename T>
class thing_allocator {
    template <typename>
    friend class thing_allocator;

    using dead_things = std::vector<std::pair<T*, std::size_t>>;
public:
    using value_type = T; /**< Value type. */
public:
    /**
     * @brief Constructs a thing allocator.
     *
     * @param arena Base arena allocator.
     */
    explicit thing_allocator(furlang::arena& arena) noexcept
      : m_arena(&arena), m_deadThings(std::make_shared<dead_things>()) {}

    /**
     * @brief Move constructor.
     */
    template <typename U>
    thing_allocator(thing_allocator<U>&& other) noexcept
      : m_arena(std::move(other.m_arena)), m_deadThings(std::move(other.m_deadThings)) {}

    /**
     * @brief Move constructor.
     */
    template <typename U>
    thing_allocator& operator=(thing_allocator<U>&& other) noexcept {
        if (this == &other) return *this;
        m_arena      = std::move(other.m_arena);
        m_deadThings = std::move(other.m_deadThings);
        return *this;
    }

    /**
     * @brief Copy constructor.
     */
    template <typename U>
    thing_allocator(const thing_allocator<U>& other) noexcept
      : m_arena(other.m_arena), m_deadThings(other.m_deadThings) {}

    /**
     * @brief Copy constructor.
     */
    template <typename U>
    thing_allocator& operator=(const thing_allocator<U>& other) noexcept {
        if (this == &other) return *this;
        m_arena      = other.m_arena;
        m_deadThings = other.m_deadThings;
        return *this;
    }
public:
    /**
     * @brief Returns a free chunk of memory.
     *
     * @param count Count of the things that must fit inside the chunk.
     * @return The chunk.
     */
    [[nodiscard]] T* allocate(std::size_t count = 1) {
        for (auto it = m_deadThings->begin(); it != m_deadThings->end(); ++it) {
            if (it->second != count) continue;
            m_deadThings->erase(it);
            return it->first;
        }
        return m_arena->allocate<T>(count);
    }

    /**
     * @brief Recycles the pointer.
     */
    void deallocate(T* ptr, std::size_t count) noexcept { m_deadThings->emplace_back(ptr, count); }
public:
    /**
     * @brief Compares two thing allocators for equality.
     *
     * @return true if the two things are equal.
     */
    template <typename U>
    bool operator==(const thing_allocator<U>& other) const noexcept {
        return m_arena == other.m_arena && m_deadThings == other.m_deadThings;
    }

    /**
     * @brief Compares two thing allocators for inequality.
     *
     * @return true if the two things are not equal.
     */
    template <typename U>
    bool operator!=(const thing_allocator<U>& other) const noexcept {
        return m_arena != other.m_arena || m_deadThings != other.m_deadThings;
    }
private:
    furlang::arena* m_arena;

    std::shared_ptr<dead_things> m_deadThings;
};

} // namespace furvm

#endif // FURVM_THING_ALLOCATOR_HPP
