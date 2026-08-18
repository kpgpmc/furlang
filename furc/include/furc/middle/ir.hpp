#ifndef FURC_MIDDLE_IR_HPP
#define FURC_MIDDLE_IR_HPP

#include "furc/front/ast.hpp"
#include "furlang/arena.hpp"

#include <algorithm>
#include <cstdint>
#include <initializer_list>
#include <optional>
#include <stack>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace furc {

struct ir_operand {
    enum type_e {
        Integer = 0,
        Register,
        Variable,
        Global,
        Function,
        Block,
        BlockPair,
        PhiPair,
    } type;
    union value_u {
        std::uint64_t integer;
        struct register_s {
            std::uint64_t name : 54;
            std::uint64_t ver : 10;
        } reg;
        std::uint16_t variable;
        std::uint16_t global;
        std::uint64_t function;
        std::uint64_t block;
        struct block_pair_s {
            std::uint64_t first;
            std::uint64_t second;
        } blockPair;
        struct phi_pair_s {
            register_s    reg;
            std::uint64_t block;
        } phiPair;

        value_u() = default;

        value_u(std::uint64_t integer)
          : integer(integer) {}

        value_u(std::uint16_t variable)
          : variable(variable) {}

        value_u(std::uint64_t first, std::uint64_t second)
          : blockPair({ first, second }) {}

        value_u(register_s reg, std::uint64_t block)
          : phiPair({ reg, block }) {}
    } value;

    template <typename... Args, typename = std::enable_if_t<std::is_constructible_v<value_u, Args...>>>
    ir_operand(type_e type, Args&&... args)
      : type(type), value(std::forward<Args>(args)...) {}
};

struct ir_type {
    enum type_e {
        Void = 0,
        S8,
        U8,
        S16,
        U16,
        S32,
        U32,
        S64,
        U64,
    } type = Void;
};

// TODO: Add data types to instructions (like mov QWORD ... in x86 assembly)
struct ir_instruction {
    enum type_e {
        Move = 0,
        Call,
        Branch,
        BranchCond,
        Return,
        Phi,

        Add,
        Sub,
        Mul,
        Div,
        Mod,

        Shl,
        Shr,
        BinAnd,
        BinOr,
        BinXor,
        And,
        Or,

        Eq,
        NotEq,
        LessThan,
        LessEq,
        GreaterThan,
        GreaterEq,

        Positive,
        Negative,
        Increment,
        Decrement,
        BinNot,
        Not,

        Sizeof,
        Pointerof,
        Lenof,
    } type;
    std::optional<ir_operand> destination;
    std::vector<ir_operand>   sources;

    ir_instruction(type_e                 type,
        std::optional<ir_operand>         destination = {},
        std::initializer_list<ir_operand> sources     = {})
      : type(type), destination(destination), sources(sources) {}

    static constexpr bool is_terminating(type_e type) {
        switch (type) {
        case Branch:
        case BranchCond:
        case Return: return true;
        default: return false;
        }
    }
};

struct ir_basic_block {
    std::vector<ir_instruction> instructions;

    bool is_terminated() const {
        return !instructions.empty() && ir_instruction::is_terminating(instructions.back().type);
    }
};

struct ir_variable {
    ir_variable() = default;

    ir_variable(ir_type type)
      : type(type) {}

    virtual ~ir_variable() = default;

    ir_variable(ir_variable&&) noexcept            = default;
    ir_variable& operator=(ir_variable&&) noexcept = default;

    ir_variable(const ir_variable&)            = default;
    ir_variable& operator=(const ir_variable&) = default;

    ir_type type;

    virtual ir_operand operand() const = 0;
};

struct ir_module_variable : ir_variable {
    ir_module_variable(ir_type type, std::uint16_t name)
      : ir_variable(type), name(name) {}

    std::uint16_t name;

    ir_operand operand() const final { return { ir_operand::Global, name }; }
};

struct ir_function_variable : ir_variable {
    ir_function_variable(ir_type type, std::uint64_t name)
      : ir_variable(type), name(name) {}

