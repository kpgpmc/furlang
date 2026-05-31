#ifndef FURLANG_IR_BLOCK_HPP
#define FURLANG_IR_BLOCK_HPP

#include "furlang/ir/instruction.hpp"

#include <memory>
#include <type_traits>
#include <vector>

namespace furlang {
namespace ir {

// https://en.wikipedia.org/wiki/Basic_block
class block {
public:
    using value_type = std::unique_ptr<instruction>;
public:
    block() = default;
public:
    template <typename T, typename... Args, typename = std::enable_if_t<std::is_base_of_v<instruction, T>>>
    void emplace(Args&&... args) {
        m_instructions.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
    }

    std::vector<value_type>&       instructions() { return m_instructions; }
    const std::vector<value_type>& instructions() const { return m_instructions; }
private:
    std::vector<value_type> m_instructions;
};

} // namespace ir
} // namespace furlang

#endif // FURLANG_IR_BLOCK_HPP