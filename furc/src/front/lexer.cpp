#include "furc/front/lexer.hpp"

#include <cctype>
#include <limits>
#include <string>
#include <unordered_map>

namespace furc::front {

using namespace std::string_literals;

lexer::lexer(std::string_view filename, std::string_view content)
  : m_filename(filename), m_content(content) {}

token_handle<> lexer::next_token() {
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
                return { current_location(), "unexpected end of file before enclosing `*/`" };
            }
            m_cursor += 2;
        } else {
            break;
        }
        skip_spaces();
    }

    location location = current_location();

    char ch = get();
    switch (ch) {
    case '(': ++m_cursor; return { location, token_t::Lparen };
    case ')': ++m_cursor; return { location, token_t::Rparen };
    case '{': ++m_cursor; return { location, token_t::Lbrace };
    case '}': ++m_cursor; return { location, token_t::Rbrace };
    case '[': ++m_cursor; return { location, token_t::Lbracket };
    case ']': ++m_cursor; return { location, token_t::Rbracket };
    case ';': ++m_cursor; return { location, token_t::Semicolon };
    case ':': ++m_cursor; return { location, token_t::Colon };
    case ',': ++m_cursor; return { location, token_t::Comma };
    case '.': ++m_cursor; return { location, token_t::Dot };
    case '+': ++m_cursor; return { location, token_t::Plus };
    case '-': ++m_cursor; return { location, token_t::Minus };
    case '*': ++m_cursor; return { location, token_t::Star };
    case '/': ++m_cursor; return { location, token_t::Slash };
    case '%': ++m_cursor; return { location, token_t::Percent };
    case '"': {
        std::size_t begin = ++m_cursor;
        while (m_cursor < m_content.size() && m_content[m_cursor] != '"')
            ++m_cursor;
        if (m_cursor >= m_content.size()) return { current_location(), "unexpected end of file before enclosing '\"'" };
        ++m_cursor;

        return { location, token_t::String, m_content.substr(begin, m_cursor - begin - 1) };
    }
    case std::char_traits<char>::eof(): return { location, token_t::None };
    default: {
        if (std::isdigit(ch) != 0) {
            integer_token integer    = 0;
            integer_token max        = std::numeric_limits<integer_token>::max();
            integer_token upperBound = max / 10;

            std::size_t start = m_cursor;
            while (std::isdigit(get()) != 0) {
                integer_token digit = get() - '0';

                if (integer > upperBound || integer == upperBound && (integer - upperBound + digit) > (max % 10)) {
                    while (std::isdigit(get()) != 0)
                        ++m_cursor;
                    return { location, "integer "s.append(m_content.substr(start, m_cursor - start)) + " is too big" };
                }
                integer *= 10;
                integer += digit;
                ++m_cursor;
            }

            return { location, integer };
        }

        if (std::isalnum(ch) != 0 || ch == '_') {
            std::size_t start = m_cursor++;
            while (std::isalnum(get()) != 0 || get() == '_')
                next();

            std::string_view value = m_content.substr(start, m_cursor - start);

            static std::unordered_map<std::string_view, keyword_token> s_keywords = {
                { "func", keyword_token::Func },
                { "return", keyword_token::Return },
            };

            if (auto it = s_keywords.find(value); it != s_keywords.end()) return { location, it->second, value };
            return { location, token_t::Identifier, value };
        }

        return { location, "unexpected character '"s.append(m_content.substr(m_cursor, 1)) + "'" };
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