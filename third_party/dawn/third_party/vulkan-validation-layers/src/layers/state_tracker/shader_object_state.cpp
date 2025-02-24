/* Copyright (c) 2023-2024 Nintendo
 * Copyright (c) 2023-2025 LunarG, Inc.
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

#include "shader_object_state.h"
#include "shader_module.h"
#include "state_tracker/state_tracker.h"

namespace vvl {
static ShaderObject::SetLayoutVector GetSetLayouts(Device &dev_data, const VkShaderCreateInfoEXT &pCreateInfo) {
    ShaderObject::SetLayoutVector set_layouts(pCreateInfo.setLayoutCount);

    for (uint32_t i = 0; i < pCreateInfo.setLayoutCount; ++i) {
        set_layouts[i] = dev_data.Get<vvl::DescriptorSetLayout>(pCreateInfo.pSetLayouts[i]);
    }
    return set_layouts;
}

ShaderObject::ShaderObject(Device &dev_data, const VkShaderCreateInfoEXT &create_info_i, VkShaderEXT handle,
                           std::shared_ptr<spirv::Module> &spirv_module, uint32_t createInfoCount, VkShaderEXT *pShaders)
    : StateObject(handle, kVulkanObjectTypeShaderEXT),
      safe_create_info(&create_info_i),
      create_info(*safe_create_info.ptr()),
      spirv(spirv_module),
      entrypoint(spirv ? spirv->FindEntrypoint(create_info.pName, create_info.stage) : nullptr),
      active_slots(GetActiveSlots(entrypoint)),
      max_active_slot(GetMaxActiveSlot(active_slots)),
      set_layouts(GetSetLayouts(dev_data, create_info)),
      push_constant_ranges(GetCanonicalId(create_info.pushConstantRangeCount, create_info.pPushConstantRanges)),
      set_compat_ids(GetCompatForSet(set_layouts, push_constant_ranges)) {
    if ((create_info.flags & VK_SHADER_CREATE_LINK_STAGE_BIT_EXT) != 0) {
        for (uint32_t i = 0; i < createInfoCount; ++i) {
            if (pShaders[i] != handle) {
                linked_shaders.push_back(pShaders[i]);
            }
        }
    }
}

VkPrimitiveTopology ShaderObject::GetTopology() const {
    if (spirv) {
        const auto topology = spirv->GetTopology(*entrypoint);
        if (topology) {
            return *topology;
        }
    }
    return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
}
}  // namespace vvl
