# Copyright (C) Microsoft Corporation. All rights reserved.
# This file is distributed under the University of Illinois Open Source License.
# See LICENSE.TXT for details.

set( LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  dxcsupport
  DXIL
  DxilContainer
  DxilHash
  DxilValidation
  DxilRootSignature
  HLSL       # for gcc link error on AreDxilResourcesDense and DxilSignatureAllocator which are used in DxilValidation
  Option     # option library
  Support    # just for assert and raw streams
  )

add_clang_library(dxcvalidator
  dxcvalidator.cpp
  )

if (MINGW)
  target_link_options(dxcvalidator PUBLIC -mconsole -municode)
  target_link_libraries(dxcvalidator PRIVATE version)
endif()

target_compile_definitions(dxcvalidator
    PRIVATE VERSION_STRING_SUFFIX=" for ${CMAKE_SYSTEM_NAME}")
