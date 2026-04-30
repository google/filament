# Copyright 2021 The Draco Authors
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License. You may obtain a copy of
# the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License.

if(DRACO_CMAKE_DRACO_EMSCRIPTEN_CMAKE_)
  return()
endif() # DRACO_CMAKE_DRACO_EMSCRIPTEN_CMAKE_

# Checks environment for Emscripten prerequisites.
macro(draco_check_emscripten_environment)
  if(NOT PYTHONINTERP_FOUND)
    message(
      FATAL_ERROR
        "Python required for Emscripten builds, but cmake cannot find it.")
  endif()
  if(NOT EXISTS "$ENV{EMSCRIPTEN}")
    message(
      FATAL_ERROR
        "The EMSCRIPTEN environment variable must be set. See README.md.")
  endif()
endmacro()

# Obtains the required Emscripten flags for Draco targets.
macro(draco_get_required_emscripten_flags)
  set(em_FLAG_LIST_VAR_COMPILER)
  set(em_FLAG_LIST_VAR_LINKER)
  set(em_flags)
  set(em_single_arg_opts FLAG_LIST_VAR_COMPILER FLAG_LIST_VAR_LINKER)
  set(em_multi_arg_opts)
  cmake_parse_arguments(em "${em_flags}" "${em_single_arg_opts}"
                        "${em_multi_arg_opts}" ${ARGN})
  if(NOT em_FLAG_LIST_VAR_COMPILER)
    message(
      FATAL
      "draco_get_required_emscripten_flags: FLAG_LIST_VAR_COMPILER required")
  endif()

  if(NOT em_FLAG_LIST_VAR_LINKER)
    message(
      FATAL
      "draco_get_required_emscripten_flags: FLAG_LIST_VAR_LINKER required")
  endif()

  if(DRACO_JS_GLUE)
    unset(required_flags)
    # TODO(tomfinegan): Revisit splitting of compile/link flags for Emscripten,
    # and drop -Wno-unused-command-line-argument. Emscripten complains about
    # what are supposedly link-only flags sent with compile commands, but then
    # proceeds to produce broken code if the warnings are heeded.
    list(APPEND ${em_FLAG_LIST_VAR_COMPILER}
                "-Wno-unused-command-line-argument")

    list(APPEND ${em_FLAG_LIST_VAR_COMPILER} "-Wno-almost-asm")
    list(APPEND ${em_FLAG_LIST_VAR_COMPILER} "--memory-init-file" "0")
    list(APPEND ${em_FLAG_LIST_VAR_COMPILER} "-fno-omit-frame-pointer")

    # According to Emscripten the following flags are linker only, but sending
    # these flags (en masse) to only the linker results in a broken Emscripten
    # build with an empty DracoDecoderModule.
    list(APPEND ${em_FLAG_LIST_VAR_COMPILER} "-sALLOW_MEMORY_GROWTH=1")
    list(APPEND ${em_FLAG_LIST_VAR_COMPILER} "-sMODULARIZE=1")
    list(APPEND ${em_FLAG_LIST_VAR_COMPILER} "-sFILESYSTEM=0")
    list(APPEND ${em_FLAG_LIST_VAR_COMPILER}
                "-sEXPORTED_FUNCTIONS=[\"_free\",\"_malloc\"]")
    list(APPEND ${em_FLAG_LIST_VAR_COMPILER} "-sPRECISE_F32=1")
    list(APPEND ${em_FLAG_LIST_VAR_COMPILER} "-sNODEJS_CATCH_EXIT=0")
    list(APPEND ${em_FLAG_LIST_VAR_COMPILER} "-sNODEJS_CATCH_REJECTION=0")

    if(DRACO_FAST)
      list(APPEND ${em_FLAG_LIST_VAR_COMPILER} "--llvm-lto" "1")
    endif()

    # The WASM flag is reported as linker only.
    if(DRACO_WASM)
      list(APPEND ${em_FLAG_LIST_VAR_COMPILER} "-sWASM=1")
    else()
      list(APPEND ${em_FLAG_LIST_VAR_COMPILER} "-sWASM=0")
    endif()

    # The LEGACY_VM_SUPPORT flag is reported as linker only.
    if(DRACO_IE_COMPATIBLE)
      list(APPEND ${em_FLAG_LIST_VAR_COMPILER} "-sLEGACY_VM_SUPPORT=1")
    endif()
  endif()
endmacro()

