#include "furvm/thing.hpp"

namespace furvm {

thing::thing(rzecz, thing_handle id, thing_t type, const context_p& context)
  : m_id(id), m_type(type), m_context(context) {}

thing::~thing() {}

} // namespace furvm