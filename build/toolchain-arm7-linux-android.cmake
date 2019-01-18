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

# architecture
set(ARCH arm-linux-androideabi)
set(DIST_ARCH armeabi-v7a)

# toolchain
set(TOOLCHAIN ${CMAKE_SOURCE_DIR}/toolchains/${CMAKE_HOST_SYSTEM_NAME}/${ARCH}-4.9)

# specify the cross compiler
set(COMPILER_SUFFIX)
if(WIN32)
    set(COMPILER_SUFFIX ".cmd")
    set(CMAKE_AR ${TOOLCHAIN}/bin/${ARCH}-ar.exe CACHE FILEPATH "Archiver")
endif()
set(CMAKE_C_COMPILER   ${TOOLCHAIN}/bin/${ARCH}-clang${COMPILER_SUFFIX})
set(CMAKE_CXX_COMPILER ${TOOLCHAIN}/bin/${ARCH}-clang++${COMPILER_SUFFIX})

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
#
# Cortex-A9 only has fpv3, but AFAIK, no Cortex-A9 has GLES 3.x capable GPU
#
# Krait support vfpv4
# Cortex-A12 and Cortex-A15 ARMv7 support neon-fpv4
# Cortex-A7 support fpv4 iif it has neon
#
# float ABI:
# for softpf: CFLAGS must have -mfloat-abi=softfp
# for hardfp: CFLAGS must have -mhard-float
#             LDFLAGS must have -Wl,--no-warn-mismatch
#
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -mthumb -march=armv7-a -mcpu=cortex-a15 -mfloat-abi=softfp -mfpu=neon-vfpv4 -DANDROID -fPIE" CACHE STRING "Toolchain CFLAGS")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS}" CACHE STRING "Toolchain CXXFLAGS")
set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS}    -march=armv7-a -Wl,--no-warn-mismatch -L${TOOLCHAIN}/arm-linux-androideabi/lib/armv7-a -static-libstdc++ -fPIE -pie" CACHE STRING "Toolchain LDFLAGS")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -march=armv7-a -Wl,--no-warn-mismatch -L${TOOLCHAIN}/arm-linux-androideabi/lib/armv7-a -static-libstdc++" CACHE STRING "Toolchain LDFLAGS")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(ANDROID TRUE)
