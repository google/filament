# Copyright 2024 The Dawn & Tint Authors
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

cmake_policy(SET CMP0048 NEW)
project(webgpu VERSION 1.1.0 LANGUAGES CXX)

cmake_minimum_required(VERSION 3.22.1)

# Hide non-public symbols; set before dawn subdirectory is imported.
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

set(DAWN_BUILD_MONOLITHIC_LIBRARY STATIC)

set(DAWN_DIR "${webgpu_SOURCE_DIR}/../../../../../..")
add_subdirectory(${DAWN_DIR} dawn)

list(INSERT CMAKE_MODULE_PATH 0 "${DAWN_DIR}/src/cmake")
include(DawnCompilerChecks)

# Make the library exposing both Dawn's API and the JNI methods.
find_package(Python3 REQUIRED)
DawnJSONGenerator(
    TARGET "jni"
    PRINT_NAME "Dawn JNI binding"
    OUTPUT_SOURCES WEBGPU_JNI_SOURCES
    OUTPUT_HEADERS WEBGPU_JNI_HEADERS
)

add_library(webgpu_c_bundled SHARED)
target_sources(webgpu_c_bundled PRIVATE
    ${WEBGPU_JNI_SOURCES}
    ${WEBGPU_JNI_HEADERS}
    JNIContext.cpp
    JNIContext.h
    Util.cpp
)

target_include_directories(webgpu_c_bundled PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(webgpu_c_bundled PRIVATE android webgpu_dawn dawn_common dawn_internal_config)

# Copy headers for webgpu_c_bundled over.
add_custom_command(DEPENDS dawncpp_headers POST_BUILD
                   OUTPUT ${CMAKE_SOURCE_DIR}/gen/include/dawn/webgpu.h
                   COMMAND rm -r "${CMAKE_SOURCE_DIR}/gen" || echo
                   COMMAND ${CMAKE_COMMAND} -E copy
                           ${CMAKE_CURRENT_BINARY_DIR}/dawn/gen/include/dawn/webgpu.h
                           ${CMAKE_SOURCE_DIR}/gen/include/dawn/webgpu.h)

# Add unit test build configuration
# To run use the following command:
# ./gradlew -i connectedAndroidTest
find_package(googletest REQUIRED CONFIG)
find_package(junit-gtest REQUIRED CONFIG)
add_library(webgpu_wrapper_tests SHARED ./../../androidTest/cpp/webgpu_wrapper_tests.cpp)
target_include_directories(webgpu_wrapper_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(webgpu_wrapper_tests
  PRIVATE
    webgpu_c_bundled
    android
    atomic
    log
    googletest::gtest
    googletest::gmock
    junit-gtest::junit-gtest
)
