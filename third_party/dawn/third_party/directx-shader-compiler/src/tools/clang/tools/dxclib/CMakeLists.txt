# Copyright (C) Microsoft Corporation. All rights reserved.
# This file is distributed under the University of Illinois Open Source License. See LICENSE.TXT for details.
# Builds dxclib

if (MSVC)
  find_package(DiaSDK REQUIRED) # Used for constants and declarations.
endif (MSVC)

set( LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  dxcsupport
  DXIL
  DxilContainer
  HLSL
  Option     # option library
  Support    # just for assert and raw streams
  )

add_clang_library(dxclib
  dxc.cpp
  )

if (MINGW)
  target_link_options(dxclib PUBLIC -mconsole -municode)
  target_link_libraries(dxclib PRIVATE version)
endif()

if(ENABLE_SPIRV_CODEGEN)
  target_link_libraries(dxclib PRIVATE SPIRV-Tools)
  target_link_libraries(dxclib PRIVATE clangSPIRV)
endif()

if (MSVC)
  include_directories(AFTER ${DIASDK_INCLUDE_DIRS})
endif (MSVC)

target_compile_definitions(dxclib
    PRIVATE VERSION_STRING_SUFFIX=" for ${CMAKE_SYSTEM_NAME}")

add_dependencies(dxclib TablegenHLSLOptions dxcompiler)
