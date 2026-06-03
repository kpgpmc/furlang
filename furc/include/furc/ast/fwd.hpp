#ifndef FURC_AST_FWD_HPP
#define FURC_AST_FWD_HPP

#include "furc/diag.hpp"
#include "furc/handle.hpp"
#include "furlang/result.hpp"

#include <string>
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
};

class node;

/**
 * @brief Alias for handle to node.
 *
 * @tparam T AST node type.
 */
template <typename T>
using node_handle = handle<T*, std::string>;

class literal_node;

/**
 * @brief Alias for handle to literal_node.
 * @see literal_node
 */
using literal_node_h = node_handle<literal_node>;

class expression_node;

/**
 * @brief Alias for handle to expression_node.
 * @see expression_node
 */
using expression_node_h = node_handle<expression_node>;

class declaration_node;

/**
 * @brief Alias for handle to declaration_node.
 * @see declaration_node
 */
using declaration_node_h = node_handle<declaration_node>;

class statement_node;

/**
 * @brief Alias for handle to statement_node.
 * @see statement_node
 */
using statement_node_h = node_handle<statement_node>;

class program_node;

/**
 * @brief Alias for handle to program_node.
 * @see program_node
 */
using program_node_h = node_handle<program_node>;

class string_literal_node;

/**
 * @brief Alias for handle to string_literal_node.
 * @see string_literal_node
 */
using string_literal_node_h = node_handle<string_literal_node>;

class integer_literal_node;

/**
 * @brief Alias for handle to integer_literal_node.
 * @see integer_literal_node
 */
using integer_literal_node_h = node_handle<integer_literal_node>;

class var_read_expression_node;

/**
 * @brief Alias for handle to var_read_expression_node.
 * @see var_read_expression_node
 */
using var_read_expression_node_h = node_handle<var_read_expression_node>;

class unaryop_expression_node;

/**
 * @brief Alias for handle to unaryop_expression_node.
 * @see unaryop_expression_node
 */
using unaryop_expression_node_h = node_handle<unaryop_expression_node>;

class binop_expression_node;

/**
 * @brief Alias for handle to binop_expression_node.
 * @see binop_expression_node
 */
using binop_expression_node_h = node_handle<binop_expression_node>;

class var_assign_expression_node;

/**
 * @brief Alias for handle to var_assign_expression_node.
 * @see var_assign_expression_node
 */
using var_assign_expression_node_h = node_handle<var_assign_expression_node>;

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
    std::vector<statement_node_h> statements;

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

/**
 * @brief Alias for handle to function_declaration_node.
 * @see function_declaration_node
 */
using function_declaration_node_h = node_handle<function_declaration_node>;

class function_definition_node;

/**
 * @brief Alias for handle to function_definition_node.
 * @see function_definition_node
 */
using function_definition_node_h = node_handle<function_definition_node>;

class return_statement_node;

/**
 * @brief Alias for handle to return_statement_node.
 * @see return_statement_node
 */
using return_statement_node_h = node_handle<return_statement_node>;

class if_statement_node;

/**
 * @brief Alias for handle to if_statement_node.
 * @see if_statement_node
 */
using if_statement_node_h = node_handle<if_statement_node>;

class compound_statement_node;

/**
 * @brief Alias for handle to compound_statement_node.
 * @see compound_statement_node
 */
using compound_statement_node_h = node_handle<compound_statement_node>;

} // namespace ast
} // namespace furc

#endif // FURC_AST_FWD_HPP