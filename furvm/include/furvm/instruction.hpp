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
     * @brief Pushes a new array onto the stack.
     *
     * Type is the next 4 bytes in little-endian.
     * If the type is dynamic the array's size will be popped off of the stack.
     */
    Array,

    /**
     * @brief Pushes an element from an array onto the stack.
     */
    Get,

    /**
     * @brief Sets an array element.
     */
    Set,

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
     * @brief Pushes a new reference onto the stack.
     *
     * Pops the top thing from the stack and pushes its reference.
     */
    Reference,

    /**
     * @brief Adds two things together on the stack.
     */
    Add,

    /**
     * @brief Subtracts two things together on the stack.
     */
    Sub,

    /**
     * @brief Multiplies two things together on the stack.
     */
    Mul,

    /**
     * @brief Divides two things together on the stack.
     */
    Div,

    /**
     * @brief Modulos two things together on the stack.
     */
    Mod,

    /**
     * @brief Compares two top-most things from the stack for equality.
     */
    Equals,

    /**
     * @brief Compares two top-most things from the stack for inequality.
     */
    NotEquals,

    /**
     * @brief Compares if the first top-most thing is less than the second top-most thing.
     */
    LessThan,

    /**
     * @brief Compares if the first top-most thing is greater than the second top-most thing.
     */
    GreaterThan,

    /**
     * @brief Compares if the first top-most thing is less than or equal to the second top-most thing.
     */
    LessEqual,

    /**
     * @brief Compares if the first top-most thing is greater than or equal to the second top-most thing.
     */
    GreaterEqual,

    /**
     * @brief Pushes a pointer of popped-off thing onto the stack.
     */
    Pointerof,

    /**
     * @brief Pushes a size of popped-off thing onto the stack.
     */
    Sizeof,

    /**
     * @brief Pushes a length of popped-off thing onto the stack.
     */
    Lengthof,

    /**
     * @brief Pushes a variable onto the stack.
     *
     * Fetches a variable denoted by next two bytes in little-endian and pushes it onto the stack.
     */
    Load,

    /**
     * @brief Stores an element from the stack in a variable.
     *
     * Pops a thing from the stack and stores it in a variable denoted by next two bytes in little-endian.
     */
    Store,

    /**
     * @brief Calls a function.
     *
     * Calls a function denoted by next two bytes in little-endian from current frame's module.
     */
    Call,

    /**
     * @brief Jumps to an instruction relative to the current instruction.
     *
     * Jumps to an instruction relative to the current instruction with offset denoted by next byte.
     */
    Jump,

    /**
     * @brief Jumps to an instruction relative to the current instruction if top thing on the stack is not zero.
     *
     * Jumps to an instruction relative to the current instruction with offset denoted by next byte if the top thing on
     * the stack is not zero (is true).
     */
    JumpNotZero,

    /**
     * @brief Pops the current call frame.
     */
    Return,
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
