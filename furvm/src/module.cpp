#include "furvm/module.hpp"

#include "furvm/detail/serialization.hpp"
#include "furvm/fwd.hpp"
#include "furvm/type.hpp"

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

    function_id funcCount = m_functions.cend() - m_functions.cbegin();
    detail::serialize(os, funcCount);
    for (function_id id = 0; id < funcCount; ++id) {
        if (!m_functions.contains(id)) {
            detail::serialize(os, "");
            detail::serialize(os, 0);
            detail::serialize(os, std::uint8_t(0xFF)); // null function
            continue;
        }

        function_h func = m_functions.at(id);
        detail::serialize(os, m_functionMap.at(func.id()));
        detail::serialize(os, func->param_count());
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
        case type_t::Array: {
            detail::serialize(os, type->array.size);
            detail::serialize(os, type->array.type.id());
        } break;
        case type_t::Import: {
            detail::serialize(os, type->imp.mod);
            detail::serialize(os, type->imp.type);
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

    function_id functionCount = 0;
    detail::load(is, functionCount);
    for (function_id id = 0; id < functionCount; ++id) {
        std::string name;
        detail::load(is, name);
        std::uint32_t paramCount = 0;
        detail::load(is, paramCount);
        std::uint8_t type = 0;
        detail::load(is, type);
        if (type == 0xFF) continue;

        switch ((function_t)type) {
        case function_t::Normal: {
            decltype(std::declval<function>().position()) offset = 0;
            detail::load(is, offset);
            if (name.empty())
                mod.emplace_function_named(std::move(name), id, paramCount, offset).dispatch();
            else
                mod.emplace_function(id, paramCount, offset).dispatch();
        } break;
        case function_t::Native: {
            std::string native;
            detail::load(is, native);
            if (name.empty())
                mod.emplace_function_named(std::move(name), id, paramCount, std::move(native)).dispatch();
            else
                mod.emplace_function(id, paramCount, std::move(native)).dispatch();
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

    type_id typeCount = 0;
    detail::load(is, typeCount);
    for (type_id id = 0; id < typeCount; ++id) {
        std::uint8_t type = 0;
        detail::load(is, type);
        if (type == 0xFF) continue;

        switch ((type_t)type) {
        case type_t::Primitive: {
            primitive_type primitive = 0;
            detail::load(is, primitive);
            mod.emplace_type(id, std::make_shared<class type>(primitive)).dispatch();
        } break;
        case type_t::Array: {
            std::size_t size = 0;
            detail::load(is, size);
            type_id typeId = 0;
            detail::load(is, typeId);
            mod.emplace_type(id, std::make_shared<class type>(mod.type_at(typeId), size)).dispatch();
        } break;
        case type_t::Import: {
            std::string modName;
            detail::load(is, modName);
            type_id typeId = 0;
            detail::load(is, typeId);
            mod.emplace_type(id, std::make_shared<class type>(import_type{ std::move(modName), typeId })).dispatch();
        } break;
        default: throw std::runtime_error("unknown type type");
        }
    }

    std::uint64_t bytecodeLength = 0;
    detail::load(is, bytecodeLength);
    mod.bytecode().resize(bytecodeLength);
    is.read(reinterpret_cast<char*>(mod.bytecode().data()), static_cast<std::streamsize>(bytecodeLength));

    return mod;
}

} // namespace furvm
