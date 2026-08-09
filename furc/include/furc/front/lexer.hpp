#ifndef FURC_FRONT_LEXER_HPP
#define FURC_FRONT_LEXER_HPP

#include "furc/front/token.hpp"

#include <cstddef>
#include <deque>
#include <string_view>

namespace furc {

class lexer {
public:
    lexer(std::string_view filepath, std::string_view content)
      : m_filepath(filepath), m_content(content) {}

    ~lexer() = default;

    lexer(lexer&&) noexcept            = default;
    lexer& operator=(lexer&&) noexcept = default;

    lexer(const lexer&)            = delete;
    lexer& operator=(const lexer&) = delete;
public:
    token next_token();
    token peek_token(std::size_t offset = 0);
private:
    token get_token();

    void           next();
    constexpr char get(std::size_t offset = 0) const;
    void           skip_spaces();

    constexpr token::location location() const;
private:
    std::string_view m_filepath;
    std::string_view m_content;
    std::size_t      m_cursor    = 0;
    std::size_t      m_row       = 0;
    std::size_t      m_lineStart = 0;

    std::deque<token> m_peekToken;
};

} // namespace furc

#endif // FURC_FRONT_LEXER_HPP
