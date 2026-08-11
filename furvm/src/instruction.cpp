#include "furvm/instruction.hpp"

namespace furvm {

instruction_argument_t instruction::s_arguments[instruction::Count] = {
    // NoOperation:
    instruction_argument::None,
    // PushS8:
    instruction_argument::Byte,
    // PushU8:
    instruction_argument::Byte,
    // PushS16:
    instruction_argument::Short,
    // PushU16:
    instruction_argument::Short,
    // PushS32:
    instruction_argument::Int,
    // PushU32:
    instruction_argument::Int,
    // PushConstant:
    instruction_argument::Constant,
    // Array:
    instruction_argument::Type,
    // Get:
    instruction_argument::None,
    // Set:
    instruction_argument::None,
    // Drop:
    instruction_argument::None,
    // Duplicate:
    instruction_argument::None,
    // Swap:
    instruction_argument::None,
    // Clone:
    instruction_argument::None,
    // Reference:
    instruction_argument::None,
    // Add:
    instruction_argument::None,
    // Sub:
    instruction_argument::None,
    // Mul:
    instruction_argument::None,
    // Div:
    instruction_argument::None,
    // Mod:
    instruction_argument::None,
    // Equals:
    instruction_argument::None,
    // NotEquals:
    instruction_argument::None,
    // LessThan:
    instruction_argument::None,
    // GreaterThan:
    instruction_argument::None,
    // LessEqual:
    instruction_argument::None,
    // GreaterEqual:
    instruction_argument::None,
    // Pointerof:
    instruction_argument::None,
    // Sizeof:
    instruction_argument::None,
    // Lengthof:
    instruction_argument::None,
    // Load:
    instruction_argument::Variable,
    // Store:
    instruction_argument::Variable,
    // Call:
    instruction_argument::Function,
    // Jump:
    instruction_argument::Offset,
    // JumpNotZero:
    instruction_argument::Offset,
    // Return:
    instruction_argument::None,
};

}
