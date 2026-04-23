#include "lexer.h"
#include "token.h"
#include <cassert>
#include <cstdio>
#include <stdexcept>
#include <cctype>

namespace cminus {

const std::unordered_map<std::string, TokenKind> Lexer::KEYWORDS = {
    {"fn",       TokenKind::KW_FN},
    {"let",      TokenKind::KW_LET},
    {"mut",      TokenKind::KW_MUT},
    {"ret",      TokenKind::KW_RET},
    {"if",       TokenKind::KW_IF},
    {"else",     TokenKind::KW_ELSE},
    {"loop",     TokenKind::KW_LOOP},
    {"while",    TokenKind::KW_WHILE},
    {"for",      TokenKind::KW_FOR},
    {"in",       TokenKind::KW_IN},
    {"break",    TokenKind::KW_BREAK},
    {"continue", TokenKind::KW_CONTINUE},
    {"struct",   TokenKind::KW_STRUCT},
    {"extern",   TokenKind::KW_EXTERN},
    {"asm",      TokenKind::KW_ASM},
    {"import",   TokenKind::KW_IMPORT},
    {"as",       TokenKind::KW_AS},
    {"null",     TokenKind::KW_NULL},
    {"sizeof",   TokenKind::KW_SIZEOF},
    {"cast",     TokenKind::KW_CAST},
    {"pub",      TokenKind::KW_PUB},
    {"true",     TokenKind::BOOL_LIT},
    {"false",    TokenKind::BOOL_LIT},
    {"i8",       TokenKind::TY_I8},
    {"i16",      TokenKind::TY_I16},
    {"i32",      TokenKind::TY_I32},
    {"i64",      TokenKind::TY_I64},
    {"u8",       TokenKind::TY_U8},
    {"u16",      TokenKind::TY_U16},
    {"u32",      TokenKind::TY_U32},
    {"u64",      TokenKind::TY_U64},
    {"f32",      TokenKind::TY_F32},
    {"f64",      TokenKind::TY_F64},
    {"bool",     TokenKind::TY_BOOL},
    {"void",     TokenKind::TY_VOID},
    {"char",     TokenKind::TY_CHAR},
};

bool Token::is_type_keyword() const {
    return kind >= TokenKind::TY_I8 && kind <= TokenKind::TY_CHAR;
}

bool Token::is_assignment_op() const {
    return kind >= TokenKind::EQ && kind <= TokenKind::RSHIFT_EQ;
}

bool Token::is_binary_op() const {
    return kind >= TokenKind::PLUS && kind <= TokenKind::OR_OR;
}

const char* Token::kind_name() const {
    switch (kind) {
#define K(x) case TokenKind::x: return #x;
        K(INTEGER_LIT) K(FLOAT_LIT) K(STRING_LIT) K(CHAR_LIT) K(BOOL_LIT)
        K(IDENT) K(KW_FN) K(KW_LET) K(KW_MUT) K(KW_RET) K(KW_IF) K(KW_ELSE)
        K(KW_LOOP) K(KW_WHILE) K(KW_FOR) K(KW_BREAK) K(KW_CONTINUE)
        K(KW_STRUCT) K(KW_EXTERN) K(KW_ASM) K(KW_IMPORT) K(KW_AS) K(KW_IN)
        K(KW_NULL) K(KW_SIZEOF) K(KW_CAST) K(KW_PUB)
        K(TY_I8) K(TY_I16) K(TY_I32) K(TY_I64)
        K(TY_U8) K(TY_U16) K(TY_U32) K(TY_U64)
        K(TY_F32) K(TY_F64) K(TY_BOOL) K(TY_VOID) K(TY_CHAR)
        K(PLUS) K(MINUS) K(STAR) K(SLASH) K(PERCENT)
        K(AMPERSAND) K(PIPE) K(CARET) K(TILDE) K(LSHIFT) K(RSHIFT)
        K(BANG) K(AND_AND) K(OR_OR)
        K(EQ_EQ) K(BANG_EQ) K(LT) K(LT_EQ) K(GT) K(GT_EQ)
        K(EQ) K(PLUS_EQ) K(MINUS_EQ) K(STAR_EQ) K(SLASH_EQ)
        K(PERCENT_EQ) K(AMP_EQ) K(PIPE_EQ) K(CARET_EQ)
        K(LSHIFT_EQ) K(RSHIFT_EQ)
        K(LPAREN) K(RPAREN) K(LBRACE) K(RBRACE) K(LBRACKET) K(RBRACKET)
        K(SEMICOLON) K(COLON) K(COLON_COLON) K(COMMA) K(DOT) K(DOT_DOT)
        K(ARROW) K(FAT_ARROW) K(AT) K(HASH) K(DOLLAR)
        K(EOF_TOKEN) K(ERROR)
#undef K
        default: return "UNKNOWN";
    }
}

Lexer::Lexer(const char* filename, std::string source)
    : m_source(std::move(source)), m_filename(filename) {}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (true) {
        Token t = next_token();
        tokens.push_back(t);
        if (t.is(TokenKind::EOF_TOKEN)) break;
    }
    return tokens;
}

