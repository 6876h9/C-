#include "codegen.h"
#include <cstdio>
#include <cstdarg>

namespace cminus {

const char* reg_name64(Reg r) {
    const char* names[] = { "rax", "rcx", "rdx", "rbx", "rsi", "rdi",
                            "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15",
                            "rsp", "rbp" };
    if ((int)r < 16) return names[(int)r];
    return "?";
}

const char* reg_name32(Reg r) {
    const char* names[] = { "eax", "ecx", "edx", "ebx", "esi", "edi",
                            "r8d", "r9d", "r10d","r11d","r12d","r13d","r14d","r15d",
                            "esp", "ebp" };
    if ((int)r < 16) return names[(int)r];
    return "?";
}

const char* reg_name16(Reg r) {
    const char* names[] = { "ax",  "cx",  "dx",  "bx",  "si",  "di",
                            "r8w", "r9w", "r10w","r11w","r12w","r13w","r14w","r15w",
                            "sp",  "bp" };
    if ((int)r < 16) return names[(int)r];
    return "?";
}

const char* reg_name8(Reg r) {
    const char* names[] = { "al",  "cl",  "dl",  "bl",  "sil", "dil",
                            "r8b", "r9b", "r10b","r11b","r12b","r13b","r14b","r15b",
                            "spl", "bpl" };
    if ((int)r < 16) return names[(int)r];
    return "?";
}

const char* reg_name_for_size(Reg r, uint32_t bytes) {
    switch (bytes) {
        case 1: return reg_name8(r);
        case 2: return reg_name16(r);
        case 4: return reg_name32(r);
        case 8: return reg_name64(r);
        default: return "?";
    }
}

Codegen::Codegen(Module& mod, TypeChecker& tc) : m_mod(mod), m_tc(tc) {}

std::string Codegen::emit_asm() {
    m_text << ".intel_syntax noprefix\n";
    m_text << ".text\n";
    gen_module();
    
    std::string result;
    if (!m_rodata.str().empty()) {
        result += ".rodata\n" + m_rodata.str();
    }
    if (!m_data.str().empty()) {
        result += ".data\n" + m_data.str();
    }
    result += m_text.str();
    return result;
}

void Codegen::gen_module() {
    for (auto& decl : m_mod.decls) {
        if (!decl) continue;
        switch (decl->kind) {
            case Decl::Kind::Function:
                gen_fn(*decl);
                break;
            case Decl::Kind::Global:
                gen_global(*decl);
                break;
            default:
                break;
        }
    }
}

void Codegen::gen_fn(Decl& d) {
    reset_frame();
    m_current_fn_name = d.fn_name;
    
    emit(".globl %s", d.fn_name.c_str());
    emit_label(d.fn_name);
    emit("push rbp");
    emit("mov rbp, rsp");
    
    if (d.body) gen_stmt(*d.body);
    
    emit("pop rbp");
    emit("ret");
}

void Codegen::gen_global(Decl& d) {
    if (d.global_init) {
        auto val = gen_expr(*d.global_init);
        m_data << d.global_name << ": .quad 0\n";
    }
}

void Codegen::gen_stmt(Stmt& s) {
    switch (s.kind) {
        case Stmt::Kind::Block:
            gen_block(s);
            break;
        case Stmt::Kind::Let:
            gen_let(s);
            break;
        case Stmt::Kind::Return:
            gen_return(s);
            break;
        case Stmt::Kind::If:
            gen_if(s);
            break;
        case Stmt::Kind::While:
            gen_while(s);
            break;
        case Stmt::Kind::Loop:
            gen_loop(s);
            break;
        case Stmt::Kind::For:
            gen_for(s);
            break;
        case Stmt::Kind::Expr:
            if (s.expr) gen_expr(*s.expr);
            break;
        default:
            break;
    }
}

void Codegen::gen_block(Stmt& s) {
    m_scope.push();
    for (auto& stmt : s.stmts)
        gen_stmt(*stmt);
    m_scope.pop();
}

void Codegen::gen_let(Stmt& s) {
    int32_t offset = alloc_local(8, 8);
    Symbol sym;
    sym.kind = Symbol::Kind::LocalVar;
    sym.type = Type::make_prim(PrimitiveType::I64);
    sym.is_mut = s.is_mut;
    m_scope.define(s.var_name, sym);
    
    if (s.init) {
        auto val = gen_expr(*s.init);
        emit("mov qword ptr [rbp - %d], rax", offset);
    }
}

void Codegen::gen_return(Stmt& s) {
    if (s.ret_val) {
        gen_expr(*s.ret_val);
    }
    emit("pop rbp");
    emit("ret");
}

void Codegen::gen_if(Stmt& s) {
    auto else_label = new_label("else");
    auto end_label = new_label("endif");
    
    gen_expr(*s.if_cond);
    emit("cmp rax, 0");
    emit("je %s", else_label.c_str());
    
    gen_stmt(*s.then_block);
    emit("jmp %s", end_label.c_str());
    
    emit_label(else_label);
    if (s.else_block) gen_stmt(*s.else_block);
    
    emit_label(end_label);
}

void Codegen::gen_while(Stmt& s) {
    auto loop_label = new_label("while");
    auto end_label = new_label("endwhile");
    
    emit_label(loop_label);
    gen_expr(*s.while_cond);
    emit("cmp rax, 0");
    emit("je %s", end_label.c_str());
    
    gen_stmt(*s.body);
    emit("jmp %s", loop_label.c_str());
    
    emit_label(end_label);
}

void Codegen::gen_loop(Stmt& s) {
    auto loop_label = new_label("loop");
    auto end_label = new_label("endloop");
    
    m_break_label = end_label;
    m_continue_label = loop_label;
    
    emit_label(loop_label);
    gen_stmt(*s.body);
    emit("jmp %s", loop_label.c_str());
    
    emit_label(end_label);
}

void Codegen::gen_for(Stmt& s) {
    auto loop_label = new_label("for");
    auto end_label = new_label("endfor");
    
    auto start_val = gen_expr(*s.for_start);
    emit("mov rax, %ld", start_val.imm);
    
    int32_t offset = alloc_local(8, 8);
    emit("mov qword ptr [rbp - %d], rax", offset);
    
    emit_label(loop_label);
    emit("mov rax, qword ptr [rbp - %d]", offset);
    
    auto end_val = gen_expr(*s.for_end);
    emit("cmp rax, %ld", end_val.imm);
    emit("jge %s", end_label.c_str());
    
    gen_stmt(*s.for_body);
    
    emit("mov rax, qword ptr [rbp - %d]", offset);
    emit("add rax, 1");
    emit("mov qword ptr [rbp - %d], rax", offset);
    emit("jmp %s", loop_label.c_str());
    
    emit_label(end_label);
}

void Codegen::gen_asm_stmt(Stmt& s) {
    m_text << s.asm_code << "\n";
}

Value Codegen::gen_expr(Expr& e) {
    switch (e.kind) {
        case Expr::Kind::IntLit:
            emit("mov rax, %ld", e.int_val);
            return Value::immediate(e.int_val, Type::make_prim(PrimitiveType::I64));
        case Expr::Kind::FloatLit:
            return Value::immediate((int64_t)e.float_val, Type::make_prim(PrimitiveType::F64));
        case Expr::Kind::BoolLit:
            emit("mov rax, %d", e.bool_val ? 1 : 0);
            return Value::immediate(e.bool_val ? 1 : 0, Type::make_prim(PrimitiveType::Bool));
        case Expr::Kind::StringLit:
            return Value::global(".S" + std::to_string(m_str_ctr++), Type::make_prim(PrimitiveType::U8));
        case Expr::Kind::Ident: {
            Symbol* sym = m_scope.lookup(e.name);
            if (sym) {
                return lookup_var(e.name);
            }
            return Value::immediate(0, Type::make_error());
        }
        case Expr::Kind::Binary:
            return gen_binary(e);
        case Expr::Kind::Unary:
            return gen_unary(e);
        case Expr::Kind::Call:
            return gen_call(e);
        case Expr::Kind::Assign:
            return gen_assign(e);
        default:
            return Value::immediate(0, Type::make_error());
    }
}

Value Codegen::gen_binary(Expr& e) {
    auto lhs = gen_expr(*e.lhs);
    emit("push rax");
    auto rhs = gen_expr(*e.rhs);
    emit("pop rcx");
    
    switch (e.op) {
        case TokenKind::PLUS:
            emit("add rax, rcx");
            break;
        case TokenKind::MINUS:
            emit("sub rcx, rax");
            emit("mov rax, rcx");
            break;
        case TokenKind::STAR:
            emit("imul rax, rcx");
            break;
        default:
            break;
    }
    
    return Value::in_reg(Reg::RAX, Type::make_prim(PrimitiveType::I64));
}

Value Codegen::gen_unary(Expr& e) {
    auto operand = gen_expr(*e.lhs);
    
    switch (e.op) {
        case TokenKind::MINUS:
            emit("neg rax");
            break;
        case TokenKind::BANG:
            emit("cmp rax, 0");
            emit("setne al");
            break;
        default:
            break;
    }
    
    return Value::in_reg(Reg::RAX, Type::make_prim(PrimitiveType::I64));
}

Value Codegen::gen_call(Expr& e) {
    gen_expr(*e.callee);
    
    for (size_t i = 0; i < e.args.size(); i++) {
        auto arg = gen_expr(*e.args[i]);
        if (i < 6) {
            emit("mov %s, rax", reg_name64(ARG_REGS[i]));
        }
    }
    
    emit("call rax");
    return Value::in_reg(Reg::RAX, Type::make_prim(PrimitiveType::I64));
}

Value Codegen::gen_assign(Expr& e) {
    auto rhs = gen_expr(*e.rhs);
    emit("push rax");
    
    auto lhs_var = lookup_var(e.lhs->name);
    emit("pop rax");
    
    if (lhs_var.loc == Value::Loc::Stack) {
        emit("mov qword ptr [rbp - %d], rax", lhs_var.offset);
    }
    
    return rhs;
}

Value Codegen::gen_cast(Expr& e) {
    return gen_expr(*e.lhs);
}

Value Codegen::gen_field_access(Expr& e) {
    return gen_expr(*e.object);
}

Value Codegen::gen_index(Expr& e) {
    gen_expr(*e.object);
    emit("push rax");
    gen_expr(*e.index);
    emit("pop rcx");
    return Value::in_reg(Reg::RAX, Type::make_prim(PrimitiveType::I64));
}

Value Codegen::gen_address_of(Expr& e) {
    auto val = gen_expr(*e.lhs);
    if (val.loc == Value::Loc::Stack) {
        emit("lea rax, [rbp - %d]", val.offset);
    }
    return Value::in_reg(Reg::RAX, Type::make_ptr(val.type, false));
}

Value Codegen::gen_deref(Expr& e) {
    gen_expr(*e.lhs);
    emit("mov rax, [rax]");
    return Value::in_reg(Reg::RAX, Type::make_prim(PrimitiveType::I64));
}

Reg Codegen::alloc_reg() {
    for (int i = 0; i < 6; i++) {
        if (!m_regs_used[i]) {
            m_regs_used[i] = true;
            return (Reg)i;
        }
    }
    return Reg::NONE;
}

void Codegen::free_reg(Reg r) {
    if ((int)r < 16) m_regs_used[(int)r] = false;
}

void Codegen::spill_all() {
    for (int i = 0; i < 16; i++) m_regs_used[i] = false;
}

Value Codegen::load_value(const Value& v, Reg target_hint) {
    Reg target = target_hint != Reg::NONE ? target_hint : Reg::RAX;
    
    switch (v.loc) {
        case Value::Loc::Reg:
            if (v.reg != target) emit("mov %s, %s", reg_name64(target), reg_name64(v.reg));
            break;
        case Value::Loc::Stack:
            emit("mov %s, [rbp - %d]", reg_name64(target), v.offset);
            break;
        case Value::Loc::Immediate:
            emit("mov %s, %ld", reg_name64(target), v.imm);
            break;
        case Value::Loc::Global:
            emit("lea %s, [%s]", reg_name64(target), v.global_name.c_str());
            break;
    }
    
    return Value::in_reg(target, v.type);
}

void Codegen::store_value(const Value& src, const Value& dst_addr) {
    load_value(src, Reg::RAX);
    if (dst_addr.loc == Value::Loc::Stack) {
        emit("mov [rbp - %d], rax", dst_addr.offset);
    }
}

void Codegen::move_to_reg(Reg dst, const Value& src) {
    emit("mov %s, %s", reg_name64(dst), reg_name64(src.reg));
}

int32_t Codegen::alloc_local(uint32_t size, uint32_t align) {
    m_stack_offset = (m_stack_offset + align - 1) & ~(align - 1);
    m_stack_offset += size;
    return m_stack_offset;
}

void Codegen::reset_frame() {
    m_locals.clear();
    m_stack_offset = 0;
    m_label_ctr = 0;
    spill_all();
}

std::string Codegen::new_label(const char* prefix) {
    return std::string(prefix) + std::to_string(m_label_ctr++);
}

void Codegen::emit_label(const std::string& lbl) {
    m_text << lbl << ":\n";
}

void Codegen::emit(const std::string& line) {
    m_text << "  " << line << "\n";
}

void Codegen::emit(const char* fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    emit(std::string(buf));
}

std::string Codegen::operand(const Value& v) {
    char buf[64];
    switch (v.loc) {
        case Value::Loc::Reg:
            return reg_name64(v.reg);
        case Value::Loc::Stack:
            snprintf(buf, sizeof(buf), "[rbp - %d]", v.offset);
            return buf;
        case Value::Loc::Immediate:
            snprintf(buf, sizeof(buf), "%ld", v.imm);
            return buf;
        case Value::Loc::Global:
            return v.global_name;
    }
    return "?";
}

void Codegen::emit_mov(const Value& dst, const Value& src, uint32_t) {
    emit("mov %s, %s", operand(dst).c_str(), operand(src).c_str());
}

void Codegen::emit_cmp(const Value& lhs, const Value& rhs, uint32_t) {
    emit("cmp %s, %s", operand(lhs).c_str(), operand(rhs).c_str());
}

Value Codegen::lookup_var(const std::string& name) {
    Symbol* sym = m_scope.lookup(name);
    if (sym && sym->kind == Symbol::Kind::LocalVar) {
        return Value::on_stack(-8, sym->type);
    }
    return Value::immediate(0, Type::make_error());
}

} // namespace cminus
