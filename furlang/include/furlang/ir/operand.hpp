#ifndef FURLANG_IR_OPERAND_HPP
#define FURLANG_IR_OPERAND_HPP

#include <cstdint>
#include <ostream>
#include <string>

namespace furlang {
namespace ir {

enum class operand_t {
    None,
    Register,
    Integer,
    String,
};

using register_operand = std::uint32_t;
using integer_operand  = std::uint64_t;
using string_operand   = std::string;

class operand {
public:
    ~operand() {
        if (m_type == operand_t::String) {
            m_value.string.~basic_string();
        }
    }

    operand(operand&& other) noexcept
      : m_type(other.m_type) {
        switch (m_type) {
        case operand_t::None: break;
        case operand_t::Register: {
            m_value.reg = other.m_value.reg;
        } break;
        case operand_t::Integer: {
            m_value.integer = other.m_value.integer;
        } break;
        case operand_t::String: {
            new (&m_value.string) std::string(std::move(other.m_value.string));
        } break;
        }
        other.m_value.destroy(other.m_type);
    }

    operand& operator=(operand&& other) noexcept {
        if (this == &other) return *this;
        m_type = other.m_type;
        switch (m_type) {
        case operand_t::None: break;
        case operand_t::Register: {
            m_value.reg = other.m_value.reg;
        } break;
        case operand_t::Integer: {
            m_value.integer = other.m_value.integer;
        } break;
        case operand_t::String: {
            new (&m_value.string) std::string(std::move(other.m_value.string));
        } break;
        }
        other.m_value.destroy(other.m_type);
        return *this;
    }

    operand(const operand&)            = delete;
    operand& operator=(const operand&) = delete;
public:
    static operand new_reg(register_operand value) {
        operand operand;
        operand.m_type = operand_t::Register;
        new (&operand.m_value) union value(value);
        return operand;
    }

    static operand new_integer(integer_operand value) {
        operand operand;
        operand.m_type = operand_t::Integer;
        new (&operand.m_value) union value(value);
        return operand;
    }

    static operand new_string(const string_operand& value) {
        operand operand;
        operand.m_type = operand_t::String;
        new (&operand.m_value) union value(value);
        return operand;
    }

    static operand new_string(string_operand&& value) {
        operand operand;
        operand.m_type = operand_t::String;
        new (&operand.m_value) union value(std::move(value));
        return operand;
    }
public:
    operand_t type() const { return m_type; }

    register_operand reg() const { return m_value.reg; }
    integer_operand  integer() const { return m_value.integer; }
public:
    friend std::ostream& operator<<(std::ostream& os, const operand& operand) {
        switch (operand.m_type) {
        case operand_t::None: return os << "none";
        case operand_t::Register: return os << '%' << operand.m_value.reg;
        case operand_t::Integer: return os << operand.m_value.integer;
        case operand_t::String: return os << '"' << operand.m_value.string << '"';
        }
        return os;
    }
private:
    operand() = default;
private:
    operand_t m_type = operand_t::None;
    union value {
        std::nullptr_t   null = nullptr;
        register_operand reg;
        integer_operand  integer;
        string_operand   string;

        void destroy(operand_t type) {
            switch (type) {
            case operand_t::String: {
                string.~basic_string();
            } break;
            default: break;
            }
            null = nullptr;
        }

        value() = default;
        ~value() {}

        value(register_operand reg)
          : reg(reg) {}

        value(integer_operand integer)
          : integer(integer) {}

        value(const std::string& string)
          : string(string) {}

        value(std::string&& string)
          : string(std::move(string)) {}

        value(value&&) noexcept            = delete;
        value& operator=(value&&) noexcept = delete;
        value(const value&)                = delete;
        value& operator=(const value&)     = delete;
    } m_value;
};

} // namespace ir
} // namespace furlang

#endif // FURLANG_IR_OPERAND_HPP