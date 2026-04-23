# C- Language Specification

## Overview

C- is a systems programming language targeting x86_64 with C-like syntax and modern type safety.

## Lexical Elements

### Keywords

```
fn       let      mut      ret      if       else
loop     while    for      in       break    continue
struct   extern   asm      import   as       null
sizeof   cast     pub
```

### Primitive Types

**Signed Integers**: `i8`, `i16`, `i32`, `i64`
**Unsigned Integers**: `u8`, `u16`, `u32`, `u64`
**Floating Point**: `f32`, `f64`
**Other**: `bool`, `char`, `void`

### Operators

| Category | Operators |
|----------|-----------|
| Arithmetic | `+`, `-`, `*`, `/`, `%` |
| Bitwise | `&`, `\|`, `^`, `~`, `<<`, `>>` |
| Logical | `&&`, `\|\|`, `!` |
| Comparison | `==`, `!=`, `<`, `<=`, `>`, `>=` |
| Assignment | `=`, `+=`, `-=`, `*=`, `/=`, `%=`, `&=`, `\|=`, `^=`, `<<=`, `>>=` |
| Unary | `-`, `!`, `~`, `&`, `*` |

## Type System

### Primitive Types

Basic scalar types with known sizes:
- i8/u8: 1 byte
- i16/u16: 2 bytes
- i32/u32: 4 bytes
- i64/u64: 8 bytes
- f32: 4 bytes
- f64: 8 bytes
- bool: 1 byte
- char: 1 byte
- void: 0 bytes

### Pointers

```c-
let ptr: *i32;           // Immutable pointer to i32
let mut_ptr: *mut i32;   // Mutable pointer to i32
```

### Arrays

```c-
let arr: [i32; 10];      // Fixed array of 10 i32s
let slice: [i32];        // Slice of i32s
```

### Structs

```c-
struct Point {
    x: i32,
    y: i32,
};

let p = Point { .x = 5, .y = 10 };
let val = p.x;
```

### Function Pointers

```c-
let fn_ptr: fn(i32, i32) -> i32;
```

## Declarations

### Functions

```c-
fn add(a: i32, b: i32) -> i32 {
    ret a + b;
}

fn no_return() -> void {
    // code without return
}

// Variadic function (if supported)
fn printf(fmt: *i8, ...) -> i32;
```

### External Functions

```c-
extern fn puts(s: *i8) -> i32;
extern fn malloc(size: u64) -> *void;
```

### Structs

```c-
struct Rectangle {
    width: i32,
    height: i32,
};
```

### Global Variables

```c-
let CONSTANT: i32 = 42;
let mut global_var: i32 = 0;
```

## Statements

### Block

```c-
{
    // statements
}
```

### Let (Variable Declaration)

```c-
let x = 10;                    // Type inferred
let y: i32 = 20;              // Explicit type
let mut z: i32 = 30;          // Mutable variable
```

### Return

```c-
ret 42;
ret;  // Returns void
```

### If/Else

```c-
if condition {
    // statements
} else {
    // statements
};
```

### While Loop

```c-
while x < 100 {
    x = x + 1;
};
```

### Infinite Loop

```c-
loop {
    // statements
    break;  // To exit
};
```

### For Loop

```c-
for i in 0..10 {
    // i goes from 0 to 9
};

for i in 0..100 {
    if i == 50 {
        break;
    };
};
```

### Break/Continue

```c-
while true {
    if some_condition {
        break;
    };
    if other_condition {
        continue;
    };
};
```

### Inline Assembly

```c-
asm("mov rax, 0");
let result = asm("mov rax, 42": "=r" (x));
```

## Expressions

### Literals

```c-
42           // Integer (i32)
3.14         // Float (f64)
true false   // Boolean
'a'          // Character
"hello"      // String (pointer to u8)
null         // Null pointer
```

### Number Formats

```c-
0xFF         // Hexadecimal
0b1010       // Binary
1_000_000    // With underscores
3.14e-2      // Scientific notation
```

### Operators

Binary operators (left-associative):
```c-
a + b
a - b
a * b
a / b
a % b
a & b
a | b
a ^ b
a << b
a >> b
a && b
a || b
a == b
a != b
a < b
a <= b
a > b
a >= b
```

Unary operators (right-associative):
```c-
-x
!x
~x
&x           // Address-of
*x           // Dereference
```

### Assignment

```c-
x = 5;
x += 1;
x -= 1;
x *= 2;
x /= 2;
x %= 2;
```

### Function Calls

```c-
add(1, 2)
puts("hello")
```

### Array/Pointer Indexing

```c-
arr[i]
ptr[0]
```

### Struct Field Access

```c-
point.x
point.y
```

### Pointer Field Access

```c-
ptr->x
ptr->y
```

### Type Casting

```c-
cast<i32>(x)
cast<*i8>(ptr)
```

### Sizeof

```c-
sizeof(i32)
sizeof([i32; 10])
```

## Memory Management

C- does not provide automatic memory management. Developers are responsible for:
- Manual allocation (via `extern fn malloc(...)`)
- Manual deallocation (via `extern fn free(...)`)
- Pointer validity

## Scope and Lifetime

Variables are function-scoped or block-scoped:
```c-
fn example() -> void {
    {
        let x = 1;  // x is scoped to this block
    }
    // x is out of scope here
}
```

## Type Conversions

### Numeric Widening

Implicit widening from smaller to larger types:
```c-
let a: i8 = 10;
let b: i32 = a;  // Valid: widening
```

### Explicit Casting

```c-
let a: i64 = 100;
let b: i32 = cast<i32>(a);
```

### Pointer to Pointer

```c-
let void_ptr: *void = ptr;
let typed_ptr: *i32 = cast<*i32>(void_ptr);
```

## Calling Convention

C- follows System V AMD64 ABI on x86_64:
- First 6 integer args: RDI, RSI, RDX, RCX, R8, R9
- Return value: RAX (for integers), RDX:RAX (for 128-bit)
- Caller cleans up stack

## Error Handling

C- does not have exceptions. Error handling is via:
- Return values
- Out parameters
- Null pointers
