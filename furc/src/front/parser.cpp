#include "furc/front/parser.hpp"

#include "furc/ast/declaration.hpp" // IWYU pragma: keep
#include "furc/ast/expression.hpp"  // IWYU pragma: keep
#include "furc/ast/fwd.hpp"
#include "furc/ast/literal.hpp"   // IWYU pragma: keep
#include "furc/ast/program.hpp"   // IWYU pragma: keep
#include "furc/ast/statement.hpp" // IWYU pragma: keep
#include "furc/front/token.hpp"

#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace furc::front {

using namespace std::string_literals;

parser::parser(furlang::arena& arena, std::string_view filename, std::string_view content)
  : m_filename(filename), m_content(content), m_lexer(m_filename, m_content), m_arena(&arena) {}

parser::parser(furlang::arena& arena, std::string_view filename)
  : m_filename(filename), m_arena(&arena) {
    std::ifstream file(m_filename, std::ios_base::binary | std::ios_base::ate);
    if (!file.is_open()) throw std::runtime_error("failed to open file "s.append(m_filename));
    std::streampos size = file.tellg();
    file.seekg(0);

    m_content.resize(size);
    file.read(m_content.data(), size);
    m_lexer = { filename, m_content };
}

ast::program_node_r parser::parse() & {
    auto program = m_arena->allocate_shared<ast::program_node>(location{ m_filename });

    while (peek_token().has_value()) {
        auto decl = parse_declaration();
        if (decl.has_error()) return ast::program_node_r(ast::error{ decl.error().location });
        program->push(std::move(decl.value()));
    }

    return program;
}

ast::type_r parser::parse_type() {
    auto token = eat_token(token_t::Keyword);
    if (token.has_error() || token.value()->keyword != keyword_token::Int32)
        return ast::type_r(ast::error{ token.error().location });
    return ast::type("" + token.value()->keyword);
}

ast::declaration_node_r parser::parse_declaration() {
    const auto& first = peek_token();
    if (first.has_error()) return ast::declaration_node_r(ast::error{ first.error().location });
    switch (first->type) {
    case token_t::Keyword: {
        token firstToken = *first;

        ast::declaration_access_t accessOverride = ast::declaration_access_t::Implicit;
        switch ((*first)->keyword) {
        default: break;
        case keyword_token::Public:
        case keyword_token::Private: {
            if ((*first)->keyword == keyword_token::Public) accessOverride = ast::declaration_access_t::Public;
            if ((*first)->keyword == keyword_token::Private) accessOverride = ast::declaration_access_t::Private;
            auto kw = eat_token(token_t::Keyword);
            if (kw.has_error()) return ast::declaration_node_r(ast::error{ kw.error().location });
            firstToken = *kw;
        } break;
        }

        ast::function_declaration_node_t funcDeclType{};

        auto kw = next_token();
        if (kw.has_error()) return ast::declaration_node_r(ast::error{ kw.error().location });
        firstToken = *kw;
        switch (firstToken->keyword) {
        case keyword_token::Import:
        case keyword_token::Native: {
            funcDeclType = (firstToken->keyword == keyword_token::Import) ? ast::function_declaration_node_t::Import
                                                                          : ast::function_declaration_node_t::Native;

            auto kw = eat_token(token_t::Keyword);
            if (kw.has_error()) return ast::declaration_node_r(ast::error{ kw.error().location });
            firstToken = *kw;
            if (firstToken.value.keyword != keyword_token::Func)
                return ast::declaration_node_r(ast::error{ firstToken.location });
        }
        case keyword_token::Func: {
            auto name = eat_token(token_t::Identifier);
            if (name.has_error()) return ast::declaration_node_r(ast::error{ name.error().location });

            auto tok = eat_token(token_t::LParen);
            if (tok.has_error()) return ast::declaration_node_r(ast::error{ tok.error().location });

            std::vector<ast::function_declaration_param> params;
            if (peek_token().has_value() && peek_token()->type != token_t::RParen) {
                while (true) {
                    auto name = eat_token(token_t::Identifier);
                    if (name.has_error()) return ast::declaration_node_r(ast::error{ name.error().location });
                    auto colon = eat_token(token_t::Colon);
                    if (colon.has_error()) return ast::declaration_node_r(ast::error{ colon.error().location });
                    auto type = parse_type();
                    if (type.has_error()) return ast::declaration_node_r(ast::error{ type.error().location });

                    params.push_back(
                        ast::function_declaration_param{ std::string(name->value.string), std::move(*type) });

                    auto comma = eat_token(token_t::Comma);
                    if (comma.has_error()) break;
                }
            }

            tok = eat_token(token_t::RParen);
            if (tok.has_error()) return ast::declaration_node_r(ast::error{ tok.error().location });

            std::optional<ast::type> returnType;
            if (peek_token().has_value() && peek_token()->type == token_t::SlimArrow) {
                auto tok  = next_token();
                auto type = parse_type();
                if (type.has_error()) return ast::declaration_node_r(ast::error{ tok->location });
                returnType = *type;
            }

            auto access = (funcDeclType == ast::function_declaration_node_t::Import)
                              ? ast::declaration_access_t::Private
                              : accessOverride;
            if (access == ast::declaration_access_t::Implicit) access = ast::declaration_access_t::Public;
            if (!ast::same_access(accessOverride, access)) return ast::declaration_node_r(ast::error{ tok->location });

            const auto& peek = peek_token();
            if (peek.has_error()) return ast::declaration_node_r(ast::error{ peek.error().location });
            switch (peek->type) {
            case token_t::LBrace: {
                ast::body_r body = parse_body();
                if (body.has_error()) return ast::declaration_node_r(ast::error{ body.error().location });
                if (funcDeclType != ast::function_declaration_node_t::Normal)
                    return ast::declaration_node_r(ast::error{ body->begin });
                return m_arena->allocate_shared<ast::function_definition_node>(firstToken.location,
                    access,
                    name->value.string,
                    std::move(returnType),
                    std::move(params),
                    std::move(body.value()));
            }
            case token_t::Semicolon: {
                m_peekBuffer.clear();
                return m_arena->allocate_shared<ast::function_declaration_node>(firstToken.location,
                    access,
                    name->value.string,
                    std::move(returnType),
                    std::move(params),
                    funcDeclType);
            }
            default: return ast::declaration_node_r(ast::error{ tok->location });
            }
        }
        default: return ast::declaration_node_r(ast::error{ firstToken.location });
        }
    }
    case token_t::None:
    case token_t::Identifier:
    case token_t::Integer:
    case token_t::LParen:
    case token_t::RParen:
    case token_t::LBrace:
    case token_t::RBrace:
    case token_t::LBracket:
    case token_t::RBracket:
    case token_t::Semicolon:
    case token_t::Colon:
    default: {
        return ast::declaration_node_r(ast::error{ first->location });
    }
    }
}

