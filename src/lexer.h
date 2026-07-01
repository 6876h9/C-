#pragma once
#include "token.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace cminus {

class Lexer {
public:
    explicit Lexer(const char* filename, std::string source);
    std::vector<Token> tokenize();
    bool has_errors() const { return m_has_errors; }

private:
    Token next_token();
    Token lex_number();
    Token lex_string();
    Token lex_char();
    Token lex_ident_or_keyword();
    void  skip_whitespace();
    char  peek(int offset = 0) const;
    char  advance();
    bool  at_end() const;
    Token make(TokenKind k, std::string text = "");
    Token error_tok(const char* msg);
    SourceLocation current_loc() const;

    std::string  m_source;
    const char*  m_filename;
    size_t       m_pos    = 0;
    uint32_t     m_line   = 1;
    uint32_t     m_col    = 1;
    bool         m_has_errors = false;

    static const std::unordered_map<std::string, TokenKind> KEYWORDS;
};

} // namespace cminus
