#pragma once
#include "ast.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

namespace cminus {

struct Type {
    enum class Kind {
        Primitive,
        Pointer,
        MutPointer,
        Array,
        Slice,
        Struct,
        FnPtr,
        Void,
        Error,
    };

    Kind kind = Kind::Error;
    PrimitiveType primitive{};
    std::shared_ptr<Type> inner;
    uint64_t              array_len = 0;
    std::string struct_name;
    std::vector<std::shared_ptr<Type>> fn_params;
    std::shared_ptr<Type>              fn_ret;

    bool is_integer()   const;
    bool is_float()     const;
    bool is_numeric()   const;
    bool is_signed()    const;
    bool is_pointer()   const;
    bool is_void()      const;
    uint32_t size_bytes() const;
    uint32_t align_bytes() const;
    bool operator==(const Type& o) const;
    bool operator!=(const Type& o) const { return !(*this == o); }
    std::string to_string() const;

    static std::shared_ptr<Type> make_prim(PrimitiveType p);
    static std::shared_ptr<Type> make_ptr(std::shared_ptr<Type> inner, bool is_mut);
    static std::shared_ptr<Type> make_void();
    static std::shared_ptr<Type> make_error();
};

using TypePtr = std::shared_ptr<Type>;

struct Symbol {
    enum class Kind { LocalVar, GlobalVar, Function, StructType };
    Kind     kind;
    TypePtr  type;
    bool     is_mut = false;

    std::vector<TypePtr> param_types;
    TypePtr              ret_type;
    bool                 is_variadic = false;

    struct Field { std::string name; TypePtr type; uint32_t offset; };
    std::vector<Field> fields;
    uint32_t           struct_size = 0;
};

class Scope {
public:
    void push();
    void pop();
    void define(const std::string& name, Symbol sym);
    Symbol* lookup(const std::string& name);

private:
    std::vector<std::unordered_map<std::string, Symbol>> m_frames;
};

class TypeChecker {
public:
    explicit TypeChecker();
    bool check(Module& mod);
    bool has_errors() const { return m_has_errors; }

private:
    void check_decl(Decl& d);
    void check_fn(Decl& d);
    void check_struct(Decl& d);
    void check_global(Decl& d);
    TypePtr check_stmt(Stmt& s);
    TypePtr check_block(Stmt& s);
    TypePtr check_let(Stmt& s);
    TypePtr check_return(Stmt& s);
    TypePtr check_if(Stmt& s);
    TypePtr check_while(Stmt& s);
    TypePtr check_for(Stmt& s);
    TypePtr check_expr(Expr& e);
    TypePtr check_binary(Expr& e);
    TypePtr check_unary(Expr& e);
    TypePtr check_call(Expr& e);
    TypePtr check_cast(Expr& e);
    TypePtr check_field(Expr& e);
    TypePtr check_index(Expr& e);
    TypePtr resolve_type(const TypeExpr& te);
    bool assignable(const TypePtr& dst, const TypePtr& src);
    void error(const SourceLocation& loc, const std::string& msg);

    Scope         m_scope;
    TypePtr       m_current_ret_type;
    bool          m_has_errors = false;
    std::unordered_map<std::string, Symbol> m_globals;
};

} // namespace cminus
