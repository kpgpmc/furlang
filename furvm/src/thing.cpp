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

thing::thing(rzecz, thing_handle id, thing_t type, const context_p& context)
  : m_id(id), m_type(type), m_context(context) {
    std::size_t size = thing_type_size(type);
    std::byte*  data = nullptr;
    if (!m_context->m_deadThingData.empty()) {
        thing_t itType{};
        for (auto it = m_context->m_deadThingData.rbegin(); it != m_context->m_deadThingData.rend(); ++it) {
            std::memcpy(&itType, static_cast<std::byte*>(*it) - sizeof(itType), sizeof(itType));
            if (size == thing_type_size(itType)) {
                data = static_cast<std::byte*>(*it);
                m_context->m_deadThingData.erase(std::next(it).base());
                break;
            }
        }
    }
    if (data == nullptr) data = m_context->m_thingArena.allocate<std::byte>(sizeof(type) + size);

    if (data == nullptr) throw std::runtime_error("failed to allocate data for new thing");
    std::memcpy(data, &type, sizeof(type));
    m_data = data + sizeof(type);

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

    m_context->m_deadThingData.push_back(m_data);
}

thing::thing(thing&& other) noexcept
  : m_id(other.m_id),
    m_type(other.m_type),
    m_context(std::move(other.m_context)),
    m_refCount(other.m_refCount),
    m_data(other.m_data) {
    other.m_id       = {};
    other.m_type     = {};
    other.m_refCount = {};
    other.m_data     = nullptr;
}

thing& thing::operator=(thing&& other) noexcept {
    if (this == &other) return *this;
    m_id       = other.m_id;
    m_type     = other.m_type;
    m_context  = std::move(other.m_context);
    m_refCount = other.m_refCount;
    m_data     = other.m_data;

    other.m_id       = {};
    other.m_type     = {};
    other.m_refCount = {};
    other.m_data     = nullptr;
    return *this;
}

thing_p thing::clone(const thing_p& thing) {
    thing_handle id = thing->m_context->m_things.size();
    if (!thing->m_context->m_deadThings.empty()) {
        id = thing->m_context->m_deadThings.front();
        thing->m_context->m_deadThings.pop();
        id += 1 << GENERATION_SIZE;
    }
    thing_handle idx = id & ((1ULL << ((sizeof(id) * 8) - GENERATION_SIZE)) - 1);

    auto th = std::make_shared<class thing>(rzecz{}, id, thing->m_type, thing->m_context);
    switch (thing->m_type) {
    // Primitives
    default: {
        memcpy(th->m_data, thing->m_data, thing_type_size(thing->m_type));
    }
    }

    thing->m_context->m_things.emplace(thing->m_context->m_things.begin() + idx, th);
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

} // namespace furvm

// NOLINTEND(cppcoreguidelines-no-malloc)