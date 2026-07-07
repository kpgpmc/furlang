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
    switch (thing.type()->t) {
    case furvm::type_t::Primitive: {
        std::cout << thing.integer();
    } break;
    case furvm::type_t::Array: {
        std::cout << "{ ";
        for (furvm::long_t i = 0; i < thing.size(); ++i) {
            if (i > 0) std::cout << ", ";
            print_thing(thing.at(i));
        }
        std::cout << " }\n";
    } break;
    default: std::cerr << "{Something went wrong}";
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
        0,
        0,
        0,
        0,
        static_cast<furvm::byte>(furvm::instruction_t::Reference),
        static_cast<furvm::byte>(furvm::instruction_t::Call),
        1,
        0,
        static_cast<furvm::byte>(furvm::instruction_t::Return),
    };

    furvm::mod_h mod = context->emplace("main", s_bytecode, s_bytecode + sizeof(s_bytecode));
    mod->emplace_type(std::make_shared<furvm::type>(context->at("core")->type_at(2), 10)).dispatch();
    auto mainFunc = mod->emplace_function_named("main", 0, 0);
    mod->emplace_function(1, "print").dispatch();
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
