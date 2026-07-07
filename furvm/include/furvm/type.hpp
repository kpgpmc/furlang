#ifndef FURVM_TYPE_HPP
#define FURVM_TYPE_HPP

#include "furvm/fwd.hpp"
#include "furvm/handle.hpp" // IWYU pragma: keep

#include <cstdint>

namespace furvm {

enum class type_t : std::uint32_t {
    Primitive = 0,
    Array,

    Import,
};

using primitive_type = std::uint64_t;

/**
 * @brief Array type.
 */
struct array_type {
    type_h type; /**< Type of the array's elements. */

    /**
     * @brief Size of the array.
     *
     * Size of the array. If size is equal to zero, then the array becomes dynamic.
     */
    std::size_t size;
};

struct import_type {
    mod_id  mod;
    type_id type;
};

struct type {
    type_t t;
    union {
        primitive_type primitive{};
        array_type     array;
        import_type    imp;
    };

    type(type_t type)
      : t(type) {}

    type(primitive_type primitive)
      : t(type_t::Primitive), primitive(primitive) {}

    type(const type_h& type, std::size_t size = 0)
      : t(type_t::Array), array(array_type{ type, size }) {}

    type(const array_type& list)
      : t(type_t::Array), array(list) {}

    type(const import_type& imp)
      : t(type_t::Import), imp(imp) {}

    ~type() {
        switch (t) {
        case type_t::Array: array.~array_type(); break;
        case type_t::Import: imp.~import_type(); break;
        default: break;
        }
    }

    type(type&& other) noexcept
      : t(other.t) {
        switch (t) {
        case type_t::Primitive: primitive = other.primitive; break;
        case type_t::Array: array = std::move(other.array); break;
        case type_t::Import: imp = std::move(other.imp); break;
        }
    }

    type& operator=(type&& other) noexcept {
        if (this == &other) return *this;
        t = other.t;
        switch (t) {
        case type_t::Primitive: primitive = other.primitive; break;
        case type_t::Array: array = std::move(other.array); break;
        case type_t::Import: imp = std::move(other.imp); break;
        }

        return *this;
    }

    type(const type& other)
      : t(other.t) {
        switch (t) {
        case type_t::Primitive: primitive = other.primitive; break;
        case type_t::Array: array = other.array; break;
        case type_t::Import: imp = other.imp; break;
        }
    }

    type& operator=(const type& other) {
        if (this == &other) return *this;
        t = other.t;
        switch (t) {
        case type_t::Primitive: primitive = other.primitive; break;
        case type_t::Array: array = other.array; break;
        case type_t::Import: imp = other.imp; break;
        }

        return *this;
    }
};

using byte_t  = std::int8_t;  /**< A 1-byte integer. */
using short_t = std::int16_t; /**< A 2-byte integer. */
using int_t   = std::int32_t; /**< A 4-byte integer. */
using long_t  = std::int64_t; /**< An 8-byte integer. */

/**
 * @brief Array type's data layout.
 */
union array_t {
    std::byte data[]; /**< Static array's elements' data. */
    struct {
        long_t     size; /**< Size of the array (in items). */
        std::byte* data; /**< Pointer to dynamic array's elements' data array. */
    } dynamic;           /**< Dynamic array's info. */
};

/**
 * @brief A 1-byte integer thing type.
 */
inline static type byteType = { sizeof(byte_t) }; // NOLINT

/**
 * @brief A 2-byte integer thing type.
 */
inline static type shortType = { sizeof(short_t) }; // NOLINT

/**
 * @brief A 4-byte integer thing type.
 */
inline static type intType = { sizeof(int_t) }; // NOLINT

/**
 * @brief An 8-byte integer thing type.
 */
inline static type longType = { sizeof(long_t) }; // NOLINT

/**
 * @brief Reference to a type.
 */
struct type_ref {
    mod_h  mod;
    type_h type;

    class type*       operator->() { return &**type; }
    const class type* operator->() const { return &**type; }
};

} // namespace furvm

#endif // FURVM_TYPE_HPP
