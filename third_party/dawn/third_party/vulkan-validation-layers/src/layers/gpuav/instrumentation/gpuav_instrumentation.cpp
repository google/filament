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

#include "gpuav/instrumentation/gpuav_instrumentation.h"

#include "chassis/chassis_modification_state.h"
#include "gpuav/core/gpuav.h"
#include "gpuav/error_message/gpuav_vuids.h"
#include "gpuav/resources/gpuav_shader_resources.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "gpuav/shaders/gpuav_error_header.h"
#include "gpuav/debug_printf/debug_printf.h"

#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/shader_object_state.h"
#include "state_tracker/shader_module.h"

namespace gpuav {

// If application is using shader objects, bindings count will be computed from bound shaders
static uint32_t LastBoundPipelineOrShaderDescSetBindingsCount(Validator &gpuav, VkPipelineBindPoint bind_point,
                                                              CommandBuffer &cb_state, const LastBound &last_bound) {
    if (last_bound.pipeline_state && last_bound.pipeline_state->PreRasterPipelineLayoutState()) {
        return static_cast<uint32_t>(last_bound.pipeline_state->PreRasterPipelineLayoutState()->set_layouts.size());
    }

    if (const vvl::ShaderObject *main_bound_shader = last_bound.GetFirstShader(bind_point)) {
        return static_cast<uint32_t>(main_bound_shader->set_layouts.size());
    }

    // Should not get there, it would mean no pipeline nor shader object was bound
    assert(false);
    return 0;
}

// If application is using shader objects, bindings count will be computed from bound shaders
static uint32_t LastBoundPipelineOrShaderPushConstantsRangesCount(Validator &gpuav, VkPipelineBindPoint bind_point,
                                                                  CommandBuffer &cb_state, const LastBound &last_bound) {
    if (last_bound.pipeline_state && last_bound.pipeline_state->PreRasterPipelineLayoutState()) {
        return static_cast<uint32_t>(
            last_bound.pipeline_state->PreRasterPipelineLayoutState()->push_constant_ranges_layout->size());
    }

    if (const vvl::ShaderObject *main_bound_shader = last_bound.GetFirstShader(bind_point)) {
        return static_cast<uint32_t>(main_bound_shader->push_constant_ranges->size());
    }

    // Should not get there, it would mean no pipeline nor shader object was bound
    assert(false);
    return 0;
}

static VkPipelineLayout CreateInstrumentationPipelineLayout(Validator &gpuav, VkPipelineBindPoint bind_point, const Location &loc,
                                                            const LastBound &last_bound,
                                                            VkDescriptorSetLayout dummy_desc_set_layout,
                                                            VkDescriptorSetLayout instrumentation_desc_set_layout,
                                                            uint32_t inst_desc_set_binding) {
    // If not using shader objects, GPU-AV should be able to retrieve a pipeline layout from last bound pipeline
    VkPipelineLayoutCreateInfo pipe_layout_ci = vku::InitStructHelper();
    std::shared_ptr<const vvl::PipelineLayout> last_bound_pipeline_pipe_layout;
    if (last_bound.pipeline_state && last_bound.pipeline_state->PreRasterPipelineLayoutState()) {
        last_bound_pipeline_pipe_layout = last_bound.pipeline_state->PreRasterPipelineLayoutState();
    }
    if (last_bound_pipeline_pipe_layout) {
        // Application is using classic pipelines, compose a pipeline layout from last bound pipeline
        // ---
        pipe_layout_ci.flags = last_bound_pipeline_pipe_layout->create_flags;
        std::vector<VkPushConstantRange> ranges;
        if (last_bound_pipeline_pipe_layout->push_constant_ranges_layout) {
            ranges.reserve(last_bound_pipeline_pipe_layout->push_constant_ranges_layout->size());
            for (const VkPushConstantRange &range : *last_bound_pipeline_pipe_layout->push_constant_ranges_layout) {
                ranges.push_back(range);
            }
        }
        pipe_layout_ci.pushConstantRangeCount = static_cast<uint32_t>(ranges.size());
        pipe_layout_ci.pPushConstantRanges = ranges.data();
        std::vector<VkDescriptorSetLayout> set_layouts;
        set_layouts.reserve(inst_desc_set_binding + 1);
        for (const auto &set_layout : last_bound_pipeline_pipe_layout->set_layouts) {
            set_layouts.push_back(set_layout->VkHandle());
        }
        for (uint32_t set_i = static_cast<uint32_t>(last_bound_pipeline_pipe_layout->set_layouts.size());
             set_i < inst_desc_set_binding; ++set_i) {
            set_layouts.push_back(dummy_desc_set_layout);
        }
        set_layouts.push_back(instrumentation_desc_set_layout);
        pipe_layout_ci.setLayoutCount = static_cast<uint32_t>(set_layouts.size());
        pipe_layout_ci.pSetLayouts = set_layouts.data();
        VkPipelineLayout pipe_layout_handle;
        VkResult result = DispatchCreatePipelineLayout(gpuav.device, &pipe_layout_ci, VK_NULL_HANDLE, &pipe_layout_handle);
        if (result != VK_SUCCESS) {
            gpuav.InternalError(gpuav.device, loc, "Failed to create instrumentation pipeline layout");
            return VK_NULL_HANDLE;
        }

        return pipe_layout_handle;
    } else {
        // Application is using shader objects, compose a pipeline layout from bound shaders
        // ---

        const vvl::ShaderObject *main_bound_shader = last_bound.GetFirstShader(bind_point);
        if (!main_bound_shader) {
            // Should not get there, it would mean no pipeline nor shader object was bound
            gpuav.InternalError(gpuav.device, loc, "Could not retrieve last bound computer/vertex/mesh shader");
            return VK_NULL_HANDLE;
        }

        // From those VUIDs:
        // VUID-vkCmdDraw-None-08878
        // - All bound graphics shader objects must have been created with identical or identically defined push constant ranges
        // VUID-vkCmdDraw-None-08879
        // - All bound graphics shader objects must have been created with identical or identically defined arrays of descriptor set
        // layouts
        // => To compose a VkPipelineLayout, only need to get compute or vertex/mesh shader and look at their bindings,
        // no need to check other shaders.
        const vvl::ShaderObject::SetLayoutVector *set_layouts = &main_bound_shader->set_layouts;
        PushConstantRangesId push_constants_layouts = main_bound_shader->push_constant_ranges;

        if (last_bound.desc_set_pipeline_layout) {
            std::shared_ptr<const vvl::PipelineLayout> last_bound_desc_set_pipe_layout =
                gpuav.Get<vvl::PipelineLayout>(last_bound.desc_set_pipeline_layout);
            if (last_bound_desc_set_pipe_layout) {
                pipe_layout_ci.flags = last_bound_desc_set_pipe_layout->CreateFlags();
            }
        }
        std::vector<VkDescriptorSetLayout> set_layout_handles;
        if (set_layouts) {
            set_layout_handles.reserve(inst_desc_set_binding + 1);
            for (const auto &set_layout : *set_layouts) {
                set_layout_handles.push_back(set_layout->VkHandle());
            }
            for (uint32_t set_i = static_cast<uint32_t>(set_layouts->size()); set_i < inst_desc_set_binding; ++set_i) {
                set_layout_handles.push_back(dummy_desc_set_layout);
            }
            set_layout_handles.push_back(instrumentation_desc_set_layout);
            pipe_layout_ci.setLayoutCount = static_cast<uint32_t>(set_layout_handles.size());
            pipe_layout_ci.pSetLayouts = set_layout_handles.data();
        }

        if (push_constants_layouts) {
            pipe_layout_ci.pushConstantRangeCount = static_cast<uint32_t>(push_constants_layouts->size());
            pipe_layout_ci.pPushConstantRanges = push_constants_layouts->data();
        }
        VkPipelineLayout pipe_layout_handle;
        VkResult result = DispatchCreatePipelineLayout(gpuav.device, &pipe_layout_ci, VK_NULL_HANDLE, &pipe_layout_handle);
        if (result != VK_SUCCESS) {
            gpuav.InternalError(gpuav.device, loc, "Failed to create instrumentation pipeline layout");
            return VK_NULL_HANDLE;
        }

        return pipe_layout_handle;
    }
}

// Computes vertex attributes fetching limits based on the set of bound vertex buffers.
// Used to detect out of bounds indices in index buffers.
static std::pair<std::optional<VertexAttributeFetchLimit>, std::optional<VertexAttributeFetchLimit>> GetVertexAttributeFetchLimits(
    const vvl::CommandBuffer &cb_state) {
    const LvlBindPoint lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS);
    const LastBound &last_bound = cb_state.lastBound[lv_bind_point];
    const vvl::Pipeline *pipeline_state = last_bound.pipeline_state;
    const bool use_shader_objects = pipeline_state == nullptr;

