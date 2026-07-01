#pragma once
#include "token.h"
#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <variant>

namespace cminus {

// ─── Forward declarations ────────────────────────────────────────────────────

struct TypeExpr;
struct Expr;
struct Stmt;
struct Decl;

using TypeExprPtr = std::unique_ptr<TypeExpr>;
using ExprPtr     = std::unique_ptr<Expr>;
using StmtPtr     = std::unique_ptr<Stmt>;
using DeclPtr     = std::unique_ptr<Decl>;

// ─── Type expressions ────────────────────────────────────────────────────────

enum class PrimitiveType {
    Int,    // 64-bit signed integer
    Float,  // 64-bit IEEE 754 double
    Char,   // single byte character
    Void,   // no type (functions)
};

struct TypeExpr {
    enum class Kind {
        Primitive,
        Pointer,        // *T
        MutPointer,     // *mut T
        Array,          // [T; N]
        Slice,          // [T]
        Named,          // MyStruct
        FnPtr,          // fn(T, T) -> T
    };

    Kind                         kind;
    SourceLocation               loc;
    PrimitiveType                primitive{};
    std::string                  name;
    TypeExprPtr                  inner;
    ExprPtr                      array_size;
    std::vector<TypeExprPtr>     fn_params;
    TypeExprPtr                  fn_ret;
};

// ─── Expressions ─────────────────────────────────────────────────────────────

struct Expr {
    enum class Kind {
        IntLit,
        FloatLit,
        BoolLit,
        StringLit,
        CharLit,
        NullLit,
        Ident,
        Unary,
        Binary,
        Call,
        Index,
        Field,
        Arrow,
        Cast,
        Sizeof,
        AddressOf,
        Deref,
        Assign,
        Ternary,
        CompoundLit,
        AsmExpr,
    };

    Kind           kind;
    SourceLocation loc;

    // Literal values
    int64_t     int_val{};
    double      float_val{};
    std::string str_val;
    char        char_val{};
    bool        bool_val{};

    // Ident
    std::string name;

    // Unary / binary
    TokenKind   op{TokenKind::ERROR};
    ExprPtr     lhs;
    ExprPtr     rhs;
    ExprPtr     cond;

    // Call
    ExprPtr                callee;
    std::vector<ExprPtr>   args;

    // Field / index
    ExprPtr     object;
    std::string field;
    ExprPtr     index;

    // Cast / sizeof
    TypeExprPtr type_arg;

    // Compound literal
    struct FieldInit {
        std::string name;
        ExprPtr     value;
    };
    std::string              struct_name;
    std::vector<FieldInit>   fields;

    // Inline asm
    std::string asm_code;
    std::vector<std::pair<std::string, ExprPtr>> asm_outputs;
    std::vector<std::pair<std::string, ExprPtr>> asm_inputs;
    std::vector<std::string>                     asm_clobbers;
};

// ─── Statements ──────────────────────────────────────────────────────────────

struct Stmt {
    enum class Kind {
        Expr,
        Block,
        Let,
        Return,
        If,
        While,
        Loop,
        For,
        Break,
        Continue,
        Asm,
    };

    Kind           kind;
    SourceLocation loc;

    // Expr stmt
    ExprPtr expr;

    // Block
    std::vector<StmtPtr> stmts;

    // Let
    std::string  var_name;
    bool         is_mut{};
    TypeExprPtr  var_type;
    ExprPtr      init;

    // Return
    ExprPtr ret_val;

    // If
    ExprPtr              if_cond;
    StmtPtr              then_block;
    StmtPtr              else_block;

    // While / Loop
    ExprPtr              while_cond;
    StmtPtr              body;

    // For
    std::string          for_var;
    ExprPtr              for_start;
    ExprPtr              for_end;
    ExprPtr              for_step;
    StmtPtr              for_body;

    // Inline asm statement
    std::string                              asm_code;
    std::vector<std::pair<std::string,ExprPtr>> asm_outputs;
    std::vector<std::pair<std::string,ExprPtr>> asm_inputs;
    std::vector<std::string>                 asm_clobbers;
};

// ─── Declarations ────────────────────────────────────────────────────────────

struct Param {
    std::string  name;
    TypeExprPtr  type;
    SourceLocation loc;
};

struct StructField {
    std::string  name;
    TypeExprPtr  type;
    SourceLocation loc;
};

struct Decl {
    enum class Kind {
        Function,
        ExternFunction,
        Struct,
        Global,
        Import,
    };

    Kind           kind;
    SourceLocation loc;
    bool           is_pub = false;

    // Function / ExternFunction
    std::string           fn_name;
    std::vector<Param>    params;
    TypeExprPtr           ret_type;
    StmtPtr               body;
    bool                  is_variadic = false;

    // Struct
    std::string              struct_name;
    std::vector<StructField> struct_fields;

    // Global variable
    std::string  global_name;
    bool         global_mut = false;
    TypeExprPtr  global_type;
    ExprPtr      global_init;

    // Import
    std::string import_path;
    std::string import_alias;
};

// ─── Top-level module ────────────────────────────────────────────────────────

struct Module {
    std::string          name;
    std::vector<DeclPtr> decls;
};

} // namespace cminus
