# CLE Clang plugin

## Prerequisites

- LLVM 14
- Clang 14 with API

## Building

```bash
cmake -B build
cmake --build build
```

## Invocation 

```bash
clang -g -c -Xclang -load -Xclang build/CLE.so -Xclang -plugin -Xclang cle test/foo.cpp 
```
