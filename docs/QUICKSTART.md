# C- Quick Start Guide

## Installation

```bash
git clone https://github.com/6876h9/C-.git
cd C-
make
```

## Your First Program

Create `hello.cm`:
```c-
extern fn puts(s: *i8) -> i32;

fn main() -> void {
    puts("Hello, C-!");
}
```

Compile:
```bash
./cminus hello.cm -S -o hello.s
```

View assembly:
```bash
cat hello.s
```

## Basic Syntax

### Variables

```c-
let x = 10;           // Type inferred as i32
let y: i32 = 20;      // Explicit type
let mut z = 30;       // Mutable variable
```

### Functions

```c-
fn add(a: i32, b: i32) -> i32 {
    ret a + b;
}

fn main() -> void {
    let result = add(5, 3);
}
```

### Control Flow

```c-
// If/else
if x > 10 {
    // true branch
} else {
    // false branch
};

// While loop
while x < 100 {
    x = x + 1;
};

// For loop
for i in 0..10 {
    // i from 0 to 9
};

// Infinite loop
loop {
    break;  // Exit loop
};
```

### Structs

```c-
struct Point {
    x: i32,
    y: i32,
};

fn main() -> void {
    let p = Point { .x = 5, .y = 10 };
    let x_val = p.x;
}
```

### Pointers

```c-
fn main() -> void {
    let x: i32 = 42;
    let ptr: *i32 = &x;      // Take address
    let value = *ptr;         // Dereference
    let arr: [i32; 10];
    let elem = arr[0];        // Index
}
```

## Compiler Flags

```bash
./cminus input.cm                    # Generate executable (a.out)
./cminus input.cm -o output          # Specify output name
./cminus input.cm -S                 # Assembly only
./cminus input.cm -dump-tokens       # Show tokens
./cminus input.cm -dump-ast          # Show AST
./cminus input.cm -v                 # Verbose
```

## Common Patterns

### Function Declaration

```c-
fn name(param1: type1, param2: type2) -> returntype {
    // body
}
```

### Variable Declaration

```c-
let var: type = value;
let mut mutable_var = initial;
```

### Assignment

```c-
x = value;
x += 1;
x -= 1;
x *= 2;
```

### Type Casting

```c-
let a: i64 = 100;
let b: i32 = cast<i32>(a);
```

### External Function Calls

```c-
extern fn printf(fmt: *i8, ...) -> i32;

fn main() -> void {
    printf("Value: %d\n", 42);
}
```

## Data Types

| Type | Size | Signed |
|------|------|--------|
| i8 | 1 byte | Yes |
| u8 | 1 byte | No |
| i16 | 2 bytes | Yes |
| u16 | 2 bytes | No |
| i32 | 4 bytes | Yes |
| u32 | 4 bytes | No |
| i64 | 8 bytes | Yes |
| u64 | 8 bytes | No |
| f32 | 4 bytes | Yes (signed) |
| f64 | 8 bytes | Yes (signed) |
| bool | 1 byte | - |
| char | 1 byte | - |
| void | 0 bytes | - |

## Operators

| Category | Operators |
|----------|-----------|
| Arithmetic | `+` `-` `*` `/` `%` |
| Bitwise | `&` `\|` `^` `~` `<<` `>>` |
| Logical | `&&` `\|\|` `!` |
| Comparison | `==` `!=` `<` `<=` `>` `>=` |
| Unary | `-` `!` `~` `&` `*` |

## Examples

See the `examples/` directory:
- `hello.cm` - Basic function and external call
- `arithmetic.cm` - Arithmetic operations
- `loops.cm` - Loop constructs
- `structs.cm` - Struct usage
- `pointers.cm` - Pointer operations
- `fibonacci.cm` - Function composition

## Tips

1. All statements end with semicolon
2. Block statements (if, while, loop) followed by `;`
3. Return type is required for functions
4. Types are explicit in function parameters
5. No implicit type conversions except numeric widening
6. Pointers are explicit with `*` prefix

## Troubleshooting

**Error: "expected type"**
- Check function parameter types are specified
- Ensure variable declarations have types (explicit or inferred from initialization)

**Error: "undefined symbol"**
- Check function names are declared before use
- Ensure external functions are declared with `extern fn`

**Error: "type mismatch"**
- Check assignment types match
- Use explicit casting with `cast<type>(value)`

**Error: "expected ';'"**
- All statements need semicolons
- Block statements (if/else) also need `;` after closing `}`

## Next Steps

1. Read `LANGUAGE.md` for complete language specification
2. Check `BUILDING.md` for build options
3. Review `CONTRIBUTING.md` for architecture details
4. Explore `examples/` for code samples
