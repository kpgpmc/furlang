#ifndef LIBFURVM

#include "furvm/furvm.hpp"

#include <array>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <memory>

static constexpr std::array<furvm::byte, 9> s_bytecode = {
    furvm::byte(furvm::instruction_t::PushB2I),
    67,
    furvm::byte(furvm::instruction_t::Duplicate),
    furvm::byte(furvm::instruction_t::Reference),
    furvm::byte(furvm::instruction_t::Add),
    furvm::byte(furvm::instruction_t::Call),
    1,
    0,
    furvm::byte(furvm::instruction_t::Return),
};

int main(void) {
    auto context = std::make_shared<furvm::context>();

    furvm::mod_h      furlangModule = context->emplace_module("furlang");
    furvm::function_h printFunc     = furlangModule->emplace_function("print", 1, "print");
    furlangModule->set_native_function("print",
        [](furvm::executor& executor) { std::cout << executor.load_thing(0)->integer() << '\n'; });

    furvm::mod_h      mainModule = context->emplace_module("main", s_bytecode.begin(), s_bytecode.end());
    furvm::function_h mainFunc   = mainModule->emplace_function("main", 0, 0);
    mainModule->emplace_function(furlangModule, printFunc).dispatch();

    furvm::executor_h executor = context->emplace_executor(context);
    executor->push_frame(mainModule, *mainFunc);

    while ((executor->flags() & furvm::executor_flags::Done) != furvm::executor_flags::Done) {
        executor->step();
    }

    return 0;
}

#endif // LUBFURVM
