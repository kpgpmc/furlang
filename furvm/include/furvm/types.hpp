#ifndef FURVM_TYPES_HPP
#define FURVM_TYPES_HPP

#include "furvm/fwd.hpp"

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace furvm {

using s8  = std::int8_t;
using s16 = std::int16_t;
using s32 = std::int32_t;
using s64 = std::int64_t;
using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

struct thing_type {
    struct array_value {
        thing_type* type;
        std::size_t size;
    };

    struct slice_value {
        thing_type* type;
    };

    enum type { // NOLINT
        S8 = 0,
        S16,
        S32,
        S64,
        U8,
        U16,
        U32,
        U64,
        Ptr,
        Ref,
        Array,
        Slice,

        Count,
    } type = Count;
    union value {
        std::nullptr_t null = nullptr;
        thing_type*    typeRef;
        array_value    array;
        slice_value    slice;

        value() = default;

        value(thing_type* type)
          : typeRef(type) {}

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
        case S8:
        case S16:
        case S32:
        case S64:
        case U8:
        case U16:
        case U32:
        case U64: return true;
        case Ptr:
        case Ref: return *value.typeRef == *other.value.typeRef;
        case Array: return *value.array.type == *other.value.array.type && value.array.size == other.value.array.size;
        case Slice: return *value.slice.type == *other.value.slice.type;
        case Count: break;
        }
        return false;
    }

    bool operator!=(const thing_type& other) const { return !this->operator==(other); }

    static bool is_primitive(enum type type) {
        switch (type) {
        case S8:
        case S16:
        case S32:
        case S64:
        case U8:
        case U16:
        case U32:
        case U64: return true;
        case Ptr:
        case Ref:
        case Array:
        case Slice: return false;
        case Count: break;
        }
        throw std::runtime_error("unreachable");
    }

    static std::size_t primitive_size(enum type type) {
        switch (type) {
        case thing_type::S8: return sizeof(s8);
        case thing_type::S16: return sizeof(s16);
        case thing_type::S32: return sizeof(s32);
        case thing_type::S64: return sizeof(s64);
        case thing_type::U8: return sizeof(u8);
        case thing_type::U16: return sizeof(u16);
        case thing_type::U32: return sizeof(u32);
        case thing_type::U64: return sizeof(u64);
        case Ptr:
        case Ref:
        case Array:
        case Slice: return 0;
        case Count: break;
        }
        throw std::runtime_error("unreachable");
    }
};

namespace detail {

template <typename T, typename = void>
struct overrides_thing_type_matching : std::false_type {};

template <typename T>
struct overrides_thing_type_matching<T, std::void_t<decltype(T::matches(std::declval<const thing_type&>()))>>
  : std::is_same<decltype(T::matches(std::declval<const thing_type&>())), bool> {};

template <typename T>
struct thing_traits {
    bool operator()(const thing_type& type) const {
        if constexpr (overrides_thing_type_matching<T>::value) {
            return T::matches(type);
        } else {
            return false;
        }
    }
};

template <>
struct thing_traits<s8> {
    bool operator()(const thing_type& type) const { return type.type == thing_type::S8; }
};

template <>
struct thing_traits<u8> {
    bool operator()(const thing_type& type) const { return type.type == thing_type::U8; }
};

template <>
struct thing_traits<s16> {
    bool operator()(const thing_type& type) const { return type.type == thing_type::S16; }
};

template <>
struct thing_traits<u16> {
    bool operator()(const thing_type& type) const { return type.type == thing_type::U16; }
};

template <>
struct thing_traits<s32> {
    bool operator()(const thing_type& type) const { return type.type == thing_type::S32; }
};

template <>
struct thing_traits<u32> {
    bool operator()(const thing_type& type) const { return type.type == thing_type::U32; }
};

template <>
struct thing_traits<s64> {
    bool operator()(const thing_type& type) const { return type.type == thing_type::S64; }
};

template <>
struct thing_traits<u64> {
    bool operator()(const thing_type& type) const { return type.type == thing_type::U64; }
};

template <typename Inner>
struct thing_traits<Inner*> {
    bool operator()(const thing_type& type) const {
        return (type.type == thing_type::Ptr || type.type == thing_type::Ref) &&
               thing_traits<Inner>{}(*type.value.typeRef);
    }
};

} // namespace detail

} // namespace furvm

#endif // FURVM_TYPES_HPP
