# Installation Guide for C-

## System Requirements

- **OS**: Linux/Unix (x86_64)
- **Compiler**: GCC 7+ or Clang 5+ (C++17 support)
- **Build Tools**: Make or CMake
- **Assembler**: GNU as (gas)
- **Linker**: ld

## Quick Install

### 1. Clone Repository
```bash
git clone https://github.com/6876h9/C-.git
cd C-
```

### 2. Build Compiler
```bash
make
```

The executable `cminus` will be created in the current directory.

### 3. Verify Installation
```bash
./cminus -v examples/hello.cm
```

Expected output:
```
C- Compiler
Input:  examples/hello.cm
Output: a.out
Compilation successful
```

## Alternative Build Methods

### Using CMake
```bash
mkdir build
cd build
cmake ..
make
sudo make install  # Optional: install to /usr/local/bin
```

### Manual Compilation
```bash
g++ -std=c++17 -Wall -Wextra -O2 *.cpp -o cminus
```

### Using Clang
```bash
clang++ -std=c++17 -Wall -Wextra -O2 *.cpp -o cminus
```

## Troubleshooting

### Build Fails with "C++17 not supported"
**Solution**: Update your compiler
```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install g++-9 make

# Or use the compiler explicitly
g++-9 -std=c++17 *.cpp -o cminus
```

### "make: command not found"
**Solution**: Install Make
```bash
# Ubuntu/Debian
sudo apt-get install build-essential

# Fedora/RHEL
sudo dnf install make gcc-c++

# macOS
xcode-select --install
```

### "g++: command not found"
**Solution**: Install GCC
```bash
# Ubuntu/Debian
sudo apt-get install g++

# Fedora/RHEL
sudo dnf install gcc-c++
```

### Permission denied when running ./cminus
**Solution**: Make executable
```bash
chmod +x cminus
```

## Post-Installation

### 1. Add to PATH (Optional)
```bash
# Copy to system directory
sudo cp cminus /usr/local/bin/

# Or add current directory to PATH
export PATH="$PWD:$PATH"
```

### 2. Test All Examples
```bash
for example in examples/*.cm; do
    echo "Testing $example..."
    ./cminus "$example" || echo "FAILED"
done
```

### 3. Compile Your First Program
Create `test.cm`:
```c-
fn main() -> void {
    let x = 42;
}
```

Compile:
```bash
./cminus test.cm -S -o test.s
cat test.s
```

## Development Setup

### For Contributors

1. **Clone with Git**
```bash
git clone https://github.com/6876h9/C-.git
cd C-
```

2. **Install Development Tools**
```bash
# Ubuntu/Debian
sudo apt-get install git build-essential gdb

# Fedora/RHEL  
sudo dnf install git gcc-c++ gdb

# macOS
brew install gcc gdb
```

3. **Build with Debug Symbols**
```bash
g++ -std=c++17 -Wall -Wextra -g *.cpp -o cminus
```

4. **Debug with GDB**
```bash
gdb ./cminus
(gdb) run examples/hello.cm
(gdb) bt  # Print backtrace on crash
```

## Docker Installation (Alternative)

Create `Dockerfile`:
```dockerfile
FROM ubuntu:20.04

RUN apt-get update && apt-get install -y \
    build-essential \
    git \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /opt/cminus
RUN git clone https://github.com/6876h9/C-.git .
RUN make

ENTRYPOINT ["./cminus"]
```

Build and run:
```bash
docker build -t cminus .
docker run -v $(pwd):/work cminus /work/test.cm -S
```

## Uninstallation

### If installed to /usr/local/bin
```bash
sudo rm /usr/local/bin/cminus
```

### Remove source directory
```bash
rm -rf ~/C-
```

## Getting Help

- **Documentation**: See `README.md`, `QUICKSTART.md`, `LANGUAGE.md`
- **Examples**: Check `examples/` directory
- **Build Issues**: See "Troubleshooting" section above
- **Contributing**: See `CONTRIBUTING.md`

## Next Steps

1. Read `QUICKSTART.md` to write your first program
2. Check `LANGUAGE.md` for complete language specification
3. Explore `examples/` for code samples
4. See `CONTRIBUTING.md` if you want to extend the compiler