    std::uint64_t name;

    ir_operand operand() const final { return { ir_operand::Variable, name }; }
};

struct ir_scope {
    ir_scope()          = default;
    virtual ~ir_scope() = default;

    ir_scope(ir_scope&&) noexcept            = default;
    ir_scope& operator=(ir_scope&&) noexcept = default;

    ir_scope(const ir_scope&)            = default;
    ir_scope& operator=(const ir_scope&) = default;

    ir_scope* previous = nullptr;

    std::unordered_map<std::string, ir_variable*> variables;

    const ir_variable* variable(const std::string& name) const {
        if (auto it = variables.find(name); it != variables.end()) return it->second;
        return (previous != nullptr) ? previous->variable(name) : nullptr;
    }

    virtual const ir_variable* allocate(furlang::arena& arena, const std::string& name, ir_type type) = 0;
};

struct ir_function : ir_scope {
    enum type_e {
        Normal = 0,
        Import,
        Native,
    } type = Normal;
    enum access_e {
        Public = 0,
        Private,
    } access = Public;

    std::string                 name;
    std::vector<ir_type>        params;
    ir_type                     retType;
    std::vector<ir_basic_block> blocks;

    std::uint64_t regCount = 0;

    const ir_variable* allocate(furlang::arena& arena, const std::string& name, ir_type type) final {
        return variables[name] = arena.allocate<ir_function_variable>(type, regCount++);
    }

    static ir_function from_name(std::string&& name) {
        ir_function func;
        func.name = std::move(name);
        return func;
    }
};

struct ir_module : ir_scope {
    std::vector<ir_function*> functions;
    furlang::arena            arena;

    std::uint16_t varCount = 0;

    const ir_variable* allocate(furlang::arena& arena, const std::string& name, ir_type type) final {
        return variables[name] = arena.allocate<ir_module_variable>(type, varCount);
    }

    ir_function* add_function(ir_function&& function) {
        return functions.emplace_back(arena.allocate<ir_function>(std::move(function)));
    }
};

struct ir_context {
    ir_context(ir_function* function)
      : function(function) {
        if (function->blocks.empty()) new_last();
        blockPtr = &function->blocks.front();
    }

    ~ir_context() {
        if (blockPtr == nullptr) return;
        if (!blockPtr->is_terminated()) {
            if (blockIdx + 1 == function->blocks.size()) {
                add_instr(ir_instruction::Return);
            } else {
                add_instr(ir_instruction::Branch, ir_operand{ ir_operand::Block, blockIdx + 1 });
            }
        }
    }

    ir_context(ir_context&& other) noexcept
      : function(other.function), blockIdx(other.blockIdx), blockPtr(other.blockPtr) {
        other.function = nullptr;
        other.blockIdx = 0;
        other.blockPtr = nullptr;
    }

    ir_context& operator=(ir_context&& other) noexcept {
        if (this == &other) return *this;
        function = other.function;
        blockIdx = other.blockIdx;
        blockPtr = other.blockPtr;

        other.function = nullptr;
        other.blockIdx = 0;
        other.blockPtr = nullptr;
        return *this;
    }

    ir_context(const ir_context&)            = delete;
    ir_context& operator=(const ir_context&) = delete;

    template <typename... Args, typename = std::enable_if_t<std::is_constructible_v<ir_instruction, Args...>>>
    ir_instruction& add_instr(Args&&... args) {
        auto it = blockPtr->instructions.end();
        if (!blockPtr->instructions.empty() && ir_instruction::is_terminating(blockPtr->instructions.back().type)) --it;
        it = blockPtr->instructions.emplace(it, std::forward<Args>(args)...);
        if (ir_instruction::is_terminating(it->type) && it + 1 != blockPtr->instructions.end())
            blockPtr->instructions.pop_back();
        return *it;
    }

    void terminate() { add_instr(ir_instruction::Return); }

