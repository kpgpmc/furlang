#include "furvm/module.hpp"

#include "furvm/detail/serialization.hpp"
#include "furvm/fwd.hpp"

#include <ios>

namespace furvm {

std::ostream& mod::serialize(std::ostream& os) const {
    os << MAGIC;
    detail::serialize(os, std::uint32_t(0)); // version

    detail::serialize(os, function_id(m_functionMap.size()));
    for (function_id id = 0; id < m_functions.cend() - m_functions.cbegin(); ++id) {
        if (!m_functions.contains(id)) {
            detail::serialize(os, "");
            detail::serialize(os, std::uint8_t(0xFF)); // null function
            continue;
        }

        function_h func     = m_functions.at(id);
        bool       isPublic = m_publicFunctions.find(func->name()) != m_publicFunctions.end();
        detail::serialize(os, isPublic ? func->name() : ""); // private functions have empty names
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
