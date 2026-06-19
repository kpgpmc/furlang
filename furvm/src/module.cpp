#include "furvm/module.hpp"

#include "furvm/detail/serialization.hpp"
#include "furvm/fwd.hpp"

#include <ios>

namespace furvm {

std::ostream& mod::serialize(std::ostream& os) const {
    os << MAGIC;
    detail::serialize(os, std::uint32_t(0)); // version

    detail::serialize(os, function_id(m_functionMap.size()));
    for (const auto& [name, id] : m_functionMap) {
        detail::serialize(os, name);
        function_h func = m_functions.at(id);
        detail::serialize(os, std::uint8_t(func->type()));
        switch (func->type()) {
        case function_t::Normal: {
            detail::serialize(os, func->position());
        } break;
        case function_t::Native: break;
        case function_t::Import: {
            detail::serialize(os, func->imp().mod);
            detail::serialize(os, func->imp().function);
        } break;
        }
    }

    detail::serialize(os, std::uint64_t(m_bytecode.size()));
    return os.write(reinterpret_cast<const char*>(m_bytecode.data()), static_cast<std::streamsize>(m_bytecode.size()));
}
} // namespace furvm
