# ~~~
# Copyright (c) 2021 Valve Corporation
# Copyright (c) 2021 LunarG, Inc.
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
# ~~~

add_library(testing_framework_util STATIC test_util.cpp)
target_link_libraries(testing_framework_util PUBLIC loader_common_options Vulkan::Headers)

if(UNIX OR APPLE)
    target_link_libraries(testing_framework_util PUBLIC ${CMAKE_DL_LIBS})
endif()

if(UNIX)
    target_compile_options(testing_framework_util PUBLIC -fPIC)
endif()
# Gives access to all headers in the framework folder, in the framework binary, and in the whole project (mainly for loader/generated)
target_include_directories(testing_framework_util PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}" "${PROJECT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/loader/generated")

if (UNIX)
    if (LOADER_ENABLE_ADDRESS_SANITIZER)
        target_compile_options(testing_framework_util PUBLIC -fsanitize=address)
        target_link_options(testing_framework_util PUBLIC -fsanitize=address)
    endif()
    if (LOADER_ENABLE_THREAD_SANITIZER)
        target_compile_options(testing_framework_util PUBLIC -fsanitize=thread)
        target_link_options(testing_framework_util PUBLIC -fsanitize=thread)
        target_compile_options(gtest PUBLIC -fsanitize=thread)
        target_link_options(gtest PUBLIC -fsanitize=thread)
    endif()
    if (LOADER_ENABLE_UNDEFINED_BEHAVIOR_SANITIZER)
        target_compile_options(testing_framework_util PUBLIC -fsanitize=undefined)
        target_link_options(testing_framework_util PUBLIC -fsanitize=undefined)
    endif()
endif()

if (MSVC)
    # silence hidden class member warnings in test framework
    target_compile_options(testing_framework_util PUBLIC /wd4458)
    # Make sure exception handling is enabled for the test framework
    target_compile_options(testing_framework_util PUBLIC /EHsc)
endif()

function(AddSharedLibrary LIBRARY_NAME)
    set(singleValueArgs DEF_FILE)
    set(multiValueArgs SOURCES DEFINITIONS)
    cmake_parse_arguments(PARSED_ARGS "" "${singleValueArgs}"
                          "${multiValueArgs}" ${ARGN} )

    add_library(${LIBRARY_NAME} SHARED ${PARSED_ARGS_SOURCES})
    target_link_libraries(${LIBRARY_NAME} PUBLIC testing_framework_util)
    target_compile_definitions(${LIBRARY_NAME} PRIVATE ${PARSED_ARGS_DEFINITIONS} VK_NO_PROTOTYPES)
    # Windows requires export definitions for these libraries
    if(WIN32)
        target_sources(${LIBRARY_NAME} PRIVATE export_definitions/${PARSED_ARGS_DEF_FILE}.def)
    endif()
endfunction()

add_subdirectory(data)
add_subdirectory(shim)
add_subdirectory(icd)
add_subdirectory(layer)

#configure the framework_config.h.in file - used to locate all the binaries generated so that it can be used in the tests
#setup framework_config_temp.h.in in the current binary directory
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/framework_config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/framework_config_temp.h.in")

# setup framework_config_$<CONFIG> using framework_config_temp.h.in as a source
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/framework_config_$<CONFIG>.h" INPUT "${CMAKE_CURRENT_BINARY_DIR}/framework_config_temp.h.in")

# Add a compiler definition for the path to framework_config.h with the correct config
target_compile_definitions(testing_framework_util PUBLIC FRAMEWORK_CONFIG_HEADER="framework_config_$<CONFIG>.h")

add_library(testing_dependencies STATIC test_environment.cpp test_environment.h)
target_link_libraries(testing_dependencies
    PUBLIC gtest Vulkan::Headers testing_framework_util shim-library)
target_include_directories(testing_dependencies PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
if (APPLE_STATIC_LOADER)
    target_compile_definitions(testing_dependencies PUBLIC "APPLE_STATIC_LOADER=1")
    target_link_libraries(testing_dependencies PUBLIC vulkan)
endif()
