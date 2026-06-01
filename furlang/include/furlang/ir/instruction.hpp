#ifndef FURLANG_IR_INSTRUCTION_HPP
#define FURLANG_IR_INSTRUCTION_HPP

#include "furlang/ir/operand.hpp"

#include <cstdint>
#include <optional>
#include <ostream>

namespace furlang {
namespace ir {

enum class instruction_t {
    Alloca,
    Assign,
    BinaryOp,
    Call,
    Branch,
    BranchCond,
    Return,
};

static inline bool is_exit_instruction(instruction_t type) {
    switch (type) {
    case instruction_t::Branch:
    case instruction_t::BranchCond:
    case instruction_t::Return: return true;
    default: return false;
    }
}

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
public:
    friend std::ostream& operator<<(std::ostream& os, const instruction& instruction) { return instruction.print(os); }
protected:
    virtual std::ostream& print(std::ostream& os) const = 0;
};

class alloca_instruction final : public instruction {
public:
    alloca_instruction() {}

    ~alloca_instruction() override = default;

    alloca_instruction(alloca_instruction&&)                 = default;
    alloca_instruction& operator=(alloca_instruction&&)      = default;
    alloca_instruction(const alloca_instruction&)            = delete;
    alloca_instruction& operator=(const alloca_instruction&) = delete;
public:
    instruction_t type() const override { return instruction_t::Alloca; }
protected:
    std::ostream& print(std::ostream& os) const override { return os << "alloca"; }
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
protected:
    std::ostream& print(std::ostream& os) const override {
        return os << "assign " << m_source << ", " << m_destination;
    }
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

static inline std::ostream& operator<<(std::ostream& os, binary_op_instruction_t type) {
    switch (type) {
    case binary_op_instruction_t::Add: return os << '+';
    case binary_op_instruction_t::Sub: return os << '-';
    case binary_op_instruction_t::Mul: return os << '*';
    case binary_op_instruction_t::Div: return os << '/';
    case binary_op_instruction_t::Mod: return os << '%';
    case binary_op_instruction_t::Eq: return os << "==";
    case binary_op_instruction_t::NotEq: return os << "!=";
    case binary_op_instruction_t::LessThan: return os << '<';
    case binary_op_instruction_t::GreaterThan: return os << '>';
    case binary_op_instruction_t::LessEq: return os << "<=";
    case binary_op_instruction_t::GreaterEq: return os << ">=";
    }
    return os;
}

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
protected:
    std::ostream& print(std::ostream& os) const override {
        return os << "binop(" << m_type << ") " << m_lhs << ", " << m_rhs << ", " << m_dst;
    }
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
protected:
    std::ostream& print(std::ostream& os) const override { return os << "branch #" << m_block; }
};

class branch_cond_instruction final : public instruction {
public:
    branch_cond_instruction(operand&& condition, block_index ifBlock, block_index elseBlock)
      : m_condition(std::move(condition)), m_ifBlock(ifBlock), m_elseBlock(elseBlock) {}

    ~branch_cond_instruction() override = default;

    branch_cond_instruction(branch_cond_instruction&&)                 = default;
    branch_cond_instruction& operator=(branch_cond_instruction&&)      = default;
    branch_cond_instruction(const branch_cond_instruction&)            = delete;
    branch_cond_instruction& operator=(const branch_cond_instruction&) = delete;
public:
    instruction_t type() const override { return instruction_t::BranchCond; }

    const operand& condition() const { return m_condition; }
    block_index    if_block() const { return m_ifBlock; }
    block_index    else_block() const { return m_elseBlock; }
private:
    operand     m_condition;
    block_index m_ifBlock;
    block_index m_elseBlock;
protected:
    std::ostream& print(std::ostream& os) const override {
        return os << "branch_cond " << m_condition << ", #" << m_ifBlock << ", #" << m_elseBlock;
    }
};

class return_instruction final : public instruction {
public:
    return_instruction() {}
    return_instruction(operand&& value)
      : m_value(std::move(value)) {}

    ~return_instruction() override = default;

    return_instruction(return_instruction&&)                 = default;
    return_instruction& operator=(return_instruction&&)      = default;
    return_instruction(const return_instruction&)            = delete;
    return_instruction& operator=(const return_instruction&) = delete;
public:
    instruction_t type() const override { return instruction_t::Return; }

    const std::optional<operand>& value() const { return m_value; }
private:
    std::optional<operand> m_value;
protected:
    std::ostream& print(std::ostream& os) const override {
        os << "return";
        if (m_value.has_value()) os << ' ' << m_value.value();
        return os;
    }
};

} // namespace ir
} // namespace furlang

#endif // FURLANG_IR_INSTRUCTION_HPP