char Lexer::peek(int offset) const {
    size_t p = m_pos + offset;
    if (p >= m_source.size()) return '\0';
    return m_source[p];
}

char Lexer::advance() {
    char c = m_source[m_pos++];
    if (c == '\n') { m_line++; m_col = 1; }
    else            { m_col++; }
    return c;
}

bool Lexer::at_end() const {
    return m_pos >= m_source.size();
}

SourceLocation Lexer::current_loc() const {
    return { m_filename, m_line, m_col };
}

Token Lexer::make(TokenKind k, std::string text) {
    return Token(k, std::move(text), current_loc());
}

Token Lexer::error_tok(const char* msg) {
    m_has_errors = true;
    fprintf(stderr, "%s:%u:%u: error: %s\n", m_filename, m_line, m_col, msg);
    return make(TokenKind::ERROR, "");
}

void Lexer::skip_whitespace() {
    while (!at_end()) {
        char c = peek();
        if (std::isspace((unsigned char)c)) {
            advance();
        } else if (c == '/' && peek(1) == '/') {
            while (!at_end() && peek() != '\n') advance();
        } else if (c == '/' && peek(1) == '*') {
            advance(); advance();
            while (!at_end()) {
                if (peek() == '*' && peek(1) == '/') {
                    advance(); advance();
                    break;
                }
                advance();
            }
        } else {
            break;
        }
    }
}

