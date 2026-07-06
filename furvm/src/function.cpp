#include "furvm/function.hpp"

#include "furvm/furvm.hpp"

#include <utility>

namespace furvm {

function::function(const mod_h& mod, const function_h& function)
  : m_type(function_t::Import), m_paramCount(0), m_value(import_function{ mod.id(), function.id() }) {}

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
  : m_type(other.m_type), m_paramCount(other.m_paramCount) {
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

    m_type       = other.m_type;
    m_paramCount = other.m_paramCount;
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
  : m_type(other.m_type), m_paramCount(other.m_paramCount) {
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

    m_type       = other.m_type;
    m_paramCount = other.m_paramCount;
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

} // namespace furvm
