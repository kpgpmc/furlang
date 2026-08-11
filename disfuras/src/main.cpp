#include "furvm/instruction.hpp"
#include "furvm/module.hpp"

#include <cassert>
#include <exception>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>

using namespace std::string_literals;

void print_type(const furvm::mod_type& type, const furvm::mod& mod) {
    switch (type.type) {
    case furvm::mod_type::S8: std::cout << "$s8"; return;
    case furvm::mod_type::S16: std::cout << "$s16"; return;
    case furvm::mod_type::S32: std::cout << "$s32"; return;
    case furvm::mod_type::S64: std::cout << "$s64"; return;
    case furvm::mod_type::U8: std::cout << "$u8"; return;
    case furvm::mod_type::U16: std::cout << "$u16"; return;
    case furvm::mod_type::U32: std::cout << "$u32"; return;
    case furvm::mod_type::U64: std::cout << "$u64"; return;
    case furvm::mod_type::Ptr:
        std::cout << "ptr ";
        print_type(*mod.type_at(type.value.typeRef), mod);
        return;
    case furvm::mod_type::Ref:
        std::cout << "ref ";
        print_type(*mod.type_at(type.value.typeRef), mod);
        return;
    case furvm::mod_type::Array:
        std::cout << "array ";
        print_type(*mod.type_at(type.value.array.typeId), mod);
        if (type.value.array.size == 0) {
            std::cout << " dynamic";
        } else {
            std::cout << ' ' << type.value.array.size;
        }
        return;
    case furvm::mod_type::Import: std::cout << "import " << type.value.imprt.modId << "::t" << type.value.imprt.typeId;
    case furvm::mod_type::Count: break;
    }
    assert(false);
}

