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
    std::memset(m_data, 0, size);
}

thing::~thing() {
    m_context->m_deadThingData.push_back(m_data);
}

thing_p thing::create(const context_p& context, thing_t type) {
    thing_handle id = context->m_things.size();
    if (!context->m_deadThings.empty()) {
        id = context->m_deadThings.front();
        context->m_deadThings.pop();
        id += 1 << GENERATION_SIZE;
    }
    thing_handle idx = id & ((1ULL << ((sizeof(id) * 8) - GENERATION_SIZE)) - 1);

    auto th = std::make_shared<thing>(rzecz{}, id, type, context);
    context->m_things.emplace(context->m_things.begin() + idx, th);
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