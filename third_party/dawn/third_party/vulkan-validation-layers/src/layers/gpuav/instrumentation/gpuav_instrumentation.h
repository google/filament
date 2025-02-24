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

#include "state_tracker/vertex_index_buffer_state.h"

#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <limits>

struct Location;
struct LogObjectList;

namespace vvl {
struct LabelCommand;
}

namespace gpuav {

class DescriptorSet;
class Validator;
class CommandBuffer;
class Queue;
struct InstrumentationErrorBlob;

void UpdateInstrumentationDescSet(Validator& gpuav, CommandBuffer& cb_state, VkDescriptorSet instrumentation_desc_set,
                                  const Location& loc, InstrumentationErrorBlob& out_instrumentation_error_blob);

void PreCallSetupShaderInstrumentationResources(Validator& gpuav, CommandBuffer& cb_state, VkPipelineBindPoint bind_point,
                                                const Location& loc);

void PostCallSetupShaderInstrumentationResources(Validator& gpuav, CommandBuffer& cb_statee, VkPipelineBindPoint bind_point,
                                                 const Location& loc);

struct VertexAttributeFetchLimit {
    // Default value indicates that no vertex buffer attribute fetching will be OOB
    VkDeviceSize max_vertex_attributes_count = std::numeric_limits<VkDeviceSize>::max();
    vvl::VertexBufferBinding binding_info{};
    VkVertexInputAttributeDescription attribute{};
};

struct InstrumentationErrorBlob {
    std::optional<VertexAttributeFetchLimit> vertex_attribute_fetch_limit_vertex_input_rate{};
    std::optional<VertexAttributeFetchLimit> vertex_attribute_fetch_limit_instance_input_rate{};
    std::optional<vvl::IndexBufferBinding> index_buffer_binding;
};
// Return true iff an error has been found
bool LogInstrumentationError(Validator& gpuav, const CommandBuffer& cb_state, const LogObjectList& objlist,
                             const InstrumentationErrorBlob& instrumentation_error_blob,
                             const std::vector<std::string>& initial_label_stack, uint32_t label_command_i,
                             uint32_t operation_index, const uint32_t* error_record,
                             const std::vector<std::shared_ptr<DescriptorSet>>& descriptor_sets,
                             VkPipelineBindPoint pipeline_bind_point, bool uses_shader_object, bool uses_robustness,
                             const Location& loc);

// Return true iff an error has been found in error_record, among the list of errors this function manages
bool LogMessageInstDescriptorIndexingOOB(Validator& gpuav, const uint32_t* error_record, std::string& out_error_msg,
                                         std::string& out_vuid_msg,
                                         const std::vector<std::shared_ptr<DescriptorSet>>& descriptor_sets, const Location& loc,
                                         bool uses_shader_object, bool& out_oob_access);
bool LogMessageInstDescriptorClass(Validator& gpuav, const uint32_t* error_record, std::string& out_error_msg,
                                   std::string& out_vuid_msg, const std::vector<std::shared_ptr<DescriptorSet>>& descriptor_sets,
                                   const Location& loc, bool uses_shader_object, bool& out_oob_access);
bool LogMessageInstBufferDeviceAddress(const uint32_t* error_record, std::string& out_error_msg, std::string& out_vuid_msg,
                                       bool& out_oob_access);
bool LogMessageInstRayQuery(const uint32_t* error_record, std::string& out_error_msg, std::string& out_vuid_msg);

}  // namespace gpuav
