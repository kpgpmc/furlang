#ifndef LIBFURVM

#include "furvm/context.hpp"

int main(void) {
    furvm::context context;

    auto mainModule = context.emplace();

    return 0;
}

#endif // LUBFURVM