ast::statement_node_r parser::parse_statement() {
    const auto& tok = peek_token();
    if (tok.has_error()) return ast::statement_node_r(ast::error{ tok.error().location });
    auto location = tok->location;
    switch (tok->type) {
    case token_t::Keyword: {
        switch (tok->value.keyword) {
        case keyword_token::Return: {
            auto tok = next_token();
            if (peek_token()->type == token_t::Semicolon) {
                next_token();
                return m_arena->allocate_shared<ast::return_statement_node>(location);
            }

            auto value = parse_expression();
            auto err   = eat_token(token_t::Semicolon);
            if (err.has_error()) return ast::statement_node_r(ast::error{ err.error().location });
            return m_arena->allocate_shared<ast::return_statement_node>(location, std::move(value.value()));
        }
        case keyword_token::If: {
            auto tok = next_token();
            auto err = eat_token(token_t::LParen);
            if (err.has_error()) return ast::statement_node_r(ast::error{ err.error().location });

            auto cond = parse_expression();

            err = eat_token(token_t::RParen);
            if (err.has_error()) return ast::statement_node_r(ast::error{ err.error().location });

            auto then = parse_statement();
            if (then.has_error()) return ast::statement_node_r(ast::error{ then.error().location });

            if (peek_token().has_value() && peek_token()->type == token_t::Keyword &&
                peek_token()->value.keyword == keyword_token::Else) {
                next_token();

                auto elseBody = parse_statement();
                if (elseBody.has_error()) return ast::statement_node_r(ast::error{ elseBody.error().location });
                return m_arena->allocate_shared<ast::if_statement_node>(location,
                    std::move(cond.value()),
                    std::move(then.value()),
                    std::move(elseBody.value()));
            }

            return m_arena->allocate_shared<ast::if_statement_node>(location,
                std::move(cond.value()),
                std::move(then.value()));
        }
        case keyword_token::While: {
            auto tok = next_token();
            auto err = eat_token(token_t::LParen);
            if (err.has_error()) return ast::statement_node_r(ast::error{ err.error().location });

            auto cond = parse_expression();
            if (cond.has_error()) return ast::statement_node_r(ast::error{ cond.error().location });

            err = eat_token(token_t::RParen);
            if (err.has_error()) return ast::statement_node_r(ast::error{ err.error().location });

            auto body = parse_statement();
            if (body.has_error()) return ast::statement_node_r(ast::error{ body.error().location });

            return m_arena->allocate_shared<ast::while_statement_node>(location,
                std::move(cond.value()),
                std::move(body.value()));
        }
        case keyword_token::None:
        case keyword_token::Func:
        default: break;
        }
    }
    case token_t::LBrace: {
        auto body = parse_body();
        if (body.has_error()) return ast::statement_node_r(ast::error{ body.error().location });
        return m_arena->allocate_shared<ast::compound_statement_node>(location, std::move(body.value()));
    }
    default: break;
    }

    auto declaration = parse_declaration();
    if (declaration.has_value()) return std::move(*declaration);
    auto expression = parse_expression();
    if (expression.has_value()) {
        auto semi = eat_token(token_t::Semicolon);
        if (semi.has_error()) return ast::statement_node_r(ast::error{ semi.error().location });
        return std::move(*expression);
    }

    auto token = next_token();
    return ast::statement_node_r(ast::error{ token->location });
}

