# Copyright (C) Microsoft Corporation. All rights reserved.
# This file is distributed under the University of Illinois Open Source License. See LICENSE.TXT for details.
# Build a dxlib_sample.dll as a sample for library and link.

set(SOURCES
  ../dxcompiler/dxcfilesystem.cpp
  lib_cache_manager.cpp
  lib_share_compile.cpp
  lib_share_preprocessor.cpp
  dxlib_sample.cpp
  dxlib_sample.def
  )

set( LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  analysis
  ipa
  dxcsupport
  DXIL
  HLSL
  Option     # option library
  Support    # just for assert and raw streams
  )

add_clang_library(dxlib_sample SHARED ${SOURCES})

target_link_libraries(dxlib_sample
  PRIVATE
  dxcompiler
  )

add_dependencies(dxlib_sample dxcompiler)

set_target_properties(dxlib_sample
  PROPERTIES
  OUTPUT_NAME "dxlib_sample"
  VERSION ${LIBCLANG_LIBRARY_VERSION})
