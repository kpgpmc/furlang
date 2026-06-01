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
    bool emplace(Args&&... args) {
        if (has_exit()) return false;
        auto instr = std::make_unique<T>(std::forward<Args>(args)...);
        if (is_exit_instruction(instr->type())) {
            m_exit = std::move(instr);
        } else {
            m_instructions.emplace_back(std::move(instr));
        }
        return true;
    }

    std::vector<value_type>&       instructions() { return m_instructions; }
    const std::vector<value_type>& instructions() const { return m_instructions; }

    bool              has_exit() const { return m_exit != nullptr; }
    value_type&       exit() { return m_exit; }
    const value_type& exit() const { return m_exit; }
private:
    std::vector<value_type> m_instructions;
    value_type              m_exit;
};

} // namespace ir
} // namespace furlang

#endif // FURLANG_IR_BLOCK_HPP