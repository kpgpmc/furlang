#ifndef FURC_FRONT_PARSER_HPP
#define FURC_FRONT_PARSER_HPP

#include "furc/ast/fwd.hpp"
#include "furc/front/lexer.hpp"
#include "furlang/arena.hpp"

#include <vector>

namespace furc {
namespace front {

/**
 * @brief Parser.
 *
 * Furlang's parser.
 */
class parser final {
public:
    /**
     * @brief Construct a new parser from content.
     *
     * @param filename Filename for debugging.
     * @param content Content.
     */
    parser(std::string_view filename, std::string_view content);

    /**
     * @brief Construct a new parser from file.
     *
     * Constructs a lexer with content read from file passed through \p filename.
     *
     * @param filename Name of the file.
     */
    parser(std::string_view filename);
    ~parser() = default;

    /**
     * @brief Move constructor.
     */
    parser(parser&&) = default;

    parser(const parser&) = delete;

    /**
     * @brief Move constructor.
     */
    parser& operator=(parser&&) = default;

    parser& operator=(const parser&) = delete;
public:
    /**
     * @brief Returns a parsed program.
     *
     * @return Handle to an AST node of the program.
     */
    ast::program_node_h parse() &;
private:
    ast::declaration_node_h parse_declaration();
    ast::statement_node_h   parse_statement();
    ast::expression_node_h  parse_expression(std::uint32_t precedence = 16);
    ast::literal_node_h     parse_literal();

    ast::expression_node_h parse_expression_primary();
    ast::expression_node_h parse_expression_unary(std::uint32_t precedence);
    ast::expression_node_h parse_expression_rhs(ast::expression_node_h&& init, std::uint32_t precedence);

    ast::body_h parse_body();
private:
    static ast::node_handle<ast::node> error_handle(const token_r& result);
private:
    token_r        next_token();
    const token_r& peek_token();
    token_r        eat_token(token_t type);
private:
    std::string          m_filename;
    std::string          m_content;
    lexer                m_lexer;
    furlang::arena       m_arena;
    std::vector<token_r> m_peekBuffer;
};

} // namespace front
} // namespace furc

#endif // FURC_FRONT_PARSER_HPP