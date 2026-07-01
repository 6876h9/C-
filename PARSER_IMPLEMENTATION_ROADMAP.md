# C-minus Parser Implementation Roadmap

This document details the specific code changes needed to complete the C-minus compiler parser.

## Current State

**Completed:**
- Lexer: Newline tokenization, keyword set updated, primitive types simplified
- Token types: All C-minus tokens defined
- AST: Primitive types simplified to int, float, char, void
- Parser: Entry point updated, parse_decl() simplified, parse_fn() updated

**Incomplete:**
- Statement parsing with newline handling
- Variable declaration parsing: `VAR = (VALUE)` and `VAR: TYPE = (VALUE)`
- Struct instantiation parsing: `struct Name s = *(value1, value2)`
- Struct field dereference: `^instance.field`
- Graphics API parsing
- Block parsing with newline-aware statement boundaries

## Implementation Phases

### Phase 1: Statement-Level Parsing

**File:** `parser.cpp`

**Current parse_block():**
```cpp
StmtPtr Parser::parse_block() {
    // Existing implementation expects { STMTS }
}
```

**Required change:**
Modify to handle newline-terminated statements within `{}` braces:

```cpp
StmtPtr Parser::parse_block() {
    auto block = std::make_unique<Stmt>();
    block->kind = Stmt::Kind::Block;
    block->loc = peek().loc;

    expect(TokenKind::LBRACE, "expected '{'");
    
    // Skip initial newlines
    while (match(TokenKind::NEWLINE)) {}

    while (!check(TokenKind::RBRACE) && !at_end()) {
        auto stmt = parse_stmt();
        if (stmt) block->stmts.push_back(std::move(stmt));
        
        // Consume trailing newlines
        while (match(TokenKind::NEWLINE)) {}
    }

    expect(TokenKind::RBRACE, "expected '}'");
    return block;
}
```

**Rationale:**
Statements end at newline. The parser must skip NEWLINE tokens between statements while respecting block boundaries (braces).

### Phase 2: Variable Declaration Parsing

**File:** `parser.cpp`

**Add new parsing function:**
```cpp
StmtPtr Parser::parse_var_decl() {
    // Parse: VAR = (VALUE)
    // Parse: VAR: TYPE = (VALUE)
    
    auto stmt = std::make_unique<Stmt>();
    stmt->kind = Stmt::Kind::Let;
    stmt->loc = peek().loc;
    
    std::string var_name = expect(TokenKind::IDENT, "expected variable name").text;
    
    // Optional type annotation: VAR: TYPE
    TypeExprPtr type_expr = nullptr;
    if (match(TokenKind::COLON)) {
        type_expr = parse_type();
    }
    
    expect(TokenKind::EQ, "expected '=' in variable declaration");
    
    // Parse initialization value
    expect(TokenKind::LPAREN, "expected '(' after '='");
    auto value = parse_expr();
    expect(TokenKind::RPAREN, "expected ')' after initialization value");
    
    // If no type annotation, infer from value during type checking
    stmt->var_name = var_name;
    stmt->var_type = std::move(type_expr);
    stmt->expr = std::move(value);
    
    return stmt;
}
```

**Modify parse_stmt() to call parse_var_decl():**
```cpp
StmtPtr Parser::parse_stmt() {
    // Skip newlines (already consumed by parse_block, but handle edge cases)
    while (match(TokenKind::NEWLINE)) {}
    
    if (at_end() || check(TokenKind::RBRACE)) return nullptr;
    
    // Detect variable declaration: IDENT = (...)
    if (check(TokenKind::IDENT)) {
        size_t saved_pos = m_pos;
        advance(); // skip IDENT
        if (check(TokenKind::EQ) || check(TokenKind::COLON)) {
            m_pos = saved_pos; // rewind
            return parse_var_decl();
        }
        m_pos = saved_pos; // rewind for expression parsing
    }
    
    // Existing control flow parsing
    if (match(TokenKind::KW_RETURN))  return parse_return();
    if (match(TokenKind::KW_IF))      return parse_if();
    if (match(TokenKind::KW_WHILE))   return parse_while();
    if (match(TokenKind::KW_FOR))     return parse_for();
    if (match(TokenKind::KW_BREAK))   return parse_break();
    if (match(TokenKind::KW_CONTINUE)) return parse_continue();
    
    // Expression statement
    auto expr_stmt = std::make_unique<Stmt>();
    expr_stmt->kind = Stmt::Kind::Expr;
    expr_stmt->loc = peek().loc;
    expr_stmt->expr = parse_expr();
    
    return expr_stmt;
}
```

