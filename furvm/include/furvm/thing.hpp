#ifndef FURVM_THING_HPP
#define FURVM_THING_HPP

#include "furvm/exceptions.hpp"
#include "furvm/fwd.hpp"
#include "furvm/module.hpp"
#include "furvm/type.hpp"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <functional>
#include <memory>
#include <new>
#include <stdexcept>
#include <utility>

namespace furvm {

template <template <typename> typename Allocator>
class thing final {
    friend class executor;
public:
    using allocator_type = Allocator<std::byte>; /**< Allocator type. */

    using mod_container = std::shared_ptr<handle_container<mod_h>>;
public:
    /**
     * @brief Constructs a thing.
     *
     * @param type Thing type reference.
     * @param allocator Allocator for the thing's data.
     */
    thing(const type_ref& type, const mod_container& modules = nullptr, const allocator_type& allocator = {})
      : m_type(type), m_size(compute_size(resolve_type(type, modules))), m_modules(modules), m_allocator(allocator) {
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
      : m_type(std::move(other.m_type)),
        m_data(other.m_data),
        m_size(other.m_size),
        m_modules(std::move(other.m_modules)),
        m_allocator(std::move(other.m_allocator)) {
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
        m_modules    = std::move(other.m_modules);
        m_allocator  = std::move(other.m_allocator);
        other.m_type = {};
        other.m_data = nullptr;
        other.m_size = 0;
        return *this;
    }

    thing(const thing&)            = delete;
    thing& operator=(const thing&) = delete;
public:
    void assign_mod_container(const mod_container& modules) { m_modules = modules; }
public:
    /**
     * @brief Returns a clone of the thing.
     *
     * @return A clone of this thing.
     */
    thing clone() const {
        auto type = resolve_type(m_type, m_modules);
        if (m_size == 0) {
            return reference();
        }

        thing res(type, m_modules, m_allocator);
        switch (type->t) {
        case type_t::Primitive:
        case type_t::Array: {
            copy_list(*type.type, res.get<array_t>(), get<array_t>());
        } break;
        case type_t::Import:
        default: throw std::runtime_error("unreachable");
        }
        return std::move(res);
    }
public:
    /**
     * @brief Returns the thing's type reference.
     *
     * @return The type reference.
     */
    auto type() { return m_type; }

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
        std::size_t size = m_size > 0 ? m_size : compute_size_na(resolve_type(m_type, m_modules));
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
        std::size_t size = m_size > 0 ? m_size : compute_size_na(resolve_type(m_type, m_modules));
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
    long_t integer() const {
        if (m_type->t != type_t::Primitive) throw bad_thing_access();
        switch (m_type->primitive) {
        case sizeof(byte_t): return get<byte_t>();
        case sizeof(short_t): return get<short_t>();
        case sizeof(int_t): return get<int_t>();
        case sizeof(long_t): return get<long_t>();
        default: throw std::runtime_error("unreachable");
        }
    }

    thing reference() const { return { m_type, m_data, m_modules, m_allocator }; }

    constexpr bool is_reference() const { return m_size == 0; }

    void resize(long_t newSize) {
        if (m_type->t != type_t::Array) throw bad_thing_access();
        if (m_type->array.size > 0) throw std::runtime_error("cannot resize a static array");

        auto& array = get<array_t>();
        if (newSize < 0 || newSize == array.dynamic.size) return;
        std::size_t innerSize = compute_size_na(*m_type->array.type);
        std::byte*  newData   = new std::byte[innerSize * newSize];
        std::memcpy(newData, array.dynamic.data, innerSize * std::min(array.dynamic.size, newSize));
        array.dynamic.size = newSize;
        delete[] array.dynamic.data;
        array.dynamic.data = newData;
    }

    thing at(long_t index) const {
        if (m_type->t != type_t::Array) throw bad_thing_access();

        std::size_t elementSize = compute_size_na(*m_type->array.type);
        if (m_type->array.size == 0) {
            auto& array = get<array_t>();
            if (index < 0 || index >= array.dynamic.size) throw std::out_of_range("index out of range");
            return { { m_type.mod, m_type->array.type },
                array.dynamic.data + (index * elementSize),
                m_modules,
                m_allocator };
        }

        std::byte* data = reinterpret_cast<array_t*>(m_data)->data;
        if (index < 0 || index >= m_type->array.size) throw std::out_of_range("index out of range");
        return { { m_type.mod, m_type->array.type }, data + (index * elementSize), m_modules, m_allocator };
    }

    std::size_t size() const {
        if (m_type->t != type_t::Array) throw std::runtime_error("not an array");
        if (m_type->array.size == 0) return get<array_t>().dynamic.size;
        return m_type->array.size;
    }
public:
    static type_ref resolve_type(const type_ref& initType, const mod_container& modules) {
        type_ref rsv = initType;
        while (rsv->t == type_t::Import) {
            auto mod = modules->at(initType->imp.mod);
            rsv      = type_ref{ mod, mod->type_at(initType->imp.type) };
        }
        return rsv;
    }
private:
    static void copy_list(const type_p& arrayType, array_t& dst, const array_t& src) {
        if (arrayType == nullptr || arrayType->t != type_t::Array || *arrayType->array.type == nullptr)
            throw std::runtime_error("invalid type");

        auto        innerType   = *arrayType->array.type;
        std::size_t elementSize = compute_size_na(innerType);

        std::byte*       data    = nullptr;
        const std::byte* srcData = nullptr;
        std::size_t      size    = 0;
        if (arrayType->array.size == 0) {
            size = dst.dynamic.size = src.dynamic.size;
            if (dst.dynamic.size < 0) {
                dst.dynamic.data = nullptr;
                return;
            }

            srcData = src.dynamic.data;
            data = dst.dynamic.data = new std::byte[dst.dynamic.size];
        } else {
            data    = dst.data;
            srcData = src.data;
            size    = arrayType->array.size;
        }

        switch (innerType->t) {
        case type_t::Primitive: std::memcpy(dst.data, src.data, size); break;
        case type_t::Array:
            for (std::size_t i = 0; i < size; ++i) {
                copy_list(*innerType->array.type,
                    *std::launder(reinterpret_cast<array_t*>(data + (i * elementSize))),
                    *std::launder(reinterpret_cast<const array_t*>(srcData + (i * elementSize))));
            }
            break;
        case type_t::Import: throw std::runtime_error("unresolved type");
        }
    }
private:
    static std::size_t compute_size_na(const type_p& type) {
        switch (type->t) {
        case type_t::Primitive: return type->primitive;
        case type_t::Array: {
            if (type->array.size == 0) return sizeof(array_t);
            return compute_size_na(*type->array.type) * type->array.size;
        }
        case type_t::Import: throw std::runtime_error("unresolved type");
        }

        throw std::runtime_error("unreachable");
    }

    static std::size_t compute_size_na(const type_ref& type) { return compute_size_na(*type.type); }

    // NOTE: Align to 4 bytes
    static std::size_t compute_size(const type_p& type) { return (compute_size_na(type) + 3) & ~3; }
    static std::size_t compute_size(const type_ref& type) { return compute_size(*type.type); }
private:
    thing(const type_ref& type, std::byte* data, const mod_container& modules, const allocator_type& allocator = {})
      : m_type(type), m_size(0), m_data(data), m_modules(modules), m_allocator(allocator) {}
private:
    template <typename Op>
    thing binary_op(const thing& rhs, const Op& op) const {
        if (m_type->t == type_t::Primitive && rhs.type()->t == type_t::Primitive) {
            type_ref    type = m_type->primitive >= rhs.type()->primitive ? m_type : rhs.type();
            std::size_t size = std::max(m_type->primitive, rhs.type()->primitive);

            long_t result = op(integer(), rhs.integer());

            thing res(type, m_modules, m_allocator);
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
    type_ref    m_type;
    std::size_t m_size;
    std::byte*  m_data;

    mod_container  m_modules;
    allocator_type m_allocator;
};

} // namespace furvm

#endif // FURVM_THING_HPP