Token Lexer::next_token() {
    skip_whitespace();

    if (at_end()) return make(TokenKind::EOF_TOKEN, "");

    SourceLocation loc = current_loc();
    char c = peek();

    if (std::isdigit((unsigned char)c)) return lex_number();
    if (c == '"') return lex_string();
    if (c == '\'') return lex_char();
    if (std::isalpha((unsigned char)c) || c == '_') return lex_ident_or_keyword();

    advance();
    switch (c) {
        case '(': return Token(TokenKind::LPAREN,    "(", loc);
        case ')': return Token(TokenKind::RPAREN,    ")", loc);
        case '{': return Token(TokenKind::LBRACE,    "{", loc);
        case '}': return Token(TokenKind::RBRACE,    "}", loc);
        case '[': return Token(TokenKind::LBRACKET,  "[", loc);
        case ']': return Token(TokenKind::RBRACKET,  "]", loc);
        case ';': return Token(TokenKind::SEMICOLON, ";", loc);
        case ',': return Token(TokenKind::COMMA,     ",", loc);
        case '@': return Token(TokenKind::AT,        "@", loc);
        case '#': return Token(TokenKind::HASH,      "#", loc);
        case '$': return Token(TokenKind::DOLLAR,    "$", loc);
        case '~': return Token(TokenKind::TILDE,     "~", loc);
        case '^':
            if (peek() == '=') { advance(); return Token(TokenKind::CARET_EQ, "^=", loc); }
            return Token(TokenKind::CARET, "^", loc);
        case '!':
            if (peek() == '=') { advance(); return Token(TokenKind::BANG_EQ,  "!=", loc); }
            return Token(TokenKind::BANG, "!", loc);
        case '=':
            if (peek() == '=') { advance(); return Token(TokenKind::EQ_EQ,    "==", loc); }
            if (peek() == '>') { advance(); return Token(TokenKind::FAT_ARROW,"=>", loc); }
            return Token(TokenKind::EQ, "=", loc);
        case '+':
            if (peek() == '=') { advance(); return Token(TokenKind::PLUS_EQ,  "+=", loc); }
            return Token(TokenKind::PLUS, "+", loc);
        case '-':
            if (peek() == '=') { advance(); return Token(TokenKind::MINUS_EQ, "-=", loc); }
            if (peek() == '>') { advance(); return Token(TokenKind::ARROW,    "->", loc); }
            return Token(TokenKind::MINUS, "-", loc);
        case '*':
            if (peek() == '=') { advance(); return Token(TokenKind::STAR_EQ,  "*=", loc); }
            return Token(TokenKind::STAR, "*", loc);
        case '/':
            if (peek() == '=') { advance(); return Token(TokenKind::SLASH_EQ, "/=", loc); }
            return Token(TokenKind::SLASH, "/", loc);
        case '%':
            if (peek() == '=') { advance(); return Token(TokenKind::PERCENT_EQ,"%=",loc); }
            return Token(TokenKind::PERCENT, "%", loc);
        case '&':
            if (peek() == '&') { advance(); return Token(TokenKind::AND_AND,  "&&", loc); }
            if (peek() == '=') { advance(); return Token(TokenKind::AMP_EQ,   "&=", loc); }
            return Token(TokenKind::AMPERSAND, "&", loc);
        case '|':
            if (peek() == '|') { advance(); return Token(TokenKind::OR_OR,    "||", loc); }
            if (peek() == '=') { advance(); return Token(TokenKind::PIPE_EQ,  "|=", loc); }
            return Token(TokenKind::PIPE, "|", loc);
        case '<':
            if (peek() == '<') {
                advance();
                if (peek() == '=') { advance(); return Token(TokenKind::LSHIFT_EQ, "<<=", loc); }
                return Token(TokenKind::LSHIFT, "<<", loc);
            }
            if (peek() == '=') { advance(); return Token(TokenKind::LT_EQ, "<=", loc); }
            return Token(TokenKind::LT, "<", loc);
        case '>':
            if (peek() == '>') {
                advance();
                if (peek() == '=') { advance(); return Token(TokenKind::RSHIFT_EQ, ">>=", loc); }
                return Token(TokenKind::RSHIFT, ">>", loc);
            }
            if (peek() == '=') { advance(); return Token(TokenKind::GT_EQ, ">=", loc); }
            return Token(TokenKind::GT, ">", loc);
        case ':':
            if (peek() == ':') { advance(); return Token(TokenKind::COLON_COLON, "::", loc); }
            return Token(TokenKind::COLON, ":", loc);
        case '.':
            if (peek() == '.') { advance(); return Token(TokenKind::DOT_DOT, "..", loc); }
            return Token(TokenKind::DOT, ".", loc);
        default: {
            char buf[64];
            snprintf(buf, sizeof(buf), "unexpected character '%c' (0x%02X)", c, (unsigned char)c);
            return error_tok(buf);
        }
    }
}

