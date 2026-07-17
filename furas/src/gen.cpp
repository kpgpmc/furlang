#include "furas/gen.hpp"

#include "furas/token.hpp"
#include "furvm/function.hpp"
#include "furvm/fwd.hpp"
#include "furvm/instruction.hpp"

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

namespace furas {

using namespace std::string_literals;

namespace {

struct instruction {
    furvm::instruction_t fur{};
    enum arg_type {
        None = 0,
        Type,
        Constant,
        Variable,
        Function,
        Label,
    } arg = None;
};

// NOLINTBEGIN
std::unordered_map<enum token::type, instruction> instructions = {
    { token::Array, { furvm::instruction_t::Array, instruction::Type } },
    { token::Get, { furvm::instruction_t::Get } },
    { token::Set, { furvm::instruction_t::Set } },
    { token::Drop, { furvm::instruction_t::Drop } },
    { token::Dup, { furvm::instruction_t::Duplicate } },
    { token::Clone, { furvm::instruction_t::Clone } },
    { token::Ref, { furvm::instruction_t::Reference } },
    { token::Add, { furvm::instruction_t::Add } },
    { token::Sub, { furvm::instruction_t::Sub } },
    { token::Mul, { furvm::instruction_t::Mul } },
    { token::Div, { furvm::instruction_t::Div } },
    { token::Mod, { furvm::instruction_t::Mod } },
    { token::Eq, { furvm::instruction_t::Equals } },
    { token::Neq, { furvm::instruction_t::NotEquals } },
    { token::Lt, { furvm::instruction_t::LessThan } },
    { token::Gt, { furvm::instruction_t::GreaterThan } },
    { token::Le, { furvm::instruction_t::LessEqual } },
    { token::Ge, { furvm::instruction_t::GreaterEqual } },
    { token::Ptrof, { furvm::instruction_t::Pointerof } },
    { token::Sizeof, { furvm::instruction_t::Sizeof } },
    { token::Lenof, { furvm::instruction_t::Lengthof } },
    { token::Load, { furvm::instruction_t::Load, instruction::Variable } },
    { token::Store, { furvm::instruction_t::Store, instruction::Variable } },
    { token::Call, { furvm::instruction_t::Call, instruction::Function } },
    { token::Jmp, { furvm::instruction_t::Jump, instruction::Label } },
    { token::Jnz, { furvm::instruction_t::JumpNotZero, instruction::Label } },
    { token::Ret, { furvm::instruction_t::Return } },
};
// NOLINTEND

const char* token_type(enum token::type type) {
    switch (type) {
    case token::Identifier: return "identifier";
    case token::Signed: return "signed";
    case token::Unsigned: return "unsigned";
    case token::Monkey: return "'@'";
    case token::Dolar: return "'$'";
    case token::Sha256: return "'#'";
    case token::Percent: return "'%'";
    case token::EqSign: return "'='";
    case token::Dot: return "'.'";
    case token::Colon: return "':'";
    case token::Func: return "func";
    case token::Type: return "type";
    case token::Native: return "native";
    case token::Import: return "import";
    case token::Public: return "public";
    case token::Private: return "private";
    case token::Push: return "push";
    case token::Array: return "array";
    case token::Get: return "get";
    case token::Set: return "set";
    case token::Drop: return "drop";
    case token::Dup: return "dup";
    case token::Clone: return "clone";
    case token::Ref: return "ref";
    case token::Add: return "add";
    case token::Sub: return "sub";
    case token::Mul: return "mul";
    case token::Div: return "div";
    case token::Mod: return "mod";
    case token::Eq: return "eq";
    case token::Neq: return "neq";
    case token::Lt: return "lt";
    case token::Gt: return "gt";
    case token::Le: return "le";
    case token::Ge: return "ge";
    case token::Ptrof: return "ptrof";
    case token::Sizeof: return "sizeof";
    case token::Lenof: return "lenof";
    case token::Load: return "load";
    case token::Store: return "store";
    case token::Call: return "call";
    case token::Jmp: return "jmp";
    case token::Jnz: return "jnz";
    case token::Ret: return "ret";
    case token::Count: break;
    }
    throw std::runtime_error("unreachable");
}

struct mod_context {
    struct label_context {
        struct function_info {
            std::string         name;
            furvm::function_sig signature;
            bool                pub;
        };

        static constexpr std::size_t INVALID = std::numeric_limits<std::size_t>::max();

        std::size_t                offset = INVALID;
        std::vector<function_info> functions;
        std::vector<std::size_t>   unknowns;
    };

    furvm::mod mod;
    std::unordered_map<std::pair<std::string, furvm::function_sig>,
        furvm::function_h,
        furlang::utility::
            pair_hash<std::string, furvm::function_sig, std::hash<std::string>, furvm::detail::function_sig_hash>>
                                                       functions;
    std::unordered_map<std::string, furvm::mod_type_h> types;

