#include "furas/lexer.hpp"

#include <cctype>
#include <unordered_map>

namespace furas {

using namespace std::string_literals;

token_r lexer::next_token() {
    while (m_cursor < m_content.size() && std::isspace(m_content[m_cursor]) != 0) {
        if (m_content[m_cursor] == '\n') {
            m_lineStart = m_cursor + 1;
            ++m_column;
        }
        ++m_cursor;
    }

    // TODO: Add support for single-line comments
    // TODO: Add support for multi-line comments

    if (m_cursor >= m_content.size()) return token_r{ lexer_error{ lexer_error::EndOfFile, location() } };

    // TODO: Add support for negative integers (I am positive thanks to stasiu :v:)
    // TODO: Add support for hexadecimal and binary numeric literals
    if (std::isdigit(m_content[m_cursor]) != 0) {
        std::uint64_t num = 0;
        while (m_cursor < m_content.size() && std::isdigit(m_content[m_cursor]) != 0) {
            num *= 10;
            num += m_content[m_cursor++] - '0';
        }
        return { num };
    }

    static std::unordered_map<std::string_view, enum token::type> s_tokens = {
        { "import", token::Import },
        { "public", token::Public },
        { "private", token::Private },

        { "push", token::Push },
        { "array", token::Array },
        { "get", token::Get },
        { "drop", token::Drop },
        { "dup", token::Dup },
        { "clone", token::Clone },
        { "ref", token::Ref },
        { "add", token::Add },
        { "sub", token::Sub },
        { "mul", token::Mul },
        { "div", token::Div },
        { "mod", token::Mod },
        { "eq", token::Eq },
        { "neq", token::Neq },
        { "lt", token::Lt },
        { "gt", token::Gt },
        { "le", token::Le },
        { "ge", token::Ge },
        { "ptrof", token::Ptrof },
        { "sizeof", token::Sizeof },
        { "lenof", token::Lenof },
        { "load", token::Load },
        { "store", token::Store },
        { "call", token::Call },
        { "jump", token::Jump },
        { "jnz", token::Jnz },
        { "ret", token::Ret },
    };

    if (std::isalnum(m_content[m_cursor]) != 0) {
        std::size_t begin = m_cursor++;
        while (m_cursor < m_content.size() && (std::isalnum(m_content[m_cursor]) != 0 || m_content[m_cursor] == '_'))
            ++m_cursor;
        std::string_view str = m_content.substr(begin, m_cursor - begin);
        if (auto it = s_tokens.find(str); it != s_tokens.end()) return { it->second };
        return { token::Identifier, str };
    }

    switch (m_content[m_cursor]) {
    case '@': ++m_cursor; return { token::Monkey };
    case '$': ++m_cursor; return { token::Dolar };
    case '#': ++m_cursor; return { token::Sha256 };
    case '%': ++m_cursor; return { token::Percent };
    case '.': ++m_cursor; return { token::Dot };
    default:
        return token_r{
            lexer_error{ lexer_error::UnknownCharacter, location(), "Unknown character '"s + m_content[m_cursor] + "'" }
        };
    }
}

} // namespace furas
