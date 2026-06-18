#ifndef FURVM_MODULE_HPP
#define FURVM_MODULE_HPP

#include "furvm/function.hpp"
#include "furvm/fwd.hpp"
#include "furvm/handle.hpp"

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
    template <typename... Args>
    function_h emplace_function(Args&&... args) {
        function_h function             = m_functions.emplace_back(std::forward<Args>(args)...);
        m_functionMap[function->name()] = function.id();
        return std::move(function);
    }

    template <typename FunctionHandle>
    void push_back(FunctionHandle&& handle) {
        m_functions.emplace_back(std::forward<FunctionHandle>(handle));
    }

    auto function_at(function_id id) { return m_functions.at(id); }
    auto function_at(function_id id) const { return m_functions.at(id); }

    template <typename NameFwd>
    auto function_at(NameFwd&& name) {
        return function_at(m_functionMap.at(std::forward<NameFwd>(name)));
    }

    template <typename NameFwd>
    auto function_at(NameFwd&& name) const {
        return function_at(m_functionMap.at(std::forward<NameFwd>(name)));
    }

    void erase_function(function_id id) { m_functions.erase(id); }
private:
    bytecode_t m_bytecode;

    std::unordered_map<std::string, function_id> m_functionMap;
    handle_container<function_h>                 m_functions;
};

} // namespace furvm

#endif // FURVM_MODULE_HPP
