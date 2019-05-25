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

# Manage platform-specific quirks
#===============================================================================

if(platform_cmake_included)
    return()
endif()
set(platform_cmake_included true)

#include("cmake/utils.cmake")

if(MKLDNN_LIBRARY_TYPE STREQUAL "SHARED")
    add_definitions(-DMKLDNN_DLL -DMKLDNN_DLL_EXPORTS)
endif()

# UNIT8_MAX-like macros are a part of the C99 standard and not a part of the
# C++ standard (see C99 standard 7.18.2 and 7.18.4)
add_definitions(-D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS)

set(CMAKE_CCXX_FLAGS)
set(CMAKE_CCXX_NOWARN_FLAGS)
set(ISA_FLAGS_SSE41)

if(MSVC)
    set(USERCONFIG_PLATFORM "x64")
    # enable intrinsic functions
    append(CMAKE_CXX_FLAGS "/Oi")
    # enable full optimizations
    append(CMAKE_CXX_FLAGS_RELEASE "/Ox")
    append(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/Ox")
    # package individual functions
    append(CMAKE_CXX_FLAGS_RELEASE "/Gy")
    append(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/Gy")
    # compiler specific settings
    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        append(CMAKE_CCXX_FLAGS "/MP")
        # int -> bool
        append(CMAKE_CCXX_NOWARN_FLAGS "/wd4800")
        # unknown pragma
        append(CMAKE_CCXX_NOWARN_FLAGS "/wd4068")
        # double -> float
        append(CMAKE_CCXX_NOWARN_FLAGS "/wd4305")
        # UNUSED(func)
        append(CMAKE_CCXX_NOWARN_FLAGS "/wd4551")
        # int64_t -> int (tent)
        append(CMAKE_CCXX_NOWARN_FLAGS "/wd4244")
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
        append(CMAKE_CCXX_FLAGS "/MP")
        set(ISA_FLAGS_SSE41 "-Qxsse4.1")
        # disable: loop was not vectorized with "simd"
        append(CMAKE_CCXX_NOWARN_FLAGS "-Qdiag-disable:15552")
        append(CMAKE_CCXX_NOWARN_FLAGS "-Qdiag-disable:15335")
        # disable: unknown pragma
        append(CMAKE_CCXX_NOWARN_FLAGS "-Qdiag-disable:3180")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(ISA_FLAGS_SSE41 "-msse4.1")
        # Clang cannot vectorize some loops with #pragma omp simd and gets
        # very upset. Tell it that it's okay and that we love it
        # unconditionally.
        append(CMAKE_CCXX_FLAGS "-Wno-pass-failed")
    endif()
    # disable secure warnings
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
elseif(UNIX OR MINGW)
    append(CMAKE_CCXX_FLAGS "-Wall -Wno-unknown-pragmas")
    append_if_product(CMAKE_CCXX_FLAGS "-Werror")
    append(CMAKE_CCXX_FLAGS "-fvisibility=internal")
    append(CMAKE_C_FLAGS "-std=c99")
    append(CMAKE_CXX_FLAGS "-std=c++11 -fvisibility-inlines-hidden")
    # compiler specific settings
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(ISA_FLAGS_SSE41 "-msse4.1")
        # Clang cannot vectorize some loops with #pragma omp simd and gets
        # very upset. Tell it that it's okay and that we love it
        # unconditionally.
        append(CMAKE_CCXX_NOWARN_FLAGS "-Wno-pass-failed")
        if(MKLDNN_USE_CLANG_SANITIZER MATCHES "Memory(WithOrigin)?")
            if(NOT MKLDNN_THREADING STREQUAL "SEQ")
                message(WARNING "Clang OpenMP is not compatible with MSan! "
                    "Expect a lot of false positives!")
            endif()
            append(CMAKE_CCXX_SANITIZER_FLAGS "-fsanitize=memory")
            if(MKLDNN_USE_CLANG_SANITIZER STREQUAL "MemoryWithOrigin")
                append(CMAKE_CCXX_SANITIZER_FLAGS
                    "-fsanitize-memory-track-origins=2")
                append(CMAKE_CCXX_SANITIZER_FLAGS
                    "-fno-omit-frame-pointer")
            endif()
            set(MKLDNN_ENABLED_CLANG_SANITIZER "${MKLDNN_USE_CLANG_SANITIZER}")
        elseif(MKLDNN_USE_CLANG_SANITIZER STREQUAL "Undefined")
            append(CMAKE_CCXX_SANITIZER_FLAGS "-fsanitize=undefined")
            append(CMAKE_CCXX_SANITIZER_FLAGS
                "-fno-sanitize=function,vptr")  # work around linking problems
            append(CMAKE_CCXX_SANITIZER_FLAGS "-fno-omit-frame-pointer")
            set(MKLDNN_ENABLED_CLANG_SANITIZER "${MKLDNN_USE_CLANG_SANITIZER}")
        elseif(MKLDNN_USE_CLANG_SANITIZER STREQUAL "Address")
            append(CMAKE_CCXX_SANITIZER_FLAGS "-fsanitize=address")
            set(MKLDNN_ENABLED_CLANG_SANITIZER "${MKLDNN_USE_CLANG_SANITIZER}")
        elseif(MKLDNN_USE_CLANG_SANITIZER STREQUAL "Thread")
            append(CMAKE_CCXX_SANITIZER_FLAGS "-fsanitize=thread")
            set(MKLDNN_ENABLED_CLANG_SANITIZER "${MKLDNN_USE_CLANG_SANITIZER}")
        elseif(MKLDNN_USE_CLANG_SANITIZER STREQUAL "Leak")
            append(CMAKE_CCXX_SANITIZER_FLAGS "-fsanitize=leak")
            set(MKLDNN_ENABLED_CLANG_SANITIZER "${MKLDNN_USE_CLANG_SANITIZER}")
        elseif(NOT MKLDNN_USE_CLANG_SANITIZER STREQUAL "")
            message(FATAL_ERROR
                "Unsupported Clang sanitizer '${MKLDNN_USE_CLANG_SANITIZER}'")
        endif()
        if(MKLDNN_ENABLED_CLANG_SANITIZER)
            message(STATUS
                "Using Clang ${MKLDNN_ENABLED_CLANG_SANITIZER} "
                "sanitizer (experimental!)")
            append(CMAKE_CCXX_SANITIZER_FLAGS "-g -fno-omit-frame-pointer")
        endif()
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0)
            set(ISA_FLAGS_SSE41 "-msse4.1")
        endif()
        # suppress warning on assumptions made regarding overflow (#146)
        append(CMAKE_CCXX_NOWARN_FLAGS "-Wno-strict-overflow")
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
        set(ISA_FLAGS_SSE41 "-xsse4.1")
        # workaround for Intel Compiler 16.0 that produces error caused
        # by pragma omp simd collapse(..)
        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "17.0")
            append(CMAKE_CCXX_NOWARN_FLAGS "-diag-disable:13379")
        endif()
        append(CMAKE_CCXX_NOWARN_FLAGS "-diag-disable:15552")
        # disable `was not vectorized: vectorization seems inefficient` remark
        append(CMAKE_CCXX_NOWARN_FLAGS "-diag-disable:15335")
        # disable optimizations in debug mode
        append(CMAKE_CXX_FLAGS_DEBUG "-O0")
    endif()
endif()

if(WIN32)
    string(REPLACE ";" "\;" ENV_PATH "$ENV{PATH}")
    set(CTESTCONFIG_PATH "${CTESTCONFIG_PATH}\;${MKLDLLPATH}\;${ENV_PATH}")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
        # Link Intel and MS libraries statically for release builds
        string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
        string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
    endif()
endif()

if(UNIX OR MINGW)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
        # Link Intel libraries statically (except for iomp5)
        if(MKLDNN_THREADING MATCHES "OMP")
            append(CMAKE_SHARED_LINKER_FLAGS "-liomp5")
        endif()
        append(CMAKE_SHARED_LINKER_FLAGS "-static-intel")
        # Tell linker to not complain about missing static libraries
        append(CMAKE_SHARED_LINKER_FLAGS "-diag-disable:10237")
    endif()
endif()

if(APPLE)
    append(CMAKE_CXX_FLAGS "-mmacosx-version-min=10.7") # makes sure code runs on older macOS versions
    append(CMAKE_CXX_FLAGS "-stdlib=libc++")            # link against libc++ which supports C++11 features
endif()
