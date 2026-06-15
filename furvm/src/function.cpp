#include "furvm/function.hpp"

namespace furvm {

function::function(funkcja, function_handle id, std::size_t position, const mod_p& mod)
  : m_id(id), m_type(function_t::Normal), m_module(mod), m_value(position) {}

function::function(funkcja, function_handle id, const native_function& native, const mod_p& mod)
  : m_id(id), m_type(function_t::Native), m_module(mod), m_value(native) {}

function::function(function&& other) noexcept
  : m_id(other.m_id), m_type(other.m_type), m_module(std::move(other.m_module)) {
    switch (m_type) {
    case function_t::Normal: {
        m_value.position = other.m_value.position;
    } break;
    case function_t::Native: {
        new (&m_value.native) native_function(std::move(other.m_value.native));
    } break;
    case function_t::Import: {
        m_value.imp.mod      = other.m_value.imp.mod;
        m_value.imp.function = other.m_value.imp.function;
    } break;
    }
    other.m_value.position = 0;
}

function& function::operator=(function&& other) noexcept {
    if (this == &other) return *this;

    m_id     = other.m_id;
    m_type   = other.m_type;
    m_module = std::move(other.m_module);
    switch (m_type) {
    case function_t::Normal: {
        m_value.position = other.m_value.position;
    } break;
    case function_t::Native: {
        new (&m_value.native) native_function(std::move(other.m_value.native));
    } break;
    case function_t::Import: {
        m_value.imp.mod      = other.m_value.imp.mod;
        m_value.imp.function = other.m_value.imp.function;
    } break;
    }
    other.m_value.position = 0;

    return *this;
}

} // namespace furvm
