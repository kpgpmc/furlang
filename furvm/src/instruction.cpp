#include "furvm/instruction.hpp"

#include <stdexcept>
#include <string>

namespace furvm {

const std::size_t instruction_argument::s_sizes[instruction_argument::Count] = {
    // None:
    0,
    // S8:
    1,
    // U8:
    1,
    // S16:
    2,
    // U16:
    2,
    // S32
    4,
    // U32
    4,
    // Constant:
    4,
    // Type:
    4,
    // Variable:
    2,
    // Function:
    4,
    // Offset:
    1,
};

const bool instruction_argument::s_signedness[instruction_argument::Count] = {
    // None:
    false,
    // S8:
    true,
    // U8:
    false,
    // S16:
    true,
    // U16:
    false,
    // S32
    true,
    // U32:
    false,
    // Constant:
    false,
    // Type:
    false,
    // Variable:
    false,
    // Function:
    false,
    // Offset:
    true,
};

const instruction_argument_t instruction::s_arguments[instruction::Count] = {
    // NoOperation:
    instruction_argument::None,
    // PushS8:
    instruction_argument::S8,
    // PushU8:
    instruction_argument::U8,
    // PushS16:
    instruction_argument::S16,
    // PushU16:
    instruction_argument::U16,
    // PushS32:
    instruction_argument::S32,
    // PushU32:
    instruction_argument::U32,
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

std::size_t instruction::read(std::string_view in) {
    type = static_cast<type_e>(in[0]);
    if (type >= Count) throw std::runtime_error("invalid instruction");
    arg.type = s_arguments[type];

    std::string_view argView = in.substr(1, arg.size());
    switch (argView.size()) {
    case 1: arg.u8 = argView[0]; break;
    case 2: arg.u16 = argView[0] | (argView[1] << 8); break;
    case 4: arg.u32 = argView[0] | (argView[1] << 8) | (argView[2] << 16) | (argView[3] << 24); break;
    default: throw std::runtime_error("unreachable");
    }

    return 1 + argView.size();
}

std::size_t instruction::write(std::string& out) const {
    if (type >= Count) throw std::runtime_error("invalid instruction");
    if (s_arguments[type] != arg.type) throw std::runtime_error("malformed instruction");
    out += static_cast<char>(type);
    switch (arg.size()) {
    case 1: out += arg.is_signed() ? std::to_string(arg.s8) : std::to_string(arg.u8); break;
    case 2: out += arg.is_signed() ? std::to_string(arg.s16) : std::to_string(arg.u16); break;
    case 4: out += arg.is_signed() ? std::to_string(arg.s32) : std::to_string(arg.u32); break;
    default: throw std::runtime_error("unreachable");
    }
    return 0;
}

} // namespace furvm
