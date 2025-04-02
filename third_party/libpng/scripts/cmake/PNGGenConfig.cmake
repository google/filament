# PNGGenConfig.cmake
# Utility functions for configuring and building libpng

# Copyright (c) 2018-2025 Cosmin Truta
# Copyright (c) 2016-2018 Glenn Randers-Pehrson
# Written by Roger Leigh, 2016
#
# Use, modification and distribution are subject to
# the same licensing terms and conditions as libpng.
# Please see the copyright notice in png.h or visit
# http://libpng.org/pub/png/src/libpng-LICENSE.txt
#
# SPDX-License-Identifier: libpng-2.0

# Generate .chk from .out with awk, based upon the automake logic:
# generate_chk(INPUT <file> OUTPUT <file> [DEPENDS <deps>...])
function(generate_chk)
  set(options)
  set(oneValueArgs INPUT OUTPUT)
  set(multiValueArgs DEPENDS)
  cmake_parse_arguments(_GC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  if(NOT _GC_INPUT)
    message(FATAL_ERROR "generate_chk: Missing INPUT argument")
  endif()
  if(NOT _GC_OUTPUT)
    message(FATAL_ERROR "generate_chk: Missing OUTPUT argument")
  endif()

  # Run genchk.cmake to generate the .chk file.
  add_custom_command(OUTPUT "${_GC_OUTPUT}"
                     COMMAND "${CMAKE_COMMAND}"
                             "-DINPUT=${_GC_INPUT}"
                             "-DOUTPUT=${_GC_OUTPUT}"
                             -P "${CMAKE_CURRENT_BINARY_DIR}/scripts/cmake/genchk.cmake"
                     DEPENDS "${_GC_INPUT}" ${_GC_DEPENDS}
                     WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
endfunction()

# Generate .out from C source file with awk:
# generate_out(INPUT <file> OUTPUT <file> [DEPENDS <deps>...])
function(generate_out)
  set(options)
  set(oneValueArgs INPUT OUTPUT)
  set(multiValueArgs DEPENDS)
  cmake_parse_arguments(_GO "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  if(NOT _GO_INPUT)
    message(FATAL_ERROR "generate_out: Missing INPUT argument")
  endif()
  if(NOT _GO_OUTPUT)
    message(FATAL_ERROR "generate_out: Missing OUTPUT argument")
  endif()

  # Run genout.cmake to generate the .out file.
  add_custom_command(OUTPUT "${_GO_OUTPUT}"
                     COMMAND "${CMAKE_COMMAND}"
                             "-DINPUT=${_GO_INPUT}"
                             "-DOUTPUT=${_GO_OUTPUT}"
                             -P "${CMAKE_CURRENT_BINARY_DIR}/scripts/cmake/genout.cmake"
                     DEPENDS "${_GO_INPUT}" ${_GO_DEPENDS}
                     WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
endfunction()

# Generate a source file with awk:
# generate_source(OUTPUT <file> [DEPENDS <deps>...])
function(generate_source)
  set(options)
  set(oneValueArgs OUTPUT)
  set(multiValueArgs DEPENDS)
  cmake_parse_arguments(_GSO "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  if(NOT _GSO_OUTPUT)
    message(FATAL_ERROR "generate_source: Missing OUTPUT argument")
  endif()

  # Run gensrc.cmake to generate the source file.
  add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${_GSO_OUTPUT}"
                     COMMAND "${CMAKE_COMMAND}"
                             "-DOUTPUT=${_GSO_OUTPUT}"
                             -P "${CMAKE_CURRENT_BINARY_DIR}/scripts/cmake/gensrc.cmake"
                     DEPENDS ${_GSO_DEPENDS}
                     WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
endfunction()

# Generate an identical file copy:
# generate_copy(INPUT <file> OUTPUT <file> [DEPENDS <deps>...])
function(generate_copy)
  set(options)
  set(oneValueArgs INPUT OUTPUT)
  set(multiValueArgs DEPENDS)
  cmake_parse_arguments(_GCO "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  if(NOT _GCO_INPUT)
    message(FATAL_ERROR "generate_copy: Missing INPUT argument")
  endif()
  if(NOT _GCO_OUTPUT)
    message(FATAL_ERROR "generate_copy: Missing OUTPUT argument")
  endif()

  # Make a forced file copy, overwriting any pre-existing output file.
  add_custom_command(OUTPUT "${_GCO_OUTPUT}"
                     COMMAND "${CMAKE_COMMAND}"
                             -E remove "${_GCO_OUTPUT}"
                     COMMAND "${CMAKE_COMMAND}"
                             -E copy "${_GCO_INPUT}" "${_GCO_OUTPUT}"
                     DEPENDS "${source}" ${_GCO_DEPENDS})
endfunction()
