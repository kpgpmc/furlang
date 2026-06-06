// NOLINTBEGIN(cppcoreguidelines-no-malloc)

#include "furvm/thing.hpp"

#include "furvm/context.hpp" // IWYU pragma: keep

#include <cstdlib>

namespace furvm {

std::size_t thing_type_size(thing_t type) {
    switch (type) {
    case thing_t::Int: return 4;
    }
    return 0;
}

thing::thing(rzecz, thing_handle id, thing_t type, const context_p& context)
  : m_id(id), m_type(type), m_context(context), m_data(std::calloc(1, thing_type_size(type))) {}

thing::~thing() {
    std::free(m_data);
}

thing_p thing::create(const context_p& context, thing_t type) {
    auto th = std::make_shared<thing>(rzecz{}, context->m_things.size(), type, context);
    context->m_things.push_back(th);
    return std::move(th);
}

} // namespace furvm

// NOLINTEND(cppcoreguidelines-no-malloc)