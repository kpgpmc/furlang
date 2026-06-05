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
private:
    bytecode_t m_bytecode;
};

} // namespace furvm

#endif // FURVM_MODULE_HPP