ast::expression_node_r parser::parse_expression(std::uint32_t precedence) {
    auto expr = parse_expression_unary(precedence);
    if (expr.has_error()) {
        return ast::expression_node_r(ast::error{ expr.error().location });
    }
    return parse_expression_rhs(std::move(expr.value()), precedence);
}

ast::expression_node_r parser::parse_expression_primary() {
    const auto& tok = peek_token();
    switch (tok->type) {
    case token_t::Identifier: {
        auto tok = next_token();
        if (tok.has_error()) return ast::expression_node_r(ast::error{ tok.error().location });
        return m_arena->allocate_shared<ast::var_read_expression_node>(tok->location, (*tok)->string);
    }
    case token_t::LParen: {
        auto tok  = next_token();
        auto node = parse_expression();
        auto err  = eat_token(token_t::RParen);
        if (err.has_error()) return ast::expression_node_r(ast::error{ err.error().location });
        return node;
    }
    case token_t::String: {
        auto tok = next_token();
        if (tok.has_error()) return ast::expression_node_r(ast::error{ tok.error().location });
        return m_arena->allocate_shared<ast::string_literal_node>(tok->location, (*tok)->string);
    }
    case token_t::Integer: {
        auto tok = next_token();
        if (tok.has_error()) return ast::expression_node_r(ast::error{ tok.error().location });
        return m_arena->allocate_shared<ast::integer_literal_node>(tok->location, (*tok)->integer);
    }
    default: {
        return ast::expression_node_r(ast::error{ tok->location });
    }
    }
}

struct unaryop_info {
    ast::unaryop_expression_node_t type;
    std::uint32_t                  precedence;
};

ast::expression_node_r parser::parse_expression_unary(std::uint32_t precedence) {
    static std::unordered_map<token_t, unaryop_info> s_prefixes = {
        { token_t::Plus, unaryop_info{ ast::unaryop_expression_node_t::Positive, 3 } },
        { token_t::Minus, unaryop_info{ ast::unaryop_expression_node_t::Negative, 3 } },
        { token_t::DPlus, unaryop_info{ ast::unaryop_expression_node_t::PrefixIncrement, 2 } },
        { token_t::DMinus, unaryop_info{ ast::unaryop_expression_node_t::PrefixDecrement, 2 } },
    };

    std::shared_ptr<ast::unary_op_expression_node> result;
    while (true) {
        auto it = s_prefixes.find(peek_token()->type);
        if (it == s_prefixes.end()) break;
        auto current = it->second;
        if (current.precedence >= precedence) break;
        auto token = next_token();

        ast::expression_node_p expression;

        auto nextIt = s_prefixes.find(peek_token()->type);
        if (nextIt != s_prefixes.end()) {
            auto next = nextIt->second;
            auto expr = parse_expression_unary(current.precedence + 1);
            if (expr.has_error()) {
                return ast::expression_node_r(ast::error{ expr.error().location });
            }
            expression = std::move(std::move(expr.value()));
        }

        result = m_arena->allocate_shared<ast::unary_op_expression_node>(token->location,
            current.type,
            std::move(expression));
    }

    if (result == nullptr) return parse_expression_primary();
    if (result->get_node() == nullptr) {
        auto expr = parse_expression_primary();
        if (expr.has_error()) {
            return ast::expression_node_r(ast::error{ expr.error().location });
        }
        result->set_node(std::move(std::move(expr.value())));
    }
    return result;
}

