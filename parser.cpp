#include "parser.h"
#include <cstdio>
#include <cassert>
#include <stdexcept>

namespace cminus {

Parser::Parser(std::vector<Token> tokens, const char* filename)
    : m_tokens(std::move(tokens)), m_filename(filename) {}

Token& Parser::peek(int offset) {
    size_t idx = m_pos + offset;
    if (idx >= m_tokens.size()) return m_tokens.back();
    return m_tokens[idx];
}

Token& Parser::advance() {
    Token& t = m_tokens[m_pos];
    if (m_pos + 1 < m_tokens.size()) m_pos++;
    return t;
}

bool Parser::check(TokenKind k) const {
    return m_tokens[m_pos].kind == k;
}

bool Parser::match(TokenKind k) {
    if (check(k)) { advance(); return true; }
    return false;
}

Token Parser::expect(TokenKind k, const char* msg) {
    if (check(k)) return advance();
    error(peek().loc, msg);
    Token dummy;
    dummy.kind = k;
    dummy.loc  = peek().loc;
    return dummy;
}

bool Parser::at_end() const {
    return m_tokens[m_pos].kind == TokenKind::EOF_TOKEN;
}

void Parser::error(const SourceLocation& loc, const char* msg) {
    m_has_errors = true;
    fprintf(stderr, "%s:%u:%u: error: %s (got '%s')\n",
        loc.file, loc.line, loc.col, msg, peek().text.c_str());
}

void Parser::synchronize() {
    while (!at_end()) {
        if (peek(-1 + 0).kind == TokenKind::SEMICOLON) return;
        switch (peek().kind) {
            case TokenKind::KW_FN:
            case TokenKind::KW_STRUCT:
            case TokenKind::KW_LET:
            case TokenKind::KW_RET:
            case TokenKind::KW_IF:
            case TokenKind::KW_WHILE:
            case TokenKind::KW_FOR:
            case TokenKind::KW_LOOP:
                return;
            default: break;
        }
        advance();
    }
}

std::unique_ptr<Module> Parser::parse_module(const std::string& name) {
    auto mod = std::make_unique<Module>();
    mod->name = name;
    while (!at_end()) {
        try {
            auto d = parse_decl();
            if (d) mod->decls.push_back(std::move(d));
        } catch (...) {
            m_has_errors = true;
            synchronize();
        }
    }
    return mod;
}

DeclPtr Parser::parse_decl() {
    bool is_pub = false;
    if (match(TokenKind::KW_PUB)) is_pub = true;

    if (check(TokenKind::KW_FN))     return parse_fn(is_pub);
    if (check(TokenKind::KW_STRUCT)) return parse_struct(is_pub);
    if (check(TokenKind::KW_EXTERN)) return parse_extern();
    if (check(TokenKind::KW_IMPORT)) return parse_import();
    if (check(TokenKind::KW_LET))    return parse_global(is_pub);

    error(peek().loc, "expected declaration (fn, struct, let, extern, import)");
    advance();
    return nullptr;
}

DeclPtr Parser::parse_fn(bool is_pub) {
    auto d = std::make_unique<Decl>();
    d->kind   = Decl::Kind::Function;
    d->is_pub = is_pub;
    d->loc    = peek().loc;

    expect(TokenKind::KW_FN, "expected 'fn'");
    d->fn_name = expect(TokenKind::IDENT, "expected function name").text;
    expect(TokenKind::LPAREN, "expected '(' after function name");

    while (!check(TokenKind::RPAREN) && !at_end()) {
        if (check(TokenKind::DOT_DOT)) {
            advance();
            d->is_variadic = true;
            break;
        }
        d->params.push_back(parse_param());
        if (!match(TokenKind::COMMA)) break;
    }
    expect(TokenKind::RPAREN, "expected ')' after parameter list");

    if (match(TokenKind::ARROW)) {
        d->ret_type = parse_type();
    } else {
        auto vt = std::make_unique<TypeExpr>();
        vt->kind = TypeExpr::Kind::Primitive;
        vt->primitive = PrimitiveType::Void;
        d->ret_type = std::move(vt);
    }

    d->body = parse_block();
    return d;
}

DeclPtr Parser::parse_extern() {
    auto d = std::make_unique<Decl>();
    d->kind = Decl::Kind::ExternFunction;
    d->loc  = peek().loc;
    expect(TokenKind::KW_EXTERN, "expected 'extern'");
    expect(TokenKind::KW_FN,     "expected 'fn' after 'extern'");
    d->fn_name = expect(TokenKind::IDENT, "expected function name").text;
    expect(TokenKind::LPAREN, "expected '('");
    while (!check(TokenKind::RPAREN) && !at_end()) {
        if (check(TokenKind::DOT_DOT)) { advance(); d->is_variadic = true; break; }
        d->params.push_back(parse_param());
        if (!match(TokenKind::COMMA)) break;
    }
    expect(TokenKind::RPAREN, "expected ')'");
    if (match(TokenKind::ARROW)) d->ret_type = parse_type();
    else {
        auto vt = std::make_unique<TypeExpr>();
        vt->kind = TypeExpr::Kind::Primitive;
        vt->primitive = PrimitiveType::Void;
        d->ret_type = std::move(vt);
    }
    expect(TokenKind::SEMICOLON, "expected ';' after extern declaration");
    return d;
}

DeclPtr Parser::parse_struct(bool is_pub) {
    auto d = std::make_unique<Decl>();
    d->kind   = Decl::Kind::Struct;
    d->is_pub = is_pub;
    d->loc    = peek().loc;
    expect(TokenKind::KW_STRUCT, "expected 'struct'");
    d->struct_name = expect(TokenKind::IDENT, "expected struct name").text;
    expect(TokenKind::LBRACE, "expected '{'");
    while (!check(TokenKind::RBRACE) && !at_end()) {
        StructField sf;
        sf.loc  = peek().loc;
        sf.name = expect(TokenKind::IDENT, "expected field name").text;
        expect(TokenKind::COLON, "expected ':' after field name");
        sf.type = parse_type();
        d->struct_fields.push_back(std::move(sf));
        if (!match(TokenKind::COMMA)) break;
    }
    expect(TokenKind::RBRACE, "expected '}' after struct fields");
    return d;
}

DeclPtr Parser::parse_global(bool is_pub) {
    auto d = std::make_unique<Decl>();
    d->kind      = Decl::Kind::Global;
    d->is_pub    = is_pub;
    d->loc       = peek().loc;
    expect(TokenKind::KW_LET, "expected 'let'");
    d->global_mut = match(TokenKind::KW_MUT);
    d->global_name = expect(TokenKind::IDENT, "expected variable name").text;
    if (match(TokenKind::COLON)) d->global_type = parse_type();
    if (match(TokenKind::EQ))    d->global_init = parse_expr();
    expect(TokenKind::SEMICOLON, "expected ';'");
    return d;
}

DeclPtr Parser::parse_import() {
    auto d = std::make_unique<Decl>();
    d->kind = Decl::Kind::Import;
    d->loc  = peek().loc;
    expect(TokenKind::KW_IMPORT, "expected 'import'");
    d->import_path = expect(TokenKind::STRING_LIT, "expected import path string").text;
    if (match(TokenKind::KW_AS))
        d->import_alias = expect(TokenKind::IDENT, "expected alias name").text;
    expect(TokenKind::SEMICOLON, "expected ';'");
    return d;
}

Param Parser::parse_param() {
    Param p;
    p.loc  = peek().loc;
    p.name = expect(TokenKind::IDENT, "expected parameter name").text;
    expect(TokenKind::COLON, "expected ':' after parameter name");
    p.type = parse_type();
    return p;
}

TypeExprPtr Parser::parse_type() {
    auto te = std::make_unique<TypeExpr>();
    te->loc = peek().loc;

    if (check(TokenKind::STAR)) {
        advance();
        if (check(TokenKind::KW_MUT)) {
            advance();
            te->kind  = TypeExpr::Kind::MutPointer;
        } else {
            te->kind  = TypeExpr::Kind::Pointer;
        }
        te->inner = parse_type();
        return te;
    }

    if (check(TokenKind::LBRACKET)) {
        advance();
        te->inner = parse_type();
        if (match(TokenKind::SEMICOLON)) {
            te->kind       = TypeExpr::Kind::Array;
            te->array_size = parse_expr();
        } else {
            te->kind = TypeExpr::Kind::Slice;
        }
        expect(TokenKind::RBRACKET, "expected ']'");
        return te;
    }

    if (check(TokenKind::KW_FN)) {
        advance();
        te->kind = TypeExpr::Kind::FnPtr;
        expect(TokenKind::LPAREN, "expected '(' in fn pointer type");
        while (!check(TokenKind::RPAREN) && !at_end()) {
            te->fn_params.push_back(parse_type());
            if (!match(TokenKind::COMMA)) break;
        }
        expect(TokenKind::RPAREN, "expected ')'");
        if (match(TokenKind::ARROW)) te->fn_ret = parse_type();
        return te;
    }

    te->kind = TypeExpr::Kind::Primitive;
    switch (peek().kind) {
        case TokenKind::TY_I8:  te->primitive = PrimitiveType::I8;  advance(); break;
        case TokenKind::TY_I16: te->primitive = PrimitiveType::I16; advance(); break;
        case TokenKind::TY_I32: te->primitive = PrimitiveType::I32; advance(); break;
        case TokenKind::TY_I64: te->primitive = PrimitiveType::I64; advance(); break;
        case TokenKind::TY_U8:  te->primitive = PrimitiveType::U8;  advance(); break;
        case TokenKind::TY_U16: te->primitive = PrimitiveType::U16; advance(); break;
        case TokenKind::TY_U32: te->primitive = PrimitiveType::U32; advance(); break;
        case TokenKind::TY_U64: te->primitive = PrimitiveType::U64; advance(); break;
        case TokenKind::TY_F32: te->primitive = PrimitiveType::F32; advance(); break;
        case TokenKind::TY_F64: te->primitive = PrimitiveType::F64; advance(); break;
        case TokenKind::TY_BOOL:te->primitive = PrimitiveType::Bool;advance(); break;
        case TokenKind::TY_VOID:te->primitive = PrimitiveType::Void;advance(); break;
        case TokenKind::TY_CHAR:te->primitive = PrimitiveType::Char;advance(); break;
        case TokenKind::IDENT:
            te->kind = TypeExpr::Kind::Named;
            te->name = advance().text;
            break;
        default:
            error(peek().loc, "expected type");
            te->kind      = TypeExpr::Kind::Primitive;
            te->primitive = PrimitiveType::I32;
    }
    return te;
}

StmtPtr Parser::parse_stmt() {
    if (check(TokenKind::LBRACE))    return parse_block();
    if (check(TokenKind::KW_LET))    return parse_let();
    if (check(TokenKind::KW_RET))    return parse_return();
    if (check(TokenKind::KW_IF))     return parse_if();
    if (check(TokenKind::KW_WHILE))  return parse_while();
    if (check(TokenKind::KW_LOOP))   return parse_loop();
    if (check(TokenKind::KW_FOR))    return parse_for();
    if (check(TokenKind::KW_ASM))    return parse_asm_stmt();
    if (check(TokenKind::KW_BREAK)) {
        auto s = std::make_unique<Stmt>();
        s->kind = Stmt::Kind::Break;
        s->loc  = peek().loc;
        advance();
        expect(TokenKind::SEMICOLON, "expected ';'");
        return s;
    }
    if (check(TokenKind::KW_CONTINUE)) {
        auto s = std::make_unique<Stmt>();
        s->kind = Stmt::Kind::Continue;
        s->loc  = peek().loc;
        advance();
        expect(TokenKind::SEMICOLON, "expected ';'");
        return s;
    }

    auto s = std::make_unique<Stmt>();
    s->kind = Stmt::Kind::Expr;
    s->loc  = peek().loc;
    s->expr = parse_expr();
    expect(TokenKind::SEMICOLON, "expected ';' after expression statement");
    return s;
}

StmtPtr Parser::parse_block() {
    auto s = std::make_unique<Stmt>();
    s->kind = Stmt::Kind::Block;
    s->loc  = peek().loc;
    expect(TokenKind::LBRACE, "expected '{'");
    while (!check(TokenKind::RBRACE) && !at_end())
        s->stmts.push_back(parse_stmt());
    expect(TokenKind::RBRACE, "expected '}'");
    return s;
}

StmtPtr Parser::parse_let() {
    auto s = std::make_unique<Stmt>();
    s->kind = Stmt::Kind::Let;
    s->loc  = peek().loc;
    expect(TokenKind::KW_LET, "expected 'let'");
    s->is_mut   = match(TokenKind::KW_MUT);
    s->var_name = expect(TokenKind::IDENT, "expected variable name").text;
    if (match(TokenKind::COLON)) s->var_type = parse_type();
    if (match(TokenKind::EQ))    s->init     = parse_expr();
    expect(TokenKind::SEMICOLON, "expected ';'");
    return s;
}

StmtPtr Parser::parse_return() {
    auto s = std::make_unique<Stmt>();
    s->kind = Stmt::Kind::Return;
    s->loc  = peek().loc;
    expect(TokenKind::KW_RET, "expected 'ret'");
    if (!check(TokenKind::SEMICOLON)) s->ret_val = parse_expr();
    expect(TokenKind::SEMICOLON, "expected ';' after return statement");
    return s;
}

StmtPtr Parser::parse_if() {
    auto s = std::make_unique<Stmt>();
    s->kind = Stmt::Kind::If;
    s->loc  = peek().loc;
    expect(TokenKind::KW_IF, "expected 'if'");
    s->if_cond    = parse_expr();
    s->then_block = parse_block();
    if (match(TokenKind::KW_ELSE)) {
        if (check(TokenKind::KW_IF)) s->else_block = parse_if();
        else                          s->else_block = parse_block();
    }
    return s;
}

StmtPtr Parser::parse_while() {
    auto s = std::make_unique<Stmt>();
    s->kind = Stmt::Kind::While;
    s->loc  = peek().loc;
    expect(TokenKind::KW_WHILE, "expected 'while'");
    s->while_cond = parse_expr();
    s->body       = parse_block();
    return s;
}

StmtPtr Parser::parse_loop() {
    auto s = std::make_unique<Stmt>();
    s->kind = Stmt::Kind::Loop;
    s->loc  = peek().loc;
    expect(TokenKind::KW_LOOP, "expected 'loop'");
    s->body = parse_block();
    return s;
}

StmtPtr Parser::parse_for() {
    auto s = std::make_unique<Stmt>();
    s->kind = Stmt::Kind::For;
    s->loc  = peek().loc;
    expect(TokenKind::KW_FOR, "expected 'for'");
    s->for_var = expect(TokenKind::IDENT, "expected loop variable").text;
    expect(TokenKind::KW_IN,  "expected 'in'");
    s->for_start = parse_expr();
    expect(TokenKind::DOT_DOT, "expected '..' in range");
    s->for_end  = parse_expr();
    if (match(TokenKind::COLON)) s->for_step = parse_expr();
    s->for_body = parse_block();
    return s;
}

StmtPtr Parser::parse_asm_stmt() {
    auto s = std::make_unique<Stmt>();
    s->kind = Stmt::Kind::Asm;
    s->loc  = peek().loc;
    expect(TokenKind::KW_ASM, "expected 'asm'");
    expect(TokenKind::LPAREN, "expected '('");
    s->asm_code = expect(TokenKind::STRING_LIT, "expected asm code string").text;
    if (match(TokenKind::COLON)) {
        while (check(TokenKind::STRING_LIT)) {
            std::string constraint = advance().text;
            expect(TokenKind::LPAREN, "expected '('");
            auto expr = parse_expr();
            expect(TokenKind::RPAREN, "expected ')'");
            s->asm_outputs.push_back({constraint, std::move(expr)});
            if (!match(TokenKind::COMMA)) break;
        }
    }
    if (match(TokenKind::COLON)) {
        while (check(TokenKind::STRING_LIT)) {
            std::string constraint = advance().text;
            expect(TokenKind::LPAREN, "expected '('");
            auto expr = parse_expr();
            expect(TokenKind::RPAREN, "expected ')'");
            s->asm_inputs.push_back({constraint, std::move(expr)});
            if (!match(TokenKind::COMMA)) break;
        }
    }
    if (match(TokenKind::COLON)) {
        while (check(TokenKind::STRING_LIT)) {
            s->asm_clobbers.push_back(advance().text);
            if (!match(TokenKind::COMMA)) break;
        }
    }
    expect(TokenKind::RPAREN, "expected ')'");
    expect(TokenKind::SEMICOLON, "expected ';'");
    return s;
}

ExprPtr Parser::parse_expr() { return parse_assign(); }

ExprPtr Parser::parse_assign() {
    auto lhs = parse_ternary();
    if (peek().is_assignment_op()) {
        TokenKind op  = peek().kind;
        SourceLocation loc = peek().loc;
        advance();
        auto rhs = parse_assign();
        auto e = std::make_unique<Expr>();
        e->kind = Expr::Kind::Assign;
        e->loc  = loc;
        e->op   = op;
        e->lhs  = std::move(lhs);
        e->rhs  = std::move(rhs);
        return e;
    }
    return lhs;
}

ExprPtr Parser::parse_ternary() {
    return parse_or();
}

#define BINARY_LEFT(name, next, ...)                                         \
ExprPtr Parser::name() {                                                     \
    auto lhs = next();                                                       \
    while (true) {                                                           \
        TokenKind k = peek().kind;                                           \
        if (!(__VA_ARGS__)) break;                                           \
        SourceLocation loc = peek().loc; advance();                         \
        auto rhs = next();                                                   \
        auto e = std::make_unique<Expr>();                                   \
        e->kind = Expr::Kind::Binary; e->loc = loc;                         \
        e->op = k; e->lhs = std::move(lhs); e->rhs = std::move(rhs);       \
        lhs = std::move(e);                                                  \
    }                                                                        \
    return lhs;                                                              \
}

BINARY_LEFT(parse_or,          parse_and,          k == TokenKind::OR_OR)
BINARY_LEFT(parse_and,         parse_bitwise_or,   k == TokenKind::AND_AND)
BINARY_LEFT(parse_bitwise_or,  parse_bitwise_xor,  k == TokenKind::PIPE)
BINARY_LEFT(parse_bitwise_xor, parse_bitwise_and,  k == TokenKind::CARET)
BINARY_LEFT(parse_bitwise_and, parse_equality,     k == TokenKind::AMPERSAND)
BINARY_LEFT(parse_equality,    parse_relational,
    k == TokenKind::EQ_EQ || k == TokenKind::BANG_EQ)
BINARY_LEFT(parse_relational,  parse_shift,
    k == TokenKind::LT    || k == TokenKind::LT_EQ ||
    k == TokenKind::GT    || k == TokenKind::GT_EQ)
BINARY_LEFT(parse_shift,       parse_additive,
    k == TokenKind::LSHIFT || k == TokenKind::RSHIFT)
BINARY_LEFT(parse_additive,    parse_multiplicative,
    k == TokenKind::PLUS || k == TokenKind::MINUS)
BINARY_LEFT(parse_multiplicative, parse_unary,
    k == TokenKind::STAR || k == TokenKind::SLASH || k == TokenKind::PERCENT)

#undef BINARY_LEFT

ExprPtr Parser::parse_unary() {
    SourceLocation loc = peek().loc;
    TokenKind k = peek().kind;
    if (k == TokenKind::BANG || k == TokenKind::MINUS || k == TokenKind::TILDE) {
        advance();
        auto e = std::make_unique<Expr>();
        e->kind = Expr::Kind::Unary;
        e->loc  = loc;
        e->op   = k;
        e->lhs  = parse_unary();
        return e;
    }
    if (k == TokenKind::AMPERSAND) {
        advance();
        auto e = std::make_unique<Expr>();
        e->kind = Expr::Kind::AddressOf;
        e->loc  = loc;
        e->lhs  = parse_unary();
        return e;
    }
    if (k == TokenKind::STAR) {
        advance();
        auto e = std::make_unique<Expr>();
        e->kind = Expr::Kind::Deref;
        e->loc  = loc;
        e->lhs  = parse_unary();
        return e;
    }

    auto base = parse_primary();
    return parse_postfix(std::move(base));
}

ExprPtr Parser::parse_postfix(ExprPtr base) {
    while (true) {
        SourceLocation loc = peek().loc;
        if (check(TokenKind::LPAREN)) {
            base = parse_call(std::move(base));
        } else if (match(TokenKind::LBRACKET)) {
            auto e = std::make_unique<Expr>();
            e->kind   = Expr::Kind::Index;
            e->loc    = loc;
            e->object = std::move(base);
            e->index  = parse_expr();
            expect(TokenKind::RBRACKET, "expected ']'");
            base = std::move(e);
        } else if (match(TokenKind::DOT)) {
            auto e = std::make_unique<Expr>();
            e->kind   = Expr::Kind::Field;
            e->loc    = loc;
            e->object = std::move(base);
            e->field  = expect(TokenKind::IDENT, "expected field name").text;
            base = std::move(e);
        } else if (match(TokenKind::ARROW)) {
            auto e = std::make_unique<Expr>();
            e->kind   = Expr::Kind::Arrow;
            e->loc    = loc;
            e->object = std::move(base);
            e->field  = expect(TokenKind::IDENT, "expected field name").text;
            base = std::move(e);
        } else {
            break;
        }
    }
    return base;
}

ExprPtr Parser::parse_call(ExprPtr callee) {
    auto e = std::make_unique<Expr>();
    e->kind   = Expr::Kind::Call;
    e->loc    = peek().loc;
    e->callee = std::move(callee);
    expect(TokenKind::LPAREN, "expected '('");
    while (!check(TokenKind::RPAREN) && !at_end()) {
        e->args.push_back(parse_expr());
        if (!match(TokenKind::COMMA)) break;
    }
    expect(TokenKind::RPAREN, "expected ')'");
    return e;
}

ExprPtr Parser::parse_primary() {
    SourceLocation loc = peek().loc;

    if (check(TokenKind::KW_CAST))   return parse_cast();
    if (check(TokenKind::KW_SIZEOF)) return parse_sizeof();
    if (check(TokenKind::KW_ASM))    return parse_asm_expr();

    if (check(TokenKind::INTEGER_LIT)) {
        auto e = std::make_unique<Expr>();
        e->kind    = Expr::Kind::IntLit;
        e->loc     = loc;
        e->int_val = peek().int_val;
        advance();
        return e;
    }
    if (check(TokenKind::FLOAT_LIT)) {
        auto e = std::make_unique<Expr>();
        e->kind      = Expr::Kind::FloatLit;
        e->loc       = loc;
        e->float_val = peek().float_val;
        advance();
        return e;
    }
    if (check(TokenKind::BOOL_LIT)) {
        auto e = std::make_unique<Expr>();
        e->kind     = Expr::Kind::BoolLit;
        e->loc      = loc;
        e->bool_val = peek().bool_val;
        advance();
        return e;
    }
    if (check(TokenKind::STRING_LIT)) {
        auto e = std::make_unique<Expr>();
        e->kind    = Expr::Kind::StringLit;
        e->loc     = loc;
        e->str_val = peek().text;
        advance();
        return e;
    }
    if (check(TokenKind::CHAR_LIT)) {
        auto e = std::make_unique<Expr>();
        e->kind     = Expr::Kind::CharLit;
        e->loc      = loc;
        e->char_val = peek().char_val;
        e->int_val  = peek().int_val;
        advance();
        return e;
    }
    if (check(TokenKind::KW_NULL)) {
        auto e = std::make_unique<Expr>();
        e->kind = Expr::Kind::NullLit;
        e->loc  = loc;
        advance();
        return e;
    }
    if (check(TokenKind::IDENT)) {
        std::string name = advance().text;
        if (check(TokenKind::LBRACE)) return parse_compound_lit(name);
        auto e = std::make_unique<Expr>();
        e->kind = Expr::Kind::Ident;
        e->loc  = loc;
        e->name = name;
        return e;
    }
    if (match(TokenKind::LPAREN)) {
        auto e = parse_expr();
        expect(TokenKind::RPAREN, "expected ')'");
        return e;
    }

    error(loc, "expected expression");
    auto e = std::make_unique<Expr>();
    e->kind    = Expr::Kind::IntLit;
    e->loc     = loc;
    e->int_val = 0;
    advance();
    return e;
}

ExprPtr Parser::parse_compound_lit(const std::string& name) {
    auto e = std::make_unique<Expr>();
    e->kind        = Expr::Kind::CompoundLit;
    e->loc         = peek().loc;
    e->struct_name = name;
    expect(TokenKind::LBRACE, "expected '{'");
    while (!check(TokenKind::RBRACE) && !at_end()) {
        Expr::FieldInit fi;
        expect(TokenKind::DOT, "expected '.' before field name in struct literal");
        fi.name  = expect(TokenKind::IDENT, "expected field name").text;
        expect(TokenKind::EQ, "expected '=' after field name");
        fi.value = parse_expr();
        e->fields.push_back(std::move(fi));
        if (!match(TokenKind::COMMA)) break;
    }
    expect(TokenKind::RBRACE, "expected '}'");
    return e;
}

ExprPtr Parser::parse_cast() {
    auto e = std::make_unique<Expr>();
    e->kind = Expr::Kind::Cast;
    e->loc  = peek().loc;
    expect(TokenKind::KW_CAST, "expected 'cast'");
    expect(TokenKind::LT,      "expected '<' after 'cast'");
    e->type_arg = parse_type();
    expect(TokenKind::GT,      "expected '>' after cast type");
    expect(TokenKind::LPAREN,  "expected '('");
    e->lhs = parse_expr();
    expect(TokenKind::RPAREN,  "expected ')'");
    return e;
}

ExprPtr Parser::parse_sizeof() {
    auto e = std::make_unique<Expr>();
    e->kind = Expr::Kind::Sizeof;
    e->loc  = peek().loc;
    expect(TokenKind::KW_SIZEOF, "expected 'sizeof'");
    expect(TokenKind::LPAREN,    "expected '('");
    e->type_arg = parse_type();
    expect(TokenKind::RPAREN,    "expected ')'");
    return e;
}

ExprPtr Parser::parse_asm_expr() {
    auto e = std::make_unique<Expr>();
    e->kind = Expr::Kind::AsmExpr;
    e->loc  = peek().loc;
    expect(TokenKind::KW_ASM,  "expected 'asm'");
    expect(TokenKind::LPAREN,  "expected '('");
    e->asm_code = expect(TokenKind::STRING_LIT, "expected asm string").text;
    if (match(TokenKind::COLON)) {
        while (check(TokenKind::STRING_LIT)) {
            std::string c = advance().text;
            expect(TokenKind::LPAREN, "expected '('");
            auto ex = parse_expr();
            expect(TokenKind::RPAREN, "expected ')'");
            e->asm_outputs.push_back({c, std::move(ex)});
            if (!match(TokenKind::COMMA)) break;
        }
    }
    if (match(TokenKind::COLON)) {
        while (check(TokenKind::STRING_LIT)) {
            std::string c = advance().text;
            expect(TokenKind::LPAREN, "expected '('");
            auto ex = parse_expr();
            expect(TokenKind::RPAREN, "expected ')'");
            e->asm_inputs.push_back({c, std::move(ex)});
            if (!match(TokenKind::COMMA)) break;
        }
    }
    if (match(TokenKind::COLON)) {
        while (check(TokenKind::STRING_LIT)) {
            e->asm_clobbers.push_back(advance().text);
            if (!match(TokenKind::COMMA)) break;
        }
    }
    expect(TokenKind::RPAREN, "expected ')'");
    return e;
}

} // namespace cminus
