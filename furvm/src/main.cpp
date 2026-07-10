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
    switch (thing.type().type) {
    case furvm::thing_type::Primitive: {
        std::cout << thing.integer();
    } break;
    case furvm::thing_type::Array: {
        std::cout << "{ ";
        for (furvm::thing<>::s64 i = 0; i < thing.size(); ++i) {
            if (i > 0) std::cout << ", ";
            print_thing(thing.at(i));
        }
        std::cout << " }\n";
    } break;
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
        static_cast<furvm::byte>(furvm::instruction_t::Reference),
        static_cast<furvm::byte>(furvm::instruction_t::Call),
        1,
        0,
        static_cast<furvm::byte>(furvm::instruction_t::Return),
    };

    auto mod       = context->emplace("main", s_bytecode, s_bytecode + sizeof(s_bytecode));
    auto intType   = mod->emplace_type(sizeof(int));
    auto arrayType = mod->emplace_type(intType.id(), 10);
    auto mainFunc  = mod->emplace_function("main", furvm::function_sig{}, 0);
    mod->emplace_function(furvm::function_sig{ { arrayType } }, "print").dispatch();
#endif
    mod->set_native_function("print", [](furvm::executor& executor) { print_thing(*executor.load_thing(0)); });

    furvm::executor_h executor = context->emplace_executor(context);
    executor->push_frame(mod, *mainFunc);

    while ((executor->flags() & furvm::executor_flags::Done) != furvm::executor_flags::Done) {
        executor->step();
    }

    return 0;
}

#endif // LUBFURVM
