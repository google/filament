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

if(DRACO_CMAKE_DRACO_OPTIONS_CMAKE_)
  return()
endif() # DRACO_CMAKE_DRACO_OPTIONS_CMAKE_
set(DRACO_CMAKE_DRACO_OPTIONS_CMAKE_)

set(draco_features_file_name "${draco_build}/draco/draco_features.h")
set(draco_features_list)

# Simple wrapper for CMake's builtin option command that tracks draco's build
# options in the list variable $draco_options.
macro(draco_option)
  unset(option_NAME)
  unset(option_HELPSTRING)
  unset(option_VALUE)
  unset(optional_args)
  unset(multi_value_args)
  set(single_value_args NAME HELPSTRING VALUE)
  cmake_parse_arguments(option "${optional_args}" "${single_value_args}"
                        "${multi_value_args}" ${ARGN})

  if(NOT
     (option_NAME
      AND option_HELPSTRING
      AND DEFINED option_VALUE))
    message(FATAL_ERROR "draco_option: NAME HELPSTRING and VALUE required.")
  endif()

  option(${option_NAME} ${option_HELPSTRING} ${option_VALUE})

  if(DRACO_VERBOSE GREATER 2)
    message(
      "--------- draco_option ---------\n"
      "option_NAME=${option_NAME}\n"
      "option_HELPSTRING=${option_HELPSTRING}\n"
      "option_VALUE=${option_VALUE}\n"
      "------------------------------------------\n")
  endif()

  list(APPEND draco_options ${option_NAME})
  list(REMOVE_DUPLICATES draco_options)
endmacro()

# Dumps the $draco_options list via CMake message command.
macro(draco_dump_options)
  foreach(option_name ${draco_options})
    message("${option_name}: ${${option_name}}")
  endforeach()
endmacro()

# Set default options.
macro(draco_set_default_options)
  draco_option(
    NAME DRACO_FAST
    HELPSTRING "Try to build faster libs."
    VALUE OFF)
  draco_option(
    NAME DRACO_JS_GLUE
    HELPSTRING "Enable JS Glue and JS targets when using Emscripten."
    VALUE ON)
  draco_option(
    NAME DRACO_IE_COMPATIBLE
    HELPSTRING "Enable support for older IE builds when using Emscripten."
    VALUE OFF)
  draco_option(
    NAME DRACO_MESH_COMPRESSION
    HELPSTRING "Enable mesh compression."
    VALUE ON)
  draco_option(
    NAME DRACO_POINT_CLOUD_COMPRESSION
    HELPSTRING "Enable point cloud compression."
    VALUE ON)
  draco_option(
    NAME DRACO_PREDICTIVE_EDGEBREAKER
    HELPSTRING "Enable predictive edgebreaker."
    VALUE ON)
  draco_option(
    NAME DRACO_STANDARD_EDGEBREAKER
    HELPSTRING "Enable stand edgebreaker."
    VALUE ON)
  draco_option(
    NAME DRACO_BACKWARDS_COMPATIBILITY
    HELPSTRING "Enable backwards compatibility."
    VALUE ON)
  draco_option(
    NAME DRACO_DECODER_ATTRIBUTE_DEDUPLICATION
    HELPSTRING "Enable attribute deduping."
    VALUE OFF)
  draco_option(
    NAME DRACO_TESTS
    HELPSTRING "Enables tests."
    VALUE OFF)
  draco_option(
    NAME DRACO_WASM
    HELPSTRING "Enables WASM support."
    VALUE OFF)
  draco_option(
    NAME DRACO_UNITY_PLUGIN
    HELPSTRING "Build plugin library for Unity."
    VALUE OFF)
  draco_option(
    NAME DRACO_ANIMATION_ENCODING
    HELPSTRING "Enable animation."
    VALUE OFF)
  draco_option(
    NAME DRACO_GLTF_BITSTREAM
    HELPSTRING "Draco GLTF extension bitstream specified features only."
    VALUE OFF)
  draco_option(
    NAME DRACO_MAYA_PLUGIN
    HELPSTRING "Build plugin library for Maya."
    VALUE OFF)
  draco_option(
    NAME DRACO_TRANSCODER_SUPPORTED
    HELPSTRING "Enable the Draco transcoder."
    VALUE OFF)
  draco_option(
    NAME DRACO_DEBUG_COMPILER_WARNINGS
    HELPSTRING "Turn on more warnings."
    VALUE OFF)
  draco_option(
    NAME DRACO_INSTALL
    HELPSTRING "Enable installation."
    VALUE ON)
  draco_check_deprecated_options()
endmacro()

# Warns when a deprecated option is used and sets the option that replaced it.
macro(draco_handle_deprecated_option)
  unset(option_OLDNAME)
  unset(option_NEWNAME)
  unset(optional_args)
  unset(multi_value_args)
  set(single_value_args OLDNAME NEWNAME)
  cmake_parse_arguments(option "${optional_args}" "${single_value_args}"
                        "${multi_value_args}" ${ARGN})

  if("${${option_OLDNAME}}")
    message(WARNING "${option_OLDNAME} is deprecated. Use ${option_NEWNAME}.")
    set(${option_NEWNAME} ${${option_OLDNAME}})
  endif()
