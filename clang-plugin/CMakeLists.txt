cmake_minimum_required(VERSION 3.13)

project(clang_plugins)

# Let CMake to find LLVM and Clang modules.
# Give `-DCMAKE_PREFIX_PATH` or `-DLLVM_DIR` and `-DCLANG_DIR`.
# See <https://cmake.org/cmake/help/v3.11/command/find_package.html>
find_package(LLVM REQUIRED CONFIG)

# Use given `clang` toolchain.
set(CMAKE_CXX_COMPILER "${LLVM_TOOLS_BINARY_DIR}/clang++")
set(CMAKE_C_COMPILER "${LLVM_TOOLS_BINARY_DIR}/clang")


list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
# Use `add_llvm_...` functions.
include(AddLLVM)
# Set compiler flags.
include(HandleLLVMOptions)
# Use LLVM and clang headers.
include_directories(${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})

add_llvm_library(CLE MODULE CLE.cpp PLUGIN_TOOL clang)

target_compile_options(CLE PRIVATE -Wall)
