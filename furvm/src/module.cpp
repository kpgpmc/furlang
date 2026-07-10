#include "furvm/module.hpp"

#include "furvm/detail/serialization.hpp"
#include "furvm/function.hpp"
#include "furvm/fwd.hpp"

#include <cstdint>
#include <cstring>
#include <ios>
#include <memory>
#include <stdexcept>
#include <utility>

namespace furvm {

std::ostream& mod::serialize(std::ostream& os) const {
    os.write(MAGIC, sizeof(MAGIC));
    detail::serialize(os, std::uint32_t(0)); // version

    mod_type_id typeCount = mod_type_id(m_types.cend() - m_types.cbegin());
    detail::serialize(os, typeCount);
    for (mod_type_id id = 0; id < typeCount; ++id) {
        if (!m_types.contains(id)) {
            detail::serialize(os, std::uint8_t(0xFF)); // null type
            continue;
        }

        auto type = *m_types.at(id);
        switch (type.type) {
        case mod_type::Primitive: detail::serialize(os, type.value.primitive); break;
        case mod_type::Array: {
            detail::serialize(os, type.value.array.typeId);
            detail::serialize(os, type.value.array.size);
        } break;
        case mod_type::Import: {
            detail::serialize(os, type.value.imprt.modId);
            detail::serialize(os, type.value.imprt.typeId);
        } break;
        case mod_type::Count: throw std::runtime_error("unreachable");
        }
    }

    function_id funcCount = m_functions.cend() - m_functions.cbegin();
    detail::serialize(os, funcCount);
    for (function_id id = 0; id < funcCount; ++id) {
        if (!m_functions.contains(id)) {
            detail::serialize(os, "");
            detail::serialize(os, 0);
            detail::serialize(os, std::uint8_t(0xFF)); // null function
            continue;
        }

        // Function name
        function_h func = m_functions.at(id);
        if (auto it = m_functionMap.find(func.id()); it != m_functionMap.end()) {
            detail::serialize(os, it->second.first);
        } else {
            detail::serialize(os, "");
        }

        // Function signature
        detail::serialize(os, func->signature().params.size());
        for (std::uint32_t i = 0; i < func->signature().params.size(); ++i)
            detail::serialize(os, func->signature().params[i].id());

        // Function body/value
        detail::serialize(os, std::uint8_t(func->type()));
        switch (func->type()) {
        case function_t::Normal: {
            detail::serialize(os, func->position());
        } break;
        case function_t::Native: {
            detail::serialize(os, func->native());
        } break;
        case function_t::Import: {
            detail::serialize(os, func->imp().mod);
            detail::serialize(os, func->imp().function);
        } break;
        }
    }

    detail::serialize(os, std::uint64_t(m_bytecode.size()));
    return os.write(reinterpret_cast<const char*>(m_bytecode.data()), static_cast<std::streamsize>(m_bytecode.size()));
}

mod mod::load(std::istream& is) {
    std::uint32_t magic = 0;
    detail::load(is, magic);
    std::uint32_t version = 0;
    detail::load(is, version);
    if (std::memcmp(&magic, MAGIC, sizeof(MAGIC)) != 0) throw std::runtime_error("invalid magic");
    if (version != 0) throw std::runtime_error("unsupported version");

    mod mod;

    mod_type_id typeCount = 0;
    detail::load(is, typeCount);
    for (mod_type_id id = 0; id < typeCount; ++id) {
        std::uint8_t type = 0;
        detail::load(is, type);
        if (type == 0xFF) continue;

        switch ((enum mod_type::type)type) {
        case mod_type::Primitive: {
            mod_type::primitive primitive = 0;
            detail::load(is, primitive);
            mod.emplace_type(id, primitive).dispatch();
        } break;
        case mod_type::Array: {
            mod_type_id typeId = 0;
            detail::load(is, typeId);
            std::size_t size = 0;
            detail::load(is, size);
            mod.emplace_type(id, typeId, size).dispatch();
        } break;
        case mod_type::Import: {
            std::string modName;
            detail::load(is, modName);
            mod_type_id typeId = 0;
            detail::load(is, typeId);
            mod.emplace_type(id, std::move(modName), typeId).dispatch();
        } break;
        default: throw std::runtime_error("unknown type type");
        }
    }

    function_id functionCount = 0;
    detail::load(is, functionCount);
    for (function_id id = 0; id < functionCount; ++id) {
        std::string name;
        detail::load(is, name);

        function_sig  signature;
        std::uint32_t paramCount = 0;
        detail::load(is, paramCount);
        signature.params.reserve(paramCount);
        while (signature.params.size() < paramCount) {
            thing_id param{ 0 };
            detail::load(is, param);
            signature.params.emplace_back(mod.type_at(param));
        }

        std::uint8_t type = 0;
        detail::load(is, type);
        if (type == 0xFF) continue;

        switch ((function_t)type) {
        case function_t::Normal: {
            decltype(std::declval<function>().position()) offset = 0;
            detail::load(is, offset);
            if (name.empty())
                mod.emplace_function(std::move(name), id, std::move(signature), offset).dispatch();
            else
                mod.emplace_function(id, std::move(signature), offset).dispatch();
        } break;
        case function_t::Native: {
            std::string native;
            detail::load(is, native);
            if (name.empty())
                mod.emplace_function(std::move(name), id, std::move(signature), std::move(native)).dispatch();
            else
                mod.emplace_function(id, std::move(signature), std::move(native)).dispatch();
        } break;
        case function_t::Import: {
            std::string modName;
            detail::load(is, modName);
            function_id function = 0;
            detail::load(is, function);
            mod.emplace_function(id, std::move(modName), function).dispatch();
        } break;
        default: throw std::runtime_error("unknown function type");
        }
    }

    std::uint64_t bytecodeLength = 0;
    detail::load(is, bytecodeLength);
    mod.bytecode().resize(bytecodeLength);
    is.read(reinterpret_cast<char*>(mod.bytecode().data()), static_cast<std::streamsize>(bytecodeLength));

    return mod;
}

} // namespace furvm