    const bool dynamic_vertex_input = use_shader_objects || pipeline_state->IsDynamic(CB_DYNAMIC_STATE_VERTEX_INPUT_EXT);

    const auto &vertex_binding_descriptions =
        dynamic_vertex_input ? cb_state.dynamic_state_value.vertex_bindings : pipeline_state->vertex_input_state->bindings;

    std::optional<VertexAttributeFetchLimit> vertex_attribute_fetch_limit_vertex_input_rate;
    std::optional<VertexAttributeFetchLimit> vertex_attribute_fetch_limit_instance_input_rate;

    vvl::unordered_set<uint32_t> vertex_shader_used_locations;
    {
        const ::spirv::EntryPoint *vertex_entry_point = last_bound.GetVertexEntryPoint();
        if (!vertex_entry_point) {
            return {vertex_attribute_fetch_limit_vertex_input_rate, vertex_attribute_fetch_limit_instance_input_rate};
        }
        for (const ::spirv::StageInterfaceVariable &interface_var : vertex_entry_point->stage_interface_variables) {
            for (const ::spirv::InterfaceSlot &interface_slot : interface_var.interface_slots) {
                vertex_shader_used_locations.insert(interface_slot.Location());
            }
        }
    }

    for (const auto &[binding, vertex_binding_desc] : vertex_binding_descriptions) {
        const vvl::VertexBufferBinding *vbb = vvl::Find(cb_state.current_vertex_buffer_binding_info, binding);
        if (!vbb) {
            // Validation error
            continue;
        }

        for (const auto &[location, attrib] : vertex_binding_desc.locations) {
            if (vertex_shader_used_locations.find(location) == vertex_shader_used_locations.end()) {
                continue;
            }
            const VkDeviceSize attribute_size = GetVertexInputFormatSize(attrib.desc.format);

            const VkDeviceSize stride =
                vbb->stride != 0 ? vbb->stride : attribute_size;  // Tracked stride should already handle all possible value origin

            VkDeviceSize vertex_buffer_remaining_size =
                vbb->effective_size > attrib.desc.offset ? vbb->effective_size - attrib.desc.offset : 0;

            VkDeviceSize vertex_attributes_count = vertex_buffer_remaining_size / stride;
            if (vertex_buffer_remaining_size > vertex_attributes_count * stride) {
                vertex_buffer_remaining_size -= vertex_attributes_count * stride;
            } else {
                vertex_buffer_remaining_size = 0;
            }

            // maybe room for one more attribute but not full stride - not having stride space does not matter for last element
            if (vertex_buffer_remaining_size >= attribute_size) {
                vertex_attributes_count += 1;
            }

            std::optional<VertexAttributeFetchLimit> *vertex_attribute_fetch_limit_ptr =
                (vertex_binding_desc.desc.inputRate == VK_VERTEX_INPUT_RATE_VERTEX)
                    ? &vertex_attribute_fetch_limit_vertex_input_rate
                    : &vertex_attribute_fetch_limit_instance_input_rate;

            if (!vertex_attribute_fetch_limit_ptr->has_value()) {
                *vertex_attribute_fetch_limit_ptr = VertexAttributeFetchLimit{};
            }
            (*vertex_attribute_fetch_limit_ptr)->max_vertex_attributes_count =
                std::min((*vertex_attribute_fetch_limit_ptr)->max_vertex_attributes_count, vertex_attributes_count);
            if ((*vertex_attribute_fetch_limit_ptr)->max_vertex_attributes_count == vertex_attributes_count) {
                (*vertex_attribute_fetch_limit_ptr)->binding_info = *vbb;
                (*vertex_attribute_fetch_limit_ptr)->attribute.location = attrib.desc.location;
                (*vertex_attribute_fetch_limit_ptr)->attribute.binding = attrib.desc.binding;
                (*vertex_attribute_fetch_limit_ptr)->attribute.format = attrib.desc.format;
                (*vertex_attribute_fetch_limit_ptr)->attribute.offset = attrib.desc.offset;
            }
        }
    }
    return {vertex_attribute_fetch_limit_vertex_input_rate, vertex_attribute_fetch_limit_instance_input_rate};
}

