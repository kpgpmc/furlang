#include "furc/front/parser.hpp"

#include "furc/ast/declaration.hpp" // IWYU pragma: keep
#include "furc/ast/expression.hpp"  // IWYU pragma: keep
#include "furc/ast/literal.hpp"     // IWYU pragma: keep
#include "furc/ast/program.hpp"     // IWYU pragma: keep
#include "furc/ast/statement.hpp"   // IWYU pragma: keep

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
        function_definition_node_h funcDef = first;
        EXPECT_EQ(funcDef->name()->string, "main");
        EXPECT_EQ(funcDef->body()->statements.size(), 0);
    }
    {
        auto second = program->declarations()[1];
        EXPECT_TRUE(second.present());
        EXPECT_EQ(second->declaration_type(), declaration_node_t::FunctionDeclaration);
        function_declaration_node_h funcDecl = second;
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
        function_definition_node_h funcDef = test1;
        EXPECT_EQ(funcDef->name()->string, "test1");
        EXPECT_EQ(funcDef->body()->statements.size(), 1);
        return_statement_node_h ret = funcDef->body()->statements[0];
        EXPECT_EQ(ret->value(), integer_literal_node({ furc::location{ "<TEMP>", 1, 26 }, 67 }));
    }
    {
        auto test2 = program->declarations()[1];
        EXPECT_TRUE(test2.present());
        EXPECT_EQ(test2->declaration_type(), declaration_node_t::FunctionDefinition);
        function_definition_node_h funcDecl = test2;
        EXPECT_EQ(funcDecl->name()->string, "test2");
    }
}

#define EXPECT_INTLIT(expr, integer)                                                                                   \
    do {                                                                                                               \
        EXPECT_EQ((expr)->expression_type(), expression_node_t::Literal);                                              \
        literal_node_h literal = (expr);                                                                               \
        EXPECT_EQ(literal->literal_type(), literal_node_t::Integer);                                                   \
        integer_literal_node_h intLit = literal;                                                                       \
        EXPECT_EQ(*intLit, (integer));                                                                                 \
    } while (0)

#define EXPECT_VARREAD(expr, varname)                                                                                  \
    do {                                                                                                               \
        EXPECT_EQ((expr)->expression_type(), expression_node_t::VarRead);                                              \
        var_read_expression_node_h varRead = (expr);                                                                   \
        EXPECT_EQ(varRead->get_name(), (varname));                                                                     \
    } while (0)

// TODO: Use arena (I am too exhausted rn to do it)
TEST(Parser, OperatorPrecedence_AddMul) {
    parser parser("<TEMP>", "func main() { return 1 + 2 * 3; }");
    auto   program = parser.parse();
    EXPECT_TRUE(program.present());
    EXPECT_EQ(program->declarations().size(), 1);
    auto func = program->declarations()[0];
    EXPECT_TRUE(func.present());
    EXPECT_EQ(func->declaration_type(), declaration_node_t::FunctionDefinition);
    function_definition_node_h funcDef = func;
    EXPECT_EQ(funcDef->name()->string, "main");
    EXPECT_EQ(funcDef->body()->statements.size(), 1);
    return_statement_node_h ret = funcDef->body()->statements[0];

    auto retVal = ret->value();
    EXPECT_TRUE(retVal.present());

    EXPECT_EQ(retVal->expression_type(), expression_node_t::Binop);
    binop_expression_node_h add = retVal;
    EXPECT_EQ(add->type(), binop_expression_node_t::Add);
    EXPECT_INTLIT(add->lhs(), 1);

    EXPECT_EQ(add->rhs()->expression_type(), expression_node_t::Binop);
    binop_expression_node_h mul = add->rhs();
    EXPECT_EQ(mul->type(), binop_expression_node_t::Mul);
    EXPECT_INTLIT(mul->lhs(), 2);
    EXPECT_INTLIT(mul->rhs(), 3);
}

