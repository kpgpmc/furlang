#ifndef FURLANG_ARENA_HPP
#define FURLANG_ARENA_HPP

#include <cstddef>
#include <cstdint>

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
    template <typename T>
    T* allocate(std::size_t count = 1) {
        return reinterpret_cast<T*>(allocate(sizeof(T), count));
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