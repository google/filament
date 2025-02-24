# Copyright 2020 The SwiftShader Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set(ROOT_PROJECT_COMPILE_OPTIONS
)

set(ROOT_PROJECT_LINK_LIBRARIES
    ${OS_LIBS}
    ${SWIFTSHADER_LIBS}
)

set(VULKAN_UNIT_TESTS_SRC_FILES
    BasicTests.cpp
    ComputeTests.cpp
    Device.cpp
    Device.hpp
    DrawTests.cpp
    Driver.cpp
    Driver.hpp
    main.cpp
    VkGlobalFuncs.hpp
    VkInstanceFuncs.hpp
)

add_executable(vk-unittests
    ${VULKAN_UNIT_TESTS_SRC_FILES}
)

# vk-unittests loads vk_swiftshader, so make sure it gets built first
add_dependencies(vk-unittests
    vk_swiftshader
)

set_target_properties(vk-unittests PROPERTIES
    FOLDER "Tests"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

target_include_directories(vk-unittests
    PRIVATE
        "${SWIFTSHADER_DIR}/include"
)

target_compile_definitions(vk-unittests
    PRIVATE
        "STANDALONE"
)

target_compile_options(vk-unittests
    PRIVATE
        ${ROOT_PROJECT_COMPILE_OPTIONS}
)

target_link_options(vk-unittests
    PRIVATE
        ${SWIFTSHADER_LINK_FLAGS}
)

target_link_libraries(vk-unittests
    PRIVATE
        gtest
        gmock
        SPIRV-Tools
        VulkanWrapper
        ${ROOT_PROJECT_LINK_LIBRARIES}
)
