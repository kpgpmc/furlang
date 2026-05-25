#ifndef FURLANG_ARENA_HPP
#define FURLANG_ARENA_HPP

#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>

namespace furlang {

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
    arena(std::size_t minCapacity = 4 * 1024ULL);
    ~arena();

    arena(arena&& other) noexcept;
    arena(const arena&) = delete;
    arena& operator=(arena&& other) noexcept;
    arena& operator=(const arena&) = delete;
public:
    template <typename T, typename = std::enable_if_t<std::is_default_constructible_v<T>>>
    T* allocate(std::size_t count) {
        T* allocated = reinterpret_cast<T*>(allocate(sizeof(T), count));
        for (std::size_t i = 0; i < count; ++i) {
            new (&allocated[i]) T();
        }
        return allocated;
    }

    template <typename T, typename... Args, typename = std::enable_if_t<std::is_constructible_v<T, Args...>>>
    T* allocate(Args&&... args) {
        T* allocated = reinterpret_cast<T*>(allocate(sizeof(T), 1));
        new (allocated) T(std::forward<Args>(args)...);
        return allocated;
    }

    template <typename T, typename... Args>
    std::shared_ptr<T> allocate_shared(Args&&... args) {
        T* allocated = allocate<T>(std::forward<Args>(args)...);
        return std::shared_ptr<T>(allocated, [](T* object) { object->~T(); });
    }

    void reset();
private:
    void* allocate(std::size_t size, std::size_t count);
private:
    std::size_t m_minCapacity;
    region*     m_head = nullptr;
    region*     m_tail = nullptr;
};

} // namespace furlang

#endif // FURLANG_ARENA_HPP