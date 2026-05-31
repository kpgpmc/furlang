#ifndef FURLANG_IR_INSTRUCTION_HPP
#define FURLANG_IR_INSTRUCTION_HPP

#include "furlang/ir/operand.hpp"

#include <cstdint>

namespace furlang {
namespace ir {

enum class instruction_t {
    Alloca,
    Assign,
    BinaryOp,
    Call,
    Branch,
    BranchNotZero,
};

class instruction {
public:
    instruction()          = default;
    virtual ~instruction() = default;

    instruction(instruction&&)                 = default;
    instruction& operator=(instruction&&)      = default;
    instruction(const instruction&)            = delete;
    instruction& operator=(const instruction&) = delete;
public:
    virtual instruction_t type() const = 0;
};

class assign_instruction final : public instruction {
public:
    assign_instruction(operand&& src, operand&& dst)
      : m_source(std::move(src)), m_destination(std::move(dst)) {}

    ~assign_instruction() override = default;

    assign_instruction(assign_instruction&&)                 = default;
    assign_instruction& operator=(assign_instruction&&)      = default;
    assign_instruction(const assign_instruction&)            = delete;
    assign_instruction& operator=(const assign_instruction&) = delete;
public:
    instruction_t type() const override { return instruction_t::Assign; }

    const operand& source() const { return m_source; }
    const operand& destination() const { return m_destination; }
private:
    operand m_source;
    operand m_destination;
};

enum class binary_op_instruction_t {
    Add,
    Sub,
    Mul,
    Div,
    Mod,

    Eq,
    NotEq,
    LessThan,
    GreaterThan,
    LessEq,
    GreaterEq,
};

class binary_op_instruction final : public instruction {
public:
    binary_op_instruction(binary_op_instruction_t type, operand&& lhs, operand&& rhs, operand&& dst)
      : m_type(type), m_lhs(std::move(lhs)), m_rhs(std::move(rhs)), m_dst(std::move(dst)) {}

    ~binary_op_instruction() override = default;

    binary_op_instruction(binary_op_instruction&&)                 = default;
    binary_op_instruction& operator=(binary_op_instruction&&)      = default;
    binary_op_instruction(const binary_op_instruction&)            = delete;
    binary_op_instruction& operator=(const binary_op_instruction&) = delete;
public:
    instruction_t type() const override { return instruction_t::BinaryOp; }

    binary_op_instruction_t op_type() const { return m_type; }
    const operand&          lhs() const { return m_lhs; }
    const operand&          rhs() const { return m_rhs; }
    const operand&          dst() const { return m_dst; }
private:
    binary_op_instruction_t m_type;
    operand                 m_lhs;
    operand                 m_rhs;
    operand                 m_dst;
};

using block_index = std::uint64_t;

class branch_instruction final : public instruction {
public:
    branch_instruction(block_index block)
      : m_block(block) {}

    ~branch_instruction() override = default;

    branch_instruction(branch_instruction&&)                 = default;
    branch_instruction& operator=(branch_instruction&&)      = default;
    branch_instruction(const branch_instruction&)            = delete;
    branch_instruction& operator=(const branch_instruction&) = delete;
public:
    instruction_t type() const override { return instruction_t::Branch; }

    block_index block() const { return m_block; }
private:
    block_index m_block;
};

class branch_nz_instruction final : public instruction {
public:
    branch_nz_instruction(operand&& condition, block_index block)
      : m_condition(std::move(condition)), m_block(block) {}

    ~branch_nz_instruction() override = default;

    branch_nz_instruction(branch_nz_instruction&&)                 = default;
    branch_nz_instruction& operator=(branch_nz_instruction&&)      = default;
    branch_nz_instruction(const branch_nz_instruction&)            = delete;
    branch_nz_instruction& operator=(const branch_nz_instruction&) = delete;
public:
    instruction_t type() const override { return instruction_t::BranchNotZero; }

    const operand& condition() const { return m_condition; }
    block_index    block() const { return m_block; }
private:
    operand     m_condition;
    block_index m_block;
};

} // namespace ir
} // namespace furlang

#endif // FURLANG_IR_INSTRUCTION_HPP