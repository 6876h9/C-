# C-

A low-level systems programming language compiler written in C++17, targeting x86_64 architecture.

## Overview

C- is a minimal yet functional programming language that compiles directly to x86_64 machine code. It combines C-style syntax with modern language features, focusing on performance and control.

## Features

- **Primitive Types**: i8, i16, i32, i64, u8, u16, u32, u64, f32, f64, bool, char, void
- **Pointers**: Both immutable and mutable pointer types
- **Arrays & Slices**: Fixed-size arrays and slice types
- **Structs**: User-defined data types with field access
- **Functions**: First-class functions with variadic support
- **Control Flow**: if/else, while, loop, for-in range loops, break, continue
- **Operators**: Arithmetic, bitwise, logical, comparison, assignment
- **Inline Assembly**: Direct x86_64 assembly integration
- **Type Safety**: Static type checking with inference

## Language Syntax

### Variables
```c-
let x: i32 = 42;
let mut y = 100;
```

### Functions
```c-
fn add(a: i32, b: i32) -> i32 {
    ret a + b;
};

fn main() -> void {
    let result = add(10, 20);
};
```

### Structs
```c-
struct Point {
    x: i32,
    y: i32,
};

fn create_point() -> void {
    let p = Point { .x = 5, .y = 10 };
};
```

### Control Flow
```c-
fn example() -> void {
    if condition {
        // true branch
    } else {
        // false branch
    };
    
    while x < 100 {
        x = x + 1;
    };
    
    for i in 0..10 {
        // loop body
    };
};
```

### Pointers & References
```c-
let ptr: *i32 = &x;
let value = *ptr;
```

## Building

### Using CMake
```bash
mkdir build
cd build
cmake ..
make
```

### Using Make
```bash
make
```

### Using g++ directly
```bash
g++ -std=c++17 -Wall -Wextra -O2 *.cpp -o cminus
```

## Usage

```bash
./cminus input.cm -o output.s      # Generate assembly
./cminus input.cm -S               # Assembly only (default output: a.out.s)
./cminus input.cm -c               # Object file
./cminus input.cm -dump-tokens     # Debug: show tokens
./cminus input.cm -dump-ast        # Debug: show AST
./cminus input.cm -v               # Verbose output
```

## Compiler Pipeline

1. **Lexing** (lexer.cpp): Tokenizes source code
2. **Parsing** (parser.cpp): Builds abstract syntax tree
3. **Type Checking** (typechecker.cpp): Validates types and scopes
4. **Code Generation** (codegen.cpp): Emits x86_64 AT&T syntax assembly

## Project Structure

```
cminus/
├── token.h              # Token definitions
├── ast.h                # AST node structures
├── lexer.h/cpp          # Lexical analysis
├── parser.h/cpp         # Syntax analysis
├── typechecker.h/cpp    # Semantic analysis & type checking
├── codegen.h/cpp        # x86_64 code generation
├── driver.h/cpp         # Compiler driver & CLI
├── main.cpp             # Entry point
├── CMakeLists.txt       # CMake configuration
├── Makefile             # GNU Make configuration
└── README.md            # This file
```

## Example Programs

### Hello World (pseudo)
```c-
extern fn puts(s: *i8) -> i32;

fn main() -> void {
    puts("Hello, World!");
};
```

### Fibonacci
```c-
fn fib(n: i32) -> i32 {
    if n <= 1 {
        ret n;
    };
    ret fib(n - 1) + fib(n - 2);
};

fn main() -> void {
    let result = fib(10);
};
```

## Type System

- **Integers**: Signed (i8, i16, i32, i64) and unsigned (u8, u16, u32, u64)
- **Floats**: f32, f64
- **Boolean**: bool
- **Char**: char
- **Pointers**: *T and *mut T
- **Arrays**: [T; N]
- **Slices**: [T]
- **Structs**: Named composite types
- **Functions**: fn(T1, T2) -> R

## Assembly Output

The compiler generates AT&T syntax x86_64 assembly compatible with GNU as (gas). Output includes:
- Function prologues/epilogues
- Stack frame management
- Register allocation
- System V AMD64 ABI calling convention compliance

## Limitations

- No heap allocation or memory management (planned)
- No generics or polymorphism (planned)
- No pattern matching (planned)
- Limited library support
- x86_64 Linux target only

## Future Enhancements

- [ ] LLVM backend support
- [ ] Generic types and templates
- [ ] Pattern matching
- [ ] Standard library
- [ ] Module system
- [ ] Error handling
- [ ] Additional architecture targets

## Development

To extend the compiler:

1. **Add keywords**: Modify `Lexer::KEYWORDS` in lexer.cpp
2. **Add types**: Extend `PrimitiveType` enum in ast.h
3. **Add AST nodes**: Extend `Expr::Kind`, `Stmt::Kind`, or `Decl::Kind`
4. **Implement parsing**: Add parsing methods in parser.cpp
5. **Implement type checking**: Add type checking in typechecker.cpp
6. **Implement code generation**: Add codegen in codegen.cpp

## License

Open source. Use freely in your projects.

## Notes

- This is a systems programming language focused on low-level control
- Performance is a primary design goal
- Safety mechanisms are minimal; undefined behavior is user responsibility
- Assembly generation is unoptimized but functional
