# Extract Declares

Given an LLVM bitcode file (ll or bc), produce declares.csv file that includes information about declared variables.


## Build

Building will create an LLVM pass plugin.

You will need to set the LLVM_DIR to the LLVM distributions cmake directory for llvm. This is used by cmake `find_package(LLVM...)` in the CMakeLists.txt to configure the build.

```console
$ export LLVM_DIR="/opt/local/libexec/llvm-14/lib/cmake/llvm"
$ mkdir build
$ pushd build
$ cmake ..
$ cmake --build .
$ popd
```

You will need the share library LLVM module located in the build directory. On macOS it named `build/ExtractDeclares.dylib`. On Linux it is likely named the same with an `.so` extension instead of `.dylib`.

```console
$ export MY_LLVM_PLUGIN_PATH="build/ExtractDeclares.dylib"
```


## Run

The plugin will be run using the LLVM opt program. Change the full path to the opt program (which should share a trunk with the LLVM_DIR tree from above) to match your own.

```console
$ /opt/local/libexec/llvm-14/bin/opt \
    -disable-output \
    -load-pass-plugin=$MY_LLVM_PLUGIN_PATH \
    -passes=declare-csv \
    test_src/foo.ll
```

Output is found in declares.csv


## Original Requirements

When you run clang with debug symbols and generate LLVM IR, it will give you some metadata about the source program. In particular, for all declarations (which arent parameters) within a function, there will be a call to an LLVM intrinsic which looks something like the following:

@llvm.dbg.declare(var, metdata, ...)

A program which takes a LLVM bitcode file as input and outputs a csv containing the following for each global variable and @llvm.dbg.declare line: 

- The LLID associated with the LLVM variable (see LLID tuples below)
- The source file, line, character associated with the metadata (second argument of dbg.declare)
- A source debug name (the name of the variable in the source code) to make looking through the csv easier

The LLID should be a tuple of 3 items whose content depends on what is given in the first argument of the llvm.dbg.declare line. There are three different cases:

1. If the first argument is a reference to a global variable, the first field of the tuple should be the name of the llvm global variable (like @x), and the other two fields should be empty. 

2. If the first argument is a reference to a local definition (like %x), you should give the name of the containing function for the first field and the second field should be an integer representing the index of the instruction in which it was defined. The third field should be left empty in this case. 

3. If the first argument is a reference to a parameter to a function, then the first field of the tuple should be the containing function for the parameter and the third field should be the parameter's index. The second field should be left empty in this case


## Notes

- More about llvm.dbg.declare: <https://llvm.org/docs/SourceLevelDebugging.html#llvm-dbg-declare>

- Documentation for writing an LLVM pass plugin for the newer pass manager: <https://llvm.org/docs/WritingAnLLVMNewPMPass.html>

    - Note: these are instructions for the new pass manager not the [**older** method](<https://llvm.org/docs/WritingAnLLVMPass.html>)


- This documentation may be a bit more helpful than the official documentation above: <https://medium.com/@mshockwave/writing-llvm-pass-in-2018-part-i-531c700e85eb>
    - Found through: <https://github.com/alexjung/Writing-an-LLVM-Pass-using-the-new-PassManager>
