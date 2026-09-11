#ifndef FURC_FRONT_PARSER_HPP
#define FURC_FRONT_PARSER_HPP

#include "furc/front/ast.hpp"
#include "furc/front/lexer.hpp"
#include "furlang/arena.hpp"

#include <stdexcept>

namespace furc {

class parser {
public:
    parser(lexer&& lexer, furlang::arena& arena)
      : m_lexer(std::move(lexer)), m_arena(&arena) {}

    ~parser() = default;

    parser(parser&&) noexcept            = default;
    parser& operator=(parser&&) noexcept = default;

    parser(const parser&)            = delete;
    parser& operator=(const parser&) = delete;
public:
    ast parse();
private:
    stmt_node* parse_stmt();
    decl_node* parse_decl();
    expr_node* parse_expr();

    ast_type       parse_type();
    comp_stmt_node parse_comp();

    expr_node* parse_expr_primary();
    expr_node* parse_expr_unary();
    expr_node* parse_expr_right(expr_node* lhs, std::uint32_t precedence = 15);
private:
    template <typename... Types>
    token eat_token(Types... types) {
        auto token = m_lexer.next_token();
        if (((token.type == types) || ...)) return token;
        throw std::runtime_error("unexpected token");
    }
private:
    lexer           m_lexer;
    furlang::arena* m_arena;
};

} // namespace furc

#endif // FURC_FRONT_PARSER_HPP