endmacro()

# Checks for use of deprecated options.
macro(draco_check_deprecated_options)
  draco_handle_deprecated_option(OLDNAME ENABLE_EXTRA_SPEED NEWNAME DRACO_FAST)
  draco_handle_deprecated_option(OLDNAME ENABLE_JS_GLUE NEWNAME DRACO_JS_GLUE)
  draco_handle_deprecated_option(OLDNAME ENABLE_MESH_COMPRESSION NEWNAME
                                 DRACO_MESH_COMPRESSION)
  draco_handle_deprecated_option(OLDNAME ENABLE_POINT_CLOUD_COMPRESSION NEWNAME
                                 DRACO_POINT_CLOUD_COMPRESSION)
  draco_handle_deprecated_option(OLDNAME ENABLE_PREDICTIVE_EDGEBREAKER NEWNAME
                                 DRACO_PREDICTIVE_EDGEBREAKER)
  draco_handle_deprecated_option(OLDNAME ENABLE_STANDARD_EDGEBREAKER NEWNAME
                                 DRACO_STANDARD_EDGEBREAKER)
  draco_handle_deprecated_option(OLDNAME ENABLE_BACKWARDS_COMPATIBILITY NEWNAME
                                 DRACO_BACKWARDS_COMPATIBILITY)
  draco_handle_deprecated_option(OLDNAME ENABLE_DECODER_ATTRIBUTE_DEDUPLICATION
                                 NEWNAME DRACO_DECODER_ATTRIBUTE_DEDUPLICATION)
  draco_handle_deprecated_option(OLDNAME ENABLE_TESTS NEWNAME DRACO_TESTS)
  draco_handle_deprecated_option(OLDNAME ENABLE_WASM NEWNAME DRACO_WASM)
  draco_handle_deprecated_option(OLDNAME BUILD_UNITY_PLUGIN NEWNAME
                                 DRACO_UNITY_PLUGIN)
  draco_handle_deprecated_option(OLDNAME BUILD_ANIMATION_ENCODING NEWNAME
                                 DRACO_ANIMATION_ENCODING)
  draco_handle_deprecated_option(OLDNAME BUILD_FOR_GLTF NEWNAME DRACO_GLTF)
  draco_handle_deprecated_option(OLDNAME BUILD_MAYA_PLUGIN NEWNAME
                                 DRACO_MAYA_PLUGIN)
  draco_handle_deprecated_option(OLDNAME BUILD_USD_PLUGIN NEWNAME
                                 BUILD_SHARED_LIBS)
  draco_handle_deprecated_option(OLDNAME DRACO_GLTF NEWNAME
                                 DRACO_GLTF_BITSTREAM)

endmacro()

# Macro for setting Draco features based on user configuration. Features enabled
# by this macro are Draco global.
macro(draco_set_optional_features)
  if(DRACO_GLTF_BITSTREAM)
    # Enable only the features included in the Draco GLTF bitstream spec.
    draco_enable_feature(FEATURE "DRACO_MESH_COMPRESSION_SUPPORTED")
    draco_enable_feature(FEATURE "DRACO_NORMAL_ENCODING_SUPPORTED")
    draco_enable_feature(FEATURE "DRACO_STANDARD_EDGEBREAKER_SUPPORTED")
  else()
    if(DRACO_POINT_CLOUD_COMPRESSION)
      draco_enable_feature(FEATURE "DRACO_POINT_CLOUD_COMPRESSION_SUPPORTED")
    endif()
    if(DRACO_MESH_COMPRESSION)
      draco_enable_feature(FEATURE "DRACO_MESH_COMPRESSION_SUPPORTED")
      draco_enable_feature(FEATURE "DRACO_NORMAL_ENCODING_SUPPORTED")

      if(DRACO_STANDARD_EDGEBREAKER)
        draco_enable_feature(FEATURE "DRACO_STANDARD_EDGEBREAKER_SUPPORTED")
      endif()
      if(DRACO_PREDICTIVE_EDGEBREAKER)
        draco_enable_feature(FEATURE "DRACO_PREDICTIVE_EDGEBREAKER_SUPPORTED")
      endif()
    endif()

    if(DRACO_BACKWARDS_COMPATIBILITY)
      draco_enable_feature(FEATURE "DRACO_BACKWARDS_COMPATIBILITY_SUPPORTED")
    endif()


    if(NOT EMSCRIPTEN)
      # For now, enable deduplication for both encoder and decoder.
      # TODO(ostava): Support for disabling attribute deduplication for the C++
      # decoder is planned in future releases.
      draco_enable_feature(FEATURE
                           DRACO_ATTRIBUTE_INDICES_DEDUPLICATION_SUPPORTED)
      draco_enable_feature(FEATURE
                           DRACO_ATTRIBUTE_VALUES_DEDUPLICATION_SUPPORTED)
    endif()
  endif()

  if(DRACO_UNITY_PLUGIN)
    draco_enable_feature(FEATURE "DRACO_UNITY_PLUGIN")
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
  endif()

  if(DRACO_MAYA_PLUGIN)
    draco_enable_feature(FEATURE "DRACO_MAYA_PLUGIN")
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
  endif()

  if(DRACO_TRANSCODER_SUPPORTED)
    draco_enable_feature(FEATURE "DRACO_TRANSCODER_SUPPORTED")
  endif()


