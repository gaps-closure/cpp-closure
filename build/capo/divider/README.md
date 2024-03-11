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

Use the --extra-arg to add compiler flags. The following is a command used to divide the websrv example, assuming mbedtls is installed in system include directories, for example, /usr/include.

```bash

EXTRA=""\
"--extra-arg=-I../test/websrv/refactored/Utils "\
"--extra-arg=-I../test/websrv/refactored/Communications "\
"--extra-arg=-Wno-deprecated-declarations "\
"--extra-arg=-Wno-implicit-const-int-float-conversion "\
"--extra-arg=-D_GNU_SOURCE "\
"--extra-arg=-DMG_ENABLE_MBEDTLS=1 "\
"--extra-arg=-DMG_ENABLE_MD5=1 "\
"--extra-arg=-DMG_ENABLE_LINES=1 "\
"--extra-arg=-DORION_COMM_=1 "\

bin/divider $EXTRA ../test/websrv/topology.json --
```
