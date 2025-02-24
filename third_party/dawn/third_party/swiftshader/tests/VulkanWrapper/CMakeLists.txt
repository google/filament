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

set(VULKAN_WRAPPER_SRC_FILES
    Buffer.cpp
    Buffer.hpp
    DrawTester.cpp
    DrawTester.hpp
    Framebuffer.cpp
    Framebuffer.hpp
    Image.cpp
    Image.hpp
    Swapchain.cpp
    Swapchain.hpp
    Util.cpp
    Util.hpp
    VulkanHeaders.cpp
    VulkanHeaders.hpp
    VulkanTester.cpp
    VulkanTester.hpp
    Window.cpp
    Window.hpp
)

add_library(VulkanWrapper STATIC
    ${VULKAN_WRAPPER_SRC_FILES}
)

if (NOT TARGET glslang)
    message(FATAL_ERROR "Missing required target: glslang")
endif()

if (NOT TARGET glslang-default-resource-limits)
    message(FATAL_ERROR "Missing required target: glslang-default-resource-limits")
endif()

if (NOT TARGET SPIRV)
    message(FATAL_ERROR "Missing required target: SPIRV")
endif()

set_target_properties(VulkanWrapper PROPERTIES
    FOLDER "Tests"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

target_include_directories(VulkanWrapper
    PUBLIC
        "."
        "${SWIFTSHADER_DIR}/include"
)

target_compile_definitions(VulkanWrapper
    PUBLIC
        "STANDALONE"
)

target_compile_options(VulkanWrapper
    PRIVATE
        ${ROOT_PROJECT_COMPILE_OPTIONS}
)

target_link_options(VulkanWrapper
    PRIVATE
        ${SWIFTSHADER_LINK_FLAGS}
)

target_link_libraries(VulkanWrapper
    PUBLIC
        glslang
        glslang-default-resource-limits
        SPIRV
)
