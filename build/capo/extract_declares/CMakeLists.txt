cmake_minimum_required(VERSION 3.13)
project(ll2csv)
set(CMAKE_CXX_STANDARD 20)

# Add LLVM cmake includes
# - From command line set LLVM_DIR to local LLVM cmake directory
# - From vscode, set in settings.json
#       "cmake.environment": {
#           "LLVM_DIR": "/opt/local/libexec/llvm-14/lib/cmake/llvm"
#       },
find_package(LLVM REQUIRED CONFIG)

# Use toolchain that corresponds to cmake includes from above
set(CMAKE_CXX_COMPILER "${LLVM_TOOLS_BINARY_DIR}/clang++")
set(CMAKE_C_COMPILER "${LLVM_TOOLS_BINARY_DIR}/clang")


# Use info from LLVM cmake includes
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}") # Use `add_llvm_...` functions.
include(AddLLVM) # Set compiler flags
include(HandleLLVMOptions) # Use LLVM and clang headers
include_directories(${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})
include_directories(include)

add_llvm_library( ExtractDeclares MODULE 
    ExtractDeclares.cpp
    PLUGIN_TOOL
    opt
)

target_compile_options(ExtractDeclares PRIVATE -Wall)
