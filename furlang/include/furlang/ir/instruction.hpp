#ifndef FURLANG_IR_INSTRUCTION_HPP
#define FURLANG_IR_INSTRUCTION_HPP

#include "furlang/ir/operand.hpp"

#include <cstdint>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace furlang {
namespace ir {

/**
 * @brief IR instruction type.
 */
enum class instruction_t {
    Alloca,      /**< Unused */
    Assign,      /**< Assign */
    Add,         /**< Addition */
    Sub,         /**< Subtraction */
    Mul,         /**< Multiplication */
    Div,         /**< Division */
    Mod,         /**< Modulo */
    Eq,          /**< Equal */
    NotEq,       /**< Not equal */
    LessThan,    /**< Less than */
    GreaterThan, /**< Greater than */
    LessEq,      /**< Less or equal */
    GreaterEq,   /**< Greater or equal */
    Pointerof,   /**< Pointerof */
    Call,        /**< Call */
    Branch,      /**< Branch */
    BranchCond,  /**< Conditional branch */
    Return,      /**< Return */
    Phi,         /**< Phi function */
};

static inline std::ostream& operator<<(std::ostream& os, instruction_t type) {
    switch (type) {
    case instruction_t::Alloca: return os << "alloca";
    case instruction_t::Assign: return os << "assign";
    case instruction_t::Add: return os << "add";
    case instruction_t::Sub: return os << "sub";
    case instruction_t::Mul: return os << "mul";
    case instruction_t::Div: return os << "div";
    case instruction_t::Mod: return os << "mod";
    case instruction_t::Eq: return os << "eq";
    case instruction_t::NotEq: return os << "notEq";
    case instruction_t::LessThan: return os << "lessThan";
    case instruction_t::GreaterThan: return os << "greaterThan";
    case instruction_t::LessEq: return os << "lessEq";
    case instruction_t::GreaterEq: return os << "greaterEq";
    case instruction_t::Pointerof: return os << "pointerof";
    case instruction_t::Call: return os << "call";
    case instruction_t::Branch: return os << "branch";
    case instruction_t::BranchCond: return os << "branchCond";
    case instruction_t::Return: return os << "return";
    case instruction_t::Phi: return os << "phi";
    }
}

/**
 * @brief Checks if an instruction type exits.
 *
 * @param type Instruction type.
 * @return true if the instruction type exits.
 */
static inline bool is_exit_instruction(instruction_t type) {
    switch (type) {
    case instruction_t::Branch:
    case instruction_t::BranchCond:
    case instruction_t::Return: return true;
    default: return false;
    }
}

/**
 * @brief IR instruction
 */
class instruction {
public:
    instruction()          = default;
    virtual ~instruction() = default;

    /**
     * @brief Move constructor
     */
    instruction(instruction&&) = default;

    /**
     * @brief Move constructor
     */
    instruction& operator=(instruction&&) = default;

    instruction(const instruction&) = delete;

    instruction& operator=(const instruction&) = delete;
public:
    /**
     * @brief Returns this instruction's type.
     *
     * @return The type.
     */
    virtual instruction_t type() const = 0;

    /**
     * @brief Returns whether this instruction has a destination operand.
     *
     * @return true if this instruction has the destination operand.
     */
    virtual bool has_destination() const { return false; }

    /**
     * @brief Returns destination operand of this instruction.
     *
     * @return The destination operand.
     */
    virtual operand& destination() { throw std::runtime_error("instruction type mismatch"); }

    /**
     * @brief Returns destination operand of this instruction.
     *
     * @return The destination operand.
     */
    virtual const operand& destination() const { throw std::runtime_error("instruction type mismatch"); }

    /**
     * @brief Returns a list of source operands of this instruction.
     *
     * @return The list of source operands.
     */
    virtual std::vector<operand*> sources() { return {}; }

