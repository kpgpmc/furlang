#ifndef FURLANG_ARENA_HPP
#define FURLANG_ARENA_HPP

#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>

namespace furlang {

/**
 * @brief An arena (region) allocator implementation.
 */
class arena {
private:
    struct region {
        using value_type = std::uintptr_t;

        static region* create(std::size_t capacity);

        region*     next;
        std::size_t capacity;
        std::size_t used;
        value_type  storage[];

        std::size_t free() const { return capacity - used; }
    };
public:
    /**
     * @brief Construct a new arena.
     *
     * @param minCapacity Minimal capacity of a single region in words.
     */
    arena(std::size_t minCapacity = 4 * 1024ULL);
    ~arena();

    /**
     * @brief Move constructor
     */
    arena(arena&& other) noexcept;

    arena(const arena&) = delete;

    /**
     * @brief Move constructor
     */
    arena& operator=(arena&& other) noexcept;

    arena& operator=(const arena&) = delete;
public:
    /**
     * @brief Allocates and default constructs objects.
     *
     * @tparam T Type of the objects.
     * @param count How many objects to allocate.
     * @return A pointer to the allocated objects.
     */
    template <typename T, typename = std::enable_if_t<std::is_default_constructible_v<T>>>
    T* allocate(std::size_t count) {
        T* allocated = reinterpret_cast<T*>(allocate(sizeof(T), count));
        for (std::size_t i = 0; i < count; ++i) {
            new (&allocated[i]) T();
        }
        return allocated;
    }

    /**
     * @brief Allocates and constructs an object.
     *
     * @tparam T Type of the object.
     * @param args Arguments passed to the object's constructor.
     * @return A pointer to the allocated object.
     */
    template <typename T, typename... Args, typename = std::enable_if_t<std::is_constructible_v<T, Args...>>>
    T* allocate(Args&&... args) {
        T* allocated = reinterpret_cast<T*>(allocate(sizeof(T), 1));
        new (allocated) T(std::forward<Args>(args)...);
        return allocated;
    }

    /**
     * @brief Allocates and constructs an object.
     *
     * @tparam T Type of the object.
     * @param args Arguments passed to the object's constructor.
     * @return A shared pointer to the allocated object.
     */
    template <typename T, typename... Args>
    std::shared_ptr<T> allocate_shared(Args&&... args) {
        T* allocated = allocate<T>(std::forward<Args>(args)...);
        return std::shared_ptr<T>(allocated, [](T* object) { object->~T(); });
    }

    /**
     * @brief Resets the arena.
     *
     * Resets occupied size of regions. Using previously allocated pointers after calling this function is
     * undefined-behaviour.
     */
    void reset();
private:
    void* allocate(std::size_t size, std::size_t count);
private:
    std::size_t m_minCapacity;
    region*     m_head = nullptr;
    region*     m_tail = nullptr;
};

template <typename T>
class arena_allocator {
    template <typename>
    friend class arena_allocator;
public:
    using value_type = T;
public:
    explicit arena_allocator(arena& arena) noexcept
      : m_arena(&arena) {}

    template <typename U>
    arena_allocator(const arena_allocator<U>& other) noexcept
      : m_arena(other.m_arena) {}

    template <typename U>
    arena_allocator& operator=(const arena_allocator<U>& other) noexcept {
        if (this == &other) return *this;
        m_arena = other.m_arena;
        return *this;
    }

    template <typename U>
    arena_allocator(arena_allocator<U>&& other) noexcept
      : m_arena(std::move(other.m_arena)) {}

    template <typename U>
    arena_allocator& operator=(arena_allocator<U>&& other) noexcept {
        if (this == &other) return *this;
        m_arena = std::move(other.m_arena);
        return *this;
    }
public:
    [[nodiscard]] T* allocate(std::size_t count = 1) { return m_arena->allocate<T>(count); }

    void deallocate(T* ptr, std::size_t count) noexcept {}
public:
    template <typename U>
    bool operator==(const arena_allocator<U>& other) const noexcept {
        return m_arena == other.m_arena;
    }

    template <typename U>
    bool operator!=(const arena_allocator<U>& other) const noexcept {
        return m_arena != other.m_arena;
    }
private:
    arena* m_arena;
};

} // namespace furlang

#endif // FURLANG_ARENA_HPP
