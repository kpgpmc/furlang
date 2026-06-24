#ifndef FURVM_MODULE_HPP
#define FURVM_MODULE_HPP

#include "furvm/function.hpp"
#include "furvm/fwd.hpp"
#include "furvm/handle.hpp"

#include <ostream>
#include <string>
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
     * @param args Arguments forwarded into the container's emplace_back function.
     * @return A handle to the emplaced function.
     */
    template <typename... Args>
    function_h emplace_function(Args&&... args) {
        function_h function             = m_functions.emplace_back(std::forward<Args>(args)...);
        m_functionMap[function->name()] = function.id();
        return std::move(function);
    }

    /**
     * @brief Inserts a function in the module's function container.
     *
     * @param function Function to insert.
     */
    template <typename Function>
    void push_back(Function&& function) {
        m_functions.emplace_back(std::forward<Function>(function));
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
    template <typename NameFwd>
    auto function_at(NameFwd&& name) {
        return function_at(m_functionMap.at(std::forward<NameFwd>(name)));
    }

    /**
     * @brief Returns a function from the module.
     *
     * @param name Name of the function.
     * @return A handle to the function.
     */
    template <typename NameFwd>
    auto function_at(NameFwd&& name) const {
        return function_at(m_functionMap.at(std::forward<NameFwd>(name)));
    }

    /**
     * @brief Erases a function from the module's function container.
     *
     * @param id Identifier of the function.
     */
    void erase_function(function_id id) { m_functions.erase(id); }
public:
    /**
     * @brief Prints the module in a bytecode form to an output stream.
     *
     * @param os Output stream.
     * @return The output stream.
     */
    std::ostream& serialize(std::ostream& os) const;
private:
    bytecode_t m_bytecode;

    std::unordered_map<std::string, function_id> m_functionMap;
    handle_container<function_h>                 m_functions;
};

} // namespace furvm

#endif // FURVM_MODULE_HPP
