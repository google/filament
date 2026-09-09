# This file contains the basic options required for building DXC using CMake on
# *nix platforms. It is passed to CMake using the `-C` flag and gets processed
# before the root CMakeLists.txt file. Only cached variables persist after this
# file executes, so all state must be saved into the cache. These variables also
# will not override explicit command line parameters, and can only read
# parameters that are specified before the `-C` flag.

if (DXC_COVERAGE)
  set(LLVM_BUILD_INSTRUMENTED_COVERAGE ON CACHE BOOL "")
  set(LLVM_PROFILE_DATA_DIR "${CMAKE_BINARY_DIR}/profile" CACHE STRING "")
  set(LLVM_CODE_COVERAGE_TARGETS "dxc;dxcompiler" CACHE STRING "")
  set(LLVM_CODE_COVERAGE_TEST_TARGETS "check-all" CACHE STRING "")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "")
set(LLVM_APPEND_VC_REV ON CACHE BOOL "")
set(LLVM_DEFAULT_TARGET_TRIPLE "dxil-ms-dx" CACHE STRING "")
set(LLVM_ENABLE_EH ON CACHE BOOL "")
set(LLVM_ENABLE_RTTI ON CACHE BOOL "")
set(LLVM_INCLUDE_DOCS OFF CACHE BOOL "")
set(LLVM_INCLUDE_EXAMPLES OFF CACHE BOOL "")
set(LLVM_OPTIMIZED_TABLEGEN OFF CACHE BOOL "")
set(LLVM_TARGETS_TO_BUILD "None" CACHE STRING "")
set(LIBCLANG_BUILD_STATIC ON CACHE BOOL "")
set(CLANG_BUILD_EXAMPLES OFF CACHE BOOL "")
set(CLANG_CL OFF CACHE BOOL "")
set(CLANG_ENABLE_ARCMT OFF CACHE BOOL "")
set(CLANG_ENABLE_STATIC_ANALYZER OFF CACHE BOOL "")
set(HLSL_INCLUDE_TESTS ON CACHE BOOL "")
set(ENABLE_SPIRV_CODEGEN ON CACHE BOOL "")
set(SPIRV_BUILD_TESTS ON CACHE BOOL "")
set(LLVM_ENABLE_TERMINFO OFF CACHE BOOL "")
