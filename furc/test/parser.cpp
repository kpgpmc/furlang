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
        EXPECT_EQ(funcDef->name()->string, "main");
        EXPECT_EQ(funcDef->body()->statements.size(), 0);
    }
    {
        auto second = program->declarations()[1];
        EXPECT_TRUE(second.present());
        EXPECT_EQ(second->declaration_type(), declaration_node_t::FunctionDeclaration);
        node_handle<function_declarartion_node> funcDecl = second;
        EXPECT_EQ(funcDecl->name()->string, "foo");
    }
}

TEST(Parser, Literals) {
    parser parser("<TEMP>", R"(
    func test1() { return 67; }
    func test2() { return "uwu"; }
    )");
    auto   program = parser.parse();
    EXPECT_TRUE(program.present());
    EXPECT_EQ(program->declarations().size(), 2);
    {
        auto test1 = program->declarations()[0];
        EXPECT_TRUE(test1.present());
        EXPECT_EQ(test1->declaration_type(), declaration_node_t::FunctionDefinition);
        node_handle<function_definition_node> funcDef = test1;
        EXPECT_EQ(funcDef->name()->string, "test1");
        EXPECT_EQ(funcDef->body()->statements.size(), 1);
        node_handle<return_statement_node> ret = funcDef->body()->statements[0];
        EXPECT_EQ(ret->value(), integer_literal_node({ furc::location{ "<TEMP>", 1, 26 }, 67 }));
    }
    {
        auto test2 = program->declarations()[1];
        EXPECT_TRUE(test2.present());
        EXPECT_EQ(test2->declaration_type(), declaration_node_t::FunctionDefinition);
        node_handle<function_definition_node> funcDecl = test2;
        EXPECT_EQ(funcDecl->name()->string, "test2");
    }
}

} // namespace