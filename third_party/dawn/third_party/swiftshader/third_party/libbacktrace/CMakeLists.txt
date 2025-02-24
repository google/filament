# Copyright 2020 The SwiftShader Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set(LIBBACKTRACE_SRC_FILES
    src/atomic.c
    src/backtrace.c
    src/backtrace.h
    src/dwarf.c
    src/fileline.c
    src/filenames.h
    src/internal.h
    src/mmap.c
    src/mmapio.c
    src/posix.c
    src/print.c
    src/simple.c
    src/sort.c
    src/state.c
    src/backtrace-supported.h
    src/config.h
)

if(WIN32)
    list(APPEND LIBBACKTRACE_SRC_FILES src/pecoff.c)
elseif(LINUX)
    list(APPEND LIBBACKTRACE_SRC_FILES src/elf.c)
    list(APPEND LIBBACKTRACE_PRIVATE_INCLUDE_DIRS "config/linux/include")
elseif(APPLE)
    message(FATAL_ERROR "libbacktrace does not support mach-o yet")
    list(APPEND LIBBACKTRACE_PRIVATE_INCLUDE_DIRS "config/darwin/include")
endif()

add_library(libbacktrace STATIC EXCLUDE_FROM_ALL
    ${LIBBACKTRACE_SRC_FILES}
)

set_target_properties(libbacktrace PROPERTIES
    POSITION_INDEPENDENT_CODE 1
    FOLDER "Core"
)

target_include_directories(libbacktrace
    PUBLIC
        "src"
    PRIVATE
        ${LIBBACKTRACE_PRIVATE_INCLUDE_DIRS}
)
