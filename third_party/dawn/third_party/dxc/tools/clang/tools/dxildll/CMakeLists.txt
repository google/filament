# Copyright (C) Microsoft Corporation. All rights reserved.
# This file is distributed under the University of Illinois Open Source License. See LICENSE.TXT for details.

set(SHARED_LIBRARY TRUE)

set(DXCComponents
  LLVMBitWriter
  LLVMDxcSupport
  LLVMDXIL
  LLVMDxilContainer
  LLVMDxilHash
  LLVMDxilRootSignature
  LLVMDxilValidation
  LLVMHLSL       # for AreDxilResourcesDense and DxilSignatureAllocator which are used in DxilValidation
  LLVMMSSupport
  LLVMSupport
  LLVMTransformUtils
  LLVMAnalysis
  LLVMDxcBindingTable
  LLVMipa
)

foreach(C ${DXCComponents})
if (WIN32)
   list(APPEND DXCLibs ${C})
   list(APPEND DXCDependencies ${C})
else()
   list(APPEND DXCLibs ${C})
   list(APPEND DXCDependencies ${C})
endif()
endforeach(C ${DXCComponents})

include_directories(
  ${LLVM_SOURCE_DIR}/tools/clang/tools/dxcvalidator
  ${LLVM_BINARY_DIR}/projects/include
)

set(sources
  dxildll.cpp
  dxildll.def
  dxcvalidator.cpp
)

if (WIN32)
add_clang_library(dxildll SHARED ${sources})
list(APPEND DXCLibs
  kernel32.lib
  atls.lib
  advapi32.lib
  ole32.lib)
else()
add_clang_library(dxildll SHARED ${sources})
endif()

target_link_libraries(dxildll PRIVATE
  ${DXCLibs}
  clangBasic # for clang::getGitCommitHash.
  dxcvalidator
)

if (WIN32)
add_dependencies(dxildll
    ${DXCDependencies}
    DxcEtw
    DxcRuntimeEtw
)
else()
add_dependencies(dxildll
    ${DXCDependencies}
)
endif()

if (WIN32)
    get_target_property(sources dxildll SOURCES)

    list(APPEND sources dxildll.rc)
    set_target_properties(${target_name} PROPERTIES SOURCES "${sources}")

    set_property(SOURCE dxildll.rc PROPERTY COMPILE_DEFINITIONS "INCLUDE_HLSL_VERSION_FILE=1")
    set_property(SOURCE dxildll.rc PROPERTY COMPILE_OPTIONS "/I" "${HLSL_VERSION_LOCATION}" "/I" "${LLVM_MAIN_SRC_DIR}")
endif()

set_target_properties(dxildll
  PROPERTIES
  OUTPUT_NAME "dxil"
)
