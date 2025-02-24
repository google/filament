/* Copyright (c) 2020-2025 The Khronos Group Inc.
 * Copyright (c) 2020-2025 Valve Corporation
 * Copyright (c) 2020-2025 LunarG, Inc.
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

#include "gpuav/descriptor_validation/gpuav_descriptor_validation.h"

#include "drawdispatch/descriptor_validator.h"
#include "gpuav/core/gpuav.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "gpuav/resources/gpuav_shader_resources.h"
#include "state_tracker/shader_module.h"

namespace gpuav {
namespace descriptor {

void PreCallActionCommandPostProcess(Validator &gpuav, CommandBuffer &cb_state, const LastBound &last_bound, const Location &loc) {
    // Can hit if current action command doesn't use any descriptor
    if (cb_state.descriptor_command_bindings.empty()) {
        return;
    }

    // Should have just been updated
    if (!last_bound.pipeline_state && !last_bound.HasShaderObjects()) {
        gpuav.InternalError(gpuav.device, loc, "Unrecognized pipeline nor shader object");
        return;
    }

    // TODO - Add Shader Object support
    if (!last_bound.pipeline_state) {
        return;
    }
    const auto &active_slot = last_bound.pipeline_state->active_slots;

    const uint32_t descriptor_command_binding_index = (uint32_t)cb_state.descriptor_command_bindings.size() - 1;
    auto &action_command_snapshot = cb_state.action_command_snapshots.emplace_back(descriptor_command_binding_index);

    const size_t number_of_sets = last_bound.ds_slots.size();
    action_command_snapshot.binding_req_maps.reserve(number_of_sets);

    for (uint32_t i = 0; i < number_of_sets; i++) {
        if (!last_bound.ds_slots[i].ds_state) {
            continue;  // can have gaps in descriptor sets
        }

        auto slot = active_slot.find(i);
        if (slot != active_slot.end()) {
            action_command_snapshot.binding_req_maps.emplace_back(&slot->second);
        }
    }
}

void PreCallActionCommand(Validator &gpuav, CommandBuffer &cb_state, VkPipelineBindPoint pipeline_bind_point, const Location &loc) {
    // Currently this is only for updating the binding_req_map which is used for post processing only
    if (!gpuav.gpuav_settings.shader_instrumentation.post_process_descriptor_index) return;

    const auto lv_bind_point = ConvertToLvlBindPoint(pipeline_bind_point);
    auto const &last_bound = cb_state.lastBound[lv_bind_point];

    PreCallActionCommandPostProcess(gpuav, cb_state, last_bound, loc);
}

void UpdateBoundDescriptorsPostProcess(Validator &gpuav, CommandBuffer &cb_state, const LastBound &last_bound,
                                       DescriptorCommandBinding &descriptor_command_binding, const Location &loc) {
    if (!gpuav.gpuav_settings.shader_instrumentation.post_process_descriptor_index) return;

    // Create a new buffer to hold our BDA pointers
    VkBufferCreateInfo buffer_info = vku::InitStructHelper();
    buffer_info.size = sizeof(glsl::PostProcessSSBO);
    buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    VmaAllocationCreateInfo alloc_info = {};
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    alloc_info.pool = VK_NULL_HANDLE;
    const bool success = descriptor_command_binding.post_process_ssbo_buffer.Create(loc, &buffer_info, &alloc_info);
    if (!success) {
        return;
    }

    auto ssbo_buffer_ptr = (glsl::PostProcessSSBO *)descriptor_command_binding.post_process_ssbo_buffer.MapMemory(loc);
    memset(ssbo_buffer_ptr, 0, sizeof(glsl::PostProcessSSBO));

    cb_state.post_process_buffer_lut = descriptor_command_binding.post_process_ssbo_buffer.VkHandle();

    const size_t number_of_sets = last_bound.ds_slots.size();
    for (uint32_t i = 0; i < number_of_sets; i++) {
        const auto &ds_slot = last_bound.ds_slots[i];
        if (!ds_slot.ds_state) {
            continue;  // can have gaps in descriptor sets
        }

        auto bound_descriptor_set = static_cast<DescriptorSet *>(ds_slot.ds_state.get());
        ssbo_buffer_ptr->descriptor_index_post_process_buffers[i] = bound_descriptor_set->GetPostProcessBuffer(gpuav, loc);
    }

    descriptor_command_binding.post_process_ssbo_buffer.UnmapMemory();
}

void UpdateBoundDescriptorsDescriptorChecks(Validator &gpuav, CommandBuffer &cb_state, const LastBound &last_bound,
                                            DescriptorCommandBinding &descriptor_command_binding, const Location &loc) {
    if (!gpuav.gpuav_settings.shader_instrumentation.descriptor_checks) return;

    // Create a new buffer to hold our BDA pointers
    VkBufferCreateInfo buffer_info = vku::InitStructHelper();
    buffer_info.size = sizeof(glsl::DescriptorStateSSBO);
    buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    VmaAllocationCreateInfo alloc_info = {};
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    alloc_info.pool = VK_NULL_HANDLE;
    const bool success = descriptor_command_binding.descritpor_state_ssbo_buffer.Create(loc, &buffer_info, &alloc_info);
    if (!success) {
        return;
    }

    auto ssbo_buffer_ptr = (glsl::DescriptorStateSSBO *)descriptor_command_binding.descritpor_state_ssbo_buffer.MapMemory(loc);
    memset(ssbo_buffer_ptr, 0, sizeof(glsl::DescriptorStateSSBO));

    cb_state.descriptor_indexing_buffer = descriptor_command_binding.descritpor_state_ssbo_buffer.VkHandle();

    ssbo_buffer_ptr->initialized_status = gpuav.desc_heap_->GetDeviceAddress();

    const size_t number_of_sets = last_bound.ds_slots.size();
    for (uint32_t i = 0; i < number_of_sets; i++) {
        const auto &ds_slot = last_bound.ds_slots[i];
        if (!ds_slot.ds_state) {
            continue;  // can have gaps in descriptor sets
        }

        auto bound_descriptor_set = static_cast<DescriptorSet *>(ds_slot.ds_state.get());
        // If update after bind, wait until we process things in UpdateDescriptorStateSSBO()
        if (!bound_descriptor_set->IsUpdateAfterBind()) {
            ssbo_buffer_ptr->descriptor_set_types[i] = bound_descriptor_set->GetTypeAddress(gpuav, loc);
        }
    }

    descriptor_command_binding.descritpor_state_ssbo_buffer.UnmapMemory();
}

void UpdateBoundDescriptors(Validator &gpuav, CommandBuffer &cb_state, VkPipelineBindPoint pipeline_bind_point,
                            const Location &loc) {
    if (!gpuav.gpuav_settings.shader_instrumentation.post_process_descriptor_index &&
        !gpuav.gpuav_settings.shader_instrumentation.descriptor_checks) {
        return;
    }

    const auto lv_bind_point = ConvertToLvlBindPoint(pipeline_bind_point);
    auto const &last_bound = cb_state.lastBound[lv_bind_point];

    const size_t number_of_sets = last_bound.ds_slots.size();
    if (number_of_sets == 0) {
        return;  // empty bind
    } else if (number_of_sets > glsl::kDebugInputBindlessMaxDescSets) {
        gpuav.InternalError(cb_state.VkHandle(), loc, "Binding more than kDebugInputBindlessMaxDescSets limit");
        return;
    }

    DescriptorCommandBinding descriptor_command_binding(gpuav);
    descriptor_command_binding.bound_descriptor_sets.reserve(number_of_sets);
    // Currently we loop through the sets multiple times to reduce complexity and seperate the various parts, can revisit if we find
    // this is actually a perf bottleneck (assume number of sets are low as people we will then to have a single large set)
    for (uint32_t i = 0; i < number_of_sets; i++) {
        const auto &ds_slot = last_bound.ds_slots[i];
        if (!ds_slot.ds_state) {
            continue;  // can have gaps in descriptor sets
        }
        std::shared_ptr<DescriptorSet> bound_descriptor_set = std::static_pointer_cast<DescriptorSet>(ds_slot.ds_state);
        descriptor_command_binding.bound_descriptor_sets.emplace_back(std::move(bound_descriptor_set));
    }

    UpdateBoundDescriptorsPostProcess(gpuav, cb_state, last_bound, descriptor_command_binding, loc);
    UpdateBoundDescriptorsDescriptorChecks(gpuav, cb_state, last_bound, descriptor_command_binding, loc);

    cb_state.descriptor_command_bindings.emplace_back(std::move(descriptor_command_binding));
}

// For the given command buffer, map its debug data buffers and update the status of any update after bind descriptors
[[nodiscard]] bool UpdateDescriptorStateSSBO(Validator &gpuav, CommandBuffer &cb_state, const Location &loc) {
    const bool need_descriptor_checks = gpuav.gpuav_settings.shader_instrumentation.descriptor_checks;
    if (!need_descriptor_checks) return true;

    for (auto &descriptor_command_binding : cb_state.descriptor_command_bindings) {
        auto ssbo_buffer_ptr = (glsl::DescriptorStateSSBO *)descriptor_command_binding.descritpor_state_ssbo_buffer.MapMemory(loc);
        for (size_t i = 0; i < descriptor_command_binding.bound_descriptor_sets.size(); i++) {
            DescriptorSet &ds_state = *descriptor_command_binding.bound_descriptor_sets[i];
            ssbo_buffer_ptr->descriptor_set_types[i] = ds_state.GetTypeAddress(gpuav, loc);
        }
        descriptor_command_binding.descritpor_state_ssbo_buffer.UnmapMemory();
    }
    return true;
}
}  // namespace descriptor

// After the GPU executed, we know which descriptor indexes were accessed and can validate with normal Core Validation logic
[[nodiscard]] bool CommandBuffer::ValidateBindlessDescriptorSets(const Location &loc) {
    for (uint32_t action_index = 0; action_index < action_command_snapshots.size(); action_index++) {
        const auto &action_command_snapshot = action_command_snapshots[action_index];
        const auto &descriptor_command_binding =
            descriptor_command_bindings[action_command_snapshot.descriptor_command_binding_index];

        // Some applications repeatedly call vkCmdBindDescriptorSets() with the same descriptor sets, avoid checking them multiple
        // times.
        vvl::unordered_set<VkDescriptorSet> validated_desc_sets;

        // TODO - Currently we don't know the actual call that triggered this, but without just giving "vkCmdDraw" we will get
        // VUID_Undefined
        Location draw_loc(vvl::Func::vkCmdDraw);

        // For each descriptor set ...
        for (uint32_t set_index = 0; set_index < descriptor_command_binding.bound_descriptor_sets.size(); set_index++) {
            auto &bound_descriptor_set = descriptor_command_binding.bound_descriptor_sets[set_index];
            if (set_index >= action_command_snapshot.binding_req_maps.size()) {
                // This can occure if binding 2 sets, but then a pipeline layout only uses the first set, so the remaining sets are
                // now not valid to use
                break;
            }
            const BindingVariableMap *binding_req_map = action_command_snapshot.binding_req_maps[set_index];
            if (!binding_req_map) continue;
            if (validated_desc_sets.count(bound_descriptor_set->VkHandle()) > 0) {
                // TODO - If you share two VkDescriptorSet across two different sets in the SPIR-V, we are not going to be
                // validating the 2nd instance of it
                continue;
            }

            if (!bound_descriptor_set->HasPostProcessBuffer()) {
                if (!bound_descriptor_set->CanPostProcess()) {
                    continue;  // hit a dummy object used as a placeholder
                }

                std::stringstream error;
                error << "In CommandBuffer::ValidateBindlessDescriptorSets, action_command_snapshots[" << action_index
                      << "].descriptor_command_binding.bound_descriptor_sets[" << set_index
                      << "].HasPostProcessBuffer() was false. This should not happen. GPU-AV is in a bad state, aborting.";
                auto gpuav = static_cast<Validator *>(&dev_data);
                gpuav->InternalError(gpuav->device, loc, error.str().c_str());
                return false;
            }
            validated_desc_sets.emplace(bound_descriptor_set->VkHandle());

            vvl::DescriptorValidator context(state_, *this, *bound_descriptor_set, set_index, VK_NULL_HANDLE /*framebuffer*/,
                                             draw_loc);

            auto descriptor_accesses = bound_descriptor_set->GetDescriptorAccesses(loc, set_index);
            for (const auto &descriptor_access : descriptor_accesses) {
                auto descriptor_binding = bound_descriptor_set->GetBinding(descriptor_access.binding);
                ASSERT_AND_CONTINUE(descriptor_binding);

                // There is a chance two descriptor bindings are aliased to each other.
                //   layout(set = 0, binding = 2) uniform sampler3D tex3d[];
                //   layout(set = 0, binding = 2) uniform sampler2D tex[];
                // This is where we can use the OpVariable ID provided to map which aliased variable is being used
                const ::spirv::ResourceInterfaceVariable *resource_variable = nullptr;
                for (auto iter = binding_req_map->find(descriptor_access.binding);
                     iter != binding_req_map->end() && iter->first == descriptor_access.binding; ++iter) {
                    if (iter->second.variable->id == descriptor_access.variable_id) {
                        resource_variable = iter->second.variable;
                        break;
                    }
                }

                // This can occur if 2 shaders have different OpVariable, but the pipelines are sharing the same descriptor set
                if (!resource_variable) continue;

                // If we already validated/updated the descriptor on the CPU, don't redo it now in GPU-AV Post Processing
                if (!bound_descriptor_set->ValidateBindingOnGPU(*descriptor_binding,
                                                                resource_variable->is_runtime_descriptor_array)) {
                    continue;
                }

                context.ValidateBindingDynamic(*resource_variable, *descriptor_binding, descriptor_access.index);
            }
        }
    }

    return true;
}
}  // namespace gpuav
