#include "furc/ast/declaration.hpp"
#include "furc/ast/literal.hpp"
#include "furc/ast/node.hpp"
#include "furc/ast/program.hpp"
#include "furc/ast/statement.hpp"

#include <ostream>

namespace furc::ast {

bool literal_node::equal(const node& rhs) const {
    return literal_type() == reinterpret_cast<const literal_node&>(rhs).literal_type();
}

void string_literal_node::accept(visitor& visitor) const {
    visitor.visit(*this);
}

std::ostream& string_literal_node::print(std::ostream& os) const {
    if (m_value.has_error()) return os << m_value.error();
    return os << '"' << *m_value << '"';
}

bool string_literal_node::equal(const node& rhs) const {
    return literal_node::equal(rhs) && m_value == reinterpret_cast<const string_literal_node&>(rhs).m_value;
}

void integer_literal_node::accept(visitor& visitor) const {
    visitor.visit(*this);
}

std::ostream& integer_literal_node::print(std::ostream& os) const {
    if (m_value.has_error()) return os << m_value.error();
    return os << *m_value;
}

bool integer_literal_node::equal(const node& rhs) const {
    return literal_node::equal(rhs) && m_value == reinterpret_cast<const integer_literal_node&>(rhs).m_value;
}

bool expression_node::equal(const node& rhs) const {
    return expression_type() == reinterpret_cast<const expression_node&>(rhs).expression_type();
}

void var_read_expression_node::accept(visitor& visitor) const {
    visitor.visit(*this);
}

std::ostream& var_read_expression_node::print(std::ostream& os) const {
    if (m_name.present()) return os << *m_name;
    return os << m_name.error();
}

bool var_read_expression_node::equal(const node& rhsNode) const {
    const auto& rhs = reinterpret_cast<const var_read_expression_node&>(rhsNode);
    return expression_node::equal(rhsNode) && m_name == rhs.m_name;
}

std::ostream& operator<<(std::ostream& os, unaryop_expression_node_t type) {
    switch (type) {
    case unaryop_expression_node_t::Positive: return os << "+";
    case unaryop_expression_node_t::Negative: return os << "-";
    case unaryop_expression_node_t::PrefixIncrement:
    case unaryop_expression_node_t::PostfixIncrement: return os << "++";
    case unaryop_expression_node_t::PrefixDecrement:
    case unaryop_expression_node_t::PostfixDecrement: return os << "--";
    }
    return os;
}

void unaryop_expression_node::accept(visitor& visitor) const {
    visitor.visit(*this);
}

std::ostream& unaryop_expression_node::print(std::ostream& os) const {
    switch (m_type) {
    case unaryop_expression_node_t::Positive:
    case unaryop_expression_node_t::Negative:
    case unaryop_expression_node_t::PrefixIncrement:
    case unaryop_expression_node_t::PrefixDecrement: return os << '(' << m_type << *m_node << ')';
    case unaryop_expression_node_t::PostfixIncrement:
    case unaryop_expression_node_t::PostfixDecrement: return os << '(' << *m_node << m_type << ')';
    }
    return os;
}

bool unaryop_expression_node::equal(const node& rhsNode) const {
    const auto& rhs = reinterpret_cast<const unaryop_expression_node&>(rhsNode);
    return expression_node::equal(rhsNode) && m_type == rhs.m_type && m_node == rhs.m_node;
}

std::ostream& operator<<(std::ostream& os, binop_expression_node_t type) {
    switch (type) {
    default:
    case binop_expression_node_t::None: return os;
    case binop_expression_node_t::Add: return os << '+';
    case binop_expression_node_t::Sub: return os << '-';
    case binop_expression_node_t::Mul: return os << '*';
    case binop_expression_node_t::Div: return os << '/';
    case binop_expression_node_t::Mod: return os << '%';
    case binop_expression_node_t::Equal: return os << "==";
    case binop_expression_node_t::NotEqual: return os << "!=";
    case binop_expression_node_t::LessThan: return os << '<';
    case binop_expression_node_t::GreaterThan: return os << '>';
    case binop_expression_node_t::LessEqual: return os << "<=";
    case binop_expression_node_t::GreaterEqual: return os << ">=";
    }
}

void binop_expression_node::accept(visitor& visitor) const {
    visitor.visit(*this);
}

std::ostream& binop_expression_node::print(std::ostream& os) const {
    if (m_type == binop_expression_node_t::None) return os;
    return os << '(' << *m_lhs << ' ' << m_type << ' ' << *m_rhs << ')';
}

bool binop_expression_node::equal(const node& rhsNode) const {
    const auto& rhs = reinterpret_cast<const binop_expression_node&>(rhsNode);
    return expression_node::equal(rhsNode) && m_type == rhs.m_type && m_lhs == rhs.m_lhs && m_rhs == rhs.m_rhs;
}

void var_assign_expression_node::accept(visitor& visitor) const {
    visitor.visit(*this);
}

std::ostream& var_assign_expression_node::print(std::ostream& os) const {
    return os << '(' << *m_lhs << ' ' << m_compound << "= " << *m_rhs << ')';
}

bool var_assign_expression_node::equal(const node& rhsNode) const {
    const auto& rhs = reinterpret_cast<const var_assign_expression_node&>(rhsNode);
    return expression_node::equal(rhsNode) && m_compound == rhs.m_compound && m_lhs == rhs.m_lhs && m_rhs == rhs.m_rhs;
}

bool declaration_node::equal(const node& rhs) const {
    return declaration_type() == reinterpret_cast<const declaration_node&>(rhs).declaration_type();
}

void function_declaration_node::accept(visitor& visitor) const {
    visitor.visit(*this);
}

std::ostream& function_declaration_node::print(std::ostream& os) const {
    return os << "function " << m_name->string << " declaration";
}

bool function_declaration_node::equal(const node& rhs) const {
    return declaration_node::equal(rhs) && m_name == reinterpret_cast<const function_declaration_node&>(rhs).m_name;
}

void function_definition_node::accept(visitor& visitor) const {
    visitor.visit(*this);
}

std::ostream& function_definition_node::print(std::ostream& os) const {
    function_declaration_node::print(os);
    os << ':';
    if (m_body.present()) {
        for (const auto& entry : m_body->statements)
            os << '\n' << entry;
        return os << '\n' << m_body->end << ": " << m_name->string << " end";
    }
    return os << m_body.error(); // error
}

bool function_definition_node::equal(const node& rhs) const {
    return function_declaration_node::equal(rhs) &&
           m_body == reinterpret_cast<const function_definition_node&>(rhs).m_body;
}

bool statement_node::equal(const node& rhs) const {
    return statement_type() == reinterpret_cast<const statement_node&>(rhs).statement_type();
}

void return_statement_node::accept(visitor& visitor) const {
    visitor.visit(*this);
}

std::ostream& return_statement_node::print(std::ostream& os) const {
    os << "return statement";
    if (m_value.present()) return os << ' ' << *m_value;
    return os;
}

bool return_statement_node::equal(const node& rhs) const {
    return statement_node::equal(rhs) && m_value == reinterpret_cast<const return_statement_node&>(rhs).m_value;
}

void if_statement_node::accept(visitor& visitor) const {
    visitor.visit(*this);
}

std::ostream& if_statement_node::print(std::ostream& os) const {
    os << "if " << *m_cond << ", then:\n";
    os << m_then;
    if (m_else.present()) os << m_else;
    return os;
}

bool if_statement_node::equal(const node& rhsNode) const {
    const auto& rhs = reinterpret_cast<const if_statement_node&>(rhsNode);
    return statement_node::equal(rhs) && m_cond == rhs.m_cond && m_then == rhs.m_then && m_else == rhs.m_else;
}

void compound_statement_node::accept(visitor& visitor) const {
    visitor.visit(*this);
}

std::ostream& compound_statement_node::print(std::ostream& os) const {
    return os << m_body;
}

bool compound_statement_node::equal(const node& rhs) const {
    return statement_node::equal(rhs) && m_body == reinterpret_cast<const compound_statement_node&>(rhs).m_body;
}

void program_node::accept(visitor& visitor) const {
    for (const auto& decl : m_declarations) {
        if (decl.has_error()) {
            visitor.visit_error(decl);
        } else {
            decl->accept(visitor);
        }
    }
}

std::ostream& program_node::print(std::ostream& os) const {
    os << "program:";
    for (const auto& handle : m_declarations) {
        os << '\n' << handle;
    }
    return os;
}

bool program_node::equal(const node& rhs) const {
    return m_declarations == reinterpret_cast<const program_node&>(rhs).m_declarations;
}

std::ostream& operator<<(std::ostream& os, const body& body) {
    os << "body:";
    for (const auto& stmt : body.statements) {
        os << '\n' << stmt;
    }
    return os;
}

} // namespace furc::ast