#include "furvm/context.hpp"

#include "furvm/thing.hpp" // IWYU pragma: keep

namespace furvm {

context::context() {}

void context::collect() {
    for (auto& ref : m_things) {
        if (ref->reference_count() != 0) continue;
        ref = nullptr;
    }
}

} // namespace furvm