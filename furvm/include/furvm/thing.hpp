#ifndef FURVM_THING_HPP
#define FURVM_THING_HPP

#include "furlang/arena.hpp"
#include "furvm/exceptions.hpp"
#include "furvm/fwd.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <utility>
#include <vector>

namespace furvm {

enum class thing_t : std::uint8_t {
    Int32,
};

/**
 * @brief Returns how many bytes a thing would take up.
 *
 * @param type Type of the thing.
 * @return The byte count.
 */
static inline std::size_t thing_type_size(thing_t type) {
    switch (type) {
    case thing_t::Int32: return sizeof(std::int32_t);
    default: return 0;
    }
}

template <template <typename> typename Allocator>
class thing final {
    friend class executor;
public:
    using allocator_type = Allocator<std::byte>; /**< Allocator type. */
public:
    /**
     * @brief Constructs a thing.
     *
     * @param type Type of the thing.
     * @param allocator Allocator for the thing's data.
     */
    thing(thing_t type, const allocator_type& allocator = {})
      : m_type(type), m_allocator(allocator) {
        m_data = m_allocator.allocate(thing_type_size(type));
    }

    /**
     * @brief Destructs a thing.
     */
    ~thing() {
        if (m_data != nullptr) m_allocator.deallocate(m_data, thing_type_size(thing_t::Int32));
    }

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
     * @return A clone of this thing.
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
     * @brief Returns the thing's type.
     *
     * @return The type.
     */
    constexpr auto type() const { return m_type; }
public:
    /**
     * @brief Returns the thing's int32 value.
     *
     * @return The value.
     */
    std::int32_t& int32() {
        if (m_type != thing_t::Int32) throw bad_thing_access();
        return *reinterpret_cast<std::int32_t*>(m_data);
    }

    /**
     * @brief Returns the thing's int32 value.
     *
     * @return The value.
     */
    const std::int32_t& int32() const {
        if (m_type != thing_t::Int32) throw bad_thing_access();
        return *reinterpret_cast<std::int32_t*>(m_data);
    }
public:
    /**
     * @brief Returns a sum of two things.
     *
     * @param rhs Thing to sum with this thing (right-hand-side).
     * @return The sum.
     */
    thing add(const thing& rhs) const {
        thing res(m_type, m_allocator);
        res.int32() = int32() + rhs.int32();
        return res;
    }

    /**
     * @brief Returns a difference of two things.
     *
     * @param rhs Thing to subtract from this thing (right-hand-side).
     * @return The sum.
     */
    thing sub(const thing& rhs) const {
        thing res(m_type, m_allocator);
        res.int32() = int32() - rhs.int32();
        return res;
    }

    /**
     * @brief Returns a product of two things.
     *
     * @param rhs Thing to multiply with this thing (right-hand-side).
     * @return The product.
     */
    thing mul(const thing& rhs) const {
        thing res(m_type, m_allocator);
        res.int32() = int32() * rhs.int32();
        return res;
    }

    /**
     * @brief Returns a quotient of two things.
     *
     * @param rhs Thing to divide this thing by (right-hand-side).
     * @return The quotient.
     */
    thing div(const thing& rhs) const {
        thing res(m_type, m_allocator);
        res.int32() = int32() / rhs.int32();
        return res;
    }

    /**
     * @brief Returns a remainder of two things.
     *
     * @param rhs Thing to divide this thing by (right-hand-side).
     * @return The remainder.
     */
    thing mod(const thing& rhs) const {
        thing res(m_type, m_allocator);
        res.int32() = int32() % rhs.int32();
        return res;
    }

    /**
     * @brief Compares two things for equality.
     *
     * @param rhs Thing to compare this thing with (right-hand-side).
     * @return A boolean result of the comparison in a thing form.
     */
    thing equals(const thing& rhs) const {
        thing res(m_type, m_allocator);
        res.int32() = int32() == rhs.int32();
        return res;
    }

    /**
     * @brief Compares two things for inequality.
     *
     * @param rhs Thing to compare this thing with (right-hand-side).
     * @return A boolean result of the comparison in a thing form.
     */
    thing not_equals(const thing& rhs) const {
        thing res(m_type, m_allocator);
        res.int32() = int32() != rhs.int32();
        return res;
    }

    /**
     * @brief Compares if this thing is less than an another thing.
     *
     * @param rhs The another thing.
     * @return A boolean result of the comparison in a thing form.
     */
    thing less_than(const thing& rhs) const {
        thing res(m_type, m_allocator);
        res.int32() = int32() < rhs.int32();
        return res;
    }

    /**
     * @brief Compares if this thing is greater than an another thing.
     *
     * @param rhs The another thing.
     * @return A boolean result of the comparison in a thing form.
     */
    thing greater_than(const thing& rhs) const {
        thing res(m_type, m_allocator);
        res.int32() = int32() > rhs.int32();
        return res;
    }

    /**
     * @brief Compares if this thing is less than or equal to an another thing.
     *
     * @param rhs The another thing.
     * @return A boolean result of the comparison in a thing form.
     */
    thing less_equals(const thing& rhs) const {
        thing res(m_type, m_allocator);
        res.int32() = int32() <= rhs.int32();
        return res;
    }

    /**
     * @brief Compares if this thing is greater than or equal to an another thing.
     *
     * @param rhs The another thing.
     * @return A boolean result of the comparison in a thing form.
     */
    thing greater_equals(const thing& rhs) const {
        thing res(m_type, m_allocator);
        res.int32() = int32() >= rhs.int32();
        return res;
    }
private:
    thing_t    m_type{};
    std::byte* m_data;

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
