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

std::ostream& integer_literal_node::print(std::ostream& os) const {
    if (m_value.has_error()) return os << m_value.error();
    return os << "integer literal (" << *m_value << ")";
}

bool integer_literal_node::equal(const node& rhs) const {
    return literal_node::equal(rhs) && m_value == reinterpret_cast<const integer_literal_node&>(rhs).m_value;
}

std::ostream& string_literal_node::print(std::ostream& os) const {
    if (m_value.has_error()) return os << m_value.error();
    return os << "string literal (" << *m_value << ")";
}

bool string_literal_node::equal(const node& rhs) const {
    return literal_node::equal(rhs) && m_value == reinterpret_cast<const string_literal_node&>(rhs).m_value;
}

bool expression_node::equal(const node& rhs) const {
    return expression_type() == reinterpret_cast<const expression_node&>(rhs).expression_type();
}

bool declaration_node::equal(const node& rhs) const {
    return declaration_type() == reinterpret_cast<const declaration_node&>(rhs).declaration_type();
}

std::ostream& function_declarartion_node::print(std::ostream& os) const {
    return os << "function " << m_name->string << " declaration";
}

bool function_declarartion_node::equal(const node& rhs) const {
    return declaration_node::equal(rhs) && m_name == reinterpret_cast<const function_declarartion_node&>(rhs).m_name;
}

std::ostream& function_definition_node::print(std::ostream& os) const {
    function_declarartion_node::print(os);
    os << ':';
    if (m_body.present()) {
        for (const auto& entry : m_body->statements)
            os << '\n' << entry;
        return os << '\n' << m_body->end << ": " << m_name->string << " end";
    }
    return os << m_body.error(); // error
}

bool function_definition_node::equal(const node& rhs) const {
    return function_declarartion_node::equal(rhs) &&
           m_body == reinterpret_cast<const function_definition_node&>(rhs).m_body;
}

bool statement_node::equal(const node& rhs) const {
    return statement_type() == reinterpret_cast<const statement_node&>(rhs).statement_type();
}

std::ostream& return_statement_node::print(std::ostream& os) const {
    os << "return statement";
    if (m_value.present()) return os << " (" << *m_value << ')';
    return os;
}

bool return_statement_node::equal(const node& rhs) const {
    return statement_node::equal(rhs) && m_value == reinterpret_cast<const return_statement_node&>(rhs).m_value;
}

bool program_node::equal(const node& rhs) const {
    return m_declarations == reinterpret_cast<const program_node&>(rhs).m_declarations;
}

std::ostream& program_node::print(std::ostream& os) const {
    os << "program:";
    for (const auto& handle : m_declarations) {
        os << '\n' << handle;
    }
    return os;
}

} // namespace furc::ast