    void terminate(ir_operand value) {
        ir_instruction instr = { ir_instruction::Return };
        instr.sources.emplace_back(value);
        add_instr(std::move(instr));
    }

    void terminate(std::uint64_t block) { add_instr(ir_instruction::Branch, ir_operand{ ir_operand::Block, block }); }

    ir_instruction* terminate(ir_operand cond, std::uint64_t thenBranch, std::uint64_t elseBranch) {
        return &add_instr(ir_instruction{ ir_instruction::BranchCond,
            ir_operand{ ir_operand::BlockPair, thenBranch, elseBranch },
            { cond } });
    }

    ir_context& new_next() {
        if (blockPtr->instructions.empty()) return *this;
        auto it = function->blocks.begin() + static_cast<std::ptrdiff_t>(++blockIdx);
        if (!blockPtr->is_terminated()) terminate(blockIdx);
        blockPtr = &*function->blocks.emplace(it);
        return *this;
    }

    ir_context& new_last() {
        blockIdx = function->blocks.size();
        blockPtr = &*function->blocks.emplace(function->blocks.end());
        return *this;
    }

    ir_context& go(std::uint64_t block) {
        blockIdx = std::min(block, function->blocks.size() - 1);
        blockPtr = function->blocks.data() + static_cast<std::ptrdiff_t>(blockIdx);
        return *this;
    }

    ir_context& go_next() { return go(blockIdx + 1); }
    ir_context& go_last() { return go(std::min<std::uint64_t>(0, function->blocks.size() - 1)); }

    ir_operand last_register() const { return { ir_operand::Register, function->regCount - 1 }; }
    ir_operand next_register() const { return { ir_operand::Register, function->regCount++ }; }

    static ir_operand block_op(std::uint64_t blockIdx) { return { ir_operand::Block, blockIdx }; }

    ir_function*    function = nullptr;
    std::uint64_t   blockIdx = 0;
    ir_basic_block* blockPtr = nullptr;
};

class ir_generator final : public ast_visitor {
public:
    ir_generator()
      : m_initContext(m_module.add_function(ir_function::from_name("module$init"))) {}

    void finalize() {
        m_module.functions.front()->blocks.emplace_back().instructions.push_back(
            ir_instruction{ ir_instruction::Return });
    }

    ir_module build() {
        m_scope                = nullptr;
        m_context              = {};
        m_initContext.blockPtr = nullptr;
        return std::move(m_module);
    }

    static ir_module generate(const ast_node& node) {
        ir_generator gen;
        node.accept(gen);
        gen.finalize();
        return gen.build();
    }

    static ir_module generate(const ast& tree) {
        ir_generator gen;
        for (const auto& node : tree.decls)
            node->accept(gen);
        gen.finalize();
        return gen.build();
    }
private:
    void visit_comp_stmt_node(const comp_stmt_node& node) override;
    void visit_if_stmt_node(const if_stmt_node& node) override;
    void visit_while_stmt_node(const while_stmt_node& node) override;
    void visit_return_stmt_node(const return_stmt_node& node) override;
    void visit_var_decl_node(const var_decl_node& node) override;
    void visit_func_decl_node(const func_decl_node& node) override;
    void visit_var_read_expr_node(const var_read_expr_node& node) override;
    void visit_func_call_expr_node(const func_call_expr_node& node) override;
    void visit_group_expr_node(const group_expr_node& node) override;
    void visit_binary_op_expr_node(const binary_op_expr_node& node) override;
    void visit_unary_op_expr_node(const unary_op_expr_node& node) override;
    void visit_if_expr_node(const if_expr_node& node) override;
    void visit_int_lit_node(const int_lit_node& node) override;
    void visit_char_lit_node(const char_lit_node& node) override;
private:
    ir_context& context() { return m_context.top(); }
private:
    ir_module              m_module;
    ir_scope*              m_scope = &m_module;
    std::stack<ir_context> m_context;

    ir_context m_initContext;
};

} // namespace furc

#endif // FURC_MIDDLE_IR_HPP
