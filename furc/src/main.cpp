#include "furc/front/lexer.hpp"
#include "furc/front/parser.hpp"
#include "furlang/arena.hpp"

int main(void) {
    furlang::arena arena;

    std::string_view content = R"(
        func main(argc: u64) -> s32 pre(arc > 1) {
            x: s32 = 1 + 2 * 3;
            println(x);
            return if (x == 9) 1 else 0;
        }
    )";

    furc::lexer  lexer  = { "<AK>", content };
    furc::parser parser = { std::move(lexer), arena };

    auto program = parser.parse();

    return 0;
}
