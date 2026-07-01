# Contributing to C-

## Architecture Overview

The compiler follows a traditional multi-pass pipeline:

```
Source Code
    ↓
Lexer (lexer.cpp) - Tokenization
    ↓
Parser (parser.cpp) - AST Construction
    ↓
Type Checker (typechecker.cpp) - Semantic Analysis
    ↓
Code Generator (codegen.cpp) - x86_64 Assembly
    ↓
Assembly Output (.s file)
```

## File Structure

- `token.h` - Token definitions and utilities
- `ast.h` - AST node definitions
- `lexer.h/cpp` - Lexical analysis
- `parser.h/cpp` - Syntax analysis & parsing
- `typechecker.h/cpp` - Type checking & symbol resolution
- `codegen.h/cpp` - x86_64 code generation
- `driver.h/cpp` - Compiler driver & CLI
- `main.cpp` - Entry point

## Adding New Features

### 1. Add a Keyword

Edit `lexer.cpp`, add to `Lexer::KEYWORDS`:
```cpp
{"keyword", TokenKind::KW_KEYWORD},
```

Then add the token kind to `TokenKind` enum in `token.h`.

### 2. Add an Operator

Add to `TokenKind` enum, then add lexer recognition in `Lexer::next_token()`.

### 3. Add a Statement Type

1. Add to `Stmt::Kind` enum in `ast.h`
2. Add fields to `Stmt` struct in `ast.h`
3. Add parsing method `Parser::parse_xyz()` in `parser.h/cpp`
4. Call it from `Parser::parse_stmt()` in `parser.cpp`
5. Add type checking in `TypeChecker::check_xyz()` in `typechecker.h/cpp`
6. Add code generation in `Codegen::gen_xyz()` in `codegen.h/cpp`

### 4. Add an Expression Type

1. Add to `Expr::Kind` enum in `ast.h`
2. Add fields to `Expr` struct in `ast.h`
3. Add parsing (usually in `Parser::parse_primary()` or postfix chain)
4. Add type checking in `TypeChecker::check_expr()`
5. Add code generation in `Codegen::gen_expr()`

### 5. Add a Primitive Type

1. Add to `PrimitiveType` enum in `ast.h`
2. Add to `Lexer::KEYWORDS` with TY_ prefix
3. Add handling in `Parser::parse_type()`
4. Update `Type::size_bytes()` in `typechecker.cpp`
5. Update `Type::align_bytes()` in `typechecker.cpp`
6. Update `Type::to_string()` in `typechecker.cpp`

## Code Style

- Use 4-space indentation
- Class members prefixed with `m_`
- Use meaningful names
- Prefer `std::unique_ptr` for AST nodes
- Prefer `std::shared_ptr` for type information
- Use lambdas sparingly

## Error Handling

- Print errors to `stderr` with format: `file:line:col: error: message`
- Set error flag instead of throwing exceptions
- Continue parsing after errors when possible (error recovery)

## Testing

Create `.cm` files in `examples/` directory:

```c-
fn test() -> i32 {
    ret 42;
}
```

Test compilation:
```bash
./cminus examples/test.cm -v
```

Test assembly output:
```bash
./cminus examples/test.cm -S -o output.s
cat output.s
```

## Performance Considerations

- Register allocation is linear scan (simple but effective)
- No peephole optimization
- No dead code elimination
- No constant folding
- No loop unrolling

Future improvements should focus on these areas.

## Known Limitations

1. **No optimization** - Assembly output is straightforward, unoptimized
2. **Simple register allocation** - Uses first 6 GP registers
3. **No inline functions** - All functions compiled separately
4. **Limited type system** - No generics, no traits
5. **Stack-based locals** - No register allocation for locals
6. **No linker integration** - Only generates `.s` files

## Debugging

Enable verbose output:
```bash
./cminus -dump-tokens input.cm    # Show tokens
./cminus -dump-ast input.cm       # Show AST structure
```

## Building for Development

With debug symbols:
```bash
g++ -std=c++17 -Wall -Wextra -g *.cpp -o cminus
gdb ./cminus examples/hello.cm
```

## Before Submitting

1. Code compiles without warnings
2. All example files parse correctly
3. Assembly generation works
4. No regression in existing tests
5. Code follows style guidelines

## Questions?

Refer to the LANGUAGE.md for language specification and BUILDING.md for build instructions.
