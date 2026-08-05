#ifndef FURC_FRONT_TOKEN_HPP
#define FURC_FRONT_TOKEN_HPP

#include <cstddef>
#include <cstdint>
#include <ostream>
#include <string_view>

namespace furc {

struct token {
    enum type {
        Identifier = 0,
        Integer,
        String,
        Char,

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

        Plus,         /**< `+` */
        Minus,        /**< `-` */
        Star,         /**< `*` */
        Slash,        /**< `/` */
        Percent,      /**< `%` */
        Ampersand,    /**< `&` */
        Pipe,         /**< `|` */
        Hat,          /**< `^` */
        DblAmpersand, /**< `&&` */
        DblPipe,      /**< `||` */

        DblPlus,  /**< `++` */
        DblMinus, /**< `--` */
        ExMark,   /**< `!` */
        CatEars,  /**< `^^` */

        Equals,          /**< `=` */
        PlusEquals,      /**< `+=` */
        MinusEquals,     /**< `-=` */
        StarEquals,      /**< `*=` */
        SlashEquals,     /**< `/=` */
        PercentEquals,   /**< `%=` */
        AmpersandEquals, /**< `&=` */
        PipeEquals,      /**< `|=` */
        HatEquals,       /**< `^=` */

        DblEquals,     /**< `==` */
        ExEquals,      /**< `!=` */
        LessThan,      /**< `<` */
        LessEquals,    /**< `<=` */
        GreaterThan,   /**< `>` */
        GreaterEquals, /**< `>=` */

        SlimArrow, /**< `->` */
        // My brother just another me
        FatArrow, /**< `=>` */

        Monkey, /**< `@` */
        Sha256, /**< `#` */

        Func,    /**< `func` */
        Return,  /**< `return` */
        If,      /**< `if` */
        Else,    /**< `else` */
        While,   /**< `while` */
        Public,  /**< `public` */
        Private, /**< `private` */

        Pointerof, /**< `pointerof` */
        Sizeof,    /**< `sizeof` */
        Lengthof,  /**< `lengthof` */

        S8,  /**< `s8` */
        U8,  /**< `u8` */
        S16, /**< `s16` */
        U16, /**< `u16` */
        S32, /**< `s32` */
        U32, /**< `u32` */
        S64, /**< `s64` */
        U64, /**< `u64` */

        // Errors:
        UnexpectedCharacter,
        UnexpectedEOF,
        EndOfFile,
    } type;
    union value {
        std::nullptr_t   null = nullptr;
        std::uint64_t    integer;
        std::string_view string;
        char             character;
    } value;

    struct location {
        std::string_view filepath;
        std::size_t      row = 0;
        std::size_t      col = 0;
    } loc;

    token(location loc, enum type type)
      : loc(loc), type(type) {}

    token(location loc, std::uint64_t integer)
      : loc(loc), type(Integer) {
        value.integer = integer;
    }

    token(location loc, enum type type, std::string_view string)
      : loc(loc), type(type) {
        value.string = string;
    }

    token(location loc, enum type type, char character)
      : loc(loc), type(type) {
        value.character = character;
    }

    friend std::ostream& operator<<(std::ostream& os, const token& token) {
        switch (token.type) {
        case token::Identifier: return os << token.value.string;
        case token::String: return os << '"' << token.value.string << '"';
        case token::Char: return os << '\'' << token.value.character << '\'';
        case token::Integer: return os << token.value.integer;
        case token::LParen: return os << "(";
        case token::RParen: return os << ")";
        case token::LBrace: return os << "{";
        case token::RBrace: return os << "}";
        case token::LBracket: return os << "[";
        case token::RBracket: return os << "]";
        case token::Semicolon: return os << ";";
        case token::Colon: return os << ":";
        case token::Comma: return os << ",";
        case token::Dot: return os << ".";
        case token::Plus: return os << "+";
        case token::Minus: return os << "-";
        case token::Star: return os << "*";
        case token::Slash: return os << "/";
        case token::Percent: return os << "%";
        case token::Ampersand: return os << "&";
        case token::Pipe: return os << "|";
        case token::Hat: return os << "^";
        case token::DblAmpersand: return os << "&&";
        case token::DblPipe: return os << "||";
        case token::DblPlus: return os << "++";
        case token::DblMinus: return os << "--";
        case token::ExMark: return os << "!";
        case token::CatEars: return os << "^^";
        case token::Equals: return os << "=";
        case token::PlusEquals: return os << "+=";
        case token::MinusEquals: return os << "-=";
        case token::StarEquals: return os << "*=";
        case token::SlashEquals: return os << "/=";
        case token::PercentEquals: return os << "%=";
        case token::AmpersandEquals: return os << "&=";
        case token::PipeEquals: return os << "|=";
        case token::HatEquals: return os << "^=";
        case token::DblEquals: return os << "==";
        case token::ExEquals: return os << "!=";
        case token::LessThan: return os << "<";
        case token::LessEquals: return os << "<=";
        case token::GreaterThan: return os << ">";
        case token::GreaterEquals: return os << ">=";
        case token::SlimArrow: return os << "->";
        case token::FatArrow: return os << "=>";
        case token::Monkey: return os << "@";
        case token::Sha256: return os << "#";
        case token::Func: return os << "func";
        case token::Return: return os << "return";
        case token::If: return os << "if";
        case token::Else: return os << "else";
        case token::While: return os << "while";
        case token::Public: return os << "public";
        case token::Private: return os << "private";
        case token::Pointerof: return os << "pointerof";
        case token::Sizeof: return os << "sizeof";
        case token::Lengthof: return os << "lengthof";
        case token::S8: return os << "s8";
        case token::U8: return os << "u8";
        case token::S16: return os << "s16";
        case token::U16: return os << "u16";
        case token::S32: return os << "s32";
        case token::U32: return os << "u32";
        case token::S64: return os << "s64";
        case token::U64: return os << "u64";
        case token::UnexpectedCharacter: return os << "Unexpected character `" << token.value.character << "`";
        case token::UnexpectedEOF: return os << "Unexpected End Of File";
        case token::EndOfFile: return os << "End Of File";
        }

        return os;
    }
};

using token_t = enum token::type;

} // namespace furc

#endif // FURC_FRONT_TOKEN_HPP
