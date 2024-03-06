cmake_minimum_required(VERSION 3.18)
project("myProject")

#find_package(LibClang REQUIRED)

find_package(Clang REQUIRED)

# Add path to LLVM modules
#set(CMAKE_MODULE_PATH
#  ${CMAKE_MODULE_PATH}
#  "${LLVM_CMAKE_DIR}"
#  )

include(AddLLVM)
include(/lib/llvm-14/lib/cmake/clang/AddClang.cmake)

include_directories(/lib/llvm-14/include)

set(LLVM_LINK_COMPONENTS
  Support
  )
message(STATUS  ${CMAKE_MODULE_PATH})

add_clang_executable(pp-traceX
PPTrace.cpp
PPCallbacksTracker.cpp)

target_link_libraries(pp-traceX
  PRIVATE
  clangAST
  clangBasic
  clangLex
  clangFrontend
  clangSerialization
  clangTooling
  )