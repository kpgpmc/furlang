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
    void visit(const ast::function_definition_node& funcDef) override;
    void visit(const ast::return_statement_node& returnStmt) override;
    void visit(const ast::if_statement_node& node) override;
    void visit(const ast::compound_statement_node& node) override;
    void visit(const ast::string_literal_node& node) override;
    void visit(const ast::integer_literal_node& node) override;
    void visit(const ast::var_read_expression_node& node) override;
    void visit(const ast::unaryop_expression_node& node) override;
    void visit(const ast::binop_expression_node& node) override;
    void visit(const ast::var_assign_expression_node& node) override;
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