    /**
     * @brief Returns a list of source operands of this instruction.
     *
     * @return The list of source operands.
     */
    virtual std::vector<const operand*> sources() const { return {}; }
public:
    /**
     * @brief Prints an instruction to an output stream.
     *
     * Equivalent to calling instruction.print(os).
     *
     * @param os Output stream.
     * @param instruction Instruction to print.
     * @return The output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const instruction& instruction) { return instruction.print(os); }
protected:
    /**
     * @brief Prints this instruction to an output stream.
     *
     * @param os Output stream.
     * @return The output stream.
     */
    virtual std::ostream& print(std::ostream& os) const = 0;
};

/**
 * @brief Alloca instruction
 */
class alloca_instruction final : public instruction {
public:
    alloca_instruction() {}

    ~alloca_instruction() override = default;

    /**
     * @brief Move constructor
     */
    alloca_instruction(alloca_instruction&&) = default;

    /**
     * @brief Move constructor
     */
    alloca_instruction& operator=(alloca_instruction&&) = default;

    alloca_instruction(const alloca_instruction&) = delete;

    alloca_instruction& operator=(const alloca_instruction&) = delete;
public:
    /**
     * @brief Returns this instruction's type.
     *
     * @return instruction_t::Alloca.
     */
    instruction_t type() const override { return instruction_t::Alloca; }
protected:
    std::ostream& print(std::ostream& os) const override { return os << "alloca"; }
};

/**
 * @brief Assign instruction
 */
class assign_instruction final : public instruction {
public:
    /**
     * @brief Construct a new assign instruction.
     *
     * @param src Source operand.
     * @param dst Destination operand.
     */
    assign_instruction(operand&& src, operand&& dst)
      : m_source(std::move(src)), m_destination(std::move(dst)) {}

    ~assign_instruction() override = default;

    /**
     * @brief Move constructor
     */
    assign_instruction(assign_instruction&&) = default;

    /**
     * @brief Move constructor
     */
    assign_instruction& operator=(assign_instruction&&) = default;

    assign_instruction(const assign_instruction&) = delete;

    assign_instruction& operator=(const assign_instruction&) = delete;
public:
    /**
     * @brief Returns this instruction's type.
     *
     * @return instruction_t::Assign.
     */
    instruction_t type() const override { return instruction_t::Assign; }

    /**
     * @brief Returns whether this instruction has a destination operand.
     *
     * @return true
     */
    bool has_destination() const override { return true; }

    /**
     * @brief Returns this instruction's destination.
     *
     * @return The destination.
     */
    operand& destination() override { return m_destination; }

    /**
     * @brief Returns this instruction's destination.
     *
     * @return The destination.
     */
    const operand& destination() const override { return m_destination; }

    /**
     * @brief Returns a list of this instruction's source operands.
     *
     * @return The list of source operands.
     */
    std::vector<operand*> sources() override { return { &m_source }; }

    /**
     * @brief Returns a list of this instruction's source operands.
     *
     * @return The list of source operands.
     */
    std::vector<const operand*> sources() const override { return { &m_source }; }
private:
    operand m_source;
    operand m_destination;
protected:
    std::ostream& print(std::ostream& os) const override {
        return os << "assign " << m_source << ", " << m_destination;
    }
};

/**
 * @brief Generic unary instruction
 */
class unary_instruction final : public instruction {
public:
    /**
     * @brief Construct a new unary instruction.
     *
     * @param type Instruction type.
     * @param src Source operand.
     * @param dst Destination operand.
     */
    unary_instruction(instruction_t type, operand&& src, operand&& dst)
      : m_type(type), m_src(std::move(src)), m_dst(std::move(dst)) {}

    ~unary_instruction() override = default;

    /**
     * @brief Move constructor
     */
    unary_instruction(unary_instruction&&) = default;

    /**
     * @brief Move constructor
     */
    unary_instruction& operator=(unary_instruction&&) = default;

    unary_instruction(const unary_instruction&) = delete;

