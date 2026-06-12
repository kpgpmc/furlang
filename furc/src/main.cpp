#ifndef LIBFURC

#include "furc/ast/program.hpp"
#include "furc/front/ir_generator.hpp"
#include "furc/front/parser.hpp"
#include "furc/front/ssa.hpp"

#include <iostream>

int main(void) {
    try {
        std::string               programStr = R"(
    func main() {
        x = 0;
        y = 10;
        z = 1;
        while (x < y) {
            x = x + z;
        }
    }
    )";
        furc::front::parser       parser("<TEMP>", programStr);
        furc::front::ir_generator generator;

        auto programResult = parser.parse();
        if (programResult.has_error()) {
            std::cerr << programResult.error() << '\n';
            return 1;
        }
        const auto& program = *programResult;
        program->accept(generator);

        auto module = std::move(generator.move_module());
        furc::front::ssa::optimize(module);

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
    } catch (...) {
        std::cerr << "Caught an exception in main!\n";
        return 1;
    }
}

#endif // LIBFURC
