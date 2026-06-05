#ifndef FURVM_INSTRUCTION_HPP
#define FURVM_INSTRUCTION_HPP

#include "furvm/fwd.hpp"

namespace furvm {

enum class instruction_t : std::uint8_t {
    /**
     * @brief No operation.
     */
    NoOperation = 0,

    /**
     * @brief Pushes a constant onto the stack.
     *
     * Pushes a constant from the constant pool denoted by two next bytes in little-endian onto the stack.
     */
    PushConstant,

    /**
     * @brief Pops top element from the stack.
     */
    Drop,

    /**
     * @brief Duplicates top element on the stack.
     */
    Duplicate,
};

struct instruction {
    instruction_t type; /**< Instruction type. */

    /**
     * @brief Instruction value.
     */
    union value {
        constant_index constant; /**< Constant instruction argument. */
    } value;                     /**< Instruction value. */
};

} // namespace furvm

#endif // FURVM_INSTRUCTION_HPP