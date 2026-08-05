#include "furc/front/lexer.hpp"

#include <iostream>

int main(void) {
    furc::lexer lexer = { "<AK>", "func main(argc: u64) -> s32 { return '\\\\'; }" };
    while (true) {
        furc::token token = lexer.next_token();
        std::cout << token.loc.filepath << ':' << token.loc.row + 1 << ':' << token.loc.col + 1 << ": " << token
                  << '\n';
        switch (token.type) {
        case furc::token::UnexpectedCharacter:
        case furc::token::UnexpectedEOF: return 1;
        case furc::token::EndOfFile: return 0;
        default: break;
        }
    }
}
