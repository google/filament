# Copyright (C) Microsoft Corporation. All rights reserved.
# This file is distributed under the University of Illinois Open Source License. See LICENSE.TXT for details.
# Builds HLSLErrors sample code

add_clang_executable(HLSLErrors
  HLSLErrors.cpp
  )

target_link_libraries(HLSLErrors
  dxcompiler
  )

set_target_properties(HLSLErrors PROPERTIES VERSION ${CLANG_EXECUTABLE_VERSION})

add_dependencies(HLSLErrors dxcompiler)

set(CLANGXX_LINK_OR_COPY copy)
set(HLSLErrors_binary "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/HLSLErrors${CMAKE_EXECUTABLE_SUFFIX}")

install(TARGETS HLSLErrors
  RUNTIME DESTINATION bin)

