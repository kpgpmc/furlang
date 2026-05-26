#include "furc/front/parser.hpp"

#include "furc/ast/declaration.hpp"

#include <fstream>
#include <iostream>
#include <string>

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

ast::node_handle<ast::program_node> parser::parse() & {
    auto program = m_arena.allocate_shared<ast::program_node>();

    while (!m_lexer.empty()) {
        program->push(std::move(parse_declaration()));
    }

    return { location{ m_filename, 0, 0 }, program };
}

ast::node_handle<ast::declaration_node> parser::parse_declaration() {
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
                ast::function_body_handle body = parse_body();
                if (body.has_error()) return body;
                return ast::node_handle<ast::function_definition_node>{ first.location(),
                    m_arena,
                    name,
                    std::move(body) };
            }
            case token_t::Semicolon: {
                m_peekBuffer.clear();
                return ast::node_handle<ast::function_declarartion_node>{ first.location(), m_arena, name };
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

ast::node_handle<ast::statement_node> parser::parse_statement() {
    const auto& tok = peek_token();
    if (tok.has_error()) return tok;
    switch (tok->type) {
    case token_t::Keyword: {
        next_token();
        auto err = eat_token(token_t::Semicolon);
        if (err.has_error()) return err;

        return ast::node_handle<ast::return_statement_node>{ tok.location(), m_arena };
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

ast::node_handle<ast::expression_node> parser::parse_expression() {
    const auto& tok = peek_token();

    auto literal = parse_literal();
    if (literal.present()) return std::move(literal);
    return { tok.location(), "unexpected token"s + tok->type + ", expected expression or literal" };
}

ast::node_handle<ast::literal_node> parser::parse_literal() {
    const auto& tok = peek_token();
    switch (tok->type) {
    case token_t::String: {
        auto tok = next_token();
        return ast::node_handle<ast::string_literal_node>{ tok.location(),
            m_arena,
            handle<std::string_view>{ tok.location(), (*tok)->string } };
    }
    case token_t::Integer: {
        auto tok = next_token();
        return ast::node_handle<ast::integer_literal_node>{ tok.location(),
            m_arena,
            handle<integer_token>{ tok.location(), (*tok)->integer } };
    }
    default: break;
    }

    return { tok.location(), "unexpected token "s + tok->type + ", expected literal" };
}

ast::function_body_handle parser::parse_body() {
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