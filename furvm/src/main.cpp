#ifndef LIBFURVM

#include "furvm/context.hpp"
#include "furvm/instruction.hpp"

#include <array>
#include <cstddef>

static constexpr std::array<furvm::byte, 1> s_bytecode = {
    furvm::byte(furvm::instruction_t::NoOperation),
};

int main(void) {
    furvm::context context;

    auto mainModule = context.emplace(s_bytecode.begin(), s_bytecode.end());

    return 0;
}

#endif // LUBFURVM