enum class associativity {
    Left,
    Right,
};

enum class rhsop_info_t {
    Unaryop,
    Binop,
    Assignment,
    FuncCall,
};

struct rhsop_info {
    rhsop_info_t  type;
    std::uint32_t precedence;
    associativity associativity;
    union {
        ast::unaryop_expression_node_t unary;
        ast::binop_expression_node_t   binary;
        ast::binop_expression_node_t   assignment;
    };

    bool has_rhs() const { return type == rhsop_info_t::Binop || type == rhsop_info_t::Assignment; }

    static rhsop_info create(ast::unaryop_expression_node_t type, std::uint32_t precedence) {
        rhsop_info info{};
        info.type          = rhsop_info_t::Unaryop;
        info.precedence    = precedence;
        info.associativity = associativity::Left;
        info.unary         = type;
        return info;
    }

    static rhsop_info create(ast::binop_expression_node_t type,
        std::uint32_t                                     precedence,
        enum associativity                                associativity) {
        rhsop_info info{};
        info.type          = rhsop_info_t::Binop;
        info.precedence    = precedence;
        info.associativity = associativity;
        info.binary        = type;
        return info;
    }

    static rhsop_info create(ast::binop_expression_node_t compound = ast::binop_expression_node_t::None) {
        rhsop_info info{};
        info.type          = rhsop_info_t::Assignment;
        info.precedence    = 14;
        info.associativity = associativity::Right;
        info.assignment    = compound;
        return info;
    }

    static rhsop_info create_function_call() {
        rhsop_info info{};
        info.type          = rhsop_info_t::FuncCall;
        info.precedence    = 1;
        info.associativity = associativity::Left;
        return info;
    }
};

