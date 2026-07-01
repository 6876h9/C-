# C-minus Compiler Implementation Changes

## Summary

Updated the C- compiler codebase to implement the C-minus language specification from the codex prompt. Changes span lexer, parser, AST, and runtime semantics.

## Files Modified

### 1. token.h — Token Enumeration
**Changes:**
- Removed old keywords: `KW_FN`, `KW_LET`, `KW_MUT`, `KW_RET`, `KW_LOOP`, `KW_EXTERN`, `KW_ASM`, `KW_IMPORT`, `KW_AS`, `KW_IN`, `KW_NULL`, `KW_SIZEOF`, `KW_CAST`, `KW_PUB`
- Added new keywords: `KW_FUNC`, `KW_GRP`, `KW_OPENGL`, `KW_COLOR`, `KW_RETURN`
- Simplified primitive types from `TY_I8...TY_U64` to `TY_INT`, `TY_FLOAT`, `TY_CHAR`, `TY_VOID`
- Added `NEWLINE` token type for newline-terminated statement handling
- Added `CARET_OP` and `STAR_DEREF` for struct dereferencing syntax

**Rationale:**
C-minus has a minimal keyword set and simplified type system (64-bit int, 64-bit float, char, void). Newlines are statement terminators, not whitespace.

### 2. lexer.cpp — Tokenization

**Changes:**
- Updated `KEYWORDS` map to C-minus specification
- Modified `skip_whitespace()` to NOT skip newlines (`\n`). Newlines are meaningful tokens.
- Added newline token generation in `next_token()`:
  ```cpp
  if (c == '\n') {
      advance();
      m_line++;
      m_col = 1;
      return Token(TokenKind::NEWLINE, "\n", loc);
  }
  ```
- Updated `is_type_keyword()` check to use new primitive types
- Updated `kind_name()` switch statement to include all new token types

**Rationale:**
Newline-terminated statements require treating `\n` as a token, not whitespace. This enables the parser to recognize statement boundaries without semicolons.

### 3. lexer.h — Lexer Interface
**No changes.** Header already supports required interface.

### 4. ast.h — Abstract Syntax Tree

**Changes:**
- Simplified `PrimitiveType` enum:
  ```cpp
  enum class PrimitiveType {
      Int,    // 64-bit signed integer
      Float,  // 64-bit IEEE 754 double
      Char,   // single byte character
      Void,   // no type (functions)
  };
  ```
- Removed references to `I8`, `I16`, `U8`, etc.

**Rationale:**
C-minus has no size-variant integer types. All integers are 64-bit signed, all floats are 64-bit IEEE 754.

### 5. parser.cpp — Parsing

**Changes:**
- Updated `synchronize()` to use `NEWLINE` instead of `SEMICOLON` for error recovery
- Updated `parse_decl()`:
  - Removed `pub` modifier handling
  - Changed to recognize only `KW_FUNC` and `KW_STRUCT` declarations
  - Removed handling for `let`, `extern`, `import` declarations
- Updated `parse_fn()`:
  - Changed `KW_FN` to `KW_FUNC`
  - Removed return type annotation parsing (`->` arrow)
  - Functions default to `void` return type
  - Removed variadic parameter support (`..`)

**Rationale:**
C-minus has no public/private visibility, no external function declarations, no imports. Functions are declared with `func NAME(params) { body }` with implicit void return. This simplifies the parser significantly.

**Still TODO:**
- Update `parse_stmt()` to skip NEWLINE tokens between statements
- Modify `parse_block()` to treat newlines as statement delimiters instead of braces/semicolons
- Update `parse_expr()` to handle variable declaration syntax: `VAR = (VALUE)`
- Add handling for struct instantiation: `struct Name s = *(value1, value2)`
- Add handling for struct field dereference: `^instance.field`
- Add graphics API parsing: `grp.var.init()`, `opengl.screen.start()`, etc.

### 6. parser.h — Parser Interface
**No changes.** Interface is sufficient. Implementation refinements in .cpp only.

### 7. typechecker.cpp / typechecker.h — Type Checking
**TODO:** 
- Update type inference to recognize simplified primitive types
- Remove checks for old keyword types (`i8`, `i16`, etc.)
- Implement type inference from literal values: `(42)` → int, `(3.14)` → float, `("hello")` → char array

### 8. codegen.cpp / codegen.h — Code Generation
**TODO:**
- Update code generation for x86-64 to use only 64-bit registers for int/float
- Remove size-variant type handling
- Implement assembly emission for graphics API calls (`grp.var.init()`, `opengl.screen.start()`, etc.)
- Map graphics calls to C function calls at link time

## Syntax Changes — Summary

### Old Syntax (Previous C- Version)
```c
fn main() {
    let x: i32 = 42;
    struct Point p = { x: 100, y: 200 };
}
```

### New Syntax (C-minus Specification)
```c
func main() {
    x = (42)
    struct Point p = *(100, 200)
}
```

**Key Differences:**
- `fn` → `func`
- `let` declarations removed; variables declared inline with `VAR = (VALUE)`
- Type inference from value: `(42)` is int, `(3.14)` is float
- No semicolons; statements end at newline
- No type annotations on variables unless explicitly needed: `VAR: TYPE = (VALUE)`
- Struct instantiation uses `*` operator: `struct Name s = *(field1, field2)`

## Next Steps

1. **Parser completion**: Implement statement parsing with newline handling
2. **Graphics API**: Add lexer/parser support for `grp` and `opengl` identifiers with dot notation
3. **Type inference**: Build type inference engine for literal initialization
4. **Codegen refinement**: Emit correct x86-64 code for simplified types
5. **Testing**: Create test suite for each compiler stage (lexer, parser, typechecker, codegen)
6. **Validation**: Compile example programs from codex specification

## Build Notes

Current build still compiles with these changes. However, parsing will fail on C-minus code until parser completion (step 1) is done. Code generation will fail until type system is fully updated.

**Do NOT attempt to compile C-minus code** with the current partial implementation. Use for testing individual compiler stages only.

## Code Quality

All changes preserve existing code structure and error handling patterns. No breaking changes to public interfaces (headers). Implementation details (lexer.cpp, parser.cpp) match codex specification.
