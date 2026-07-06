#ifndef FURVM_MODULE_HPP
#define FURVM_MODULE_HPP

#include "furvm/function.hpp"
#include "furvm/fwd.hpp"
#include "furvm/handle.hpp"
#include "furvm/type.hpp" // IWYU pragma: keep

#include <functional>
#include <istream>
#include <ostream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace furvm {

class mod {
    friend class function;
    friend class serializer;
public:
    using bytecode_t = std::vector<byte>; /**< An alias to a vector of bytes. */

    static constexpr char MAGIC[4] = { 'F', 'u', 'r', 'M' }; /** Furvm module file magic. */

    using native_function = std::function<void(executor&)>;
public:
    /**
     * @brief Constructs a module.
     *
     * @param name Name of the module.
     * @param args Arguments forwarded to bytecode's constructor.
     */
    template <typename... Args, typename = std::enable_if_t<std::is_constructible_v<bytecode_t, Args...>>>
    mod(Args&&... args)
      : m_bytecode(std::forward<Args>(args)...) {}

    ~mod() = default;

    /**
     * @brief Move constructor.
     */
    mod(mod&&) = default;

    /**
     * @brief Move constructor.
     */
    mod& operator=(mod&&) = default;

    mod(const mod&)            = delete;
    mod& operator=(const mod&) = delete;
public:
    /**
     * @brief Returns a byte from bytecode of this module.
     *
     * @param offset An offset of the byte.
     * @return The byte.
     */
    constexpr byte byte(std::size_t offset) const { return m_bytecode.at(offset); }

    /**
     * @brief Returns the module's bytecode.
     *
     * @return A reference to the bytecode.
     */
    constexpr bytecode_t& bytecode() { return m_bytecode; }

    /**
     * @brief Returns the module's bytecode.
     *
     * @return A constant reference to the bytecode.
     */
    constexpr const bytecode_t& bytecode() const { return m_bytecode; }
public:
    /**
     * @brief Emplaces a function in the module's function container.
     *
     * Emplaces the function in module's function container and name to function map and public functions map.
     *
     * @param args Arguments forwarded into the container's emplace_back function.
     * @return A handle to the emplaced function.
     */
    template <typename... Args>
    function_h emplace_function(Args&&... args) {
        function_h function;
        if constexpr (std::is_constructible_v<class function, Args...>) {
            function = std::move(m_functions.emplace_back(std::forward<Args>(args)...));
        } else {
            function = std::move(m_functions.emplace(std::forward<Args>(args)...));
        }
        m_functionMap[function.id()] = "";
        return std::move(function);
    }

    /**
     * @brief Emplaces a function in the module's function container.
     *
     * Emplaces the function in module's function container and name to function map.
     *
     * @param args Arguments forwarded into the container's emplace_back function.
     * @return A handle to the emplaced function.
     */
    template <typename NameFwd,
        typename... Args,
        typename = std::enable_if_t<std::is_constructible_v<std::string, NameFwd>>>
    function_h emplace_function_named(NameFwd&& name, Args&&... args) {
        function_h function;
        if constexpr (std::is_constructible_v<class function, Args...>) {
            function = std::move(m_functions.emplace_back(std::forward<Args>(args)...));
        } else {
            function = std::move(m_functions.emplace(std::forward<Args>(args)...));
        }
        std::string nameInst         = std::forward<NameFwd>(name);
        m_functionNames[nameInst]    = function.id();
        m_functionMap[function.id()] = nameInst;
        return std::move(function);
    }

    /**
     * @brief Returns a function from the module.
     *
     * @param id Identifier of the function.
     * @return A handle to the function.
     */
    auto function_at(function_id id) { return m_functions.at(id); }

    /**
     * @brief Returns a function from the module.
     *
     * @param id Identifier of the function.
     * @return A handle to the function.
     */
    auto function_at(function_id id) const { return m_functions.at(id); }

    /**
     * @brief Returns a function from the module.
     *
     * @param name Name of the function.
     * @return A handle to the function.
     */
    template <typename NameFwd, typename = std::enable_if_t<std::is_constructible_v<std::string, NameFwd>>>
    auto function_at(NameFwd&& name) {
        return function_at(m_functionNames.at(std::forward<NameFwd>(name)));
    }

    /**
     * @brief Returns a function from the module.
     *
     * @param name Name of the function.
     * @return A handle to the function.
     */
    template <typename NameFwd, typename = std::enable_if_t<std::is_constructible_v<std::string, NameFwd>>>
    auto function_at(NameFwd&& name) const {
        return function_at(m_functionNames.at(std::forward<NameFwd>(name)));
    }

    /**
     * @brief Erases a function from the module's function container.
     *
     * @param id Identifier of the function.
     */
    void erase_function(function_id id) { m_functions.erase(id); }
public:
    template <typename NameFwd, typename Func>
    void set_native_function(NameFwd&& name, Func&& func) {
        m_nativeFunctions.emplace(std::forward<NameFwd>(name), std::forward<Func>(func));
    }

    template <typename NameFwd>
    native_function get_native_function(NameFwd&& name) const {
        return m_nativeFunctions.at(std::forward<NameFwd>(name));
    }
public:
    /**
     * @brief Emplaces a type in the context.
     *
     * @param args Arguments forwarded to the type constructor.
     * @return The emplaced type.
     */
    template <typename... Args>
    auto emplace_type(Args&&... args) {
        if constexpr (std::is_constructible_v<type_p, Args...>) {
            return m_types.emplace_back(std::forward<Args>(args)...);
        } else {
            return m_types.emplace(std::forward<Args>(args)...);
        }
    }

    /**
     * @brief Returns a type from the context.
     *
     * @param args type's id.
     * @return A handle to the type.
     */
    template <typename... Args>
    auto type_at(Args&&... args) {
        return m_types.at(std::forward<Args>(args)...);
    }

    /**
     * @brief Returns a type from the context.
     *
     * @param args type's id.
     * @return A handle to the type.
     */
    template <typename... Args>
    auto type_at(Args&&... args) const {
        return m_types.at(std::forward<Args>(args)...);
    }

    /**
     * @brief Erases a type from the context.
     *
     * @param args type's id.
     */
    template <typename... Args>
    void erase_type(Args&&... args) {
        m_types.erase(std::forward<Args>(args)...);
    }
public:
    /**
     * @brief Prints the module in a bytecode form to an output stream.
     *
     * @param os Output stream.
     * @return The output stream.
     */
    std::ostream& serialize(std::ostream& os) const;

    /**
     * @brief Loads a module in a bytecode form from an input stream.
     *
     * @param is Input stream.
     * @return The loaded module.
     */
    static mod load(std::istream& is);
private:
    bytecode_t m_bytecode;

    std::unordered_map<std::string, function_id> m_functionNames;
    std::unordered_map<function_id, std::string> m_functionMap;
    handle_container<function_h>                 m_functions;

    handle_container<type_h> m_types;

    std::unordered_map<std::string, native_function> m_nativeFunctions;
};

} // namespace furvm

#endif // FURVM_MODULE_HPP
