#include "furc/front/parser.hpp"

#include "furc/front/ast.hpp"
#include "furc/front/token.hpp"

#include <cassert>
#include <stdexcept>
#include <unordered_map>
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
    return parse_expr_right(parse_expr_unary());
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

expr_node* parser::parse_expr_primary() {
    return parse_lit();
}

expr_node* parser::parse_expr_unary() {
    static std::unordered_map<token_t, unary_op_expr_node::unary_op_type> s_prefixOps = {
        { token::Plus, unary_op_expr_node::Positive },
        { token::Minus, unary_op_expr_node::Negative },
        { token::DblPlus, unary_op_expr_node::PreInc },
        { token::DblMinus, unary_op_expr_node::PreDec },
        { token::Tilde, unary_op_expr_node::BinNot },
        { token::ExMark, unary_op_expr_node::Not },

        { token::Sizeof, unary_op_expr_node::Sizeof },
        { token::Pointerof, unary_op_expr_node::Pointerof },
        { token::Lengthof, unary_op_expr_node::Lengthof },
    };

    static std::unordered_map<token_t, unary_op_expr_node::unary_op_type> s_postfixOps = {
        { token::DblPlus, unary_op_expr_node::PostInc },
        { token::DblMinus, unary_op_expr_node::PostDec },
    };

    auto it = s_prefixOps.find(m_lexer.peek_token().type);
    if (it == s_prefixOps.end()) {
        auto* expr = parse_expr_primary();
        while (true) {
            auto postIt = s_postfixOps.find(m_lexer.peek_token().type);
            if (postIt == s_postfixOps.end()) return expr;
            m_lexer.next_token();

            unary_op_expr_node unary;
            unary.lhs  = expr;
            unary.type = postIt->second;
            expr       = m_arena->allocate<unary_op_expr_node>(std::move(unary));
        }
    }

    auto token = m_lexer.next_token();

    unary_op_expr_node unary;
    unary.lhs  = parse_expr_unary();
    unary.type = it->second;
    return m_arena->allocate<unary_op_expr_node>(std::move(unary));
}

expr_node* parser::parse_expr_right(expr_node* lhs, std::uint32_t precedence) {
    struct op_info {
        binary_op_expr_node::binary_op_type type;
        std::uint32_t                       precedence;
        bool                                right = false;
    };

    static std::unordered_map<token_t, op_info> s_ops = {
        { token::Plus, { binary_op_expr_node::Add, 4 } },
        { token::Minus, { binary_op_expr_node::Sub, 4 } },
        { token::Star, { binary_op_expr_node::Mul, 3 } },
        { token::Slash, { binary_op_expr_node::Div, 3 } },
        { token::Percent, { binary_op_expr_node::Mod, 3 } },
        { token::DblLT, { binary_op_expr_node::Shl, 5 } },
        { token::DblGT, { binary_op_expr_node::Shr, 5 } },
        { token::Ampersand, { binary_op_expr_node::BinAnd, 8 } },
        { token::Pipe, { binary_op_expr_node::BinOr, 10 } },
        { token::Hat, { binary_op_expr_node::BinXor, 9 } },
        { token::DblAmpersand, { binary_op_expr_node::And, 11 } },
        { token::DblPipe, { binary_op_expr_node::Or, 12 } },
        { token::DblEquals, { binary_op_expr_node::Equals, 7 } },
        { token::ExEquals, { binary_op_expr_node::NotEquals, 7 } },
        { token::LessThan, { binary_op_expr_node::LessThan, 6 } },
        { token::LessEquals, { binary_op_expr_node::LessEquals, 6 } },
        { token::GreaterThan, { binary_op_expr_node::GreaterThan, 6 } },
        { token::GreaterEquals, { binary_op_expr_node::GreaterEquals, 6 } },
    };

    while (true) {
        auto it = s_ops.find(m_lexer.peek_token().type);
        if (it == s_ops.end() || it->second.precedence >= precedence) return lhs;
        auto op = it->second;
        m_lexer.next_token();

        expr_node* rhs    = parse_expr_unary();
        auto       nextIt = s_ops.find(m_lexer.peek_token().type);
        if (nextIt != s_ops.end()) {
            rhs = parse_expr_right(rhs, op.precedence + (op.right ? 1 : 0));
        }

        binary_op_expr_node binary;
        binary.lhs  = lhs;
        binary.rhs  = rhs;
        binary.type = op.type;
        lhs         = m_arena->allocate<binary_op_expr_node>(std::move(binary));
    }
}

} // namespace furc
