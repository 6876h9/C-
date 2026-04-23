# Building C-

## Requirements

- C++17 compatible compiler (g++, clang++)
- Linux/Unix environment
- Make or CMake

## Build Methods

### 1. Using Make (Fastest)

```bash
cd cminus
make
```

Output: `cminus` executable in current directory

### 2. Using CMake

```bash
cd cminus
mkdir build
cd build
cmake ..
make
```

Output: `cminus` executable in `build/` directory

### 3. Using g++ Directly

```bash
cd cminus
g++ -std=c++17 -Wall -Wextra -O2 *.cpp -o cminus
```

## Compilation Flags

- `-std=c++17`: C++17 standard
- `-Wall -Wextra`: All warnings
- `-O2`: Optimization level 2
- `-g`: Debug symbols (optional)

## Verifying Build

```bash
./cminus -v examples/hello.cm
```

Should output:
```
C- Compiler
Input:  examples/hello.cm
Output: a.out
Compilation successful
```

## Troubleshooting

**Problem**: `g++: error: unknown type name 'std'`
- **Solution**: Ensure `-std=c++17` flag is used

**Problem**: `undefined reference to 'cminus::...'`
- **Solution**: Recompile all `.cpp` files together, not just single files

**Problem**: `error: expected type`
- **Solution**: This is a C- syntax error in your source code, not a build error
