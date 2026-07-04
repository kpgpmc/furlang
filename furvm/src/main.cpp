#ifndef LIBFURVM

#include "furvm/detail/serialization.hpp"
#include "furvm/furvm.hpp"

#include <array>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <mod.fmod>\n";
        return 1;
    }

    auto context = std::make_shared<furvm::context>();

    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << argv[1] << '\n';
        return 1;
    }

    furvm::mod_h      mod  = context->emplace("main", furvm::mod::load(file));
    furvm::function_h func = mod->function_at("main");

    mod->set_native_function("print",
        [](furvm::executor& executor) { std::cout << executor.load_thing(0)->integer() << '\n'; });

    furvm::executor_h executor = context->emplace_executor(context);
    executor->push_frame(mod, *func);

    while ((executor->flags() & furvm::executor_flags::Done) != furvm::executor_flags::Done) {
        executor->step();
    }

    return 0;
}

#endif // LUBFURVM
