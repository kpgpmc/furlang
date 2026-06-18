#ifndef FURVM_THING_HPP
#define FURVM_THING_HPP

#include "furlang/arena.hpp"
#include "furvm/fwd.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <utility>
#include <vector>

namespace furvm {

enum class thing_t : std::uint8_t {
    Int32,
};

/**
 * @brief Returns data size of a thing.
 *
 * @param type Type of the thing.
 * @return The data size of the thing.
 */
static inline std::size_t thing_type_size(thing_t type) {
    switch (type) {
    case thing_t::Int32: return sizeof(std::int32_t);
    default: return 0;
    }
}

class bad_thing_access : public std::exception {
public:
    bad_thing_access()           = default;
    ~bad_thing_access() override = default;

    /**
     * @brief Move constructor.
     */
    bad_thing_access(bad_thing_access&&) noexcept = default;

    /**
     * @brief Move constructor.
     */
    bad_thing_access& operator=(bad_thing_access&&) noexcept = default;

    /**
     * @brief Copy constructor.
     */
    bad_thing_access(const bad_thing_access&) = default;

    /**
     * @brief Copy constructor.
     */
    bad_thing_access& operator=(const bad_thing_access&) = default;
public:
    /**
     * @brief Returns a C-style string describing the cause of the error.
     *
     * @return The cause of the error.
     */
    const char* what() const noexcept override { return "bad thing access"; }
};

template <template <typename> typename Allocator>
class thing final {
    friend class executor;
public:
    using allocator_type = Allocator<std::byte>;
public:
    thing(thing_t type, const allocator_type& allocator = {})
      : m_type(type), m_allocator(allocator) {
        m_data = m_allocator.allocate(thing_type_size(type));
    }

    ~thing() { m_allocator.deallocate(m_data, thing_type_size(thing_t::Int32)); }

    /**
     * @brief Move constructor.
     */
    thing(thing&& other) noexcept
      : m_type(other.m_type), m_data(other.m_data), m_allocator(std::move(other.m_allocator)) {
        other.m_type = {};
        other.m_data = nullptr;
    }

    /**
     * @brief Move constructor.
     */
    thing& operator=(thing&& other) noexcept {
        if (this == &other) return *this;
        m_type       = other.m_type;
        m_data       = other.m_data;
        m_allocator  = std::move(other.m_allocator);
        other.m_type = {};
        other.m_data = nullptr;
        return *this;
    }

    thing(const thing&)            = delete;
    thing& operator=(const thing&) = delete;
public:
    /**
     * @brief Returns a clone of the thing.
     *
     * @param thing Thing to clone.
     * @return Shared pointer to a clone of the thing.
     */
    thing clone() const {
        class thing res(m_type, m_allocator);
        switch (m_type) {
        default: {
            std::memcpy(res.m_data, m_data, thing_type_size(m_type));
        } break;
        }
        return std::move(res);
    }
public:
    /**
     * @brief Returns an int32 value from this thing.
     *
     * @return The value.
     */
    std::int32_t& int32() {
        if (m_type != thing_t::Int32) throw bad_thing_access();
        return *reinterpret_cast<std::int32_t*>(m_data);
    }

    /**
     * @brief Returns an int32 value from this thing.
     *
     * @return The value.
     */
    const std::int32_t& int32() const {
        if (m_type != thing_t::Int32) throw bad_thing_access();
        return *reinterpret_cast<std::int32_t*>(m_data);
    }
public:
    thing add(const thing& rhs) const {
        thing res(m_type, m_allocator);
        res.int32() = int32() + rhs.int32();
        return res;
    }

    thing sub(const thing& rhs) const {
        thing res(m_type, m_allocator);
        res.int32() = int32() - rhs.int32();
        return res;
    }

    thing mul(const thing& rhs) const {
        thing res(m_type, m_allocator);
        res.int32() = int32() * rhs.int32();
        return res;
    }

    thing div(const thing& rhs) const {
        thing res(m_type, m_allocator);
        res.int32() = int32() / rhs.int32();
        return res;
    }

    thing mod(const thing& rhs) const {
        thing res(m_type, m_allocator);
        res.int32() = int32() % rhs.int32();
        return res;
    }

    thing equals(const thing& rhs) const {
        thing res(m_type, m_allocator);
        res.int32() = int32() == rhs.int32();
        return res;
    }

    thing not_equals(const thing& rhs) const {
        thing res(m_type, m_allocator);
        res.int32() = int32() != rhs.int32();
        return res;
    }

    thing less_than(const thing& rhs) const {
        thing res(m_type, m_allocator);
        res.int32() = int32() < rhs.int32();
        return res;
    }

    thing greater_than(const thing& rhs) const {
        thing res(m_type, m_allocator);
        res.int32() = int32() > rhs.int32();
        return res;
    }

    thing less_equals(const thing& rhs) const {
        thing res(m_type, m_allocator);
        res.int32() = int32() <= rhs.int32();
        return res;
    }

    thing greater_equals(const thing& rhs) const {
        thing res(m_type, m_allocator);
        res.int32() = int32() >= rhs.int32();
        return res;
    }
private:
    thing_t    m_type{};
    std::byte* m_data = nullptr;

    allocator_type m_allocator;
};

template <typename T>
class thing_allocator {
    template <typename>
    friend class thing_allocator;
public:
    using value_type = T;
public:
    explicit thing_allocator(furlang::arena& arena) noexcept
      : m_arena(&arena) {}

    template <typename U>
    thing_allocator(const thing_allocator<U>& other) noexcept
      : m_arena(other.m_arena), m_deadThings(other.m_deadThings) {}

    template <typename U>
    thing_allocator& operator=(const thing_allocator<U>& other) noexcept {
        if (this == &other) return *this;
        m_arena      = other.m_arena;
        m_deadThings = other.m_deadThings;
        return *this;
    }

    template <typename U>
    thing_allocator(thing_allocator<U>&& other) noexcept
      : m_arena(std::move(other.m_arena)) {}

    template <typename U>
    thing_allocator& operator=(thing_allocator<U>&& other) noexcept {
        if (this == &other) return *this;
        m_arena = std::move(other.m_arena);
        return *this;
    }
public:
    [[nodiscard]] T* allocate(std::size_t count = 1) { return m_arena->allocate<T>(count); }

    void deallocate(T* ptr, std::size_t count) noexcept { m_deadThings.emplace_back(ptr, count); }
public:
    template <typename U>
    bool operator==(const thing_allocator<U>& other) const noexcept {
        return m_arena == other.m_arena && m_deadThings == other.m_deadThings;
    }

    template <typename U>
    bool operator!=(const thing_allocator<U>& other) const noexcept {
        return m_arena != other.m_arena || m_deadThings != other.m_deadThings;
    }
private:
    furlang::arena* m_arena;

    std::vector<std::pair<T*, std::size_t>> m_deadThings;
};

} // namespace furvm

#endif // FURVM_THING_HPP
