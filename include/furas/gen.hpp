#ifndef FURAS_GEN_HPP
#define FURAS_GEN_HPP

#include "furas/lexer.hpp"
#include "furvm/module.hpp"

#include <string>

namespace furas {

struct generator_error {
    enum type {
        Success = 0,
        Eof     = 1,

        UnexpectedEof    = -1,
        UnexpectedToken  = -2,
        UnknownCharacter = -3,
        UnknownType      = -4,
    } type = Success;
    std::string message;
};

class generator {
public:
    struct result {
        generator_error error;
        furvm::mod      mod;
    };
public:
    static result generate(lexer lexer);
};

} // namespace furas

#endif // FURAS_GEN_HPP
