#ifndef LIBFURVM

#include "furvm/context.hpp"
#include "furvm/executor.hpp"
#include "furvm/function.hpp"
#include "furvm/instruction.hpp"
#include "furvm/serializer.hpp"
#include "furvm/thing.hpp"

#include <array>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <memory>

static constexpr std::array<furvm::byte, 8> s_bytecode = {
    furvm::byte(furvm::instruction_t::PushB2I),
    67,
    furvm::byte(furvm::instruction_t::Clone),
    furvm::byte(furvm::instruction_t::Add),
    furvm::byte(furvm::instruction_t::Return),
};

int main(void) {
    auto context = std::make_shared<furvm::context>();

    auto mainModule = context->emplace_module("main", s_bytecode.begin(), s_bytecode.end());
    mainModule->emplace_function("main", 0);

    auto executor = context->emplace_executor(context);
    executor->push_frame(mainModule, 0);

    static constexpr std::size_t FPC = 3; // Frames per collection

    std::size_t count = 0;
    while ((executor->flags() & furvm::executor_flags::Done) != furvm::executor_flags::Done) {
        executor->step();
        if ((++count % FPC) == 0) context->collect();
    }

    return 0;
}

#endif // LUBFURVM
