#include "furvm/function.hpp"

#include <utility>

namespace furvm {

function::~function() {
    switch (m_type) {
    case function_t::Normal:
    case function_t::Native:
    default: break;
    case function_t::Import: {
        m_value.imp.~import_function();
    } break;
    }
}

function::function(function&& other) noexcept
  : m_name(std::move(other.m_name)), m_type(other.m_type) {
    switch (m_type) {
    case function_t::Normal: {
        m_value.position = other.m_value.position;
    } break;
    case function_t::Native: {
        new (&m_value.native) native_function(std::move(other.m_value.native));
    } break;
    case function_t::Import: {
        m_value.imp = std::move(other.m_value.imp);
    } break;
    }
    other.m_value.position = 0;
}

function& function::operator=(function&& other) noexcept {
    if (this == &other) return *this;

    m_name = std::move(other.m_name);
    m_type = other.m_type;
    switch (m_type) {
    case function_t::Normal: {
        m_value.position = other.m_value.position;
    } break;
    case function_t::Native: {
        new (&m_value.native) native_function(std::move(other.m_value.native));
    } break;
    case function_t::Import: {
        m_value.imp = std::move(other.m_value.imp);
    } break;
    }
    other.m_value.position = 0;

    return *this;
}

function::function(const function& other)
  : m_name(other.m_name), m_type(other.m_type) {
    switch (m_type) {
    case function_t::Normal: {
        m_value.position = other.m_value.position;
    } break;
    case function_t::Native: {
        new (&m_value.native) native_function(other.m_value.native);
    } break;
    case function_t::Import: {
        m_value.imp = other.m_value.imp;
    } break;
    }
}

function& function::operator=(const function& other) {
    if (this == &other) return *this;

    m_name = other.m_name;
    m_type = other.m_type;
    switch (m_type) {
    case function_t::Normal: {
        m_value.position = other.m_value.position;
    } break;
    case function_t::Native: {
        new (&m_value.native) native_function(other.m_value.native);
    } break;
    case function_t::Import: {
        m_value.imp = other.m_value.imp;
    } break;
    }

    return *this;
}

} // namespace furvm
