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

#include "shader_stage_state.h"

#include "state_tracker/shader_module.h"
#include <vulkan/utility/vk_safe_struct.hpp>

// Common for both Pipeline and Shader Object
void GetActiveSlots(ActiveSlotMap &active_slots, const std::shared_ptr<const spirv::EntryPoint> &entrypoint) {
    if (!entrypoint) {
        return;
    }
    // Capture descriptor uses for the pipeline
    for (const auto &variable : entrypoint->resource_interface_variables) {
        // While validating shaders capture which slots are used by the pipeline
        DescriptorRequirement entry;
        entry.variable = &variable;
        entry.revalidate_hash = variable.descriptor_hash;
        active_slots[variable.decorations.set].emplace(variable.decorations.binding, entry);
    }
}

// Used by pipeline
ActiveSlotMap GetActiveSlots(const std::vector<ShaderStageState> &stage_states) {
    ActiveSlotMap active_slots;
    for (const auto &stage : stage_states) {
        GetActiveSlots(active_slots, stage.entrypoint);
    }
    return active_slots;
}

// Used by Shader Object
ActiveSlotMap GetActiveSlots(const std::shared_ptr<const spirv::EntryPoint> &entrypoint) {
    ActiveSlotMap active_slots;
    GetActiveSlots(active_slots, entrypoint);
    return active_slots;
}

uint32_t GetMaxActiveSlot(const ActiveSlotMap &active_slots) {
    uint32_t max_active_slot = 0;
    for (const auto &entry : active_slots) {
        max_active_slot = std::max(max_active_slot, entry.first);
    }
    return max_active_slot;
}

const char *ShaderStageState::GetPName() const {
    return (pipeline_create_info) ? pipeline_create_info->pName : shader_object_create_info->pName;
}

VkShaderStageFlagBits ShaderStageState::GetStage() const {
    return (pipeline_create_info) ? pipeline_create_info->stage : shader_object_create_info->stage;
}

vku::safe_VkSpecializationInfo *ShaderStageState::GetSpecializationInfo() const {
    return (pipeline_create_info) ? pipeline_create_info->pSpecializationInfo : shader_object_create_info->pSpecializationInfo;
}

const void *ShaderStageState::GetPNext() const {
    return (pipeline_create_info) ? pipeline_create_info->pNext : shader_object_create_info->pNext;
}

bool ShaderStageState::GetInt32ConstantValue(const spirv::Instruction &insn, uint32_t *value) const {
    const spirv::Instruction *type_id = spirv_state->FindDef(insn.Word(1));
    if (type_id->Opcode() != spv::OpTypeInt || type_id->Word(2) != 32) {
        return false;
    }

    if (insn.Opcode() == spv::OpConstant) {
        *value = insn.Word(3);
        return true;
    } else if (insn.Opcode() == spv::OpSpecConstant) {
        *value = insn.Word(3);  // default value
        const auto *spec_info = GetSpecializationInfo();
        const uint32_t spec_id = spirv_state->static_data_.id_to_spec_id.at(insn.Word(2));
        if (spec_info && spec_id < spec_info->mapEntryCount) {
            memcpy(value, (uint8_t *)spec_info->pData + spec_info->pMapEntries[spec_id].offset,
                   spec_info->pMapEntries[spec_id].size);
        }
        return true;
    }

    // This means the value is not known until runtime and will need to be checked in GPU-AV
    return false;
}

bool ShaderStageState::GetBooleanConstantValue(const spirv::Instruction &insn, bool *value) const {
    const spirv::Instruction *type_id = spirv_state->FindDef(insn.Word(1));
    if (type_id->Opcode() != spv::OpTypeBool) {
        return false;
    }

    if (insn.Opcode() == spv::OpConstantFalse) {
        *value = false;
        return true;
    } else if (insn.Opcode() == spv::OpConstantTrue) {
        *value = true;
        return true;
    } else if (insn.Opcode() == spv::OpSpecConstantTrue || insn.Opcode() == spv::OpSpecConstantFalse) {
        *value = insn.Opcode() == spv::OpSpecConstantTrue;  // default value
        const auto *spec_info = GetSpecializationInfo();
        const uint32_t spec_id = spirv_state->static_data_.id_to_spec_id.at(insn.Word(2));
        if (spec_info && spec_id < spec_info->mapEntryCount) {
            memcpy(value, (uint8_t *)spec_info->pData + spec_info->pMapEntries[spec_id].offset, 1);
        }
        return true;
    }

    // This means the value is not known until runtime and will need to be checked in GPU-AV
    return false;
}

ShaderStageState::ShaderStageState(const vku::safe_VkPipelineShaderStageCreateInfo *pipeline_create_info,
                                   const vku::safe_VkShaderCreateInfoEXT *shader_object_create_info,
                                   std::shared_ptr<const vvl::ShaderModule> module_state,
                                   std::shared_ptr<const spirv::Module> spirv_state)
    : module_state(module_state),
      spirv_state(spirv_state),
      pipeline_create_info(pipeline_create_info),
      shader_object_create_info(shader_object_create_info),
      entrypoint(spirv_state ? spirv_state->FindEntrypoint(GetPName(), GetStage()) : nullptr) {}