void UpdateInstrumentationDescSet(Validator &gpuav, CommandBuffer &cb_state, VkDescriptorSet instrumentation_desc_set,
                                  const Location &loc, InstrumentationErrorBlob &out_instrumentation_error_blob) {
    std::vector<VkWriteDescriptorSet> desc_writes = {};

    // Error output buffer
    VkDescriptorBufferInfo error_output_desc_buffer_info = {};
    {
        error_output_desc_buffer_info.range = VK_WHOLE_SIZE;
        error_output_desc_buffer_info.buffer = cb_state.GetErrorOutputBuffer();
        error_output_desc_buffer_info.offset = 0;

        VkWriteDescriptorSet wds = vku::InitStructHelper();
        wds.dstBinding = glsl::kBindingInstErrorBuffer;
        wds.descriptorCount = 1;
        wds.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        wds.pBufferInfo = &error_output_desc_buffer_info;
        wds.dstSet = instrumentation_desc_set;
        desc_writes.emplace_back(wds);
    }

    // Buffer holding action command index in command buffer
    VkDescriptorBufferInfo indices_desc_buffer_info = {};
    {
        indices_desc_buffer_info.range = sizeof(uint32_t);
        indices_desc_buffer_info.buffer = gpuav.indices_buffer_.VkHandle();
        indices_desc_buffer_info.offset = 0;

        VkWriteDescriptorSet wds = vku::InitStructHelper();
        wds.dstBinding = glsl::kBindingInstActionIndex;
        wds.descriptorCount = 1;
        wds.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        wds.pBufferInfo = &indices_desc_buffer_info;
        wds.dstSet = instrumentation_desc_set;
        desc_writes.emplace_back(wds);
    }

    // Buffer holding a resource index from the per command buffer command resources list
    {
        VkWriteDescriptorSet wds = vku::InitStructHelper();
        wds.dstBinding = glsl::kBindingInstCmdResourceIndex;
        wds.descriptorCount = 1;
        wds.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        wds.pBufferInfo = &indices_desc_buffer_info;
        wds.dstSet = instrumentation_desc_set;
        desc_writes.emplace_back(wds);
    }

    // Errors count buffer
    VkDescriptorBufferInfo cmd_errors_counts_desc_buffer_info = {};
    {
        cmd_errors_counts_desc_buffer_info.range = VK_WHOLE_SIZE;
        cmd_errors_counts_desc_buffer_info.buffer = cb_state.GetCmdErrorsCountsBuffer();
        cmd_errors_counts_desc_buffer_info.offset = 0;

        VkWriteDescriptorSet wds = vku::InitStructHelper();
        wds.dstBinding = glsl::kBindingInstCmdErrorsCount;
        wds.descriptorCount = 1;
        wds.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        wds.pBufferInfo = &cmd_errors_counts_desc_buffer_info;
        wds.dstSet = instrumentation_desc_set;
        desc_writes.emplace_back(wds);
    }

    // Post Processing Output buffer
    VkDescriptorBufferInfo post_process_buffer_info = {};
    if (cb_state.post_process_buffer_lut != VK_NULL_HANDLE) {
        post_process_buffer_info.range = VK_WHOLE_SIZE;
        post_process_buffer_info.buffer = cb_state.post_process_buffer_lut;
        post_process_buffer_info.offset = 0;

        VkWriteDescriptorSet wds = vku::InitStructHelper();
        wds.dstBinding = glsl::kBindingInstPostProcess;
        wds.descriptorCount = 1;
        wds.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        wds.pBufferInfo = &post_process_buffer_info;
        wds.dstSet = instrumentation_desc_set;
        desc_writes.emplace_back(wds);
    }

    // Descriptor Indexing input buffer
    VkDescriptorBufferInfo di_input_desc_buffer_info = {};
    if (cb_state.descriptor_indexing_buffer != VK_NULL_HANDLE) {
        di_input_desc_buffer_info.range = VK_WHOLE_SIZE;
        di_input_desc_buffer_info.buffer = cb_state.descriptor_indexing_buffer;
        di_input_desc_buffer_info.offset = 0;

        VkWriteDescriptorSet wds = vku::InitStructHelper();
        wds.dstBinding = glsl::kBindingInstDescriptorIndexingOOB;
        wds.descriptorCount = 1;
        wds.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        wds.pBufferInfo = &di_input_desc_buffer_info;
        wds.dstSet = instrumentation_desc_set;
        desc_writes.emplace_back(wds);
    }

    // BDA snapshot buffer
    VkDescriptorBufferInfo bda_input_desc_buffer_info = {};
    if (gpuav.gpuav_settings.shader_instrumentation.buffer_device_address) {
        bda_input_desc_buffer_info.range = VK_WHOLE_SIZE;
        bda_input_desc_buffer_info.buffer = cb_state.GetBdaRangesSnapshot().VkHandle();
        bda_input_desc_buffer_info.offset = 0;

        VkWriteDescriptorSet wds = vku::InitStructHelper();
        wds.dstBinding = glsl::kBindingInstBufferDeviceAddress;
        wds.descriptorCount = 1;
        wds.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        wds.pBufferInfo = &bda_input_desc_buffer_info;
        wds.dstSet = instrumentation_desc_set;
        desc_writes.emplace_back(wds);
    }

    // Vertex attribute fetching
    VkDescriptorBufferInfo vertex_attribute_fetch_limits_buffer_bi = {};
    if (gpuav.gpuav_settings.shader_instrumentation.vertex_attribute_fetch_oob) {
        VkBufferCreateInfo buffer_info = vku::InitStructHelper();
        buffer_info.size = 4 * sizeof(uint32_t);
        buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        VmaAllocationCreateInfo alloc_info = {};
        alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        alloc_info.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        vko::Buffer vertex_attribute_fetch_limits_buffer =
            cb_state.gpu_resources_manager.GetManagedBuffer(gpuav, loc, buffer_info, alloc_info);
        if (vertex_attribute_fetch_limits_buffer.IsDestroyed()) {
            return;
        }

        const auto [vertex_attribute_fetch_limit_vertex_input_rate, vertex_attribute_fetch_limit_instance_input_rate] =
            GetVertexAttributeFetchLimits(cb_state);
        auto vertex_attribute_fetch_limits_buffer_ptr = (uint32_t *)vertex_attribute_fetch_limits_buffer.MapMemory(loc);
        if (vertex_attribute_fetch_limit_vertex_input_rate.has_value()) {
            vertex_attribute_fetch_limits_buffer_ptr[0] = 1u;
            vertex_attribute_fetch_limits_buffer_ptr[1] =
                (uint32_t)vertex_attribute_fetch_limit_vertex_input_rate->max_vertex_attributes_count;
        } else {
            vertex_attribute_fetch_limits_buffer_ptr[0] = 0u;
            vertex_attribute_fetch_limits_buffer_ptr[1] = std::numeric_limits<uint32_t>::max();
        }
        if (vertex_attribute_fetch_limit_instance_input_rate.has_value()) {
            vertex_attribute_fetch_limits_buffer_ptr[2] = 1u;
            vertex_attribute_fetch_limits_buffer_ptr[3] =
                (uint32_t)vertex_attribute_fetch_limit_instance_input_rate->max_vertex_attributes_count;
        } else {
            vertex_attribute_fetch_limits_buffer_ptr[2] = 0u;
            vertex_attribute_fetch_limits_buffer_ptr[3] = std::numeric_limits<uint32_t>::max();
        }

        vertex_attribute_fetch_limits_buffer.UnmapMemory();
        out_instrumentation_error_blob.vertex_attribute_fetch_limit_vertex_input_rate =
            vertex_attribute_fetch_limit_vertex_input_rate;
        out_instrumentation_error_blob.vertex_attribute_fetch_limit_instance_input_rate =
            vertex_attribute_fetch_limit_instance_input_rate;
        out_instrumentation_error_blob.index_buffer_binding = cb_state.index_buffer_binding;

        vertex_attribute_fetch_limits_buffer_bi.buffer = vertex_attribute_fetch_limits_buffer.VkHandle();
        vertex_attribute_fetch_limits_buffer_bi.offset = 0;
        vertex_attribute_fetch_limits_buffer_bi.range = VK_WHOLE_SIZE;

        VkWriteDescriptorSet wds = vku::InitStructHelper();
        wds.dstSet = instrumentation_desc_set;
        wds.dstBinding = glsl::kBindingInstVertexAttributeFetchLimits;
        wds.descriptorCount = 1;
        wds.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        wds.pBufferInfo = &vertex_attribute_fetch_limits_buffer_bi;
        desc_writes.emplace_back(wds);
    }

    DispatchUpdateDescriptorSets(gpuav.device, static_cast<uint32_t>(desc_writes.size()), desc_writes.data(), 0, nullptr);
}

