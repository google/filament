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

#include <vulkan/vulkan_core.h>

struct Location;
namespace gpuav {
struct DebugPrintfBufferInfo;
class CommandBuffer;
class Validator;

namespace debug_printf {
bool UpdateInstrumentationDescSet(Validator& gpuav, CommandBuffer& cb_state, VkDescriptorSet instrumentation_desc_set,
                                  VkPipelineBindPoint bind_point, const Location& loc);
void AnalyzeAndGenerateMessage(Validator& gpuav, VkCommandBuffer command_buffer, VkQueue queue, DebugPrintfBufferInfo& buffer_info,
                               uint32_t* const debug_output_buffer, const Location& loc);
}  // namespace debug_printf

}  // namespace gpuav