#ifndef FURVM_MODULE_HPP
#define FURVM_MODULE_HPP

#include "furvm/function.hpp"
#include "furvm/fwd.hpp"

#include <algorithm>
#include <memory>
#include <stdexcept>
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
public:
    /**
     * @brief Construct a new module.
     *
     * @param name Name of this module.
     * @param args Arguments to forward to bytecode's constructor.
     */
    template <typename Name,
        typename... Args,
        typename = std::enable_if_t<std::is_constructible_v<bytecode_t, Args...>>>
    mod(Name&& name, Args&&... args)
      : m_name(std::forward<Name>(name)), m_bytecode(std::forward<Args>(args)...) {}

    /**
     * @brief Construct a new module.
     *
     * @param name C-string name of this module.
     * @param args Arguments to forward to bytecode's constructor.
     */
    template <typename... Args, typename = std::enable_if_t<std::is_constructible_v<bytecode_t, Args...>>>
    mod(const char* name, Args&&... args)
      : m_name(name), m_bytecode(std::forward<Args>(args)...) {}

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
     * @brief Returns this module's name.
     *
     * @return The name.
     */
    constexpr const std::string& name() const { return m_name; }

    /**
     * @brief Returns a byte from bytecode of this module.
     *
     * @param offset An offset of the byte.
     * @return The byte.
     */
    constexpr byte byte(std::size_t offset) const { return m_bytecode.at(offset); }

    /**
     * @brief Returns a reference to the module's bytecode.
     *
     * @return The reference to bytecode.
     */
    constexpr bytecode_t& bytecode() { return m_bytecode; }

    /**
     * @brief Returns a const reference to the module's bytecode.
     *
     * @return The reference to bytecode.
     */
    constexpr const bytecode_t& bytecode() const { return m_bytecode; }
public:
    /**
     * @brief Returns a function from this module.
     *
     * @param name Name of the function.
     * @return The function.
     */
    template <typename Name>
    constexpr const function_p& function_at(Name&& name) const {
        return m_functions.at(std::forward<Name>(name));
    }

    /**
     * @brief Returns a function from this module.
     *
     * @param id Id of the function.
     * @return The function.
     */
    constexpr const function_p& function_at(function_handle id) const { return m_functions.at(id); }

    /**
     * @brief Returns an id of a function.
     *
     * @param function Function to get the id of.
     * @return The function's id.
     */
    function_handle function_id(const function_p& function) const {
        if (auto it = std::find(m_functions.begin(), m_functions.end(), function); it != m_functions.end())
            return it - m_functions.begin();
        throw std::runtime_error("function not in the module");
    }

    template <typename... Args>
    function_handle emplace_function(Args&&... args) {
        function_p function = std::make_shared<class function>(std::forward<Args>(args)...);
        m_functionMap.emplace(function->name(), function);
        function_handle id = m_functions.size();
        m_functions.emplace_back(std::move(function));
        return id;
    }
private:
    std::string m_name;
    bytecode_t  m_bytecode;

    std::unordered_map<std::string, function_p> m_functionMap;
    std::vector<function_p>                     m_functions;
};

} // namespace furvm

#endif // FURVM_MODULE_HPP
