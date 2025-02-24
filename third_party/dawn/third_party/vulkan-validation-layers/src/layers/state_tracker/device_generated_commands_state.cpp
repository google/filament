/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "state_tracker/device_generated_commands_state.h"

vvl::IndirectExecutionSet::IndirectExecutionSet(vvl::Device &dev, VkIndirectExecutionSetEXT handle,
                                                const VkIndirectExecutionSetCreateInfoEXT *pCreateInfo)
    : StateObject(handle, kVulkanObjectTypeIndirectExecutionSetEXT),
      safe_create_info(pCreateInfo),
      create_info(*safe_create_info.ptr()),
      is_pipeline(pCreateInfo->type == VK_INDIRECT_EXECUTION_SET_INFO_TYPE_PIPELINES_EXT),
      is_shader_objects(pCreateInfo->type == VK_INDIRECT_EXECUTION_SET_INFO_TYPE_SHADER_OBJECTS_EXT) {
    if (is_pipeline && pCreateInfo->info.pPipelineInfo) {
        const VkIndirectExecutionSetPipelineInfoEXT &pipeline_info = *pCreateInfo->info.pPipelineInfo;
        max_pipeline_count = pipeline_info.maxPipelineCount;
    } else if (is_shader_objects && pCreateInfo->info.pShaderInfo) {
        const VkIndirectExecutionSetShaderInfoEXT &shader_info = *pCreateInfo->info.pShaderInfo;
        max_shader_count = shader_info.maxShaderCount;
    }
}

vvl::IndirectCommandsLayout::IndirectCommandsLayout(vvl::Device &dev, VkIndirectCommandsLayoutEXT handle,
                                                    const VkIndirectCommandsLayoutCreateInfoEXT *pCreateInfo)
    : StateObject(handle, kVulkanObjectTypeIndirectCommandsLayoutEXT),
      safe_create_info(pCreateInfo),
      create_info(*safe_create_info.ptr()),
      // default to graphics as it is most common and has most cases
      bind_point(VK_PIPELINE_BIND_POINT_GRAPHICS) {
    for (uint32_t i = 0; i < pCreateInfo->tokenCount; i++) {
        const VkIndirectCommandsLayoutTokenEXT &token = pCreateInfo->pTokens[i];
        switch (token.type) {
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_EXECUTION_SET_EXT:
                has_execution_set_token = true;
                execution_set_token_shader_stage_flags = token.data.pExecutionSet->shaderStages;
                break;
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_VERTEX_BUFFER_EXT:
                has_vertex_buffer_token = true;
                break;
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_INDEXED_COUNT_EXT:
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_COUNT_EXT:
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_COUNT_EXT:
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_COUNT_NV_EXT:
                has_draw_token = true;
                has_multi_draw_count_token = true;
                break;
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_INDEXED_EXT:
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT:
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_NV_EXT:
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_EXT:
                has_draw_token = true;
                break;
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DISPATCH_EXT:
                bind_point = VK_PIPELINE_BIND_POINT_COMPUTE;
                break;
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_TRACE_RAYS2_EXT:
                bind_point = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
                break;
            default:
                break;
        }
    }
}
