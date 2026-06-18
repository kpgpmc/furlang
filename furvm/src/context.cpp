#include "furvm/context.hpp"

#include "furvm/thing.hpp" // IWYU pragma: keep

namespace furvm {

context::context()
  : m_thingAllocator(m_thingArena) {}

void context::collect() {}

} // namespace furvm
