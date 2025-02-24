/* Copyright (c) 2023-2025 Nintendo
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

#pragma once

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "state_tracker/state_object.h"
#include "state_tracker/shader_stage_state.h"
#include "state_tracker/pipeline_layout_state.h"

namespace vvl {
// Represents a VkShaderEXT (VK_EXT_shader_object) handle
struct ShaderObject : public StateObject {
    ShaderObject(Device &dev_data, const VkShaderCreateInfoEXT &create_info, VkShaderEXT shader_object,
                 std::shared_ptr<spirv::Module> &spirv_module, uint32_t createInfoCount, VkShaderEXT *pShaders);

    const vku::safe_VkShaderCreateInfoEXT safe_create_info;
    const VkShaderCreateInfoEXT &create_info;

    std::shared_ptr<const spirv::Module> spirv;
    std::shared_ptr<const spirv::EntryPoint> entrypoint;
    std::vector<VkShaderEXT> linked_shaders;

    // NOTE: this map is 'almost' const and used in performance critical code paths.
    // The values of existing entries in the samplers_used_by_image map
    // are updated at various times. Locking requirements are TBD.
    const ActiveSlotMap active_slots;
    const uint32_t max_active_slot = 0;  // the highest set number in active_slots for pipeline layout compatibility checks

    using SetLayoutVector = std::vector<std::shared_ptr<vvl::DescriptorSetLayout const>>;
    const SetLayoutVector set_layouts;
    const PushConstantRangesId push_constant_ranges;
    const std::vector<PipelineLayoutCompatId> set_compat_ids;

    // TOOD Create a shader object inherited class
    struct InstrumentationData {
        bool was_instrumented = false;
        uint32_t unique_shader_id = 0;
    } instrumentation_data;

    VkShaderEXT VkHandle() const { return handle_.Cast<VkShaderEXT>(); }
    bool IsGraphicsShaderState() const { return create_info.stage != VK_SHADER_STAGE_COMPUTE_BIT; };
    VkPrimitiveTopology GetTopology() const;
};
}  // namespace vvl
