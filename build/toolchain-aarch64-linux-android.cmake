# Copyright (C) 2018 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# For information on how to generate the toolchain, see filament/README.md

# this one is important
set(CMAKE_SYSTEM_NAME Linux)

# this one not so much
set(CMAKE_SYSTEM_VERSION 1)

# android
set(API_LEVEL 21)

# architecture
set(ARCH aarch64-linux-android)
set(DIST_ARCH arm64-v8a)

# toolchain
string(TOLOWER ${CMAKE_HOST_SYSTEM_NAME} HOST_NAME_L)
file(TO_CMAKE_PATH $ENV{ANDROID_HOME} ANDROID_HOME_UNIX)

message(STATUS "Try using NDK \'${FILAMENT_NDK_VERSION}\'")
if (NOT FILAMENT_NDK_VERSION)
    file(READ "${CMAKE_CURRENT_LIST_DIR}/common/versions" VERSIONS_STR)
    string(REGEX MATCH "GITHUB_NDK_VERSION=(\\d+)" _UNUSED ${VERSIONS_STR})
    if(CMAKE_MATCH_1)
        set(FILAMENT_NDK_VERSION "${CMAKE_MATCH_1}")
    endif()
endif()
message(STATUS "Using NDK \'${FILAMENT_NDK_VERSION}\'")

file(GLOB NDK_VERSIONS LIST_DIRECTORIES true ${ANDROID_HOME_UNIX}/ndk/${FILAMENT_NDK_VERSION}*)
list(SORT NDK_VERSIONS)
list(GET NDK_VERSIONS -1 NDK_VERSION)
get_filename_component(NDK_VERSION ${NDK_VERSION} NAME)
set(TOOLCHAIN ${ANDROID_HOME_UNIX}/ndk/${NDK_VERSION}/toolchains/llvm/prebuilt/${HOST_NAME_L}-x86_64)

# specify the cross compiler
set(COMPILER_SUFFIX)
set(TOOL_SUFFIX)
if(WIN32)
    set(COMPILER_SUFFIX ".cmd")
    set(TOOL_SUFFIX     ".exe")
endif()
set(CMAKE_C_COMPILER   ${TOOLCHAIN}/bin/${ARCH}${API_LEVEL}-clang${COMPILER_SUFFIX})
set(CMAKE_CXX_COMPILER ${TOOLCHAIN}/bin/${ARCH}${API_LEVEL}-clang++${COMPILER_SUFFIX})
set(CMAKE_AR           ${TOOLCHAIN}/bin/llvm-ar${TOOL_SUFFIX} CACHE FILEPATH "Archiver")
set(CMAKE_RANLIB       ${TOOLCHAIN}/bin/llvm-ranlib${TOOL_SUFFIX})
set(CMAKE_STRIP        ${TOOLCHAIN}/bin/llvm-strip${TOOL_SUFFIX})

# where is the target environment
set(CMAKE_FIND_ROOT_PATH  ${TOOLCHAIN}/sysroot)

# search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# compiler and linker flags
# note for gcc add:
#   C_FLAGS += -Wl,-pie
#   CXX_FLAGS += -lstdc++

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIE -mcpu=cortex-a57" CACHE STRING "Toolchain CFLAGS")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS}" CACHE STRING "Toolchain CXXFLAGS")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fPIE -pie -static-libstdc++" CACHE STRING "Toolchain LDFLAGS")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static-libstdc++" CACHE STRING "Toolchain LDFLAGS")

set(ANDROID TRUE)
set(EGL TRUE)

# we are compiling Android on Windows
set(ANDROID_ON_WINDOWS FALSE)
if(WIN32)
    set(ANDROID_ON_WINDOWS TRUE)
endif()