static const char* typeNames[furvm::instruction::Count] = {
    // NoOperation:
    "nop",
    // PushS8:
    "push $__t0",
    // PushU8:
    "push $__t1",
    // PushS16:
    "push $__t2",
    // PushU16:
    "push $__t3",
    // PushS32:
    "push $__t4",
    // PushU32:
    "push $__t5",
    // PushConstant:
    "push",
    // Array:
    "array",
    // Get:
    "get",
    // Set:
    "set",
    // Drop:
    "drop",
    // Duplicate:
    "dup",
    // Swap:
    "swap",
    // Clone:
    "clone",
    // Reference:
    "ref",
    // Add:
    "add",
    // Sub:
    "sub",
    // Mul:
    "mul",
    // Div:
    "div",
    // Mod:
    "mod",
    // Equals:
    "eq",
    // NotEquals:
    "ne",
    // LessThan:
    "lt",
    // GreaterThan:
    "gt",
    // LessEqual:
    "le",
    // GreaterEqual:
    "ge",
    // Pointerof:
    "pointerof",
    // Sizeof:
    "sizeof",
    // Lengthof:
    "lengthof",
    // Load:
    "load",
    // Store:
    "store",
    // Call:
    "call",
    // Jump:
    "jmp",
    // JumpNotZero:
    "jnz",
    // Return:
    "ret",
};

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <module.fmod>\n";
        return 1;
    }

    try {
        std::ifstream file(argv[1], std::ios::binary | std::ios::in);

        furvm::mod mod;
        try {
            mod = furvm::mod::load(file);
        } catch (std::exception ex) {
            std::cerr << "Failed to load module " << argv[1] << ": " << ex.what() << '\n';
            return 1;
        }

        std::cout << "; Generated with disfuras\n";

        std::cout << "; Types:\n";
        for (const auto& ptr : mod.types()) {
            const auto& [header, type] = *ptr;
            std::cout << "type __t" << header.id() << " = ";
            print_type(type, mod);
            std::cout << '\n';
        }

        std::unordered_map<furvm::function_id, std::string> funcNames;
        std::size_t                                         unnamedCounter = 0;

        std::unordered_map<std::size_t, std::string> labels;
        std::unordered_set<std::size_t>              deadLabels;
        std::size_t                                  labelCounter = 0;

        std::cout << "; Functions:\n";
        for (const auto& ptr : mod.functions()) {
            const auto& [header, func] = *ptr;
            auto it                    = mod.function_map().find(header.id());
            if (it != mod.function_map().end()) {
                std::cout << "public func " << (funcNames[header.id()] = it->second.first);
            } else {
                std::cout << "func " << (funcNames[header.id()] = "__f"s + std::to_string(unnamedCounter++));
            }
            for (const auto& param : func.signature().params) {
                std::cout << " $__t" << param.id();
            }

            std::cout << " = ";
            if (func.signature().returnType.has_value())
                std::cout << "$__t" << func.signature().returnType->id() << ' '; // NOLINT
            switch (func.type()) {
            case furvm::function_t::Normal: {
                auto it = labels.find(func.position());
                if (it == labels.end()) it = labels.emplace(func.position(), funcNames[header.id()]).first;
                deadLabels.insert(func.position());
                std::cout << '#' << it->second;
            } break;
            case furvm::function_t::Native: {
                std::cout << "native " << func.native();
            } break;
            case furvm::function_t::Import: {
                std::cout << "import " << func.imp().mod << "::" << func.imp().function;
            } break;
            }
            std::cout << '\n';
        }

        std::cout << "; Bytecode:\n";
        // Label pass:
        for (std::size_t off = 0; off < mod.bytecode().size();) {
            furvm::instruction instr{};
            off += instr.read(mod.bytecode_view().subview(off));
            if (instr.type != furvm::instruction::Jump && instr.type != furvm::instruction::JumpNotZero) continue;
            if (labels.find(off + instr.arg.s8) == labels.end())
                labels[off + instr.arg.s8] = "__l"s + std::to_string(labelCounter++);
            deadLabels.insert(off + instr.arg.s8);
        }
        // Actual printing pass:
        for (std::size_t off = 0; off < mod.bytecode().size();) {
            if (auto it = labels.find(off); it != labels.end()) {
                std::cout << it->second << ":\n";
                deadLabels.erase(off);
            }
            furvm::instruction instr{};
            off += instr.read(mod.bytecode_view().subview(off));
            std::cout << "    " << typeNames[instr.type];
            switch (instr.arg.type) {
            case furvm::instruction_argument::None: break;
            case furvm::instruction_argument::S8: std::cout << ' ' << std::to_string(instr.arg.s8); break;
            case furvm::instruction_argument::U8: std::cout << ' ' << std::to_string(instr.arg.u8); break;
            case furvm::instruction_argument::S16: std::cout << ' ' << instr.arg.s16; break;
            case furvm::instruction_argument::U16: std::cout << ' ' << instr.arg.u16; break;
            case furvm::instruction_argument::S32: std::cout << ' ' << std::to_string(instr.arg.s8); break;
            case furvm::instruction_argument::U32: std::cout << ' ' << std::to_string(instr.arg.u8); break;
            case furvm::instruction_argument::Constant: throw std::runtime_error("unimplemented");
            case furvm::instruction_argument::Type: std::cout << " $__t" << instr.arg.u16; break;
            case furvm::instruction_argument::Variable: std::cout << " %" << instr.arg.u16; break;
            case furvm::instruction_argument::Function: std::cout << ' ' << funcNames[instr.arg.s16]; break;
            case furvm::instruction_argument::Offset: std::cout << " #" << labels[instr.arg.s16]; break;
            case furvm::instruction_argument::Count: break;
            }
            std::cout << '\n';
        }
        if (!deadLabels.empty()) {
            std::cerr << "Malformed module: invalid jumps\n";
            return 1;
        }
    } catch (const std::exception& ex) {
        std::cerr << "Exception uncaught: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}

// TODO: Disassemble into FIR (furlang's IR)