### Phase 3: Struct Instantiation Parsing

**File:** `parser.cpp`

**Modify parse_expr() / parse_primary() to handle struct instantiation:**

```cpp
ExprPtr Parser::parse_primary() {
    // ... existing code ...
    
    // Detect struct instantiation: struct NAME = *(...)
    if (check(TokenKind::KW_STRUCT)) {
        auto struct_expr = std::make_unique<Expr>();
        struct_expr->kind = Expr::Kind::CompoundLit;
        struct_expr->loc = peek().loc;
        
        advance(); // consume 'struct'
        struct_expr->struct_name = expect(TokenKind::IDENT, "expected struct name").text;
        
        expect(TokenKind::EQ, "expected '=' after struct name");
        expect(TokenKind::STAR, "expected '*' for struct instantiation");
        expect(TokenKind::LPAREN, "expected '(' after '*'");
        
        // Parse field values as positional arguments
        size_t field_index = 0;
        while (!check(TokenKind::RPAREN) && !at_end()) {
            Expr::FieldInit field_init;
            field_init.name = "field_" + std::to_string(field_index++);
            field_init.value = parse_expr();
            struct_expr->fields.push_back(std::move(field_init));
            
            if (!match(TokenKind::COMMA)) break;
        }
        
        expect(TokenKind::RPAREN, "expected ')' after struct fields");
        return struct_expr;
    }
    
    // ... rest of parse_primary() ...
}
```

### Phase 4: Struct Field Dereference

**File:** `parser.cpp`

**Modify parse_postfix() to handle `^instance.field`:**

```cpp
ExprPtr Parser::parse_postfix(ExprPtr base) {
    while (true) {
        if (match(TokenKind::CARET)) {
            // ^ dereference for struct field access
            auto deref = std::make_unique<Expr>();
            deref->kind = Expr::Kind::Deref;
            deref->loc = peek().loc;
            deref->lhs = std::move(base);
            
            // Expect .fieldname after ^
            if (match(TokenKind::DOT)) {
                auto field_access = std::make_unique<Expr>();
                field_access->kind = Expr::Kind::Field;
                field_access->loc = peek().loc;
                field_access->object = std::move(deref);
                field_access->field = expect(TokenKind::IDENT, "expected field name").text;
                base = std::move(field_access);
            } else {
                // Just a dereference
                base = std::move(deref);
            }
            continue;
        }
        
        if (match(TokenKind::DOT)) {
            auto field_access = std::make_unique<Expr>();
            field_access->kind = Expr::Kind::Field;
            field_access->loc = peek().loc;
            field_access->object = std::move(base);
            field_access->field = expect(TokenKind::IDENT, "expected field name").text;
            base = std::move(field_access);
            continue;
        }
        
        // ... rest of postfix parsing (function calls, array indexing, etc.) ...
        break;
    }
    return base;
}
```

### Phase 5: Graphics API Parsing

**File:** `parser.cpp`

**Modify parse_postfix() to handle `grp.var.init()` chains:**

Graphics API calls are method chains on identifiers. They parse as regular method calls:
```
grp.var.init()
opengl.screen.start(width, height, quality)
```

These parse naturally as:
1. `grp` — identifier
2. `.var` — field access
3. `.init` — field access on result
4. `()` — function call

**No special parsing required** if postfix parsing handles dot notation and function calls correctly.

**Ensure parse_postfix() handles:**
```cpp
ExprPtr Parser::parse_postfix(ExprPtr base) {
    while (true) {
        // Function call: expr()
        if (check(TokenKind::LPAREN) && base) {
            base = parse_call(std::move(base));
            continue;
        }
        
        // Field access: expr.field
        if (match(TokenKind::DOT)) {
            auto field = std::make_unique<Expr>();
            field->kind = Expr::Kind::Field;
            field->loc = peek().loc;
            field->object = std::move(base);
            field->field = expect(TokenKind::IDENT, "expected field name").text;
            base = std::move(field);
            continue;
        }
        
        // ... other postfix operators ...
        break;
    }
    return base;
}
```

