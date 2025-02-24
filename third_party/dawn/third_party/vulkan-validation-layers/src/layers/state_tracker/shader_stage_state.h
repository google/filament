/* Copyright (c) 2024-2025 The Khronos Group Inc.
 * Copyright (c) 2024-2025 Valve Corporation
 * Copyright (c) 2024-2025 LunarG, Inc.
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
 *
 */

#pragma once
#include <vulkan/vulkan.h>
#include "containers/custom_containers.h"

namespace vku {
struct safe_VkPipelineShaderStageCreateInfo;
struct safe_VkShaderCreateInfoEXT;
struct safe_VkSpecializationInfo;
}  // namespace vku

namespace vvl {
struct ShaderModule;
}  // namespace vvl

namespace spirv {
struct Module;
struct EntryPoint;
class Instruction;
}  // namespace spirv

// This is a wrapper around the Shader as it may come from a Pipeline or Shader Object.
struct ShaderStageState {
    // We use this over a spirv::Module because there are times we need to create empty objects
    std::shared_ptr<const vvl::ShaderModule> module_state;
    std::shared_ptr<const spirv::Module> spirv_state;
    const vku::safe_VkPipelineShaderStageCreateInfo *pipeline_create_info;
    const vku::safe_VkShaderCreateInfoEXT *shader_object_create_info;
    // If null, means it is an empty object, no SPIR-V backing it
    std::shared_ptr<const spirv::EntryPoint> entrypoint;

    ShaderStageState(const vku::safe_VkPipelineShaderStageCreateInfo *pipeline_create_info,
                     const vku::safe_VkShaderCreateInfoEXT *shader_object_create_info,
                     std::shared_ptr<const vvl::ShaderModule> module_state, std::shared_ptr<const spirv::Module> spirv_state);

    bool HasPipeline() const { return pipeline_create_info != nullptr; }
    const char *GetPName() const;
    VkShaderStageFlagBits GetStage() const;
    vku::safe_VkSpecializationInfo *GetSpecializationInfo() const;
    const void *GetPNext() const;
    bool GetInt32ConstantValue(const spirv::Instruction &insn, uint32_t *value) const;
    bool GetBooleanConstantValue(const spirv::Instruction &insn, bool *value) const;
};

namespace spirv {
struct ResourceInterfaceVariable;
}  // namespace spirv

struct DescriptorRequirement {
    uint64_t revalidate_hash;
    const spirv::ResourceInterfaceVariable *variable;
    DescriptorRequirement() : revalidate_hash(0), variable(nullptr) {}
};

inline bool operator==(const DescriptorRequirement &a, const DescriptorRequirement &b) noexcept {
    return a.revalidate_hash == b.revalidate_hash;
}

inline bool operator<(const DescriptorRequirement &a, const DescriptorRequirement &b) noexcept {
    return a.revalidate_hash < b.revalidate_hash;
}

// < binding index (of descriptor set) : meta data >
typedef std::unordered_multimap<uint32_t, DescriptorRequirement> BindingVariableMap;

// Capture which slots (set#->bindings) are actually used by the shaders of this pipeline/shaderObject.
// This is same as "statically used" in vkspec.html#shaders-staticuse
using ActiveSlotMap = vvl::unordered_map<uint32_t, BindingVariableMap>;

void GetActiveSlots(ActiveSlotMap &active_slots, const std::shared_ptr<const spirv::EntryPoint> &entrypoint);
ActiveSlotMap GetActiveSlots(const std::vector<ShaderStageState> &stage_states);
ActiveSlotMap GetActiveSlots(const std::shared_ptr<const spirv::EntryPoint> &entrypoint);

uint32_t GetMaxActiveSlot(const ActiveSlotMap &active_slots);