    std::unordered_map<std::string, label_context> labels;

    struct token_result {
        generator_error error;
        token           token;

        struct token* operator->() { return &token; }
        struct token& operator*() { return token; }

        bool operator!() const { return error.type != generator_error::Success; }
    };

    mod_context() {
        types.emplace("s8", mod.emplace_type(furvm::mod_type::S8));
        types.emplace("u8", mod.emplace_type(furvm::mod_type::U8));
        types.emplace("s16", mod.emplace_type(furvm::mod_type::S16));
        types.emplace("u16", mod.emplace_type(furvm::mod_type::U16));
        types.emplace("s32", mod.emplace_type(furvm::mod_type::S32));
        types.emplace("u32", mod.emplace_type(furvm::mod_type::U32));
        types.emplace("s64", mod.emplace_type(furvm::mod_type::S64));
        types.emplace("u64", mod.emplace_type(furvm::mod_type::U64));
    }

    static token_result next_token(lexer& lexer) {
        auto token = lexer.next_token();
        if (token.has_error()) {
            switch (token.error().type) {
            case lexer_error::EndOfFile:
                return { { generator_error::UnexpectedEof, "Unexpected end of file" }, { token::Count } };
            case lexer_error::UnknownCharacter:
                return { { generator_error::UnknownCharacter, token.error().message }, { token::Count } };
            }
        }
        return { { generator_error::Success }, *token };
    }

    static token_result eat_token(lexer& lexer, enum token::type type) {
        auto token = next_token(lexer);
        if (!token) return token;
        if (token.token.type != type) {
            return { { generator_error::UnexpectedToken,
                         "Expected "s + token_type(type) + ", but got " + token_type(token->type) },
                { token::Count } };
        }
        return { { generator_error::Success }, token.token };
    }

