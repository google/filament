# Copyright (C) Microsoft Corporation. All rights reserved.
# This file is distributed under the University of Illinois Open Source License. See LICENSE.TXT for details.
# Generate ETW instrumentation.

# Create the header in a temporary file and only update when necessary,
# to avoid invalidating targets that depend on it.
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/tmpdxcruntimeetw.h
  COMMAND mc -r ${CMAKE_CURRENT_BINARY_DIR} -h ${CMAKE_CURRENT_BINARY_DIR} -p DxcRuntimeEtw_ -um -z tmpdxcruntimeetw ${CMAKE_CURRENT_SOURCE_DIR}/DxcRuntime.man
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/DxcRuntime.man
  COMMENT "Building instrumentation manifest ..."
)
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/DxcRuntimeEtw.h
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_BINARY_DIR}/tmpdxcruntimeetw.h
        ${CMAKE_CURRENT_BINARY_DIR}/DxcRuntimeEtw.h
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_BINARY_DIR}/tmpdxcruntimeetw.rc
        ${CMAKE_CURRENT_BINARY_DIR}/DxcRuntimeEtw.rc
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_BINARY_DIR}/tmpdxcruntimeetwTEMP.bin
        ${CMAKE_CURRENT_BINARY_DIR}/DxcRuntimeEtwtemp.BIN
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_BINARY_DIR}/tmpdxcruntimeetw_msg00001.bin
        ${CMAKE_CURRENT_BINARY_DIR}/DxcRuntimeEtw_msg00001.bin
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tmpdxcruntimeetw.h
  COMMENT "Updating instrumentation manifest ..."
)

set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/DxcRuntimeEtw.h PROPERTIES GENERATED 1)
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/DxcRuntimeEtw.rc PROPERTIES GENERATED 1)
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/DxcRuntimeEtwtemp.bin PROPERTIES GENERATED 1)
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/DxcRuntimeEtw_msg00001.bin PROPERTIES GENERATED 1)

add_custom_target(DxcRuntimeEtw
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/DxcRuntimeEtw.h
  SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/DxcRuntime.man
)

# Not quite library, but close enough.
set_target_properties(DxcRuntimeEtw PROPERTIES FOLDER "Dxilconv libraries")
