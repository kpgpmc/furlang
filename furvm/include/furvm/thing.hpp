#ifndef FURVM_THING_HPP
#define FURVM_THING_HPP

#include "furlang/arena.hpp"
#include "furvm/exceptions.hpp"
#include "furvm/fwd.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <new>
#include <stdexcept>
#include <utility>
#include <vector>

namespace furvm {

enum class thing_type_t : std::uint32_t {
    Primitive = 0,
};

struct thing_type {
    thing_type_t  type;
    std::uint64_t value;
};

using byte_t  = std::int8_t;  /**< A 1-byte integer. */
using short_t = std::int16_t; /**< A 2-byte integer. */
using int_t   = std::int32_t; /**< A 4-byte integer. */
using long_t  = std::int64_t; /**< An 8-byte integer. */

/**
 * @brief A 1-byte integer thing type.
 */
static constexpr thing_type ByteType = { thing_type_t::Primitive, sizeof(byte_t) };

/**
 * @brief A 2-byte integer thing type.
 */
static constexpr thing_type ShortType = { thing_type_t::Primitive, sizeof(short_t) };

/**
 * @brief A 4-byte integer thing type.
 */
static constexpr thing_type IntType = { thing_type_t::Primitive, sizeof(int_t) };

/**
 * @brief An 8-byte integer thing type.
 */
static constexpr thing_type LongType = { thing_type_t::Primitive, sizeof(long_t) };

template <template <typename> typename Allocator>
class thing final {
    friend class executor;
public:
    using allocator_type = Allocator<std::byte>; /**< Allocator type. */
public:
    /**
     * @brief Constructs a thing.
     *
     * @param type Thing type.
     * @param allocator Allocator for the thing's data.
     */
    thing(const thing_type& type, const allocator_type& allocator = {})
      : m_type(type), m_allocator(allocator) {
        switch (type.type) {
        case thing_type_t::Primitive: {
            m_size = type.value;
        } break;
        }

        m_size = (m_size + 3) & ~3; // NOTE: Align to 4 bytes
        // TODO: Account for alignment
        m_data = m_allocator.allocate(m_size);
    }

    /**
     * @brief Destructs a thing.
     */
    ~thing() {
        if (m_data != nullptr) m_allocator.deallocate(m_data, m_size);
    }

    /**
     * @brief Move constructor.
     */
    thing(thing&& other) noexcept
      : m_type(other.m_type), m_data(other.m_data), m_size(other.m_size), m_allocator(std::move(other.m_allocator)) {
        other.m_type = {};
        other.m_data = nullptr;
        other.m_size = 0;
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
        other.m_size = 0;
        return *this;
    }

    thing(const thing&)            = delete;
    thing& operator=(const thing&) = delete;
public:
    /**
     * @brief Returns a clone of the thing.
     *
     * @return A clone of this thing.
     */
    thing clone() const {
        thing res(m_type, m_allocator);
        switch (m_type.type) {
        case thing_type_t::Primitive: {
            std::memcpy(res.m_data, m_data, m_size);
        } break;
        }
        return std::move(res);
    }
public:
    /**
     * @brief Returns the thing's type.
     *
     * @return The type.
     */
    constexpr auto type() const { return m_type; }
public:
    /**
     * @brief Returns a raw data pointer.
     *
     * @return The data pointer.
     */
    void* raw() { return m_data; }

    /**
     * @brief Returns a raw data pointer.
     *
     * @return The data pointer.
     */
    const void* raw() const { return m_data; }
public:
    /**
     * @brief Returns the thing's value.
     *
     * @return The value.
     */
    template <typename T>
    T& get() {
        if (m_type.type == thing_type_t::Primitive && m_type.value != sizeof(T)) throw bad_thing_access();
        return *std::launder(reinterpret_cast<T*>(m_data));
    }

    /**
     * @brief Returns the thing's value.
     *
     * @return The value.
     */
    template <typename T>
    const T& get() const {
        if (m_type.type == thing_type_t::Primitive && m_type.value != sizeof(T)) throw bad_thing_access();
        return *std::launder(reinterpret_cast<const T*>(m_data));
    }
public:
    /**
     * @brief Returns a sum of two things.
     *
     * @param rhs Thing to sum with this thing (right-hand-side).
     * @return The sum.
     */
    thing add(const thing& rhs) const { return binary_op(rhs, std::plus<>{}); }

