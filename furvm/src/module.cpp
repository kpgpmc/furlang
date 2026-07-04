#include "furvm/module.hpp"

#include "furvm/detail/serialization.hpp"
#include "furvm/fwd.hpp"
#include "furvm/type.hpp"

#include <ios>
#include <stdexcept>

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

    type_id typeCount = type_id(m_types.cend() - m_types.cbegin());
    detail::serialize(os, typeCount);
    for (type_id id = 0; id < typeCount; ++id) {
        if (!m_types.contains(id)) {
            detail::serialize(os, std::uint8_t(0xFF)); // null type
            continue;
        }

        auto type = *m_types.at(id);
        switch (type->t) {
        case type_t::Primitive: detail::serialize(os, type->primitive); break;
        case type_t::Reference: throw std::runtime_error("reference type serialization is unimplemented");
        case type_t::List: detail::serialize(os, type->list.id()); break;
        case type_t::Import: {
            detail::serialize(os, type->imp.mod);
            detail::serialize(os, type->imp.type);
        } break;
        }
    }

    detail::serialize(os, std::uint64_t(m_bytecode.size()));
    return os.write(reinterpret_cast<const char*>(m_bytecode.data()), static_cast<std::streamsize>(m_bytecode.size()));
}
} // namespace furvm
