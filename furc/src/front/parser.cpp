#include "furc/front/parser.hpp"

#include "furc/ast/declaration.hpp"

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

    while (peek_token().present() && peek_token()->type != token_t::None && !m_lexer.empty()) {
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
            token_handle<> name = eat_token(token_t::Identifier);
            if (name.has_error()) return { name.location(), name.error() };

            auto tok = eat_token(token_t::Lparen);
            if (tok.has_error()) return tok;
            tok = eat_token(token_t::Rparen);
            if (tok.has_error()) return tok;

            const auto& peek = peek_token();
            if (peek.has_error()) return peek;
            switch (peek->type) {
            case token_t::Lbrace: {
                ast::function_body_h body = parse_body();
                if (body.has_error()) return body;
                return ast::function_definition_node_h{ first.location(), m_arena, *name, std::move(body) };
            }
            case token_t::Semicolon: {
                m_peekBuffer.clear();
                return ast::function_declaration_node_h{ first.location(), m_arena, *name };
            }
            default: return { tok.location(), "unexpected token "s + tok->type };
            }
        }
        default: return { first.location(), "unexpected keyword "s + (*first)->keyword };
        }
    }
    case token_t::None:
    case token_t::Identifier:
    case token_t::Integer:
    case token_t::Lparen:
    case token_t::Rparen:
    case token_t::Lbrace:
    case token_t::Rbrace:
    case token_t::Lbracket:
    case token_t::Rbracket:
    case token_t::Semicolon:
    case token_t::Colon:
    default: {
        return { first.location(), "unexpected token "s + first->type };
    }
    }
}

ast::statement_node_h parser::parse_statement() {
    const auto& tok = peek_token();
    if (tok.has_error()) return tok;
    switch (tok->type) {
    case token_t::Keyword: {
        auto tok = next_token();
        if (peek_token()->type == token_t::Semicolon) {
            next_token();
            return ast::return_statement_node_h{ tok.location(), m_arena };
        }

        auto value = parse_expression();
        auto err   = eat_token(token_t::Semicolon);
        if (err.has_error()) return err;
        return ast::return_statement_node_h{ tok.location(), m_arena, std::move(value) };
    }
    default: break;
    }

    auto declaration = parse_declaration();
    if (declaration.present()) return std::move(declaration);
    auto expression = parse_expression();
    if (expression.present()) {
        auto semi = eat_token(token_t::Semicolon);
        if (semi.has_error()) return semi;
        return std::move(expression);
    }

    auto token = next_token();
    return { token.location(), "unexpected token "s + token->type + ", expected statement, declaration or expression" };
}

ast::expression_node_h parser::parse_expression() {
    return parse_expression_rhs(parse_expression_unary(16), 16);
}

ast::literal_node_h parser::parse_literal() {
    const auto& tok = peek_token();
    switch (tok->type) {
    case token_t::String: {
        auto tok = next_token();
        return ast::string_literal_node_h{ tok.location(),
            m_arena,
            handle<std::string_view>{ tok.location(), (*tok)->string } };
    }
    case token_t::Integer: {
        auto tok = next_token();
        return ast::integer_literal_node_h{ tok.location(),
            m_arena,
            handle<integer_token>{ tok.location(), (*tok)->integer } };
    }
    default: break;
    }

    return { tok.location(), "unexpected token "s + tok->type + ", expected literal" };
}

ast::expression_node_h parser::parse_expression_primary() {
    const auto& tok = peek_token();

    auto literal = parse_literal();
    if (literal.present()) return std::move(literal);
    return { tok.location(), "unexpected token"s + tok->type + ", expected expression or literal" };
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

        result = { token.location(), m_arena, current.type, std::move(expression) };
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
};

struct rhsop_info {
    rhsop_info_t  type;
    std::uint32_t precedence;
    associativity associativity;
    union {
        ast::unaryop_expression_node_t unary;
        ast::binop_expression_node_t   binary;
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
    };

    ast::expression_node_h lhs = std::move(init);
    while (peek_token().present()) {
        auto it = s_rhsops.find(peek_token()->type);
        if (it == s_rhsops.end()) return lhs;

        rhsop_info current = it->second;
        if (current.precedence >= precedence) return lhs;
        auto opToken = next_token();

        ast::expression_node_h rhs;
        if (current.type == rhsop_info_t::Binop) {
            rhs = parse_expression_unary(current.precedence + 1); // unary prefix is always right-associative
        }

        auto nextIt = s_rhsops.find(peek_token()->type);
        if (nextIt != s_rhsops.end()) {
            rhsop_info next = nextIt->second;

            if (current.type == rhsop_info_t::Binop) {
                rhs = std::move(parse_expression_rhs(std::move(rhs),
                    current.precedence + static_cast<std::uint32_t>(current.associativity == associativity::Right)));
            } else {
                lhs = std::move(parse_expression_rhs(std::move(lhs), current.precedence));
            }
        }

        lhs = current.type == rhsop_info_t::Binop
                  ? ast::expression_node_h(ast::binop_expression_node_h{ opToken.location(),
                        m_arena,
                        current.binary,
                        std::move(lhs),
                        std::move(rhs) })
                  : ast::expression_node_h(
                        ast::unaryop_expression_node_h{ opToken.location(), m_arena, current.unary, std::move(lhs) });
    }
    return lhs;
}

ast::function_body_h parser::parse_body() {
    ast::function_body body;

    token_handle<> begin = eat_token(token_t::Lbrace);
    if (begin.has_error()) return begin;
    body.begin = begin.location();

    while (!peek_token().has_error() && peek_token()->type != token_t::None && peek_token()->type != token_t::Rbrace) {
        body.statements.push_back(parse_statement());
    }

    token_handle<> end = eat_token(token_t::Rbrace);
    if (end.has_error()) return end;
    body.end = end.location();

    return { begin.location(), body };
}

token_handle<> parser::next_token() {
    if (!m_peekBuffer.empty()) {
        token_handle<> token = std::move(m_peekBuffer.back());
        m_peekBuffer.pop_back();
        return token;
    }
    return m_lexer.next_token();
}

const token_handle<>& parser::peek_token() {
    if (m_peekBuffer.empty()) {
        token_handle<> token = m_lexer.next_token();
        return m_peekBuffer.emplace_back(std::move(token));
    }
    return m_peekBuffer.front();
}

token_handle<> parser::eat_token(token_t type) {
    token_handle<> token = next_token();
    if (!token.present()) return token;
    if (token->type != type) {
        if (token->type == token_t::None) return { token.location(), "unexpected end of file, expected " + type };
        return { token.location(), "unexpected token "s + token->type + ", expected " + type };
    }
    return token;
}

} // namespace furc::front