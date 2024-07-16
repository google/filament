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
  set(em_FLAG_LIST_VAR)
  set(em_flags)
  set(em_single_arg_opts FLAG_LIST_VAR)
  set(em_multi_arg_opts)
  cmake_parse_arguments(em "${em_flags}" "${em_single_arg_opts}"
                        "${em_multi_arg_opts}" ${ARGN})
  if(NOT em_FLAG_LIST_VAR)
    message(FATAL "draco_get_required_emscripten_flags: FLAG_LIST_VAR required")
  endif()

  if(DRACO_JS_GLUE)
    unset(required_flags)
    list(APPEND ${em_FLAG_LIST_VAR} "-sALLOW_MEMORY_GROWTH=1")
    list(APPEND ${em_FLAG_LIST_VAR} "-Wno-almost-asm")
    list(APPEND ${em_FLAG_LIST_VAR} "--memory-init-file" "0")
    list(APPEND ${em_FLAG_LIST_VAR} "-fno-omit-frame-pointer")
    list(APPEND ${em_FLAG_LIST_VAR} "-sMODULARIZE=1")
    list(APPEND ${em_FLAG_LIST_VAR} "-sNO_FILESYSTEM=1")
    list(APPEND ${em_FLAG_LIST_VAR} "-sEXPORTED_RUNTIME_METHODS=[]")
    list(APPEND ${em_FLAG_LIST_VAR} "-sPRECISE_F32=1")
    list(APPEND ${em_FLAG_LIST_VAR} "-sNODEJS_CATCH_EXIT=0")
    list(APPEND ${em_FLAG_LIST_VAR} "-sNODEJS_CATCH_REJECTION=0")

    if(DRACO_FAST)
      list(APPEND ${em_FLAG_LIST_VAR} "--llvm-lto" "1")
    endif()
    if(DRACO_WASM)
      list(APPEND ${em_FLAG_LIST_VAR} "-sWASM=1")
    else()
      list(APPEND ${em_FLAG_LIST_VAR} "-sWASM=0")
    endif()
    if(DRACO_IE_COMPATIBLE)
      list(APPEND ${em_FLAG_LIST_VAR} "-sLEGACY_VM_SUPPORT=1")
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
    message("--------- draco_generate_emscripten_glue -----------\n"
            "glue_INPUT_IDL=${glue_INPUT_IDL}\n"
            "glue_OUTPUT_PATH=${glue_OUTPUT_PATH}\n" ]
            "----------------------------------------------------\n")
  endif()

  if(NOT glue_INPUT_IDL OR NOT glue_OUTPUT_PATH)
    message(
      FATAL_ERROR
        "draco_generate_emscripten_glue: INPUT_IDL and OUTPUT_PATH required.")
  endif()

  # Generate the glue source.
  execute_process(COMMAND ${PYTHON_EXECUTABLE}
                          $ENV{EMSCRIPTEN}/tools/webidl_binder.py
                          ${glue_INPUT_IDL} ${glue_OUTPUT_PATH})
  if(NOT EXISTS "${glue_OUTPUT_PATH}.cpp")
    message(FATAL_ERROR "JS glue generation failed for ${glue_INPUT_IDL}.")
  endif()

  # Create a dependency so that it regenerated on edits.
  add_custom_command(OUTPUT "${glue_OUTPUT_PATH}.cpp"
                     COMMAND ${PYTHON_EXECUTABLE}
                             $ENV{EMSCRIPTEN}/tools/webidl_binder.py
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
  set(multi_value_args SOURCES DEFINES FEATURES INCLUDES LINK_FLAGS
                       PRE_LINK_JS_SOURCES POST_LINK_JS_SOURCES)

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
    message("--------- draco_add_emscripten_executable ---------\n"
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

  if(DRACO_GLTF)
    draco_add_executable(NAME
                         ${emexe_NAME}
                         OUTPUT_NAME
                         ${emexe_NAME}_gltf
                         SOURCES
                         ${emexe_SOURCES}
                         DEFINES
                         ${emexe_DEFINES}
                         INCLUDES
                         ${emexe_INCLUDES}
                         LINK_FLAGS
                         ${emexe_LINK_FLAGS})
  else()
    draco_add_executable(NAME ${emexe_NAME} SOURCES ${emexe_SOURCES} DEFINES
                         ${emexe_DEFINES} INCLUDES ${emexe_INCLUDES} LINK_FLAGS
                         ${emexe_LINK_FLAGS})
  endif()

  foreach(feature ${emexe_FEATURES})
    draco_enable_feature(FEATURE ${feature} TARGETS ${emexe_NAME})
  endforeach()

  set_property(SOURCE ${emexe_SOURCES}
               APPEND
               PROPERTY OBJECT_DEPENDS "${emexe_GLUE_PATH}.cpp")
  em_link_pre_js(${emexe_NAME} ${emexe_PRE_LINK_JS_SOURCES})
  em_link_post_js(${emexe_NAME} "${emexe_GLUE_PATH}.js"
                  ${emexe_POST_LINK_JS_SOURCES})
endmacro()
