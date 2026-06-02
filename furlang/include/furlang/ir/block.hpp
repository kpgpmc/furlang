#ifndef FURLANG_IR_BLOCK_HPP
#define FURLANG_IR_BLOCK_HPP

#include "furlang/ir/instruction.hpp"

#include <memory>
#include <type_traits>
#include <vector>

namespace furlang {
namespace ir {

/**
 * @brief Basic block.
 *
 * A basic block of IR instructions. https://en.wikipedia.org/wiki/Basic_block
 */
class block {
public:
    using value_type = std::unique_ptr<instruction>; /**< Value type */
public:
    block() = default;
public:
    /**
     * @brief Emplaces a new instruction.
     *
     * Emplaces a new instruction, if exit instruction hasn't been emplaced in this block yet.
     *
     * @tparam T Type of the instruction to emplace.
     * @param args Arguments to call the constructor with.
     * @return true if the instruction has been emplaced successfully.
     */
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

    /**
     * @brief Returns this block's instructions.
     *
     * @return The instructions.
     */
    std::vector<value_type>& instructions() { return m_instructions; }

    /**
     * @brief Returns this block's instructions.
     *
     * @return The instructions.
     */
    const std::vector<value_type>& instructions() const { return m_instructions; }

    /**
     * @brief Checks whether an exit instruction has been emplaced in this block yet.
     *
     * @return true if the exit instruction has been emplaced.
     */
    bool has_exit() const { return m_exit != nullptr; }

    /**
     * @brief Returns this block's exit instruction.
     *
     * @return The exit instruction.
     */
    value_type& exit() { return m_exit; }

    /**
     * @brief Returns this block's exit instruction.
     *
     * @return The exit instruction.
     */
    const value_type& exit() const { return m_exit; }
private:
    std::vector<value_type> m_instructions;
    value_type              m_exit;
};

} // namespace ir
} // namespace furlang

#endif // FURLANG_IR_BLOCK_HPP