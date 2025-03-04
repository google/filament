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

set(protobuf_INSTALL OFF CACHE BOOL "Install protobuf binaries and files" FORCE)
set(protobuf_BUILD_CONFORMANCE OFF CACHE BOOL "Build conformance tests" FORCE)
set(protobuf_BUILD_EXAMPLES OFF CACHE BOOL "Build examples" FORCE)
set(protobuf_BUILD_LIBPROTOC OFF CACHE BOOL "Build libprotoc" FORCE)
set(protobuf_BUILD_TESTS OFF CACHE BOOL "Controls whether protobuf tests are built" FORCE)
set(protobuf_MSVC_STATIC_RUNTIME OFF CACHE BOOL "Controls whether a protobuf static runtime is built" FORCE)

set(protobuf_BUILD_PROTOC_BINARIES ON CACHE BOOL "Build libprotoc and protoc compiler" FORCE)
set(protobuf_DISABLE_RTTI ON CACHE BOOL "Remove runtime type information in the binaries" FORCE)

add_subdirectory("${DAWN_PROTOBUF_DIR}/cmake")

target_compile_definitions(libprotobuf PUBLIC "-DGOOGLE_PROTOBUF_INTERNAL_DONATE_STEAL_INLINE=0")

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

    add_custom_command(
      OUTPUT ${GENERATED_SRCS}
      COMMAND protobuf::protoc
      ARGS ${ARGS_PROTOC_OPTIONS} --${ARGS_LANGUAGE}_out ${_plugin_options}:${ARGS_PROTOC_OUT_DIR} ${_plugin} ${PROTOBUF_INCLUDE_PATH} ${ABS_FILE}
      DEPENDS ${ABS_FILE} protobuf::protoc
      COMMENT ${COMMENT}
      VERBATIM)
  endforeach()

  set_source_files_properties(${ALL_GENERATED_SRCS} PROPERTIES GENERATED TRUE)
  if(ARGS_TARGET)
    target_sources(${ARGS_TARGET} PRIVATE ${ALL_GENERATED_SRCS})
  endif()
endfunction()
