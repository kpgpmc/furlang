#ifndef FURVM_INSTRUCTION_HPP
#define FURVM_INSTRUCTION_HPP

#include "furvm/fwd.hpp"

namespace furvm {

struct instruction_argument {
    enum type_e {
        None = 0,
        Byte,
        Short,
        Int,
        Constant,
        Type,
        Variable,
        Function,
        Offset,
    } type;
    union {
        std::int8_t   s8;
        std::uint8_t  u8;
        std::int16_t  s16;
        std::uint16_t u16;
        std::int32_t  s32;
        std::uint32_t u32;
    };
};

using instruction_argument_t = instruction_argument::type_e;

struct instruction {
    enum type_e : byte {
        NoOperation = 0,
        PushS8,
        PushU8,
        PushS16,
        PushU16,
        PushS32,
        PushU32,
        PushConstant,
        Array,
        Get,
        Set,
        Drop,
        Duplicate,
        Swap,
        Clone,
        Reference,
        Add,
        Sub,
        Mul,
        Div,
        Mod,
        Equals,
        NotEquals,
        LessThan,
        GreaterThan,
        LessEqual,
        GreaterEqual,
        Pointerof,
        Sizeof,
        Lengthof,
        Load,
        Store,
        Call,
        Jump,
        JumpNotZero,
        Return,

        Count,
    } type;
    instruction_argument arg;

    static instruction_argument_t s_arguments[Count];
};

using instruction_t = instruction::type_e;

} // namespace furvm

#endif // FURVM_INSTRUCTION_HPP