# Macro for generating C++ glue code from IDL for Emscripten targets. Executes
# python to generate the C++ binding, and establishes dendency: $OUTPUT_PATH.cpp
# on $INPUT_IDL.
macro(draco_generate_emscripten_glue)
  set(glue_flags)
  set(glue_single_arg_opts INPUT_IDL OUTPUT_PATH)
  set(glue_multi_arg_opts)
  cmake_parse_arguments(glue "${glue_flags}" "${glue_single_arg_opts}"
                        "${glue_multi_arg_opts}" ${ARGN})

  if(DRACO_VERBOSE GREATER 1)
    message(
      "--------- draco_generate_emscripten_glue -----------\n"
      "glue_INPUT_IDL=${glue_INPUT_IDL}\n"
      "glue_OUTPUT_PATH=${glue_OUTPUT_PATH}\n"
      "----------------------------------------------------\n")
  endif()

  if(NOT glue_INPUT_IDL OR NOT glue_OUTPUT_PATH)
    message(
      FATAL_ERROR
        "draco_generate_emscripten_glue: INPUT_IDL and OUTPUT_PATH required.")
  endif()

  # Generate the glue source.
  execute_process(
    COMMAND ${PYTHON_EXECUTABLE} $ENV{EMSCRIPTEN}/tools/webidl_binder.py
            ${glue_INPUT_IDL} ${glue_OUTPUT_PATH})
  if(NOT EXISTS "${glue_OUTPUT_PATH}.cpp")
    message(FATAL_ERROR "JS glue generation failed for ${glue_INPUT_IDL}.")
  endif()

  # Create a dependency so that it regenerated on edits.
  add_custom_command(
    OUTPUT "${glue_OUTPUT_PATH}.cpp"
    COMMAND ${PYTHON_EXECUTABLE} $ENV{EMSCRIPTEN}/tools/webidl_binder.py
            ${glue_INPUT_IDL} ${glue_OUTPUT_PATH}
    DEPENDS ${draco_js_dec_idl}
    COMMENT "Generating ${glue_OUTPUT_PATH}.cpp."
    WORKING_DIRECTORY ${draco_build}
    VERBATIM)
endmacro()

# Wrapper for draco_add_executable() that handles the extra work necessary for
# emscripten targets when generating JS glue:
#
# ~~~
# - Set source level dependency on the C++ binding.
# - Pre/Post link emscripten magic.
#
# Required args:
#   - GLUE_PATH: Base path for glue file. Used to generate .cpp and .js files.
#   - PRE_LINK_JS_SOURCES: em_link_pre_js() source files.
#   - POST_LINK_JS_SOURCES: em_link_post_js() source files.
# Optional args:
#   - FEATURES:
# ~~~
macro(draco_add_emscripten_executable)
  unset(emexe_NAME)
  unset(emexe_FEATURES)
  unset(emexe_SOURCES)
  unset(emexe_DEFINES)
  unset(emexe_INCLUDES)
  unset(emexe_LINK_FLAGS)
  set(optional_args)
  set(single_value_args NAME GLUE_PATH)
  set(multi_value_args
      SOURCES
      DEFINES
      FEATURES
      INCLUDES
      LINK_FLAGS
      PRE_LINK_JS_SOURCES
      POST_LINK_JS_SOURCES)

  cmake_parse_arguments(emexe "${optional_args}" "${single_value_args}"
                        "${multi_value_args}" ${ARGN})

  if(NOT
     (emexe_GLUE_PATH
      AND emexe_POST_LINK_JS_SOURCES
      AND emexe_PRE_LINK_JS_SOURCES))
    message(FATAL
            "draco_add_emscripten_executable: GLUE_PATH PRE_LINK_JS_SOURCES "
            "POST_LINK_JS_SOURCES args required.")
  endif()

  if(DRACO_VERBOSE GREATER 1)
    message(
      "--------- draco_add_emscripten_executable ---------\n"
      "emexe_NAME=${emexe_NAME}\n"
      "emexe_SOURCES=${emexe_SOURCES}\n"
      "emexe_DEFINES=${emexe_DEFINES}\n"
      "emexe_INCLUDES=${emexe_INCLUDES}\n"
      "emexe_LINK_FLAGS=${emexe_LINK_FLAGS}\n"
      "emexe_GLUE_PATH=${emexe_GLUE_PATH}\n"
      "emexe_FEATURES=${emexe_FEATURES}\n"
      "emexe_PRE_LINK_JS_SOURCES=${emexe_PRE_LINK_JS_SOURCES}\n"
      "emexe_POST_LINK_JS_SOURCES=${emexe_POST_LINK_JS_SOURCES}\n"
      "----------------------------------------------------\n")
  endif()

  # The Emscripten linker needs the C++ flags in addition to whatever has been
  # passed in with the target.
  list(APPEND emexe_LINK_FLAGS ${DRACO_CXX_FLAGS})

  if(DRACO_GLTF_BITSTREAM)
    # Add "_gltf" suffix to target output name.
    draco_add_executable(
      NAME ${emexe_NAME}
      OUTPUT_NAME ${emexe_NAME}_gltf
      SOURCES ${emexe_SOURCES}
      DEFINES ${emexe_DEFINES}
      INCLUDES ${emexe_INCLUDES}
      LINK_FLAGS ${emexe_LINK_FLAGS})
  else()
    draco_add_executable(
      NAME ${emexe_NAME}
      SOURCES ${emexe_SOURCES}
      DEFINES ${emexe_DEFINES}
      INCLUDES ${emexe_INCLUDES}
      LINK_FLAGS ${emexe_LINK_FLAGS})
  endif()

  foreach(feature ${emexe_FEATURES})
    draco_enable_feature(FEATURE ${feature} TARGETS ${emexe_NAME})
  endforeach()

  set_property(
    SOURCE ${emexe_SOURCES}
    APPEND
    PROPERTY OBJECT_DEPENDS "${emexe_GLUE_PATH}.cpp")
  em_link_pre_js(${emexe_NAME} ${emexe_PRE_LINK_JS_SOURCES})
  em_link_post_js(${emexe_NAME} "${emexe_GLUE_PATH}.js"
                  ${emexe_POST_LINK_JS_SOURCES})
endmacro()
