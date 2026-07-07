#ifndef LIBFURC

#include "furc/ast/program.hpp"
#include "furc/back/furvm.hpp"
#include "furc/front/ir_generator.hpp"
#include "furc/front/parser.hpp"
#include "furc/front/post_process.hpp"
#include "furlang/arena.hpp"

#include <fstream>
#include <furvm/furvm.hpp>
#include <iostream>

int main(void) {
    try {
        std::string               programStr = R"(
    private native func print(value: int32);

    func main() -> int32 {
        x = 0;
        y = 10;
        z = 1;
        while (x < y) {
            x = x + z;
        }
        print(sizeof x);
    }
    )";
        furlang::arena            arena{};
        furc::front::parser       parser(arena, "<TEMP>", programStr);
        furc::front::ir_generator generator;

        auto programResult = parser.parse();
        if (programResult.has_error()) {
            std::cerr << programResult.error() << '\n';
            return 1;
        }
        const auto& program = *programResult;
        program->accept(generator);

        auto mod = std::move(generator.move_module());

        furc::front::post_process postProcess;
        postProcess.push_stage(furc::front::post_process::Ssa);
        postProcess.push_stage(furc::front::post_process::Sccp);
        postProcess.push_stage(furc::front::post_process::Adce);
        postProcess.push_stage(furc::front::post_process::DeSsa);
        postProcess.process(mod);

        std::cout << "Generated IR:\n";
        for (const auto& function : mod.functions()) {
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

        auto context  = std::make_shared<furvm::context>();
        auto furvmMod = context->emplace("main", furc::back::furvm_generator::generate(mod));

        std::ofstream file("./a.fmod", std::ios::binary);
        furvmMod->serialize(file);
        file.close();

        furvmMod->set_native_function("print",
            [](furvm::executor& executor) { std::cout << executor.load_thing(0)->integer() << '\n'; });

        furvm::executor_h executor = context->emplace_executor(context);
        executor->push_frame(furvmMod, *furvmMod->function_at("main", furvm::function_sig{}));

        std::cout << "--- Interpreting:\n";

        while ((executor->flags() & furvm::executor_flags::Done) != furvm::executor_flags::Done) {
            executor->step();
        }

        return 0;
    } catch (...) {
        std::cerr << "Caught an exception in main!\n";
        return 1;
    }
}

#endif // LIBFURC
