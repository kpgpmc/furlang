#ifndef FURAS_LEXER_HPP
#define FURAS_LEXER_HPP

#include "furas/token.hpp"
#include "furlang/result.hpp"

#include <cstddef>
#include <string_view>

namespace furas {

struct lexer_location {
    std::string_view filename;
    std::size_t      row, col;
};

struct lexer_error {
    enum type {
        EndOfFile = 0,
        UnknownCharacter,
    } type;

    lexer_location location;
    std::string    message;
};

using token_r = furlang::result<token, lexer_error>;

class lexer {
public:
    lexer(std::string_view filename, std::string_view content)
      : m_filename(filename), m_content(content) {}

    token_r next_token();
private:
    constexpr lexer_location location() const { return { m_filename, m_cursor - m_lineStart, m_column }; }
private:
    std::string_view m_filename;
    std::string_view m_content;
    std::size_t      m_cursor    = 0;
    std::size_t      m_lineStart = 0;
    std::size_t      m_column    = 0;
};

} // namespace furas

#endif // FURAS_LEXER_HPP