    generator_error generate(lexer& lexer) {
        auto result = next_token(lexer);
        if (result.error.type == generator_error::UnexpectedEof) return { generator_error::Eof };
        if (!result) return result.error;
        switch (result->type) {
        case token::Identifier: {
            std::string labelName = std::string(result->value.string);

            result = eat_token(lexer, token::Colon);
            if (!result) return result.error;

            auto& label = labels[labelName];
            if (label.offset != label_context::INVALID)
                return { generator_error::UnexpectedToken, "Label "s + labelName + " already exists" };
            label.offset = mod.bytecode().size();

            for (auto& func : label.functions) {
                furvm::function_h handle =
                    (func.pub ? mod.emplace_function(func.name, std::move(func.signature), label.offset)
                              : mod.emplace_function(std::move(func.signature), label.offset));

                functions.emplace(std::make_pair(std::move(func.name), std::move(func.signature)), handle);

                handle.dispatch();
            }
            for (auto unknown : label.unknowns) {
                std::ptrdiff_t jmpOff = static_cast<std::ptrdiff_t>(label.offset - unknown);
                if (jmpOff < std::numeric_limits<std::int8_t>::min() ||
                    jmpOff > std::numeric_limits<std::int8_t>::max()) {
                    assert(false); // TODO: Further jumps are not implemented
                }
                mod.bytecode()[unknown - 1] = jmpOff;
            }
            label.functions = {};
            label.unknowns  = {};
            return { generator_error::Success };
        }

        case token::Func:
        case token::Type:
        case token::Public:
        case token::Private: {
            bool pub = false;
            if (result->type == token::Public) {
                pub    = true;
                result = next_token(lexer);
                if (!result) return { result.error };
            } else if (result->type == token::Private) {
                result = next_token(lexer);
                if (!result) return { result.error };
            }

            if (result->type == token::Func) {
                auto nameRes = next_token(lexer);
                if (!nameRes) return nameRes.error;

                furvm::function_sig signature;

                result = next_token(lexer);
                if (!result) return result.error;
                while (result->type == token::Dolar) {
                    result = eat_token(lexer, token::Identifier);
                    if (!result) return result.error;
                    if (auto it = types.find(std::string(result->value.string)); it != types.end()) {
                        signature.params.push_back(it->second);
                    } else {
                        return { generator_error::UnknownType, "Unknown type "s + std::string(result->value.string) };
                    }

                    result = next_token(lexer);
                    if (!result) return result.error;
                }
                if (result->type != token::EqSign) return result.error;

                result = next_token(lexer);
                if (!result) return result.error;
                if (result->type == token::Dolar) {
                    result = eat_token(lexer, token::Identifier);
                    if (!result) return result.error;
                    if (auto it = types.find(std::string(result->value.string)); it != types.end()) {
                        signature.returnType = it->second;
                    } else {
                        return { generator_error::UnknownType, "Unknown type "s + std::string(result->value.string) };
                    }

                    result = next_token(lexer);
                    if (!result) return result.error;
                }

                switch (result->type) {
                case token::Sha256: {
                    result = eat_token(lexer, token::Identifier);
                    if (!result) return result.error;

                    std::string name = std::string(nameRes->value.string);

                    std::string labelName = std::string(result->value.string);
                    std::size_t offset    = 0;
                    if (auto it = labels.find(labelName); it != labels.end()) {
                        offset = it->second.offset;
                    } else {
                        labels[labelName].functions.emplace_back(
                            label_context::function_info{ std::move(name), std::move(signature), pub });
                        return { generator_error::Success };
                    }

                    furvm::function_h handle =
                        (pub ? mod.emplace_function(name, signature, offset) : mod.emplace_function(signature, offset));

                    functions.emplace(std::make_pair(name, std::move(signature)), handle);

                    handle.dispatch();
                    return { generator_error::Success };
                }
                case token::Native: {
                    result = eat_token(lexer, token::Identifier);
                    if (!result) return result.error;

                    std::string nativeName = std::string(result->value.string);

                    std::string name = std::string(nameRes->value.string);

                    furvm::function_h handle = (pub ? mod.emplace_function(name, signature, std::move(nativeName))
                                                    : mod.emplace_function(signature, std::move(nativeName)));

                    functions.emplace(std::make_pair(name, std::move(signature)), handle);

                    handle.dispatch();
                    return { generator_error::Success };
                }
                case token::Import: {
                    throw std::runtime_error("unimplemented");
                }
                default:
                    return { generator_error::UnexpectedToken,
                        "Unexpected token "s + token_type(result->type) +
                            ", expected label name, `native`, or `import`" };
                }
            }

            if (result->type == token::Type) {
                auto nameRes = eat_token(lexer, token::Identifier);
                if (!nameRes) return nameRes.error;

                result = eat_token(lexer, token::EqSign);
                if (!result) return result.error;

                result = next_token(lexer);
                if (!result) return result.error;
                switch (result->type) {
                case token::Import: {
                    auto typeNameRes = eat_token(lexer, token::Identifier);
                    if (!typeNameRes) return typeNameRes.error;
                    return { generator_error::Success };
                }
                case token::Array: {
                    result = eat_token(lexer, token::Dolar);
                    if (!result) return result.error;

                    auto typeNameRes = eat_token(lexer, token::Identifier);
                    if (!typeNameRes) return typeNameRes.error;

                    auto size = eat_token(lexer, token::Unsigned);
                    if (!size) return size.error;

                    furvm::mod_type_id innerId = 0;
                    if (auto it = types.find(std::string(typeNameRes->value.string)); it != types.end()) {
                        innerId = it->second.id();
                    } else {
                        return { generator_error::UnknownType,
                            "Unknown type "s + std::string(typeNameRes->value.string) };
                    }

                    auto type = mod.emplace_type(innerId, size->value.uint);
                    types.emplace(std::string(nameRes->value.string), type);
                    type.dispatch();

                    return { generator_error::Success };
                }
                default:
                    return { generator_error::UnexpectedToken,
                        "Unexpected token "s + token_type(result->type) +
                            ", expected either type, `import`, or `array`" };
                }
            }

            return { generator_error::UnexpectedToken,
                "Unexpected token "s + token_type(result->type) + ", expected either `func` or `type`" };
        }

        case token::Push: {
            auto result = eat_token(lexer, token::Dolar);
            if (!result) return result.error;
            auto typeName = eat_token(lexer, token::Identifier);
            if (!typeName) return result.error;
            auto it = types.find(std::string(typeName->value.string));
            if (it == types.end())
                return { generator_error::UnexpectedToken, "Unknown type "s + std::string(typeName->value.string) };
            auto value = eat_token(lexer, token::Unsigned);
            if (!value) return result.error;
            auto type = it->second;
            switch (type->type) {
            case furvm::mod_type::S8: {
                mod.bytecode().push_back(static_cast<furvm::byte>(furvm::instruction_t::PushS8));
            } break;
            case furvm::mod_type::U8: {
                mod.bytecode().push_back(static_cast<furvm::byte>(furvm::instruction_t::PushU8));
            } break;
            case furvm::mod_type::S16: {
                mod.bytecode().push_back(static_cast<furvm::byte>(furvm::instruction_t::PushS16));
            } break;
            case furvm::mod_type::U16: {
                mod.bytecode().push_back(static_cast<furvm::byte>(furvm::instruction_t::PushU16));
            } break;
            case furvm::mod_type::S32: {
                mod.bytecode().push_back(static_cast<furvm::byte>(furvm::instruction_t::PushS32));
            } break;
            case furvm::mod_type::U32: {
                mod.bytecode().push_back(static_cast<furvm::byte>(furvm::instruction_t::PushU32));
            } break;
            default:
                return { generator_error::UnexpectedToken, "Unexpected type "s + std::string(typeName->value.string) };
            }
            return { generator_error::Success };
        }

        case token::Array:
        case token::Get:
        case token::Set:
        case token::Drop:
        case token::Dup:
        case token::Clone:
        case token::Ref:
        case token::Add:
        case token::Sub:
        case token::Mul:
        case token::Div:
        case token::Mod:
        case token::Eq:
        case token::Neq:
        case token::Lt:
        case token::Gt:
        case token::Le:
        case token::Ge:
        case token::Ptrof:
        case token::Sizeof:
        case token::Lenof:
        case token::Load:
        case token::Store:
        case token::Call:
        case token::Jmp:
        case token::Jnz:
        case token::Ret: {
            auto it = instructions.find(result->type);
            assert(it != instructions.end());
            mod.bytecode().push_back(static_cast<furvm::byte>(it->second.fur));
            switch (it->second.arg) {
            case instruction::None: break;
            case instruction::Type: {
                result = eat_token(lexer, token::Dolar);
                if (!result) return result.error;
                result = eat_token(lexer, token::Identifier);
                if (!result) return result.error;
                auto type = types.find(std::string(result->value.string));
                if (type == types.end())
                    return { generator_error::UnknownType, "Unknown type "s + std::string(result->value.string) };
                auto id = type->second.id();
                mod.bytecode().push_back((id >> 0) & 0xFF);
                mod.bytecode().push_back((id >> 8) & 0xFF);
                mod.bytecode().push_back((id >> 16) & 0xFF);
                mod.bytecode().push_back((id >> 24) & 0xFF);
            } break;
            case instruction::Constant: {
                assert(false); // TODO: Unimplemented
            } break;
            case instruction::Variable: {
                result = eat_token(lexer, token::Percent);
                if (!result) return result.error;
                result = eat_token(lexer, token::Unsigned);
                if (!result) return result.error;
                std::uint16_t var = result->value.uint;
                mod.bytecode().push_back((var >> 0) & 0xFF);
                mod.bytecode().push_back((var >> 8) & 0xFF);
            } break;
            case instruction::Function: {
                furvm::function_sig signature;
                while ((result = next_token(lexer)).error.type == generator_error::Success &&
                       result->type == token::Dolar) {
                    result = eat_token(lexer, token::Identifier);
                    if (!result) return result.error;
                    auto type = types.find(std::string(result->value.string));
                    if (type == types.end())
                        return { generator_error::UnknownType, "Unknown type "s + std::string(result->value.string) };
                    signature.params.push_back(type->second);
                }
                if (!result || result->type != token::Identifier) return result.error;
                std::string name(result->value.string);

                auto func = functions.find(std::make_pair(name, signature));
                if (func == functions.end())
                    return { generator_error::UnknownType, "Unknown type "s + std::string(result->value.string) };
                auto id = func->second.id();
                mod.bytecode().push_back((id >> 0) & 0xFF);
                mod.bytecode().push_back((id >> 8) & 0xFF);
            } break;
            case instruction::Label: {
                result = eat_token(lexer, token::Sha256);
                if (!result) return result.error;
                result = eat_token(lexer, token::Identifier);
                if (!result) return result.error;
                auto& label  = labels[std::string(result->value.string)];
                auto  offset = label.offset;
                if (offset == label_context::INVALID) {
                    label.unknowns.push_back(mod.bytecode().size() + 1);
                    mod.bytecode().push_back(0);
                    return { generator_error::Success };
                }
                std::ptrdiff_t jmpOff = static_cast<std::ptrdiff_t>(offset - mod.bytecode().size() + 1);
                if (jmpOff < std::numeric_limits<std::int8_t>::min() ||
                    jmpOff > std::numeric_limits<std::int8_t>::max()) {
                    assert(false); // TODO: Further jumps are not implemented
                }
                mod.bytecode().push_back(jmpOff);
            } break;
            }
            return { generator_error::Success };
        }

        case token::Signed:
        case token::Unsigned:
        case token::Colon:
        case token::Monkey:
        case token::Dolar:
        case token::Sha256:
        case token::Percent:
        case token::EqSign:
        case token::Dot:
        case token::Native:
        case token::Import:
        case token::Count: break;
        }
        return { generator_error::UnexpectedToken, "Unexpected token "s + token_type(result->type) };
    }
};

} // namespace

generator::result generator::generate(lexer lexer) {
    mod_context     context;
    generator_error error;
    do {
        error = context.generate(lexer);
    } while (error.type == generator_error::Success);
    if (error.type == generator_error::Eof) return { { generator_error::Success }, std::move(context.mod) };
    return { error };
}

} // namespace furas
