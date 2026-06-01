// NOLINTBEGIN(readability-identifier-naming)

#include "furc/front/lexer.hpp"

#include "furlang/result.hpp"

#include "gtest/gtest.h"
#include <string>

namespace {

using namespace furc::front;
using namespace std::string_view_literals;
using namespace std::string_literals;

using token_r    = furlang::result<token, std::string>;
using lexer_case = std::pair<std::string, std::vector<token_r>>;

class LexerTestingFixture : public testing::TestWithParam<lexer_case> {};

TEST_P(LexerTestingFixture, LexerTest) {
    auto [code, expected] = GetParam();

    lexer lexer("<TEMP>", code);
    auto  it = expected.begin();
    while (it != expected.end()) {
        const auto& expected = *it++;

        auto token = std::move(lexer.next_token());
        if (expected.has_error()) {
            EXPECT_TRUE(token.has_error());
            EXPECT_EQ(token.error(), expected.error());
        } else {
            EXPECT_EQ(token, expected.value());
        }
    }
    // EOF
    EXPECT_EQ(lexer.next_token(), token{});
}

INSTANTIATE_TEST_SUITE_P(EmptyTests,
    LexerTestingFixture,
    testing::Values(lexer_case("", {}), lexer_case(" ", {}), lexer_case("\t", {}), lexer_case("\n", {})));

INSTANTIATE_TEST_SUITE_P(Comments,
    LexerTestingFixture,
    testing::Values(lexer_case("(/** skibidi **/func{//)\n}",
        { { token_t::LParen }, { keyword_token::Func }, { token_t::LBrace }, { token_t::RBrace } })));

INSTANTIATE_TEST_SUITE_P(Integers,
    LexerTestingFixture,
    testing::Values(lexer_case("67 6\n7", { { 67 }, { 6 }, { 7 } }),
        lexer_case("18446744073709551615 18446744073709551616",
            { { 18446744073709551615ULL }, token_r(std::string("integer 18446744073709551616 is too big")) })));

INSTANTIATE_TEST_SUITE_P(Tokens,
    LexerTestingFixture,
    testing::Values(lexer_case("()\n\t\t{\n}[\"shto-to\"];    :,.main return func",
        { { token_t::LParen },
            { token_t::RParen },
            { token_t::LBrace },
            { token_t::RBrace },
            { token_t::LBracket },
            { token_t::String, "shto-to"sv },
            { token_t::RBracket },
            { token_t::Semicolon },
            { token_t::Colon },
            { token_t::Comma },
            { token_t::Dot },
            { token_t::Identifier, "main"sv },
            { keyword_token::Return },
            { keyword_token::Func } })));

} // namespace

// NOLINTEND(readability-identifier-naming)