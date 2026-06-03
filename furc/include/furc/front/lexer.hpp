#ifndef FURC_FRONT_LEXER_HPP
#define FURC_FRONT_LEXER_HPP

#include "furc/front/token.hpp"

namespace furc {
namespace front {

/**
 * @brief Lexer.
 *
 * Furlang's lazy tokenizer.
 */
class lexer {
public:
    lexer() = default;

    /**
     * @brief Construct a new lexer.
     *
     * @param filename Filename for debugging.
     * @param content Content.
     */
    lexer(std::string_view filename, std::string_view content);
    ~lexer() = default;

    /**
     * @brief Move constructor.
     */
    lexer(lexer&&) = default;

    lexer(const lexer&) = delete;

    /**
     * @brief Move constructor.
     */
    lexer& operator=(lexer&&) = default;

    lexer& operator=(const lexer&) = delete;
public:
    /**
     * @brief Returns a handle to next token.
     *
     * @return The token handle.
     */
    token_r next_token();

    /**
     * @brief Checks whether the cursor is at the EOF.
     *
     * @return true if the cursor is at the EOF.
     */
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