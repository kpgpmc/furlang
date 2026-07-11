#ifndef LIBFURVM

#include "furvm/detail/serialization.hpp"
#include "furvm/furvm.hpp"

#include <array>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>

static void print_thing(const furvm::thing<furvm::thing_allocator>& thing) {
    using namespace furvm;

    switch (thing.type().type) {
    case thing_type::S8: std::cout << thing.cast_to<thing_type::s16>(); break;
    case thing_type::S16: std::cout << thing.get<thing_type::s16>(); break;
    case thing_type::S32: std::cout << thing.get<thing_type::s32>(); break;
    case thing_type::S64: std::cout << thing.get<thing_type::s64>(); break;
    case thing_type::U8: std::cout << thing.get<thing_type::u8>(); break;
    case thing_type::U16: std::cout << thing.get<thing_type::u16>(); break;
    case thing_type::U32: std::cout << thing.get<thing_type::u32>(); break;
    case thing_type::U64: std::cout << thing.get<thing_type::u64>(); break;
    case furvm::thing_type::Array:
        std::cout << "{ ";
        for (thing_type::u64 i = 0; i < thing.size(); ++i) {
            if (i > 0) std::cout << ", ";
            print_thing(thing.at(i));
        }
        std::cout << " }";
        break;
    default: std::cerr << "{Type not recognized}";
    }
}

int main(int argc, char** argv) {
    auto context = std::make_shared<furvm::context>();

#if 0 // NOLINT
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <mod.fmod>\n";
        return 1;
    }


    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << argv[1] << '\n';
        return 1;
    }

    furvm::mod_h mod = context->emplace("main", std::move(furvm::mod::load(file)));
#else
    static const furvm::byte s_bytecode[] = {
        static_cast<furvm::byte>(furvm::instruction_t::Array),
        1,
        0,
        0,
        0,
        static_cast<furvm::byte>(furvm::instruction_t::Pointerof),
        static_cast<furvm::byte>(furvm::instruction_t::Call),
        1,
        0,
        static_cast<furvm::byte>(furvm::instruction_t::Return),
    };

    auto mod       = context->emplace("main", s_bytecode, s_bytecode + sizeof(s_bytecode));
    auto charType  = mod->emplace_type(furvm::mod_type::S8);
    auto arrayType = mod->emplace_type(charType.id(), 10);
    auto mainFunc  = mod->emplace_function("main", furvm::function_sig{}, 0);
    mod->emplace_function(furvm::function_sig{ { arrayType } }, "print").dispatch();
#endif
    mod->set_native_function("print", [](furvm::executor& executor) {
        print_thing(*executor.load_thing(0));
        std::cout << '\n';
    });

    furvm::executor_h executor = context->emplace_executor(context);
    executor->push_frame(mod, *mainFunc);

    while ((executor->flags() & furvm::executor_flags::Done) != furvm::executor_flags::Done) {
        executor->step();
    }

    return 0;
}

#endif // LUBFURVM
