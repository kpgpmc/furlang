#ifndef FURAS_TOKEN_HPP
#define FURAS_TOKEN_HPP

#include <cstdint>
#include <string_view>

namespace furas {

struct token {
    enum type {
        Identifier = 0, /**< An identifier. */
        Signed,         /**< A signed integer. */
        Unsigned,       /**< An unsigned integer. */

        // Markers
        Monkey,  /**< Constant marker (`@`). */
        Dolar,   /**< Type marker(`$`). */
        Sha256,  /**< Label marker(`#`). */
        Percent, /**< Variable marker(`%`). The more the better. */

        Dot, /**< . */

        // Keywords
        Import,  /**< `import` keyword for importing functions and types. */
        Public,  /**< `public` access specifier. */
        Private, /**< `private` access specifier. */

        // Instructions
        Push,
        Array,
        Get,
        Drop,
        Dup,
        Clone,
        Ref,
        Add,
        Sub,
        Mul,
        Div,
        Mod,
        Eq,
        Neq,
        Lt,
        Gt,
        Le,
        Ge,
        Ptrof,
        Sizeof,
        Lenof,
        Load,
        Store,
        Call,
        Jump,
        Jnz,
        Ret,

        Count
    } type = Count;
    union value {
        std::nullptr_t   null = nullptr;
        std::string_view string;
        std::int64_t     integer;
        std::uint64_t    uint;
    } value;

    token(enum type type)
      : type(type) {}

    token(enum type type, std::string_view string)
      : type(type) {
        value.string = string;
    }

    token(std::uint64_t num)
      : type(Unsigned) {
        value.uint = num;
    }

    token(std::int64_t num)
      : type(Signed) {
        value.integer = num;
    }
};

} // namespace furas

#endif // FURAS_TOKEN_HPP