void PreCallSetupShaderInstrumentationResources(Validator &gpuav, CommandBuffer &cb_state, VkPipelineBindPoint bind_point,
                                                const Location &loc) {
    if (!gpuav.gpuav_settings.IsSpirvModified()) return;

    assert(bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS || bind_point == VK_PIPELINE_BIND_POINT_COMPUTE ||
           bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);

    const auto lv_bind_point = ConvertToLvlBindPoint(bind_point);
    const LastBound &last_bound = cb_state.lastBound[lv_bind_point];

    // If nothing was updated, we don't want to bind anything
    if (!last_bound.WasInstrumented()) return;

    if (!last_bound.pipeline_state && !last_bound.HasShaderObjects()) {
        gpuav.InternalError(cb_state.VkHandle(), loc, "Neither pipeline state nor shader object states were found.");
        return;
    }

    VkDescriptorSet instrumentation_desc_set =
        cb_state.gpu_resources_manager.GetManagedDescriptorSet(cb_state.GetInstrumentationDescriptorSetLayout());
    if (!instrumentation_desc_set) {
        gpuav.InternalError(cb_state.VkHandle(), loc, "Unable to allocate instrumentation descriptor sets.");
        return;
    }

    // Pathetic way of trying to make sure we take care of updating all
    // bindings of the instrumentation descriptor set
    assert(gpuav.instrumentation_bindings_.size() == 9);

    if (gpuav.gpuav_settings.debug_printf_enabled) {
        if (!debug_printf::UpdateInstrumentationDescSet(gpuav, cb_state, instrumentation_desc_set, bind_point, loc)) {
            // TODO - need cleaner way to indicate if we want to return because of an error or because we want to save from doing
            // unnecessary work
            return;
        }
    }
    InstrumentationErrorBlob instrumentation_error_blob;
    if (gpuav.gpuav_settings.IsShaderInstrumentationEnabled()) {
        UpdateInstrumentationDescSet(gpuav, cb_state, instrumentation_desc_set, loc, instrumentation_error_blob);
    }

    const uint32_t operation_index = (bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS)          ? cb_state.draw_index
                                     : (bind_point == VK_PIPELINE_BIND_POINT_COMPUTE)         ? cb_state.compute_index
                                     : (bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR) ? cb_state.trace_rays_index
                                                                                              : 0;

    const bool uses_shader_object = last_bound.pipeline_state == nullptr;

    // Bind instrumentation descriptor set, using an appropriate pipeline layout
    // ---

    // First find this appropriate pipeline layout.
    // Always try to grab pipeline layout from last bound pipeline. Looking at PreRasterPipelineLayoutState
    // is enough to get the layout whether the application is using standard pipelines or GPL.
    // If GPU-AV failed to get a pipeline layout this way, fall back to pipeline layout specified in last
    // vkCmdBindDescriptorSets, or in last vkCmdPushConstantRanges.

    enum class PipelineLayoutSource { NoPipelineLayout, LastBoundPipeline, LastBoundDescriptorSet, LastPushedConstants };
    std::shared_ptr<const vvl::PipelineLayout> inst_binding_pipe_layout_state;
    PipelineLayoutSource inst_binding_pipe_layout_src = PipelineLayoutSource::NoPipelineLayout;
    if (last_bound.pipeline_state && !last_bound.pipeline_state->PreRasterPipelineLayoutState()->Destroyed()) {
        inst_binding_pipe_layout_state = last_bound.pipeline_state->PreRasterPipelineLayoutState();
        inst_binding_pipe_layout_src = PipelineLayoutSource::LastBoundPipeline;

        // One exception when using GPL is we need to look out for INDEPENDENT_SETS_BIT which will have null sets inside them.
        // We have a fake merged_graphics_layout to mimic the complete layout, but the app must bind it to descriptor set
        if (inst_binding_pipe_layout_state->create_flags & VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT) {
            inst_binding_pipe_layout_state = gpuav.Get<vvl::PipelineLayout>(last_bound.desc_set_pipeline_layout);
            inst_binding_pipe_layout_src = PipelineLayoutSource::LastBoundDescriptorSet;
        }
    } else if (last_bound.desc_set_pipeline_layout) {
        inst_binding_pipe_layout_state = gpuav.Get<vvl::PipelineLayout>(last_bound.desc_set_pipeline_layout);
        inst_binding_pipe_layout_src = PipelineLayoutSource::LastBoundDescriptorSet;
    } else if (cb_state.push_constant_latest_used_layout[lv_bind_point] != VK_NULL_HANDLE) {
        inst_binding_pipe_layout_state = gpuav.Get<vvl::PipelineLayout>(cb_state.push_constant_latest_used_layout[lv_bind_point]);
        inst_binding_pipe_layout_src = PipelineLayoutSource::LastPushedConstants;
    }

    // TODO: Using cb_state.per_command_resources.size() is kind of a hack? Worth considering passing the resource index as a
    // parameter
    const uint32_t error_logger_i = static_cast<uint32_t>(cb_state.per_command_error_loggers.size());
    const std::array<uint32_t, 2> dynamic_offsets = {
        {operation_index * gpuav.indices_buffer_alignment_, error_logger_i * gpuav.indices_buffer_alignment_}};
    if (inst_binding_pipe_layout_state) {
        if ((uint32_t)inst_binding_pipe_layout_state->set_layouts.size() > gpuav.instrumentation_desc_set_bind_index_) {
            gpuav.InternalWarning(cb_state.Handle(), loc,
                                  "Unable to bind instrumentation descriptor set, it would override application's bound set");
            return;
        }

        switch (inst_binding_pipe_layout_src) {
            case PipelineLayoutSource::NoPipelineLayout:
                // should not get there, because inst_desc_set_binding_pipe_layout_state is not null
                assert(false);
                break;
            case PipelineLayoutSource::LastBoundPipeline:
                DispatchCmdBindDescriptorSets(cb_state.VkHandle(), bind_point, inst_binding_pipe_layout_state->VkHandle(),
                                              gpuav.instrumentation_desc_set_bind_index_, 1, &instrumentation_desc_set,
                                              static_cast<uint32_t>(dynamic_offsets.size()), dynamic_offsets.data());
                break;
            case PipelineLayoutSource::LastBoundDescriptorSet:
            case PipelineLayoutSource::LastPushedConstants: {
                // Currently bound pipeline/set of shader objects may have bindings that are not compatible with last
                // bound descriptor sets: GPU-AV may create this incompatibility by adding its empty padding descriptor sets.
                // To alleviate that, since we could not get a pipeline layout from last pipeline binding (it was either
                // destroyed, or never has been created if using shader objects), a pipeline layout matching bindings of last
                // bound pipeline or
                // last bound shader objects is created and used.
                // If will also be cached: heuristic is next action command will likely need the same.

                const uint32_t last_pipe_bindings_count =
                    LastBoundPipelineOrShaderDescSetBindingsCount(gpuav, bind_point, cb_state, last_bound);
                const uint32_t last_pipe_pcr_count =
                    LastBoundPipelineOrShaderPushConstantsRangesCount(gpuav, bind_point, cb_state, last_bound);

                // If the number of binding of the currently bound pipeline's layout (or the equivalent for shader objects) is
                // less that the number of bindings in the pipeline layout used to bind descriptor sets,
                // GPU-AV needs to create a temporary pipeline layout matching the the currently bound pipeline's layout
                // to bind the instrumentation descriptor set
                if (last_pipe_bindings_count < (uint32_t)inst_binding_pipe_layout_state->set_layouts.size() ||
                    last_pipe_pcr_count < (uint32_t)inst_binding_pipe_layout_state->push_constant_ranges_layout->size()) {
                    VkPipelineLayout instrumentation_pipe_layout = CreateInstrumentationPipelineLayout(
                        gpuav, bind_point, loc, last_bound, gpuav.dummy_desc_layout_, gpuav.GetInstrumentationDescriptorSetLayout(),
                        gpuav.instrumentation_desc_set_bind_index_);

                    if (instrumentation_pipe_layout != VK_NULL_HANDLE) {
                        DispatchCmdBindDescriptorSets(cb_state.VkHandle(), bind_point, instrumentation_pipe_layout,
                                                      gpuav.instrumentation_desc_set_bind_index_, 1, &instrumentation_desc_set,
                                                      static_cast<uint32_t>(dynamic_offsets.size()), dynamic_offsets.data());
                        DispatchDestroyPipelineLayout(gpuav.device, instrumentation_pipe_layout, nullptr);
                    } else {
                        // Could not create instrumentation pipeline layout
                        return;
                    }
                } else {
                    // No incompatibility detected, safe to use pipeline layout for last bound descriptor set/push constants.
                    DispatchCmdBindDescriptorSets(cb_state.VkHandle(), bind_point, inst_binding_pipe_layout_state->VkHandle(),
                                                  gpuav.instrumentation_desc_set_bind_index_, 1, &instrumentation_desc_set,
                                                  static_cast<uint32_t>(dynamic_offsets.size()), dynamic_offsets.data());
                }
            } break;
        }

    } else {
        // If no pipeline layout was bound when using shader objects that don't use any descriptor set, and no push constants, bind
        // the instrumentation pipeline layout
        DispatchCmdBindDescriptorSets(cb_state.VkHandle(), bind_point, gpuav.GetInstrumentationPipelineLayout(),
                                      gpuav.instrumentation_desc_set_bind_index_, 1, &instrumentation_desc_set,
                                      static_cast<uint32_t>(dynamic_offsets.size()), dynamic_offsets.data());
    }

    // It is possible to have no descriptor sets bound, for example if using push constants.
    const uint32_t descriptor_binding_index =
        !cb_state.descriptor_command_bindings.empty() ? uint32_t(cb_state.descriptor_command_bindings.size()) - 1 : vvl::kU32Max;

    const bool uses_robustness = (gpuav.enabled_features.robustBufferAccess || gpuav.enabled_features.robustBufferAccess2 ||
                                  (last_bound.pipeline_state && last_bound.pipeline_state->uses_pipeline_robustness));

    const uint32_t last_label_command_i =
        !cb_state.GetLabelCommands().empty() ? uint32_t(cb_state.GetLabelCommands().size() - 1) : vvl::kU32Max;

    CommandBuffer::ErrorLoggerFunc error_logger =
        [loc, descriptor_binding_index, descriptor_binding_list = &cb_state.descriptor_command_bindings, bind_point,
         last_label_command_i, operation_index, uses_shader_object, uses_robustness,
         instrumentation_error_blob](Validator &gpuav, const CommandBuffer &cb_state, const uint32_t *error_record,
                                     const LogObjectList &objlist, const std::vector<std::string> &initial_label_stack) {
            bool skip = false;

            const DescriptorCommandBinding *descriptor_command_binding =
                descriptor_binding_index != vvl::kU32Max ? &(*descriptor_binding_list)[descriptor_binding_index] : nullptr;
            skip |= LogInstrumentationError(gpuav, cb_state, objlist, instrumentation_error_blob, initial_label_stack,
                                            last_label_command_i, operation_index, error_record,
                                            descriptor_command_binding ? descriptor_command_binding->bound_descriptor_sets
                                                                       : std::vector<std::shared_ptr<DescriptorSet>>(),
                                            bind_point, uses_shader_object, uses_robustness, loc);
            return skip;
        };

    cb_state.per_command_error_loggers.emplace_back(error_logger);
}

