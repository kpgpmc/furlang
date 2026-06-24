#ifndef FURVM_FUNCTION_HPP
#define FURVM_FUNCTION_HPP

#include "furvm/fwd.hpp"

#include <functional>
#include <stdexcept>
#include <string>
#include <utility>

namespace furvm {

enum class function_t : std::uint8_t {
    Normal = 0, /**< A normal bytecode function. */
    Native,     /**< A native function implemented through furvm API. */
    Import,     /**< A function imported from another module. */
};

/**
 * @brief A native function.
 */
using native_function = std::function<void(executor&)>;

/**
 * @brief A function import.
 */
struct import_function {
    mod_id      mod;
    function_id function;
};

class function {
public:
    /**
     * @brief Constructs a normal function.
     *
     * @param name Name of the function.
     * @param position Offset in bytecode of the function.
     */
    template <typename Name>
    function(Name&& name, bytecode_pos position)
      : m_name(std::forward<Name>(name)), m_type(function_t::Normal), m_value(position) {}

    /**
     * @brief Constructs a native function.
     *
     * @param name Name of the function.
     * @param native Native function.
     */
    template <typename Name>
    function(Name&& name, const native_function& native)
      : m_name(std::forward<Name>(name)), m_type(function_t::Native), m_value(native) {}

    /**
     * @brief Constructs an import function.
     *
     * @param name Name of the function.
     * @param imp Import function.
     */
    template <typename Name>
    function(Name&& name, const import_function& imp)
      : m_name(std::forward<Name>(name)), m_type(function_t::Import), m_value(imp) {}

    /**
     * @brief Destructs a function.
     */
    ~function();

    /**
     * @brief Move constructor.
     */
    function(function&&) noexcept;

    /**
     * @brief Move constructor.
     */
    function& operator=(function&&) noexcept;

    /**
     * @brief Copy constructor.
     */
    function(const function&);

    /**
     * @brief Copy constructor.
     */
    function& operator=(const function&);
public:
    /**
     * @brief Returns a name of this function.
     *
     * @return The name.
     */
    constexpr const std::string& name() const { return m_name; }

    /**
     * @brief Returns a type of this function.
     *
     * @return The type.
     */
    constexpr function_t type() const { return m_type; }
public:
    /**
     * @brief Returns normal function's value.
     *
     * @return The value.
     */
    std::size_t position() const {
        if (m_type != function_t::Normal) throw std::runtime_error("function type mismatch");
        return m_value.position;
    }

    /**
     * @brief Returns native function's value.
     *
     * @return The value.
     */
    const native_function& native() const {
        if (m_type != function_t::Native) throw std::runtime_error("function type mismatch");
        return m_value.native;
    }

    /**
     * @brief Returns import function's value.
     *
     * @return The name.
     */
    const import_function& imp() const {
        if (m_type != function_t::Import) throw std::runtime_error("function type mismatch");
        return m_value.imp;
    }
private:
    std::string m_name;
    function_t  m_type;

    union value {
        std::size_t     position = 0;
        native_function native;
        import_function imp;

        value() = default;

        value(std::size_t position)
          : position(position) {}

        value(const native_function& native)
          : native(native) {}

        value(const import_function& imp)
          : imp(imp) {}

        ~value() {}

        value(value&& other)                 = delete;
        value& operator=(value&& other)      = delete;
        value(const value& other)            = delete;
        value& operator=(const value& other) = delete;
    } m_value;
};

} // namespace furvm

#endif // FURVM_FUNCTION_HPP
