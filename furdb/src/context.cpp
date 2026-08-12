#include "context.hpp"

void context::run() {
    if ((executor->flags() & furvm::executor_flags::Done) != furvm::executor_flags::Done)
        executor->push_frame(mod, *mainFunction);
    while ((executor->flags() & furvm::executor_flags::Done) != furvm::executor_flags::Done) {
        executor->step();
    }
}
