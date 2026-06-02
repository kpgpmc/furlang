// NOLINTBEGIN(readability-identifier-naming)

#include "furc/front/lexer.hpp"

#include "gtest/gtest.h"
#include <string>

namespace {

using namespace furc::front;
using namespace std::string_view_literals;
using namespace std::string_literals;

using lexer_case = std::pair<std::string, std::vector<token_r>>;

class LexerTestingFixture : public testing::TestWithParam<lexer_case> {};

TEST_P(LexerTestingFixture, LexerTest) {
    auto [code, expected] = GetParam();

    lexer lexer("<TEMP>", code);
    auto  it = expected.begin();
    while (it != expected.end()) {
        const auto& expected = *it++;

        EXPECT_EQ(lexer.next_token(), expected);
    }

    auto eof = std::move(lexer.next_token());
    ASSERT_TRUE(eof.has_error());
    ASSERT_EQ(eof.error().type, token_error_t::EndOfFile);
}

furc::location loc(size_t col, size_t line) {
    return furc::location{ "<TEMP>", line, col };
}

INSTANTIATE_TEST_SUITE_P(EmptyTests,
    LexerTestingFixture,
    testing::Values(lexer_case("", {}), lexer_case(" ", {}), lexer_case("\t", {}), lexer_case("\n", {})));

INSTANTIATE_TEST_SUITE_P(Comments,
    LexerTestingFixture,
    testing::Values(lexer_case("(/** skibidi **/func{//)\n}",
        { { loc(0, 0), token_t::LParen },
            { loc(16, 0), keyword_token::Func },
            { loc(20, 0), token_t::LBrace },
            { loc(0, 1), token_t::RBrace } })));

INSTANTIATE_TEST_SUITE_P(Integers,
    LexerTestingFixture,
    testing::Values(lexer_case("67 6\n7", { { loc(0, 0), 67 }, { loc(3, 0), 6 }, { loc(0, 1), 7 } }),
        lexer_case("18446744073709551615\n18446744073709551616",
            { { loc(0, 0), 18446744073709551615ULL },
                token_r(
                    token_error{ loc(0, 1), token_error_t::IntegerOverflow, std::string("18446744073709551616") }) })));

INSTANTIATE_TEST_SUITE_P(Tokens,
    LexerTestingFixture,
    testing::Values(lexer_case("()\n\t\t{\n}[\"shto-to\"];    :,.main return func",
        { { loc(0, 0), token_t::LParen },
            { loc(1, 0), token_t::RParen },
            { loc(2, 1), token_t::LBrace },
            { loc(0, 2), token_t::RBrace },
            { loc(1, 2), token_t::LBracket },
            { loc(2, 2), token_t::String, "shto-to"sv },
            { loc(10, 2), token_t::RBracket },
            { loc(11, 2), token_t::Semicolon },
            { loc(15, 2), token_t::Colon },
            { loc(16, 2), token_t::Comma },
            { loc(17, 2), token_t::Dot },
            { loc(18, 2), token_t::Identifier, "main"sv },
            { loc(23, 2), keyword_token::Return },
            { loc(30, 2), keyword_token::Func } })));

} // namespace

// NOLINTEND(readability-identifier-naming)