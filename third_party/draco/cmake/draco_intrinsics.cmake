# Copyright 2021 The Draco Authors
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License. You may obtain a copy of
# the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License.

if(DRACO_CMAKE_DRACO_INTRINSICS_CMAKE_)
  return()
endif() # DRACO_CMAKE_DRACO_INTRINSICS_CMAKE_
set(DRACO_CMAKE_DRACO_INTRINSICS_CMAKE_ 1)

# Returns the compiler flag for the SIMD intrinsics suffix specified by the
# SUFFIX argument via the variable specified by the VARIABLE argument:
# draco_get_intrinsics_flag_for_suffix(SUFFIX <suffix> VARIABLE <var name>)
macro(draco_get_intrinsics_flag_for_suffix)
  unset(intrinsics_SUFFIX)
  unset(intrinsics_VARIABLE)
  unset(optional_args)
  unset(multi_value_args)
  set(single_value_args SUFFIX VARIABLE)
  cmake_parse_arguments(intrinsics "${optional_args}" "${single_value_args}"
                        "${multi_value_args}" ${ARGN})

  if(NOT (intrinsics_SUFFIX AND intrinsics_VARIABLE))
    message(FATAL_ERROR "draco_get_intrinsics_flag_for_suffix: SUFFIX and "
                        "VARIABLE required.")
  endif()

  if(intrinsics_SUFFIX MATCHES "neon")
    if(NOT MSVC)
      set(${intrinsics_VARIABLE} "${DRACO_NEON_INTRINSICS_FLAG}")
    endif()
  elseif(intrinsics_SUFFIX MATCHES "sse4")
    if(NOT MSVC)
      set(${intrinsics_VARIABLE} "-msse4.1")
    endif()
  else()
    message(FATAL_ERROR "draco_get_intrinsics_flag_for_suffix: Unknown "
                        "instrinics suffix: ${intrinsics_SUFFIX}")
  endif()

  if(DRACO_VERBOSE GREATER 1)
    message("draco_get_intrinsics_flag_for_suffix: "
            "suffix:${intrinsics_SUFFIX} flag:${${intrinsics_VARIABLE}}")
  endif()
endmacro()

# Processes source files specified by SOURCES and adds intrinsics flags as
# necessary: draco_process_intrinsics_sources(SOURCES <sources>)
#
# Detects requirement for intrinsics flags using source file name suffix.
# Currently supports only SSE4.1.
macro(draco_process_intrinsics_sources)
  unset(arg_TARGET)
  unset(arg_SOURCES)
  unset(optional_args)
  set(single_value_args TARGET)
  set(multi_value_args SOURCES)
  cmake_parse_arguments(arg "${optional_args}" "${single_value_args}"
                        "${multi_value_args}" ${ARGN})
  if(NOT (arg_TARGET AND arg_SOURCES))
    message(FATAL_ERROR "draco_process_intrinsics_sources: TARGET and "
                        "SOURCES required.")
  endif()

  if(DRACO_ENABLE_SSE4_1 AND draco_have_sse4)
    unset(sse4_sources)
    list(APPEND sse4_sources ${arg_SOURCES})

    list(FILTER sse4_sources INCLUDE REGEX "${draco_sse4_source_file_suffix}$")

    if(sse4_sources)
      unset(sse4_flags)
      draco_get_intrinsics_flag_for_suffix(
        SUFFIX ${draco_sse4_source_file_suffix} VARIABLE sse4_flags)
      if(sse4_flags)
        draco_set_compiler_flags_for_sources(SOURCES ${sse4_sources} FLAGS
                                             ${sse4_flags})
      endif()
    endif()
  endif()

  if(DRACO_ENABLE_NEON AND draco_have_neon)
    unset(neon_sources)
    list(APPEND neon_sources ${arg_SOURCES})
    list(FILTER neon_sources INCLUDE REGEX "${draco_neon_source_file_suffix}$")

    if(neon_sources AND DRACO_NEON_INTRINSICS_FLAG)
      unset(neon_flags)
      draco_get_intrinsics_flag_for_suffix(
        SUFFIX ${draco_neon_source_file_suffix} VARIABLE neon_flags)
      if(neon_flags)
        draco_set_compiler_flags_for_sources(SOURCES ${neon_sources} FLAGS
                                             ${neon_flags})
      endif()
    endif()
  endif()
endmacro()
