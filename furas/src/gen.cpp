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

// NOLINTBEGIN
std::unordered_map<enum token::type, furvm::instruction_t> instructions = {
    { token::Array, furvm::instruction_t::Array },
    { token::Get, furvm::instruction_t::Get },
    { token::Set, furvm::instruction_t::Set },
    { token::Drop, furvm::instruction_t::Drop },
    { token::Dup, furvm::instruction_t::Duplicate },
    { token::Swap, furvm::instruction_t::Swap },
    { token::Clone, furvm::instruction_t::Clone },
    { token::Ref, furvm::instruction_t::Reference },
    { token::Add, furvm::instruction_t::Add },
    { token::Sub, furvm::instruction_t::Sub },
    { token::Mul, furvm::instruction_t::Mul },
    { token::Div, furvm::instruction_t::Div },
    { token::Mod, furvm::instruction_t::Mod },
    { token::Eq, furvm::instruction_t::Equals },
    { token::Neq, furvm::instruction_t::NotEquals },
    { token::Lt, furvm::instruction_t::LessThan },
    { token::Gt, furvm::instruction_t::GreaterThan },
    { token::Le, furvm::instruction_t::LessEqual },
    { token::Ge, furvm::instruction_t::GreaterEqual },
    { token::Ptrof, furvm::instruction_t::Pointerof },
    { token::Sizeof, furvm::instruction_t::Sizeof },
    { token::Lenof, furvm::instruction_t::Lengthof },
    { token::Load, furvm::instruction_t::Load },
    { token::Store, furvm::instruction_t::Store },
    { token::LoadGlobal, furvm::instruction_t::LoadGlobal },
    { token::StoreGlobal, furvm::instruction_t::StoreGlobal },
    { token::Call, furvm::instruction_t::Call },
    { token::Jmp, furvm::instruction_t::Jump },
    { token::Jnz, furvm::instruction_t::JumpNotZero },
    { token::Ret, furvm::instruction_t::Return },
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
    case token::Allocate: return "allocate";
    case token::Push: return "push";
    case token::Array: return "array";
    case token::Get: return "get";
    case token::Set: return "set";
    case token::Drop: return "drop";
    case token::Dup: return "dup";
    case token::Swap: return "swap";
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
    case token::LoadGlobal: return "loadg";
    case token::StoreGlobal: return "storeg";
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

    furvm::mod                                                      mod;
    std::unordered_map<std::string, std::vector<furvm::function_h>> functions;
    std::unordered_map<std::string, furvm::mod_type_h>              types;
    std::unordered_map<std::string, std::uint16_t>                  variables;

    std::unordered_map<std::string, label_context> labels;

    struct token_result {
        generator_error error;
        token           value;

        token* operator->() { return &value; }
        token& operator*() { return value; }

        bool operator!() const { return error.type != generator_error::Success; }
    };

    bool compare_types(const furvm::mod_type_h& lhs, const furvm::mod_type_h& rhs) const {
        if (lhs->type != rhs->type) return false;
        switch (lhs->type) {
        case furvm::mod_type::S8:
        case furvm::mod_type::S16:
        case furvm::mod_type::S32:
        case furvm::mod_type::S64:
        case furvm::mod_type::U8:
        case furvm::mod_type::U16:
        case furvm::mod_type::U32:
        case furvm::mod_type::U64: return true;
        case furvm::mod_type::Ptr:
        case furvm::mod_type::Ref:
            return compare_types(mod.type_at(lhs->value.typeRef), mod.type_at(rhs->value.typeRef));
        case furvm::mod_type::Array:
            return lhs->value.array.size == rhs->value.array.size &&
                   compare_types(mod.type_at(lhs->value.array.typeId), mod.type_at(rhs->value.array.typeId));
        case furvm::mod_type::Import:
            return lhs->value.imprt.modId == rhs->value.imprt.modId &&
                   lhs->value.imprt.typeId == rhs->value.imprt.typeId;
        case furvm::mod_type::Count: break;
        }
        throw std::runtime_error("unreachable");
    }

    furvm::function_h find_function(const std::string& name, const furvm::function_sig& signature) const {
        if (auto it = functions.find(name); it != functions.end()) {
            for (const auto& func : it->second) {
                if (func->signature().params.size() != signature.params.size()) continue;
                std::size_t idx = 0;
                while (idx < signature.params.size()) {
                    if (!compare_types(func->signature().params[idx], signature.params[idx])) break;
                    ++idx;
                }
                if (idx == signature.params.size()) return func;
            }
        }
        return {};
    }

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
        if (token.value.type != type) {
            return { { generator_error::UnexpectedToken,
                         "Expected "s + token_type(type) + ", but got " + token_type(token->type) },
                { token::Count } };
        }
        return { { generator_error::Success }, token.value };
    }

    furvm::mod_type_h eat_type(lexer& lexer, const token& tok) {
        switch (tok.type) {
        case token::Dolar: {
            auto result = eat_token(lexer, token::Identifier);
            if (auto it = types.find(std::string(result->value.string)); it != types.end()) {
                return it->second;
            }
            throw std::runtime_error("unknown type");
        }
        case token::Import: {
            throw std::runtime_error("unimplemented");
            // auto typeNameRes = eat_token(lexer, token::Identifier);
            // if (!typeNameRes) return typeNameRes.error;
            // return { generator_error::Success };
        }
        case token::Ref: {
            auto result = next_token(lexer);
            if (!result) throw std::runtime_error("error");

            auto inner = eat_type(lexer, result.value);

            return mod.emplace_type(furvm::mod_type::Ref, inner.id());
        }
        case token::Array: {
            auto result = next_token(lexer);
            if (!result) throw std::runtime_error("error");

            auto inner = eat_type(lexer, result.value);

            auto size = eat_token(lexer, token::Unsigned);
            if (!size) throw std::runtime_error("error");

            return mod.emplace_type(inner.id(), size->value.uint);
        }
        default: throw std::runtime_error("error");
        }
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

                functions[std::move(func.name)].push_back(handle);
                handle.dispatch();
            }
            for (auto unknown : label.unknowns) {
                const auto jmpOff = static_cast<std::ptrdiff_t>(label.offset) - static_cast<std::ptrdiff_t>(unknown);
                if (jmpOff < std::numeric_limits<std::int8_t>::min() ||
                    jmpOff > std::numeric_limits<std::int8_t>::max()) {
                    assert(false); // TODO: Further jumps are not implemented
                }
                mod.bytecode()[unknown - 1] = static_cast<std::int8_t>(jmpOff);
            }
            label.functions = {};
            label.unknowns  = {};
            return { generator_error::Success };
        }

        case token::Allocate: {
            result = eat_token(lexer, token::Identifier);
            if (!result) return result.error;
            std::string name = std::string(result->value.string);
            variables.emplace(name, variables.size());
            mod.set_global_variable_count(variables.size());
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
                while (result->type != token::EqSign) {
                    signature.params.push_back(eat_type(lexer, result.value));

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

                std::string name = std::string(nameRes->value.string);

                if (!find_function(name, signature).empty()) throw std::runtime_error("function already defined");

                switch (result->type) {
                case token::Sha256: {
                    result = eat_token(lexer, token::Identifier);
                    if (!result) return result.error;

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
                    functions[name].push_back(handle);
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
                    functions[name].push_back(handle);
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

                auto type = eat_type(lexer, result.value);
                types.emplace(std::string(nameRes->value.string), std::move(type));
                return { generator_error::Success };
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
            // TODO: Add support for signed integers
            auto value = eat_token(lexer, token::Unsigned);
            if (!value) return result.error;
            auto type = it->second;
            switch (type->type) {
            case furvm::mod_type::S8: {
                mod.bytecode().push_back(static_cast<furvm::byte>(furvm::instruction_t::PushS8));
                mod.bytecode().push_back(value->value.uint & 0xFF);
            } break;
            case furvm::mod_type::U8: {
                mod.bytecode().push_back(static_cast<furvm::byte>(furvm::instruction_t::PushU8));
                mod.bytecode().push_back(value->value.uint & 0xFF);
            } break;
            case furvm::mod_type::S16: {
                mod.bytecode().push_back(static_cast<furvm::byte>(furvm::instruction_t::PushS16));
                mod.bytecode().push_back(value->value.uint & 0xFF);
                mod.bytecode().push_back((value->value.uint >> 8) & 0xFF);
            } break;
            case furvm::mod_type::U16: {
                mod.bytecode().push_back(static_cast<furvm::byte>(furvm::instruction_t::PushU16));
                mod.bytecode().push_back(value->value.uint & 0xFF);
                mod.bytecode().push_back((value->value.uint >> 8) & 0xFF);
            } break;
            case furvm::mod_type::S32: {
                mod.bytecode().push_back(static_cast<furvm::byte>(furvm::instruction_t::PushS32));
                mod.bytecode().push_back(value->value.uint & 0xFF);
            } break;
            case furvm::mod_type::U32: {
                mod.bytecode().push_back(static_cast<furvm::byte>(furvm::instruction_t::PushU32));
                mod.bytecode().push_back(value->value.uint & 0xFF);
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
        case token::Swap:
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
        case token::LoadGlobal:
        case token::StoreGlobal:
        case token::Call:
        case token::Jmp:
        case token::Jnz:
        case token::Ret: {
            auto it = instructions.find(result->type);
            assert(it != instructions.end());
            furvm::instruction instr{ it->second };
            instr.arg.type = furvm::instruction::s_arguments[instr.type];
            switch (instr.arg.type) {
            case furvm::instruction_argument::None: break;
            case furvm::instruction_argument::Type: {
                result = eat_token(lexer, token::Dolar);
                if (!result) return result.error;
                result = eat_token(lexer, token::Identifier);
                if (!result) return result.error;
                auto type = types.find(std::string(result->value.string));
                if (type == types.end())
                    return { generator_error::UnknownType, "Unknown type "s + std::string(result->value.string) };
                instr.arg.u32 = type->second.id();
            } break;
            case furvm::instruction_argument::Constant: {
                assert(false); // TODO: Unimplemented
            } break;
            case furvm::instruction_argument::Variable: {
                result = eat_token(lexer, token::Percent);
                if (!result) return result.error;
                result = eat_token(lexer, token::Unsigned);
                if (!result) return result.error;
                instr.arg.u16 = result->value.uint;
            } break;
            case furvm::instruction_argument::GlobalVariable: {
                result = eat_token(lexer, token::Percent);
                if (!result) return result.error;
                result = eat_token(lexer, token::Identifier);
                if (!result) return result.error;
                if (auto it = variables.find(std::string(result->value.string)); it != variables.end())
                    instr.arg.u16 = it->second;
                else
                    throw std::runtime_error("Unknown global variable");
            } break;
            case furvm::instruction_argument::Function: {
                furvm::function_sig signature;
                while ((result = next_token(lexer)).error.type == generator_error::Success &&
                       result->type != token::Identifier) {
                    signature.params.push_back(eat_type(lexer, result.value));
                }
                if (!result || result->type != token::Identifier) return result.error;
                std::string name(result->value.string);

                auto func = find_function(name, signature);
                if (func.empty())
                    return { generator_error::UnknownType, "Unknown function "s + std::string(result->value.string) };
                instr.arg.u16 = func.id();
            } break;
            case furvm::instruction_argument::Offset: {
                result = eat_token(lexer, token::Sha256);
                if (!result) return result.error;
                result = eat_token(lexer, token::Identifier);
                if (!result) return result.error;
                auto& label  = labels[std::string(result->value.string)];
                auto  offset = label.offset;
                if (offset == label_context::INVALID) {
                    label.unknowns.push_back(mod.bytecode().size() + 2);
                    instr.arg.s8 = 0;
                    break;
                }
                const auto jmpOff =
                    static_cast<std::ptrdiff_t>(offset) - static_cast<std::ptrdiff_t>(mod.bytecode().size()) - 2;
                if (jmpOff < std::numeric_limits<std::int8_t>::min() ||
                    jmpOff > std::numeric_limits<std::int8_t>::max()) {
                    assert(false); // TODO: Further jumps are not implemented
                }
                instr.arg.s8 = static_cast<std::int8_t>(jmpOff);
            } break;
            case furvm::instruction_argument::S8:
            case furvm::instruction_argument::U8:
            case furvm::instruction_argument::S16:
            case furvm::instruction_argument::U16:
            case furvm::instruction_argument::S32:
            case furvm::instruction_argument::U32:
            case furvm::instruction_argument::Count:
            default: throw std::runtime_error("unreachable");
            }
            instr.write(mod.bytecode());
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
