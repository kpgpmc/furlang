#ifndef LIBFURVM

#include "furvm/context.hpp"
#include "furvm/executor.hpp"
#include "furvm/instruction.hpp"

#include <array>
#include <cstddef>
#include <memory>

static constexpr std::array<furvm::byte, 4> s_bytecode = {
    furvm::byte(furvm::instruction_t::PushB2I),
    67,
    furvm::byte(furvm::instruction_t::Drop),
    furvm::byte(furvm::instruction_t::Return),
};

int main(void) {
    auto context = std::make_shared<furvm::context>();

    auto mainModule = context->emplace(s_bytecode.begin(), s_bytecode.end());

    auto executor = furvm::executor::create(context);
    executor->push_frame(mainModule, 0);

    while ((executor->flags() & furvm::executor_flags::Done) != furvm::executor_flags::Done) {
        executor->step();
    }

    return 0;
}

#endif // LUBFURVM