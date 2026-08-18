#include "furc/front/lexer.hpp"
#include "furc/front/parser.hpp"
#include "furc/middle/ir.hpp"
#include "furc/middle/ssa.hpp"
#include "furlang/arena.hpp"

int main(void) {
    furlang::arena arena;

    std::string_view content = R"(
        func main(argc: u64) -> s32 {
            x: s32 = 1 + 2 * 3;
            return if (x == 9) 1 else 0;
        }
    )";

    furc::lexer     lexer    = { "<AK>", content };
    furc::parser    parser   = { std::move(lexer), arena };
    furc::ir_module irModule = furc::ir_generator::generate(parser.parse());
    furc::ssa::process(irModule);

    return 0;
}
