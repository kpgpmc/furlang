#include "furc/front/lexer.hpp"

#include "gtest/gtest.h"

namespace {

using namespace furc::front;
using namespace std::string_view_literals;

#define EXPECT_TOKEN(lexer, t) EXPECT_EQ((lexer).next_token(), (t));
#define EXPECT_EOF(lexer) EXPECT_EQ((lexer).next_token(), (token{}));
#define EXPECT_ERROR(lexer, err)                                                                                       \
    do {                                                                                                               \
        auto handle = (lexer).next_token();                                                                            \
        EXPECT_TRUE(handle.has_error());                                                                               \
        EXPECT_EQ(handle.error(), (err));                                                                              \
    } while (0)

TEST(Lexer, Tokens) {
    lexer lexer("<TEMP>", "()\n\t\t{\n}[\"shto-to\"];    :,.main return func");
    EXPECT_TOKEN(lexer, (token{ token_t::Lparen, "("sv }));
    EXPECT_TOKEN(lexer, (token{ token_t::Rparen, ")"sv }));
    EXPECT_TOKEN(lexer, (token{ token_t::Lbrace, "{"sv }));
    EXPECT_TOKEN(lexer, (token{ token_t::Rbrace, "}"sv }));
    EXPECT_TOKEN(lexer, (token{ token_t::Lbracket, "["sv }));
    EXPECT_TOKEN(lexer, (token{ token_t::String, "shto-to"sv }));
    EXPECT_TOKEN(lexer, (token{ token_t::Rbracket, "]"sv }));
    EXPECT_TOKEN(lexer, (token{ token_t::Semicolon, ";"sv }));
    EXPECT_TOKEN(lexer, (token{ token_t::Colon, ":"sv }));
    EXPECT_TOKEN(lexer, (token{ token_t::Comma, ","sv }));
    EXPECT_TOKEN(lexer, (token{ token_t::Dot, "."sv }));
    EXPECT_TOKEN(lexer, (token{ token_t::Identifier, "main"sv }));
    EXPECT_TOKEN(lexer, (token{ keyword_token::Return }));
    EXPECT_TOKEN(lexer, (token{ keyword_token::Func }));
    EXPECT_EOF(lexer);
}

TEST(Lexer, Comments) {
    lexer lexer("<TEMP>", "(/** skibidi **/func{//)\n}");
    EXPECT_TOKEN(lexer, (token{ token_t::Lparen, "("sv }));
    EXPECT_TOKEN(lexer, (token{ keyword_token::Func }));
    EXPECT_TOKEN(lexer, (token{ token_t::Lbrace, "{"sv }));
    EXPECT_TOKEN(lexer, (token{ token_t::Rbrace, "}"sv }));
    EXPECT_EOF(lexer);
}

TEST(Lexer, Integers) {
    lexer lexer("<TEMP>", "67 18446744073709551615 18446744073709551616");
    EXPECT_TOKEN(lexer, (token{ 67ULL }));
    EXPECT_TOKEN(lexer, (token{ 18446744073709551615ULL }));
    EXPECT_ERROR(lexer, "integer 18446744073709551616 is too big");
    EXPECT_EOF(lexer);
}

} // namespace