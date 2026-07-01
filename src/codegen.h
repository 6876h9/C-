#pragma once
#include "ast.h"
#include "typechecker.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <memory>

namespace cminus {

enum class Reg : uint8_t {
    RAX, RCX, RDX, RBX, RSI, RDI,
    R8,  R9,  R10, R11, R12, R13, R14, R15,
    RSP, RBP,
    NONE,
};

const char* reg_name64(Reg r);
const char* reg_name32(Reg r);
const char* reg_name16(Reg r);
const char* reg_name8(Reg  r);
const char* reg_name_for_size(Reg r, uint32_t bytes);

static constexpr Reg ARG_REGS[6] = {
    Reg::RDI, Reg::RSI, Reg::RDX, Reg::RCX, Reg::R8, Reg::R9
};

struct Value {
    enum class Loc { Reg, Stack, Global, Immediate };

    Loc      loc   = Loc::Immediate;
    Reg      reg   = Reg::NONE;
    int32_t  offset = 0;
    std::string global_name;
    int64_t  imm   = 0;
    TypePtr  type;

    static Value in_reg(Reg r, TypePtr t) {
        Value v; v.loc = Loc::Reg; v.reg = r; v.type = std::move(t); return v;
    }
    static Value on_stack(int32_t off, TypePtr t) {
        Value v; v.loc = Loc::Stack; v.offset = off; v.type = std::move(t); return v;
    }
    static Value immediate(int64_t imm, TypePtr t) {
        Value v; v.loc = Loc::Immediate; v.imm = imm; v.type = std::move(t); return v;
    }
    static Value global(const std::string& name, TypePtr t) {
        Value v; v.loc = Loc::Global; v.global_name = name; v.type = std::move(t); return v;
    }
};

class Codegen {
public:
    explicit Codegen(Module& mod, TypeChecker& tc);
    std::string emit_asm();

private:
    void gen_module();
    void gen_fn(Decl& d);
    void gen_global(Decl& d);

    void gen_stmt(Stmt& s);
    void gen_block(Stmt& s);
    void gen_let(Stmt& s);
    void gen_return(Stmt& s);
    void gen_if(Stmt& s);
    void gen_while(Stmt& s);
    void gen_loop(Stmt& s);
    void gen_for(Stmt& s);
    void gen_asm_stmt(Stmt& s);

    Value gen_expr(Expr& e);
    Value gen_binary(Expr& e);
    Value gen_unary(Expr& e);
    Value gen_call(Expr& e);
    Value gen_assign(Expr& e);
    Value gen_cast(Expr& e);
    Value gen_field_access(Expr& e);
    Value gen_index(Expr& e);
    Value gen_address_of(Expr& e);
    Value gen_deref(Expr& e);

    Reg   alloc_reg();
    void  free_reg(Reg r);
    void  spill_all();
    Value load_value(const Value& v, Reg target_hint = Reg::NONE);
    void  store_value(const Value& src, const Value& dst_addr);
    void  move_to_reg(Reg dst, const Value& src);

    int32_t alloc_local(uint32_t size, uint32_t align);
    void    reset_frame();

    std::string new_label(const char* prefix = ".L");
    void        emit_label(const std::string& lbl);

    void emit(const std::string& line);
    void emit(const char* fmt, ...);

    std::string  operand(const Value& v);
    void         emit_mov(const Value& dst, const Value& src, uint32_t bytes);
    void         emit_cmp(const Value& lhs, const Value& rhs, uint32_t bytes);

    Value        lookup_var(const std::string& name);

    Module&       m_mod;
    TypeChecker&  m_tc;
    Scope         m_scope;
    std::ostringstream m_text;
    std::ostringstream m_data;
    std::ostringstream m_rodata;

    std::unordered_map<std::string, Value> m_locals;
    int32_t  m_stack_offset = 0;
    int32_t  m_frame_size   = 0;
    uint32_t m_label_ctr    = 0;
    std::string m_current_fn_name;
    std::string m_break_label;
    std::string m_continue_label;

    bool m_regs_used[16] = {};
    uint32_t m_str_ctr = 0;
};

} // namespace cminus
