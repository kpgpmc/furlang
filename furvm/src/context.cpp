#include "furvm/context.hpp"

#include "furvm/thing.hpp" // IWYU pragma: keep
#include "furvm/type.hpp"

#include <cstdint>
#include <memory>

namespace furvm {

context::context()
  : m_thingAllocator(m_thingArena) {
    mod core;
    core.emplace_type(std::make_shared<type>(byteType));
    core.emplace_type(std::make_shared<type>(shortType));
    core.emplace_type(std::make_shared<type>(intType));
    core.emplace_type(std::make_shared<type>(longType));
    static_assert(sizeof(std::uintptr_t) == 8, "Unsupported platform");
    core.emplace_type(*core.type_at(3));

    emplace("core", std::move(core)).dispatch();
}

} // namespace furvm