void PostCallSetupShaderInstrumentationResources(Validator &gpuav, CommandBuffer &cb_state, VkPipelineBindPoint bind_point,
                                                 const Location &loc) {
    if (!gpuav.gpuav_settings.IsSpirvModified()) return;

    assert(bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS || bind_point == VK_PIPELINE_BIND_POINT_COMPUTE ||
           bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);

    const LvlBindPoint lv_bind_point = ConvertToLvlBindPoint(bind_point);
    const LastBound &last_bound = cb_state.lastBound[lv_bind_point];

    // If nothing was updated, we don't want to bind anything
    if (!last_bound.WasInstrumented()) return;

    // Only need to rebind application desc sets if they have been disturbed by GPU-AV binding its instrumentation desc set.
    // - Can happen if the pipeline layout used to bind instrumentation descriptor set is not compatible with the one used by the
    // app to bind the last/all the last desc set. This pipeline layout is referred to as "last_bound_desc_set_pipe_layout_state"
    // hereinafter.
    // => We create this incompatibility when we add our empty descriptor set.
    // See PositiveGpuAVDescriptorIndexing.SharedPipelineLayoutSubsetGraphics for instance
    if (last_bound.desc_set_pipeline_layout) {
        std::shared_ptr<const vvl::PipelineLayout> last_bound_desc_set_pipe_layout_state =
            gpuav.Get<vvl::PipelineLayout>(last_bound.desc_set_pipeline_layout);
        if (last_bound_desc_set_pipe_layout_state) {
            const uint32_t desc_set_bindings_counts_from_last_pipeline =
                LastBoundPipelineOrShaderDescSetBindingsCount(gpuav, bind_point, cb_state, last_bound);

            const bool any_disturbed_desc_sets_bindings =
                desc_set_bindings_counts_from_last_pipeline <
                static_cast<uint32_t>(last_bound_desc_set_pipe_layout_state->set_layouts.size());

            if (any_disturbed_desc_sets_bindings) {
                const uint32_t disturbed_bindings_count = static_cast<uint32_t>(
                    last_bound_desc_set_pipe_layout_state->set_layouts.size() - desc_set_bindings_counts_from_last_pipeline);
                const uint32_t first_disturbed_set = desc_set_bindings_counts_from_last_pipeline;

                for (uint32_t set_i = 0; set_i < disturbed_bindings_count; ++set_i) {
                    const uint32_t last_bound_set_i = set_i + first_disturbed_set;
                    const auto &last_bound_set_state = last_bound.ds_slots[last_bound_set_i].ds_state;
                    // last_bound.ds_slot is a LUT, and descriptor sets before the last one could be unbound.
                    if (!last_bound_set_state) {
                        continue;
                    }
                    VkDescriptorSet last_bound_set = last_bound_set_state->VkHandle();
                    const std::vector<uint32_t> &dynamic_offset = last_bound.ds_slots[last_bound_set_i].dynamic_offsets;
                    const uint32_t dynamic_offset_count = static_cast<uint32_t>(dynamic_offset.size());
                    DispatchCmdBindDescriptorSets(cb_state.VkHandle(), bind_point,
                                                  last_bound_desc_set_pipe_layout_state->VkHandle(), last_bound_set_i, 1,
                                                  &last_bound_set, dynamic_offset_count, dynamic_offset.data());
                }
            }
        }
    }
}

