#ifndef FURC_AST_FWD_HPP
#define FURC_AST_FWD_HPP

#include "furc/diag.hpp"
#include "furlang/result.hpp"

#include <cstdint>
#include <memory>
#include <vector>

namespace furc {

/**
 * @brief Abstract Syntax Tree definitions.
 */
namespace ast {

/**
 * @brief AST error.
 */
struct error {
    location location; /**< Location of the error. */

    /**
     * @brief Compares two AST errors for equality.
     *
     * @param other Error to compare against.
     * @return true if the errors are equal.
     */
    bool operator==(const error& other) const { return location == other.location; }

    /**
     * @brief Compares two AST errors for inequality.
     *
     * @param other Error to compare against.
     * @return true if the errors are not equal.
     */
    bool operator!=(const error& other) const { return !this->operator==(other); }

    /**
     * @brief Prints an AST error to output stream.
     *
     * @param os Output stream.
     * @param error AST error to print.
     * @return The output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const error& error);
};

class node;

/**
 * @brief Alias for a shared pointer to node.
 *
 * @tparam T AST node type.
 */
template <typename T = node>
using node_p = std::shared_ptr<T>;

/**
 * @brief Alias for node result.
 *
 * @tparam T AST node type.
 */
template <typename T = node>
using node_r = furlang::result<node_p<T>, error>;

class expression_node;

using expression_node_p = node_p<expression_node>; /**< Alias for a shared pointer to expression_node. */

using expression_node_r = node_r<expression_node>; /**< Alias for expression_node result */

class declaration_node;

using declaration_node_p = node_p<declaration_node>; /**< Alias for a shared pointer to declaration_node. */

using declaration_node_r = node_r<declaration_node>; /**< Alias for declaration_node result */

class statement_node;

using statement_node_p = node_p<statement_node>; /**< Alias for a shared pointer to statement_node. */

using statement_node_r = node_r<statement_node>; /**< Alias for statement_node result */

class program_node;

using program_node_p = node_p<program_node>; /**< Alias for a shared pointer to program_node. */

using program_node_r = node_r<program_node>; /**< Alias for program_node result */

/**
 * @brief Literal node type.
 */
enum class literal_node_t {
    String,  /**< String literal. */
    Integer, /**< Integer literal. */
};

template <typename, literal_node_t>
class literal_node;

/**
 * @brief String literal AST node.
 */
using string_literal_node = literal_node<std::string, literal_node_t::String>;

using string_literal_node_p = node_p<string_literal_node>; /**< Alias for a shared pointer to string_literal_node */

using string_literal_node_r = node_r<string_literal_node>; /**< Alias for string_literal_node result */

/**
 * @brief Integer literal AST node.
 */
using integer_literal_node = literal_node<std::uint64_t, literal_node_t::Integer>;

using integer_literal_node_p = node_p<integer_literal_node>; /**< Alias for a shared pointer to integer_literal_node */

using integer_literal_node_r = node_r<integer_literal_node>; /**< Alias for integer_literal_node result */

class var_read_expression_node;

using var_read_expression_node_p =
    node_p<var_read_expression_node>; /**< Alias for a shared pointer to var_read_expression_node. */

using var_read_expression_node_r = node_r<var_read_expression_node>; /**< Alias for var_read_expression_node result */

class unaryop_expression_node;

using unaryop_expression_node_p =
    node_p<unaryop_expression_node>; /**< Alias for a shared pointer to unaryop_expression_node. */

using unaryop_expression_node_r = node_r<unaryop_expression_node>; /**< Alias for unaryop_expression_node result */

class binop_expression_node;

using binop_expression_node_p =
    node_p<binop_expression_node>; /**< Alias for a shared pointer to binop_expression_node. */

using binop_expression_node_r = node_r<binop_expression_node>; /**< Alias for binop_expression_node result */

class var_assign_expression_node;

using var_assign_expression_node_p =
    node_p<var_assign_expression_node>; /**< Alias for a shared pointer to var_assign_expression_node. */

using var_assign_expression_node_r =
    node_r<var_assign_expression_node>; /**< Alias for var_assign_expression_node result */

/**
 * @brief List of statements.
 */
struct body {
    /**
     * @brief Location of the opening curly.
     */
    location begin;

    /**
     * @brief Location of the closing curly.
     */
    location end;

    /**
     * @brief List of statements.
     */
    std::vector<statement_node_r> statements;

    /**
     * @brief Compares two bodies for equality.
     *
     * @param rhs Body to compare against.
     * @return true if the bodies are equal.
     */
    bool operator==(const body& rhs) const {
        return begin == rhs.begin && end == rhs.end && statements == rhs.statements;
    }

    /**
     * @brief Compares two bodies for inequality.
     *
     * @param rhs Body to compare against.
     * @return true if the bodies are not equal.
     */
    bool operator!=(const body& rhs) const { return !this->operator==(rhs); }

    /**
     * @brief Prints a body to an output stream.
     *
     * @param os Output stream.
     * @param body Body to print.
     * @return The output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const body& body);
};

/**
 * @brief Alias for body result.
 * @see body
 */
using body_r = furlang::result<body, error>;

class function_declaration_node;

using function_declaration_node_p =
    node_p<function_declaration_node>; /**< Alias for a shared pointer to function_declaration_node. */

using function_declaration_node_r =
    node_r<function_declaration_node>; /**< Alias for function_declaration_node result */

class function_definition_node;

using function_definition_node_p =
    node_p<function_definition_node>; /**< Alias for a shared pointer to function_definition_node. */

using function_definition_node_r = node_r<function_definition_node>; /**< Alias for function_definition_node result */

class return_statement_node;

using return_statement_node_p =
    node_p<return_statement_node>; /**< Alias for a shared pointer to return_statement_node. */

using return_statement_node_r = node_r<return_statement_node>; /**< Alias for return_statement_node result */

class if_statement_node;

using if_statement_node_p = node_p<if_statement_node>; /**< Alias for a shared pointer to if_statement_node. */

using if_statement_node_r = node_r<if_statement_node>; /**< Alias for if_statement_node result */

class compound_statement_node;

using compound_statement_node_p =
    node_p<compound_statement_node>; /**< Alias for a shared pointer to compound_statement_node. */

using compound_statement_node_r = node_r<compound_statement_node>; /**< Alias for compound_statement_node result */

} // namespace ast
} // namespace furc

#endif // FURC_AST_FWD_HPP