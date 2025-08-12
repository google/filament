# Copyright (C) Microsoft Corporation. All rights reserved.
# This file is distributed under the University of Illinois Open Source License. See LICENSE.TXT for details.
find_package(TAEF REQUIRED)
find_package(D3D12 REQUIRED) # Used for ExecutionTest.cpp.

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")

add_clang_library(ExecHLSLTests SHARED
  ExecutionTest.cpp
  ShaderOpTest.cpp
  TableParameterHandler.cpp
  LongVectors.cpp
  ExecHLSLTests.rc
  )

add_dependencies(ClangUnitTests ExecHLSLTests)

set_target_properties(ExecHLSLTests PROPERTIES FOLDER "Execution tests")

target_link_libraries(ExecHLSLTests PRIVATE
  dxcompiler
  ${TAEF_LIBRARIES}
  ${D3D12_LIBRARIES}
  )

# Add includes for platform helpers and dxc API.
include_directories(${TAEF_INCLUDE_DIRS})
include_directories(${DIASDK_INCLUDE_DIRS})
include_directories(${D3D12_INCLUDE_DIRS})
include_directories(${LLVM_MAIN_INCLUDE_DIR}/dxc/Test)
# Using include path with subdirectories here to allow '#include "d3dx12.h"'
# to get use a different (newer) d3dx12.h in when building in Direct3D repo
include_directories(${LLVM_MAIN_INCLUDE_DIR}/dxc/Support)

add_dependencies(ExecHLSLTests dxcompiler)

if (NOT CLANG_INCLUDE_TESTS)
  set_output_directory(ExecHLSLTests
    ${LLVM_RUNTIME_OUTPUT_INTDIR} ${LLVM_LIBRARY_OUTPUT_INTDIR})
endif()

# Add a .user file with settings for te.exe.
file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}" DOS_STYLE_SOURCE_DIR)
file(TO_NATIVE_PATH "${TAEF_BIN_DIR}" DOS_TAEF_BIN_DIR)
configure_file(ExecHLSLTests.vcxproj.user.txt ExecHLSLTests.vcxproj.user)
