#ifndef FURC_FRONT_IR_GENERATOR_HPP
#define FURC_FRONT_IR_GENERATOR_HPP

#include "furc/ast/fwd.hpp"
#include "furc/ast/visitor.hpp"
#include "furlang/ir/module.hpp"

namespace furc {
namespace front {

class ir_generator final : public ast::visitor {
public:
    ir_generator()           = default;
    ~ir_generator() override = default;

    ir_generator(ir_generator&&)                 = default;
    ir_generator& operator=(ir_generator&&)      = default;
    ir_generator(const ir_generator&)            = delete;
    ir_generator& operator=(const ir_generator&) = delete;
public:
    furlang::ir::module&& move_module() { return std::move(m_module); }
public:
    void visit_function_definition_node(const ast::function_definition_node& funcDef) override;
    void visit_return_statement_node(const ast::return_statement_node& returnStmt) override;
    void visit_if_statement_node(const ast::if_statement_node& node) override;
    void visit_compound_statement_node(const ast::compound_statement_node& node) override;
    void visit_string_literal_node(const ast::string_literal_node& node) override;
    void visit_integer_literal_node(const ast::integer_literal_node& node) override;
    void visit_var_read_expression_node(const ast::var_read_expression_node& node) override;
    void visit_unaryop_expression_node(const ast::unaryop_expression_node& node) override;
    void visit_binop_expression_node(const ast::binop_expression_node& node) override;
    void visit_var_assign_expression_node(const ast::var_assign_expression_node& node) override;
private:
    furlang::ir::block_index push_block();
private:
    furlang::ir::module                    m_module;
    std::unique_ptr<furlang::ir::function> m_currentFunction;
    std::shared_ptr<furlang::ir::block>    m_currentBlock;
    std::uint32_t                          m_registerCounter = 0;
};

} // namespace front
} // namespace furc

#endif // FURC_FRONT_IR_GENERATOR_HPP