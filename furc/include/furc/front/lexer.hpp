#ifndef FURC_FRONT_LEXER_HPP
#define FURC_FRONT_LEXER_HPP

#include "furc/front/token.hpp"

namespace furc {
namespace front {

class lexer {
public:
    lexer() = default;
    lexer(std::string_view filename, std::string_view content);
    ~lexer() = default;

    lexer(lexer&&)                 = default;
    lexer(const lexer&)            = delete;
    lexer& operator=(lexer&&)      = default;
    lexer& operator=(const lexer&) = delete;
public:
    token_handle<> next_token();

    bool empty() const { return m_cursor >= m_content.size(); }
private:
    void     next();
    char     get(std::size_t offset = 0) const;
    void     skip_spaces();
    location current_location();
private:
    std::string_view m_filename;
    std::string_view m_content;
    std::size_t      m_cursor    = 0;
    std::size_t      m_row       = 0;
    std::size_t      m_lineStart = 0;
};

} // namespace front
} // namespace furc

#endif // FURC_FRONT_LEXER_HPP