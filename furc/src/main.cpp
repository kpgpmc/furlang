#ifndef LIBFURC

#include "furc/front/ir_generator.hpp"
#include "furc/front/parser.hpp"

#include <iostream>

int main(void) {
    std::string               programStr = R"(
    func main() {
        x = 5;
        x -= 3;
        if (x < 3) {
            y = x * 2;
            w = y;
        } else {
            y = x - 3;
        }
        w = x - y;
        z = x + y;
    }
    )";
    furc::front::parser       parser("<TEMP>", programStr);
    furc::front::ir_generator generator;

    auto program = parser.parse();
    if (program.has_error()) {
        std::cerr << program << '\n';
    }
    program->accept(generator);

    auto module = std::move(generator.move_module());
    std::cout << "Generated IR:\n";
    for (const auto& function : module.functions()) {
        std::cout << function->name() << ":\n";
        furlang::ir::block_index blockIndex = 0;
        for (const auto& block : function->blocks()) {
            std::cout << "  # block " << blockIndex++ << '\n';
            for (const auto& instruction : block->instructions()) {
                std::cout << "  " << *instruction << '\n';
            }
            std::cout << "  " << *block->exit() << '\n';
        }
    }

    return 0;
}

#endif // LIBFURC