Token Lexer::lex_number() {
    SourceLocation loc = current_loc();
    std::string text;
    bool is_float = false;
    bool is_hex   = false;
    bool is_bin   = false;

    if (peek() == '0' && (peek(1) == 'x' || peek(1) == 'X')) {
        text += advance(); text += advance();
        is_hex = true;
        while (!at_end() && (std::isxdigit((unsigned char)peek()) || peek() == '_'))
            if (peek() != '_') text += advance(); else advance();
    } else if (peek() == '0' && (peek(1) == 'b' || peek(1) == 'B')) {
        text += advance(); text += advance();
        is_bin = true;
        while (!at_end() && (peek() == '0' || peek() == '1' || peek() == '_'))
            if (peek() != '_') text += advance(); else advance();
    } else {
        while (!at_end() && (std::isdigit((unsigned char)peek()) || peek() == '_'))
            if (peek() != '_') text += advance(); else advance();
        if (!at_end() && peek() == '.' && std::isdigit((unsigned char)peek(1))) {
            is_float = true;
            text += advance();
            while (!at_end() && (std::isdigit((unsigned char)peek()) || peek() == '_'))
                if (peek() != '_') text += advance(); else advance();
        }
        if (!at_end() && (peek() == 'e' || peek() == 'E')) {
            is_float = true;
            text += advance();
            if (!at_end() && (peek() == '+' || peek() == '-')) text += advance();
            while (!at_end() && std::isdigit((unsigned char)peek())) text += advance();
        }
    }

    Token t(is_float ? TokenKind::FLOAT_LIT : TokenKind::INTEGER_LIT, text, loc);
    if (is_float) {
        t.float_val = std::stod(text);
    } else if (is_hex) {
        t.int_val = static_cast<int64_t>(std::stoull(text.substr(2), nullptr, 16));
    } else if (is_bin) {
        t.int_val = static_cast<int64_t>(std::stoull(text.substr(2), nullptr, 2));
    } else {
        t.int_val = static_cast<int64_t>(std::stoull(text, nullptr, 10));
    }
    return t;
}

Token Lexer::lex_string() {
    SourceLocation loc = current_loc();
    advance();
    std::string val;
    while (!at_end() && peek() != '"') {
        if (peek() == '\\') {
            advance();
            switch (advance()) {
                case 'n':  val += '\n'; break;
                case 't':  val += '\t'; break;
                case 'r':  val += '\r'; break;
                case '\\': val += '\\'; break;
                case '"':  val += '"';  break;
                case '0':  val += '\0'; break;
                default:   val += '?';  break;
            }
        } else {
            val += advance();
        }
    }
    if (at_end()) return error_tok("unterminated string literal");
    advance();
    Token t(TokenKind::STRING_LIT, val, loc);
    t.str_val = val;
    return t;
}

Token Lexer::lex_char() {
    SourceLocation loc = current_loc();
    advance();
    char val = '\0';
    if (peek() == '\\') {
        advance();
        switch (advance()) {
            case 'n':  val = '\n'; break;
            case 't':  val = '\t'; break;
            case 'r':  val = '\r'; break;
            case '\\': val = '\\'; break;
            case '\'': val = '\''; break;
            case '0':  val = '\0'; break;
            default:   val = '?';  break;
        }
    } else {
        val = advance();
    }
    if (peek() != '\'') return error_tok("unterminated character literal");
    advance();
    Token t(TokenKind::CHAR_LIT, std::string(1, val), loc);
    t.char_val = val;
    t.int_val  = (int64_t)(unsigned char)val;
    return t;
}

Token Lexer::lex_ident_or_keyword() {
    SourceLocation loc = current_loc();
    std::string text;
    while (!at_end() && (std::isalnum((unsigned char)peek()) || peek() == '_'))
        text += advance();

    auto it = KEYWORDS.find(text);
    if (it != KEYWORDS.end()) {
        Token t(it->second, text, loc);
        if (it->second == TokenKind::BOOL_LIT)
            t.bool_val = (text == "true");
        return t;
    }
    return Token(TokenKind::IDENT, text, loc);
}

} // namespace cminus