ast::expression_node_r parser::parse_expression_rhs(ast::expression_node_p&& init, std::uint32_t precedence) {
    static std::unordered_map<token_t, rhsop_info> s_rhsops = {
        { token_t::Plus, rhsop_info::create(ast::binop_expression_node_t::Add, 5, associativity::Left) },
        { token_t::Minus, rhsop_info::create(ast::binop_expression_node_t::Sub, 5, associativity::Left) },
        { token_t::Star, rhsop_info::create(ast::binop_expression_node_t::Mul, 4, associativity::Left) },
        { token_t::Slash, rhsop_info::create(ast::binop_expression_node_t::Div, 4, associativity::Left) },
        { token_t::Percent, rhsop_info::create(ast::binop_expression_node_t::Mod, 5, associativity::Left) },
        { token_t::DPlus, rhsop_info::create(ast::unaryop_expression_node_t::PostfixIncrement, 1) },
        { token_t::DMinus, rhsop_info::create(ast::unaryop_expression_node_t::PostfixDecrement, 1) },
        { token_t::DMinus, rhsop_info::create(ast::unaryop_expression_node_t::PostfixDecrement, 1) },
        { token_t::Eq, rhsop_info::create() },
        { token_t::PlusEq, rhsop_info::create(ast::binop_expression_node_t::Add) },
        { token_t::MinusEq, rhsop_info::create(ast::binop_expression_node_t::Sub) },
        { token_t::StarEq, rhsop_info::create(ast::binop_expression_node_t::Mul) },
        { token_t::SlashEq, rhsop_info::create(ast::binop_expression_node_t::Div) },
        { token_t::PercentEq, rhsop_info::create(ast::binop_expression_node_t::Mod) },
        { token_t::DEq, rhsop_info::create(ast::binop_expression_node_t::Equal, 10, associativity::Left) },
        { token_t::NotEq, rhsop_info::create(ast::binop_expression_node_t::NotEqual, 10, associativity::Left) },
        { token_t::LessThan, rhsop_info::create(ast::binop_expression_node_t::LessThan, 9, associativity::Left) },
        { token_t::GreaterThan, rhsop_info::create(ast::binop_expression_node_t::GreaterThan, 9, associativity::Left) },
        { token_t::LessEq, rhsop_info::create(ast::binop_expression_node_t::LessEqual, 9, associativity::Left) },
        { token_t::GreaterEq, rhsop_info::create(ast::binop_expression_node_t::GreaterEqual, 9, associativity::Left) },
        { token_t::LParen, rhsop_info::create_function_call() },
    };

    ast::expression_node_p lhs = std::move(init);
    while (peek_token().has_value()) {
        auto it = s_rhsops.find(peek_token()->type);
        if (it == s_rhsops.end()) return lhs;

        rhsop_info current = it->second;
        if (current.precedence >= precedence) return lhs;
        auto opToken = next_token();

        ast::expression_node_p              rhs;
        std::vector<ast::expression_node_p> params;
        if (current.has_rhs()) {
            auto expr = parse_expression_unary(current.precedence + 1); // unary prefix is always right-associative
            if (expr.has_error()) {
                return ast::expression_node_r(ast::error{ expr.error().location });
            }
            rhs = std::move(expr.value());
        } else if (current.type == rhsop_info_t::FuncCall && peek_token().has_value()) {
            if (peek_token()->type != token_t::RParen) {
                while (true) {
                    auto expr = parse_expression_unary(current.precedence + 1);
                    if (expr.has_error()) {
                        return ast::expression_node_r(ast::error{ expr.error().location });
                    }
                    params.emplace_back(std::move(expr.value()));

                    if (eat_token(token_t::Comma).has_error()) break;
                }
            }
            auto enclosing = eat_token(token_t::RParen);
            if (enclosing.has_error()) return ast::expression_node_r(ast::error{ enclosing.error().location });
        }

        auto nextIt = s_rhsops.find(peek_token()->type);
        if (nextIt != s_rhsops.end()) {
            rhsop_info next = nextIt->second;

            auto expr = std::move(parse_expression_rhs(std::move(rhs),
                current.precedence + static_cast<std::uint32_t>(current.associativity == associativity::Right)));
            if (expr.has_error()) {
                return ast::expression_node_r(ast::error{ expr.error().location });
            }
            if (current.type != rhsop_info_t::Unaryop) {
                rhs = std::move(expr.value());
            } else {
                lhs = std::move(expr.value());
            }
        }

        switch (current.type) {
        case rhsop_info_t::Unaryop:
            lhs = m_arena->allocate_shared<ast::unary_op_expression_node>(opToken->location,
                current.unary,
                std::move(lhs));
            break;
        case rhsop_info_t::Binop:
            lhs = m_arena->allocate_shared<ast::binary_op_expression_node>(opToken->location,
                current.binary,
                std::move(lhs),
                std::move(rhs));
            break;
        case rhsop_info_t::Assignment:
            lhs = m_arena->allocate_shared<ast::var_assign_expression_node>(opToken->location,
                current.assignment,
                std::move(lhs),
                std::move(rhs));
            break;
        case rhsop_info_t::FuncCall:
            lhs = m_arena->allocate_shared<ast::function_call_expression_node>(opToken->location,
                std::move(lhs),
                std::move(params));
            break;
        }
    }
    return lhs;
}

ast::body_r parser::parse_body() {
    ast::body body;

    auto begin = eat_token(token_t::LBrace);
    if (begin.has_error()) return ast::body_r(ast::error{ begin.error().location });
    body.begin = begin->location;

    while (!peek_token().has_error() && peek_token()->type != token_t::None && peek_token()->type != token_t::RBrace) {
        body.statements.push_back(parse_statement());
    }

    auto end = eat_token(token_t::RBrace);
    if (end.has_error()) return ast::body_r(ast::error{ end.error().location });
    body.end = end->location;

    return body;
}

token_r parser::next_token() {
    if (!m_peekBuffer.empty()) {
        auto token = std::move(m_peekBuffer.back());
        m_peekBuffer.pop_back();
        return token;
    }
    return m_lexer.next_token();
}

const token_r& parser::peek_token() {
    if (m_peekBuffer.empty()) {
        auto token = m_lexer.next_token();
        return m_peekBuffer.emplace_back(std::move(token));
    }
    return m_peekBuffer.front();
}

token_r parser::eat_token(token_t type) {
    if (const auto& token = peek_token(); token.has_error() || peek_token()->type != type) {
        if (token.has_error()) return token;
        if (token->type == token_t::None)
            return token_r(token_error{ token->location, token_error_t::UnexpectedToken, ", expected " + type });
        return token_r(
            token_error{ token->location, token_error_t::UnexpectedToken, ""s + token->type + ", expected " + type });
    }
    return next_token();
}

} // namespace furc::front
