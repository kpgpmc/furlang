#include "furc/front/lexer.hpp"

#include "gtest/gtest.h"

namespace {

using namespace furc::front;
using namespace std::string_view_literals;

#define EXPECT_TOKEN(lexer, t) EXPECT_EQ((lexer).next_token(), (t));

TEST(Lexer, Tokens) {
    lexer lexer("<TEMP>", "()\n\t\t{\n}[];    :,.main return func");
    EXPECT_TOKEN(lexer, (token{ token_t::Lparen, "("sv }));
    EXPECT_TOKEN(lexer, (token{ token_t::Rparen, ")"sv }));
    EXPECT_TOKEN(lexer, (token{ token_t::Lbrace, "{"sv }));
    EXPECT_TOKEN(lexer, (token{ token_t::Rbrace, "}"sv }));
    EXPECT_TOKEN(lexer, (token{ token_t::Lbracket, "["sv }));
    EXPECT_TOKEN(lexer, (token{ token_t::Rbracket, "]"sv }));
    EXPECT_TOKEN(lexer, (token{ token_t::Semicolon, ";"sv }));
    EXPECT_TOKEN(lexer, (token{ token_t::Colon, ":"sv }));
    EXPECT_TOKEN(lexer, (token{ token_t::Comma, ","sv }));
    EXPECT_TOKEN(lexer, (token{ token_t::Dot, "."sv }));
    EXPECT_TOKEN(lexer, (token{ token_t::Identifier, "main"sv }));
    EXPECT_TOKEN(lexer, (token{ keyword_token::Return }));
    EXPECT_TOKEN(lexer, (token{ keyword_token::Func }));
}

} // namespace