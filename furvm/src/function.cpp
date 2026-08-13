#include "furvm/function.hpp"

#include "furlang/utility/hash.hpp"
#include "furvm/furvm.hpp"

#include <utility>

namespace furvm {

function::function(const mod_h& mod, const function_h& function)
  : m_type(function_t::Import), m_value(import_function{ mod.id(), function.id() }) {}

function::~function() {
    switch (m_type) {
    case function_t::Normal:
    default: break;
    case function_t::Native: {
        m_value.native.~native_function();
    } break;
    case function_t::Import: {
        m_value.imp.~import_function();
    } break;
    }
}

function::function(function&& other) noexcept
  : m_type(other.m_type), m_signature(std::move(other.m_signature)) {
    switch (m_type) {
    case function_t::Normal: {
        m_value.position = other.m_value.position;
    } break;
    case function_t::Native: {
        new (&m_value.native) native_function(std::move(other.m_value.native));
    } break;
    case function_t::Import: {
        new (&m_value.imp) import_function(std::move(other.m_value.imp));
    } break;
    default: break;
    }
    other.m_value.position = 0;
}

function& function::operator=(function&& other) noexcept {
    if (this == &other) return *this;

    m_type      = other.m_type;
    m_signature = other.m_signature;
    switch (m_type) {
    case function_t::Normal: {
        m_value.position = other.m_value.position;
    } break;
    case function_t::Native: {
        new (&m_value.native) native_function(std::move(other.m_value.native));
    } break;
    case function_t::Import: {
        new (&m_value.imp) import_function(std::move(other.m_value.imp));
    } break;
    default: break;
    }
    other.m_value.position = 0;

    return *this;
}

function::function(const function& other)
  : m_type(other.m_type), m_signature(other.m_signature) {
    switch (m_type) {
    case function_t::Normal: {
        m_value.position = other.m_value.position;
    } break;
    case function_t::Native: {
        new (&m_value.native) native_function(other.m_value.native);
    } break;
    case function_t::Import: {
        new (&m_value.imp) import_function(other.m_value.imp);
    } break;
    default: break;
    }
}

function& function::operator=(const function& other) {
    if (this == &other) return *this;

    m_type      = other.m_type;
    m_signature = other.m_signature;
    switch (m_type) {
    case function_t::Normal: {
        m_value.position = other.m_value.position;
    } break;
    case function_t::Native: {
        new (&m_value.native) native_function(other.m_value.native);
    } break;
    case function_t::Import: {
        new (&m_value.imp) import_function(other.m_value.imp);
    } break;
    default: break;
    }

    return *this;
}

namespace detail {

struct mod_type_hash {
    std::size_t operator()(const mod_type_h& type) const {
        std::size_t hash = std::hash<decltype(type->type)>{}(type->type);
        switch (type->type) {
        case mod_type::S8:
        case mod_type::S16:
        case mod_type::S32:
        case mod_type::S64:
        case mod_type::U8:
        case mod_type::U16:
        case mod_type::U32:
        case mod_type::U64: return hash;
        case mod_type::Ptr:
        case mod_type::Ref: return furlang::utility::hash_combine(hash, type->value.typeRef);
        case mod_type::Array:
            hash = furlang::utility::hash_combine(hash, type->value.array.size);
            return furlang::utility::hash_combine(hash, type->value.array.typeId);
        case mod_type::Import:
            hash = furlang::utility::hash_combine(hash,
                std::hash<decltype(type->value.imprt.modId)>{}(type->value.imprt.modId));
            return furlang::utility::hash_combine(hash, type->value.imprt.typeId);
        case mod_type::Count: break;
        }
        return hash;
    }
};

std::size_t function_sig_hash::operator()(const function_sig& signature) const {
    return furlang::utility::vector_hash<mod_type_h, mod_type_hash>{}(signature.params);
}

} // namespace detail

} // namespace furvm
