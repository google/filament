# Copyright 2023 The Dawn & Tint Authors
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

# A simplified version of protobuf_generate()
function(generate_protos)
  set(OPTIONS APPEND_PATH)
  set(SINGLE_ARGS TARGET LANGUAGE EXPORT_MACRO PROTOC_OUT_DIR PLUGIN PLUGIN_OPTIONS)
  set(MULTI_ARGS IMPORT_DIRS GENERATE_EXTENSIONS PROTOC_OPTIONS)
  cmake_parse_arguments(ARGS "${OPTIONS}" "${SINGLE_ARGS}" "${MULTI_ARGS}" "${ARGN}")

  if(NOT ARGS_TARGET)
    message(FATAL_ERROR "generate_protos called without a target")
  endif()

  if(NOT ARGS_LANGUAGE)
    set(ARGS_LANGUAGE cpp)
  endif()
  string(TOLOWER ${ARGS_LANGUAGE} ARGS_LANGUAGE)

  if(NOT ARGS_PROTOC_OUT_DIR)
    set(ARGS_PROTOC_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
  endif()

  foreach(OPTION ${ARGS_PLUGIN_OPTIONS})
    # append comma - not using CMake lists and string replacement as users
    # might have semicolons in options
    if(PLUGIN_OPTIONS)
      set( PLUGIN_OPTIONS "${PLUGIN_OPTIONS},")
    endif()
    set(PLUGIN_OPTIONS "${PLUGIN_OPTIONS}${OPTION}")
  endforeach()

  if(ARGS_PLUGIN)
      set(_plugin "--plugin=${ARGS_PLUGIN}")
  endif()

  if(NOT ARGS_GENERATE_EXTENSIONS)
    if(ARGS_LANGUAGE STREQUAL cpp)
      set(ARGS_GENERATE_EXTENSIONS .pb.h .pb.cc)
    elseif(ARGS_LANGUAGE STREQUAL python)
      set(ARGS_GENERATE_EXTENSIONS _pb2.py)
    else()
      message(FATAL_ERROR "generate_protos given unknown Language ${LANGUAGE}, please provide a value for GENERATE_EXTENSIONS")
    endif()
  endif()

  if(ARGS_TARGET)
    get_target_property(SOURCE_LIST ${ARGS_TARGET} SOURCES)
    foreach(FILE ${SOURCE_LIST})
      if(FILE MATCHES ".proto$")
        list(APPEND PROTO_FILES ${FILE})
      endif()
    endforeach()
  endif()

  if(NOT PROTO_FILES)
    message(FATAL_ERROR "generate_protos could not find any .proto files")
  endif()

  if(ARGS_APPEND_PATH)
    # Create an include path for each file specified
    foreach(FILE ${PROTO_FILES})
      get_filename_component(ABS_FILE ${FILE} ABSOLUTE)
      get_filename_component(ABS_PATH ${ABS_FILE} PATH)
      list(FIND PROTOBUF_INCLUDE_PATH ${ABS_PATH} FOUND)
      if(${FOUND} EQUAL -1)
          list(APPEND PROTOBUF_INCLUDE_PATH -I ${ABS_PATH})
      endif()
    endforeach()
  endif()

  foreach(DIR ${ARGS_IMPORT_DIRS})
    get_filename_component(ABS_PATH ${DIR} ABSOLUTE)
    list(FIND PROTOBUF_INCLUDE_PATH ${ABS_PATH} FOUND)
    if(${FOUND} EQUAL -1)
        list(APPEND PROTOBUF_INCLUDE_PATH -I ${ABS_PATH})
    endif()
  endforeach()

  if(NOT PROTOBUF_INCLUDE_PATH)
    set(PROTOBUF_INCLUDE_PATH -I ${CMAKE_CURRENT_SOURCE_DIR})
  endif()

  set(ALL_GENERATED_SRCS)
  foreach(PROTO_FILE ${PROTO_FILES})
    get_filename_component(ABS_FILE ${PROTO_FILE} ABSOLUTE)
    get_filename_component(ABS_DIR ${ABS_FILE} DIRECTORY)

    get_filename_component(FILE_FULL_NAME ${PROTO_FILE} NAME)
    string(FIND "${FILE_FULL_NAME}" "." FILE_LAST_EXT_POS REVERSE)
    string(SUBSTRING "${FILE_FULL_NAME}" 0 ${FILE_LAST_EXT_POS} BASENAME)

    set(SUITABLE_INCLUDE_FOUND FALSE)
    foreach(DIR ${PROTOBUF_INCLUDE_PATH})
      if(NOT DIR STREQUAL "-I")
        file(RELATIVE_PATH REL_DIR ${DIR} ${ABS_DIR})
        string(FIND "${REL_DIR}" "../" IS_IN_PARENT_FOLDER)
        if (NOT ${IS_IN_PARENT_FOLDER} EQUAL 0)
          set(SUITABLE_INCLUDE_FOUND TRUE)
          break()
        endif()
      endif()
    endforeach()

    if(NOT SUITABLE_INCLUDE_FOUND)
      message(FATAL_ERROR "generate_protos could not find any correct proto include directory.")
    endif()

    set(GENERATED_SRCS)
    foreach(EXT ${ARGS_GENERATE_EXTENSIONS})
      list(APPEND GENERATED_SRCS "${ARGS_PROTOC_OUT_DIR}/${REL_DIR}/${BASENAME}${EXT}")
    endforeach()
    list(APPEND ALL_GENERATED_SRCS ${GENERATED_SRCS})

    set(COMMENT "Running ${ARGS_LANGUAGE} protocol buffer compiler on ${PROTO_FILE}")
    if(ARGS_PROTOC_OPTIONS)
      set(COMMENT "${COMMENT}, protoc-options: ${ARGS_PROTOC_OPTIONS}")
    endif()
    if(PLUGIN_OPTIONS)
      set(COMMENT "${COMMENT}, plugin-options: ${PLUGIN_OPTIONS}")
    endif()

    file(MAKE_DIRECTORY "${ARGS_PROTOC_OUT_DIR}/${REL_DIR}")

    if(PLUGIN_OPTIONS)
      set(lang_out_arg "--${ARGS_LANGUAGE}_out=${PLUGIN_OPTIONS}:${ARGS_PROTOC_OUT_DIR}")
    else()
      set(lang_out_arg "--${ARGS_LANGUAGE}_out=${ARGS_PROTOC_OUT_DIR}")
    endif()

    if (PROTOC_EXECUTABLE)
      set(PROTOC_CMD ${PROTOC_EXECUTABLE})
      set(PROTOC_DEPENDS "")
    else()
      set(PROTOC_CMD $<TARGET_FILE:protobuf::protoc>)
      set(PROTOC_DEPENDS $<TARGET_FILE:protobuf::protoc>)
    endif()

    add_custom_command(
      OUTPUT ${GENERATED_SRCS}
      COMMAND ${PROTOC_CMD}
      ARGS ${ARGS_PROTOC_OPTIONS} ${lang_out_arg} ${_plugin} ${PROTOBUF_INCLUDE_PATH} ${ABS_FILE}
      DEPENDS ${ABS_FILE} ${PROTOC_DEPENDS}
      COMMENT ${COMMENT}
      VERBATIM)
  endforeach()

  set_source_files_properties(${ALL_GENERATED_SRCS} PROPERTIES GENERATED TRUE)
  if(ARGS_TARGET)
    target_sources(${ARGS_TARGET} PRIVATE ${ALL_GENERATED_SRCS})
  endif()
endfunction()

set(protobuf_INSTALL OFF CACHE BOOL "Install protobuf binaries and files" FORCE)
set(protobuf_BUILD_CONFORMANCE OFF CACHE BOOL "Build conformance tests" FORCE)
set(protobuf_BUILD_EXAMPLES OFF CACHE BOOL "Build examples" FORCE)
set(protobuf_BUILD_LIBPROTOC OFF CACHE BOOL "Build libprotoc" FORCE)
set(protobuf_BUILD_TESTS OFF CACHE BOOL "Controls whether protobuf tests are built" FORCE)
set(protobuf_MSVC_STATIC_RUNTIME OFF CACHE BOOL "Controls whether a protobuf static runtime is built" FORCE)

if (NOT PROTOC_EXECUTABLE AND Protobuf_PROTOC_EXECUTABLE)
  set(PROTOC_EXECUTABLE ${Protobuf_PROTOC_EXECUTABLE})
endif()

if (CMAKE_CROSSCOMPILING AND NOT PROTOC_EXECUTABLE AND NOT CMAKE_CROSSCOMPILING_EMULATOR)
  message(FATAL_ERROR "When cross-compiling, you must specify a host protoc via -DPROTOC_EXECUTABLE=... or provide a CMAKE_CROSSCOMPILING_EMULATOR.")
endif()

if (PROTOC_EXECUTABLE)
  set(protobuf_BUILD_PROTOC_BINARIES OFF CACHE BOOL "Build libprotoc and protoc compiler" FORCE)
else()
  set(protobuf_BUILD_PROTOC_BINARIES ON CACHE BOOL "Build libprotoc and protoc compiler" FORCE)
endif()
set(protobuf_DISABLE_RTTI ON CACHE BOOL "Remove runtime type information in the binaries" FORCE)

# Well Known Types (WKTs) are no longer checked into the upstream
# chromium repository, so need to intercept usages of them and turn
# them into a dependency on a new target that builds them.

# Intercept file_lists.cmake to remove missing WKTs and dependent subsystems before add_subdirectory
set(protobuf_SOURCE_DIR "${DAWN_PROTOBUF_DIR}")
include("${DAWN_PROTOBUF_DIR}/src/file_lists.cmake")

set(OFFENDING_FILES
  "${protobuf_SOURCE_DIR}/src/google/protobuf/any.pb.cc"
  "${protobuf_SOURCE_DIR}/src/google/protobuf/api.pb.cc"
  "${protobuf_SOURCE_DIR}/src/google/protobuf/duration.pb.cc"
  "${protobuf_SOURCE_DIR}/src/google/protobuf/empty.pb.cc"
  "${protobuf_SOURCE_DIR}/src/google/protobuf/field_mask.pb.cc"
  "${protobuf_SOURCE_DIR}/src/google/protobuf/source_context.pb.cc"
  "${protobuf_SOURCE_DIR}/src/google/protobuf/struct.pb.cc"
  "${protobuf_SOURCE_DIR}/src/google/protobuf/timestamp.pb.cc"
  "${protobuf_SOURCE_DIR}/src/google/protobuf/type.pb.cc"
  "${protobuf_SOURCE_DIR}/src/google/protobuf/wrappers.pb.cc"
  "${protobuf_SOURCE_DIR}/src/google/protobuf/any.pb.h"
  "${protobuf_SOURCE_DIR}/src/google/protobuf/api.pb.h"
  "${protobuf_SOURCE_DIR}/src/google/protobuf/duration.pb.h"
  "${protobuf_SOURCE_DIR}/src/google/protobuf/empty.pb.h"
  "${protobuf_SOURCE_DIR}/src/google/protobuf/field_mask.pb.h"
  "${protobuf_SOURCE_DIR}/src/google/protobuf/source_context.pb.h"
  "${protobuf_SOURCE_DIR}/src/google/protobuf/struct.pb.h"
  "${protobuf_SOURCE_DIR}/src/google/protobuf/timestamp.pb.h"
  "${protobuf_SOURCE_DIR}/src/google/protobuf/type.pb.h"
  "${protobuf_SOURCE_DIR}/src/google/protobuf/wrappers.pb.h"
  "${protobuf_SOURCE_DIR}/src/google/protobuf/util/type_resolver_util.cc"
  "${protobuf_SOURCE_DIR}/src/google/protobuf/util/type_resolver_util.h"
  "${protobuf_SOURCE_DIR}/src/google/protobuf/util/type_resolver_util_test.cc"
  "${protobuf_SOURCE_DIR}/src/google/protobuf/util/field_mask_util.cc"
  "${protobuf_SOURCE_DIR}/src/google/protobuf/util/field_mask_util.h"
  "${protobuf_SOURCE_DIR}/src/google/protobuf/util/field_mask_util_test.cc"
  "${protobuf_SOURCE_DIR}/src/google/protobuf/util/time_util.cc"
  "${protobuf_SOURCE_DIR}/src/google/protobuf/util/time_util.h"
  "${protobuf_SOURCE_DIR}/src/google/protobuf/util/time_util_test.cc"
)

list(REMOVE_ITEM libprotobuf_srcs ${OFFENDING_FILES})
list(REMOVE_ITEM libprotobuf_hdrs ${OFFENDING_FILES})
list(REMOVE_ITEM util_test_files ${OFFENDING_FILES})

# Also remove everything in src/google/protobuf/json from the lists.
list(FILTER libprotobuf_srcs EXCLUDE REGEX "src/google/protobuf/json")
list(FILTER libprotobuf_hdrs EXCLUDE REGEX "src/google/protobuf/json")

add_subdirectory("${DAWN_PROTOBUF_DIR}")

# Defining a separate library for WKTs and linking it to libprotobuf
# as an INTERFACE dependency. To break the circular dependency with
# protoc (which links libprotobuf), only link libprotobuf_wkt if the
# consuming target does NOT have the SKIP_WKT property set.
set(WKT_PROTOS
  "${DAWN_PROTOBUF_DIR}/src/google/protobuf/any.proto"
  "${DAWN_PROTOBUF_DIR}/src/google/protobuf/api.proto"
  "${DAWN_PROTOBUF_DIR}/src/google/protobuf/duration.proto"
  "${DAWN_PROTOBUF_DIR}/src/google/protobuf/empty.proto"
  "${DAWN_PROTOBUF_DIR}/src/google/protobuf/field_mask.proto"
  "${DAWN_PROTOBUF_DIR}/src/google/protobuf/source_context.proto"
  "${DAWN_PROTOBUF_DIR}/src/google/protobuf/struct.proto"
  "${DAWN_PROTOBUF_DIR}/src/google/protobuf/timestamp.proto"
  "${DAWN_PROTOBUF_DIR}/src/google/protobuf/type.proto"
  "${DAWN_PROTOBUF_DIR}/src/google/protobuf/wrappers.proto"
)

add_library(libprotobuf_wkt STATIC ${WKT_PROTOS})
generate_protos(
  TARGET libprotobuf_wkt
  IMPORT_DIRS "${DAWN_PROTOBUF_DIR}/src"
)
target_link_libraries(libprotobuf_wkt PRIVATE libprotobuf)
target_include_directories(libprotobuf_wkt PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")

# Inject the generated WKTs into libprotobuf's consumers, except those involved in building protoc.
target_link_libraries(libprotobuf INTERFACE "$<IF:$<BOOL:$<TARGET_PROPERTY:SKIP_WKT>>,,libprotobuf_wkt>")
target_include_directories(libprotobuf PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")

# Mark protoc and libprotoc to skip WKT interface dependency to break cycles.
if(TARGET protoc)
  set_target_properties(protoc PROPERTIES SKIP_WKT TRUE)
endif()
if(TARGET libprotoc)
  set_target_properties(libprotoc PROPERTIES SKIP_WKT TRUE)
endif()

target_compile_definitions(libprotobuf PUBLIC "-DPROTOBUF_ENABLE_DEBUG_LOGGING_MAY_LEAK_PII=0")

target_compile_options(libprotobuf PUBLIC -fno-exceptions)
if (NOT DAWN_ENABLE_RTTI)
  target_compile_options(libprotobuf PUBLIC -fno-rtti)
endif()

# Allowing usage of enable_if() and nullability extensions in abseil and avoid shadowing errors
if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") OR
    ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang"))
  target_compile_options(libprotobuf PUBLIC
          -Wno-gcc-compat
          -Wno-unreachable-code-break
          -Wno-nullability-extension
          -Wno-shadow)
endif()
