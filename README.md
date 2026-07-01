# C-minus Compiler

A minimal, low-level systems programming language that compiles to x86-64 assembly with built-in graphics support and static memory safety analysis.

## Overview

C-minus is designed to bridge the gap between high-level programming convenience and low-level control. It provides a simple syntax without semicolons, explicit type inference, and integrated OpenGL graphics capabilities while maintaining freestanding operation on x86-64 architecture.

### Key Features

- **Newline-terminated statements** — No semicolons required. Statements end at newline.
- **Type inference** — Variable types inferred from initialization values: `x = (42)` infers int.
- **Simplified type system** — Four primitive types: int (64-bit), float (64-bit IEEE 754), char, void.
- **Struct support** — User-defined structures with field access via `^` dereference operator.
- **Graphics integration** — Built-in OpenGL API with graphics protocol initialization.
- **Memory safety analysis** — Static analysis detects null pointer dereference, buffer overflow, uninitialized variables.
- **Freestanding operation** — No standard library dependency. Stack-only memory allocation.
- **x86-64 code generation** — Direct assembly emission targeting System V AMD64 ABI.

## Language Specification

### Variable Declaration

```c
x = (42)
y = (3.14)
name = ("hello")
count: int = (100)
```

Type is inferred from the initialization value. Explicit type annotation uses colon syntax.

### Struct Definition and Instantiation

```c
struct Point {
    x = (0)
    y = (0)
}

p = *(100, 200)
print(^p.x)
```

Struct fields are initialized with default values. Instantiation uses `*` operator with positional arguments. Field access requires `^` dereference.

### Functions

```c
func main() {
    x = (10)
}

func add(a: int, b: int) {
    result = (a + b)
}
```

Functions declared with `func` keyword. Parameters require explicit type annotation. Functions default to void return type.

### Control Flow

```c
if x > 5 {
    y = (1)
} else {
    y = (0)
}

while count > 0 {
    count = (count - 1)
}

for i = (0); i < 10; i = (i + 1) {
    sum = (sum + i)
}
```

Standard if/else, while, and for loops. All conditions must be expressions.

### Graphics API

```c
grp.var.init()
opengl.screen.start(1920, 1080, "1080p")
opengl.screen.vector(100, 200, grp.var.init(coord))
opengl.screen.draw(grp.var.call(coord), color[255, 0, 0])
```

Graphics operations require initialization via `grp.var.init()`. Coordinates stored via `init()`, retrieved via `call()`. Colors specified as RGB values in brackets.

## Project Structure

```
C-minus/
├── lexer.cpp / lexer.h      — Tokenization with newline handling
├── parser.cpp / parser.h    — AST construction from token stream
├── ast.h                    — Abstract syntax tree definitions
├── typechecker.cpp / .h     — Type inference and memory safety analysis
├── codegen.cpp / codegen.h  — x86-64 assembly code generation
├── driver.cpp / driver.h    — Compiler driver and entry point
├── token.h                  — Token type enumeration
├── main.cpp                 — CLI entry point
├── CMakeLists.txt           — Build configuration
├── Makefile                 — Alternative build
├── examples/                — Example C-minus programs
├── cminus_codex_prompt.md   — Language specification for AI tools
├── COMPILER_CHANGES.md      — Summary of implementation changes
└── PARSER_IMPLEMENTATION_ROADMAP.md — Step-by-step parser completion guide
```

## Build Instructions

### Requirements

- C++17 or later
- CMake 3.15+
- NASM (for assembly processing)
- Standard C library

### Compilation

```bash
mkdir build
cd build
cmake ..
make
```

### Running

```bash
./cminus input.cm -o output.s
```

Produces x86-64 assembly file. Link with C runtime:

```bash
gcc output.s -o executable
./executable
```

## Implementation Status

### Completed

- Lexer: Full tokenization with newline-aware statement termination
- Token definitions: All C-minus token types defined
- AST: Simplified primitive type system (int, float, char, void)
- Parser: Entry points updated for func/struct declarations
- Core infrastructure: Driver, module handling, error reporting

### In Progress

- Statement-level parsing with newline handling
- Variable declaration parsing: `VAR = (VALUE)` syntax
- Struct instantiation: `struct Name s = *(field1, field2)`
- Struct field dereference: `^instance.field`
- Graphics API method chaining

### Not Started

- Type inference engine
- Memory safety analysis (null pointer, buffer overflow detection)
- Code generation for x86-64
- Optimization passes

## Parser Implementation Roadmap

Six phases remain for complete parser implementation:

1. **Block parsing** — Handle newline-terminated statements within braces
2. **Variable declaration** — Parse `VAR = (VALUE)` and `VAR: TYPE = (VALUE)`
3. **Struct instantiation** — Parse `struct Name s = *(values)`
4. **Struct field access** — Parse `^instance.field`
5. **Graphics API** — Automatic via postfix expression parsing
6. **Type parsing** — Simplify to int, float, char, void only

Detailed implementation guide with code samples available in `PARSER_IMPLEMENTATION_ROADMAP.md`.

## Example Program

```c
struct Vertex {
    x = (0.0)
    y = (0.0)
    z = (0.0)
}

func render_triangle(v1: Vertex, v2: Vertex, v3: Vertex) {
    grp.var.init()
    opengl.screen.start(800, 600, "1080p")
    
    opengl.screen.vector(^v1.x, ^v1.y, grp.var.init(vert1))
    opengl.screen.draw(grp.var.call(vert1), color[255, 0, 0])
    
    opengl.screen.vector(^v2.x, ^v2.y, grp.var.init(vert2))
    opengl.screen.draw(grp.var.call(vert2), color[0, 255, 0])
    
    opengl.screen.vector(^v3.x, ^v3.y, grp.var.init(vert3))
    opengl.screen.draw(grp.var.call(vert3), color[0, 0, 255])
}

func main() {
    v1 = *(100.0, 100.0, 0.0)
    v2 = *(200.0, 100.0, 0.0)
    v3 = *(150.0, 200.0, 0.0)
    
    render_triangle(v1, v2, v3)
}
```

## Contributing

Development focuses on completing the parser implementation. Follow the roadmap in `PARSER_IMPLEMENTATION_ROADMAP.md` for specific implementation tasks.

Testing should include:
- Lexer unit tests for each token type
- Parser tests for each language construct
- Type checker tests for inference and safety analysis
- Codegen tests verifying correct x86-64 assembly output

## Documentation

- `cminus_codex_prompt.md` — Complete language specification suitable for use with AI code generation tools
- `COMPILER_CHANGES.md` — Detailed summary of all code modifications and rationale
- `PARSER_IMPLEMENTATION_ROADMAP.md` — Phase-by-phase implementation guide with exact code samples

## License

MIT License. See LICENSE file for details.

## Author

6876h9 (GitHub handle)

## References

- System V AMD64 ABI specification
- x86-64 instruction set reference
- OpenGL API documentation
