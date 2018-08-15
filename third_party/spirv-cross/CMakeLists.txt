# Copyright 2016 Google Inc.
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

cmake_minimum_required(VERSION 2.8)
project(SPIRV-Cross)
enable_testing()

option(SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS "Instead of throwing exceptions assert" OFF)

if(${CMAKE_GENERATOR} MATCHES "Makefile")
  if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
    message(FATAL_ERROR "Build out of tree to avoid overwriting Makefile")
  endif()
endif()

set(spirv-compiler-options "")
set(spirv-compiler-defines "")

if(SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS)
  set(spirv-compiler-defines ${spirv-compiler-defines} SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS)
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
  target_include_directories(${name} PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
      $<INSTALL_INTERFACE:include/spirv_cross>)
  set_target_properties(${name} PROPERTIES
      PUBLIC_HEADERS "${hdrs}")
  target_compile_options(${name} PRIVATE ${spirv-compiler-options})
  target_compile_definitions(${name} PRIVATE ${spirv-compiler-defines})
  install(TARGETS ${name}
      EXPORT ${config_name}Config
      RUNTIME DESTINATION bin
      LIBRARY DESTINATION lib
      ARCHIVE DESTINATION lib
      PUBLIC_HEADER DESTINATION include/spirv_cross)
  install(FILES ${hdrs} DESTINATION include/spirv_cross)
  install(EXPORT ${config_name}Config DESTINATION share/${config_name}/cmake)
  export(TARGETS ${name} FILE ${config_name}Config.cmake)
endmacro()


spirv_cross_add_library(spirv-cross-core spirv_cross_core STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/GLSL.std.450.h
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv_common.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv_cross.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv_cross.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv_cfg.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv_cfg.cpp)

spirv_cross_add_library(spirv-cross-glsl spirv_cross_glsl STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv_glsl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv_glsl.hpp)

spirv_cross_add_library(spirv-cross-cpp spirv_cross_cpp STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv_cpp.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv_cpp.cpp)

spirv_cross_add_library(spirv-cross-reflect spirv_cross_reflect STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv_reflect.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv_reflect.cpp)

spirv_cross_add_library(spirv-cross-msl spirv_cross_msl STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv_msl.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv_msl.cpp)

spirv_cross_add_library(spirv-cross-hlsl spirv_cross_hlsl STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv_hlsl.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv_hlsl.cpp)

spirv_cross_add_library(spirv-cross-util spirv_cross_util STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv_cross_util.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv_cross_util.cpp)

add_executable(spirv-cross main.cpp)
target_compile_options(spirv-cross PRIVATE ${spirv-compiler-options})
target_compile_definitions(spirv-cross PRIVATE ${spirv-compiler-defines})

install(TARGETS spirv-cross RUNTIME DESTINATION bin)
target_link_libraries(spirv-cross spirv-cross-glsl spirv-cross-hlsl spirv-cross-cpp spirv-cross-reflect spirv-cross-msl spirv-cross-util spirv-cross-core)
target_link_libraries(spirv-cross-util spirv-cross-core)
target_link_libraries(spirv-cross-glsl spirv-cross-core)
target_link_libraries(spirv-cross-msl spirv-cross-glsl)
target_link_libraries(spirv-cross-hlsl spirv-cross-glsl)
target_link_libraries(spirv-cross-cpp spirv-cross-glsl)

# Set up tests, using only the simplest modes of the test_shaders
# script.  You have to invoke the script manually to:
#  - Update the reference files
#  - Get cycle counts from malisc
#  - Keep failing outputs
find_package(PythonInterp)
if (${PYTHONINTERP_FOUND})
  if (${PYTHON_VERSION_MAJOR} GREATER 2)
	add_test(NAME spirv-cross-test
		COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_shaders.py
			${CMAKE_CURRENT_SOURCE_DIR}/shaders)
	add_test(NAME spirv-cross-test-no-opt
		COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_shaders.py
			${CMAKE_CURRENT_SOURCE_DIR}/shaders-no-opt)
	add_test(NAME spirv-cross-test-metal
		COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_shaders.py --metal
			${CMAKE_CURRENT_SOURCE_DIR}/shaders-msl)
	add_test(NAME spirv-cross-test-metal-no-opt
		COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_shaders.py --metal
			${CMAKE_CURRENT_SOURCE_DIR}/shaders-msl-no-opt)
	add_test(NAME spirv-cross-test-hlsl
		COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_shaders.py --hlsl
			${CMAKE_CURRENT_SOURCE_DIR}/shaders-hlsl)
	add_test(NAME spirv-cross-test-hlsl-no-opt
		COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_shaders.py --hlsl
			${CMAKE_CURRENT_SOURCE_DIR}/shaders-hlsl-no-opt)
	add_test(NAME spirv-cross-test-opt
		COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_shaders.py --opt
			${CMAKE_CURRENT_SOURCE_DIR}/shaders)
	add_test(NAME spirv-cross-test-metal-opt
		COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_shaders.py --metal --opt
			${CMAKE_CURRENT_SOURCE_DIR}/shaders-msl)
	add_test(NAME spirv-cross-test-hlsl-opt
		COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_shaders.py --hlsl --opt
			${CMAKE_CURRENT_SOURCE_DIR}/shaders-hlsl)
  endif()
else()
  message(WARNING "Testing disabled. Could not find python3. If you have python3 installed try running "
		  "cmake with -DPYTHON_EXECUTABLE:FILEPATH=/path/to/python3 to help it find the executable")
endif()