This allows: `grp.var.init()` to parse as:
1. `grp` (identifier)
2. `.var` (field access on grp)
3. `.init` (field access on var)
4. `()` (function call on init)

### Phase 6: Type Parsing Simplification

**File:** `parser.cpp`

**Update parse_type():**
```cpp
TypeExprPtr Parser::parse_type() {
    auto ty = std::make_unique<TypeExpr>();
    ty->loc = peek().loc;
    
    // Primitive types: int, float, char, void
    if (match(TokenKind::TY_INT)) {
        ty->kind = TypeExpr::Kind::Primitive;
        ty->primitive = PrimitiveType::Int;
        return ty;
    }
    if (match(TokenKind::TY_FLOAT)) {
        ty->kind = TypeExpr::Kind::Primitive;
        ty->primitive = PrimitiveType::Float;
        return ty;
    }
    if (match(TokenKind::TY_CHAR)) {
        ty->kind = TypeExpr::Kind::Primitive;
        ty->primitive = PrimitiveType::Char;
        return ty;
    }
    if (match(TokenKind::TY_VOID)) {
        ty->kind = TypeExpr::Kind::Primitive;
        ty->primitive = PrimitiveType::Void;
        return ty;
    }
    
    // Pointer: *T
    if (match(TokenKind::STAR)) {
        ty->kind = TypeExpr::Kind::Pointer;
        ty->inner = parse_type();
        return ty;
    }
    
    // Named type: MyStruct
    if (check(TokenKind::IDENT)) {
        ty->kind = TypeExpr::Kind::Named;
        ty->name = advance().text;
        return ty;
    }
    
    error(peek().loc, "expected type");
    return ty;
}
```

## Testing Plan

### Unit Tests

**Lexer Tests:**
- Newline tokenization
- Keyword recognition (func, struct, grp, opengl, color, etc.)
- Type keyword recognition (int, float, char, void)

**Parser Tests:**

**Variable Declaration:**
```c
x = (42)
y: float = (3.14)
name = ("hello")
```

**Struct Definition & Instantiation:**
```c
struct Point {
    x = (0)
    y = (0)
}
p = *(100, 200)
print(^p.x)
```

**Function Declaration:**
```c
func main() {
    x = (10)
}

func add(a: int, b: int) {
    return
}
```

**Graphics API:**
```c
grp.var.init()
opengl.screen.start(1920, 1080, "1080p")
opengl.screen.vector(100, 200, grp.var.init(coord))
opengl.screen.draw(grp.var.call(coord), color[255, 0, 0])
```

## Implementation Order

1. **Phase 1**: Statement-level parsing (necessary foundation)
2. **Phase 2**: Variable declaration parsing (most common operation)
3. **Phase 3**: Struct parsing (common data structure)
4. **Phase 4**: Struct field dereference (necessary for struct usage)
5. **Phase 5**: Graphics API (automatic from proper postfix parsing)
6. **Phase 6**: Type parsing (polish)

## Error Handling

All parse functions should:
1. Check preconditions with `expect()` or `match()`
2. Call `error()` with descriptive message and location
3. Return a valid AST node (may be incomplete) for error recovery
4. Allow `synchronize()` to skip to next statement on error

Example:
```cpp
expect(TokenKind::LPAREN, "expected '(' in grp.var.init");
if (!check(TokenKind::RPAREN)) {
    parse_expr(); // may fail, but we continue
}
expect(TokenKind::RPAREN, "expected ')' to close grp.var.init");
```

## Validation Checklist

- [ ] Lexer produces NEWLINE tokens
- [ ] Parser skips NEWLINE tokens in parse_block()
- [ ] Variables parse: `x = (42)` and `x: int = (42)`
- [ ] Struct definitions parse
- [ ] Struct instantiation parses: `p = *(100, 200)`
- [ ] Struct field access parses: `^p.x`
- [ ] Graphics API parses: `grp.var.init()`, `opengl.screen.start(w, h, q)`
- [ ] Function declarations parse with parameter types
- [ ] Control flow (if/else/while/for) parses correctly
- [ ] All error messages are actionable and precise
- [ ] Synchronization recovers from parse errors
