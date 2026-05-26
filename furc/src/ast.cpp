#include "furc/ast/declaration.hpp"
#include "furc/ast/literal.hpp"
#include "furc/ast/node.hpp"
#include "furc/ast/program.hpp"
#include "furc/ast/statement.hpp"

#include <ostream>

namespace furc::ast {

std::ostream& integer_literal_node::print(std::ostream& os) const {
    if (m_value.has_error()) return os << m_value.error();
    return os << "integer literal (" << *m_value << ")";
}

std::ostream& function_declarartion_node::print(std::ostream& os) const {
    return os << "function " << m_name->string << " declaration";
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

std::ostream& return_statement_node::print(std::ostream& os) const {
    os << "return statement";
    if (m_value.present()) return os << " (" << *m_value << ')';
    return os;
}

std::ostream& program_node::print(std::ostream& os) const {
    os << "program:";
    for (const auto& handle : m_declarations) {
        os << '\n' << handle;
    }
    return os;
}

} // namespace furc::ast