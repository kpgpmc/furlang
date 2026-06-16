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

using native_function = std::function<void(executor&)>;

class function {
public:
    template <typename Name>
    function(Name&& name, bytecode_pos position)
      : m_name(std::forward<Name>(name)), m_type(function_t::Normal), m_value(position) {}

    function(const char* name, bytecode_pos position)
      : m_name(name), m_type(function_t::Normal), m_value(position) {}

    template <typename Name>
    function(Name&& name, const native_function& native)
      : m_name(std::forward<Name>(name)), m_type(function_t::Native), m_value(native) {}

    function(const char* name, const native_function& native)
      : m_name(name), m_type(function_t::Native), m_value(native) {}

    template <typename Name, typename ModuleName>
    function(Name&& name, ModuleName&& moduleName, function_id function)
      : m_name(std::forward<Name>(name)),
        m_type(function_t::Import),
        m_value(std::forward<ModuleName>(moduleName), function) {}

    template <typename ModuleName>
    function(const char* name, ModuleName&& moduleName, function_id function)
      : m_name(name), m_type(function_t::Import), m_value(std::forward<ModuleName>(moduleName), function) {}

    ~function();

    /**
     * @brief Move constructor.
     */
    function(function&&) noexcept;

    /**
     * @brief Move constructor.
     */
    function& operator=(function&&) noexcept;

    function(const function&)            = delete;
    function& operator=(const function&) = delete;
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
     * @brief Returns a value for normal function.
     *
     * @return The value.
     */
    std::size_t position() const {
        if (m_type != function_t::Normal) throw std::runtime_error("function type mismatch");
        return m_value.position;
    }

    /**
     * @brief Returns a value for native function.
     *
     * @return The value.
     */
    const native_function& native() const {
        if (m_type != function_t::Native) throw std::runtime_error("function type mismatch");
        return m_value.native;
    }

    /**
     * @brief Returns a name of the function's module.
     *
     * @return The name.
     */
    const std::string& imported_module() const {
        if (m_type != function_t::Import) throw std::runtime_error("function type mismatch");
        return m_value.imp.moduleName;
    }

    /**
     * @brief Returns a module of imported function.
     *
     * @return A handle to the module.
     */
    function_id imported_function() const {
        if (m_type != function_t::Import) throw std::runtime_error("function type mismatch");
        return m_value.imp.function;
    }
private:
    std::string m_name;
    function_t  m_type;

    union value {
        std::size_t     position = 0;
        native_function native;
        struct {
            std::string moduleName;
            function_id function;
        } imp;

        value() = default;

        value(std::size_t position)
          : position(position) {}

        value(const native_function& native)
          : native(native) {}

        template <typename Name>
        value(Name&& moduleName, function_id function)
          : imp({ std::forward<Name>(moduleName), function }) {}

        ~value() {}

        value(value&& other)                 = delete;
        value& operator=(value&& other)      = delete;
        value(const value& other)            = delete;
        value& operator=(const value& other) = delete;
    } m_value;
};

} // namespace furvm

#endif // FURVM_FUNCTION_HPP
