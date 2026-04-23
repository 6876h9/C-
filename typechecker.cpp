#include "typechecker.h"
#include <cstdio>
#include <cassert>
#include <cstring>

namespace cminus {

bool Type::is_integer() const {
    if (kind != Kind::Primitive) return false;
    switch (primitive) {
        case PrimitiveType::I8:  case PrimitiveType::I16:
        case PrimitiveType::I32: case PrimitiveType::I64:
        case PrimitiveType::U8:  case PrimitiveType::U16:
        case PrimitiveType::U32: case PrimitiveType::U64:
        case PrimitiveType::Char:
            return true;
        default: return false;
    }
}

bool Type::is_float() const {
    if (kind != Kind::Primitive) return false;
    return primitive == PrimitiveType::F32 || primitive == PrimitiveType::F64;
}

bool Type::is_numeric() const { return is_integer() || is_float(); }

bool Type::is_signed() const {
    if (kind != Kind::Primitive) return false;
    switch (primitive) {
        case PrimitiveType::I8:  case PrimitiveType::I16:
        case PrimitiveType::I32: case PrimitiveType::I64:
        case PrimitiveType::F32: case PrimitiveType::F64:
            return true;
        default: return false;
    }
}

bool Type::is_pointer() const {
    return kind == Kind::Pointer || kind == Kind::MutPointer;
}

bool Type::is_void() const {
    return kind == Kind::Primitive && primitive == PrimitiveType::Void;
}

uint32_t Type::size_bytes() const {
    switch (kind) {
        case Kind::Primitive:
            switch (primitive) {
                case PrimitiveType::I8:   case PrimitiveType::U8:
                case PrimitiveType::Bool: case PrimitiveType::Char: return 1;
                case PrimitiveType::I16:  case PrimitiveType::U16:  return 2;
                case PrimitiveType::I32:  case PrimitiveType::U32:
                case PrimitiveType::F32:                            return 4;
                case PrimitiveType::I64:  case PrimitiveType::U64:
                case PrimitiveType::F64:                            return 8;
                case PrimitiveType::Void:                           return 0;
            }
            break;
        case Kind::Pointer:   case Kind::MutPointer:
        case Kind::FnPtr:                               return 8;
        case Kind::Array:
            return inner ? inner->size_bytes() * (uint32_t)array_len : 0;
        case Kind::Struct:    return 0;
        default:              return 0;
    }
    return 0;
}

uint32_t Type::align_bytes() const {
    uint32_t s = size_bytes();
    if (s == 0) return 1;
    if (s >= 8) return 8;
    if (s >= 4) return 4;
    if (s >= 2) return 2;
    return 1;
}

bool Type::operator==(const Type& o) const {
    if (kind != o.kind) return false;
    switch (kind) {
        case Kind::Primitive:  return primitive == o.primitive;
        case Kind::Pointer:
        case Kind::MutPointer: return inner && o.inner && *inner == *o.inner;
        case Kind::Struct:     return struct_name == o.struct_name;
        case Kind::Array:      return array_len == o.array_len &&
                                      inner && o.inner && *inner == *o.inner;
        case Kind::Void:       return true;
        case Kind::Error:      return true;
        default:               return false;
    }
}

std::string Type::to_string() const {
    switch (kind) {
        case Kind::Primitive:
            switch (primitive) {
                case PrimitiveType::I8:   return "i8";
                case PrimitiveType::I16:  return "i16";
                case PrimitiveType::I32:  return "i32";
                case PrimitiveType::I64:  return "i64";
                case PrimitiveType::U8:   return "u8";
                case PrimitiveType::U16:  return "u16";
                case PrimitiveType::U32:  return "u32";
                case PrimitiveType::U64:  return "u64";
                case PrimitiveType::F32:  return "f32";
                case PrimitiveType::F64:  return "f64";
                case PrimitiveType::Bool: return "bool";
                case PrimitiveType::Void: return "void";
                case PrimitiveType::Char: return "char";
            }
            break;
        case Kind::Pointer:    return "*" + (inner ? inner->to_string() : "?");
        case Kind::MutPointer: return "*mut " + (inner ? inner->to_string() : "?");
        case Kind::Array:      return "[" + (inner ? inner->to_string() : "?") + "; " + std::to_string(array_len) + "]";
        case Kind::Struct:     return struct_name;
        case Kind::Void:       return "void";
        case Kind::Error:      return "<error>";
        default:               return "?";
    }
    return "?";
}

TypePtr Type::make_prim(PrimitiveType p) {
    auto t = std::make_shared<Type>();
    t->kind      = Kind::Primitive;
    t->primitive = p;
    return t;
}

TypePtr Type::make_ptr(TypePtr inner, bool is_mut) {
    auto t = std::make_shared<Type>();
    t->kind  = is_mut ? Kind::MutPointer : Kind::Pointer;
    t->inner = std::move(inner);
    return t;
}

TypePtr Type::make_void() {
    auto t = std::make_shared<Type>();
    t->kind      = Kind::Primitive;
    t->primitive = PrimitiveType::Void;
    return t;
}

TypePtr Type::make_error() {
    auto t = std::make_shared<Type>();
    t->kind = Kind::Error;
    return t;
}

void Scope::push() { m_frames.emplace_back(); }
void Scope::pop()  { if (!m_frames.empty()) m_frames.pop_back(); }

void Scope::define(const std::string& name, Symbol sym) {
    if (!m_frames.empty())
        m_frames.back()[name] = std::move(sym);
}

Symbol* Scope::lookup(const std::string& name) {
    for (int i = (int)m_frames.size() - 1; i >= 0; --i) {
        auto it = m_frames[i].find(name);
        if (it != m_frames[i].end()) return &it->second;
    }
    return nullptr;
}

TypeChecker::TypeChecker() {}

bool TypeChecker::check(Module& mod) {
    for (auto& d : mod.decls) {
        if (!d) continue;
        switch (d->kind) {
            case Decl::Kind::Function:
            case Decl::Kind::ExternFunction: {
                Symbol sym;
                sym.kind        = Symbol::Kind::Function;
                sym.is_variadic = d->is_variadic;
                sym.ret_type    = resolve_type(*d->ret_type);
                for (auto& p : d->params)
                    sym.param_types.push_back(resolve_type(*p.type));
                m_globals[d->fn_name] = std::move(sym);
                break;
            }
            case Decl::Kind::Struct: {
                Symbol sym;
                sym.kind = Symbol::Kind::StructType;
                uint32_t offset = 0;
                for (auto& f : d->struct_fields) {
                    auto ft = resolve_type(*f.type);
                    uint32_t align = ft->align_bytes();
                    offset = (offset + align - 1) & ~(align - 1);
                    sym.fields.push_back({f.name, ft, offset});
                    offset += ft->size_bytes();
                }
                sym.struct_size = offset;
                m_globals[d->struct_name] = std::move(sym);
                break;
            }
            case Decl::Kind::Global: {
                Symbol sym;
                sym.kind   = Symbol::Kind::GlobalVar;
                sym.is_mut = d->global_mut;
                sym.type   = d->global_type ? resolve_type(*d->global_type) : Type::make_error();
                m_globals[d->global_name] = std::move(sym);
                break;
            }
            default: break;
        }
    }

    for (auto& d : mod.decls) {
        if (!d) continue;
        check_decl(*d);
    }
    return !m_has_errors;
}

void TypeChecker::check_decl(Decl& d) {
    switch (d.kind) {
        case Decl::Kind::Function:     check_fn(d);     break;
        case Decl::Kind::Struct:       check_struct(d); break;
        case Decl::Kind::Global:       check_global(d); break;
        case Decl::Kind::ExternFunction:
        case Decl::Kind::Import:
            break;
    }
}

void TypeChecker::check_fn(Decl& d) {
    m_scope.push();
    m_current_ret_type = resolve_type(*d.ret_type);

    for (auto& p : d.params) {
        Symbol sym;
        sym.kind   = Symbol::Kind::LocalVar;
        sym.type   = resolve_type(*p.type);
        sym.is_mut = true;
        m_scope.define(p.name, std::move(sym));
    }

    if (d.body) check_block(*d.body);

    m_scope.pop();
}

void TypeChecker::check_struct(Decl&) {}

void TypeChecker::check_global(Decl& d) {
    if (!d.global_init) return;
    auto init_ty = check_expr(*d.global_init);
    if (d.global_type) {
        auto decl_ty = resolve_type(*d.global_type);
        if (!assignable(decl_ty, init_ty))
            error(d.loc, "global variable initializer type mismatch");
    }
}

TypePtr TypeChecker::check_block(Stmt& s) {
    assert(s.kind == Stmt::Kind::Block);
    m_scope.push();
    TypePtr last = Type::make_void();
    for (auto& child : s.stmts)
        last = check_stmt(*child);
    m_scope.pop();
    return last;
}

TypePtr TypeChecker::check_stmt(Stmt& s) {
    switch (s.kind) {
        case Stmt::Kind::Block:    return check_block(s);
        case Stmt::Kind::Let:      return check_let(s);
        case Stmt::Kind::Return:   return check_return(s);
        case Stmt::Kind::If:       return check_if(s);
        case Stmt::Kind::While:    return check_while(s);
        case Stmt::Kind::For:      return check_for(s);
        case Stmt::Kind::Expr:
            if (s.expr) return check_expr(*s.expr);
            return Type::make_void();
        case Stmt::Kind::Break:
        case Stmt::Kind::Continue:
        case Stmt::Kind::Loop:
            if (s.body) check_stmt(*s.body);
            return Type::make_void();
        case Stmt::Kind::Asm:      return Type::make_void();
        default:                   return Type::make_void();
    }
}

TypePtr TypeChecker::check_let(Stmt& s) {
    TypePtr ty;
    if (s.var_type) ty = resolve_type(*s.var_type);

    if (s.init) {
        auto init_ty = check_expr(*s.init);
        if (ty) {
            if (!assignable(ty, init_ty))
                error(s.loc, "type mismatch in 'let'");
        } else {
            ty = init_ty;
        }
    }

    if (!ty) ty = Type::make_error();

    Symbol sym;
    sym.kind   = Symbol::Kind::LocalVar;
    sym.type   = ty;
    sym.is_mut = s.is_mut;
    m_scope.define(s.var_name, std::move(sym));
    return Type::make_void();
}

TypePtr TypeChecker::check_return(Stmt& s) {
    TypePtr ret_ty = s.ret_val ? check_expr(*s.ret_val) : Type::make_void();
    if (m_current_ret_type && !assignable(m_current_ret_type, ret_ty))
        error(s.loc, "return type mismatch");
    return ret_ty;
}

TypePtr TypeChecker::check_if(Stmt& s) {
    auto cond_ty = check_expr(*s.if_cond);
    if (!cond_ty->is_integer() && cond_ty->kind != Type::Kind::Primitive)
        error(s.loc, "if condition must be boolean or integer");
    if (s.then_block) check_stmt(*s.then_block);
    if (s.else_block) check_stmt(*s.else_block);
    return Type::make_void();
}

TypePtr TypeChecker::check_while(Stmt& s) {
    check_expr(*s.while_cond);
    if (s.body) check_stmt(*s.body);
    return Type::make_void();
}

TypePtr TypeChecker::check_for(Stmt& s) {
    auto start_ty = check_expr(*s.for_start);
    auto end_ty   = check_expr(*s.for_end);
    if (!start_ty->is_integer())
        error(s.loc, "for range must be integer");

    Symbol sym;
    sym.kind   = Symbol::Kind::LocalVar;
    sym.type   = start_ty;
    sym.is_mut = false;
    m_scope.push();
    m_scope.define(s.for_var, std::move(sym));
    if (s.for_body) check_stmt(*s.for_body);
    m_scope.pop();
    return Type::make_void();
}

TypePtr TypeChecker::check_expr(Expr& e) {
    switch (e.kind) {
        case Expr::Kind::IntLit:
            return Type::make_prim(PrimitiveType::I32);
        case Expr::Kind::FloatLit:
            return Type::make_prim(PrimitiveType::F64);
        case Expr::Kind::BoolLit:
            return Type::make_prim(PrimitiveType::Bool);
        case Expr::Kind::CharLit:
            return Type::make_prim(PrimitiveType::Char);
        case Expr::Kind::StringLit: {
            auto inner = Type::make_prim(PrimitiveType::U8);
            return Type::make_ptr(inner, false);
        }
        case Expr::Kind::NullLit: {
            auto inner = Type::make_prim(PrimitiveType::Void);
            return Type::make_ptr(inner, false);
        }
        case Expr::Kind::Ident: {
            Symbol* sym = m_scope.lookup(e.name);
            if (!sym) {
                auto it = m_globals.find(e.name);
                if (it != m_globals.end()) sym = &it->second;
            }
            if (!sym) {
                error(e.loc, "undefined symbol");
                return Type::make_error();
            }
            if (sym->kind == Symbol::Kind::Function) {
                auto t = std::make_shared<Type>();
                t->kind       = Type::Kind::FnPtr;
                t->fn_params  = sym->param_types;
                t->fn_ret     = sym->ret_type;
                return t;
            }
            return sym->type;
        }
        case Expr::Kind::Binary:    return check_binary(e);
        case Expr::Kind::Unary:     return check_unary(e);
        case Expr::Kind::Call:      return check_call(e);
        case Expr::Kind::Cast:      return check_cast(e);
        case Expr::Kind::Field:
        case Expr::Kind::Arrow:     return check_field(e);
        case Expr::Kind::Index:     return check_index(e);
        case Expr::Kind::AddressOf: {
            auto inner = check_expr(*e.lhs);
            return Type::make_ptr(inner, false);
        }
        case Expr::Kind::Deref: {
            auto pt = check_expr(*e.lhs);
            if (!pt->is_pointer()) {
                error(e.loc, "cannot dereference non-pointer type");
                return Type::make_error();
            }
            return pt->inner ? pt->inner : Type::make_error();
        }
        case Expr::Kind::Assign: {
            auto lhs = check_expr(*e.lhs);
            auto rhs = check_expr(*e.rhs);
            if (!assignable(lhs, rhs))
                error(e.loc, "assignment type mismatch");
            return lhs;
        }
        case Expr::Kind::Sizeof:
            return Type::make_prim(PrimitiveType::U64);
        case Expr::Kind::AsmExpr:
            return Type::make_prim(PrimitiveType::I64);
        case Expr::Kind::CompoundLit: {
            auto it = m_globals.find(e.struct_name);
            if (it == m_globals.end() || it->second.kind != Symbol::Kind::StructType) {
                error(e.loc, "unknown struct");
                return Type::make_error();
            }
            auto t = std::make_shared<Type>();
            t->kind        = Type::Kind::Struct;
            t->struct_name = e.struct_name;
            return t;
        }
        default:
            return Type::make_error();
    }
}

TypePtr TypeChecker::check_binary(Expr& e) {
    auto lhs = check_expr(*e.lhs);
    auto rhs = check_expr(*e.rhs);

    switch (e.op) {
        case TokenKind::PLUS:
        case TokenKind::MINUS:
        case TokenKind::STAR:
        case TokenKind::SLASH:
        case TokenKind::PERCENT:
        case TokenKind::AMPERSAND:
        case TokenKind::PIPE:
        case TokenKind::CARET:
        case TokenKind::LSHIFT:
        case TokenKind::RSHIFT:
            if (!lhs->is_numeric() && !lhs->is_pointer())
                error(e.loc, "arithmetic operand must be numeric");
            return lhs;

        case TokenKind::EQ_EQ:
        case TokenKind::BANG_EQ:
        case TokenKind::LT:
        case TokenKind::LT_EQ:
        case TokenKind::GT:
        case TokenKind::GT_EQ:
            return Type::make_prim(PrimitiveType::Bool);

        case TokenKind::AND_AND:
        case TokenKind::OR_OR:
            return Type::make_prim(PrimitiveType::Bool);

        default:
            return lhs;
    }
}

TypePtr TypeChecker::check_unary(Expr& e) {
    auto operand = check_expr(*e.lhs);
    switch (e.op) {
        case TokenKind::BANG:  return Type::make_prim(PrimitiveType::Bool);
        case TokenKind::MINUS: return operand;
        case TokenKind::TILDE: return operand;
        default:               return operand;
    }
}

TypePtr TypeChecker::check_call(Expr& e) {
    auto callee_ty = check_expr(*e.callee);
    for (auto& arg : e.args) check_expr(*arg);

    if (callee_ty->kind == Type::Kind::FnPtr)
        return callee_ty->fn_ret ? callee_ty->fn_ret : Type::make_void();

    return Type::make_void();
}

TypePtr TypeChecker::check_cast(Expr& e) {
    check_expr(*e.lhs);
    return resolve_type(*e.type_arg);
}

TypePtr TypeChecker::check_field(Expr& e) {
    auto obj_ty = check_expr(*e.object);
    TypePtr struct_ty = obj_ty;
    if (e.kind == Expr::Kind::Arrow) {
        if (!obj_ty->is_pointer()) {
            error(e.loc, "'->' requires pointer type");
            return Type::make_error();
        }
        struct_ty = obj_ty->inner;
    }
    if (!struct_ty || struct_ty->kind != Type::Kind::Struct) {
        error(e.loc, "field access on non-struct type");
        return Type::make_error();
    }
    auto it = m_globals.find(struct_ty->struct_name);
    if (it == m_globals.end()) return Type::make_error();
    for (auto& f : it->second.fields)
        if (f.name == e.field) return f.type;
    error(e.loc, "struct has no field");
    return Type::make_error();
}

TypePtr TypeChecker::check_index(Expr& e) {
    auto obj_ty = check_expr(*e.object);
    check_expr(*e.index);
    if (obj_ty->kind == Type::Kind::Array || obj_ty->kind == Type::Kind::Slice)
        return obj_ty->inner ? obj_ty->inner : Type::make_error();
    if (obj_ty->is_pointer())
        return obj_ty->inner ? obj_ty->inner : Type::make_error();
    error(e.loc, "indexing non-array type");
    return Type::make_error();
}

TypePtr TypeChecker::resolve_type(const TypeExpr& te) {
    switch (te.kind) {
        case TypeExpr::Kind::Primitive:
            return Type::make_prim(te.primitive);
        case TypeExpr::Kind::Pointer:
            return Type::make_ptr(resolve_type(*te.inner), false);
        case TypeExpr::Kind::MutPointer:
            return Type::make_ptr(resolve_type(*te.inner), true);
        case TypeExpr::Kind::Named: {
            auto t = std::make_shared<Type>();
            t->kind        = Type::Kind::Struct;
            t->struct_name = te.name;
            return t;
        }
        case TypeExpr::Kind::Array: {
            auto t       = std::make_shared<Type>();
            t->kind      = Type::Kind::Array;
            t->inner     = resolve_type(*te.inner);
            t->array_len = 0;
            return t;
        }
        case TypeExpr::Kind::Slice: {
            auto t   = std::make_shared<Type>();
            t->kind  = Type::Kind::Slice;
            t->inner = resolve_type(*te.inner);
            return t;
        }
        case TypeExpr::Kind::FnPtr: {
            auto t      = std::make_shared<Type>();
            t->kind     = Type::Kind::FnPtr;
            for (auto& p : te.fn_params)
                t->fn_params.push_back(resolve_type(*p));
            t->fn_ret = te.fn_ret ? resolve_type(*te.fn_ret) : Type::make_void();
            return t;
        }
        default:
            return Type::make_error();
    }
}

bool TypeChecker::assignable(const TypePtr& dst, const TypePtr& src) {
    if (!dst || !src) return false;
    if (dst->kind == Type::Kind::Error || src->kind == Type::Kind::Error) return true;
    if (*dst == *src) return true;
    if (dst->is_numeric() && src->is_numeric()) return true;
    if (dst->is_pointer() && src->is_pointer() &&
        src->inner && src->inner->is_void()) return true;
    return false;
}

void TypeChecker::error(const SourceLocation& loc, const std::string& msg) {
    m_has_errors = true;
    fprintf(stderr, "%s:%u:%u: type error: %s\n",
        loc.file, loc.line, loc.col, msg.c_str());
}

} // namespace cminus
