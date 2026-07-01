# C-minus Compiler Codex Prompt

You are building a C-minus compiler from scratch. This is a low-level systems language that compiles to x86-64 assembly with built-in graphics support and memory safety analysis.

## Language Specification

### 1. Syntax Rules

**Statement Termination:**
- All statements end at newline. No semicolons, no explicit terminators.
- Multi-line expressions are not supported. Keep expressions on a single line.

**Variable Declaration and Initialization:**
```
VARNAME = (VALUE)
VARNAME = (VALUE)  # type inferred from VALUE at compile time
gpa = (3.13)       # inferred as float
count = (42)       # inferred as int
name = ("hello")   # inferred as string/char array
```

**Type Annotation (explicit):**
```
VARNAME = (VALUE)
# Type is inferred. If you need explicit type, use:
VARNAME: TYPE = (VALUE)
count: int = (42)
```

**Struct Definition:**
```
struct StructName {
    field1 = (VALUE)
    field2 = (VALUE)
}
```

**Struct Instantiation:**
```
struct StructName instance = *(VALUE1, VALUE2)
# The * after = distinguishes instantiation from variable declaration
struct Point p = *(100, 200)
```

**Struct Field Access:**
```
^instance.fieldname    # ^ dereferences and accesses the field
print(^p.x)           # prints field x from struct p
```

**Function Declaration:**
```
func functionName(param1: TYPE, param2: TYPE) {
    # function body
    # statements end at newline
}

func addgpa(gpa: float) {
    gpa = gpa + 1.0
}

func main() {
    # entry point
}
```

**Function Calls:**
```
functionName(arg1, arg2)
addgpa(3.5)
```

**Control Flow:**
```
if CONDITION {
    # statements
}

if CONDITION {
    # statements
} else {
    # statements
}

while CONDITION {
    # statements
}

for INIT; CONDITION; UPDATE {
    # statements
}
```

**Comments:**
```
# single line comment
```

### 2. Type System

**Primitive Types:**
- `int` — 64-bit signed integer
- `float` — 64-bit IEEE 754 double
- `char` — single byte character
- `void` — no type (for functions that return nothing)

**Compound Types:**
- `struct` — user-defined structure with named fields
- Arrays are NOT explicitly declared; use pointers instead

**Type Inference:**
The compiler infers types from initialization:
```
x = (42)        # int
y = (3.14)      # float
c = ('a')       # char
s = ("hello")   # char array (string)
```

### 3. Graphics API (OpenGL Integration)

**Initialize Graphics Protocol:**
```
grp.var.init()
# Runs once at program start
# Checks display limitations, maps screen
# Must be called before any drawing operations
```

**Create Screen:**
```
opengl.screen.start(WIDTH, HEIGHT, "QUALITY")
opengl.screen.start(1920, 1080, "1080p")
opengl.screen.start(3840, 2160, "4k")
opengl.screen.start(7680, 4320, "8k")
```

**Draw Pixel/Vector:**
```
opengl.screen.vector(X, Y, grp.var.init(VARIABLE_NAME))
# First two args: X, Y coordinates
# Third arg: grp.var.init(VARIABLE_NAME) stores coordinate reference

opengl.screen.vector(100, 200, grp.var.init(point_coords))
```

**Draw with Color:**
```
opengl.screen.draw(grp.var.call(VARIABLE_NAME), color[R, G, B])
# First arg: grp.var.call() retrieves stored coordinate (not init)
# Second arg: color with RGB values (0-255)

opengl.screen.draw(grp.var.call(point_coords), color[255, 0, 0])  # red
```

**Key Difference:**
- `grp.var.init(name)` — initializes/stores a coordinate variable
- `grp.var.call(name)` — retrieves the stored coordinate for drawing

### 4. Memory Management

**Stack Allocation Only (Freestanding):**
- All variables are stack-allocated by default
- No malloc/free available
- Stack size is implementation-defined (e.g., 1MB)

