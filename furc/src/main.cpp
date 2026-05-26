#ifndef LIBFURC

#include "furc/front/parser.hpp"

#include <iostream>

int main(void) {
    furc::front::parser parser("<TEMP>", "func main() {\n    return 67;return \"uwu\";\n}");
    std::cout << parser.parse() << '\n';

    return 0;
}

#endif // LIBFURC