endmacro()

# Macro that handles tracking of Draco preprocessor symbols for the purpose of
# producing draco_features.h.
#
# ~~~
# draco_enable_feature(FEATURE <feature_name> [TARGETS <target_name>])
# ~~~
#
# FEATURE  is required. It should be a Draco preprocessor symbol. TARGETS is
# optional. It can be one or more draco targets.
#
# When the TARGETS argument is not present the preproc symbol is added to
# draco_features.h. When it is draco_features.h is unchanged, and
# target_compile_options() is called for each target specified.
macro(draco_enable_feature)
  set(def_flags)
  set(def_single_arg_opts FEATURE)
  set(def_multi_arg_opts TARGETS)
  cmake_parse_arguments(DEF "${def_flags}" "${def_single_arg_opts}"
                        "${def_multi_arg_opts}" ${ARGN})
  if("${DEF_FEATURE}" STREQUAL "")
    message(FATAL_ERROR "Empty FEATURE passed to draco_enable_feature().")
  endif()

  # Do nothing/return early if $DEF_FEATURE is already in the list.
  list(FIND draco_features_list ${DEF_FEATURE} df_index)
  if(NOT df_index EQUAL -1)
    return()
  endif()

  list(LENGTH DEF_TARGETS df_targets_list_length)
  if(${df_targets_list_length} EQUAL 0)
    list(APPEND draco_features_list ${DEF_FEATURE})
  else()
    foreach(target ${DEF_TARGETS})
      target_compile_definitions(${target} PRIVATE ${DEF_FEATURE})
    endforeach()
  endif()
endmacro()

# Function for generating draco_features.h.
function(draco_generate_features_h)
  file(WRITE "${draco_features_file_name}.new"
       "// GENERATED FILE -- DO NOT EDIT\n\n" "#ifndef DRACO_FEATURES_H_\n"
       "#define DRACO_FEATURES_H_\n\n")

  foreach(feature ${draco_features_list})
    file(APPEND "${draco_features_file_name}.new" "#define ${feature}\n")
  endforeach()

  if(MSVC)
    if(NOT DRACO_DEBUG_COMPILER_WARNINGS)
      file(APPEND "${draco_features_file_name}.new"
           "// Enable DRACO_DEBUG_COMPILER_WARNINGS at CMake generation \n"
           "// time to remove these pragmas.\n")

      # warning C4018: '<operator>': signed/unsigned mismatch.
      file(APPEND "${draco_features_file_name}.new"
           "#pragma warning(disable:4018)\n")

      # warning C4146: unary minus operator applied to unsigned type, result
      # still unsigned
      file(APPEND "${draco_features_file_name}.new"
           "#pragma warning(disable:4146)\n")

      # warning C4244: 'return': conversion from '<type>' to '<type>', possible
      # loss of data.
      file(APPEND "${draco_features_file_name}.new"
           "#pragma warning(disable:4244)\n")

      # warning C4267: 'initializing' conversion from '<type>' to '<type>',
      # possible loss of data.
      file(APPEND "${draco_features_file_name}.new"
           "#pragma warning(disable:4267)\n")

      # warning C4305: 'context' : truncation from 'type1' to 'type2'.
      file(APPEND "${draco_features_file_name}.new"
           "#pragma warning(disable:4305)\n")

      # warning C4661: 'identifier' : no suitable definition provided for
      # explicit template instantiation request.
      file(APPEND "${draco_features_file_name}.new"
           "#pragma warning(disable:4661)\n")

      # warning C4800: Implicit conversion from 'type' to bool. Possible
      # information loss.
      # Also, in older MSVC releases:
      # warning C4800: 'type' : forcing value to bool 'true' or 'false'
      # (performance warning).
      file(APPEND "${draco_features_file_name}.new"
           "#pragma warning(disable:4800)\n")

      # warning C4804: '<operator>': unsafe use of type '<type>' in operation.
      file(APPEND "${draco_features_file_name}.new"
           "#pragma warning(disable:4804)\n")
    endif()
  endif()

  file(APPEND "${draco_features_file_name}.new"
       "\n#endif  // DRACO_FEATURES_H_\n")

  # Will replace ${draco_features_file_name} only if the file content has
  # changed. This prevents forced Draco rebuilds after CMake runs.
  configure_file("${draco_features_file_name}.new"
                 "${draco_features_file_name}")
  file(REMOVE "${draco_features_file_name}.new")
endfunction()

# Sets default options for the build and processes user controlled options to
# compute enabled features.
macro(draco_setup_options)
  draco_set_default_options()
  draco_set_optional_features()
endmacro()
