# Tests

Many of these tests are end-to-end tests that rely on correct output from, but don't necessarily test directly, the following components:

- `../clang-plugin`
- `../svf/Release-build/bin/dump-ptg` 
- `../extract_declares`
- `../points_to_edges.py`


## Usage

SVF (and dump-ptg) depend on version 14 of LLVM. Because this is an older version of LLVM it isn't likely to be the default version on the system. As such, the user must set two environmental variables that specify the location of version 14 of both clang (CLANG_14_EXECUTABLE) and opt (OPT_14_EXECUTABLE).

Set the two environmental variables and then run pytest as follows:

```shell
$ export CLANG_14_EXECUTABLE='/opt/local/libexec/llvm-14/bin/clang'
$ export OPT_14_EXECUTABLE='/opt/local/libexec/llvm-14/bin/opt'
$ pytest
```
