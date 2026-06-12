#ifndef FURLANG_IR_OPERAND_HPP
#define FURLANG_IR_OPERAND_HPP

#include <cstdint>
#include <ostream>
#include <string>

namespace furlang {
namespace ir {

/**
 * @brief Operand type
 */
enum class operand_t {
    None,     /**< None */
    Register, /**< Register */
    Variable, /**< Variable */
    Integer,  /**< Integer */
    String,   /**< String */
};

/**
 * @brief Alias to a register type.
 */
using register_t = std::uint32_t;

/**
 * @brief Register operand alias.
 * @see operand_t::Register
 */
struct register_operand {
    register_t    reg;
    std::uint32_t ver = 0;

    register_operand(register_t reg)
      : reg(reg) {}

    register_operand(register_t reg, std::uint32_t ver)
      : reg(reg), ver(ver) {}

    operator std::uint32_t&() { return reg; }
    operator const std::uint32_t&() const { return reg; }

    friend std::ostream& operator<<(std::ostream& os, const register_operand& op) {
        return os << op.reg << '_' << op.ver;
    }
};

/**
 * @brief Variable operand alias.
 * @see operand_t::Variable
 */
using variable_operand = std::string;
/**
 * @brief Integer operand alias.
 * @see operand_t::Integer
 */
using integer_operand = std::uint64_t;
/**
 * @brief String operand alias.
 * @see operand_t::String
 */
using string_operand = std::string;

/**
 * @brief IR operand
 */
class operand {
public:
    ~operand() {
        if (m_type == operand_t::String) {
            m_value.string.~basic_string();
        }
    }

    /**
     * @brief Move constructor.
     */
    operand(operand&& other) noexcept
      : m_type(other.m_type) {
        switch (m_type) {
        case operand_t::None: break;
        case operand_t::Register: {
            m_value.reg = other.m_value.reg;
        } break;
        case operand_t::Variable: {
            new (&m_value.variable) variable_operand(std::move(other.m_value.variable));
        } break;
        case operand_t::Integer: {
            m_value.integer = other.m_value.integer;
        } break;
        case operand_t::String: {
            new (&m_value.string) string_operand(std::move(other.m_value.string));
        } break;
        }
        other.m_value.destroy(other.m_type);
    }

    /**
     * @brief Move constructor.
     */
    operand& operator=(operand&& other) noexcept {
        if (this == &other) return *this;
        m_type = other.m_type;
        switch (m_type) {
        case operand_t::None: break;
        case operand_t::Register: {
            m_value.reg = other.m_value.reg;
        } break;
        case operand_t::Variable: {
            new (&m_value.variable) variable_operand(std::move(other.m_value.variable));
        } break;
        case operand_t::Integer: {
            m_value.integer = other.m_value.integer;
        } break;
        case operand_t::String: {
            new (&m_value.string) string_operand(std::move(other.m_value.string));
        } break;
        }
        other.m_value.destroy(other.m_type);
        return *this;
    }

    operand(const operand&)            = delete;
    operand& operator=(const operand&) = delete;
public:
    /**
     * @brief Construct a new register operand.
     *
     * @param value Value of the new register operand.
     * @return The register operand.
     */
    static operand new_reg(register_t value) {
        operand operand;
        operand.m_type      = operand_t::Register;
        operand.m_value.reg = { value, 0 };
        return operand;
    }

    /**
     * @brief Construct a new register operand.
     *
     * @param value Value of the new register operand.
     * @return The register operand.
     */
    static operand new_reg(register_operand value) {
        operand operand;
        operand.m_type      = operand_t::Register;
        operand.m_value.reg = value;
        return operand;
    }

    /**
     * @brief Construct a new variable operand.
     *
     * @param value Value of the new variable operand.
     * @return The variable operand.
     */
    template <typename T>
    static operand new_variable(T&& value) {
        operand operand;
        operand.m_type = operand_t::Variable;
        new (&operand.m_value.variable) variable_operand(std::forward<T>(value));
        return operand;
    }

    /**
     * @brief Construct a new integer operand.
     *
     * @param value Value of the new integer operand.
     * @return The integer operand.
     */
    static operand new_integer(integer_operand value) {
        operand operand;
        operand.m_type          = operand_t::Integer;
        operand.m_value.integer = value;
        return operand;
    }

    /**
     * @brief Construct a new string operand.
     *
     * @param value Value of the new string operand.
     * @return The string operand.
     */
    template <typename T>
    static operand new_string(T&& value) {
        operand operand;
        operand.m_type = operand_t::String;
        new (&operand.m_value.string) string_operand(std::forward<T>(value));
        return operand;
    }
public:
    /**
     * @brief Returns this operand's type.
     *
     * @return The operand type.
     */
    operand_t type() const { return m_type; }

    /**
     * @brief Returns this operand's register value.
     *
     * @return The register value.
     */
    register_operand reg() const { return m_value.reg; }

    /**
     * @brief Returns this operand's integer value.
     *
     * @return The integer value.
     */
    integer_operand integer() const { return m_value.integer; }
public:
    /**
     * @brief Prints an operand to an output stream.
     *
     * @param os Output stream.
     * @param operand Operand to print.
     * @return The output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const operand& operand) {
        switch (operand.m_type) {
        case operand_t::None: return os << "none";
        case operand_t::Register: return os << '%' << operand.m_value.reg;
        case operand_t::Variable: return os << operand.m_value.variable;
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
        variable_operand variable;
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

        value(value&&) noexcept            = delete;
        value& operator=(value&&) noexcept = delete;
        value(const value&)                = delete;
        value& operator=(const value&)     = delete;
    } m_value;
};

} // namespace ir
} // namespace furlang

#endif // FURLANG_IR_OPERAND_HPP
