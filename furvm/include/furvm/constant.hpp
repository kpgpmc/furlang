#ifndef FURVM_CONSTANT_HPP
#define FURVM_CONSTANT_HPP

#include "furvm/exceptions.hpp"
#include "furvm/fwd.hpp"

#include <string_view>

namespace furvm {

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
