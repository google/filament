if(DRACO_CMAKE_DRACO_HELPERS_CMAKE_)
  return()
endif() # DRACO_CMAKE_DRACO_HELPERS_CMAKE_
set(DRACO_CMAKE_DRACO_HELPERS_CMAKE_ 1)

# Kills build generation using message(FATAL_ERROR) and outputs all data passed
# to the console via use of $ARGN.
macro(draco_die)
  message(FATAL_ERROR ${ARGN})
endmacro()

# Converts semi-colon delimited list variable(s) to string. Output is written to
# variable supplied via the DEST parameter. Input is from an expanded variable
# referenced by SOURCE and/or variable(s) referenced by SOURCE_VARS.
macro(draco_set_and_stringify)
  set(optional_args)
  set(single_value_args DEST SOURCE_VAR)
  set(multi_value_args SOURCE SOURCE_VARS)
  cmake_parse_arguments(sas "${optional_args}" "${single_value_args}"
                        "${multi_value_args}" ${ARGN})

  if(NOT sas_DEST OR NOT (sas_SOURCE OR sas_SOURCE_VARS))
    draco_die("draco_set_and_stringify: DEST and at least one of SOURCE "
              "SOURCE_VARS required.")
  endif()

  unset(${sas_DEST})

  if(sas_SOURCE)
    # $sas_SOURCE is one or more expanded variables, just copy the values to
    # $sas_DEST.
    set(${sas_DEST} "${sas_SOURCE}")
  endif()

  if(sas_SOURCE_VARS)
    # $sas_SOURCE_VARS is one or more variable names. Each iteration expands a
    # variable and appends it to $sas_DEST.
    foreach(source_var ${sas_SOURCE_VARS})
      set(${sas_DEST} "${${sas_DEST}} ${${source_var}}")
    endforeach()

    # Because $sas_DEST can be empty when entering this scope leading whitespace
    # can be introduced to $sas_DEST on the first iteration of the above loop.
    # Remove it:
    string(STRIP "${${sas_DEST}}" ${sas_DEST})
  endif()

  # Lists in CMake are simply semicolon delimited strings, so stringification is
  # just a find and replace of the semicolon.
  string(REPLACE ";" " " ${sas_DEST} "${${sas_DEST}}")

  if(DRACO_VERBOSE GREATER 1)
    message("draco_set_and_stringify: ${sas_DEST}=${${sas_DEST}}")
  endif()
endmacro()

# Creates a dummy source file in $DRACO_GENERATED_SOURCES_DIRECTORY and adds it
# to the specified target. Optionally adds its path to a list variable.
#
# draco_create_dummy_source_file(<TARGET <target> BASENAME <basename of file>>
# [LISTVAR <list variable>])
macro(draco_create_dummy_source_file)
  set(optional_args)
  set(single_value_args TARGET BASENAME LISTVAR)
  set(multi_value_args)
  cmake_parse_arguments(cdsf "${optional_args}" "${single_value_args}"
                        "${multi_value_args}" ${ARGN})

  if(NOT cdsf_TARGET OR NOT cdsf_BASENAME)
    draco_die("draco_create_dummy_source_file: TARGET and BASENAME required.")
  endif()

  if(NOT DRACO_GENERATED_SOURCES_DIRECTORY)
    set(DRACO_GENERATED_SOURCES_DIRECTORY "${draco_build}/gen_src")
  endif()

  set(dummy_source_dir "${DRACO_GENERATED_SOURCES_DIRECTORY}")
  set(dummy_source_file
      "${dummy_source_dir}/draco_${cdsf_TARGET}_${cdsf_BASENAME}.cc")
  set(dummy_source_code
      "// Generated file. DO NOT EDIT!\n"
      "// C++ source file created for target ${cdsf_TARGET}.\n"
      "void draco_${cdsf_TARGET}_${cdsf_BASENAME}_dummy_function(void)\;\n"
      "void draco_${cdsf_TARGET}_${cdsf_BASENAME}_dummy_function(void) {}\n")
  file(WRITE "${dummy_source_file}" ${dummy_source_code})

  target_sources(${cdsf_TARGET} PRIVATE ${dummy_source_file})

  if(cdsf_LISTVAR)
    list(APPEND ${cdsf_LISTVAR} "${dummy_source_file}")
  endif()
endmacro()

# Loads the version string from $draco_source/draco/version.h and sets
# $DRACO_VERSION.
macro(draco_load_version_info)
  file(STRINGS "${draco_src_root}/core/draco_version.h" version_file_strings)
  foreach(str ${version_file_strings})
    if(str MATCHES "char kDracoVersion")
      string(FIND "${str}" "\"" open_quote_pos)
      string(FIND "${str}" ";" semicolon_pos)
      math(EXPR open_quote_pos "${open_quote_pos} + 1")
      math(EXPR close_quote_pos "${semicolon_pos} - 1")
      math(EXPR version_string_length "${close_quote_pos} - ${open_quote_pos}")
      string(SUBSTRING "${str}" ${open_quote_pos} ${version_string_length}
                       DRACO_VERSION)
      break()
    endif()
  endforeach()
endmacro()