    unary_instruction& operator=(const unary_instruction&) = delete;
public:
    /**
     * @brief Returns this instruction's type.
     *
     * @return The instruction type.
     */
    instruction_t type() const override { return m_type; }

    bool has_destination() const override { return true; }

    operand& destination() override { return m_dst; }

    const operand& destination() const override { return m_dst; }

    std::vector<operand*> sources() override { return { &m_src }; }

    std::vector<const operand*> sources() const override { return { &m_src }; }

    /**
     * @brief Returns this instruction's source operand.
     *
     * @return The operand.
     */
    const operand& src() const { return m_src; }

    /**
     * @brief Returns this instruction's destination operand.
     *
     * @return The operand.
     */
    const operand& dst() const { return m_dst; }
private:
    instruction_t m_type;
    operand       m_src;
    operand       m_dst;
protected:
    std::ostream& print(std::ostream& os) const override { return os << m_dst << " = " << m_type << ' ' << m_src; }
};

/**
 * @brief Generic binary instruction
 */
class binary_instruction final : public instruction {
public:
    /**
     * @brief Construct a new binary operation instruction.
     *
     * @param type Instruction type.
     * @param lhs Left-hand-side operand.
     * @param rhs Right-hand-side operand.
     * @param dst Destination operand.
     */
    binary_instruction(instruction_t type, operand&& lhs, operand&& rhs, operand&& dst)
      : m_type(type), m_lhs(std::move(lhs)), m_rhs(std::move(rhs)), m_dst(std::move(dst)) {}

    ~binary_instruction() override = default;

    /**
     * @brief Move constructor
     */
    binary_instruction(binary_instruction&&) = default;

    /**
     * @brief Move constructor
     */
    binary_instruction& operator=(binary_instruction&&) = default;

    binary_instruction(const binary_instruction&) = delete;

    binary_instruction& operator=(const binary_instruction&) = delete;
public:
    /**
     * @brief Returns this instruction's type.
     *
     * @return instruction_t::BinaryOp.
     */
    instruction_t type() const override { return m_type; }

    bool has_destination() const override { return true; }

    operand& destination() override { return m_dst; }

    const operand& destination() const override { return m_dst; }

    std::vector<operand*> sources() override { return { &m_lhs, &m_rhs }; }

    std::vector<const operand*> sources() const override { return { &m_lhs, &m_rhs }; }

    /**
     * @brief Returns this instruction's left-hand-side operand.
     *
     * @return The operand.
     */
    const operand& lhs() const { return m_lhs; }

    /**
     * @brief Returns this instruction's right-hand-side operand.
     *
     * @return The operand.
     */
    const operand& rhs() const { return m_rhs; }

    /**
     * @brief Returns this instruction's destination operand.
     *
     * @return The operand.
     */
    const operand& dst() const { return m_dst; }
private:
    instruction_t m_type;
    operand       m_lhs /**< Left-hand-side operand */;
    operand       m_rhs /**< Right-hand-side operand */;
    operand       m_dst /**< Destination operand */;
protected:
    std::ostream& print(std::ostream& os) const override {
        return os << m_dst << " = " << m_lhs << ' ' << m_type << ' ' << m_rhs;
    }
};

using block_index = std::uint64_t; /**< IR block index alias */

/**
 * @brief Branch instruction
 */
class branch_instruction final : public instruction {
public:
    /**
     * @brief Construct a new branch instruction.
     *
     * @param block Destination block index.
     */
    branch_instruction(block_index block)
      : m_block(block) {}

    ~branch_instruction() override = default;

    /**
     * @brief Move constructor.
     */
    branch_instruction(branch_instruction&&) = default;

    /**
     * @brief Move constructor.
     */
    branch_instruction& operator=(branch_instruction&&) = default;

    branch_instruction(const branch_instruction&) = delete;

    branch_instruction& operator=(const branch_instruction&) = delete;
public:
    /**
     * @brief Returns this instruction's type.
     *
     * @return instruction_t::Branch.
     */
    instruction_t type() const override { return instruction_t::Branch; }

