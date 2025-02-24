# ~~~
# Copyright (c) 2016-2023 Valve Corporation
# Copyright (c) 2016-2023 LunarG, Inc.
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
add_library(VkLayer_device_profile_api MODULE)

target_sources(VkLayer_device_profile_api PRIVATE
    ${VVL_SOURCE_DIR}/layers/utils/vk_layer_extension_utils.cpp
    device_profile_api.cpp
    vk_lunarg_device_profile_api_layer.h
)

target_link_libraries(VkLayer_device_profile_api PRIVATE VkLayer_utils)

if (WIN32)
    target_link_options(VkLayer_device_profile_api PRIVATE /DEF:${CMAKE_CURRENT_SOURCE_DIR}/VkLayer_device_profile_api.def)
elseif(APPLE)
    set_target_properties(VkLayer_device_profile_api PROPERTIES SUFFIX ".dylib")
else()
    target_link_options(VkLayer_device_profile_api PRIVATE LINKER:--version-script=${CMAKE_CURRENT_SOURCE_DIR}/libVkLayer_device_profile_api.map)
endif()

target_compile_options(VkLayer_device_profile_api PRIVATE "$<IF:$<CXX_COMPILER_ID:MSVC>,/wd4100,-Wno-unused-parameter>")

if(${CMAKE_CXX_COMPILER_ID} MATCHES "(GNU|Clang)")
    target_compile_options(VkLayer_device_profile_api PRIVATE
        -Wno-sign-compare
        -Wno-shorten-64-to-32
        -Wno-missing-field-initializers
    )
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(VkLayer_device_profile_api PRIVATE
            -Wno-sign-conversion
            -Wno-implicit-int-conversion
        )
    endif()
elseif(MSVC)
    target_compile_options(VkLayer_device_profile_api PRIVATE
        /wd4458 # hiding class member
        /wd4457 # hiding function parameter
        /wd4702 # unreachable code
        /wd4389 # signed/unsigned mismatch
    )
endif()

target_include_directories(VkLayer_device_profile_api PRIVATE .)

set(INTERMEDIATE_FILE "${CMAKE_CURRENT_BINARY_DIR}/profiling.json")

if (WIN32)
    set(JSON_LIBRARY_PATH ".\\\\VkLayer_device_profile_api.dll")
elseif(APPLE)
    set(JSON_LIBRARY_PATH "./libVkLayer_device_profile_api.dylib")
else()
    set(JSON_LIBRARY_PATH "./libVkLayer_device_profile_api.so")
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/VkLayer_device_profile_api.json.in" ${INTERMEDIATE_FILE} @ONLY)

# It's much simpler for development if the device profile layer is in the same location as the validation layer
add_custom_command(TARGET VkLayer_device_profile_api POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:VkLayer_device_profile_api> $<TARGET_FILE_DIR:vvl>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${INTERMEDIATE_FILE} "$<TARGET_FILE_DIR:vvl>/VkLayer_device_profile_api.json"
)
