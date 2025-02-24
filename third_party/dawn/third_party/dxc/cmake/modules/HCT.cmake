option(HLSL_COPY_GENERATED_SOURCES "Copy generated sources if different" Off)
option(HLSL_DISABLE_SOURCE_GENERATION "Disable generation of in-tree sources" Off)
mark_as_advanced(HLSL_DISABLE_SOURCE_GENERATION)

add_custom_target(HCTGen)

find_program(CLANG_FORMAT_EXE NAMES clang-format)

if (NOT CLANG_FORMAT_EXE)
  message(WARNING "Clang-format is not available. Generating included sources is not supported.")
  if (HLSL_COPY_GENERATED_SOURCES)
    message(FATAL_ERROR "Generating sources requires clang-format")
  endif ()
endif ()

if (WIN32 AND NOT DEFINED HLSL_AUTOCRLF)
  find_program(git_executable NAMES git git.exe git.cmd)
  execute_process(COMMAND ${git_executable} config --get core.autocrlf
                  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                  TIMEOUT 5
                  RESULT_VARIABLE result
                  OUTPUT_VARIABLE output
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
  if( result EQUAL 0 )
    # This is a little counterintuitive... Because the repo's gitattributes set
    # text=auto, autocrlf behavior will be enabled for autocrlf true or false.
    # For reasons unknown to me, autocrlf=input overrides the gitattributes, so
    # that is the case we need special handling for.
    set(val On)
    if (output STREQUAL "input")
      set(val Off)
    endif()
    set(HLSL_AUTOCRLF ${val} CACHE BOOL "Is core.autocrlf enabled in this clone")
    message(STATUS "Git checkout autocrlf: ${HLSL_AUTOCRLF}")
  endif()
endif()

function(add_hlsl_hctgen mode)
  cmake_parse_arguments(ARG
    "BUILD_DIR;CODE_TAG"
    "OUTPUT"
    ""
    ${ARGN})

  if (NOT ARG_OUTPUT)
    message(FATAL_ERROR "add_hlsl_hctgen requires OUTPUT argument")
  endif()

  if (HLSL_DISABLE_SOURCE_GENERATION AND NOT ARG_BUILD_DIR)
    return()
  endif()
 
  set(temp_output ${CMAKE_CURRENT_BINARY_DIR}/tmp/${ARG_OUTPUT})
  set(full_output ${CMAKE_CURRENT_SOURCE_DIR}/${ARG_OUTPUT})
  if (ARG_BUILD_DIR)
    set(full_output ${CMAKE_CURRENT_BINARY_DIR}/${ARG_OUTPUT})
  endif()
  set(hctgen ${LLVM_SOURCE_DIR}/utils/hct/hctgen.py)
  set(hctdb ${LLVM_SOURCE_DIR}/utils/hct/hctdb.py)
  set(hctdb_helper ${LLVM_SOURCE_DIR}/utils/hct/hctdb_instrhelp.py)
  set(output ${full_output})
  set(hct_dependencies ${LLVM_SOURCE_DIR}/utils/hct/gen_intrin_main.txt
                       ${hctgen}
                       ${hctdb}
                       ${hctdb_helper})

  get_filename_component(output_extension ${full_output} LAST_EXT)

  if (CLANG_FORMAT_EXE AND output_extension MATCHES "\.h|\.cpp|\.inl")
    set(format_cmd COMMAND ${CLANG_FORMAT_EXE} -i ${temp_output})
  endif ()

  set(copy_sources Off)
  if(ARG_BUILD_DIR OR HLSL_COPY_GENERATED_SOURCES)
    set(copy_sources On)
  endif()

  if(ARG_CODE_TAG)
    set(input_flag --input ${full_output})
    if (UNIX)
      execute_process(COMMAND file ${full_output} OUTPUT_VARIABLE output)
      if (output MATCHES ".*, with CRLF line terminators")
        set(force_lf "--force-crlf")
      endif()
    endif()

    list(APPEND hct_dependencies ${full_output})
    if (HLSL_COPY_GENERATED_SOURCES)
      # The generation command both depends on and produces the final output if
      # source copying is enabled for CODE_TAG sources. That means we need to
      # create an extra temporary to key the copy step on.
      set(output ${temp_output}.2)
      set(second_copy COMMAND ${CMAKE_COMMAND} -E copy_if_different ${temp_output} ${temp_output}.2)
    endif()
  endif()

  # If we're not copying the sources, set the output for the target as the temp
  # file, and define the verification command
  if(NOT copy_sources)
    set(output ${temp_output})
    if (CLANG_FORMAT_EXE) # Only verify sources if clang-format is available.
      set(verification COMMAND ${CMAKE_COMMAND} -E compare_files ${temp_output} ${full_output})
    endif()
  endif()
  if(WIN32 AND NOT HLSL_AUTOCRLF)
    set(force_lf "--force-lf")
  endif()

  add_custom_command(OUTPUT ${temp_output}
                     COMMAND ${Python3_EXECUTABLE}
                             ${hctgen} ${force_lf}
                             ${mode} --output ${temp_output} ${input_flag}
                     ${format_cmd}
                     COMMENT "Building ${ARG_OUTPUT}..."
                     DEPENDS ${hct_dependencies}
                     )
  if(copy_sources)
    add_custom_command(OUTPUT ${output}
                      COMMAND ${CMAKE_COMMAND} -E copy_if_different
                              ${temp_output} ${full_output}
                      ${second_copy}
                      DEPENDS ${temp_output}
                      COMMENT "Updating ${ARG_OUTPUT}..."
                      )
  endif()

  add_custom_command(OUTPUT ${temp_output}.stamp
                     COMMAND ${verification}
                     COMMAND ${CMAKE_COMMAND} -E touch ${temp_output}.stamp
                     DEPENDS ${output}
                     COMMENT "Verifying clang-format results...")

  add_custom_target(${mode}
                    DEPENDS ${temp_output}.stamp)

  add_dependencies(HCTGen ${mode})
endfunction()
