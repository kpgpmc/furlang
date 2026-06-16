#ifndef FURVM_MODULE_HPP
#define FURVM_MODULE_HPP

#include "furvm/fwd.hpp"

#include <string>
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
     * @param id Id of the function.
     * @return The function.
     */
    constexpr const function_p& function_at(function_handle id) const { return m_functions.at(id); }
private:
    std::string             m_name;
    bytecode_t              m_bytecode;
    std::vector<function_p> m_functions;
};

} // namespace furvm

#endif // FURVM_MODULE_HPP
