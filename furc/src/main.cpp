#ifndef LIBFURC

#include "furc/front/parser.hpp"

#include <iostream>

int main(void) {
    furc::front::parser parser("<TEMP>", "func main() {\n    return;\n}");
    std::cout << parser.parse() << '\n';

    return 0;
}

#endif // LIBFURC