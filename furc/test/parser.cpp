#include "furc/front/parser.hpp"

#include "gtest/gtest.h"

namespace {

using namespace furc::front;
using namespace furc::ast;
using namespace std::string_view_literals;

TEST(Parser, EmptyFunctions) {
    parser parser("<TEMP>", "func main() {}\nfunc foo();");
    auto   program = parser.parse();
    EXPECT_TRUE(program.present());
    EXPECT_EQ(program->declarations().size(), 2);
    {
        auto first = program->declarations()[0];
        EXPECT_TRUE(first.present());
        EXPECT_EQ(first->declaration_type(), declaration_node_t::FunctionDefinition);
        node_handle<function_definition_node> funcDef = first;
        EXPECT_EQ(funcDef->name().value, "main");
        EXPECT_EQ(funcDef->body()->statements.size(), 0);
    }
    {
        auto second = program->declarations()[1];
        EXPECT_TRUE(second.present());
        EXPECT_EQ(second->declaration_type(), declaration_node_t::FunctionDeclaration);
        node_handle<function_declarartion_node> funcDecl = second;
        EXPECT_EQ(funcDecl->name().value, "foo");
    }
}

} // namespace