bool LogMessageInstDescriptorIndexingOOB(Validator &gpuav, const uint32_t *error_record, std::string &out_error_msg,
                                         std::string &out_vuid_msg,
                                         const std::vector<std::shared_ptr<DescriptorSet>> &descriptor_sets, const Location &loc,
                                         bool uses_shader_object, bool &out_oob_access) {
    using namespace glsl;
    bool error_found = true;
    std::ostringstream strm;
    const GpuVuid &vuid = GetGpuVuid(loc.function);
    const uint32_t set_num = error_record[kInstDescriptorIndexingDescSetOffset];
    const uint32_t binding_num = error_record[kInstDescriptorIndexingDescBindingOffset];

    const uint32_t descriptor_index = error_record[kInstDescriptorIndexingDescIndexOffset];
    const uint32_t array_length = error_record[kInstDescriptorIndexingParamOffset_0];
    switch (error_record[kHeaderErrorSubCodeOffset]) {
        case kErrorSubCodeDescriptorIndexingBounds: {
            strm << "(set = " << set_num << ", binding = " << binding_num << ") Index of " << descriptor_index
                 << " used to index descriptor array of length " << array_length << ".";
            out_vuid_msg = vuid.descriptor_index_oob_10068;
            error_found = true;
        } break;

        case kErrorSubCodeDescriptorIndexingUninitialized: {
            const auto &dsl = descriptor_sets[set_num]->Layout();
            strm << "(set = " << set_num << ", binding = " << binding_num << ") Descriptor index " << descriptor_index
                 << " is uninitialized.";

            if (descriptor_index == 0 && array_length == 1) {
                strm << " (There is no array, but descriptor is viewed as having an array of length 1)";
            }

            const VkDescriptorBindingFlags binding_flags = dsl.GetDescriptorBindingFlagsFromBinding(binding_num);
            if (binding_flags & VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT) {
                strm << " VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT was used and the original descriptorCount ("
                     << dsl.GetDescriptorCountFromBinding(binding_num)
                     << ") could have been reduced during AllocateDescriptorSets.";
            } else if (gpuav.enabled_features.nullDescriptor) {
                strm << " nullDescriptor feature is on, but vkUpdateDescriptorSets was not called with VK_NULL_HANDLE for this "
                        "descriptor.";
            }

            out_vuid_msg = vuid.invalid_descriptor_08114;
            error_found = true;
        } break;

        case kErrorSubCodeDescriptorIndexingDestroyed: {
            strm << "(set = " << set_num << ", binding = " << binding_num << ") Descriptor index " << descriptor_index
                 << " references a resource that was destroyed.";

            if (descriptor_index == 0 && array_length == 1) {
                strm << " (There is no array, but descriptor is viewed as having an array of length 1)";
            }

            out_vuid_msg = "UNASSIGNED-Descriptor destroyed";
            error_found = true;
        } break;
    }
    out_error_msg += strm.str();
    return error_found;
}

bool LogMessageInstDescriptorClass(Validator &gpuav, const uint32_t *error_record, std::string &out_error_msg,
                                   std::string &out_vuid_msg, const std::vector<std::shared_ptr<DescriptorSet>> &descriptor_sets,
                                   const Location &loc, bool uses_shader_object, bool &out_oob_access) {
    using namespace glsl;
    bool error_found = true;
    out_oob_access = true;
    std::ostringstream strm;
    const GpuVuid &vuid = GetGpuVuid(loc.function);

    const uint32_t set_num = error_record[kInstDescriptorClassDescSetOffset];
    const uint32_t binding_num = error_record[kInstDescriptorClassDescBindingOffset];
    const uint32_t desc_index = error_record[kInstDescriptorClassDescIndexOffset];

    strm << "(set = " << set_num << ", binding = " << binding_num << ", index " << desc_index << ") ";
    switch (error_record[kHeaderErrorSubCodeOffset]) {
        case kErrorSubCodeDescriptorClassGeneralBufferBounds: {
            const auto *binding_state = descriptor_sets[set_num]->GetBinding(binding_num);
            const vvl::Buffer *buffer_state =
                static_cast<const vvl::BufferBinding *>(binding_state)->descriptors[desc_index].GetBufferState();
            if (buffer_state) {
                const uint32_t byte_offset = error_record[kInstDescriptorClassParamOffset_0];
                const uint32_t resource_size = error_record[kInstDescriptorClassParamOffset_1];
                strm << " access out of bounds. The descriptor buffer (" << gpuav.FormatHandle(buffer_state->Handle())
                     << ") size is " << buffer_state->create_info.size << " bytes, " << resource_size
                     << " bytes were bound, and the highest out of bounds access was at [" << byte_offset << "] bytes";
            } else {
                // This will only get called when using nullDescriptor without bindless
                strm << "Trying to access a null descriptor, but vkUpdateDescriptorSets was not called with VK_NULL_HANDLE for "
                        "this descriptor. ";
            }

            if (binding_state->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
                binding_state->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
                out_vuid_msg = uses_shader_object ? vuid.uniform_access_oob_08612 : vuid.uniform_access_oob_06935;
            } else {
                out_vuid_msg = uses_shader_object ? vuid.storage_access_oob_08613 : vuid.storage_access_oob_06936;
            }
        } break;

        case kErrorSubCodeDescriptorClassTexelBufferBounds: {
            const auto *binding_state = descriptor_sets[set_num]->GetBinding(binding_num);
            const vvl::BufferView *buffer_view_state =
                static_cast<const vvl::TexelBinding *>(binding_state)->descriptors[desc_index].GetBufferViewState();
            if (buffer_view_state) {
                const uint32_t byte_offset = error_record[kInstDescriptorClassParamOffset_0];
                const uint32_t resource_size = error_record[kInstDescriptorClassParamOffset_1];

                strm << " access out of bounds. The descriptor texel buffer (" << gpuav.FormatHandle(buffer_view_state->Handle())
                     << ") size is " << resource_size << " texels and the highest out of bounds access was at [" << byte_offset
                     << "] bytes";
            } else {
                // This will only get called when using nullDescriptor without bindless
                strm << "Trying to access a null descriptor, but vkUpdateDescriptorSets was not called with VK_NULL_HANDLE for "
                        "this descriptor. ";
            }

            // https://gitlab.khronos.org/vulkan/vulkan/-/issues/3977
            out_vuid_msg = "UNASSIGNED-Descriptor Texel Buffer texel out of bounds";
        } break;

        default:
            error_found = false;
            out_oob_access = false;
            assert(false);  // other OOB checks are not implemented yet
    }

    out_error_msg += strm.str();
    return error_found;
}

bool LogMessageInstBufferDeviceAddress(const uint32_t *error_record, std::string &out_error_msg, std::string &out_vuid_msg,
                                       bool &out_oob_access) {
    using namespace glsl;
    bool error_found = true;
    std::ostringstream strm;
    switch (error_record[kHeaderErrorSubCodeOffset]) {
        case kErrorSubCodeBufferDeviceAddressUnallocRef: {
            out_oob_access = true;
            const char *access_type = error_record[kInstBuffAddrAccessOpcodeOffset] == spv::OpStore ? "written" : "read";
            uint64_t address = *reinterpret_cast<const uint64_t *>(error_record + kInstBuffAddrUnallocDescPtrLoOffset);
            strm << "Out of bounds access: " << error_record[kInstBuffAddrAccessByteSizeOffset] << " bytes " << access_type
                 << " at buffer device address 0x" << std::hex << address << '.';
            out_vuid_msg = "UNASSIGNED-Device address out of bounds";
        } break;
        case kErrorSubCodeBufferDeviceAddressAlignment: {
            const char *access_type = error_record[kInstBuffAddrAccessOpcodeOffset] == spv::OpStore ? "OpStore" : "OpLoad";
            uint64_t address = *reinterpret_cast<const uint64_t *>(error_record + kInstBuffAddrUnallocDescPtrLoOffset);
            strm << "Unaligned pointer access: The " << access_type << " at buffer device address 0x" << std::hex << address
                 << " is not aligned to the instruction Aligned operand of " << std::dec
                 << error_record[kInstBuffAddrAccessAlignmentOffset] << '.';
            out_vuid_msg = "VUID-RuntimeSpirv-PhysicalStorageBuffer64-06315";
        } break;
        default:
            error_found = false;
            break;
    }
    out_error_msg += strm.str();
    return error_found;
}

