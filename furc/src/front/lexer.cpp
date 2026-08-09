#include "furc/front/lexer.hpp"

#include <cctype>
#include <limits>
#include <string_view>
#include <unordered_map>

namespace furc {

token lexer::next_token() {
    if (m_peekToken.has_value()) {
        auto tok    = m_peekToken.value();
        m_peekToken = {};
        return tok;
    }

    skip_spaces();

    if (m_cursor >= m_content.size()) return { location(), token::EndOfFile };

    auto loc = location();

    if (std::isdigit(get()) != 0) {
        std::uint64_t value = get() - '0';
        next();
        while (m_cursor < m_content.size() && std::isdigit(get()) != 0) {
            static constexpr std::uint64_t MAX        = std::numeric_limits<std::uint64_t>::max();
            static constexpr std::uint64_t MAX_MULTS  = MAX / 10;
            static constexpr std::uint64_t LAST_DIGIT = MAX % 10;

            if (value > MAX_MULTS) {
                return { location(), token::InvalidInteger };
            }
            std::uint64_t digit = get() - '0';
            if (value == MAX_MULTS && digit > LAST_DIGIT) {
                return { location(), token::InvalidInteger };
            }

            value *= 10;
            value += digit;
            next();
        }
        return { loc, value };
    }

    if (std::isalnum(get()) != 0 || get() == '_') {
        static std::unordered_map<std::string_view, token_t> s_keywords = {
            { "func", token::Func },
            { "return", token::Return },
            { "if", token::If },
            { "else", token::Else },
            { "while", token::While },
            { "public", token::Public },
            { "private", token::Private },
            { "pre", token::Pre },
            { "post", token::Post },
            { "pointerof", token::Pointerof },
            { "sizeof", token::Sizeof },
            { "lengthof", token::Lengthof },
            { "s8", token::S8 },
            { "u8", token::U8 },
            { "s16", token::S16 },
            { "u16", token::U16 },
            { "s32", token::S32 },
            { "u32", token::U32 },
            { "s64", token::S64 },
            { "u64", token::U64 },
        };

        std::size_t begin = m_cursor;
        next();
        while (m_cursor < m_content.size() && (std::isalnum(get()) != 0 || get() == '_'))
            next();
        std::string_view name = m_content.substr(begin, m_cursor - begin);
        if (auto it = s_keywords.find(name); it != s_keywords.end()) {
            return { loc, it->second };
        }
        return { loc, token::Identifier, name };
    }

    if (get() == '"') {
        next();

        std::size_t begin = m_cursor;
        while (m_cursor < m_content.size() && get() != '"')
            next();
        if (m_cursor >= m_content.size()) return { location(), token::UnexpectedEOF };

        next();
        return { loc, token::String, m_content.substr(begin, m_cursor - begin - 1) };
    }

    if (get() == '\'') {
        next();
        bool slash = get() == '\\';
        if (slash) next();
        auto loc2      = location();
        char character = get();
        next();
        if (get() != '\'') return { location(), token::UnexpectedCharacter, get() };
        next();
        if (slash) {
            switch (character) {
            case '\\': character = '\\'; break;
            case 'n': character = '\n'; break;
            case 'r': character = '\r'; break;
            case 't': character = '\t'; break;
            default: return { loc2, token::UnexpectedCharacter, character };
            }
        }
        return { loc, token::Char, character };
    }

    static std::unordered_map<std::string_view, token_t> s_tokens = {
        { "(", token::LParen },
        { ")", token::RParen },
        { "{", token::LBrace },
        { "}", token::RBrace },
        { "[", token::LBracket },
        { "]", token::RBracket },
        { ";", token::Semicolon },
        { ":", token::Colon },
        { ",", token::Comma },
        { ".", token::Dot },
        { "+", token::Plus },
        { "-", token::Minus },
        { "*", token::Star },
        { "/", token::Slash },
        { "%", token::Percent },
        { "&", token::Ampersand },
        { "|", token::Pipe },
        { "^", token::Hat },
        { "&&", token::DblAmpersand },
        { "||", token::DblPipe },
        { "++", token::DblPlus },
        { "--", token::DblMinus },
        { "!", token::ExMark },
        { "^^", token::CatEars },
        { "=", token::Equals },
        { "+=", token::PlusEquals },
        { "-=", token::MinusEquals },
        { "*=", token::StarEquals },
        { "/=", token::SlashEquals },
        { "%=", token::PercentEquals },
        { "&=", token::AmpersandEquals },
        { "|=", token::PipeEquals },
        { "^=", token::HatEquals },
        { "==", token::DblEquals },
        { "!=", token::ExEquals },
        { "<", token::LessThan },
        { "<=", token::LessEquals },
        { ">", token::GreaterThan },
        { ">=", token::GreaterEquals },
        { "->", token::SlimArrow },
        { "=>", token::FatArrow },
        { "@", token::Monkey },
        { "#", token::Sha256 },
    };

    std::size_t begin = m_cursor;
    std::size_t len   = 1;
    while (begin + len - 1 < m_content.size() && s_tokens.find(m_content.substr(begin, len)) != s_tokens.end())
        ++len;

    if (len > 1) {
        auto type = s_tokens[m_content.substr(begin, len - 1)];
        m_cursor += len - 1;
        return { loc, type };
    }

    return { loc, token::UnexpectedCharacter, get() };
}

token lexer::peek_token() {
    if (m_peekToken.has_value()) return m_peekToken.value();
    auto tok    = next_token();
    m_peekToken = tok;
    return tok;
}

token lexer::skip_token() {
    m_peekToken = {};
    return next_token();
}

void lexer::next() {
    if (m_cursor < m_content.size()) ++m_cursor;
}

constexpr char lexer::get(std::size_t offset) const {
    return m_content[m_cursor + offset];
}

void lexer::skip_spaces() {
    while (m_cursor < m_content.size() && std::isspace(get()) != 0)
        ++m_cursor;
}

constexpr token::location lexer::location() const {
    return { m_filepath, m_row, m_cursor - m_lineStart };
}

} // namespace furc
