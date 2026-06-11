#ifndef LIBFURVM

#include "furvm/context.hpp"
#include "furvm/executor.hpp"
#include "furvm/function.hpp"
#include "furvm/instruction.hpp"
#include "furvm/thing.hpp"

#include <array>
#include <cstddef>
#include <iostream>
#include <memory>

static constexpr std::array<furvm::byte, 6> s_bytecode = {
    furvm::byte(furvm::instruction_t::PushB2I),
    67,
    furvm::byte(furvm::instruction_t::Clone),
    furvm::byte(furvm::instruction_t::Add),
    furvm::byte(furvm::instruction_t::Drop),
    furvm::byte(furvm::instruction_t::Return),
};

void print(const furvm::executor_p& exec) {
    std::cout << exec->pop_thing()->int32() << '\n';
}

int main(void) {
    auto context = std::make_shared<furvm::context>();

    auto mainModule = context->emplace(s_bytecode.begin(), s_bytecode.end());

    auto mainFunction = furvm::function::create(mainModule, 0);

    auto printFunction = furvm::function::create(mainModule, print);

    auto executor = furvm::executor::create(context);
    executor->push_frame(mainFunction);

    static constexpr std::size_t FPC = 3; // Frames per collection

    std::size_t count = 0;
    while ((executor->flags() & furvm::executor_flags::Done) != furvm::executor_flags::Done) {
        executor->step();
        if ((++count % FPC) == 0) context->collect();
    }

    return 0;
}

#endif // LUBFURVM