bool LogMessageInstRayQuery(const uint32_t *error_record, std::string &out_error_msg, std::string &out_vuid_msg) {
    using namespace glsl;
    bool error_found = true;
    std::ostringstream strm;
    switch (error_record[kHeaderErrorSubCodeOffset]) {
        case kErrorSubCodeRayQueryNegativeMin: {
            // TODO - Figure a way to properly use GLSL floatBitsToUint and print the float values
            strm << "OpRayQueryInitializeKHR operand Ray Tmin value is negative. ";
            out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06349";
        } break;
        case kErrorSubCodeRayQueryNegativeMax: {
            strm << "OpRayQueryInitializeKHR operand Ray Tmax value is negative. ";
            out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06349";
        } break;
        case kErrorSubCodeRayQueryMinMax: {
            strm << "OpRayQueryInitializeKHR operand Ray Tmax is less than RayTmin. ";
            out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06350";
        } break;
        case kErrorSubCodeRayQueryMinNaN: {
            strm << "OpRayQueryInitializeKHR operand Ray Tmin is NaN. ";
            out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06351";
        } break;
        case kErrorSubCodeRayQueryMaxNaN: {
            strm << "OpRayQueryInitializeKHR operand Ray Tmax is NaN. ";
            out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06351";
        } break;
        case kErrorSubCodeRayQueryOriginNaN: {
            strm << "OpRayQueryInitializeKHR operand Ray Origin contains a NaN. ";
            out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06351";
        } break;
        case kErrorSubCodeRayQueryDirectionNaN: {
            strm << "OpRayQueryInitializeKHR operand Ray Direction contains a NaN. ";
            out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06351";
        } break;
        case kErrorSubCodeRayQueryOriginFinite: {
            strm << "OpRayQueryInitializeKHR operand Ray Origin contains a non-finite value. ";
            out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06348";
        } break;
        case kErrorSubCodeRayQueryDirectionFinite: {
            strm << "OpRayQueryInitializeKHR operand Ray Direction contains a non-finite value. ";
            out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06348";
        } break;
        case kErrorSubCodeRayQueryBothSkip: {
            const uint32_t value = error_record[kInstRayQueryParamOffset_0];
            strm << "OpRayQueryInitializeKHR operand Ray Flags is 0x" << std::hex << value << ". ";
            out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06889";
        } break;
        case kErrorSubCodeRayQuerySkipCull: {
            const uint32_t value = error_record[kInstRayQueryParamOffset_0];
            strm << "OpRayQueryInitializeKHR operand Ray Flags is 0x" << std::hex << value << ". ";
            out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06890";
        } break;
        case kErrorSubCodeRayQueryOpaque: {
            const uint32_t value = error_record[kInstRayQueryParamOffset_0];
            strm << "OpRayQueryInitializeKHR operand Ray Flags is 0x" << std::hex << value << ". ";
            out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06891";
        } break;
        default:
            error_found = false;
            break;
    }
    out_error_msg += strm.str();
    return error_found;
}

bool LogMessageInstIndexedDraw(Validator &gpuav, const uint32_t *error_record, std::string &out_error_msg,
                               std::string &out_vuid_msg, const Location &loc, const InstrumentationErrorBlob &inst_error_blob) {
    const uint32_t error_sub_code = error_record[glsl::kHeaderErrorSubCodeOffset];
    if (error_sub_code != glsl::kErrorSubCode_IndexedDraw_OOBVertexIndex &&
        error_sub_code != glsl::kErrorSubCode_IndexedDraw_OOBInstanceIndex) {
        return false;
    }

    switch (loc.function) {
        case vvl::Func::vkCmdDrawIndexed:
            out_vuid_msg = "VUID-vkCmdDrawIndexed-None-02721";
            break;
        case vvl::Func::vkCmdDrawIndexedIndirectCount:
            out_vuid_msg = "VUID-vkCmdDrawIndexedIndirectCount-None-02721";
            break;
        case vvl::Func::vkCmdDrawIndexedIndirect:
            out_vuid_msg = "VUID-vkCmdDrawIndexedIndirect-None-02721";
            break;
// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9323
#if 0
        case vvl::Func::vkCmdDrawMultiIndexedEXT:
            out_vuid_msg = "VUID-vkCmdDrawMultiIndexedEXT-None-02721";
            break;
#endif
        default:
            return false;
    }

    assert(inst_error_blob.vertex_attribute_fetch_limit_vertex_input_rate.has_value() ||
           inst_error_blob.vertex_attribute_fetch_limit_instance_input_rate.has_value());
    assert(inst_error_blob.index_buffer_binding.has_value());

    auto add_vertex_buffer_binding_info = [&gpuav](const VertexAttributeFetchLimit &vertex_attribute_fetch_limit,
                                                   std::string &out) {
        out += "- Buffer: ";
        out += gpuav.FormatHandle(vertex_attribute_fetch_limit.binding_info.buffer);
        out += '\n';
        out += "- Binding: ";
        out += std::to_string(vertex_attribute_fetch_limit.attribute.binding);
        out += '\n';
        out += "- Binding size (effective): ";
        out += std::to_string(vertex_attribute_fetch_limit.binding_info.effective_size);
        out += " bytes\n";
        out += "- Binding offset: ";
        out += std::to_string(vertex_attribute_fetch_limit.binding_info.offset);
        out += " bytes\n";
        out += "- Binding stride: ";
        out += std::to_string(vertex_attribute_fetch_limit.binding_info.stride);
        out += " bytes\n";
        out += "- Vertices count: ";
        out += std::to_string(vertex_attribute_fetch_limit.max_vertex_attributes_count);
        out += '\n';
    };

    auto add_vertex_attribute_info = [](const VertexAttributeFetchLimit &vertex_attribute_fetch_limit, std::string &out) {
        out += "At least the following vertex attribute caused OOB access:\n";
        out += "- Location: ";
        out += std::to_string(vertex_attribute_fetch_limit.attribute.location);
        out += '\n';
        out += "- Binding: ";
        out += std::to_string(vertex_attribute_fetch_limit.attribute.binding);
        out += '\n';
        out += "- Format: ";
        out += string_VkFormat(vertex_attribute_fetch_limit.attribute.format);
        out += '\n';
        out += "- Offset: ";
        out += std::to_string(vertex_attribute_fetch_limit.attribute.offset);
        out += " bytes\n";
    };

    if (error_sub_code == glsl::kErrorSubCode_IndexedDraw_OOBVertexIndex) {
        out_error_msg += "Vertex index ";
        const uint32_t oob_vertex_index = error_record[glsl::kHeaderStageInfoOffset_0];
        out_error_msg += std::to_string(oob_vertex_index);
    } else {
        out_error_msg += "Instance index ";
        const uint32_t oob_instance_index = error_record[glsl::kHeaderStageInfoOffset_1];
        out_error_msg += std::to_string(oob_instance_index);
    }

    out_error_msg += " is not within the smallest bound vertex buffer.\n";

    if (error_sub_code == glsl::kErrorSubCode_IndexedDraw_OOBVertexIndex) {
        out_error_msg += "Smallest vertex buffer binding info, causing OOB access with VK_VERTEX_INPUT_RATE_VERTEX:\n";
        add_vertex_buffer_binding_info(*inst_error_blob.vertex_attribute_fetch_limit_vertex_input_rate, out_error_msg);
        add_vertex_attribute_info(*inst_error_blob.vertex_attribute_fetch_limit_vertex_input_rate, out_error_msg);

    } else {
        out_error_msg += "Smallest vertex buffer binding info, causing OOB access with VK_VERTEX_INPUT_RATE_INSTANCE:\n";
        add_vertex_buffer_binding_info(*inst_error_blob.vertex_attribute_fetch_limit_instance_input_rate, out_error_msg);
        add_vertex_attribute_info(*inst_error_blob.vertex_attribute_fetch_limit_instance_input_rate, out_error_msg);
    }

    if (error_sub_code == glsl::kErrorSubCode_IndexedDraw_OOBVertexIndex) {
        const uint32_t index_bits_size = GetIndexBitsSize(inst_error_blob.index_buffer_binding->index_type);
        const uint32_t max_indices_in_buffer =
            static_cast<uint32_t>(inst_error_blob.index_buffer_binding->size / (index_bits_size / 8u));
        out_error_msg += "Index buffer binding info:\n";
        out_error_msg += "- Buffer: ";
        out_error_msg += gpuav.FormatHandle(inst_error_blob.index_buffer_binding->buffer);
        out_error_msg += '\n';
        out_error_msg += "- Index type: ";
        out_error_msg += string_VkIndexType(inst_error_blob.index_buffer_binding->index_type);
        out_error_msg += '\n';
        out_error_msg += "- Binding offset: ";
        out_error_msg += std::to_string(inst_error_blob.index_buffer_binding->offset);
        out_error_msg += " bytes\n";
        out_error_msg += "- Binding size: ";
        out_error_msg += std::to_string(inst_error_blob.index_buffer_binding->size);
        out_error_msg += " bytes (or ";
        out_error_msg += std::to_string(max_indices_in_buffer);
        out_error_msg += ' ';
        out_error_msg += string_VkIndexType(inst_error_blob.index_buffer_binding->index_type);
        out_error_msg += ")\n";
    }
    out_error_msg +=
        "Note: Vertex buffer binding size is the effective, valid one, based on how the VkBuffer was created and "
        "vertex buffer binding parameters. So it can be clamped up to 0 if binding was invalid.";

    return true;
}

