# CLOSURE Divider Clang plugin

## Prerequisites

- LLVM 14
- Clang 14 with API

## Building

```bash
cmake -B build
cmake --build build
```

## Invocation 

Use the --extra-arg to add compiler flags. The following is a command used to divide the websrv example.

```bash
bin/divider --extra-arg=-I../test/websrv/refactored/Utils --extra-arg=-I../test/websrv/refactored/Communications --extra-arg=-Wno-deprecated-declarations --extra-arg=-Wno-implicit-const-int-float-conversion ../test/websrv/topology.json --
```
