# Copyright (C) Microsoft Corporation. All rights reserved.
# This file is distributed under the University of Illinois Open Source License. See LICENSE.TXT for details.
find_package(DiaSDK REQUIRED) # Used for constants and declarations.

set(LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  analysis
  asmparser
#  asmprinter # no support for LLVM codegen
  bitreader
  bitwriter
#  codegen # no support for LLVM codegen
  core
#  debuginfodwarf # no support for DWARF files (IR debug info is OK)
#  debuginfopdb # no support for PDB files
  dxcsupport
  dxcbindingtable
  dxrfallback
  dxil
  dxilcontainer
  dxilrootsignature
  hlsl
  instcombine
  ipa
  ipo
  irreader
#  libdriver
#  lineeditor
  linker
#  lto
#  mirparser # no support for LLVM codegen
  mssupport
#  object # no support for object files (coff, elf)
  option
#  passes
  profiledata
  scalaropts
#  selectiondag # no support for LLVM codegen
  support
  target
  transformutils
  vectorize
  )

set(SOURCES
  dxcapi.cpp
  DXCompiler.cpp
  DXCompiler.rc
  DXCompiler.def
  dxillib.cpp
  dxcutil.cpp
  dxcdxrfallbackcompiler.cpp
  dxcvalidator.cpp
  )

add_clang_library(dxrfallbackcompiler SHARED ${SOURCES})
target_link_libraries(dxrfallbackcompiler PRIVATE ${LIBRARIES} ${DIASDK_LIBRARIES} dxcvalidator)
# SPIRV change starts
if (ENABLE_SPIRV_CODEGEN)
  target_link_libraries(dxrfallbackcompiler PRIVATE clangSPIRV)
endif (ENABLE_SPIRV_CODEGEN)
# SPIRV change ends
add_dependencies(dxrfallbackcompiler DxcEtw)
include_directories(AFTER ${LLVM_INCLUDE_DIR}/dxc/Tracing ${DIASDK_INCLUDE_DIRS})
include_directories(${LLVM_SOURCE_DIR}/tools/clang/tools/dxcvalidator)

set_target_properties(dxrfallbackcompiler
  PROPERTIES
  OUTPUT_NAME "dxrfallbackcompiler"
  VERSION ${LIBCLANG_LIBRARY_VERSION}
  DEFINE_SYMBOL _CINDEX_LIB_)
