#ifndef FURVM_MODULE_HPP
#define FURVM_MODULE_HPP

#include "furvm/function.hpp"
#include "furvm/fwd.hpp"

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
    function_h function_at(Name&& name) const {
        if (auto it = m_functionMap.find(std::forward<Name>(name)); it != m_functionMap.end()) return it->second;
        throw std::runtime_error("invalid function");
    }

    /**
     * @brief Returns a function from this module.
     *
     * @param id Id of the function.
     * @return The function.
     */
    function_h function_at(function_id id) const { return { id, m_functions.at(id) }; }

    template <typename... Args>
    function_h emplace_function(Args&&... args) {
        function    function(std::forward<Args>(args)...);
        std::string name   = function.name();
        function_id id     = m_functions.size();
        function_h  handle = { id, m_functions.emplace_back(std::move(function)) };
        m_functionMap.emplace(std::move(name), handle);
        return handle;
    }
private:
    std::string m_name;
    bytecode_t  m_bytecode;

    std::unordered_map<std::string, function_h> m_functionMap;
    std::vector<function>                       m_functions;
};

} // namespace furvm

#endif // FURVM_MODULE_HPP
