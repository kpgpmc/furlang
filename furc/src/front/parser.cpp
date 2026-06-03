#include "furc/front/parser.hpp"

#include "furc/ast/declaration.hpp" // IWYU pragma: keep
#include "furc/ast/expression.hpp"  // IWYU pragma: keep
#include "furc/ast/literal.hpp"     // IWYU pragma: keep
#include "furc/ast/program.hpp"     // IWYU pragma: keep
#include "furc/ast/statement.hpp"   // IWYU pragma: keep

#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

namespace furc::front {

using namespace std::string_literals;

parser::parser(std::string_view filename, std::string_view content)
  : m_filename(filename), m_content(content), m_lexer(m_filename, m_content) {}

parser::parser(std::string_view filename)
  : m_filename(filename) {
    std::ifstream file(m_filename, std::ios_base::binary | std::ios_base::ate);
    if (!file.is_open()) throw std::runtime_error("failed to open file "s.append(m_filename));
    std::streampos size = file.tellg();
    file.seekg(0);

    m_content.resize(size);
    file.read(m_content.data(), size);
    m_lexer = { filename, m_content };
}

ast::program_node_h parser::parse() & {
    auto program = m_arena.allocate_shared<ast::program_node>();

    while (peek_token().has_value()) {
        program->push(std::move(parse_declaration()));
    }

    return { location{ m_filename, 0, 0 }, program };
}

ast::declaration_node_h parser::parse_declaration() {
    const auto& first = peek_token();
    switch (first->type) {
    case token_t::Keyword: {
        auto first = next_token();
        switch ((*first)->keyword) {
        case keyword_token::Func: {
            auto name = eat_token(token_t::Identifier);
            if (name.has_error()) return error_handle(name);

            auto tok = eat_token(token_t::LParen);
            if (tok.has_error()) return error_handle(tok);
            tok = eat_token(token_t::RParen);
            if (tok.has_error()) return error_handle(tok);

            const auto& peek = peek_token();
            if (peek.has_error()) return error_handle(peek);
            switch (peek->type) {
            case token_t::LBrace: {
                ast::body_r body = parse_body();
                if (body.has_error()) return ast::node_handle<ast::node>(body.error().location, "Body error");
                return ast::function_definition_node_h{ first->location, m_arena, *name, std::move(body) };
            }
            case token_t::Semicolon: {
                m_peekBuffer.clear();
                return ast::function_declaration_node_h{ first->location, m_arena, *name };
            }
            default: return { tok->location, "unexpected token "s + tok->type };
            }
        }
        default: return { first->location, "unexpected keyword "s + (*first)->keyword };
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
        return { first->location, "unexpected token "s + first->type };
    }
    }
}

ast::statement_node_h parser::parse_statement() {
    const auto& tok = peek_token();
    if (tok.has_error()) return error_handle(tok);
    auto location = tok->location;
    switch (tok->type) {
    case token_t::Keyword: {
        switch (tok->value.keyword) {
        case keyword_token::Return: {
            auto tok = next_token();
            if (peek_token()->type == token_t::Semicolon) {
                next_token();
                return ast::return_statement_node_h{ tok->location, m_arena };
            }

            auto value = parse_expression();
            auto err   = eat_token(token_t::Semicolon);
            if (err.has_error()) return error_handle(err);
            return ast::return_statement_node_h{ tok->location, m_arena, std::move(value) };
        }
        case keyword_token::If: {
            auto tok = next_token();
            auto err = eat_token(token_t::LParen);
            if (err.has_error()) return error_handle(err);

            auto cond = parse_expression();

            err = eat_token(token_t::RParen);
            if (err.has_error()) return error_handle(err);

            auto then = parse_statement();
            if (then.has_error()) return then;

            if (peek_token().has_value() && peek_token()->type == token_t::Keyword &&
                peek_token()->value.keyword == keyword_token::Else) {
                next_token();

                return ast::if_statement_node_h{ location,
                    m_arena,
                    std::move(cond),
                    std::move(then),
                    std::move(parse_statement()) };
            }

            return ast::if_statement_node_h{ location, m_arena, std::move(cond), std::move(then) };
        }
        case keyword_token::None:
        case keyword_token::Func:
        default: break;
        }
    }
    case token_t::LBrace: return ast::compound_statement_node_h{ location, m_arena, parse_body() };
    default: break;
    }

    auto declaration = parse_declaration();
    if (declaration.present()) return std::move(declaration);
    auto expression = parse_expression();
    if (expression.present()) {
        auto semi = eat_token(token_t::Semicolon);
        if (semi.has_error()) return error_handle(semi);
        return std::move(expression);
    }

    auto token = next_token();
    return { token->location, "unexpected token "s + token->type + ", expected statement, declaration or expression" };
}

ast::expression_node_h parser::parse_expression(std::uint32_t precedence) {
    return parse_expression_rhs(parse_expression_unary(precedence), precedence);
}

ast::literal_node_h parser::parse_literal() {
    const auto& tok = peek_token();
    switch (tok->type) {
    case token_t::String: {
        auto tok = next_token();
        return ast::string_literal_node_h{ tok->location,
            m_arena,
            handle<std::string_view>{ tok->location, (*tok)->string } };
    }
    case token_t::Integer: {
        auto tok = next_token();
        return ast::integer_literal_node_h{ tok->location,
            m_arena,
            handle<integer_token>{ tok->location, (*tok)->integer } };
    }
    default: break;
    }

    return { tok->location, "unexpected token "s + tok->type + ", expected literal" };
}

ast::expression_node_h parser::parse_expression_primary() {
    const auto& tok = peek_token();
    switch (tok->type) {
    case token_t::Identifier: {
        auto tok = next_token();
        return ast::var_read_expression_node_h{ tok->location,
            m_arena,
            handle<std::string_view>{ tok->location, (*tok)->string } };
    }
    case token_t::LParen: {
        auto tok  = next_token();
        auto node = parse_expression();
        auto err  = eat_token(token_t::RParen);
        if (err.has_error()) return error_handle(err);
        return node;
    }
    default: {
        auto literal = parse_literal();
        if (literal.present()) return std::move(literal);
        return { tok->location, "unexpected token"s + tok->type + ", expected expression or literal" };
    }
    }
}

struct unaryop_info {
    ast::unaryop_expression_node_t type;
    std::uint32_t                  precedence;
};

ast::expression_node_h parser::parse_expression_unary(std::uint32_t precedence) {
    static std::unordered_map<token_t, unaryop_info> s_prefixes = {
        { token_t::Plus, unaryop_info{ ast::unaryop_expression_node_t::Positive, 3 } },
        { token_t::Minus, unaryop_info{ ast::unaryop_expression_node_t::Negative, 3 } },
        { token_t::DPlus, unaryop_info{ ast::unaryop_expression_node_t::PrefixIncrement, 2 } },
        { token_t::DMinus, unaryop_info{ ast::unaryop_expression_node_t::PrefixDecrement, 2 } },
    };

    ast::unaryop_expression_node_h result;
    while (true) {
        auto it = s_prefixes.find(peek_token()->type);
        if (it == s_prefixes.end()) break;
        auto current = it->second;
        if (current.precedence >= precedence) break;
        auto token = next_token();

        ast::expression_node_h expression;

        auto nextIt = s_prefixes.find(peek_token()->type);
        if (nextIt != s_prefixes.end()) {
            auto next  = nextIt->second;
            expression = parse_expression_unary(current.precedence + 1);
        }

        result = { token->location, m_arena, current.type, std::move(expression) };
    }

    if (!result.present()) return parse_expression_primary();
    if (!result->get_node().present()) result->set_node(parse_expression_primary());
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
};

ast::expression_node_h parser::parse_expression_rhs(ast::expression_node_h&& init, std::uint32_t precedence) {
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
    };

    ast::expression_node_h lhs = std::move(init);
    while (peek_token().has_value()) {
        auto it = s_rhsops.find(peek_token()->type);
        if (it == s_rhsops.end()) return lhs;

        rhsop_info current = it->second;
        if (current.precedence >= precedence) return lhs;
        auto opToken = next_token();

        ast::expression_node_h rhs;
        if (current.type != rhsop_info_t::Unaryop) {
            rhs = parse_expression_unary(current.precedence + 1); // unary prefix is always right-associative
        }

        auto nextIt = s_rhsops.find(peek_token()->type);
        if (nextIt != s_rhsops.end()) {
            rhsop_info next = nextIt->second;

            if (current.type != rhsop_info_t::Unaryop) {
                rhs = std::move(parse_expression_rhs(std::move(rhs),
                    current.precedence + static_cast<std::uint32_t>(current.associativity == associativity::Right)));
            } else {
                lhs = std::move(parse_expression_rhs(std::move(lhs), current.precedence));
            }
        }

        switch (current.type) {
        case rhsop_info_t::Unaryop:
            lhs = ast::unaryop_expression_node_h{ opToken->location, m_arena, current.unary, std::move(lhs) };
            break;
        case rhsop_info_t::Binop:
            lhs = ast::binop_expression_node_h{ opToken->location,
                m_arena,
                current.binary,
                std::move(lhs),
                std::move(rhs) };
            break;
        case rhsop_info_t::Assignment:
            lhs = ast::var_assign_expression_node_h{ opToken->location,
                m_arena,
                current.assignment,
                std::move(lhs),
                std::move(rhs) };
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

ast::node_handle<ast::node> parser::error_handle(const token_r& result) {
    return { result.error().location, std::string(result.error().message) };
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
    auto token = next_token();
    if (token.has_error()) return token;
    if (token->type != type) {
        if (token->type == token_t::None)
            return token_r(token_error{ token->location, token_error_t::UnexpectedToken, ", expected " + type });
        return token_r(
            token_error{ token->location, token_error_t::UnexpectedToken, ""s + token->type + ", expected " + type });
    }
    return token;
}

} // namespace furc::front