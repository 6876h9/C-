#pragma once
#include "token.h"
#include "ast.h"
#include <vector>
#include <memory>

namespace cminus {

class Parser {
public:
    explicit Parser(std::vector<Token> tokens, const char* filename);
    std::unique_ptr<Module> parse_module(const std::string& name);
    bool has_errors() const { return m_has_errors; }

private:
    // Declarations
    DeclPtr     parse_decl();
    DeclPtr     parse_fn(bool is_pub);
    DeclPtr     parse_extern();
    DeclPtr     parse_struct(bool is_pub);
    DeclPtr     parse_global(bool is_pub);
    DeclPtr     parse_import();
    Param       parse_param();

    // Types
    TypeExprPtr parse_type();

    // Statements
    StmtPtr     parse_stmt();
    StmtPtr     parse_block();
    StmtPtr     parse_let();
    StmtPtr     parse_return();
    StmtPtr     parse_if();
    StmtPtr     parse_while();
    StmtPtr     parse_loop();
    StmtPtr     parse_for();
    StmtPtr     parse_asm_stmt();

    // Expressions
    ExprPtr     parse_expr();
    ExprPtr     parse_assign();
    ExprPtr     parse_ternary();
    ExprPtr     parse_or();
    ExprPtr     parse_and();
    ExprPtr     parse_bitwise_or();
    ExprPtr     parse_bitwise_xor();
    ExprPtr     parse_bitwise_and();
    ExprPtr     parse_equality();
    ExprPtr     parse_relational();
    ExprPtr     parse_shift();
    ExprPtr     parse_additive();
    ExprPtr     parse_multiplicative();
    ExprPtr     parse_unary();
    ExprPtr     parse_postfix(ExprPtr base);
    ExprPtr     parse_primary();
    ExprPtr     parse_call(ExprPtr callee);
    ExprPtr     parse_compound_lit(const std::string& name);
    ExprPtr     parse_cast();
    ExprPtr     parse_sizeof();
    ExprPtr     parse_asm_expr();

    // Helpers
    Token&      peek(int offset = 0);
    Token&      advance();
    bool        check(TokenKind k)  const;
    bool        match(TokenKind k);
    Token       expect(TokenKind k, const char* msg);
    bool        at_end()            const;
    void        error(const SourceLocation& loc, const char* msg);
    void        synchronize();

    std::vector<Token> m_tokens;
    size_t             m_pos      = 0;
    bool               m_has_errors = false;
    const char*        m_filename;
};

} // namespace cminus
