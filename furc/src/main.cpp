#include "furc/front/lexer.hpp"

#include <iostream>

int main(void) {
    furc::front::lexer lexer("<TEMP>", "func main() {\n    return 0;\n}");

    furc::front::token token = {};
    while (!furc::front::is_token_type_empty((token = lexer.next_token()).type)) {
        std::cout << token << '\n';
    }
    if (token.type == furc::front::token_t::Error) std::cout << token << '\n';

    return 0;
}