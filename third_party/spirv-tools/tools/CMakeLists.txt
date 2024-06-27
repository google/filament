# Copyright (c) 2015-2016 The Khronos Group Inc.
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

if (NOT ${SPIRV_SKIP_EXECUTABLES})
 add_subdirectory(lesspipe)
endif()
add_subdirectory(emacs)

# Add a SPIR-V Tools command line tool. Signature:
#   add_spvtools_tool(
#     TARGET target_name
#     SRCS   src_file1.cpp src_file2.cpp
#     LIBS   lib_target1 lib_target2
#   )
function(add_spvtools_tool)
  set(one_value_args TARGET)
  set(multi_value_args SRCS LIBS)
  cmake_parse_arguments(
    ARG "" "${one_value_args}" "${multi_value_args}" ${ARGN})

  add_executable(${ARG_TARGET} ${ARG_SRCS})
  spvtools_default_compile_options(${ARG_TARGET})
  target_link_libraries(${ARG_TARGET} PRIVATE ${ARG_LIBS})
  target_include_directories(${ARG_TARGET} PRIVATE
    ${spirv-tools_SOURCE_DIR}
    ${spirv-tools_BINARY_DIR}
  )
  set_property(TARGET ${ARG_TARGET} PROPERTY FOLDER "SPIRV-Tools executables")
endfunction()

set(COMMON_TOOLS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/util/flags.cpp")

if (NOT ${SPIRV_SKIP_EXECUTABLES})
  add_spvtools_tool(TARGET spirv-diff SRCS ${COMMON_TOOLS_SRCS} diff/diff.cpp util/cli_consumer.cpp LIBS SPIRV-Tools-diff SPIRV-Tools-opt ${SPIRV_TOOLS_FULL_VISIBILITY})
  add_spvtools_tool(TARGET spirv-dis  SRCS ${COMMON_TOOLS_SRCS} dis/dis.cpp LIBS ${SPIRV_TOOLS_FULL_VISIBILITY})
  add_spvtools_tool(TARGET spirv-val  SRCS ${COMMON_TOOLS_SRCS} val/val.cpp util/cli_consumer.cpp LIBS ${SPIRV_TOOLS_FULL_VISIBILITY})
  add_spvtools_tool(TARGET spirv-opt  SRCS ${COMMON_TOOLS_SRCS} opt/opt.cpp util/cli_consumer.cpp LIBS SPIRV-Tools-opt ${SPIRV_TOOLS_FULL_VISIBILITY})
  if(NOT (${CMAKE_SYSTEM_NAME} STREQUAL "iOS")) # iOS does not allow std::system calls which spirv-reduce requires
    add_spvtools_tool(TARGET spirv-reduce SRCS ${COMMON_TOOLS_SRCS} reduce/reduce.cpp util/cli_consumer.cpp LIBS SPIRV-Tools-reduce ${SPIRV_TOOLS_FULL_VISIBILITY})
  endif()
  add_spvtools_tool(TARGET spirv-link SRCS ${COMMON_TOOLS_SRCS} link/linker.cpp LIBS SPIRV-Tools-link ${SPIRV_TOOLS_FULL_VISIBILITY})
  add_spvtools_tool(TARGET spirv-lint SRCS ${COMMON_TOOLS_SRCS} lint/lint.cpp util/cli_consumer.cpp LIBS SPIRV-Tools-lint SPIRV-Tools-opt ${SPIRV_TOOLS_FULL_VISIBILITY})
  add_spvtools_tool(TARGET spirv-as
                    SRCS as/as.cpp
                         ${COMMON_TOOLS_SRCS}
                    LIBS ${SPIRV_TOOLS_FULL_VISIBILITY})
  target_include_directories(spirv-as PRIVATE ${spirv-tools_SOURCE_DIR}
                                              ${SPIRV_HEADER_INCLUDE_DIR})
  add_spvtools_tool(TARGET spirv-cfg
                    SRCS cfg/cfg.cpp
                         cfg/bin_to_dot.h
                         cfg/bin_to_dot.cpp
                         ${COMMON_TOOLS_SRCS}
                    LIBS ${SPIRV_TOOLS_FULL_VISIBILITY})
  target_include_directories(spirv-cfg PRIVATE ${spirv-tools_SOURCE_DIR}
                                               ${SPIRV_HEADER_INCLUDE_DIR})
  set(SPIRV_INSTALL_TARGETS spirv-as spirv-dis spirv-val spirv-opt
                            spirv-cfg spirv-link spirv-lint)

  if(NOT (${CMAKE_SYSTEM_NAME} STREQUAL "Android"))
    add_spvtools_tool(TARGET spirv-objdump
                      SRCS objdump/objdump.cpp
                           objdump/extract_source.cpp
                           util/cli_consumer.cpp
                           ${COMMON_TOOLS_SRCS}
                      LIBS ${SPIRV_TOOLS_FULL_VISIBILITY})
    target_include_directories(spirv-objdump PRIVATE ${spirv-tools_SOURCE_DIR}
                                                     ${SPIRV_HEADER_INCLUDE_DIR})
    set(SPIRV_INSTALL_TARGETS ${SPIRV_INSTALL_TARGETS} spirv-objdump)
  endif()

  if(NOT (${CMAKE_SYSTEM_NAME} STREQUAL "iOS"))
    set(SPIRV_INSTALL_TARGETS ${SPIRV_INSTALL_TARGETS} spirv-reduce)
  endif()

  if(SPIRV_BUILD_FUZZER)
    add_spvtools_tool(TARGET spirv-fuzz SRCS fuzz/fuzz.cpp util/cli_consumer.cpp LIBS SPIRV-Tools-fuzz ${SPIRV_TOOLS_FULL_VISIBILITY})
    set(SPIRV_INSTALL_TARGETS ${SPIRV_INSTALL_TARGETS} spirv-fuzz)
  endif(SPIRV_BUILD_FUZZER)

  if(ENABLE_SPIRV_TOOLS_INSTALL)
    install(TARGETS ${SPIRV_INSTALL_TARGETS} EXPORT SPIRV-Tools-toolsTargets)
    export(EXPORT SPIRV-Tools-toolsTargets FILE SPIRV-Tools-toolsTargets.cmake)

    spvtools_config_package_dir(SPIRV-Tools-tools PACKAGE_DIR)
    install(EXPORT SPIRV-Tools-toolsTargets FILE SPIRV-Tools-toolsTargets.cmake
            DESTINATION ${PACKAGE_DIR})

    file(WRITE ${CMAKE_BINARY_DIR}/SPIRV-Tools-toolsConfig.cmake
      "include(CMakeFindDependencyMacro)\n"
      "find_dependency(${SPIRV_TOOLS})\n"
      "include(\${CMAKE_CURRENT_LIST_DIR}/SPIRV-Tools-toolsTargets.cmake)\n"
      )

    install(FILES ${CMAKE_BINARY_DIR}/SPIRV-Tools-toolsConfig.cmake DESTINATION ${PACKAGE_DIR})
  endif(ENABLE_SPIRV_TOOLS_INSTALL)
endif()
