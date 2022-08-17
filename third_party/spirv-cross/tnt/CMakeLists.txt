# Copyright 2018 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.19)
project(SPIRV-Cross)

# Use assertions instead of exceptions so we can succesfully compile with -fno-exceptions.
option(SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS "Instead of throwing exceptions assert" ON)

if(${CMAKE_GENERATOR} MATCHES "Makefile")
  if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
    message(FATAL_ERROR "Build out of tree to avoid overwriting Makefile")
  endif()
endif()

set(spirv-compiler-options "")
set(spirv-compiler-defines "")
set(spirv-compiler-public-defines "")

if(SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS)
  # This needs to be public, because exceptions are exposed in public headers.
  set(spirv-compiler-public-defines ${spirv-compiler-public-defines} SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS)
endif()

# To specify special debug or optimization options, use
# -DCMAKE_CXX_COMPILE_FLAGS
# However, we require the C++11 dialect.
if (NOT "${MSVC}")
  set(spirv-compiler-options ${spirv-compiler-options} -std=c++11 -Wall -Wextra -Werror -Wshadow)
  set(spirv-compiler-defines ${spirv-compiler-defines} __STDC_LIMIT_MACROS)

  if(SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS)
    set(spirv-compiler-options ${spirv-compiler-options} -fno-exceptions)
  endif()
endif()

macro(extract_headers out_abs file_list)
  set(${out_abs}) # absolute paths
  foreach(_a ${file_list})
    # get_filename_component only returns the longest extension, so use a regex
    string(REGEX REPLACE ".*\\.(h|hpp)" "\\1" ext ${_a})
    if(("${ext}" STREQUAL "h") OR ("${ext}" STREQUAL "hpp"))
      list(APPEND ${out_abs} "${_a}")
    endif()
  endforeach()
endmacro()

macro(spirv_cross_add_library name config_name)
  add_library(${name} ${ARGN})
  extract_headers(hdrs "${ARGN}")
  target_include_directories(${name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
  set_target_properties(${name} PROPERTIES
      PUBLIC_HEADERS "${hdrs}")
  target_compile_options(${name} PRIVATE ${spirv-compiler-options})
  target_compile_definitions(${name} PRIVATE ${spirv-compiler-defines})
  target_compile_definitions(${name} PUBLIC ${spirv-compiler-public-defines})
  target_compile_options(${name} PRIVATE $<$<PLATFORM_ID:Linux>:-fPIC>)
endmacro()

spirv_cross_add_library(spirv-cross-core spirv_cross_core STATIC
    ../GLSL.std.450.h
    ../spirv_common.hpp
    ../spirv_cross_containers.hpp
    ../spirv_cross_error_handling.hpp
    ../spirv.hpp
    ../spirv_cross.hpp
    ../spirv_cross.cpp
    ../spirv_parser.hpp
    ../spirv_parser.cpp
    ../spirv_cross_parsed_ir.hpp
    ../spirv_cross_parsed_ir.cpp
    ../spirv_cfg.hpp
    ../spirv_cfg.cpp)

spirv_cross_add_library(spirv-cross-glsl spirv_cross_glsl STATIC
    ../spirv_glsl.cpp
    ../spirv_glsl.hpp)

spirv_cross_add_library(spirv-cross-msl spirv_cross_msl STATIC
    ../spirv_msl.cpp
    ../spirv_msl.hpp)

target_link_libraries(spirv-cross-glsl spirv-cross-msl spirv-cross-core)

set_target_properties(spirv-cross-glsl PROPERTIES FOLDER ThirdParty)
set_target_properties(spirv-cross-msl PROPERTIES FOLDER ThirdParty)
set_target_properties(spirv-cross-core PROPERTIES FOLDER ThirdParty)