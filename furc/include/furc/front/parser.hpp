#ifndef FURC_FRONT_PARSER_HPP
#define FURC_FRONT_PARSER_HPP

#include "furc/ast/declaration.hpp"
#include "furc/ast/expression.hpp"
#include "furc/ast/literal.hpp"
#include "furc/ast/program.hpp"
#include "furc/front/lexer.hpp"
#include "furlang/arena.hpp"

#include <vector>

namespace furc {
namespace front {

class parser {
public:
    parser(std::string_view filename, std::string_view content);
    parser(std::string_view filename);
    ~parser() = default;

    parser(parser&&)                 = default;
    parser(const parser&)            = delete;
    parser& operator=(parser&&)      = default;
    parser& operator=(const parser&) = delete;
public:
    // parser owns the arena :3c
    ast::program_node_h parse() &;
private:
    ast::declaration_node_h parse_declaration();
    ast::statement_node_h   parse_statement();
    ast::expression_node_h  parse_expression();
    ast::literal_node_h     parse_literal();

    ast::expression_node_h parse_expression_primary();
    ast::expression_node_h parse_expression_rhs(const ast::expression_node_h& init, std::uint32_t precedence);

    ast::function_body_h parse_body();
private:
    token_handle<>        next_token();
    const token_handle<>& peek_token();
    token_handle<>        eat_token(token_t type);
private:
    std::string                 m_filename;
    std::string                 m_content;
    lexer                       m_lexer;
    furlang::arena              m_arena;
    std::vector<token_handle<>> m_peekBuffer;
};

} // namespace front
} // namespace furc

#endif // FURC_FRONT_PARSER_HPP