#ifndef FURVM_CONSTANT_HPP
#define FURVM_CONSTANT_HPP

#include "furvm/fwd.hpp"

#include <exception>
#include <string_view>

namespace furvm {

/**
 * @brief Bad constant access exception.
 */
class bad_constant_access : public std::exception {
public:
    bad_constant_access()           = default;
    ~bad_constant_access() override = default;

    /**
     * @brief Move constructor.
     */
    bad_constant_access(bad_constant_access&&) noexcept = default;

    /**
     * @brief Move constructor.
     */
    bad_constant_access& operator=(bad_constant_access&&) noexcept = default;

    /**
     * @brief Copy constructor.
     */
    bad_constant_access(const bad_constant_access&) = default;

    /**
     * @brief Copy constructor.
     */
    bad_constant_access& operator=(const bad_constant_access&) = default;
public:
    /**
     * @brief Returns a C-style string describing the cause of the error.
     *
     * @return The cause of the error.
     */
    const char* what() const noexcept override { return "bad constant access"; }
};

enum class constant_t : std::uint8_t {
    String, /**< String constant. */
};

class constant {
public:
    using string_type = std::string_view; /**< String constant type. */
public:
    /**
     * @brief Construct a new string constant.
     *
     * @param string String.
     */
    constant(string_type string)
      : m_type(constant_t::String), m_value(string) {}

    ~constant() = default;

    /**
     * @brief Move constructor.
     */
    constant(constant&&) = default;

    /**
     * @brief Move constructor.
     */
    constant& operator=(constant&&) = default;

    constant(const constant&)            = delete;
    constant& operator=(const constant&) = delete;
public:
    /**
     * @brief Returns this constant's type.
     * @see constant_t
     *
     * @return The constant type.
     */
    constexpr constant_t type() const { return m_type; }

    /**
     * @brief Returns this constant's string value.
     * @throws bad_constant_access if this constant's type is not constant_t::String.
     *
     * @return The string value.
     */
    constexpr string_type string() const {
        require_type(constant_t::String);
        return m_value.string;
    }
private:
    void require_type(constant_t type) const {
        if (m_type != type) throw bad_constant_access();
    }
private:
    constant_t m_type{};
    union value {
        string_type string;

        value(string_type sv)
          : string(sv) {}
    } m_value;
};

} // namespace furvm

#endif // FURVM_CONSTANT_HPP