#ifndef FURVM_TYPE_HPP
#define FURVM_TYPE_HPP

#include "furvm/fwd.hpp"

#include <cstdint>
#include <memory>
#include <utility>

namespace furvm {

enum class type_t : std::uint32_t {
    Primitive = 0,
    Reference,
    List,

    Import,
};

using primitive_type = std::uint64_t;
using reference_type = std::shared_ptr<type>;
using list_type      = std::shared_ptr<type>;

struct import_type {
    mod_id  mod;
    type_id type;
};

struct type {
    type_t t;
    union {
        primitive_type primitive;
        reference_type reference;
        list_type      list;
        import_type    imp;
    };

    type(type_t type)
      : t(type), primitive(0) {}

    type(primitive_type primitive)
      : t(type_t::Primitive), primitive(primitive) {}

    type(const reference_type& reference)
      : t(type_t::Reference), reference(reference) {}

    static type make_list(const list_type& list) {
        type type(type_t::List);
        new (&type.list) list_type(list);
        return type;
    }

    type(const import_type& imp)
      : t(type_t::Import), imp(imp) {}

    ~type() {
        switch (t) {
        case type_t::Reference: reference.~reference_type(); break;
        case type_t::List: list.~list_type(); break;
        case type_t::Import: imp.~import_type(); break;
        default: break;
        }
    }

    type(type&& other) noexcept
      : t(other.t) {
        switch (t) {
        case type_t::Primitive: primitive = other.primitive; break;
        case type_t::Reference: reference = std::move(other.reference); break;
        case type_t::List: list = std::move(other.list); break;
        case type_t::Import: imp = std::move(other.imp); break;
        }
    }

    type& operator=(type&& other) noexcept {
        if (this == &other) return *this;
        t = other.t;
        switch (t) {
        case type_t::Primitive: primitive = other.primitive; break;
        case type_t::Reference: reference = std::move(other.reference); break;
        case type_t::List: list = std::move(other.list); break;
        case type_t::Import: imp = std::move(other.imp); break;
        }

        return *this;
    }

    type(const type& other)
      : t(other.t) {
        switch (t) {
        case type_t::Primitive: primitive = other.primitive; break;
        case type_t::Reference: reference = other.reference; break;
        case type_t::List: list = other.list; break;
        case type_t::Import: imp = other.imp; break;
        }
    }

    type& operator=(const type& other) {
        if (this == &other) return *this;
        t = other.t;
        switch (t) {
        case type_t::Primitive: primitive = other.primitive; break;
        case type_t::Reference: reference = other.reference; break;
        case type_t::List: list = other.list; break;
        case type_t::Import: imp = other.imp; break;
        }

        return *this;
    }
};

using byte_t  = std::int8_t;  /**< A 1-byte integer. */
using short_t = std::int16_t; /**< A 2-byte integer. */
using int_t   = std::int32_t; /**< A 4-byte integer. */
using long_t  = std::int64_t; /**< An 8-byte integer. */

using reference_t = std::byte*;

struct list_t {
    long_t     size;
    std::byte* data;
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

} // namespace furvm

#endif // FURVM_TYPE_HPP
