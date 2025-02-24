/* Copyright (c) 2020-2024 The Khronos Group Inc.
 * Copyright (c) 2020-2024 Valve Corporation
 * Copyright (c) 2020-2024 LunarG, Inc.
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

#include <vulkan/vulkan.h>

struct Location;
struct LastBound;

namespace gpuav {
class CommandBuffer;
class Validator;
struct DescriptorCommandBinding;

namespace descriptor {

void PreCallActionCommandPostProcess(Validator& gpuav, CommandBuffer& cb_state, const LastBound& last_bound, const Location& loc);
void PreCallActionCommand(Validator& gpuav, CommandBuffer& cb_state, VkPipelineBindPoint pipeline_bind_point, const Location& loc);

void UpdateBoundDescriptorsPostProcess(Validator& gpuav, CommandBuffer& cb_state, const LastBound& last_bound,
                                       DescriptorCommandBinding& descriptor_command_binding, const Location& loc);
void UpdateBoundDescriptorsDescriptorChecks(Validator& gpuav, CommandBuffer& cb_state, const LastBound& last_bound,
                                            DescriptorCommandBinding& descriptor_command_binding, const Location& loc);
void UpdateBoundDescriptors(Validator& gpuav, CommandBuffer& cb_state, VkPipelineBindPoint pipeline_bind_point,
                            const Location& loc);
[[nodiscard]] bool UpdateDescriptorStateSSBO(Validator& gpuav, CommandBuffer& cb_state, const Location& loc);
}  // namespace descriptor
}  // namespace gpuav
