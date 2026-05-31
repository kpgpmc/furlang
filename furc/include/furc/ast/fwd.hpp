#ifndef FURC_AST_FWD_HPP
#define FURC_AST_FWD_HPP

#include "furc/handle.hpp"

#include <string>
#include <vector>

namespace furc {
namespace ast {

class node;

template <typename T>
using node_handle = handle<T*, std::string>;

class literal_node;
using literal_node_h = node_handle<literal_node>;
class expression_node;
using expression_node_h = node_handle<expression_node>;
class declaration_node;
using declaration_node_h = node_handle<declaration_node>;
class statement_node;
using statement_node_h = node_handle<statement_node>;
class program_node;
using program_node_h = node_handle<program_node>;

class string_literal_node;
using string_literal_node_h = node_handle<string_literal_node>;
class integer_literal_node;
using integer_literal_node_h = node_handle<integer_literal_node>;

class var_read_expression_node;
using var_read_expression_node_h = node_handle<var_read_expression_node>;
class unaryop_expression_node;
using unaryop_expression_node_h = node_handle<unaryop_expression_node>;
class binop_expression_node;
using binop_expression_node_h = node_handle<binop_expression_node>;
class var_assign_expression_node;
using var_assign_expression_node_h = node_handle<var_assign_expression_node>;

struct body {
    location                      begin, end;
    std::vector<statement_node_h> statements;

    bool operator==(const body& rhs) const {
        return begin == rhs.begin && end == rhs.end && statements == rhs.statements;
    }

    bool operator!=(const body& rhs) const { return !this->operator==(rhs); }

    friend std::ostream& operator<<(std::ostream&, const body&);
};

using body_h = handle<body>;

class function_declaration_node;
using function_declaration_node_h = node_handle<function_declaration_node>;
class function_definition_node;
using function_definition_node_h = node_handle<function_definition_node>;

class return_statement_node;
using return_statement_node_h = node_handle<return_statement_node>;
class if_statement_node;
using if_statement_node_h = node_handle<if_statement_node>;

} // namespace ast
} // namespace furc

#endif // FURC_AST_FWD_HPP