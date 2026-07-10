#ifndef FURVM_THING_HPP
#define FURVM_THING_HPP

#include "furlang/arena.hpp"
#include "furlang/utility/hash.hpp"
#include "furvm/exceptions.hpp"
#include "furvm/fwd.hpp"
#include "furvm/thing_allocator.hpp" // IWYU pragma: keep

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <functional>
#include <limits>
#include <new>
#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace furvm {

struct thing_type {
    using primitive = std::uint64_t;

    struct array {
        thing_type* type;
        std::size_t size;
    };

    enum type { // NOLINT
        Primitive = 0,
        Array,

        Count,
    } type;
    union value {
        primitive primitive;
        array     array;

        value(std::uint64_t primitive)
          : primitive(primitive) {}

        value(thing_type* type, std::size_t size)
          : array({}) {
            array.type = type;
            array.size = size;
        }
    } value;

    static constexpr thing_type_id INVALID_ID = std::numeric_limits<thing_type_id>::max();

    thing_type_id id = INVALID_ID;

    bool operator==(const thing_type& other) const {
        if (type != other.type) return false;
        switch (type) {
        case Primitive: return value.primitive == other.value.primitive;
        case Array: return *value.array.type == *other.value.array.type && value.array.size == other.value.array.size;
        case Count: break;
        }
        return false;
    }

    bool operator!=(const thing_type& other) const { return !this->operator==(other); }
};

namespace detail {

struct thing_type_hash {
    std::size_t operator()(const thing_type& type) const {
        std::size_t seed = std::hash<decltype(type.type)>{}(type.type);
        switch (type.type) {
        case thing_type::Primitive:
            seed =
                furlang::utility::hash_combine(seed, std::hash<decltype(type.value.primitive)>{}(type.value.primitive));
            return seed;
        case thing_type::Array:
            seed = furlang::utility::hash_combine(seed, thing_type_hash{}(*type.value.array.type));
            seed = furlang::utility::hash_combine(seed,
                std::hash<decltype(type.value.array.size)>{}(type.value.array.size));
            return seed;
        case thing_type::Count: break;
        }
        throw std::runtime_error("unreachable");
    }
};

} // namespace detail

class thing_type_store {
public:
    thing_type* insert(thing_type& type) {
        if (auto it = m_typeMap.find(type); it != m_typeMap.end()) return m_map[type.id = it->second];
        if (type.id == thing_type::INVALID_ID) type.id = m_counter++;

        thing_type* ptr = m_arena.allocate<thing_type>(type);
        m_map[type.id]  = ptr;
        m_typeMap[type] = type.id;
        return ptr;
    }

    thing_type* at(thing_type_id id) const {
        if (auto it = m_map.find(id); it != m_map.end()) return it->second;
        return nullptr;
    }
private:
    furlang::arena                                                         m_arena;
    std::unordered_map<thing_type_id, thing_type*>                         m_map;
    std::unordered_map<thing_type, thing_type_id, detail::thing_type_hash> m_typeMap;
    thing_type_id                                                          m_counter = 0;
};

template <template <typename> typename Allocator>
class thing final {
    friend class executor;
public:
    using allocator_type = Allocator<std::byte>; /**< Allocator type. */
public:
    using s8  = std::int8_t;
    using s16 = std::int16_t;
    using s32 = std::int32_t;
    using s64 = std::int64_t;

    union array {
        std::byte flat[];
        struct {
            std::size_t size;
            std::byte*  data;
        } dynamic;
    };
public:
    /**
     * @brief Constructs a thing.
     *
     * @param type Thing type.
     * @param allocator Allocator for the thing's data.
     */
    thing(const thing_type& type, const allocator_type& allocator = {})
      : m_type(type), m_size(compute_size(type)), m_allocator(allocator) {
        // TODO: Account for alignment
        m_data = m_allocator.allocate(m_size);
        std::memset(m_data, 0, m_size);
    }

    /**
     * @brief Destructs a thing.
     */
    ~thing() {
        if (m_data != nullptr && m_size > 0) m_allocator.deallocate(m_data, m_size);
    }

    /**
     * @brief Move constructor.
     */
    thing(thing&& other) noexcept
      : m_type(other.m_type), m_data(other.m_data), m_size(other.m_size), m_allocator(std::move(other.m_allocator)) {
        other.m_type.type = thing_type::Count;
        other.m_data      = nullptr;
        other.m_size      = 0;
    }

