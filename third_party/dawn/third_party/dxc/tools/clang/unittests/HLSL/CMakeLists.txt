# Copyright (C) Microsoft Corporation. All rights reserved.
# This file is distributed under the University of Illinois Open Source License. See LICENSE.TXT for details.
if(WIN32)
find_package(TAEF REQUIRED)
find_package(DiaSDK REQUIRED) # Used for constants and declarations.
find_package(D3D12 REQUIRED)  # Used in DxilContainerTest for DXBC compilation/reflection
endif(WIN32)

set( LLVM_LINK_COMPONENTS
  support
  mssupport
  dxcsupport
  dxil
  dxilcontainer
  dxilrootsignature
  hlsl
  dxilhash
  option
  bitreader
  bitwriter
  analysis
  ipa
  irreader
  transformutils  # for CloneModule
  )

if(WIN32)
set(HLSL_IGNORE_SOURCES
  TestMain.cpp
  HLSLTestOptions.cpp
)
add_clang_library(ClangHLSLTests SHARED
  AllocatorTest.cpp
  CompilerTest.cpp
  DxilContainerTest.cpp
  DxilModuleTest.cpp
  DxilResourceTests.cpp
  DXIsenseTest.cpp
  ExtensionTest.cpp
  FunctionTest.cpp
  LinkerTest.cpp
  MSFileSysTest.cpp
  Objects.cpp
  OptimizerTest.cpp
  OptionsTest.cpp
  PixDiaTest.cpp
  PixTest.cpp
  PixTestUtils.cpp
  RewriterTest.cpp
  SystemValueTest.cpp
  ValidationTest.cpp
  VerifierTest.cpp
  )

  add_dependencies(ClangUnitTests ClangHLSLTests)
else (WIN32)
set(HLSL_IGNORE_SOURCES
  MSFileSysTest.cpp
  PixDiaTest.cpp
  )

add_clang_unittest(ClangHLSLTests
  AllocatorTest.cpp
  CompilerTest.cpp
  DxilContainerTest.cpp
  DxilModuleTest.cpp
  DxilResourceTests.cpp
  DXIsenseTest.cpp
  ExtensionTest.cpp
  FunctionTest.cpp
  HLSLTestOptions.cpp
  LinkerTest.cpp
  Objects.cpp
  OptimizerTest.cpp
  OptionsTest.cpp
  RewriterTest.cpp
  PixTest.cpp
  PixTestUtils.cpp
  SystemValueTest.cpp
  TestMain.cpp
  ValidationTest.cpp
  VerifierTest.cpp
  )

endif(WIN32)

set_target_properties(ClangHLSLTests PROPERTIES FOLDER "Clang tests")

if (WIN32)
target_link_libraries(ClangHLSLTests PRIVATE
  dxcompiler
  HLSLTestLib
  LLVMDxilContainer
  LLVMDxilDia
  ${TAEF_LIBRARIES}
  ${DIASDK_LIBRARIES}
  ${D3D12_LIBRARIES}
  shlwapi
  )
else(WIN32)
target_link_libraries(ClangHLSLTests
  dxcompiler
  LLVMDxilDia
  HLSLTestLib
  )
endif(WIN32)

if(WIN32)
# Add includes for platform helpers and dxc API.
include_directories(${TAEF_INCLUDE_DIRS})
include_directories(${DIASDK_INCLUDE_DIRS})
include_directories(${D3D12_INCLUDE_DIRS})
include_directories(${LLVM_MAIN_INCLUDE_DIR}/dxc/Test)
endif(WIN32)

# Add includes to directly reference intrinsic tables.
include_directories(${CLANG_BINARY_DIR}/lib/Sema)

add_dependencies(ClangHLSLTests dxcompiler)

if (NOT CLANG_INCLUDE_TESTS)
  set_output_directory(ClangHLSLTests
    ${LLVM_RUNTIME_OUTPUT_INTDIR} ${LLVM_LIBRARY_OUTPUT_INTDIR})
  if (NOT WIN32)
    add_test(NAME test-hlsl-codegen
            COMMAND $<TARGET_FILE:ClangHLSLTests> --HlslDataDir
              ${PROJECT_SOURCE_DIR}/tools/clang/test/HLSL)
  endif()
endif()

if(WIN32)
# Add a .user file with settings for te.exe.
file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}" DOS_STYLE_SOURCE_DIR)
file(TO_NATIVE_PATH "${TAEF_BIN_DIR}" DOS_TAEF_BIN_DIR)
configure_file(ClangHLSLTests.vcxproj.user.txt ClangHLSLTests.vcxproj.user)
endif(WIN32)
