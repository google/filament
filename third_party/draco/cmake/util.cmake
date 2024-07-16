if(DRACO_CMAKE_UTIL_CMAKE_)
  return()
endif()
set(DRACO_CMAKE_UTIL_CMAKE_ 1)

# Creates dummy source file in $draco_build_dir named $basename.$extension and
# returns the full path to the dummy source file via the $out_file_path
# parameter.
function(create_dummy_source_file basename extension out_file_path)
  set(dummy_source_file "${draco_build_dir}/${basename}.${extension}")
  file(WRITE "${dummy_source_file}.new"
       "// Generated file. DO NOT EDIT!\n"
       "// ${target_name} needs a ${extension} file to force link language, \n"
       "// or to silence a harmless CMake warning: Ignore me.\n"
       "void ${target_name}_dummy_function(void) {}\n")

  # Will replace ${dummy_source_file} only if the file content has changed.
  # This prevents forced Draco rebuilds after CMake runs.
  configure_file("${dummy_source_file}.new" "${dummy_source_file}")
  file(REMOVE "${dummy_source_file}.new")

  set(${out_file_path} ${dummy_source_file} PARENT_SCOPE)
endfunction()

# Convenience function for adding a dummy source file to $target_name using
# $extension as the file extension. Wraps create_dummy_source_file().
function(add_dummy_source_file_to_target target_name extension)
  create_dummy_source_file("${target_name}" "${extension}" "dummy_source_file")
  target_sources(${target_name} PRIVATE ${dummy_source_file})
endfunction()

# Extracts the version number from $version_file and returns it to the user via
# $version_string_out_var. This is achieved by finding the first instance of the
# kDracoVersion variable and then removing everything but the string literal
# assigned to the variable. Quotes and semicolon are stripped from the returned
# string.
function(extract_version_string version_file version_string_out_var)
  file(STRINGS "${version_file}" draco_version REGEX "kDracoVersion")
  list(GET draco_version 0 draco_version)
  string(REPLACE "static const char kDracoVersion[] = " "" draco_version
                 "${draco_version}")
  string(REPLACE ";" "" draco_version "${draco_version}")
  string(REPLACE "\"" "" draco_version "${draco_version}")
  set("${version_string_out_var}" "${draco_version}" PARENT_SCOPE)
endfunction()

# Sets CMake compiler launcher to $launcher_name when $launcher_name is found in
# $PATH. Warns user about ignoring build flag $launcher_flag when $launcher_name
# is not found in $PATH.
function(set_compiler_launcher launcher_flag launcher_name)
  find_program(launcher_path "${launcher_name}")
  if(launcher_path)
    set(CMAKE_C_COMPILER_LAUNCHER "${launcher_path}" PARENT_SCOPE)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${launcher_path}" PARENT_SCOPE)
    message("--- Using ${launcher_name} as compiler launcher.")
  else()
    message(
      WARNING "--- Cannot find ${launcher_name}, ${launcher_flag} ignored.")
  endif()
endfunction()

# Terminates CMake execution when $var_name is unset in the environment. Sets
# CMake variable to the value of the environment variable when the variable is
# present in the environment.
macro(require_variable var_name)
  if("$ENV{${var_name}}" STREQUAL "")
    message(FATAL_ERROR "${var_name} must be set in environment.")
  endif()
  set_variable_if_unset(${var_name} "")
endmacro()

# Sets $var_name to $default_value if not already set.
macro(set_variable_if_unset var_name default_value)
  if(NOT "$ENV{${var_name}}" STREQUAL "")
    set(${var_name} $ENV{${var_name}})
  elseif(NOT ${var_name})
    set(${var_name} ${default_value})
  endif()
endmacro()