    /**
     * @brief Returns this instruction's destination block index.
     *
     * @return The destination block index.
     */
    block_index block() const { return m_block; }
private:
    block_index m_block; /**< Destination block index. */
protected:
    std::ostream& print(std::ostream& os) const override { return os << "branch #" << m_block; }
};

/**
 * @brief Conditional branch instruction
 */
class branch_cond_instruction final : public instruction {
public:
    /**
     * @brief Construct a new conditional branch instruction.
     *
     * @param condition Condition operand.
     * @param ifBlock Destination block index.
     * @param elseBlock Else block index.
     */
    branch_cond_instruction(operand&& condition, block_index ifBlock, block_index elseBlock)
      : m_condition(std::move(condition)), m_ifBlock(ifBlock), m_elseBlock(elseBlock) {}

    ~branch_cond_instruction() override = default;

    /**
     * @brief Move constructor.
     */
    branch_cond_instruction(branch_cond_instruction&&) = default;

    /**
     * @brief Move constructor.
     */
    branch_cond_instruction& operator=(branch_cond_instruction&&) = default;

    branch_cond_instruction(const branch_cond_instruction&) = delete;

    branch_cond_instruction& operator=(const branch_cond_instruction&) = delete;
public:
    instruction_t type() const override { return instruction_t::BranchCond; }

    std::vector<operand*>       sources() override { return { &m_condition }; }
    std::vector<const operand*> sources() const override { return { &m_condition }; }

    /**
     * @brief Returns this instruction's condition operand.
     *
     * @return The operand.
     */
    const operand& condition() const { return m_condition; }

    /**
     * @brief Returns this instruction's destination block index.
     *
     * @return The destination block index.
     */
    block_index if_block() const { return m_ifBlock; }

    /**
     * @brief Returns this instruction's else block index.
     *
     * @return The else block index.
     */
    block_index else_block() const { return m_elseBlock; }
private:
    operand     m_condition /**< Condition operand. */;
    block_index m_ifBlock /**< Destination block index. */;
    block_index m_elseBlock /**< Else block index. */;
protected:
    std::ostream& print(std::ostream& os) const override {
        return os << "branch_cond " << m_condition << ", #" << m_ifBlock << ", #" << m_elseBlock;
    }
};

/**
 * @brief Return instruction
 */
class return_instruction final : public instruction {
public:
    return_instruction() = default;

    /**
     * @brief Construct a new return instruction.
     *
     * @param value Return value operand.
     */
    return_instruction(operand&& value)
      : m_value(std::move(value)) {}

    ~return_instruction() override = default;

    /**
     * @brief Move constructor.
     */
    return_instruction(return_instruction&&) = default;

    /**
     * @brief Move constructor.
     */
    return_instruction& operator=(return_instruction&&) = default;

    return_instruction(const return_instruction&) = delete;

    return_instruction& operator=(const return_instruction&) = delete;
public:
    /**
     * @brief Returns this instruction's type.
     *
     * @return instruction_t::Return.
     */
    instruction_t type() const override { return instruction_t::Return; }

    std::vector<operand*> sources() override {
        if (m_value.has_value()) return { &*m_value };
        return {};
    }

    std::vector<const operand*> sources() const override {
        if (m_value.has_value()) return { &*m_value };
        return {};
    }

    /**
     * @brief Returns this instruction's return value operand.
     *
     * @return The operand.
     */
    const std::optional<operand>& value() const { return m_value; }
private:
    std::optional<operand> m_value; /**< The return value operand. */
protected:
    std::ostream& print(std::ostream& os) const override {
        os << "return";
        if (m_value.has_value()) os << ' ' << m_value.value();
        return os;
    }
};

/**
 * @brief Phi instruction
 *
 * Used to implement the phi node in the SSA graph.
 */
class phi_instruction final : public instruction {
public:
    phi_instruction(register_operand dst)
      : m_dst(operand::new_reg(dst)) {}

