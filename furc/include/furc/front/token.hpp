#ifndef FURC_FRONT_TOKEN_HPP
#define FURC_FRONT_TOKEN_HPP

#include "furc/handle.hpp"

#include <cassert>
#include <ostream>
#include <string_view>

namespace furc {
namespace front {

enum class token_t {
    None,
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
    Comma,
    Dot,
};

static inline std::ostream& operator<<(std::ostream& os, token_t type) {
    switch (type) {
    case token_t::None: return os << "none";
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
    case token_t::Comma: return os << "','";
    case token_t::Dot: return os << "'.'";
    }
}

static inline std::string operator+(const std::string& str, token_t type) {
    switch (type) {
    case token_t::None: return str + "none";
    case token_t::Identifier: return str + "identifier";
    case token_t::Keyword: return str + "keyword";
    case token_t::Integer: return str + "integer";
    case token_t::Lparen: return str + "'('";
    case token_t::Rparen: return str + "')'";
    case token_t::Lbrace: return str + "'{'";
    case token_t::Rbrace: return str + "'}'";
    case token_t::Lbracket: return str + "'['";
    case token_t::Rbracket: return str + "']'";
    case token_t::Semicolon: return str + "';'";
    case token_t::Colon: return str + "':'";
    case token_t::Comma: return str + "','";
    case token_t::Dot: return str + "'.'";
    }
}

enum class keyword_token {
    None,
    Func,
    Return,
};

static inline std::ostream& operator<<(std::ostream& os, keyword_token keyword) {
    switch (keyword) {
    case keyword_token::None: return os << "none";
    case keyword_token::Func: return os << "func";
    case keyword_token::Return: return os << "return";
    }
}

static inline std::string operator+(const std::string& str, keyword_token keyword) {
    switch (keyword) {
    case keyword_token::None: return str + "none";
    case keyword_token::Func: return str + "func";
    case keyword_token::Return: return str + "return";
    }
}

struct token {
    token_t          type    = token_t::None;
    keyword_token    keyword = keyword_token::None;
    std::string_view value;

    token() = default;

    token(token_t type, std::string_view value = {})
      : type(type), value(value) {}

    token(keyword_token keyword, std::string_view value = {})
      : type(token_t::Keyword), keyword(keyword), value(value) {}

    bool operator==(const token& rhs) const {
        if (type != rhs.type) return false;
        switch (type) {
        case token_t::None:
        case token_t::Identifier: return value == rhs.value;
        case token_t::Keyword: return keyword == rhs.keyword;
        case token_t::Integer: return value == rhs.value;
        case token_t::Lparen:
        case token_t::Rparen:
        case token_t::Lbrace:
        case token_t::Rbrace:
        case token_t::Lbracket:
        case token_t::Rbracket:
        case token_t::Semicolon:
        case token_t::Colon:
        case token_t::Comma:
        case token_t::Dot: return true;
        }
    }
};

static inline std::ostream& operator<<(std::ostream& os, const token& token) {
    switch (token.type) {
    case token_t::Identifier:
    case token_t::Integer: return os << token.value;
    case token_t::Keyword: return os << token.keyword;
    default: return os << token.type;
    }
}

template <typename Error = std::string>
using token_handle = handle<token, Error>;

} // namespace front
} // namespace furc

#endif // FURC_FRONT_TOKEN_HPP