TEST(Parser, OperatorPrecedence_Complex) {
    parser parser("<TEMP>", "func main() { return 1 + 2 * 3 - 4 / 2; }");
    auto   program = parser.parse();
    EXPECT_TRUE(program.present());
    EXPECT_EQ(program->declarations().size(), 1);
    auto func = program->declarations()[0];
    EXPECT_TRUE(func.present());
    EXPECT_EQ(func->declaration_type(), declaration_node_t::FunctionDefinition);
    function_definition_node_h funcDef = func;
    EXPECT_EQ(funcDef->name()->string, "main");
    EXPECT_EQ(funcDef->body()->statements.size(), 1);
    return_statement_node_h ret = funcDef->body()->statements[0];

    auto retVal = ret->value();
    EXPECT_TRUE(retVal.present());

    EXPECT_EQ(retVal->expression_type(), expression_node_t::Binop);
    binop_expression_node_h sub = retVal;
    EXPECT_EQ(sub->type(), binop_expression_node_t::Sub);

    EXPECT_EQ(sub->lhs()->expression_type(), expression_node_t::Binop);
    binop_expression_node_h add = sub->lhs();
    EXPECT_EQ(add->type(), binop_expression_node_t::Add);
    EXPECT_INTLIT(add->lhs(), 1);

    EXPECT_EQ(add->rhs()->expression_type(), expression_node_t::Binop);
    binop_expression_node_h mul = add->rhs();
    EXPECT_EQ(mul->type(), binop_expression_node_t::Mul);
    EXPECT_INTLIT(mul->lhs(), 2);
    EXPECT_INTLIT(mul->rhs(), 3);

    EXPECT_EQ(sub->rhs()->expression_type(), expression_node_t::Binop);
    binop_expression_node_h div = sub->rhs();
    EXPECT_EQ(div->type(), binop_expression_node_t::Div);
    EXPECT_INTLIT(div->lhs(), 4);
    EXPECT_INTLIT(div->rhs(), 2);
}

TEST(Parser, UnaryOperator_Simple) {
    parser parser("<TEMP>", "func main() { return -5; }");
    auto   program = parser.parse();
    EXPECT_TRUE(program.present());
    EXPECT_EQ(program->declarations().size(), 1);
    auto func = program->declarations()[0];
    EXPECT_TRUE(func.present());
    EXPECT_EQ(func->declaration_type(), declaration_node_t::FunctionDefinition);
    function_definition_node_h funcDef = func;
    EXPECT_EQ(funcDef->name()->string, "main");
    EXPECT_EQ(funcDef->body()->statements.size(), 1);
    return_statement_node_h ret = funcDef->body()->statements[0];

    auto retVal = ret->value();
    EXPECT_TRUE(retVal.present());

    EXPECT_EQ(retVal->expression_type(), expression_node_t::Unaryop);
    unaryop_expression_node_h neg = retVal;
    EXPECT_EQ(neg->type(), unaryop_expression_node_t::Negative);
    EXPECT_INTLIT(neg->get_node(), 5);
}

TEST(Parser, UnaryOperator_PrePost) {
    parser parser("<TEMP>", "func main() { return --5++; }");
    auto   program = parser.parse();
    EXPECT_TRUE(program.present());
    EXPECT_EQ(program->declarations().size(), 1);
    auto func = program->declarations()[0];
    EXPECT_TRUE(func.present());
    EXPECT_EQ(func->declaration_type(), declaration_node_t::FunctionDefinition);
    function_definition_node_h funcDef = func;
    EXPECT_EQ(funcDef->name()->string, "main");
    EXPECT_EQ(funcDef->body()->statements.size(), 1);
    return_statement_node_h ret = funcDef->body()->statements[0];

    auto retVal = ret->value();
    EXPECT_TRUE(retVal.present());

    EXPECT_EQ(retVal->expression_type(), expression_node_t::Unaryop);
    unaryop_expression_node_h inc = retVal;
    EXPECT_EQ(inc->type(), unaryop_expression_node_t::PostfixIncrement);

    EXPECT_EQ(inc->get_node()->expression_type(), expression_node_t::Unaryop);
    unaryop_expression_node_h dec = inc->get_node();
    EXPECT_INTLIT(dec->get_node(), 5);
}

