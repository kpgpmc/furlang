#ifndef FURC_FRONT_TOKEN_HPP
#define FURC_FRONT_TOKEN_HPP

#include "furc/diag.hpp"
#include "furlang/result.hpp"

#include <cassert>
#include <cstdint>
#include <ostream>
#include <string_view>

namespace furc {
namespace front {

/**
 * @brief Token type.
 */
enum class token_t {
    None,       /**< None */
    Identifier, /**< Identifier */
    String,     /**< String */
    Keyword,    /**< Keyword */
    Integer,    /**< Integer */

    LParen,    /**< `(` */
    RParen,    /**< `)` */
    LBrace,    /**< `{` */
    RBrace,    /**< `}` */
    LBracket,  /**< `[` */
    RBracket,  /**< `]` */
    Semicolon, /**< `;` */
    Colon,     /**< `:` */
    Comma,     /**< `,` */
    Dot,       /**< `.` */

    Plus,    /**< `+` */
    Minus,   /**< `-` */
    Star,    /**< `*` */
    Slash,   /**< `/` */
    Percent, /**< `%` */
    DPlus,   /**< `++` */
    DMinus,  /**< `--` */

    Eq,        /**< `=` */
    PlusEq,    /**< `+=` */
    MinusEq,   /**< `-=` */
    StarEq,    /**< `*=` */
    SlashEq,   /**< `/=` */
    PercentEq, /**< `%=` */

    DEq,         /**< `==` */
    NotEq,       /**< `!=` */
    LessThan,    /**< `<` */
    GreaterThan, /**< `>` */
    LessEq,      /**< `<=` */
    GreaterEq,   /**< `>=` */

    SlimArrow, /**< `->` */
    FatArrow,  /**< `=>` */
};

static inline std::ostream& operator<<(std::ostream& os, token_t type) {
    switch (type) {
    case token_t::None: return os << "none";
    case token_t::Identifier: return os << "identifier";
    case token_t::String: return os << "string";
    case token_t::Keyword: return os << "keyword";
    case token_t::Integer: return os << "integer";
    case token_t::LParen: return os << "'('";
    case token_t::RParen: return os << "')'";
    case token_t::LBrace: return os << "'{'";
    case token_t::RBrace: return os << "'}'";
    case token_t::LBracket: return os << "'['";
    case token_t::RBracket: return os << "']'";
    case token_t::Semicolon: return os << "';'";
    case token_t::Colon: return os << "':'";
    case token_t::Comma: return os << "','";
    case token_t::Dot: return os << "'.'";
    case token_t::Plus: return os << "'+'";
    case token_t::Minus: return os << "'-'";
    case token_t::Star: return os << "'*'";
    case token_t::Slash: return os << "'/'";
    case token_t::Percent: return os << "'%'";
    case token_t::DPlus: return os << "++";
    case token_t::DMinus: return os << "--";
    case token_t::Eq: return os << "=";
    case token_t::PlusEq: return os << "+=";
    case token_t::MinusEq: return os << "-=";
    case token_t::StarEq: return os << "*=";
    case token_t::SlashEq: return os << "/=";
    case token_t::PercentEq: return os << "%=";
    case token_t::DEq: return os << "==";
    case token_t::NotEq: return os << "!=";
    case token_t::LessThan: return os << "<";
    case token_t::GreaterThan: return os << ">";
    case token_t::LessEq: return os << "<=";
    case token_t::GreaterEq: return os << ">=";
    case token_t::SlimArrow: return os << "->";
    case token_t::FatArrow: return os << "=>";
    }
    return os;
}

static inline std::string operator+(const std::string& str, token_t type) {
    switch (type) {
    case token_t::None: return str + "none";
    case token_t::Identifier: return str + "identifier";
    case token_t::String: return str + "string";
    case token_t::Keyword: return str + "keyword";
    case token_t::Integer: return str + "integer";
    case token_t::LParen: return str + "'('";
    case token_t::RParen: return str + "')'";
    case token_t::LBrace: return str + "'{'";
    case token_t::RBrace: return str + "'}'";
    case token_t::LBracket: return str + "'['";
    case token_t::RBracket: return str + "']'";
    case token_t::Semicolon: return str + "';'";
    case token_t::Colon: return str + "':'";
    case token_t::Comma: return str + "','";
    case token_t::Dot: return str + "'.'";
    case token_t::Plus: return str + "'+'";
    case token_t::Minus: return str + "'-'";
    case token_t::Star: return str + "'*'";
    case token_t::Slash: return str + "'/'";
    case token_t::Percent: return str + "'%'";
    case token_t::DPlus: return str + "++";
    case token_t::DMinus: return str + "--";
    case token_t::Eq: return str + "=";
    case token_t::PlusEq: return str + "+=";
    case token_t::MinusEq: return str + "-=";
    case token_t::StarEq: return str + "*=";
    case token_t::SlashEq: return str + "/=";
    case token_t::PercentEq: return str + "%=";
    case token_t::DEq: return str + "==";
    case token_t::NotEq: return str + "!=";
    case token_t::LessThan: return str + "<";
    case token_t::GreaterThan: return str + ">";
    case token_t::LessEq: return str + "<=";
    case token_t::GreaterEq: return str + ">=";
    case token_t::SlimArrow: return str + "->";
    case token_t::FatArrow: return str + "=>";
    }
    return str;
}

/**
 * @brief Keyword token.
 */
enum class keyword_token {
    None,   /**< None */
    Func,   /**< `func` */
    Return, /**< `return` */
    If,     /**< `if` */
    Else,   /**< `else` */
    While,  /**< `while` */

