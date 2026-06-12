#ifndef FURLANG_IR_INSTRUCTION_HPP
#define FURLANG_IR_INSTRUCTION_HPP

#include "furlang/ir/operand.hpp"

#include <cstdint>
#include <optional>
#include <ostream>
#include <utility>
#include <vector>

namespace furlang {
namespace ir {

/**
 * @brief IR instruction type.
 */
enum class instruction_t {
    Alloca,     /**< Unused */
    Assign,     /**< Assign */
    BinaryOp,   /**< Binary operation */
    Call,       /**< Call */
    Branch,     /**< Branch */
    BranchCond, /**< Conditional branch */
    Return,     /**< Return */
    Phi,        /**< Phi function */
};

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
     * @brief Returns this instruction's source.
     *
     * @return The source.
     */
    const operand& source() const { return m_source; }

    /**
     * @brief Returns this instruction's destination.
     *
     * @return The destination.
     */
    const operand& destination() const { return m_destination; }
private:
    operand m_source;
    operand m_destination;
protected:
    std::ostream& print(std::ostream& os) const override {
        return os << "assign " << m_source << ", " << m_destination;
    }
};

/**
 * @brief Binary operation instruction type
 */
enum class binary_op_instruction_t {
    Add, /**< Addition */
    Sub, /**< Subtraction */
    Mul, /**< Multiplication */
    Div, /**< Division */
    Mod, /**< Modulo */

    Eq,          /**< Equal */
    NotEq,       /**< Not equal */
    LessThan,    /**< Less than */
    GreaterThan, /**< Greater than */
    LessEq,      /**< Less or equal */
    GreaterEq,   /**< Greater or equal */
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

/**
 * @brief Binary operation instruction
 */
class binary_op_instruction final : public instruction {
public:
    /**
     * @brief Construct a new binary operation instruction.
     *
     * @param type Operation type.
     * @param lhs Left-hand-side operand.
     * @param rhs Right-hand-side operand.
     * @param dst Destination operand.
     */
    binary_op_instruction(binary_op_instruction_t type, operand&& lhs, operand&& rhs, operand&& dst)
      : m_type(type), m_lhs(std::move(lhs)), m_rhs(std::move(rhs)), m_dst(std::move(dst)) {}

    ~binary_op_instruction() override = default;

    /**
     * @brief Move constructor
     */
    binary_op_instruction(binary_op_instruction&&) = default;

    /**
     * @brief Move constructor
     */
    binary_op_instruction& operator=(binary_op_instruction&&) = default;

    binary_op_instruction(const binary_op_instruction&) = delete;

    binary_op_instruction& operator=(const binary_op_instruction&) = delete;
public:
    /**
     * @brief Returns this instruction's type.
     *
     * @return instruction_t::BinaryOp.
     */
    instruction_t type() const override { return instruction_t::BinaryOp; }

    /**
     * @brief Returns this instruction's operation type.
     *
     * @return The operation type.
     */
    binary_op_instruction_t op_type() const { return m_type; }

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
    binary_op_instruction_t m_type;
    operand                 m_lhs /**< Left-hand-side operand */;
    operand                 m_rhs /**< Right-hand-side operand */;
    operand                 m_dst /**< Destination operand */;
protected:
    std::ostream& print(std::ostream& os) const override {
        return os << "binop(" << m_type << ") " << m_lhs << ", " << m_rhs << ", " << m_dst;
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
    phi_instruction(std::vector<std::pair<operand, block_index>>&& labels)
      : m_labels(std::move(labels)) {}

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

    /**
     * @brief Returns this instruction's labels.
     *
     * @return The labels.
     */
    const std::vector<std::pair<operand, block_index>>& labels() const { return m_labels; }
private:
    std::vector<std::pair<operand, block_index>> m_labels;
protected:
    std::ostream& print(std::ostream& os) const override {
        os << "phi";
        for (const auto& pair : m_labels) {
            os << ' ' << pair.second << ": " << pair.first;
        }
        return os;
    }
};

} // namespace ir
} // namespace furlang

#endif // FURLANG_IR_INSTRUCTION_HPP
