#pragma once
#include <string>
#include <cstdint>

namespace cminus {

enum class TokenKind : uint16_t {
    // Literals
    INTEGER_LIT,
    FLOAT_LIT,
    STRING_LIT,
    CHAR_LIT,
    BOOL_LIT,
    // Identifiers and keywords
    IDENT,
    // Keywords
    KW_FN,
    KW_LET,
    KW_MUT,
    KW_RET,
    KW_IF,
    KW_ELSE,
    KW_LOOP,
    KW_WHILE,
    KW_FOR,
    KW_BREAK,
    KW_CONTINUE,
    KW_STRUCT,
    KW_EXTERN,
    KW_ASM,
    KW_IMPORT,
    KW_AS,
    KW_IN,
    KW_NULL,
    KW_SIZEOF,
    KW_CAST,
    KW_PUB,
    // Primitive types
    TY_I8,
    TY_I16,
    TY_I32,
    TY_I64,
    TY_U8,
    TY_U16,
    TY_U32,
    TY_U64,
    TY_F32,
    TY_F64,
    TY_BOOL,
    TY_VOID,
    TY_CHAR,
    // Operators - arithmetic
    PLUS,
    MINUS,
    STAR,
    SLASH,
    PERCENT,
    // Operators - bitwise
    AMPERSAND,
    PIPE,
    CARET,
    TILDE,
    LSHIFT,
    RSHIFT,
    // Operators - logical
    BANG,
    AND_AND,
    OR_OR,
    // Operators - comparison
    EQ_EQ,
    BANG_EQ,
    LT,
    LT_EQ,
    GT,
    GT_EQ,
    // Operators - assignment
    EQ,
    PLUS_EQ,
    MINUS_EQ,
    STAR_EQ,
    SLASH_EQ,
    PERCENT_EQ,
    AMP_EQ,
    PIPE_EQ,
    CARET_EQ,
    LSHIFT_EQ,
    RSHIFT_EQ,
    // Punctuation
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    LBRACKET,
    RBRACKET,
    SEMICOLON,
    COLON,
    COLON_COLON,
    COMMA,
    DOT,
    DOT_DOT,
    ARROW,
    FAT_ARROW,
    AT,
    HASH,
    DOLLAR,
    // Special
    EOF_TOKEN,
    ERROR,
};

struct SourceLocation {
    const char* file;
    uint32_t    line;
    uint32_t    col;
};

struct Token {
    TokenKind      kind;
    std::string    text;
    SourceLocation loc;
    union {
        int64_t  int_val;
        double   float_val;
    };
    char char_val = '\0';
    bool bool_val = false;
    std::string str_val;

    Token() : kind(TokenKind::ERROR), int_val(0) {}
    Token(TokenKind k, std::string t, SourceLocation l)
        : kind(k), text(std::move(t)), loc(l), int_val(0) {}

    bool is(TokenKind k)          const { return kind == k; }
    bool is_type_keyword()        const;
    bool is_assignment_op()       const;
    bool is_binary_op()           const;
    const char* kind_name()       const;
};

} // namespace cminus
