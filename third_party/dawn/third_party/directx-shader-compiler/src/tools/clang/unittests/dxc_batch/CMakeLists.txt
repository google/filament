# Copyright (C) Microsoft Corporation. All rights reserved.
# This file is distributed under the University of Illinois Open Source License. See LICENSE.TXT for details.
# Builds dxc_batch.exe

set( LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  analysis
  ipa
  dxcsupport
  DXIL
  DxilContainer
  HLSL
  Option     # option library
  Support    # just for assert and raw streams
  MSSupport  # for CreateMSFileSystemForDisk
  )

add_clang_executable(dxc_batch
  dxc_batch.cpp
  )

target_link_libraries(dxc_batch
  clangAST
  clangCodeGen
  clangSema
  dxcompiler
  dxlib_sample
  )

set_target_properties(dxc PROPERTIES VERSION ${CLANG_EXECUTABLE_VERSION})

include_directories(AFTER ${DIASDK_INCLUDE_DIRS})

add_dependencies(dxc_batch dxcompiler dxlib_sample)

if(UNIX)
  set(CLANGXX_LINK_OR_COPY create_symlink)
# Create a relative symlink
  set(dxc_batch_binary "dxc_batch${CMAKE_EXECUTABLE_SUFFIX}")
else()
  set(CLANGXX_LINK_OR_COPY copy)
  set(dxc_batch_binary "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/dxc_batch${CMAKE_EXECUTABLE_SUFFIX}")
endif()

install(TARGETS dxc_batch
  RUNTIME DESTINATION bin)

