#include "context.hpp"

#include <furvm/executor.hpp>
#include <iostream>

void context::run() {
    if ((executor->flags() & furvm::executor_flags::Done) != furvm::executor_flags::Done)
        executor->push_frame(mod, *mainFunction);
    while ((executor->flags() & furvm::executor_flags::Done) != furvm::executor_flags::Done && !halt) {
        executor->step();
    }
    if ((executor->flags() & furvm::executor_flags::Done) == furvm::executor_flags::Done) {
        std::cout << "Execution finished\n";
    }
}
