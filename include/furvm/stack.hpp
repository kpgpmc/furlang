#ifndef FURVM_STACK_HPP
#define FURVM_STACK_HPP

#include <cstddef>
#include <new>
#include <stack>

namespace furvm {

template <typename T>
struct stack {
    stack(std::size_t capacity = (1024ULL * 1024ULL) / sizeof(T))
      : begin(new T[capacity]()), cursor(begin), capacity(capacity) {}

    T*             begin;
    T*             cursor;
    std::size_t    capacity;
    std::stack<T*> frames;

    void push_frame() { frames.push(cursor); }

    void pop_frame() {
        cursor = frames.top();
        frames.pop();
    }
};

template <typename T>
class stack_allocator {
public:
    stack_allocator() = default;

    stack_allocator(stack<T>& stack)
      : m_ref(&stack) {}

    template <typename U>
    constexpr stack_allocator(const stack_allocator<U>& other) noexcept
      : m_ref(other.m_ref) {}
public:
    T* allocate(std::size_t n) {
        if (m_ref == nullptr) throw std::bad_alloc();
        if (m_ref->capacity - (m_ref->cursor - m_ref->begin) < n) throw std::bad_alloc();

        T* ptr         = m_ref->cursor;
        m_ref->cursor += n;
        return ptr;
    }

    void deallocate(T* ptr, std::size_t n) {}
private:
    stack<T>* m_ref = nullptr;
};

} // namespace furvm

#endif // FURVM_STACK_HPP
