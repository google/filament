# Copyright (C) Microsoft Corporation. All rights reserved.
# This file is distributed under the University of Illinois Open Source License. See LICENSE.TXT for details.
# Builds dxopt.exe

set( LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  dxcsupport
  Support    # just for assert and raw streams
  )

add_clang_executable(dxopt
  dxopt.cpp
  )

target_link_libraries(dxopt
  dxcompiler
  )

if (MINGW)
  target_link_options(dxopt PRIVATE -mconsole -municode)
endif()

set_target_properties(dxopt PROPERTIES VERSION ${CLANG_EXECUTABLE_VERSION})

add_dependencies(dxopt dxcompiler)

if(UNIX)
  set(CLANGXX_LINK_OR_COPY create_symlink)
# Create a relative symlink
  set(dxopt_binary "dxopt${CMAKE_EXECUTABLE_SUFFIX}")
else()
  set(CLANGXX_LINK_OR_COPY copy)
  set(dxopt_binary "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/dxopt${CMAKE_EXECUTABLE_SUFFIX}")
endif()

install(TARGETS dxopt
  RUNTIME DESTINATION bin)

