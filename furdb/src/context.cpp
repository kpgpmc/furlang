#include "context.hpp"

#include <furvm/executor.hpp>
#include <furvm/instruction.hpp>
#include <iostream>
#include <stdexcept>

void context::run() {
    if ((executor->flags() & furvm::executor_flags::Done) != furvm::executor_flags::Done)
        executor->push_frame(mod, *mainFunction);
    while ((executor->flags() & furvm::executor_flags::Done) != furvm::executor_flags::Done && !halt) {
        executor->step();
    }
    if ((executor->flags() & furvm::executor_flags::Done) == furvm::executor_flags::Done) {
        std::cout << "Execution finished\n";
    }
}

void context::print_instruction() const {
    static const char* s_instrNames[furvm::instruction::Count] = {
        // NoOperation
        "nop",
        // PushS8
        "push $s8",
        // PushU8
        "push $u8",
        // PushS16
        "push $s16",
        // PushU16
        "push $U16",
        // PushS32
        "push $S32",
        // PushU32
        "push $U32",
        // PushConstant
        "push",
        // Array
        "array",
        // Get
        "get",
        // Set
        "set",
        // Drop
        "drop",
        // Duplicate
        "dup",
        // Swap
        "swap",
        // Clone
        "clone",
        // Reference
        "ref",
        // Add
        "add",
        // Sub
        "sub",
        // Mul
        "mul",
        // Div
        "div",
        // Mod
        "mod",
        // Equals
        "eq",
        // NotEquals
        "ne",
        // LessThan
        "lt",
        // GreaterThan
        "gt",
        // LessEqual
        "le",
        // GreaterEqual
        "ge",
        // Pointerof
        "pointerof",
        // Sizeof
        "sizeof",
        // Lengthof
        "lenof",
        // Load
        "load",
        // Store
        "store",
        // LoadGlobal
        "loadg",
        // StoreGlobal
        "storeg",
        // Call
        "call",
        // Jump
        "jmp",
        // JumpNotZero
        "jnz",
        // Return
        "ret",
    };

    furvm::instruction instr{};
    instr.read(mod->bytecode_view().subview(executor->top_frame().position));
    std::cout << s_instrNames[instr.type];
    if (instr.arg.type != furvm::instruction_argument_t::None) {
        std::cout << ' ';
        switch (instr.arg.type) {
        case furvm::instruction_argument::S8:
        case furvm::instruction_argument::U8:
        case furvm::instruction_argument::S16:
        case furvm::instruction_argument::U16:
        case furvm::instruction_argument::S32:
        case furvm::instruction_argument::U32:
        case furvm::instruction_argument::None:
        case furvm::instruction_argument::Count: break;
        case furvm::instruction_argument::Constant: throw std::runtime_error("unimplemented");
        case furvm::instruction_argument::Type: std::cout << "$__t" << instr.arg.u32; break;
        case furvm::instruction_argument::Variable: std::cout << "%" << instr.arg.u16; break;
        case furvm::instruction_argument::GlobalVariable: std::cout << "%__g" << instr.arg.u16; break;
        case furvm::instruction_argument::Function: std::cout << "<a function>"; break;
        case furvm::instruction_argument::Offset: std::cout << instr.arg.s8; break;
        }
    }
    std::cout << '\n';
}
