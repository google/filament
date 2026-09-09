# Copyright (C) Microsoft Corporation. All rights reserved.
# This file is distributed under the University of Illinois Open Source License. See LICENSE.TXT for details.
# Builds HLSLHost.exe

set( LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  Support    # For Atomic increment/decrement
  )

add_clang_executable(HLSLHost
  HLSLHost.cpp
  )

set_target_properties(HLSLHost PROPERTIES VERSION ${CLANG_EXECUTABLE_VERSION})

include_directories(AFTER ${DIASDK_INCLUDE_DIRS})

if(UNIX)
  set(CLANGXX_LINK_OR_COPY create_symlink)
# Create a relative symlink
  set(HLSLHost_binary "HLSLHost${CMAKE_EXECUTABLE_SUFFIX}")
else()
  set(CLANGXX_LINK_OR_COPY copy)
  set(HLSLHost_binary "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/HLSLHost${CMAKE_EXECUTABLE_SUFFIX}")
endif()

install(TARGETS HLSLHost
  RUNTIME DESTINATION bin)

