// #ifndef LIBFURC

#include "furc/front/ir_generator.hpp"
#include "furc/front/parser.hpp"

#include <iostream>

int main(void) {
    furc::front::parser       parser("<TEMP>", "func main() {\n    return 7 + 6 * 10;\n}");
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
        }
    }

    return 0;
}

// #endif // LIBFURC