TEST(Parser, Paren) {
    parser parser("<TEMP>", "func main() { return --(x++); }");
    auto   program = parser.parse();
    EXPECT_TRUE(program.present());
    EXPECT_EQ(program->declarations().size(), 1);
    auto func = program->declarations()[0];
    EXPECT_TRUE(func.present());
    EXPECT_EQ(func->declaration_type(), declaration_node_t::FunctionDefinition);
    function_definition_node_h funcDef = func;
    EXPECT_EQ(funcDef->name()->string, "main");
    EXPECT_EQ(funcDef->body()->statements.size(), 1);
    return_statement_node_h ret = funcDef->body()->statements[0];

    auto retVal = ret->value();
    EXPECT_TRUE(retVal.present());

    EXPECT_EQ(retVal->expression_type(), expression_node_t::Unaryop);
    unaryop_expression_node_h dec = retVal;
    EXPECT_EQ(dec->type(), unaryop_expression_node_t::PrefixDecrement);

    EXPECT_EQ(dec->get_node()->expression_type(), expression_node_t::Unaryop);
    unaryop_expression_node_h inc = dec->get_node();
    EXPECT_EQ(inc->type(), unaryop_expression_node_t::PostfixIncrement);
    EXPECT_VARREAD(inc->get_node(), "x");
}

TEST(Parser, Assignment) {
    parser parser("<TEMP>", "func main() { x = 10; }");
    auto   program = parser.parse();
    EXPECT_TRUE(program.present());
    EXPECT_EQ(program->declarations().size(), 1);
    auto func = program->declarations()[0];
    EXPECT_TRUE(func.present());
    EXPECT_EQ(func->declaration_type(), declaration_node_t::FunctionDefinition);
    function_definition_node_h funcDef = func;
    EXPECT_EQ(funcDef->name()->string, "main");
    EXPECT_EQ(funcDef->body()->statements.size(), 1);

    EXPECT_EQ(funcDef->body()->statements[0]->statement_type(), statement_node_t::Expression);
    expression_node_h expr = funcDef->body()->statements[0];

    EXPECT_EQ(expr->expression_type(), expression_node_t::VarAssign);
    var_assign_expression_node_h assign = expr;
    EXPECT_EQ(assign->compound(), binop_expression_node_t::None);

    expression_node_h lhs = assign->lhs();
    EXPECT_EQ(lhs->expression_type(), expression_node_t::VarRead);
    var_read_expression_node_h varRead = lhs;
    EXPECT_EQ(varRead->get_name(), "x");

    expression_node_h rhs = assign->rhs();
    EXPECT_EQ(rhs->expression_type(), expression_node_t::Literal);
    EXPECT_INTLIT(rhs, 10);
}

TEST(Parser, CompoundAssignment) {
    parser parser("<TEMP>", "func main() { x += 10; }");
    auto   program = parser.parse();
    EXPECT_TRUE(program.present());
    EXPECT_EQ(program->declarations().size(), 1);
    auto func = program->declarations()[0];
    EXPECT_TRUE(func.present());
    EXPECT_EQ(func->declaration_type(), declaration_node_t::FunctionDefinition);
    function_definition_node_h funcDef = func;
    EXPECT_EQ(funcDef->name()->string, "main");
    EXPECT_EQ(funcDef->body()->statements.size(), 1);

    EXPECT_EQ(funcDef->body()->statements[0]->statement_type(), statement_node_t::Expression);
    expression_node_h expr = funcDef->body()->statements[0];

    EXPECT_EQ(expr->expression_type(), expression_node_t::VarAssign);
    var_assign_expression_node_h assign = expr;
    EXPECT_EQ(assign->compound(), binop_expression_node_t::Add);

    expression_node_h lhs = assign->lhs();
    EXPECT_EQ(lhs->expression_type(), expression_node_t::VarRead);
    var_read_expression_node_h varRead = lhs;
    EXPECT_EQ(varRead->get_name(), "x");

    expression_node_h rhs = assign->rhs();
    EXPECT_EQ(rhs->expression_type(), expression_node_t::Literal);
    EXPECT_INTLIT(rhs, 10);
}

} // namespace