    ~phi_instruction() override = default;

    /**
     * @brief Move constructor.
     */
    phi_instruction(phi_instruction&&) noexcept = default;

    /**
     * @brief Move constructor.
     */
    phi_instruction& operator=(phi_instruction&&) noexcept = default;

    phi_instruction(const phi_instruction&) = delete;

    phi_instruction& operator=(const phi_instruction&) = delete;
public:
    /**
     * @brief Returns this instruction's type.
     *
     * @return instruction_t::Phi.
     */
    instruction_t type() const override { return instruction_t::Phi; }

    bool has_destination() const override { return true; }

    /**
     * @brief Returns this instruction's destination register.
     *
     * @return The register.
     */
    operand& destination() override { return m_dst; }

    /**
     * @brief Returns this instruction's destination register.
     *
     * @return The register.
     */
    const operand& destination() const override { return m_dst; }

    std::vector<const operand*> sources() const override {
        std::vector<const operand*> srcs;
        srcs.reserve(m_labels.size());
        for (const auto& [op, _block] : m_labels)
            srcs.push_back(&op);
        return srcs;
    }

    /**
     * @brief Returns this instruction's labels.
     *
     * @return The labels.
     */
    std::vector<std::pair<operand, block_index>>& labels() { return m_labels; }

    /**
     * @brief Returns this instruction's labels.
     *
     * @return The labels.
     */
    const std::vector<std::pair<operand, block_index>>& labels() const { return m_labels; }
private:
    operand                                      m_dst;
    std::vector<std::pair<operand, block_index>> m_labels;
protected:
    std::ostream& print(std::ostream& os) const override {
        os << m_dst << " = phi";
        bool first = true;
        for (const auto& pair : m_labels) {
            if (!first) os << ',';
            first = false;
            os << ' ' << pair.second << ": " << pair.first;
        }
        return os;
    }
};

/**
 * @brief Function call instruction.
 */
class call_instruction final : public instruction {
public:
    template <typename NameFwd, typename ArgsFwd>
    call_instruction(NameFwd&& name, operand&& dst, ArgsFwd&& args)
      : m_name(std::forward<NameFwd>(name)), m_dst(std::move(dst)), m_args(std::forward<ArgsFwd>(args)) {}

    ~call_instruction() override = default;

    call_instruction(call_instruction&&) noexcept            = default;
    call_instruction& operator=(call_instruction&&) noexcept = default;
    call_instruction(const call_instruction&)                = delete;
    call_instruction& operator=(const call_instruction&)     = delete;
public:
    /**
     * @brief Returns this instruction's type.
     *
     * @return instruction_t::Call.
     */
    instruction_t type() const override { return instruction_t::Call; }

    bool has_destination() const override { return true; }

    /**
     * @brief Returns this instruction's destination register.
     *
     * @return The register.
     */
    operand& destination() override { return m_dst; }

    /**
     * @brief Returns this instruction's destination register.
     *
     * @return The register.
     */
    const operand& destination() const override { return m_dst; }

    std::vector<const operand*> sources() const override {
        std::vector<const operand*> srcs;
        srcs.reserve(m_args.size());
        for (const auto& op : m_args)
            srcs.push_back(&op);
        return srcs;
    }

    /**
     * @brief Returns this instruction's callee name.
     *
     * @return The name.
     */
    const std::string& name() const { return m_name; }
private:
    std::string          m_name;
    operand              m_dst;
    std::vector<operand> m_args;
protected:
    std::ostream& print(std::ostream& os) const override {
        os << "call " << m_name << '(';
        bool first = true;
        for (const auto& op : m_args) {
            if (!first) os << ", ";
            first = false;
            os << op;
        }
        return os << ") = " << m_dst;
    }
};

} // namespace ir
} // namespace furlang

#endif // FURLANG_IR_INSTRUCTION_HPP
