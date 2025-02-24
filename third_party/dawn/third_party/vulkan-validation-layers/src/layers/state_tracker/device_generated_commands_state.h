/* Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
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

#pragma once

#include "state_tracker/state_object.h"
#include <vulkan/utility/vk_safe_struct.hpp>

namespace vvl {
class Device;
class Pipeline;
struct ShaderObject;

class IndirectExecutionSet : public StateObject {
  public:
    const vku::safe_VkIndirectExecutionSetCreateInfoEXT safe_create_info;
    const VkIndirectExecutionSetCreateInfoEXT &create_info;

    IndirectExecutionSet(Device &dev, VkIndirectExecutionSetEXT handle, const VkIndirectExecutionSetCreateInfoEXT *pCreateInfo);
    VkIndirectExecutionSetEXT VkHandle() const { return handle_.Cast<VkIndirectExecutionSetEXT>(); }

    const bool is_pipeline;
    const bool is_shader_objects;

    // Need to keep a smart pointer around because from spec:
    // "The characteristics of initialPipeline will be used to validate all pipelines added to the set even if they are removed from
    // the set or destroyed"
    std::shared_ptr<Pipeline> initial_pipeline;
    std::shared_ptr<ShaderObject> initial_fragment_shader_object;

    uint32_t max_pipeline_count = 0;
    uint32_t max_shader_count = 0;
    VkShaderStageFlags shader_stage_flags = 0;
};

class IndirectCommandsLayout : public StateObject {
  public:
    const vku::safe_VkIndirectCommandsLayoutCreateInfoEXT safe_create_info;
    const VkIndirectCommandsLayoutCreateInfoEXT &create_info;

    IndirectCommandsLayout(Device &dev, VkIndirectCommandsLayoutEXT handle,
                           const VkIndirectCommandsLayoutCreateInfoEXT *pCreateInfo);
    VkIndirectCommandsLayoutEXT VkHandle() const { return handle_.Cast<VkIndirectCommandsLayoutEXT>(); }

    VkPipelineBindPoint bind_point;

    bool has_execution_set_token = false;     // VK_INDIRECT_COMMANDS_TOKEN_TYPE_EXECUTION_SET_EXT
    bool has_vertex_buffer_token = false;     // VK_INDIRECT_COMMANDS_TOKEN_TYPE_VERTEX_BUFFER_EXT
    bool has_draw_token = false;              // VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_*
    bool has_multi_draw_count_token = false;  // VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_*_COUNT_*

    VkShaderStageFlags execution_set_token_shader_stage_flags = 0;
};

}  // namespace vvl
