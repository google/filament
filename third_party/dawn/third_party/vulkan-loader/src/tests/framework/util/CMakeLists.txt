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

add_library(testing_framework_util STATIC
    builder_defines.h
    dispatch_table.cpp
    dispatch_table.h
    dispatchable_handle.h
    dynamic_library_wrapper.cpp
    dynamic_library_wrapper.h
    env_var_wrapper.cpp
    env_var_wrapper.h
    equality_helpers.cpp
    equality_helpers.h
    folder_manager.cpp
    folder_manager.h
    functions.h
    get_executable_path.cpp
    get_executable_path.h
    json_writer.cpp
    json_writer.h
    manifest_builders.cpp
    manifest_builders.h
    platform_wsi.cpp
    platform_wsi.h
    test_defines.h
    vulkan_object_wrappers.cpp
    vulkan_object_wrappers.h
    wide_char_handling.cpp
    wide_char_handling.h
    )
target_link_libraries(testing_framework_util PUBLIC loader_common_options Vulkan::Headers gtest)

if(UNIX OR APPLE)
    target_link_libraries(testing_framework_util PUBLIC ${CMAKE_DL_LIBS})
endif()

if(UNIX)
    target_compile_options(testing_framework_util PUBLIC -fPIC)
endif()
# Gives access to all headers in this folder, the framework folder, the framework binary folder, and the loader/generated folder
target_include_directories(testing_framework_util
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/.." "${CMAKE_CURRENT_BINARY_DIR}" "${PROJECT_SOURCE_DIR}")

if (UNIX)
    if (LOADER_ENABLE_ADDRESS_SANITIZER)
        target_compile_options(testing_framework_util PUBLIC -fsanitize=address,undefined)
        target_link_options(testing_framework_util PUBLIC -fsanitize=address,undefined)
        # workaround regression in macOS SDK 15 when address sanitizer is enabled in debug mode
        # https://developer.apple.com/forums/thread/774632
        if (APPLE)
            target_compile_options(testing_framework_util PRIVATE -mllvm -asan-globals=0)
        endif()
    endif()
    if (LOADER_ENABLE_THREAD_SANITIZER)
        target_compile_options(testing_framework_util PUBLIC -fsanitize=thread)
        target_link_options(testing_framework_util PUBLIC -fsanitize=thread)
        target_compile_options(gtest PUBLIC -fsanitize=thread)
        target_link_options(gtest PUBLIC -fsanitize=thread)
    endif()
endif()

if (MSVC)
    # silence hidden class member warnings in test framework
    target_compile_options(testing_framework_util PUBLIC /wd4458)
    # Make sure exception handling is enabled for the test framework
    target_compile_options(testing_framework_util PUBLIC /EHsc)
endif()

# Add a compiler definition for the path to framework_config.h with the correct config
target_compile_definitions(testing_framework_util PUBLIC FRAMEWORK_CONFIG_HEADER="${FRAMEWORK_CONFIG_HEADER_PATH}")

if (APPLE_STATIC_LOADER)
    target_compile_definitions(testing_framework_util PUBLIC "APPLE_STATIC_LOADER=1")
    target_link_libraries(testing_framework_util PRIVATE vulkan)
endif()
