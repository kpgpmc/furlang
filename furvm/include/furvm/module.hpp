#ifndef FURVM_MODULE_HPP
#define FURVM_MODULE_HPP

#include "furvm/fwd.hpp"

#include <vector>

namespace furvm {

class mod {
public:
    using bytecode_t = std::vector<byte>; /**< An alias to a vector of bytes. */
public:
    /**
     * @brief Construct a new module.
     *
     * @param id Id of this module.
     * @param args Arguments to forward to bytecode's constructor.
     */
    template <typename... Args, typename = std::enable_if_t<std::is_constructible_v<bytecode_t, Args...>>>
    mod(module_handle id, Args&&... args)
      : m_id(id), m_bytecode(std::forward<Args>(args)...) {}

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
     * @brief Returns an id of this module.
     *
     * @return The id.
     */
    constexpr module_handle id() const { return m_id; }

    /**
     * @brief Returns a byte from bytecode of this module.
     *
     * @param offset An offset of the byte.
     * @return The byte.
     */
    constexpr byte byte(std::size_t offset) const { return m_bytecode.at(offset); }
private:
    module_handle m_id;
    bytecode_t    m_bytecode;
};

} // namespace furvm

#endif // FURVM_MODULE_HPP