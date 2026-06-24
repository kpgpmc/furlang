#include "furc/front/lexer.hpp"

#include "furc/front/token.hpp"

#include <cctype>
#include <limits>
#include <map>
#include <string>
#include <unordered_map>

namespace furc::front {

using namespace std::string_literals;

lexer::lexer(std::string_view filename, std::string_view content)
  : m_filename(filename), m_content(content) {}

token_r lexer::next_token() {
    skip_spaces();
    while (m_cursor + 2 <= m_content.size() && m_content[m_cursor] == '/') {
        if (m_content[m_cursor + 1] == '/') {
            m_cursor += 2;
            while (m_content[m_cursor] != '\n') {
                next();
            }
        } else if (m_content[m_cursor + 1] == '*') {
            m_cursor += 2;
            while (m_cursor + 2 < m_content.size()) {
                if (m_content[m_cursor + 1] == '*') {
                    next();
                } else if (m_content[m_cursor + 0] != '*' || m_content[m_cursor + 1] != '/') {
                    next();
                    next();
                } else {
                    break;
                }
            }
            if (m_cursor + 2 >= m_content.size()) {
                next();
                return token_r(
                    token_error{ current_location(), token_error_t::UnexpectedEof, "before enclosing `*/`" });
            }
            m_cursor += 2;
        } else {
            break;
        }
        skip_spaces();
    }

    location location = current_location();

    switch (get()) {
    case '"': {
        std::size_t begin = ++m_cursor;
        while (m_cursor < m_content.size() && m_content[m_cursor] != '"')
            ++m_cursor;
        if (m_cursor >= m_content.size()) {
            return token_r(token_error{ current_location(), token_error_t::UnexpectedEof, "before enclosing '\"'" });
        }
        ++m_cursor;

        return { location, token_t::String, m_content.substr(begin, m_cursor - begin - 1) };
    }
    case std::char_traits<char>::eof(): return token_r(token_error{ current_location(), token_error_t::EndOfFile });
    default: {
        if (std::isdigit(get()) != 0) {
            integer_token integer    = 0;
            integer_token max        = std::numeric_limits<integer_token>::max();
            integer_token upperBound = max / 10;

            std::size_t start = m_cursor;
            while (std::isdigit(get()) != 0) {
                integer_token digit = get() - '0';

                if (integer > upperBound || integer == upperBound && (integer - upperBound + digit) > (max % 10)) {
                    while (std::isdigit(get()) != 0)
                        ++m_cursor;
                    return token_r(token_error{ location,
                        token_error_t::IntegerOverflow,
                        std::string(m_content.substr(start, m_cursor - start)) });
                }
                integer *= 10;
                integer += digit;
                ++m_cursor;
            }

            return { location, integer };
        }

        if (std::isalnum(get()) != 0 || get() == '_') {
            std::size_t start = m_cursor++;
            while (std::isalnum(get()) != 0 || get() == '_')
                next();

            std::string_view value = m_content.substr(start, m_cursor - start);

            static std::unordered_map<std::string_view, keyword_token> s_keywords = {
                { "func", keyword_token::Func },
                { "return", keyword_token::Return },
                { "if", keyword_token::If },
                { "else", keyword_token::Else },
                { "while", keyword_token::While },
                { "int32", keyword_token::Int32 },
            };

            if (auto it = s_keywords.find(value); it != s_keywords.end()) return { location, it->second };
            return { location, token_t::Identifier, value };
        }

        struct compare {
            bool operator()(const std::string_view& lhs, const std::string_view& rhs) const {
                if (lhs.size() != rhs.size()) return lhs.size() > rhs.size();
                return lhs < rhs;
            }
        };

        static std::map<std::string_view, token_t, compare> s_tokens = {
            { "(", token_t::LParen },
            { ")", token_t::RParen },
            { "{", token_t::LBrace },
            { "}", token_t::RBrace },
            { "[", token_t::LBracket },
            { "]", token_t::RBracket },
            { ";", token_t::Semicolon },
            { ":", token_t::Colon },
            { ",", token_t::Comma },
            { ".", token_t::Dot },
            { "+", token_t::Plus },
            { "-", token_t::Minus },
            { "*", token_t::Star },
            { "/", token_t::Slash },
            { "%", token_t::Percent },
            { "++", token_t::DPlus },
            { "--", token_t::DMinus },
            { "=", token_t::Eq },
            { "+=", token_t::PlusEq },
            { "-=", token_t::MinusEq },
            { "*=", token_t::StarEq },
            { "/=", token_t::SlashEq },
            { "%=", token_t::PercentEq },
            { "==", token_t::DEq },
            { "!=", token_t::NotEq },
            { "<", token_t::LessThan },
            { ">", token_t::GreaterThan },
            { "<=", token_t::LessEq },
            { ">=", token_t::GreaterEq },
        };

        token_t     type   = token_t::None;
        std::size_t length = 1;
        while (m_cursor + length <= m_content.size()) {
            auto it = s_tokens.find(m_content.substr(m_cursor, length));
            if (it == s_tokens.end()) break;
            type = it->second;
            ++length;
        }
        if (type != token_t::None) {
            m_cursor += length - 1;
            return { location, type };
        }

        return token_r(
            token_error{ location, token_error_t::UnexpectedCharacter, std::string(m_content.substr(m_cursor, 1)) });
    }
    }
}

void lexer::next() {
    if (m_cursor >= m_content.size()) return;
    char ch = get();
    ++m_cursor;
    if (ch == '\n') {
        ++m_row;
        m_lineStart = m_cursor;
    }
}

char lexer::get(std::size_t offset) const {
    if (m_cursor + offset < m_content.size()) return m_content[m_cursor + offset];
    return std::char_traits<char>::eof();
}

void lexer::skip_spaces() {
    while (std::isspace(get()) != 0)
        next();
}

location lexer::current_location() {
    return { m_filename, m_row, m_cursor - m_lineStart };
}

} // namespace furc::front
