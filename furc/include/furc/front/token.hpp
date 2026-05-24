#ifndef FURC_FRONT_TOKEN_HPP
#define FURC_FRONT_TOKEN_HPP

#include "furc/diag.hpp"

#include <cassert>
#include <ostream>
#include <string_view>

namespace furc {
namespace front {

enum class token_t {
    None,
    Error,
    Identifier,
    Keyword,
    Integer,
    Lparen,
    Rparen,
    Lbrace,
    Rbrace,
    Lbracket,
    Rbracket,
    Semicolon,
    Colon,
};

static inline bool is_token_type_empty(token_t type) {
    return type == token_t::None || type == token_t::Error;
}

static inline std::ostream& operator<<(std::ostream& os, token_t type) {
    switch (type) {
    case token_t::None: return os << "none";
    case token_t::Error: return os << "error";
    case token_t::Identifier: return os << "identifier";
    case token_t::Keyword: return os << "keyword";
    case token_t::Integer: return os << "integer";
    case token_t::Lparen: return os << "'('";
    case token_t::Rparen: return os << "')'";
    case token_t::Lbrace: return os << "'{'";
    case token_t::Rbrace: return os << "'}'";
    case token_t::Lbracket: return os << "'['";
    case token_t::Rbracket: return os << "']'";
    case token_t::Semicolon: return os << "';'";
    case token_t::Colon: return os << "':'";
    }
}

enum class error_token {
    None,
    UnexpectedCharacter,
};

static inline std::ostream& operator<<(std::ostream& os, error_token error) {
    switch (error) {
    case error_token::None: return os << "none";
    case error_token::UnexpectedCharacter: return os << "unexpected character";
    }
}

enum class keyword_token {
    None,
    Func,
    Return,
};

struct token {
    token_t          type    = token_t::None;
    error_token      error   = error_token::None;
    keyword_token    keyword = keyword_token::None;
    std::string_view value;
    location         location;

    token() = default;

    token(token_t type, std::string_view value = {})
      : type(type), value(value) {}

    token(error_token error, std::string_view value = {})
      : type(token_t::Error), error(error), value(value) {}

    token(keyword_token keyword, std::string_view value = {})
      : type(token_t::Keyword), keyword(keyword), value(value) {}
};

static inline std::ostream& operator<<(std::ostream& os, const token& token) {
    os << token.location << ": " << token.type;
    switch (token.type) {
    case token_t::Error: {
        os << ": " << token.error;
        if (!token.value.empty()) os << " \"" << token.value << '"';
        return os;
    }
    case token_t::Identifier:
    case token_t::Keyword:
    case token_t::Integer: return os << ": " << token.value;
    default: return os;
    }
}

} // namespace front
} // namespace furc

#endif // FURC_FRONT_TOKEN_HPP