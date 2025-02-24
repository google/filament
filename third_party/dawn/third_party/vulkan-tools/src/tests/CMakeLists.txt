# ~~~
# Copyright (c) 2023 Valve Corporation
# Copyright (c) 2023 LunarG, Inc.
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

if (ANDROID)

    # Ensure ANativeActivity_onCreate is being exported from vkcube
    find_program(GNU_NM NAMES nm)
    if (GNU_NM)
        add_test(NAME ANativeActivity_onCreate COMMAND ${GNU_NM} --dynamic $<TARGET_FILE:vkcube>)
        set_tests_properties(ANativeActivity_onCreate
            PROPERTIES PASS_REGULAR_EXPRESSION "T ANativeActivity_onCreate"
        )
    endif()

    return()
endif()

find_package(VulkanLoader CONFIG)

find_package(GTest REQUIRED CONFIG QUIET)

add_executable(vulkan_tools_tests)
target_sources(vulkan_tools_tests PRIVATE
    main.cpp
    test_common.h
    icd/mock_icd_tests.cpp
)
get_target_property(TEST_SOURCES vulkan_tools_tests SOURCES)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${TEST_SOURCES})

target_include_directories(vulkan_tools_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(vulkan_tools_tests GTest::gtest Vulkan::Headers Vulkan::Loader)
if (WIN32)
    target_compile_definitions(vulkan_tools_tests PUBLIC -DVK_USE_PLATFORM_WIN32_KHR -DWIN32_LEAN_AND_MEAN -DNOMINMAX)
endif()
set_target_properties(vulkan_tools_tests PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

# Set the exact path to Mock JSON ICD
target_compile_definitions(vulkan_tools_tests PRIVATE MOCK_ICD_JSON_MANIFEST_PATH="$<TARGET_FILE_DIR:VkICD_mock_icd>")

if (DEFINED GIT_BRANCH_NAME AND DEFINED GIT_TAG_INFO)
    target_compile_definitions(vulkan_tools_tests PRIVATE GIT_BRANCH_NAME="${GIT_BRANCH_NAME}" GIT_TAG_INFO="${GIT_TAG_INFO}")
endif()

if (WIN32)
    # Copy the loader shared lib (if built) to the test application directory so the test app finds it.
    add_custom_command(TARGET vulkan_tools_tests POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Vulkan::Loader> $<TARGET_FILE_DIR:vulkan_tools_tests>)
endif()

include(GoogleTest)
gtest_discover_tests(vulkan_tools_tests DISCOVERY_TIMEOUT 100)
