#ifndef FURVM_FUNCTION_HPP
#define FURVM_FUNCTION_HPP

#include "furvm/fwd.hpp"
#include "furvm/module.hpp" // IWYU pragma: keep

#include <functional>
#include <stdexcept>

namespace furvm {

enum class function_t : std::uint8_t {
    Normal = 0, /**< A normal bytecode function. */
    Native,     /**< A native function implemented through furvm API. */
};

using native_function = std::function<void(const executor_p&)>;

class function {
private:
    /**
     * @brief A private token for the private constructor.
     *
     * Also `funkcja` in Polish translates to `function` from what I heard.
     */
    struct funkcja {
        explicit funkcja() = default;
    };
public:
    /**
     * @brief Private constructor.
     *
     * @param id
     * @param position
     * @param mod
     */
    function(funkcja, function_handle id, std::size_t position, const mod_p& mod);

    /**
     * @brief Private constructor.
     *
     * @param id
     * @param native
     * @param mod
     */
    function(funkcja, function_handle id, const native_function& native, const mod_p& mod);

    ~function() = default;

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
     * @brief Returns a new function.
     *
     * @param mod Module.
     * @param args Arguments to pass to the function constructor.
     * @return The new function.
     */
    template <typename... Args,
        typename = std::enable_if_t<std::is_constructible_v<function, funkcja, function_handle, Args..., const mod_p&>>>
    static function_p create(const mod_p& mod, Args&&... args) {
        function_handle id = mod->m_functions.size();

        auto func = std::make_shared<function>(funkcja{}, id, std::forward<Args>(args)..., mod);
        mod->m_functions.emplace(mod->m_functions.begin() + id, func);
        return std::move(func);
    }
public:
    /**
     * @brief Returns an id of this function.
     *
     * @return The id.
     */
    constexpr function_handle id() const { return m_id; }

    /**
     * @brief Returns a type of this function.
     *
     * @return The type.
     */
    constexpr function_t type() const { return m_type; }

    /**
     * @brief Returns a parent module of this function.
     *
     * @return A shared pointer to the module.
     */
    const mod_p& mod() const { return m_module; }
public:
    /**
     * @brief Returns a value for normal function.
     *
     * @return The value.
     */
    std::size_t position() const {
        if (m_type == function_t::Normal) throw std::runtime_error("function type mismatch");
        return m_value.position;
    }

    /**
     * @brief Returns a value for native function.
     *
     * @return The value.
     */
    const native_function& native() const {
        if (m_type == function_t::Native) throw std::runtime_error("function type mismatch");
        return m_value.native;
    }
private:
    function_handle m_id;
    function_t      m_type;
    mod_p           m_module;

    union value {
        std::size_t     position = 0;
        native_function native;

        value() = default;

        value(std::size_t position)
          : position(position) {}

        value(const native_function& native)
          : native(native) {}

        ~value() {}

        value(value&& other)                 = delete;
        value& operator=(value&& other)      = delete;
        value(const value& other)            = delete;
        value& operator=(const value& other) = delete;
    } m_value;
};

} // namespace furvm

#endif // FURVM_FUNCTION_HPP