#===============================================================================
# Copyright 2016-2018 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#===============================================================================

if (NOT MKLDNN_BUILD_TESTS)
    return()
endif()

if(POLICY CMP0065)
    cmake_policy(SET CMP0065 NEW)
endif()

# propagate TEST specific flags
append(CMAKE_C_FLAGS "${CMAKE_TEST_CCXX_FLAGS}")
append(CMAKE_CXX_FLAGS "${CMAKE_TEST_CCXX_FLAGS}")

set(CMAKE_TEST_CCXX_NOWARN_FLAGS)

# propagate no warning flags
append(CMAKE_TEST_CCXX_NOWARN_FLAGS "${CMAKE_CCXX_NOWARN_FLAGS}")

# propagate sanitizer flags
append(CMAKE_C_FLAGS "${CMAKE_CCXX_SANITIZER_FLAGS}")
append(CMAKE_CXX_FLAGS "${CMAKE_CCXX_SANITIZER_FLAGS}")

# allow tests to include internal header files with, e.g.
# include "src/common/mkldnn_thread.hpp"
include_directories(${PROJECT_SOURCE_DIR})

if(UNIX OR MINGW)
    # workaround for Intel Compiler 16.0 that doesn't suppress warning on
    # deprecation with "-Wno-deprecated" compiler flag
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel"
            AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "17.0")
        append(CMAKE_TEST_CCXX_NOWARN_FLAGS "-diag-disable:1478")
    else()
        append(CMAKE_TEST_CCXX_NOWARN_FLAGS "-Wno-deprecated-declarations")
    endif()
elseif(WIN32 AND NOT MINGW)
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
        # 1478, 1786: deprecated functions
        append(CMAKE_TEST_CCXX_NOWARN_FLAGS "/Qdiag-disable:1478")
        append(CMAKE_TEST_CCXX_NOWARN_FLAGS "/Qdiag-disable:1786")
        # 2415: unused variable
        append(CMAKE_TEST_CCXX_NOWARN_FLAGS "/Qdiag-disable:2415")
    else()
        # c4244: conversion with possible loss of data
        # c4996: unsafe / deprecated functions
        append(CMAKE_TEST_CCXX_NOWARN_FLAGS "/wd4996 /wd4244")
    endif()
endif()

append(CMAKE_C_FLAGS "${CMAKE_TEST_CCXX_NOWARN_FLAGS}")
append(CMAKE_CXX_FLAGS "${CMAKE_TEST_CCXX_NOWARN_FLAGS}")

register_exe(api-c api.c "test")

if(UNIX OR MINGW)
    set(test_c_symbols "${CMAKE_CURRENT_BINARY_DIR}/test_c_symbols.c")
    add_custom_command(
        OUTPUT ${test_c_symbols}
        COMMAND /bin/bash ${CMAKE_CURRENT_SOURCE_DIR}/generate_c_symbols_refs.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/.. ${PROJECT_BINARY_DIR}/include ${test_c_symbols}
    )
    register_exe(test_c_symbols-c ${test_c_symbols} "test")
# elseif(WIN32)
# No Windows support for: test_c_symbols.c
endif()

add_subdirectory(gtests)
add_subdirectory(benchdnn)
