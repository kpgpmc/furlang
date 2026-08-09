#ifndef FURC_FRONT_AST_HPP
#define FURC_FRONT_AST_HPP

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace furc {

struct ast_type {
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

class ast_node {
public:
    enum category_e {
        Statement,
        Declaration,
        Expression,
        Literal,
    };
public:
    ast_node()          = default;
    virtual ~ast_node() = default;

    ast_node(ast_node&&) noexcept            = default;
    ast_node& operator=(ast_node&&) noexcept = default;

    ast_node(const ast_node&)            = delete;
    ast_node& operator=(const ast_node&) = delete;
public:
    virtual category_e category() const = 0;
};

using ast_node_cat = ast_node::category_e;

class stmt_node : public ast_node {
public:
    enum stmt_type_e {
        Declaration = 0,
        Expression,

        Compound,
        If,
        While,
        Return,
    };
public:
    category_e category() const override { return ast_node_cat::Statement; }

    virtual stmt_type_e stmt_type() const = 0;
};

struct comp_stmt_node final : public stmt_node {
    stmt_type_e stmt_type() const override { return Compound; }

    std::vector<stmt_node*> stmts;
};

class expr_node;

struct if_stmt_node final : public stmt_node {
    stmt_type_e stmt_type() const override { return If; }

    expr_node* cond       = nullptr;
    stmt_node* thenBranch = nullptr;
    stmt_node* elseBranch = nullptr;
};

struct while_stmt_node final : public stmt_node {
    stmt_type_e stmt_type() const override { return While; }

    expr_node* cond = nullptr;
    stmt_node* body = nullptr;
};

struct return_stmt_node final : public stmt_node {
    stmt_type_e stmt_type() const override { return Return; }

    expr_node* value = nullptr;
};

class decl_node : public stmt_node {
public:
    enum decl_type_e {
        Variable,
        Function
    };
public:
    category_e category() const override { return ast_node_cat::Declaration; }

    stmt_type_e stmt_type() const override { return Declaration; }

    virtual decl_type_e decl_type() const = 0;
};

struct var_decl_node final : public decl_node {
    decl_type_e decl_type() const override { return Variable; }

    std::string name;
    ast_type    type;
    expr_node*  init = nullptr;
};

struct func_decl_node final : public decl_node {
    decl_type_e decl_type() const override { return Function; }

    struct def_s {
        comp_stmt_node          body;
        std::vector<expr_node*> preConds;
        std::vector<expr_node*> postConds;
    };

    std::string                name;
    ast_type                   type;
    std::vector<var_decl_node> params;
    std::optional<def_s>       def;
};

class expr_node : public stmt_node {
public:
    enum expr_type_e {
        Literal,

        VarRead,
        Group,
        BinaryOp,
        UnaryOp,
        If,
    };
public:
    category_e category() const override { return ast_node_cat::Expression; }

    stmt_type_e stmt_type() const override { return Expression; }

    virtual expr_type_e expr_type() const = 0;
};

struct var_read_expr_node final : public expr_node {
    expr_type_e expr_type() const override { return VarRead; }

    std::string name;

    var_read_expr_node(std::string&& name)
      : name(std::move(name)) {}
};

struct group_expr_node final : public expr_node {
    expr_type_e expr_type() const override { return Group; }

    expr_node* inner = nullptr;
};

struct binary_op_expr_node final : public expr_node {
    enum binary_op_type {
        Add = 0,
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

        Equals,
        NotEquals,
        LessThan,
        LessEquals,
        GreaterThan,
        GreaterEquals,
    };

    expr_type_e expr_type() const override { return BinaryOp; }

    expr_node*     lhs  = nullptr;
    expr_node*     rhs  = nullptr;
    binary_op_type type = Add;
};

struct unary_op_expr_node final : public expr_node {
    enum unary_op_type {
        Positive = 0,
        Negative,
        PreInc,
        PreDec,
        PostInc,
        PostDec,
        BinNot,
        Not,

        Sizeof,
        Pointerof,
        Lengthof,
    };

    expr_type_e expr_type() const override { return UnaryOp; }

    expr_node*    lhs  = nullptr;
    unary_op_type type = Positive;
};

struct if_expr_node final : public expr_node {
    expr_type_e expr_type() const override { return If; }

    expr_node* cond     = nullptr;
    expr_node* thenExpr = nullptr;
    expr_node* elseExpr = nullptr;
};

class lit_node : public expr_node {
public:
    enum lit_type_e {
        Integer,
        Char,
    };
public:
    category_e category() const override { return ast_node_cat::Literal; }

    expr_type_e expr_type() const override { return Literal; }

    virtual lit_type_e lit_type() const = 0;
};

struct int_lit_node final : public lit_node {
    int_lit_node(std::uint64_t value)
      : value(value) {}

    lit_type_e lit_type() const override { return Integer; }

    std::uint64_t value;
};

struct char_lit_node final : public lit_node {
    char_lit_node(char value)
      : value(value) {}

    lit_type_e lit_type() const override { return Char; }

    char value;
};

struct ast {
    std::vector<decl_node*> decls;
};

} // namespace furc

#endif // FURC_FRONT_AST_HPP
