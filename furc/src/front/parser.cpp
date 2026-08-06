#include "furc/front/parser.hpp"

#include "furc/front/ast.hpp"
#include "furc/front/token.hpp"

#include <cassert>
#include <stdexcept>
#include <utility>

namespace furc {

ast parser::parse() {
    ast tree;
    while (m_lexer.peek_token().type != token::EndOfFile) {
        auto* decl = parse_decl();
        assert(decl);
        tree.decls.push_back(decl);
    }

    return std::move(tree);
}

stmt_node* parser::parse_stmt() {
    switch (m_lexer.peek_token().type) {
    case token::LBrace: return m_arena->allocate<comp_stmt_node>(parse_comp());
    default: break;
    }

    try {
        return parse_decl();
    } catch (...) {
        return parse_expr();
    }
}

decl_node* parser::parse_decl() {
    auto first = eat_token(token::Identifier, token::Func);
    if (first.type == token::Func) {
        func_decl_node func;

        func.name = std::string(eat_token(token::Identifier).value.string);
        eat_token(token::LParen);
        if (m_lexer.peek_token().type != token::RParen) {
            do {
                var_decl_node param;

                param.name = std::string(eat_token(token::Identifier).value.string);
                eat_token(token::Colon);
                param.type = parse_type();
                if (m_lexer.peek_token().type == token::Equals) {
                    m_lexer.next_token();
                    param.init = parse_expr();
                }
                func.params.emplace_back(std::move(param));
            } while (eat_token(token::Comma, token::RParen).type == token::Comma);
        } else {
            eat_token(token::RParen);
        }

        if (m_lexer.peek_token().type == token::SlimArrow) {
            m_lexer.next_token();
            func.type = parse_type();
        }

        if (m_lexer.peek_token().type == token::Semicolon) {
            m_lexer.next_token();
            return m_arena->allocate<func_decl_node>(std::move(func));
        }

        func.body = parse_comp();

        return m_arena->allocate<func_decl_node>(std::move(func));
    }

    var_decl_node var;
    var.name = std::string(first.value.string);
    eat_token(token::Colon); // TODO: Auto-deduce the type
    var.type = parse_type();
    if (eat_token(token::Equals, token::Semicolon).type == token::Equals) {
        var.init = parse_expr();
        eat_token(token::Semicolon);
    }

    return m_arena->allocate<var_decl_node>(std::move(var));
}

expr_node* parser::parse_expr() {
    if (auto* lit = parse_lit(); lit != nullptr) return lit;
    return nullptr;
}

lit_node* parser::parse_lit() {
    auto token = eat_token(token::Integer, token::Char);
    switch (token.type) {
    case token::Integer: {
        return m_arena->allocate<int_lit_node>(int_lit_node(token.value.integer));
    }
    case token::Char: {
        return m_arena->allocate<char_lit_node>(char_lit_node(token.value.character));
    }
    default: throw std::runtime_error("unreachable");
    }
}

ast_type parser::parse_type() {
    auto token =
        eat_token(token::S8, token::U8, token::S16, token::U16, token::S32, token::U32, token::S64, token::U64);
    switch (token.type) {
    case token::S8: return { ast_type::S8 };
    case token::U8: return { ast_type::U8 };
    case token::S16: return { ast_type::S16 };
    case token::U16: return { ast_type::U16 };
    case token::S32: return { ast_type::S32 };
    case token::U32: return { ast_type::U32 };
    case token::S64: return { ast_type::S64 };
    case token::U64: return { ast_type::U64 };
    default: throw std::runtime_error("unreachable");
    }
}

comp_stmt_node parser::parse_comp() {
    comp_stmt_node comp;

    eat_token(token::LBrace);
    while (m_lexer.peek_token().type != token::EndOfFile && m_lexer.peek_token().type != token::RBrace) {
        comp.stmts.push_back(parse_stmt());
    }
    eat_token(token::RBrace);

    return comp;
}

} // namespace furc
