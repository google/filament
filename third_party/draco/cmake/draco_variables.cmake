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

if(DRACO_CMAKE_DRACO_VARIABLES_CMAKE_)
  return()
endif() # DRACO_CMAKE_DRACO_VARIABLES_CMAKE_
set(DRACO_CMAKE_DRACO_VARIABLES_CMAKE_ 1)

# Halts generation when $variable_name does not refer to a directory that
# exists.
macro(draco_variable_must_be_directory variable_name)
  if("${variable_name}" STREQUAL "")
    message(
      FATAL_ERROR
        "Empty variable_name passed to draco_variable_must_be_directory.")
  endif()

  if("${${variable_name}}" STREQUAL "")
    message(
      FATAL_ERROR "Empty variable ${variable_name} is required to build draco.")
  endif()

  if(NOT IS_DIRECTORY "${${variable_name}}")
    message(
      FATAL_ERROR
        "${variable_name}, which is ${${variable_name}}, does not refer to a\n"
        "directory.")
  endif()
endmacro()

# Adds $var_name to the tracked variables list.
macro(draco_track_configuration_variable var_name)
  if(DRACO_VERBOSE GREATER 2)
    message("---- draco_track_configuration_variable ----\n"
            "var_name=${var_name}\n"
            "----------------------------------------------\n")
  endif()

  list(APPEND draco_configuration_variables ${var_name})
  list(REMOVE_DUPLICATES draco_configuration_variables)
endmacro()

# Logs current C++ and executable linker flags via the CMake message command.
macro(draco_dump_cmake_flag_variables)
  unset(flag_variables)
  list(APPEND flag_variables "CMAKE_CXX_FLAGS_INIT" "CMAKE_CXX_FLAGS"
              "CMAKE_EXE_LINKER_FLAGS_INIT" "CMAKE_EXE_LINKER_FLAGS")
  if(CMAKE_BUILD_TYPE)
    list(
      APPEND flag_variables
             "CMAKE_BUILD_TYPE"
             "CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}_INIT"
             "CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}"
             "CMAKE_EXE_LINKER_FLAGS_${CMAKE_BUILD_TYPE}_INIT"
             "CMAKE_EXE_LINKER_FLAGS_${CMAKE_BUILD_TYPE}")
  endif()
  foreach(flag_variable ${flag_variables})
    message("${flag_variable}:${${flag_variable}}")
  endforeach()
endmacro()

# Dumps the variables tracked in $draco_configuration_variables via the CMake
# message command.
macro(draco_dump_tracked_configuration_variables)
  foreach(config_variable ${draco_configuration_variables})
    message("${config_variable}:${${config_variable}}")
  endforeach()
endmacro()
