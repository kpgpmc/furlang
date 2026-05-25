#include "furc/front/lexer.hpp"

#include <cctype>
#include <string>
#include <unordered_map>

namespace furc::front {

using namespace std::string_literals;

lexer::lexer(std::string_view filename, std::string_view content)
  : m_filename(filename), m_content(content) {}

token_handle<> lexer::next_token() {
    skip_spaces();

    location location = current_location();

    char ch = get();
    switch (ch) {
    case '(': return { location, token_t::Lparen, m_content.substr(m_cursor++, 1) };
    case ')': return { location, token_t::Rparen, m_content.substr(m_cursor++, 1) };
    case '{': return { location, token_t::Lbrace, m_content.substr(m_cursor++, 1) };
    case '}': return { location, token_t::Rbrace, m_content.substr(m_cursor++, 1) };
    case '[': return { location, token_t::Lbracket, m_content.substr(m_cursor++, 1) };
    case ']': return { location, token_t::Rbracket, m_content.substr(m_cursor++, 1) };
    case ';': return { location, token_t::Semicolon, m_content.substr(m_cursor++, 1) };
    case ':': return { location, token_t::Colon, m_content.substr(m_cursor++, 1) };
    case ',': return { location, token_t::Comma, m_content.substr(m_cursor++, 1) };
    case '.': return { location, token_t::Dot, m_content.substr(m_cursor++, 1) };
    case std::char_traits<char>::eof(): return { location, token_t::None };
    default: {
        if (std::isdigit(ch) != 0) {}

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