**Pointer Syntax:**
```
ptr: *TYPE    # pointer to TYPE
arr: *int     # pointer to int
# Dereference: *ptr
# Address-of: &variable
```

### 5. Memory Safety Analysis

The compiler performs static analysis and reports errors:

**Memory Safety Checks:**
- Null pointer dereference detection
- Buffer overflow detection (array bounds checking)
- Use-after-free detection (not applicable without dynamic allocation)
- Uninitialized variable usage

**Error Reporting Format:**
```
Error: Potential null pointer dereference at line N
Location: function_name
Variable: variable_name
Fix: Initialize variable before use or add null check
```

```
Error: Potential buffer overflow at line N
Location: function_name
Variable: array_name
Fix: Array size is X, access index is Y (out of bounds)
```

### 6. Assembly Code Generation

**Target:**
- x86-64 architecture
- System V AMD64 ABI calling convention
- AT&T syntax (or Intel syntax, specify choice)

**Output:**
- .s (assembly source file) or direct object file
- Link with standard C library for `printf`, `exit`, etc.

**Freestanding Entry Point:**
```
.globl main
main:
    # prologue
    pushq %rbp
    movq %rsp, %rbp
    
    # function body
    
    # epilogue
    popq %rbp
    retq
```

### 7. Compilation Pipeline

1. **Lexer** — tokenize source code
2. **Parser** — build AST (Abstract Syntax Tree)
3. **Type Checker** — verify types and perform memory safety analysis
4. **Codegen** — emit x86-64 assembly
5. **Assembler** — (optional) convert to object file

### 8. Example Program

```c
struct Point {
    x = (0)
    y = (0)
}

func draw_point(p: Point, r: int, g: int, b: int) {
    opengl.screen.vector(^p.x, ^p.y, grp.var.init(coord))
    opengl.screen.draw(grp.var.call(coord), color[r, g, b])
}

func main() {
    grp.var.init()
    opengl.screen.start(1920, 1080, "1080p")
    
    point = *(100, 200)
    draw_point(point, 255, 0, 0)
}
```

### 9. Implementation Notes

**Parser Requirements:**
- Newline-sensitive (statements terminate at newline)
- Distinguish between variable declaration `VAR = (VALUE)` and struct instantiation `struct S s = *(VALUE)`
- Handle struct field access with `^` operator

**Type Checker Requirements:**
- Infer types from initialization expressions
- Validate function parameter types
- Perform pointer/null analysis
- Report memory safety violations with line numbers and fixes

**Codegen Requirements:**
- Allocate local variables on stack
- Generate function prologue/epilogue
- Emit OpenGL API calls (map to C function calls)
- Preserve System V AMD64 ABI conventions

**No External Dependencies:**
- Freestanding: no standard library except what's necessary for graphics
- Graphics calls map to OpenGL library (user provides at link time)
- Math operations compile directly to x86-64 instructions

## Build Configuration

**Language:** C++17 or later
**Build System:** CMake or Makefile
**Target:** x86-64 Linux (primary), extensible to Windows/macOS
**Dependencies:** LLVM (optional for backend), or custom assembler

## Testing

Each feature should have:
1. Positive test (valid code that compiles)
2. Negative test (code that should error with specific message)
3. Assembly output verification

Example test structure:
```
tests/
  lexer/
    test_tokenization.cpp
  parser/
    test_function_parsing.cpp
    test_struct_parsing.cpp
  typechecker/
    test_type_inference.cpp
    test_memory_safety.cpp
  codegen/
    test_assembly_output.cpp
```

## Success Criteria

1. Lexer correctly tokenizes all C-minus syntax
2. Parser builds valid AST with proper error recovery
3. Type checker infers types and detects memory safety issues
4. Codegen emits valid x86-64 assembly for all language features
5. Compiled programs run correctly on x86-64 systems
6. Graphics API calls compile and link with OpenGL library
7. Memory safety errors reported with actionable fix suggestions
