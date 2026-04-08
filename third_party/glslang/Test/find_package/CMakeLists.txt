# Copyright (C) 2025 The Khronos Group Inc.
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#    Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
#    Redistributions in binary form must reproduce the above
#    copyright notice, this list of conditions and the following
#    disclaimer in the documentation and/or other materials provided
#    with the distribution.
#
#    Neither the name of The Khronos Group Inc. nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
cmake_minimum_required(VERSION 3.22.1)

# NOTE: The whole point of this file is to test what the client will
# see when they install glslang via a package manager.
project(FP_TEST LANGUAGES C CXX)

# Assume the user install here to simplify CI
# It also makes local testing easier.
list(APPEND CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/../../build/install")
find_package(glslang REQUIRED CONFIG)

if (NOT DEFINED glslang_VERSION)
    message(FATAL_ERROR "glslang_VERSION not defined!")
endif()
message(STATUS "glslang_VERSION = ${glslang_VERSION}")

get_target_property(target_type glslang::glslang TYPE)
message(STATUS "glslang::glslang is ${target_type}")

add_executable(example example.c)
target_link_libraries(example PRIVATE
    glslang::glslang
    glslang::glslang-default-resource-limits    
)

if (NOT TARGET glslang::glslang-standalone)
    message(FATAL_ERROR "glslang::glslang-standalone does not exist!")
endif()

if (target_type STREQUAL STATIC_LIBRARY)
    # When these stub libraries are removed from  the future. Simply remove this code.
    # For now it ensures backcompat is preserved.
    add_library(stub STATIC stub.cpp)
    target_link_libraries(example PRIVATE
        glslang::GenericCodeGen
        glslang::MachineIndependent
        glslang::OSDependent
        glslang::SPIRV
    )

    # Ensure these executables run without issue
    add_custom_target(test-executables ALL
        COMMAND glslang::glslang-standalone --version
    )
endif()
