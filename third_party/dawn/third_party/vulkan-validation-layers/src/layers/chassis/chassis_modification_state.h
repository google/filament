/* Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2022 RasterGrid Kft.
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
#include <vector>
#include "state_tracker/shader_module.h"

namespace vvl {
class Pipeline;
}  // namespace vvl

// These structure are here as a way to bridge information down the chassis.
// This allows the 4 different calls (PreCallValidate, PreCallRecord, Dispatch, PostCallRecord) to share information
//
// The "reason" lots of things are here and not in GPU-AV code is there is no stack memory that GPU-AV can use between PreCall (when
// instrumentation occurs) and PostCall (when we can save it into the state object) to pass information between. While using only
// core checks, there is a small memory overhead cost, but it is small and all on the stack.
namespace chassis {

struct CreateShaderModule {
    // allows PreCallRecord to return a value like PreCallValidate
    bool skip = false;

    // We build a spirv::Module at PreCallRecord time were we can do basic validation of the SPIR-V (which can crash drivers
    // if passed in the Dispatch). It is then passed to PostCallRecord to save in state tracking so it can be used at Pipeline
    // creation time where the rest of the information is needed to do the remaining SPIR-V validation.
    std::shared_ptr<spirv::Module> module_state;  // contains SPIR-V to validate
    spirv::StatelessData stateless_data;
};

struct ShaderObjectInstrumentationData {
    // Need to hold this in memory between the PreCallRecord and the Dispatch
    std::vector<uint32_t> instrumented_spirv;
    // Need to hold this in memory between the PreCallRecord and the PostCallRecord
    uint32_t unique_shader_id = 0;
    bool IsInstrumented() { return unique_shader_id != 0; }

    std::vector<VkDescriptorSetLayout> new_layouts;
};

// VkShaderEXT (VK_EXT_shader_object)
struct ShaderObject {
    // allows PreCallRecord to return a value like PreCallValidate
    bool skip = false;

    std::vector<std::shared_ptr<spirv::Module>> module_states;  // contains SPIR-V to validate
    std::vector<spirv::StatelessData> stateless_data;

    std::vector<VkShaderCreateInfoEXT> modified_create_infos;
    const VkShaderCreateInfoEXT* pCreateInfos = nullptr;

    // Pass the instrumented SPIR-V info from PreCallRecord to Dispatch (so GPU-AV logic can run with it)
    std::vector<ShaderObjectInstrumentationData> instrumentations_data;

    ShaderObject(uint32_t createInfoCount, const VkShaderCreateInfoEXT* create_info) {
        pCreateInfos = create_info;
        module_states.resize(createInfoCount);
        stateless_data.resize(createInfoCount);
    }
};

// Contains a single shader stage (ex. VkGraphicsPipelineCreateInfo::pStages[i])
struct ShaderInstrumentationMetadata {
    // Unique shader ids are used to map SPIR-V to a specific VkShaderModule/VkPipeline/etc handle
    uint32_t unique_shader_id = 0;
    bool IsInstrumented() { return unique_shader_id != 0; }

    // Used to know if VkShaderModuleCreateInfo is passed down VkPipelineShaderStageCreateInfo
    bool passed_in_shader_stage_ci = false;
};

struct CreateGraphicsPipelines {
    std::vector<vku::safe_VkGraphicsPipelineCreateInfo> modified_create_infos;
    const VkGraphicsPipelineCreateInfo* pCreateInfos = nullptr;
    spirv::StatelessData stateless_data[kCommonMaxGraphicsShaderStages];
    // 2D array for [pipelineCount][stageCount]
    std::vector<std::vector<ShaderInstrumentationMetadata>> shader_instrumentations_metadata;

    CreateGraphicsPipelines(const VkGraphicsPipelineCreateInfo* create_info) { pCreateInfos = create_info; }
};

struct CreateComputePipelines {
    std::vector<vku::safe_VkComputePipelineCreateInfo> modified_create_infos;
    const VkComputePipelineCreateInfo* pCreateInfos = nullptr;
    spirv::StatelessData stateless_data;
    // 2D array for [pipelineCount][stageCount]
    // While only 1 compute can be used, need interface to match with graphics/rtx structs
    std::vector<std::vector<ShaderInstrumentationMetadata>> shader_instrumentations_metadata;
    CreateComputePipelines(const VkComputePipelineCreateInfo* create_info) { pCreateInfos = create_info; }
};

struct CreateRayTracingPipelinesNV {
    std::vector<vku::safe_VkRayTracingPipelineCreateInfoCommon> modified_create_infos;
    const VkRayTracingPipelineCreateInfoNV* pCreateInfos = nullptr;
    // 2D array for [pipelineCount][stageCount]
    std::vector<std::vector<ShaderInstrumentationMetadata>> shader_instrumentations_metadata;

    CreateRayTracingPipelinesNV(const VkRayTracingPipelineCreateInfoNV* create_info) { pCreateInfos = create_info; }
};

struct CreateRayTracingPipelinesKHR {
    std::vector<vku::safe_VkRayTracingPipelineCreateInfoKHR> modified_create_infos;
    const VkRayTracingPipelineCreateInfoKHR* pCreateInfos = nullptr;
    // 2D array for [pipelineCount][stageCount]
    std::vector<std::vector<ShaderInstrumentationMetadata>> shader_instrumentations_metadata;

    CreateRayTracingPipelinesKHR(const VkRayTracingPipelineCreateInfoKHR* create_info) { pCreateInfos = create_info; }
};

struct CreatePipelineLayout {
    // This currently only works because GPU-AV is the only layer who creates this state
    // If a 2nd layer starts to use it, can have conflicting values
    std::vector<VkDescriptorSetLayout> new_layouts;
    VkPipelineLayoutCreateInfo modified_create_info;
};

struct CreateBuffer {
    VkBufferCreateInfo modified_create_info;
};

}  // namespace chassis
