# Copyright (C) Microsoft Corporation. All rights reserved.
# This file is distributed under the University of Illinois Open Source License. See LICENSE.TXT for details.
# Builds dxr.exe

set( LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  dxcsupport
  Option     # option library
  Support    # For Atomic increment/decrement
  )

add_clang_executable(dxr
  dxr.cpp
#  dxr.rc
  )

target_link_libraries(dxr
  dxclib
  dxcompiler
  )

set_target_properties(dxr PROPERTIES VERSION ${CLANG_EXECUTABLE_VERSION})
# set_target_properties(dxr PROPERTIES ENABLE_EXPORTS 1)

include_directories(${LLVM_SOURCE_DIR}/tools/clang/tools)

add_dependencies(dxr dxclib dxcompiler)

if(UNIX)
  set(CLANGXX_LINK_OR_COPY create_symlink)
# Create a relative symlink
  set(dxr_binary "dxr${CMAKE_EXECUTABLE_SUFFIX}")
else()
  set(CLANGXX_LINK_OR_COPY copy)
  set(dxr_binary "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/dxr${CMAKE_EXECUTABLE_SUFFIX}")
endif()

install(TARGETS dxr
  RUNTIME DESTINATION bin)

