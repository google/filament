# Copyright 2016 Google Inc. All rights reserved.
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

cmake_minimum_required(VERSION 3.5)
project(LibProtobufMutator CXX)

enable_language(C)
enable_language(CXX)

option(LIB_PROTO_MUTATOR_TESTING "Enable test building" ON)
option(LIB_PROTO_MUTATOR_DOWNLOAD_PROTOBUF
       "Automatically download working protobuf" OFF)
option(LIB_PROTO_MUTATOR_WITH_ASAN "Enable address sanitizer" OFF)
option(PKG_CONFIG_PATH "Directory to install pkgconfig file" "share/pkgconfig")
set(LIB_PROTO_MUTATOR_FUZZER_LIBRARIES "" CACHE STRING "Fuzzing engine libs")

# External dependencies
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/external)

# External dependencies
include(ProcessorCount)
include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads)

find_package(LibLZMA)
include_directories(${LIBLZMA_INCLUDE_DIRS})

find_package(ZLIB)
include_directories(${ZLIB_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 11)

include_directories(${PROJECT_SOURCE_DIR})

if (MSVC)
  option(LIB_PROTO_MUTATOR_MSVC_STATIC_RUNTIME "Link static runtime libraries" ON)
  if (LIB_PROTO_MUTATOR_DOWNLOAD_PROTOBUF OR LIB_PROTO_MUTATOR_MSVC_STATIC_RUNTIME)
    # This is the default for protobuf with MSVC
    # http://www.cmake.org/Wiki/CMake_FAQ#How_can_I_build_my_MSVC_application_with_a_static_runtime.3F
    foreach(flag_var
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
      if(${flag_var} MATCHES "/MD")
        string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
      endif(${flag_var} MATCHES "/MD")
    endforeach(flag_var)
  endif (LIB_PROTO_MUTATOR_DOWNLOAD_PROTOBUF OR LIB_PROTO_MUTATOR_MSVC_STATIC_RUNTIME)
endif (MSVC)

set(CMAKE_REQUIRED_FLAGS "-fsanitize=address")
check_cxx_compiler_flag(-fsanitize=address LIB_PROTO_MUTATOR_HAS_SANITIZE_ADDRESS)
check_cxx_compiler_flag("-fsanitize=address -fsanitize-address-use-after-scope"
                        LIB_PROTO_MUTATOR_HAS_SANITIZE_SCOPE)
unset(CMAKE_REQUIRED_FLAGS)

set(CMAKE_REQUIRED_FLAGS "-fsanitize-coverage=0")
check_cxx_compiler_flag(-fsanitize-coverage= LIB_PROTO_MUTATOR_HAS_NO_COVERAGE)
unset(CMAKE_REQUIRED_FLAGS)

set(CMAKE_REQUIRED_FLAGS "-fsanitize=fuzzer-no-link")
check_cxx_compiler_flag(-fsanitize=fuzzer-no-link LIB_PROTO_MUTATOR_HAS_SANITIZE_FUZZER)
unset(CMAKE_REQUIRED_FLAGS)

set(CMAKE_REQUIRED_FLAGS "-fno-sanitize=fuzzer")
check_cxx_compiler_flag(-fno-sanitize=fuzzer LIB_PROTO_MUTATOR_HAS_NO_SANITIZE_FUZZER)
unset(CMAKE_REQUIRED_FLAGS)

check_cxx_compiler_flag(-Wstring-conversion LIB_PROTO_MUTATOR_HAS_WSTRING_CONVERSION)

if (NOT MSVC)
  set(EXTRA_FLAGS "-fno-exceptions -Werror -Wall")
  if (LIB_PROTO_MUTATOR_HAS_WSTRING_CONVERSION)
    set(EXTRA_FLAGS "${EXTRA_FLAGS} -Wstring-conversion")
  endif()
endif(NOT MSVC)

if (LIB_PROTO_MUTATOR_WITH_ASAN)
  if (LIB_PROTO_MUTATOR_HAS_SANITIZE_ADDRESS)
    set(EXTRA_FLAGS "${EXTRA_FLAGS} -fsanitize=address")
    if (LIB_PROTO_MUTATOR_HAS_SANITIZE_SCOPE)
      set(EXTRA_FLAGS "${EXTRA_FLAGS} -fsanitize-address-use-after-scope")
    endif()
  endif()
endif()

# Assume CFLAGS has coverage options if LIB_PROTO_MUTATOR_FUZZER_LIBRARIES was set
if ("${LIB_PROTO_MUTATOR_FUZZER_LIBRARIES}" STREQUAL "")
  if (LIB_PROTO_MUTATOR_HAS_SANITIZE_FUZZER)
    set(FUZZING_FLAGS "-fsanitize=fuzzer-no-link")
    set(FUZZING_FLAGS_BINARY "-fsanitize=fuzzer")
  endif()
  if (LIB_PROTO_MUTATOR_HAS_SANITIZE_NO_FUZZER)
    set(NO_FUZZING_FLAGS "-fno-sanitize=fuzzer")
  endif()
endif()
if (LIB_PROTO_MUTATOR_HAS_NO_COVERAGE)
  set(NO_FUZZING_FLAGS "${NO_FUZZING_FLAGS} -fsanitize-coverage=0")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_FLAGS}")

set(PROTOBUF_CFLAGS "${CMAKE_C_FLAGS} ${NO_FUZZING_FLAGS} -w")
set(PROTOBUF_CXXFLAGS "${CMAKE_CXX_FLAGS} ${NO_FUZZING_FLAGS} -w")
if(CMAKE_USE_PTHREADS_INIT)
  set(PROTOBUF_CFLAGS "${PROTOBUF_CFLAGS} -pthread")
  set(PROTOBUF_CXXFLAGS "${PROTOBUF_CXXFLAGS} -pthread")
endif()

if (LIB_PROTO_MUTATOR_DOWNLOAD_PROTOBUF)
  include(protobuf)
else()
  find_package(Protobuf REQUIRED)
  include_directories(${PROTOBUF_INCLUDE_DIRS})
  include_directories(${CMAKE_CURRENT_BINARY_DIR})
endif()

set(LIB_DIR "lib${LIB_SUFFIX}")

if (LIB_PROTO_MUTATOR_TESTING)
  enable_testing()

  include(googletest)

  if (NOT LIB_PROTO_MUTATOR_CTEST_JOBS)
    ProcessorCount(LIB_PROTO_MUTATOR_CTEST_JOBS)
  endif()
  add_custom_target(check
                    COMMAND ${CMAKE_CTEST_COMMAND} -j${LIB_PROTO_MUTATOR_CTEST_JOBS} --output-on-failure)
endif()

add_subdirectory(src)

if (NOT "${LIB_PROTO_MUTATOR_FUZZER_LIBRARIES}" STREQUAL "" OR
    NOT "${FUZZING_FLAGS}" STREQUAL "")
  add_subdirectory(examples EXCLUDE_FROM_ALL)
endif()

install(EXPORT libprotobuf-mutatorTargets FILE libprotobuf-mutatorTargets.cmake
  NAMESPACE libprotobuf-mutator:: DESTINATION lib/cmake/libprotobuf-mutator)
configure_file(libprotobuf-mutatorConfig.cmake.in libprotobuf-mutatorConfig.cmake @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libprotobuf-mutatorConfig.cmake"
  DESTINATION lib/cmake/libprotobuf-mutator)
configure_file("libprotobuf-mutator.pc.in" "libprotobuf-mutator.pc" @ONLY)
install(FILES "${CMAKE_BINARY_DIR}/libprotobuf-mutator.pc"
  DESTINATION ${PKG_CONFIG_PATH})
install(DIRECTORY ./port ./src DESTINATION include/libprotobuf-mutator
  FILES_MATCHING PATTERN "*.h")