    /**
     * @brief Returns a difference of two things.
     *
     * @param rhs Thing to subtract from this thing (right-hand-side).
     * @return The sum.
     */
    thing sub(const thing& rhs) const { return binary_op(rhs, std::minus<>{}); }

    /**
     * @brief Returns a product of two things.
     *
     * @param rhs Thing to multiply with this thing (right-hand-side).
     * @return The product.
     */
    thing mul(const thing& rhs) const { return binary_op(rhs, std::multiplies<>{}); }

    /**
     * @brief Returns a quotient of two things.
     *
     * @param rhs Thing to divide this thing by (right-hand-side).
     * @return The quotient.
     */
    thing div(const thing& rhs) const { return binary_op(rhs, std::divides<>{}); }

    /**
     * @brief Returns a remainder of two things.
     *
     * @param rhs Thing to divide this thing by (right-hand-side).
     * @return The remainder.
     */
    thing mod(const thing& rhs) const { return binary_op(rhs, std::modulus<>{}); }

    /**
     * @brief Compares two things for equality.
     *
     * @param rhs Thing to compare this thing with (right-hand-side).
     * @return A boolean result of the comparison in a thing form.
     */
    thing equals(const thing& rhs) const { return binary_op(rhs, std::equal_to<>{}); }

    /**
     * @brief Compares two things for inequality.
     *
     * @param rhs Thing to compare this thing with (right-hand-side).
     * @return A boolean result of the comparison in a thing form.
     */
    thing not_equals(const thing& rhs) const { return binary_op(rhs, std::not_equal_to<>{}); }

    /**
     * @brief Compares if this thing is less than an another thing.
     *
     * @param rhs The another thing.
     * @return A boolean result of the comparison in a thing form.
     */
    thing less_than(const thing& rhs) const { return binary_op(rhs, std::less<>{}); }

    /**
     * @brief Compares if this thing is greater than an another thing.
     *
     * @param rhs The another thing.
     * @return A boolean result of the comparison in a thing form.
     */
    thing greater_than(const thing& rhs) const { return binary_op(rhs, std::greater<>{}); }

    /**
     * @brief Compares if this thing is less than or equal to an another thing.
     *
     * @param rhs The another thing.
     * @return A boolean result of the comparison in a thing form.
     */
    thing less_equals(const thing& rhs) const { return binary_op(rhs, std::less_equal<>{}); }

    /**
     * @brief Compares if this thing is greater than or equal to an another thing.
     *
     * @param rhs The another thing.
     * @return A boolean result of the comparison in a thing form.
     */
    thing greater_equals(const thing& rhs) const { return binary_op(rhs, std::greater_equal<>{}); }
public:
    /**
     * @brief Returns a largest integer value of the thing.
     *
     * @return The integer value.
     */
    long_t integer() const {
        if (m_type.type != thing_type_t::Primitive) throw bad_thing_access();
        switch (m_type.value) {
        case sizeof(byte_t): return get<byte_t>();
        case sizeof(short_t): return get<short_t>();
        case sizeof(int_t): return get<int_t>();
        case sizeof(long_t): return get<long_t>();
        default: throw std::runtime_error("unreachable");
        }
    }
private:
    template <typename Op>
    thing binary_op(const thing& rhs, const Op& op) const {
        if (type().type == thing_type_t::Primitive && rhs.type().type == thing_type_t::Primitive) {
            std::size_t size = std::max(type().value, rhs.type().value);

            long_t result = op(integer(), rhs.integer());

            thing res(thing_type{ thing_type_t::Primitive, size }, m_allocator);
            switch (size) {
            case sizeof(byte_t): res.get<byte_t>() = result; break;
            case sizeof(short_t): res.get<short_t>() = result; break;
            case sizeof(int_t): res.get<int_t>() = result; break;
            case sizeof(long_t): res.get<long_t>() = result; break;
            default: throw std::runtime_error("unreachable");
            }
            return std::move(res);
        }

        throw std::runtime_error("unexpected operation");
    }
private:
    thing_type  m_type;
    std::size_t m_size;
    std::byte*  m_data;

    allocator_type m_allocator;
};

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

#endif // FURVM_THING_HPP
