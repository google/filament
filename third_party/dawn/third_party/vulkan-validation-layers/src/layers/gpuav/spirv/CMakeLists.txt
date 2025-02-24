# ~~~
# Copyright (c) 2024-2025 LunarG, Inc.
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

add_library(gpu_av_spirv STATIC)
target_sources(gpu_av_spirv PRIVATE
    # Passes
    descriptor_indexing_oob_pass.h
    descriptor_indexing_oob_pass.cpp
    descriptor_class_general_buffer_pass.h
    descriptor_class_general_buffer_pass.cpp
    descriptor_class_texel_buffer_pass.h
    descriptor_class_texel_buffer_pass.cpp
    buffer_device_address_pass.h
    buffer_device_address_pass.cpp
    ray_query_pass.h
    ray_query_pass.cpp
    debug_printf_pass.h
    debug_printf_pass.cpp
    post_process_descriptor_indexing_pass.h
    post_process_descriptor_indexing_pass.cpp
    vertex_attribute_fetch_oob.h
    vertex_attribute_fetch_oob.cpp

    # Framework
    instruction.h
    instruction.cpp
    function_basic_block.h
    function_basic_block.cpp
    interface.h
    link.h
    module.h
    module.cpp
    type_manager.h
    type_manager.cpp
    inject_conditional_function_pass.h
    inject_conditional_function_pass.cpp
    inject_function_pass.h
    inject_function_pass.cpp
    pass.h
    pass.cpp
    ${VVL_SOURCE_DIR}/layers/${API_TYPE}/generated/spirv_grammar_helper.cpp

    ${VVL_SOURCE_DIR}/layers/${API_TYPE}/generated/instrumentation_descriptor_indexing_oob_bindless_comp.h
    ${VVL_SOURCE_DIR}/layers/${API_TYPE}/generated/instrumentation_descriptor_indexing_oob_bindless_comp.cpp
    ${VVL_SOURCE_DIR}/layers/${API_TYPE}/generated/instrumentation_descriptor_indexing_oob_non_bindless_comp.h
    ${VVL_SOURCE_DIR}/layers/${API_TYPE}/generated/instrumentation_descriptor_indexing_oob_non_bindless_comp.cpp
    ${VVL_SOURCE_DIR}/layers/${API_TYPE}/generated/instrumentation_descriptor_class_general_buffer_comp.h
    ${VVL_SOURCE_DIR}/layers/${API_TYPE}/generated/instrumentation_descriptor_class_general_buffer_comp.cpp
    ${VVL_SOURCE_DIR}/layers/${API_TYPE}/generated/instrumentation_descriptor_class_texel_buffer_comp.h
    ${VVL_SOURCE_DIR}/layers/${API_TYPE}/generated/instrumentation_descriptor_class_texel_buffer_comp.cpp
    ${VVL_SOURCE_DIR}/layers/${API_TYPE}/generated/instrumentation_buffer_device_address_comp.h
    ${VVL_SOURCE_DIR}/layers/${API_TYPE}/generated/instrumentation_buffer_device_address_comp.cpp
    ${VVL_SOURCE_DIR}/layers/${API_TYPE}/generated/instrumentation_ray_query_comp.h
    ${VVL_SOURCE_DIR}/layers/${API_TYPE}/generated/instrumentation_ray_query_comp.cpp
    ${VVL_SOURCE_DIR}/layers/${API_TYPE}/generated/instrumentation_post_process_descriptor_index_comp.h
    ${VVL_SOURCE_DIR}/layers/${API_TYPE}/generated/instrumentation_post_process_descriptor_index_comp.cpp
    ${VVL_SOURCE_DIR}/layers/${API_TYPE}/generated/instrumentation_vertex_attribute_fetch_oob_vert.h
    ${VVL_SOURCE_DIR}/layers/${API_TYPE}/generated/instrumentation_vertex_attribute_fetch_oob_vert.cpp
)

target_include_directories(gpu_av_spirv PRIVATE .
    ${VVL_SOURCE_DIR}/layers
    ${VVL_SOURCE_DIR}/layers/${API_TYPE}
)

target_link_libraries(gpu_av_spirv PRIVATE
    SPIRV-Headers::SPIRV-Headers
    VkLayer_utils # To pull in compile_definitions
)

if (VVL_ENABLE_TRACY)
    target_link_libraries(gpu_av_spirv PRIVATE TracyClient)
endif()

if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    target_compile_options(gpu_av_spirv PRIVATE
        -Wno-sign-conversion
        -Wno-implicit-int-conversion
    )
elseif(MSVC)
    target_compile_options(gpu_av_spirv PRIVATE
        /wd4324 # padding
        /wd4458 # hiding class member
        /wd4457 # hiding function parameter
        /wd4702 # unreachable code
        /wd4389 # signed/unsigned mismatch
    )
endif()