    /**
     * @brief Move constructor.
     */
    thing& operator=(thing&& other) noexcept {
        if (this == &other) return *this;
        m_type            = other.m_type;
        m_data            = other.m_data;
        m_allocator       = std::move(other.m_allocator);
        other.m_type.type = thing_type::Count;
        other.m_data      = nullptr;
        other.m_size      = 0;
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
        if (m_size == 0) return reference();

        thing res(m_type, m_allocator);
        switch (m_type.type) {
        case thing_type::Primitive: std::memcpy(res.m_data, m_data, m_size); return std::move(res);
        case thing_type::Array: copy_list(m_type, res.get<array>(), get<array>()); return std::move(res);
        case thing_type::Count: break;
        }
        throw std::runtime_error("unreachable");
    }
public:
    /**
     * @brief Returns the thing's type reference.
     *
     * @return The type reference.
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
        std::size_t size = m_size > 0 ? m_size : compute_size_na(m_type);
        if (size != sizeof(T)) throw bad_thing_access();
        return *std::launder(reinterpret_cast<T*>(m_data));
    }

    /**
     * @brief Returns the thing's value.
     *
     * @return The value.
     */
    template <typename T>
    const T& get() const {
        std::size_t size = m_size > 0 ? m_size : compute_size_na(m_type);
        if (size != sizeof(T)) throw bad_thing_access();
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
    s64 integer() const {
        if (m_type.type != thing_type::Primitive) throw bad_thing_access();
        switch (m_type.value.primitive) {
        case sizeof(s8): return get<s8>();
        case sizeof(s16): return get<s16>();
        case sizeof(s32): return get<s32>();
        case sizeof(s64): return get<s64>();
        default: throw std::runtime_error("unreachable");
        }
    }

    thing reference() const { return { m_type, m_data, m_allocator }; }

    constexpr bool is_reference() const { return m_size == 0; }

    void resize(s64 newSize) {
        if (m_type.type != thing_type::Array) throw bad_thing_access();
        if (m_type.value.array.size > 0) throw std::runtime_error("cannot resize a static array");

        auto& array = get<union array>();
        if (newSize < 0 || newSize == array.dynamic.size) return;
        std::size_t innerSize = compute_size_na(*m_type.value.array.type);
        std::byte*  newData   = new std::byte[innerSize * newSize];
        std::memcpy(newData,
            array.dynamic.data,
            innerSize * std::min(static_cast<std::int64_t>(array.dynamic.size), newSize));
        array.dynamic.size = newSize;
        delete[] array.dynamic.data;
        array.dynamic.data = newData;
    }

    thing at(s64 index) const {
        if (m_type.type != thing_type::Array) throw bad_thing_access();

        std::size_t elementSize = compute_size_na(*m_type.value.array.type);
        if (m_type.value.array.size == 0) {
            auto& array = get<union array>();
            if (index < 0 || index >= array.dynamic.size) throw std::out_of_range("index out of range");
            return { *m_type.value.array.type, array.dynamic.data + (index * elementSize), m_allocator };
        }

        std::byte* data = reinterpret_cast<array*>(m_data)->flat;
        if (index < 0 || index >= m_type.value.array.size) throw std::out_of_range("index out of range");
        return { *m_type.value.array.type, data + (index * elementSize), m_allocator };
    }

    std::size_t size() const {
        if (m_type.type != thing_type::Array) throw bad_thing_access();
        return m_type.value.array.size == 0 ? get<array>().dynamic.size : m_type.value.array.size;
    }
private:
    static void copy_list(const thing_type& arrayType, array& dst, const array& src) {
        if (arrayType.type != thing_type::Array || arrayType.value.array.type == nullptr)
            throw std::runtime_error("invalid type");

        const auto& innerType   = *arrayType.value.array.type;
        std::size_t elementSize = compute_size_na(innerType);

        std::byte*       data    = nullptr;
        const std::byte* srcData = nullptr;
        std::size_t      size    = 0;
        if (arrayType.value.array.size == 0) {
            size = dst.dynamic.size = src.dynamic.size;
            if (dst.dynamic.size < 0) {
                dst.dynamic.data = nullptr;
                return;
            }

            srcData = src.dynamic.data;
            data = dst.dynamic.data = new std::byte[dst.dynamic.size];
        } else {
            data    = dst.flat;
            srcData = src.flat;
            size    = arrayType.value.array.size;
        }

        switch (innerType.type) {
        case thing_type::Primitive: std::memcpy(dst.flat, src.flat, size); return;
        case thing_type::Array:
            for (std::size_t i = 0; i < size; ++i) {
                copy_list(*innerType.value.array.type,
                    *std::launder(reinterpret_cast<array*>(data + (i * elementSize))),
                    *std::launder(reinterpret_cast<const array*>(srcData + (i * elementSize))));
            }
            return;
        case thing_type::Count: break;
        }
        throw std::runtime_error("unreachable");
    }
private:
    static std::size_t compute_size_na(const thing_type& type) {
        switch (type.type) {
        case thing_type::Primitive: return type.value.primitive;
        case thing_type::Array:
            return type.value.array.size == 0 ? sizeof(array)
                                              : compute_size_na(*type.value.array.type) * type.value.array.size;
        case thing_type::Count: break;
        }

        throw std::runtime_error("unreachable");
    }

    // NOTE: Align to 4 bytes
    static std::size_t compute_size(const thing_type& type) { return (compute_size_na(type) + 3) & ~3; }
private:
    thing(const thing_type& type, std::byte* data, const allocator_type& allocator = {})
      : m_type(type), m_size(0), m_data(data), m_allocator(allocator) {}
private:
    template <typename Op>
    thing binary_op(const thing& rhs, const Op& op) const {
        if (m_type.type == thing_type::Primitive && rhs.m_type.type == thing_type::Primitive) {
            thing_type  resultType = m_type.value.primitive >= rhs.m_type.value.primitive ? m_type : rhs.m_type;
            std::size_t size       = std::max(m_type.value.primitive, rhs.m_type.value.primitive);

            s64 result = op(integer(), rhs.integer());

            thing res(resultType, m_allocator);
            switch (size) {
            case sizeof(s8): res.get<s8>() = result; break;
            case sizeof(s16): res.get<s16>() = result; break;
            case sizeof(s32): res.get<s32>() = result; break;
            case sizeof(s64): res.get<s64>() = result; break;
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

} // namespace furvm

#endif // FURVM_THING_HPP
