#ifndef FURVM_INSTRUCTION_HPP
#define FURVM_INSTRUCTION_HPP

#include "furvm/fwd.hpp"

namespace furvm {

enum class instruction_t : byte {
    /**
     * @brief No operation.
     */
    NoOperation = 0,

    /**
     * @brief Pushes an integer from a byte onto the stack.
     *
     * Pushes an integer constructed from a next byte onto the stack.
     */
    PushB2I,

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

    /**
     * @brief Clones top element on the stack.
     */
    Clone,

    /**
     * @brief Pops the current call frame.
     */
    Return,

    /**
     * @brief Pops the current call frame and pushes the first element from the previous stack onto the new stack.
     */
    ReturnValue,
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