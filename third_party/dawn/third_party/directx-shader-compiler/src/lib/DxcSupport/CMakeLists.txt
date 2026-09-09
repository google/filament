# Copyright (C) Microsoft Corporation. All rights reserved.
# This file is distributed under the University of Illinois Open Source License. See LICENSE.TXT for details.
add_llvm_library(LLVMDxcSupport
  dxcapi.use.cpp
  dxcmem.cpp
  FileIOHelper.cpp
  Global.cpp
  HLSLOptions.cpp
  Unicode.cpp
  WinAdapter.cpp
  WinIncludes.cpp
  WinFunctions.cpp
  dxcapi.extval.cpp
  )

#generate header with platform-specific library name
configure_file(
${LLVM_MAIN_SRC_DIR}/lib/DxcSupport/SharedLibAffix.inc
${LLVM_INCLUDE_DIR}/dxc/Support/SharedLibAffix.h
)

target_link_libraries(LLVMDxcSupport PUBLIC LLVMSupport)

add_dependencies(LLVMDxcSupport LLVMSupport TablegenHLSLOptions)
