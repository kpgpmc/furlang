#include "furc/front/lexer.hpp"
#include "furc/front/parser.hpp"
#include "furlang/arena.hpp"

int main(void) {
    furlang::arena arena;

    furc::lexer  lexer  = { "<AK>", "func main(argc: u64) -> s32 { x: s32 = '\\\\'; }" };
    furc::parser parser = { std::move(lexer), arena };

    auto program = parser.parse();

    return 0;
}
