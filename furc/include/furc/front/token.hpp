#ifndef FURC_FRONT_TOKEN_HPP
#define FURC_FRONT_TOKEN_HPP

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace furc {

struct token {
    enum type {
        Identifier = 0,
        Integer,
        String,

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

    token(location loc, std::string_view string)
      : loc(loc), type(String) {
        value.string = string;
    }

    token(location loc, char character)
      : loc(loc), type(UnexpectedCharacter) {
        value.character = character;
    }
};

using token_t = enum token::type;

} // namespace furc

#endif // FURC_FRONT_TOKEN_HPP
