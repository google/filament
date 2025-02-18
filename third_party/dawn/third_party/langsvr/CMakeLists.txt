# Copyright 2024 The langsvr Authors
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

cmake_minimum_required(VERSION 3.10.2)

project(
    langsvr
    DESCRIPTION "langsvr, a C++ Language Server Protocol library"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()

################################################################################
# Configuration options
################################################################################

# option_if_not_defined(name default description)
# Behaves like:
#   option(name description default)
# If a variable is not already defined with the given name, otherwise the
# function does nothing.
# Simplifies customization by projects that use langsvr as a dependency.
function(option_if_not_defined name default description)
    if(NOT DEFINED ${name})
        option(${name} ${description} ${default})
    endif()
endfunction()

# set_if_not_defined(name value description)
# Behaves like:
#   set(${name} ${value} CACHE STRING ${description})
# If a variable is not already defined with the given name, otherwise the
# function does nothing.
# Simplifies customization by projects that use langsvr as a dependency.
function(set_if_not_defined name value description)
    if(NOT DEFINED ${name})
        set(${name} ${value} CACHE STRING ${description})
    endif()
endfunction()

set_if_not_defined(LANGSVR_THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party" "path to the third_party directory")
set_if_not_defined(LANGSVR_JSON_LIB_DIR "${LANGSVR_THIRD_PARTY_DIR}/jsoncpp" "path to JSON library that langsvr will use")
option_if_not_defined(LANGSVR_BUILD_TESTS true "build the langsvr unittests")

# Detect JSON library in use
if(NOT EXISTS "${LANGSVR_JSON_LIB_DIR}")
    message(FATAL_ERROR "LANGSVR_JSON_LIB_DIR (${LANGSVR_JSON_LIB_DIR}) does not exist")
endif()
if(EXISTS "${LANGSVR_JSON_LIB_DIR}/jsoncppConfig.cmake.in")
    set(LANGSVR_JSON_LIB JSONCPP)
else()
    message(FATAL_ERROR "Unrecognised JSON library at LANGSVR_JSON_LIB_DIR")
endif()

################################################################################
# langsvr
################################################################################
add_library(langsvr
    include/langsvr/json/builder.h
    include/langsvr/json/types.h
    include/langsvr/json/value.h
    include/langsvr/lsp/comparators.h
    include/langsvr/lsp/decode.h
    include/langsvr/lsp/encode.h
    include/langsvr/lsp/lsp.h
    include/langsvr/lsp/primitives.h
    include/langsvr/result.h
    include/langsvr/session.h
    include/langsvr/traits.h
    src/buffer_reader.cc
    src/buffer_writer.cc
    src/content_stream.cc
    src/reader.cc
    src/session.cc
    src/writer.cc
    src/lsp/decode.cc
    src/lsp/encode.cc
    src/lsp/lsp.cc
    src/utils/block_allocator.h
)

target_include_directories(langsvr PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(langsvr PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

if(${LANGSVR_JSON_LIB} STREQUAL JSONCPP)
    if(NOT TARGET jsoncpp_static)
        add_subdirectory("${LANGSVR_THIRD_PARTY_DIR}/jsoncpp" EXCLUDE_FROM_ALL)
    endif()
    target_sources(langsvr PRIVATE src/json/jsoncpp.cc)
    target_link_libraries(langsvr jsoncpp_static)
endif()

################################################################################
# langsvr_tests
################################################################################
if(LANGSVR_BUILD_TESTS)
    if(NOT TARGET gmock)
        add_subdirectory("${LANGSVR_THIRD_PARTY_DIR}/googletest" EXCLUDE_FROM_ALL)
    endif()

    add_executable(langsvr_tests
        src/buffer_reader_test.cc
        src/buffer_writer_test.cc
        src/content_stream_test.cc
        src/json/builder_test.cc
        src/lsp/comparators_test.cc
        src/lsp/decode_test.cc
        src/lsp/encode_test.cc
        src/one_of_test.cc
        src/optional_test.cc
        src/result_test.cc
        src/session_test.cc
        src/span_test.cc
        src/traits_test.cc
        src/utils/block_allocator_test.cc
    )

    target_include_directories(langsvr_tests PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${gmock_SOURCE_DIR}/include"
    )

    target_link_libraries(langsvr_tests
        gmock
        gtest_main
        langsvr
    )
endif()
