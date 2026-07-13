#include "furas/lexer.hpp"

#include <iostream>

int main(void) { // NOLINT
    std::string  source = "%uwu 10 public private int le";
    furas::lexer lexer("<NONE>", source);
    while (true) {
        furas::token_r token = lexer.next_token();
        if (token.has_error()) {
            std::cout << token.error().message << '\n';
            break;
        }
        std::cout << token->type << '\n';
    }

    return 0;
}