    Int32, /**< `int32` */
};

static inline std::ostream& operator<<(std::ostream& os, keyword_token keyword) {
    switch (keyword) {
    case keyword_token::None: return os << "none";
    case keyword_token::Func: return os << "func";
    case keyword_token::Return: return os << "return";
    case keyword_token::If: return os << "if";
    case keyword_token::Else: return os << "else";
    case keyword_token::While: return os << "while";
    case keyword_token::Int32: return os << "int32";
    }
    return os;
}

static inline std::string operator+(const std::string& str, keyword_token keyword) {
    switch (keyword) {
    case keyword_token::None: return str + "none";
    case keyword_token::Func: return str + "func";
    case keyword_token::Return: return str + "return";
    case keyword_token::If: return str + "if";
    case keyword_token::Else: return str + "else";
    case keyword_token::While: return str + "while";
    case keyword_token::Int32: return str + "int32";
    }
    return str;
}

using integer_token = std::uint64_t; /**< Integer token. */

/**
 * @brief Token.
 */
struct token {
    location location;             /**< Token location. */
    token_t  type = token_t::None; /**< Token type. */

    /**
     * @brief Token's value.
     */
    union value {
        /**
         * @brief Null value. For token_t::None, token_t::Plus, token_t::Minus, etc.
         * @see token_t::None
         */
        std::nullptr_t null;

        /**
         * @brief String value. For token_t::Identifier and token_t::String.
         * @see token_t::Identifier
         * @see token_t::String
         */
        std::string_view string;

        /**
         * @brief Keyword value. For token_t::Keyword.
         * @see token_t::Keyword.
         */
        keyword_token keyword;

        /**
         * @brief Integer value. For token_t::Integer.
         * @see token_t::Integer
         */
        integer_token integer;

        /**
         * @brief Construct a new null value.
         *
         * @param value Null value.
         */
        value(std::nullptr_t value = nullptr)
          : null(value) {}

        /**
         * @brief Construct a new string value.
         *
         * @param value The string.
         */
        value(std::string_view value)
          : string(value) {}

        /**
         * @brief Construct a new keyword value.
         *
         * @param value The keyword.
         */
        value(keyword_token value)
          : keyword(value) {}

        /**
         * @brief Construct a new integer value.
         *
         * @param value The integer.
         */
        value(integer_token value)
          : integer(value) {}
    } value; /**< Token value. */

    token() = default;

    /**
     * @brief Construct a new null token.
     *
     * @param location Token's location.
     * @param type Token's type.
     */
    token(struct location location, token_t type)
      : location(location), type(type) {}

    /**
     * @brief Construct a new string token.
     *
     * @param location Token's location.
     * @param type Token's type.
     * @param value String value.
     */
    token(struct location location, token_t type, std::string_view value)
      : location(location), type(type), value(value) {}

    /**
     * @brief Construct a new keyword token.
     *
     * @param location Token's location.
     * @param keyword Keyword value.
     */
    token(struct location location, keyword_token keyword)
      : location(location), type(token_t::Keyword), value(keyword) {}

    /**
     * @brief Construct a new integer token.
     *
     * @param location Token's location.
     * @param integer Integer value.
     */
    token(struct location location, integer_token integer)
      : location(location), type(token_t::Integer), value(integer) {}

    /**
     * @brief Returns pointer to this token's value.
     *
     * @return Pointer to the token's value.
     */
    union value* operator->() { return &value; }

    /**
     * @brief Returns constant pointer to this token's value.
     *
     * @return Pointer to the token's value.
     */
    const union value* operator->() const { return &value; }

    /**
     * @brief Compares two tokens for equality.
     *
     * @param rhs Token to compare against.
     * @return true if the tokens are equal.
     */
    bool operator==(const token& rhs) const {
        if (type != rhs.type) return false;
        switch (type) {
        case token_t::Identifier:
        case token_t::String: return value.string == rhs.value.string;
        case token_t::Keyword: return value.keyword == rhs.value.keyword;
        case token_t::Integer: return value.integer == rhs.value.integer;
        default: return true;
        }
    }
};

static inline std::ostream& operator<<(std::ostream& os, const token& token) {
    switch (token.type) {
    case token_t::Identifier:
    case token_t::String: return os << token.value.string;
    case token_t::Keyword: return os << token.value.keyword;
    case token_t::Integer: return os << token.value.integer;
    default: return os << token.type;
    }
}

/**
 * @brief Token error type
 */
enum class token_error_t {
    EndOfFile,           /**< End of file */
    UnexpectedEof,       /**< Unexpected end of file */
    UnexpectedCharacter, /**< Unexpected character */
    UnexpectedToken,     /**< Unexpected character */
    IntegerOverflow,     /**< Integer overflow */
};

/**
 * @brief Token error
 *
 * For token_r alias.
 */
struct token_error {
    location      location; /**< Error location. */
    token_error_t type;     /**< Error type. */
    std::string   message;  /**< Error message. */

    /**
     * @brief Compares two token errors for equality.
     *
     * @param rhs Error to compare against.
     * @return true if the errors are equal.
     */
    bool operator==(const token_error& rhs) const {
        return location == rhs.location && type == rhs.type && message == rhs.message;
    }

    /**
     * @brief Compares two token errors for inequality.
     *
     * @param rhs Error to compare against.
     * @return true if the errors are not equal.
     */
    bool operator!=(const token_error& rhs) const { return !this->operator==(rhs); }

    /**
     * @brief Prints a token error to an output stream.
     *
     * @param os Output stream.
     * @param error Token error to print.
     * @return The output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const token_error& error) {
        return os << error.location << ": error: unknown";
    }
};

using token_r = furlang::result<token, token_error>; /**< Alias to a token result. */

} // namespace front
} // namespace furc

#endif // FURC_FRONT_TOKEN_HPP