// Pull together all the information from the debug record to build the error message strings,
// and then assemble them into a single message string.
// Retrieve the shader program referenced by the unique shader ID provided in the debug record.
// We had to keep a copy of the shader program with the same lifecycle as the pipeline to make
// sure it is available when the pipeline is submitted.  (The ShaderModule tracking object also
// keeps a copy, but it can be destroyed after the pipeline is created and before it is submitted.)
//
bool LogInstrumentationError(Validator &gpuav, const CommandBuffer &cb_state, const LogObjectList &objlist,
                             const InstrumentationErrorBlob &instrumentation_error_blob,
                             const std::vector<std::string> &initial_label_stack, uint32_t label_command_i,
                             uint32_t operation_index, const uint32_t *error_record,
                             const std::vector<std::shared_ptr<DescriptorSet>> &descriptor_sets,
                             VkPipelineBindPoint pipeline_bind_point, bool uses_shader_object, bool uses_robustness,
                             const Location &loc) {
    // The second word in the debug output buffer is the number of words that would have
    // been written by the shader instrumentation, if there was enough room in the buffer we provided.
    // The number of words actually written by the shaders is determined by the size of the buffer
    // we provide via the descriptor. So, we process only the number of words that can fit in the
    // buffer.
    // Each "report" written by the shader instrumentation is considered a "record". This function
    // is hard-coded to process only one record because it expects the buffer to be large enough to
    // hold only one record. If there is a desire to process more than one record, this function needs
    // to be modified to loop over records and the buffer size increased.

    std::string error_msg;
    std::string vuid_msg;
    bool oob_access = false;
    bool error_found = false;
    switch (error_record[glsl::kHeaderErrorGroupOffset]) {
        case glsl::kErrorGroupInstDescriptorIndexingOOB:
            error_found = LogMessageInstDescriptorIndexingOOB(gpuav, error_record, error_msg, vuid_msg, descriptor_sets, loc,
                                                              uses_shader_object, oob_access);
            break;
        case glsl::kErrorGroupInstDescriptorClass:
            error_found = LogMessageInstDescriptorClass(gpuav, error_record, error_msg, vuid_msg, descriptor_sets, loc,
                                                        uses_shader_object, oob_access);
            break;
        case glsl::kErrorGroupInstBufferDeviceAddress:
            error_found = LogMessageInstBufferDeviceAddress(error_record, error_msg, vuid_msg, oob_access);
            break;
        case glsl::kErrorGroupInstRayQuery:
            error_found = LogMessageInstRayQuery(error_record, error_msg, vuid_msg);
            break;
        case glsl::kErrorGroupInstIndexedDraw:
            error_found = LogMessageInstIndexedDraw(gpuav, error_record, error_msg, vuid_msg, loc, instrumentation_error_blob);
            break;
        default:
            break;
    }

    if (error_found) {
        // Lookup the VkShaderModule handle and SPIR-V code used to create the shader, using the unique shader ID value returned
        // by the instrumented shader.
        const InstrumentedShader *instrumented_shader = nullptr;
        const uint32_t shader_id = error_record[glsl::kHeaderShaderIdOffset];
        auto it = gpuav.instrumented_shaders_map_.find(shader_id);
        if (it != gpuav.instrumented_shaders_map_.end()) {
            instrumented_shader = &it->second;
        }

        std::string debug_region_name = cb_state.GetDebugLabelRegion(label_command_i, initial_label_stack);
        Location loc_with_debug_region(loc, debug_region_name);
        std::string debug_info_message = gpuav.GenerateDebugInfoMessage(
            cb_state.VkHandle(), error_record[gpuav::glsl::kHeaderStageIdOffset],
            error_record[gpuav::glsl::kHeaderStageInfoOffset_0], error_record[gpuav::glsl::kHeaderStageInfoOffset_1],
            error_record[gpuav::glsl::kHeaderStageInfoOffset_2], error_record[gpuav::glsl::kHeaderInstructionIdOffset],
            instrumented_shader, shader_id, pipeline_bind_point, operation_index);

        if (uses_robustness && oob_access) {
            if (gpuav.gpuav_settings.warn_on_robust_oob) {
                gpuav.LogWarning(vuid_msg.c_str(), objlist, loc_with_debug_region, "%s\n%s", error_msg.c_str(),
                                 debug_info_message.c_str());
            }
        } else {
            gpuav.LogError(vuid_msg.c_str(), objlist, loc_with_debug_region, "%s\n%s", error_msg.c_str(),
                           debug_info_message.c_str());
        }
    }

    return error_found;
}

}  // namespace gpuav
