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
set(ARCH i686-linux-android)
set(DIST_ARCH x86)

# toolchain
string(TOLOWER ${CMAKE_HOST_SYSTEM_NAME} HOST_NAME_L)
set(TOOLCHAIN $ENV{ANDROID_HOME}/ndk-bundle/toolchains/llvm/prebuilt/${HOST_NAME_L}-x86_64/)

# specify the cross compiler
set(COMPILER_SUFFIX)
set(TOOL_SUFFIX)
if(WIN32)
    set(COMPILER_SUFFIX ".cmd")
    set(TOOL_SUFFIX     ".exe")
endif()
set(CMAKE_C_COMPILER   ${TOOLCHAIN}/bin/${ARCH}${API_LEVEL}-clang${COMPILER_SUFFIX})
set(CMAKE_CXX_COMPILER ${TOOLCHAIN}/bin/${ARCH}${API_LEVEL}-clang++${COMPILER_SUFFIX})
set(CMAKE_AR           ${TOOLCHAIN}/bin/${ARCH}-ar${TOOL_SUFFIX}  CACHE FILEPATH "Archiver")
set(CMAKE_RANLIB       ${TOOLCHAIN}/bin/${ARCH}-ranlib${TOOL_SUFFIX})
set(CMAKE_STRIP        ${TOOLCHAIN}/bin/${ARCH}-strip${TOOL_SUFFIX})

# where is the target environment
set(CMAKE_FIND_ROOT_PATH  ${TOOLCHAIN}/sysroot)

# search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# compiler and linker flags
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -DANDROID -fPIE" CACHE STRING "Toolchain CFLAGS")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS}" CACHE STRING "Toolchain CXXFLAGS")
set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS}    -Wl,--no-warn-mismatch -L${TOOLCHAIN}/i686-linux-android/lib64/ -static-libstdc++ -fPIE -pie" CACHE STRING "Toolchain LDFLAGS")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-warn-mismatch -L${TOOLCHAIN}/i686-linux-android/lib64/ -static-libstdc++" CACHE STRING "Toolchain LDFLAGS")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(ANDROID TRUE)

# we are compiling Android on Windows
set(ANDROID_ON_WINDOWS FALSE)
if(WIN32)
    set(ANDROID_ON_WINDOWS TRUE)
endif()
