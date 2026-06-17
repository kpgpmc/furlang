// NOLINTBEGIN(cppcoreguidelines-no-malloc)

#include "furvm/thing.hpp"

#include "furvm/context.hpp" // IWYU pragma: keep

#include <cstdlib>
#include <cstring>
#include <stdexcept>

namespace furvm {

std::size_t thing_type_size(thing_t type) {
    switch (type) {
    case thing_t::Int32: return 4;
    }
    return 0;
}

thing::thing(thing_t type)
  : m_type(type) {
    std::size_t size = thing_type_size(type);
    byte*       data = new byte[size];
    if (data == nullptr) throw std::runtime_error("failed to allocate data for new thing");
    std::memcpy(data, &type, sizeof(type));
    m_data = data + sizeof(type);

    switch (m_type) {
    // Primitives are zero-initialized.
    default:
    case thing_t::Int32: std::memset(m_data, 0, size);
    }
}

thing::thing(thing_t type, void* data)
  : m_type(type), m_ownData(false) {
    std::size_t size = thing_type_size(type);
    if (data == nullptr) throw std::runtime_error("failed to allocate data for new thing");
    std::memcpy(data, &type, sizeof(type));
    m_data = static_cast<char*>(data) + sizeof(type);

    switch (m_type) {
    // Primitives are zero-initialized.
    default:
    case thing_t::Int32: std::memset(m_data, 0, size);
    }
}

thing::~thing() {
    switch (m_type) {
    // Primitives are not destructed.
    default:
    case thing_t::Int32: break;
    }

    if (m_ownData) delete[] static_cast<char*>(m_data);
}

thing::thing(thing&& other) noexcept
  : m_type(other.m_type), m_refCount(other.m_refCount), m_data(other.m_data) {
    other.m_type     = {};
    other.m_ownData  = false;
    other.m_refCount = {};
    other.m_data     = nullptr;
}

thing& thing::operator=(thing&& other) noexcept {
    if (this == &other) return *this;
    m_type     = other.m_type;
    m_refCount = other.m_refCount;
    m_data     = other.m_data;

    other.m_type     = {};
    other.m_refCount = {};
    other.m_data     = nullptr;
    return *this;
}

static constexpr std::uint16_t thing_type_pair(thing_t lhs, thing_t rhs) {
    return (static_cast<std::uint16_t>(lhs) << 8) | static_cast<std::uint16_t>(rhs);
}

thing_h thing::clone(const context_p& context, const thing_h& thing) {
    auto th = context->emplace_thing((*thing)->m_type);
    switch ((*thing)->m_type) {
    // Primitives
    default: {
        memcpy((*th)->m_data, (*thing)->m_data, thing_type_size((*thing)->m_type));
    }
    }

    return std::move(th);
}

std::int32_t& thing::int32() {
    if (m_type != thing_t::Int32) throw bad_thing_access();
    return *static_cast<std::int32_t*>(m_data);
}

const std::int32_t& thing::int32() const {
    if (m_type != thing_t::Int32) throw bad_thing_access();
    return *static_cast<std::int32_t*>(m_data);
}

thing_h thing::add(const context_p& context, const thing_h& rhs) const {
    switch (thing_type_pair(m_type, (*rhs)->m_type)) {
    case thing_type_pair(thing_t::Int32, thing_t::Int32): {
        thing_h res     = context->emplace_thing(thing_t::Int32);
        (*res)->int32() = int32() + (*rhs)->int32();
        return res;
    }
    default: throw std::runtime_error("unreachable");
    }
}

thing_h thing::sub(const context_p& context, const thing_h& rhs) const {
    switch (thing_type_pair(m_type, (*rhs)->m_type)) {
    case thing_type_pair(thing_t::Int32, thing_t::Int32): {
        thing_h res     = context->emplace_thing(thing_t::Int32);
        (*res)->int32() = int32() - (*rhs)->int32();
        return res;
    }
    default: throw std::runtime_error("unreachable");
    }
}

thing_h thing::mul(const context_p& context, const thing_h& rhs) const {
    switch (thing_type_pair(m_type, (*rhs)->m_type)) {
    case thing_type_pair(thing_t::Int32, thing_t::Int32): {
        thing_h res     = context->emplace_thing(thing_t::Int32);
        (*res)->int32() = int32() * (*rhs)->int32();
        return res;
    }
    default: throw std::runtime_error("unreachable");
    }
}

thing_h thing::div(const context_p& context, const thing_h& rhs) const {
    switch (thing_type_pair(m_type, (*rhs)->m_type)) {
    case thing_type_pair(thing_t::Int32, thing_t::Int32): {
        thing_h res     = context->emplace_thing(thing_t::Int32);
        (*res)->int32() = int32() / (*rhs)->int32();
        return res;
    }
    default: throw std::runtime_error("unreachable");
    }
}

thing_h thing::mod(const context_p& context, const thing_h& rhs) const {
    switch (thing_type_pair(m_type, (*rhs)->m_type)) {
    case thing_type_pair(thing_t::Int32, thing_t::Int32): {
        thing_h res     = context->emplace_thing(thing_t::Int32);
        (*res)->int32() = int32() % (*rhs)->int32();
        return res;
    }
    default: throw std::runtime_error("unreachable");
    }
}

thing_h thing::equals(const context_p& context, const thing_h& rhs) const {
    switch (thing_type_pair(m_type, (*rhs)->m_type)) {
    case thing_type_pair(thing_t::Int32, thing_t::Int32): {
        thing_h res     = context->emplace_thing(thing_t::Int32);
        (*res)->int32() = static_cast<std::int32_t>(int32() == (*rhs)->int32());
        return res;
    }
    default: throw std::runtime_error("unreachable");
    }
}

thing_h thing::not_equals(const context_p& context, const thing_h& rhs) const {
    switch (thing_type_pair(m_type, (*rhs)->m_type)) {
    case thing_type_pair(thing_t::Int32, thing_t::Int32): {
        thing_h res     = context->emplace_thing(thing_t::Int32);
        (*res)->int32() = static_cast<std::int32_t>(int32() != (*rhs)->int32());
        return res;
    }
    default: throw std::runtime_error("unreachable");
    }
}

thing_h thing::less_than(const context_p& context, const thing_h& rhs) const {
    switch (thing_type_pair(m_type, (*rhs)->m_type)) {
    case thing_type_pair(thing_t::Int32, thing_t::Int32): {
        thing_h res     = context->emplace_thing(thing_t::Int32);
        (*res)->int32() = static_cast<std::int32_t>(int32() < (*rhs)->int32());
        return res;
    }
    default: throw std::runtime_error("unreachable");
    }
}

thing_h thing::greater_than(const context_p& context, const thing_h& rhs) const {
    switch (thing_type_pair(m_type, (*rhs)->m_type)) {
    case thing_type_pair(thing_t::Int32, thing_t::Int32): {
        thing_h res     = context->emplace_thing(thing_t::Int32);
        (*res)->int32() = static_cast<std::int32_t>(int32() > (*rhs)->int32());
        return res;
    }
    default: throw std::runtime_error("unreachable");
    }
}

thing_h thing::less_equals(const context_p& context, const thing_h& rhs) const {
    switch (thing_type_pair(m_type, (*rhs)->m_type)) {
    case thing_type_pair(thing_t::Int32, thing_t::Int32): {
        thing_h res     = context->emplace_thing(thing_t::Int32);
        (*res)->int32() = static_cast<std::int32_t>(int32() <= (*rhs)->int32());
        return res;
    }
    default: throw std::runtime_error("unreachable");
    }
}

thing_h thing::greater_equals(const context_p& context, const thing_h& rhs) const {
    switch (thing_type_pair(m_type, (*rhs)->m_type)) {
    case thing_type_pair(thing_t::Int32, thing_t::Int32): {
        thing_h res     = context->emplace_thing(thing_t::Int32);
        (*res)->int32() = static_cast<std::int32_t>(int32() >= (*rhs)->int32());
        return res;
    }
    default: throw std::runtime_error("unreachable");
    }
}

} // namespace furvm

// NOLINTEND(cppcoreguidelines-no-malloc)
