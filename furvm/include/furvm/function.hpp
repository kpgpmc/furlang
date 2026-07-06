#ifndef FURVM_FUNCTION_HPP
#define FURVM_FUNCTION_HPP

#include "furvm/fwd.hpp"
#include "furvm/handle.hpp" // IWYU pragma: keep

#include <cstdint>
#include <stdexcept>
#include <string>
#include <type_traits>
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
using native_function = std::string;

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
     * @param paramCount Paremeter count.
     * @param position Offset in bytecode of the function.
     */
    function(std::uint32_t paramCount, bytecode_pos position)
      : m_type(function_t::Normal), m_paramCount(paramCount), m_value(position) {}

    /**
     * @brief Constructs a native function.
     *
     * @param paramCount Paremeter count.
     * @param native Native function tag.
     */
    template <typename Native, typename = std::enable_if_t<std::is_constructible_v<native_function, Native>>>
    function(std::uint32_t paramCount, Native&& native)
      : m_type(function_t::Native), m_paramCount(paramCount), m_value(std::forward<Native>(native)) {}

    /**
     * @brief Constructs an import function.
     *
     * @param mod Module's id.
     * @param function Function's id.
     */
    template <typename ModFwd, typename = std::enable_if_t<std::is_constructible_v<mod_id, ModFwd>>>
    function(ModFwd&& mod, function_id function)
      : m_type(function_t::Import), m_paramCount(0), m_value(import_function{ std::forward<ModFwd>(mod), function }) {}

    /**
     * @brief Constructs an import function.
     *
     * @param mod Module.
     * @param function Function.
     */
    function(const mod_h& mod, const function_h& function);

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
     * @brief Returns a type of this function.
     *
     * @return The type.
     */
    constexpr function_t type() const { return m_type; }

    /**
     * @brief Returns this function's parameter count.
     *
     * @return The parameter count.
     */
    constexpr std::uint32_t param_count() const { return m_paramCount; }
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
     * @return The value.
     */
    const import_function& imp() const {
        if (m_type != function_t::Import) throw std::runtime_error("function type mismatch");
        return m_value.imp;
    }
private:
    function_t    m_type;
    std::uint32_t m_paramCount;

    union value {
        std::size_t     position = 0;
        native_function native;
        import_function imp;

        value() = default;

        value(std::size_t position)
          : position(position) {}

        template <typename Native, typename = std::enable_if_t<std::is_constructible_v<native_function, Native>>>
        value(Native&& native)
          : native(std::forward<Native>(native)) {}

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
