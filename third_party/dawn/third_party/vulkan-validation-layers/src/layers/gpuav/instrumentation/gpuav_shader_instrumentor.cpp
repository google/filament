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

#include "gpuav/instrumentation/gpuav_shader_instrumentor.h"
#include <vulkan/vulkan_core.h>
#include <spirv/unified1/NonSemanticShaderDebugInfo100.h>
#include <spirv/unified1/spirv.hpp>

#include "error_message/error_location.h"
#include "generated/vk_extension_helper.h"
#include "generated/dispatch_functions.h"
#include "gpuav/shaders/gpuav_shaders_constants.h"
#include "chassis/chassis_modification_state.h"
#include "gpuav/shaders/gpuav_error_codes.h"
#include "utils/vk_layer_utils.h"
#include "utils/shader_utils.h"
#include "sync/sync_utils.h"
#include "state_tracker/pipeline_state.h"
#include "error_message/spirv_logging.h"
#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/descriptor_sets.h"
#include "state_tracker/shader_object_state.h"

#include "gpuav/spirv/module.h"
#include "gpuav/spirv/descriptor_indexing_oob_pass.h"
#include "gpuav/spirv/buffer_device_address_pass.h"
#include "gpuav/spirv/descriptor_indexing_oob_pass.h"
#include "gpuav/spirv/descriptor_class_general_buffer_pass.h"
#include "gpuav/spirv/descriptor_class_texel_buffer_pass.h"
#include "gpuav/spirv/ray_query_pass.h"
#include "gpuav/spirv/debug_printf_pass.h"
#include "gpuav/spirv/post_process_descriptor_indexing_pass.h"
#include "gpuav/spirv/vertex_attribute_fetch_oob.h"

#include <cassert>
#include <string>

namespace gpuav {

ReadLockGuard GpuShaderInstrumentor::ReadLock() const {
    if (global_settings.fine_grained_locking) {
        return ReadLockGuard(validation_object_mutex, std::defer_lock);
    } else {
        return ReadLockGuard(validation_object_mutex);
    }
}

WriteLockGuard GpuShaderInstrumentor::WriteLock() {
    if (global_settings.fine_grained_locking) {
        return WriteLockGuard(validation_object_mutex, std::defer_lock);
    } else {
        return WriteLockGuard(validation_object_mutex);
    }
}

// In charge of getting things for shader instrumentation that both GPU-AV and DebugPrintF will need
void GpuShaderInstrumentor::PostCreateDevice(const VkDeviceCreateInfo *pCreateInfo, const Location &loc) {
    BaseClass::PostCreateDevice(pCreateInfo, loc);

    VkPhysicalDeviceFeatures supported_features{};
    DispatchGetPhysicalDeviceFeatures(physical_device, &supported_features);
    if (!supported_features.fragmentStoresAndAtomics) {
        InternalError(
            device, loc,
            "GPU Shader Instrumentation requires fragmentStoresAndAtomics to allow writting out data inside the fragment shader.");
        return;
    }
    if (!supported_features.vertexPipelineStoresAndAtomics) {
        InternalError(device, loc,
                      "GPU Shader Instrumentation requires vertexPipelineStoresAndAtomics to allow writting out data inside the "
                      "vertex shader.");
        return;
    }

    // maxBoundDescriptorSets limit, but possibly adjusted
    const uint32_t adjusted_max_desc_sets_limit =
        std::min(kMaxAdjustedBoundDescriptorSet, phys_dev_props.limits.maxBoundDescriptorSets);
    // If gpu_validation_reserve_binding_slot: the max slot is where we reserved
    // else: always use the last possible set as least likely to be used
    instrumentation_desc_set_bind_index_ = adjusted_max_desc_sets_limit - 1;

    // We can't do anything if there is only one.
    // Device probably not a legit Vulkan device, since there should be at least 4. Protect ourselves.
    if (adjusted_max_desc_sets_limit == 1) {
        InternalError(device, loc, "Device can bind only a single descriptor set.");
        return;
    }

    const VkDescriptorSetLayoutCreateInfo debug_desc_layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0,
                                                                    static_cast<uint32_t>(instrumentation_bindings_.size()),
                                                                    instrumentation_bindings_.data()};

    VkResult result = DispatchCreateDescriptorSetLayout(device, &debug_desc_layout_info, nullptr, &instrumentation_desc_layout_);
    if (result != VK_SUCCESS) {
        InternalError(device, loc, "vkCreateDescriptorSetLayout failed for internal descriptor set");
        Cleanup();
        return;
    }

    const VkDescriptorSetLayoutCreateInfo dummy_desc_layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0,
                                                                    0, nullptr};
    result = DispatchCreateDescriptorSetLayout(device, &dummy_desc_layout_info, nullptr, &dummy_desc_layout_);
    if (result != VK_SUCCESS) {
        InternalError(device, loc, "vkCreateDescriptorSetLayout failed for internal dummy descriptor set");
        Cleanup();
        return;
    }

    std::vector<VkDescriptorSetLayout> debug_layouts;
    for (uint32_t j = 0; j < instrumentation_desc_set_bind_index_; ++j) {
        debug_layouts.push_back(dummy_desc_layout_);
    }
    debug_layouts.push_back(instrumentation_desc_layout_);

    const VkPipelineLayoutCreateInfo debug_pipeline_layout_info = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                                                                   nullptr,
                                                                   0u,
                                                                   static_cast<uint32_t>(debug_layouts.size()),
                                                                   debug_layouts.data(),
                                                                   0u,
                                                                   nullptr};
    result = DispatchCreatePipelineLayout(device, &debug_pipeline_layout_info, nullptr, &instrumentation_pipeline_layout_);
    if (result != VK_SUCCESS) {
        InternalError(device, loc, "vkCreateDescriptorSetLayout failed for internal pipeline layout");
        Cleanup();
        return;
    }
}

void GpuShaderInstrumentor::Cleanup() {
    if (instrumentation_desc_layout_) {
        DispatchDestroyDescriptorSetLayout(device, instrumentation_desc_layout_, nullptr);
        instrumentation_desc_layout_ = VK_NULL_HANDLE;
    }
    if (dummy_desc_layout_) {
        DispatchDestroyDescriptorSetLayout(device, dummy_desc_layout_, nullptr);
        dummy_desc_layout_ = VK_NULL_HANDLE;
    }

    if (instrumentation_pipeline_layout_) {
        DispatchDestroyPipelineLayout(device, instrumentation_pipeline_layout_, nullptr);
        instrumentation_pipeline_layout_ = VK_NULL_HANDLE;
    }
}

void GpuShaderInstrumentor::PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator,
                                                       const RecordObject &record_obj) {
    Cleanup();
    BaseClass::PreCallRecordDestroyDevice(device, pAllocator, record_obj);
}

// Just gives a warning about a possible deadlock.
bool GpuShaderInstrumentor::ValidateCmdWaitEvents(VkCommandBuffer command_buffer, VkPipelineStageFlags2 src_stage_mask,
                                                  const Location &loc) const {
    if (src_stage_mask & VK_PIPELINE_STAGE_2_HOST_BIT) {
        std::ostringstream error_msg;
        error_msg << loc.Message()
                  << " recorded with VK_PIPELINE_STAGE_HOST_BIT set. GPU-Assisted validation waits on queue completion. This wait "
                     "could block the host's signaling of this event, resulting in deadlock.";
        InternalError(command_buffer, loc, error_msg.str().c_str());
    }
    return false;
}

bool GpuShaderInstrumentor::PreCallValidateCmdWaitEvents(
    VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents, VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
    const VkImageMemoryBarrier *pImageMemoryBarriers, const ErrorObject &error_obj) const {
    return ValidateCmdWaitEvents(commandBuffer, static_cast<VkPipelineStageFlags2>(srcStageMask), error_obj.location);
}

bool GpuShaderInstrumentor::PreCallValidateCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount,
                                                             const VkEvent *pEvents, const VkDependencyInfoKHR *pDependencyInfos,
                                                             const ErrorObject &error_obj) const {
    return PreCallValidateCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos, error_obj);
}

bool GpuShaderInstrumentor::PreCallValidateCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount,
                                                          const VkEvent *pEvents, const VkDependencyInfo *pDependencyInfos,
                                                          const ErrorObject &error_obj) const {
    VkPipelineStageFlags2 src_stage_mask = 0;

    for (uint32_t i = 0; i < eventCount; i++) {
        auto stage_masks = sync_utils::GetGlobalStageMasks(pDependencyInfos[i]);
        src_stage_mask |= stage_masks.src;
    }

    return ValidateCmdWaitEvents(commandBuffer, src_stage_mask, error_obj.location);
}

void GpuShaderInstrumentor::PreCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                                              const VkAllocationCallbacks *pAllocator,
                                                              VkPipelineLayout *pPipelineLayout, const RecordObject &record_obj,
                                                              chassis::CreatePipelineLayout &chassis_state) {
    if (gpuav_settings.IsSpirvModified()) {
        if (chassis_state.modified_create_info.setLayoutCount > instrumentation_desc_set_bind_index_) {
            std::ostringstream strm;
            strm << "pCreateInfo::setLayoutCount (" << chassis_state.modified_create_info.setLayoutCount
                 << ") will conflicts with validation's descriptor set at slot " << instrumentation_desc_set_bind_index_ << ". "
                 << "This Pipeline Layout has too many descriptor sets that will not allow GPU shader instrumentation to be setup "
                    "for "
                    "pipelines created with it, therefor no validation error will be repored for them by GPU-AV at "
                    "runtime.";
            InternalWarning(device, record_obj.location, strm.str().c_str());
        } else {
            // Modify the pipeline layout by:
            // 1. Copying the caller's descriptor set desc_layouts
            // 2. Fill in dummy descriptor layouts up to the max binding
            // 3. Fill in with the debug descriptor layout at the max binding slot
            chassis_state.new_layouts.reserve(instrumentation_desc_set_bind_index_ + 1);
            chassis_state.new_layouts.insert(chassis_state.new_layouts.end(), &pCreateInfo->pSetLayouts[0],
                                             &pCreateInfo->pSetLayouts[pCreateInfo->setLayoutCount]);
            for (uint32_t i = pCreateInfo->setLayoutCount; i < instrumentation_desc_set_bind_index_; ++i) {
                chassis_state.new_layouts.push_back(dummy_desc_layout_);
            }
            chassis_state.new_layouts.push_back(instrumentation_desc_layout_);
            chassis_state.modified_create_info.pSetLayouts = chassis_state.new_layouts.data();
            chassis_state.modified_create_info.setLayoutCount = instrumentation_desc_set_bind_index_ + 1;
        }
    }
    BaseClass::PreCallRecordCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout, record_obj, chassis_state);
}

void GpuShaderInstrumentor::PostCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                                               const VkAllocationCallbacks *pAllocator,
                                                               VkPipelineLayout *pPipelineLayout, const RecordObject &record_obj) {
    if (record_obj.result != VK_SUCCESS) {
        InternalError(device, record_obj.location, "Unable to create pipeline layout.");
        return;
    }
    BaseClass::PostCallRecordCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout, record_obj);
}

void GpuShaderInstrumentor::PostCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator, VkShaderModule *pShaderModule,
                                                             const RecordObject &record_obj,
                                                             chassis::CreateShaderModule &chassis_state) {
    BaseClass::PostCallRecordCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule, record_obj, chassis_state);

    // By default, we instrument everything, but if the setting is enabled, we only will instrument the shaders the app picks
    if (gpuav_settings.select_instrumented_shaders && IsSelectiveInstrumentationEnabled(pCreateInfo->pNext)) {
        // If this is being filled up, likely only a few shaders and the app scope is narrowed down, so no need to spend time
        // removing these later
        selected_instrumented_shaders.insert(*pShaderModule);
    };
}

void GpuShaderInstrumentor::PreCallRecordShaderObjectInstrumentation(
    VkShaderCreateInfoEXT &create_info, const Location &create_info_loc,
    chassis::ShaderObjectInstrumentationData &instrumentation_data) {
    if (gpuav_settings.select_instrumented_shaders && !IsSelectiveInstrumentationEnabled(create_info.pNext)) return;

    std::vector<uint32_t> &instrumented_spirv = instrumentation_data.instrumented_spirv;
    InstrumentationDescriptorSetLayouts instrumentation_dsl;
    BuildDescriptorSetLayoutInfo(create_info, instrumentation_dsl);

    const uint32_t unique_shader_id = unique_shader_module_id_++;
    const bool is_shader_instrumented =
        InstrumentShader(vvl::make_span(static_cast<const uint32_t *>(create_info.pCode), create_info.codeSize / sizeof(uint32_t)),
                         unique_shader_id, instrumentation_dsl, create_info_loc, instrumented_spirv);

    if (is_shader_instrumented) {
        instrumentation_data.unique_shader_id = unique_shader_id;
        create_info.pCode = instrumented_spirv.data();
        create_info.codeSize = instrumented_spirv.size() * sizeof(uint32_t);
    }
}

void GpuShaderInstrumentor::PreCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount,
                                                          const VkShaderCreateInfoEXT *pCreateInfos,
                                                          const VkAllocationCallbacks *pAllocator, VkShaderEXT *pShaders,
                                                          const RecordObject &record_obj, chassis::ShaderObject &chassis_state) {
    BaseClass::PreCallRecordCreateShadersEXT(device, createInfoCount, pCreateInfos, pAllocator, pShaders, record_obj,
                                             chassis_state);
    if (!gpuav_settings.IsSpirvModified()) return;

    chassis_state.modified_create_infos.reserve(createInfoCount);

    // Resize here so if using just CoreCheck we don't waste time allocating this
    chassis_state.instrumentations_data.resize(createInfoCount);

    for (uint32_t i = 0; i < createInfoCount; ++i) {
        VkShaderCreateInfoEXT new_create_info = pCreateInfos[i];
        const Location &create_info_loc = record_obj.location.dot(vvl::Field::pCreateInfos, i);
        auto &instrumentation_data = chassis_state.instrumentations_data[i];

        // See pipeline version for explanation
        if (new_create_info.flags & VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT) {
            InternalError(device, create_info_loc,
                          "Unable to instrument shader using VkIndirectExecutionSetEXT validly, things might work, but likely will "
                          "not because of GPU-AV's usage of VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC (If you don't "
                          "need VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT, turn it off).");
        }

        if (new_create_info.setLayoutCount > instrumentation_desc_set_bind_index_) {
            std::ostringstream strm;
            strm << "pCreateInfos[" << i << "]::setLayoutCount (" << new_create_info.setLayoutCount
                 << ") will conflicts with validation's descriptor set at slot " << instrumentation_desc_set_bind_index_ << ". "
                 << "This Shader Object has too many descriptor sets that will not allow GPU shader instrumentation to be setup "
                    "for VkShaderEXT created with it, therefor no validation error will be repored for them by GPU-AV at "
                    "runtime.";
            InternalWarning(device, record_obj.location, strm.str().c_str());
        } else {
            // Modify the pipeline layout by:
            // 1. Copying the caller's descriptor set desc_layouts
            // 2. Fill in dummy descriptor layouts up to the max binding
            // 3. Fill in with the debug descriptor layout at the max binding slot
            instrumentation_data.new_layouts.reserve(instrumentation_desc_set_bind_index_ + 1);
            instrumentation_data.new_layouts.insert(instrumentation_data.new_layouts.end(), pCreateInfos[i].pSetLayouts,
                                                    &pCreateInfos[i].pSetLayouts[pCreateInfos[i].setLayoutCount]);
            for (uint32_t j = pCreateInfos[i].setLayoutCount; j < instrumentation_desc_set_bind_index_; ++j) {
                instrumentation_data.new_layouts.push_back(dummy_desc_layout_);
            }
            instrumentation_data.new_layouts.push_back(instrumentation_desc_layout_);
            new_create_info.pSetLayouts = instrumentation_data.new_layouts.data();
            new_create_info.setLayoutCount = instrumentation_desc_set_bind_index_ + 1;
        }

        PreCallRecordShaderObjectInstrumentation(new_create_info, create_info_loc, instrumentation_data);

        chassis_state.modified_create_infos.emplace_back(std::move(new_create_info));
    }

    chassis_state.pCreateInfos = reinterpret_cast<VkShaderCreateInfoEXT *>(chassis_state.modified_create_infos.data());
}

void GpuShaderInstrumentor::PostCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount,
                                                           const VkShaderCreateInfoEXT *pCreateInfos,
                                                           const VkAllocationCallbacks *pAllocator, VkShaderEXT *pShaders,
                                                           const RecordObject &record_obj, chassis::ShaderObject &chassis_state) {
    BaseClass::PostCallRecordCreateShadersEXT(device, createInfoCount, pCreateInfos, pAllocator, pShaders, record_obj,
                                              chassis_state);
    if (!gpuav_settings.IsSpirvModified()) return;

    for (uint32_t i = 0; i < createInfoCount; ++i) {
        auto &instrumentation_data = chassis_state.instrumentations_data[i];

        // if the shader for some reason was not instrumented, there is nothing to save
        if (!instrumentation_data.IsInstrumented()) {
            continue;
        }
        if (const auto &shader_object_state = Get<vvl::ShaderObject>(pShaders[i])) {
            shader_object_state->instrumentation_data.was_instrumented = true;
            shader_object_state->instrumentation_data.unique_shader_id = instrumentation_data.unique_shader_id;
        }

        instrumented_shaders_map_.insert_or_assign(instrumentation_data.unique_shader_id, VK_NULL_HANDLE, VK_NULL_HANDLE,
                                                   pShaders[i], instrumentation_data.instrumented_spirv);
    }
}

void GpuShaderInstrumentor::PreCallRecordDestroyShaderEXT(VkDevice device, VkShaderEXT shader,
                                                          const VkAllocationCallbacks *pAllocator, const RecordObject &record_obj) {
    if (auto shader_object_state = Get<vvl::ShaderObject>(shader)) {
        instrumented_shaders_map_.pop(shader_object_state->instrumentation_data.unique_shader_id);
    }
    BaseClass::PreCallRecordDestroyShaderEXT(device, shader, pAllocator, record_obj);
}

void GpuShaderInstrumentor::PreCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                 const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                                 const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                                 const RecordObject &record_obj, PipelineStates &pipeline_states,
                                                                 chassis::CreateGraphicsPipelines &chassis_state) {
    BaseClass::PreCallRecordCreateGraphicsPipelines(device, pipelineCache, count, pCreateInfos, pAllocator, pPipelines, record_obj,
                                                    pipeline_states, chassis_state);
    if (!gpuav_settings.IsSpirvModified()) return;

    chassis_state.shader_instrumentations_metadata.resize(count);
    chassis_state.modified_create_infos.resize(count);

    for (uint32_t i = 0; i < count; ++i) {
        const auto &pipeline_state = pipeline_states[i];
        const Location create_info_loc = record_obj.location.dot(vvl::Field::pCreateInfos, i);

        // Need to make a deep copy so if SPIR-V is inlined, user doesn't see it after the call
        auto &new_pipeline_ci = chassis_state.modified_create_infos[i];
        new_pipeline_ci.initialize(&pipeline_state->GraphicsCreateInfo());

        if (!NeedPipelineCreationShaderInstrumentation(*pipeline_state, create_info_loc)) {
            continue;
        }

        auto &shader_instrumentation_metadata = chassis_state.shader_instrumentations_metadata[i];

        bool success = false;
        if (pipeline_state->linking_shaders != 0) {
            success = PreCallRecordPipelineCreationShaderInstrumentationGPL(pAllocator, *pipeline_state, new_pipeline_ci,
                                                                            create_info_loc, shader_instrumentation_metadata);
        } else {
            success = PreCallRecordPipelineCreationShaderInstrumentation(pAllocator, *pipeline_state, new_pipeline_ci,
                                                                         create_info_loc, shader_instrumentation_metadata);
        }
        if (!success) {
            return;
        }
    }

    chassis_state.pCreateInfos = reinterpret_cast<VkGraphicsPipelineCreateInfo *>(chassis_state.modified_create_infos.data());
}

void GpuShaderInstrumentor::PreCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                const VkComputePipelineCreateInfo *pCreateInfos,
                                                                const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                                const RecordObject &record_obj, PipelineStates &pipeline_states,
                                                                chassis::CreateComputePipelines &chassis_state) {
    BaseClass::PreCallRecordCreateComputePipelines(device, pipelineCache, count, pCreateInfos, pAllocator, pPipelines, record_obj,
                                                   pipeline_states, chassis_state);
    if (!gpuav_settings.IsSpirvModified()) return;

    chassis_state.shader_instrumentations_metadata.resize(count);
    chassis_state.modified_create_infos.resize(count);

    for (uint32_t i = 0; i < count; ++i) {
        const auto &pipeline_state = pipeline_states[i];
        const Location create_info_loc = record_obj.location.dot(vvl::Field::pCreateInfos, i);

        // Need to make a deep copy so if SPIR-V is inlined, user doesn't see it after the call
        auto &new_pipeline_ci = chassis_state.modified_create_infos[i];
        new_pipeline_ci.initialize(&pipeline_state->ComputeCreateInfo());

        if (!NeedPipelineCreationShaderInstrumentation(*pipeline_state, create_info_loc)) {
            continue;
        }

        auto &shader_instrumentation_metadata = chassis_state.shader_instrumentations_metadata[i];

        bool success = PreCallRecordPipelineCreationShaderInstrumentation(pAllocator, *pipeline_state, new_pipeline_ci,
                                                                          create_info_loc, shader_instrumentation_metadata);
        if (!success) {
            return;
        }
    }

    chassis_state.pCreateInfos = reinterpret_cast<VkComputePipelineCreateInfo *>(chassis_state.modified_create_infos.data());
}

void GpuShaderInstrumentor::PreCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                     const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
                                                                     const VkAllocationCallbacks *pAllocator,
                                                                     VkPipeline *pPipelines, const RecordObject &record_obj,
                                                                     PipelineStates &pipeline_states,
                                                                     chassis::CreateRayTracingPipelinesNV &chassis_state) {
    BaseClass::PreCallRecordCreateRayTracingPipelinesNV(device, pipelineCache, count, pCreateInfos, pAllocator, pPipelines,
                                                        record_obj, pipeline_states, chassis_state);
    if (!gpuav_settings.IsSpirvModified()) return;

    chassis_state.shader_instrumentations_metadata.resize(count);
    chassis_state.modified_create_infos.resize(count);

    for (uint32_t i = 0; i < count; ++i) {
        const auto &pipeline_state = pipeline_states[i];
        const Location create_info_loc = record_obj.location.dot(vvl::Field::pCreateInfos, i);

        // Need to make a deep copy so if SPIR-V is inlined, user doesn't see it after the call
        auto &new_pipeline_ci = chassis_state.modified_create_infos[i];
        new_pipeline_ci = pipeline_state->RayTracingCreateInfo();  // use copy operation to fight the Common vs NV

        if (!NeedPipelineCreationShaderInstrumentation(*pipeline_state, create_info_loc)) {
            continue;
        }

        auto &shader_instrumentation_metadata = chassis_state.shader_instrumentations_metadata[i];

        bool success = PreCallRecordPipelineCreationShaderInstrumentation(pAllocator, *pipeline_state, new_pipeline_ci,
                                                                          create_info_loc, shader_instrumentation_metadata);
        if (!success) {
            return;
        }
    }

    chassis_state.pCreateInfos = reinterpret_cast<VkRayTracingPipelineCreateInfoNV *>(chassis_state.modified_create_infos.data());
}

void GpuShaderInstrumentor::PreCallRecordCreateRayTracingPipelinesKHR(
    VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t count,
    const VkRayTracingPipelineCreateInfoKHR *pCreateInfos, const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
    const RecordObject &record_obj, PipelineStates &pipeline_states, chassis::CreateRayTracingPipelinesKHR &chassis_state) {
    BaseClass::PreCallRecordCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, count, pCreateInfos, pAllocator,
                                                         pPipelines, record_obj, pipeline_states, chassis_state);
    if (!gpuav_settings.IsSpirvModified()) return;

    chassis_state.shader_instrumentations_metadata.resize(count);
    chassis_state.modified_create_infos.resize(count);

    for (uint32_t i = 0; i < count; ++i) {
        const auto &pipeline_state = pipeline_states[i];
        const Location create_info_loc = record_obj.location.dot(vvl::Field::pCreateInfos, i);

        // Need to make a deep copy so if SPIR-V is inlined, user doesn't see it after the call
        auto &new_pipeline_ci = chassis_state.modified_create_infos[i];
        new_pipeline_ci.initialize(&pipeline_state->RayTracingCreateInfo());

        if (!NeedPipelineCreationShaderInstrumentation(*pipeline_state, create_info_loc)) {
            continue;
        }

        auto &shader_instrumentation_metadata = chassis_state.shader_instrumentations_metadata[i];

        bool success = PreCallRecordPipelineCreationShaderInstrumentation(pAllocator, *pipeline_state, new_pipeline_ci,
                                                                          create_info_loc, shader_instrumentation_metadata);
        if (!success) {
            return;
        }
    }

    chassis_state.pCreateInfos = reinterpret_cast<VkRayTracingPipelineCreateInfoKHR *>(chassis_state.modified_create_infos.data());
}

template <typename CreateInfos, typename SafeCreateInfos>
static void UtilCopyCreatePipelineFeedbackData(CreateInfos &create_info, SafeCreateInfos &safe_create_info) {
    auto src_feedback_struct = vku::FindStructInPNextChain<VkPipelineCreationFeedbackCreateInfo>(safe_create_info.pNext);
    if (!src_feedback_struct) return;
    auto dst_feedback_struct = const_cast<VkPipelineCreationFeedbackCreateInfo *>(
        vku::FindStructInPNextChain<VkPipelineCreationFeedbackCreateInfo>(create_info.pNext));
    *dst_feedback_struct->pPipelineCreationFeedback = *src_feedback_struct->pPipelineCreationFeedback;
    for (uint32_t j = 0; j < src_feedback_struct->pipelineStageCreationFeedbackCount; j++) {
        dst_feedback_struct->pPipelineStageCreationFeedbacks[j] = src_feedback_struct->pPipelineStageCreationFeedbacks[j];
    }
}

void GpuShaderInstrumentor::PostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                  const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                                  const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                                  const RecordObject &record_obj, PipelineStates &pipeline_states,
                                                                  chassis::CreateGraphicsPipelines &chassis_state) {
    BaseClass::PostCallRecordCreateGraphicsPipelines(device, pipelineCache, count, pCreateInfos, pAllocator, pPipelines, record_obj,
                                                     pipeline_states, chassis_state);
    if (!gpuav_settings.IsSpirvModified()) return;
    // VK_PIPELINE_COMPILE_REQUIRED means that the current pipeline creation call was used to poke the driver cache,
    // no pipeline is created in this case
    if (record_obj.result == VK_PIPELINE_COMPILE_REQUIRED) return;

    for (uint32_t i = 0; i < count; ++i) {
        UtilCopyCreatePipelineFeedbackData(pCreateInfos[i], chassis_state.modified_create_infos[i]);

        auto pipeline_state = Get<vvl::Pipeline>(pPipelines[i]);
        ASSERT_AND_CONTINUE(pipeline_state);

        // Move all instrumentation until the final linking time
        if (pipeline_state->create_flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) continue;

        auto &shader_instrumentation_metadata = chassis_state.shader_instrumentations_metadata[i];
        if (pipeline_state->linking_shaders != 0) {
            PostCallRecordPipelineCreationShaderInstrumentationGPL(*pipeline_state, pAllocator, shader_instrumentation_metadata);
        } else {
            PostCallRecordPipelineCreationShaderInstrumentation(*pipeline_state, shader_instrumentation_metadata);
        }
    }
}

void GpuShaderInstrumentor::PostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                 const VkComputePipelineCreateInfo *pCreateInfos,
                                                                 const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                                 const RecordObject &record_obj, PipelineStates &pipeline_states,
                                                                 chassis::CreateComputePipelines &chassis_state) {
    BaseClass::PostCallRecordCreateComputePipelines(device, pipelineCache, count, pCreateInfos, pAllocator, pPipelines, record_obj,
                                                    pipeline_states, chassis_state);
    if (!gpuav_settings.IsSpirvModified()) return;
    // VK_PIPELINE_COMPILE_REQUIRED means that the current pipeline creation call was used to poke the driver cache,
    // no pipeline is created in this case
    if (record_obj.result == VK_PIPELINE_COMPILE_REQUIRED) return;

    for (uint32_t i = 0; i < count; ++i) {
        UtilCopyCreatePipelineFeedbackData(pCreateInfos[i], chassis_state.modified_create_infos[i]);

        auto pipeline_state = Get<vvl::Pipeline>(pPipelines[i]);
        ASSERT_AND_CONTINUE(pipeline_state);
        auto &shader_instrumentation_metadata = chassis_state.shader_instrumentations_metadata[i];
        PostCallRecordPipelineCreationShaderInstrumentation(*pipeline_state, shader_instrumentation_metadata);
    }
}

void GpuShaderInstrumentor::PostCallRecordCreateRayTracingPipelinesNV(
    VkDevice device, VkPipelineCache pipelineCache, uint32_t count, const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
    const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines, const RecordObject &record_obj,
    PipelineStates &pipeline_states, chassis::CreateRayTracingPipelinesNV &chassis_state) {
    BaseClass::PostCallRecordCreateRayTracingPipelinesNV(device, pipelineCache, count, pCreateInfos, pAllocator, pPipelines,
                                                         record_obj, pipeline_states, chassis_state);
    if (!gpuav_settings.IsSpirvModified()) return;
    // VK_PIPELINE_COMPILE_REQUIRED means that the current pipeline creation call was used to poke the driver cache,
    // no pipeline is created in this case
    if (record_obj.result == VK_PIPELINE_COMPILE_REQUIRED) return;

    for (uint32_t i = 0; i < count; ++i) {
        UtilCopyCreatePipelineFeedbackData(pCreateInfos[i], chassis_state.modified_create_infos[i]);

        auto pipeline_state = Get<vvl::Pipeline>(pPipelines[i]);
        ASSERT_AND_CONTINUE(pipeline_state);
        auto &shader_instrumentation_metadata = chassis_state.shader_instrumentations_metadata[i];
        PostCallRecordPipelineCreationShaderInstrumentation(*pipeline_state, shader_instrumentation_metadata);
    }
}

void GpuShaderInstrumentor::PostCallRecordCreateRayTracingPipelinesKHR(
    VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t count,
    const VkRayTracingPipelineCreateInfoKHR *pCreateInfos, const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
    const RecordObject &record_obj, PipelineStates &pipeline_states,
    std::shared_ptr<chassis::CreateRayTracingPipelinesKHR> chassis_state) {
    BaseClass::PostCallRecordCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, count, pCreateInfos, pAllocator,
                                                          pPipelines, record_obj, pipeline_states, chassis_state);
    if (!gpuav_settings.IsSpirvModified()) return;
    // VK_PIPELINE_COMPILE_REQUIRED means that the current pipeline creation call was used to poke the driver cache,
    // no pipeline is created in this case
    if (record_obj.result == VK_PIPELINE_COMPILE_REQUIRED) return;

    const bool is_operation_deferred = deferredOperation != VK_NULL_HANDLE && record_obj.result == VK_OPERATION_DEFERRED_KHR;

    if (is_operation_deferred) {
        for (uint32_t i = 0; i < count; ++i) {
            UtilCopyCreatePipelineFeedbackData(pCreateInfos[i], chassis_state->modified_create_infos[i]);
        }

        if (dispatch_device_->wrap_handles) {
            deferredOperation = dispatch_device_->Unwrap(deferredOperation);
        }

        auto found = dispatch_device_->deferred_operation_post_check.pop(deferredOperation);
        std::vector<std::function<void(const std::vector<VkPipeline> &)>> deferred_op_post_checks;
        if (found->first) {
            deferred_op_post_checks = std::move(found->second);
        } else {
            // vvl::Device::PostCallRecordCreateRayTracingPipelinesKHR should have added a lambda in
            // deferred_operation_post_check for the current deferredOperation.
            // This lambda is responsible for initializing the pipeline state we maintain,
            // this state will be accessed in the following lambda.
            // Given how PostCallRecordCreateRayTracingPipelinesKHR is called in
            // GpuShaderInstrumentor::PostCallRecordCreateRayTracingPipelinesKHR
            // conditions holds as of writing. But it is something we need to be aware of.
            assert(false);
            return;
        }

        deferred_op_post_checks.emplace_back(
            [this, held_chassis_state = chassis_state](const std::vector<VkPipeline> &vk_pipelines) mutable {
                for (size_t i = 0; i < vk_pipelines.size(); ++i) {
                    std::shared_ptr<vvl::Pipeline> pipeline_state =
                        ((GpuShaderInstrumentor *)this)->Get<vvl::Pipeline>(vk_pipelines[i]);
                    ASSERT_AND_CONTINUE(pipeline_state);
                    auto &shader_instrumentation_metadata = held_chassis_state->shader_instrumentations_metadata[i];
                    PostCallRecordPipelineCreationShaderInstrumentation(*pipeline_state, shader_instrumentation_metadata);
                }
            });
        dispatch_device_->deferred_operation_post_check.insert(deferredOperation, std::move(deferred_op_post_checks));
    } else {
        for (uint32_t i = 0; i < count; ++i) {
            UtilCopyCreatePipelineFeedbackData(pCreateInfos[i], chassis_state->modified_create_infos[i]);

            auto pipeline_state = Get<vvl::Pipeline>(pPipelines[i]);

            auto &shader_instrumentation_metadata = chassis_state->shader_instrumentations_metadata[i];
            PostCallRecordPipelineCreationShaderInstrumentation(*pipeline_state, shader_instrumentation_metadata);
        }
    }
}

// Remove all the shader trackers associated with this destroyed pipeline.
void GpuShaderInstrumentor::PreCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline,
                                                         const VkAllocationCallbacks *pAllocator, const RecordObject &record_obj) {
    if (auto pipeline_state = Get<vvl::Pipeline>(pipeline)) {
        for (auto [unique_shader_id, shader_module_handle] : pipeline_state->instrumentation_data.instrumented_shader_modules) {
            instrumented_shaders_map_.pop(unique_shader_id);
            DispatchDestroyShaderModule(device, shader_module_handle, pAllocator);
        }
        if (pipeline_state->instrumentation_data.pre_raster_lib != VK_NULL_HANDLE) {
            DispatchDestroyPipeline(device, pipeline_state->instrumentation_data.pre_raster_lib, pAllocator);
        }
        if (pipeline_state->instrumentation_data.frag_out_lib != VK_NULL_HANDLE) {
            DispatchDestroyPipeline(device, pipeline_state->instrumentation_data.frag_out_lib, pAllocator);
        }
    }

    BaseClass::PreCallRecordDestroyPipeline(device, pipeline, pAllocator, record_obj);
}

template <typename CreateInfo>
VkShaderModule GetShaderModule(const CreateInfo &create_info, VkShaderStageFlagBits stage) {
    for (uint32_t i = 0; i < create_info.stageCount; ++i) {
        if (create_info.pStages[i].stage == stage) {
            return create_info.pStages[i].module;
        }
    }
    return {};
}

template <>
VkShaderModule GetShaderModule(const VkComputePipelineCreateInfo &create_info, VkShaderStageFlagBits) {
    return create_info.stage.module;
}

template <typename SafeType>
void SetShaderModule(SafeType &create_info, const vku::safe_VkPipelineShaderStageCreateInfo &stage_info,
                     VkShaderModule shader_module, uint32_t stage_ci_index) {
    create_info.pStages[stage_ci_index] = stage_info;
    create_info.pStages[stage_ci_index].module = shader_module;
}

template <>
void SetShaderModule(vku::safe_VkComputePipelineCreateInfo &create_info,
                     const vku::safe_VkPipelineShaderStageCreateInfo &stage_info, VkShaderModule shader_module,
                     uint32_t stage_ci_index) {
    assert(stage_ci_index == 0);
    create_info.stage = stage_info;
    create_info.stage.module = shader_module;
}

template <typename CreateInfo, typename StageInfo>
StageInfo &GetShaderStageCI(CreateInfo &ci, VkShaderStageFlagBits stage) {
    static StageInfo null_stage{};
    for (uint32_t i = 0; i < ci.stageCount; ++i) {
        if (ci.pStages[i].stage == stage) {
            return ci.pStages[i];
        }
    }
    return null_stage;
}

template <>
vku::safe_VkPipelineShaderStageCreateInfo &GetShaderStageCI(vku::safe_VkComputePipelineCreateInfo &ci, VkShaderStageFlagBits) {
    return ci.stage;
}

bool GpuShaderInstrumentor::IsSelectiveInstrumentationEnabled(const void *pNext) {
    if (auto features = vku::FindStructInPNextChain<VkValidationFeaturesEXT>(pNext)) {
        for (uint32_t i = 0; i < features->enabledValidationFeatureCount; i++) {
            if (features->pEnabledValidationFeatures[i] == VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT) {
                return true;
            }
        }
    }
    return false;
}

bool GpuShaderInstrumentor::NeedPipelineCreationShaderInstrumentation(vvl::Pipeline &pipeline_state, const Location &loc) {
    // Currently there is a VU (VUID-VkIndirectExecutionSetPipelineInfoEXT-initialPipeline-11019) that prevents
    // VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC in the pipeline layout, but we need it currently for GPU-AV.
    // As a temporary solution, we will just not support people using DGC with IES
    if (pipeline_state.create_flags & VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT) {
        InternalError(device, loc,
                      "Unable to instrument shader using VkIndirectExecutionSetEXT validly, things might work, but likely will not "
                      "because of GPU-AV's usage of VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC (If you don't need "
                      "VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT, turn it off).");
        // don't return false, some drivers seem to not care and app might get away with it
    }

    // will hit with using GPL without shaders in them (ex. fragment output)
    if (pipeline_state.stage_states.empty()) return false;

    // Move all instrumentation until the final linking time
    // This still needs to create a copy of the create_info (we *could* have a mix of GPL and non-GPL)
    if (pipeline_state.create_flags & VK_PIPELINE_CREATE_2_LIBRARY_BIT_KHR) return false;

    // If the app requests all available sets, the pipeline layout was not modified at pipeline layout creation and the
    // already instrumented shaders need to be replaced with uninstrumented shaders
    if (pipeline_state.active_slots.find(instrumentation_desc_set_bind_index_) != pipeline_state.active_slots.end()) {
        return false;
    }
    const auto pipeline_layout = pipeline_state.PipelineLayoutState();
    if (pipeline_layout && pipeline_layout->set_layouts.size() > instrumentation_desc_set_bind_index_) {
        return false;
    }

    return true;
}

void GpuShaderInstrumentor::BuildDescriptorSetLayoutInfo(const vvl::Pipeline &pipeline_state,
                                                         InstrumentationDescriptorSetLayouts &out_instrumentation_dsl) {
    const auto pipeline_layout = pipeline_state.PipelineLayoutState();
    if (!pipeline_layout) return;

    out_instrumentation_dsl.set_index_to_bindings_layout_lut.resize(pipeline_layout->set_layouts.size());
    for (uint32_t set_layout_index = 0; set_layout_index < pipeline_layout->set_layouts.size(); set_layout_index++) {
        if (const auto set_layout_state = pipeline_layout->set_layouts[set_layout_index]) {
            BuildDescriptorSetLayoutInfo(*set_layout_state, set_layout_index, out_instrumentation_dsl);
        }
    }
}

void GpuShaderInstrumentor::BuildDescriptorSetLayoutInfo(const VkShaderCreateInfoEXT &create_info,
                                                         InstrumentationDescriptorSetLayouts &out_instrumentation_dsl) {
    out_instrumentation_dsl.set_index_to_bindings_layout_lut.resize(create_info.setLayoutCount);
    for (const auto [set_layout_index, set_layout] : vvl::enumerate(create_info.pSetLayouts, create_info.setLayoutCount)) {
        if (auto set_layout_state = Get<vvl::DescriptorSetLayout>(set_layout)) {
            BuildDescriptorSetLayoutInfo(*set_layout_state, set_layout_index, out_instrumentation_dsl);
        }
    }
}

void GpuShaderInstrumentor::BuildDescriptorSetLayoutInfo(const vvl::DescriptorSetLayout &set_layout_state,
                                                         const uint32_t set_layout_index,
                                                         InstrumentationDescriptorSetLayouts &out_instrumentation_dsl) {
    if (set_layout_state.GetBindingCount() == 0) return;
    const uint32_t binding_count = set_layout_state.GetMaxBinding() + 1;

    auto &binding_layouts = out_instrumentation_dsl.set_index_to_bindings_layout_lut[set_layout_index];
    binding_layouts.resize(binding_count);

    uint32_t start = 0;
    auto dsl_bindings = set_layout_state.GetBindings();
    for (uint32_t binding_index = 0; binding_index < dsl_bindings.size(); binding_index++) {
        auto &dsl_binding = dsl_bindings[binding_index];
        if (dsl_binding.descriptorType == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK) {
            binding_layouts[dsl_binding.binding] = {start, 1};
            start += 1;
        } else {
            binding_layouts[dsl_binding.binding] = {start, dsl_binding.descriptorCount};
            start += dsl_binding.descriptorCount;
        }

        const VkDescriptorBindingFlags flags = set_layout_state.GetDescriptorBindingFlagsFromBinding(binding_index);
        if (vvl::IsBindless(flags)) {
            out_instrumentation_dsl.has_bindless_descriptors = true;
        }
    }
}

// Instrument all SPIR-V that is sent through pipeline. This can be done in various ways
// 1. VkCreateShaderModule and passed in VkShaderModule.
//    For this we create our own VkShaderModule with instrumented shader and manage it inside the pipeline state
// 2. GPL
//    We defer until linking time, otherwise we will instrument many libraries that might never be used.
//    (this also spreads the compile time cost evenly instead of a huge spike on startup)
// 3. Inlined via VkPipelineShaderStageCreateInfo pNext
//    We just instrument the shader and update the inlined SPIR-V
// 4. VK_EXT_shader_module_identifier
//    We will skip these as we don't know the incoming SPIR-V
// Note: Shader Objects are handled in their own path as they don't use pipelines
template <typename SafeCreateInfo>
bool GpuShaderInstrumentor::PreCallRecordPipelineCreationShaderInstrumentation(
    const VkAllocationCallbacks *pAllocator, vvl::Pipeline &pipeline_state, SafeCreateInfo &modified_pipeline_ci,
    const Location &loc, std::vector<chassis::ShaderInstrumentationMetadata> &shader_instrumentation_metadata) {
    // Init here instead of in chassis so we don't pay cost when GPU-AV is not used
    const size_t total_stages = pipeline_state.stage_states.size();
    shader_instrumentation_metadata.resize(total_stages);

    InstrumentationDescriptorSetLayouts instrumentation_dsl;
    BuildDescriptorSetLayoutInfo(pipeline_state, instrumentation_dsl);

    for (uint32_t stage_state_i = 0; stage_state_i < static_cast<uint32_t>(pipeline_state.stage_states.size()); ++stage_state_i) {
        const auto &stage_state = pipeline_state.stage_states[stage_state_i];
        auto modified_module_state = std::const_pointer_cast<vvl::ShaderModule>(stage_state.module_state);
        ASSERT_AND_CONTINUE(modified_module_state);
        auto &instrumentation_metadata = shader_instrumentation_metadata[stage_state_i];

        // Check pNext for inlined SPIR-V
        // ---
        vku::safe_VkShaderModuleCreateInfo *modified_shader_module_ci = nullptr;
        {
            const VkShaderStageFlagBits stage = stage_state.GetStage();
            auto &stage_ci =
                GetShaderStageCI<SafeCreateInfo, vku::safe_VkPipelineShaderStageCreateInfo>(modified_pipeline_ci, stage);
            modified_shader_module_ci =
                const_cast<vku::safe_VkShaderModuleCreateInfo *>(reinterpret_cast<const vku::safe_VkShaderModuleCreateInfo *>(
                    vku::FindStructInPNextChain<VkShaderModuleCreateInfo>(stage_ci.pNext)));

            if (gpuav_settings.select_instrumented_shaders) {
                if (modified_shader_module_ci && !IsSelectiveInstrumentationEnabled(modified_shader_module_ci->pNext)) {
                    continue;
                } else if (selected_instrumented_shaders.find(modified_module_state->VkHandle()) ==
                           selected_instrumented_shaders.end()) {
                    continue;
                }
            }
        }
        std::vector<uint32_t> instrumented_spirv;
        const uint32_t unique_shader_id = unique_shader_module_id_++;
        const bool is_shader_instrumented =
            InstrumentShader(modified_module_state->spirv->words_, unique_shader_id, instrumentation_dsl, loc, instrumented_spirv);
        if (is_shader_instrumented) {
            instrumentation_metadata.unique_shader_id = unique_shader_id;
            if (modified_module_state->VkHandle() != VK_NULL_HANDLE) {
                // If the user used vkCreateShaderModule, we create a new VkShaderModule to replace with the instrumented
                // shader
                VkShaderModuleCreateInfo instrumented_shader_module_ci = vku::InitStructHelper();
                instrumented_shader_module_ci.pCode = instrumented_spirv.data();
                instrumented_shader_module_ci.codeSize = instrumented_spirv.size() * sizeof(uint32_t);
                VkShaderModule instrumented_shader_module = VK_NULL_HANDLE;
                VkResult result =
                    DispatchCreateShaderModule(device, &instrumented_shader_module_ci, pAllocator, &instrumented_shader_module);
                if (result == VK_SUCCESS) {
                    SetShaderModule(modified_pipeline_ci, *stage_state.pipeline_create_info, instrumented_shader_module,
                                    stage_state_i);

                    pipeline_state.instrumentation_data.instrumented_shader_modules.emplace_back(
                        std::pair<uint32_t, VkShaderModule>{unique_shader_id, instrumented_shader_module});
                } else {
                    InternalError(device, loc, "Unable to replace non-instrumented shader with instrumented one.");
                    return false;
                }
            } else if (modified_shader_module_ci) {
                // The user is inlining the Shader Module into the pipeline, so just need to update the spirv
                instrumentation_metadata.passed_in_shader_stage_ci = true;
                // TODO - This makes a copy, but could save on Chassis stack instead (then remove function from VUL).
                // The core issue is we always use std::vector<uint32_t> but Safe Struct manages its own version of the pCode
                // memory. It would be much harder to change everything from std::vector and instead to adjust Safe Struct to not
                // double-free the memory on us. If making any changes, we have to consider a case where the user inlines the
                // fragment shader, but use a normal VkShaderModule in the vertex shader.
                modified_shader_module_ci->SetCode(instrumented_spirv);
            } else {
                assert(false);
                return false;
            }
        }
    }
    return true;
}

// Now that we have created the pipeline (and have its handle) build up the shader map for each shader we instrumented
void GpuShaderInstrumentor::PostCallRecordPipelineCreationShaderInstrumentation(
    vvl::Pipeline &pipeline_state, std::vector<chassis::ShaderInstrumentationMetadata> &shader_instrumentation_metadata) {
    // if we return early from NeedPipelineCreationShaderInstrumentation, will need to skip at this point in PostCall
    if (shader_instrumentation_metadata.empty()) return;

    for (uint32_t stage_state_i = 0; stage_state_i < static_cast<uint32_t>(pipeline_state.stage_states.size()); ++stage_state_i) {
        auto &instrumentation_metadata = shader_instrumentation_metadata[stage_state_i];

        // if the shader for some reason was not instrumented, there is nothing to save
        if (!instrumentation_metadata.IsInstrumented()) {
            continue;
        }
        pipeline_state.instrumentation_data.was_instrumented = true;

        const auto &stage_state = pipeline_state.stage_states[stage_state_i];
        auto &module_state = stage_state.module_state;

        // We currently need to store a copy of the original, non-instrumented shader so if there is debug information,
        // we can reference it by the instruction number printed out in the shader. Since the application can destroy the
        // original VkShaderModule, there is a chance this will be gone, we need to copy it now.
        // TODO - in the instrumentation, instead of printing the instruction number only, if we print out debug info, we
        // can remove this copy
        std::vector<uint32_t> code;
        if (module_state && module_state->spirv) code = module_state->spirv->words_;

        VkShaderModule shader_module_handle = module_state->VkHandle();
        if (shader_module_handle == VK_NULL_HANDLE && instrumentation_metadata.passed_in_shader_stage_ci) {
            shader_module_handle = kPipelineStageInfoHandle;
        }

        instrumented_shaders_map_.insert_or_assign(instrumentation_metadata.unique_shader_id, pipeline_state.VkHandle(),
                                                   shader_module_handle, VK_NULL_HANDLE, std::move(code));
    }
}

// While have an almost duplicated function is not ideal, the core issue is we have a single, templated function designed for
// Graphics, Compute, and Ray Tracing. GPL is only for graphics, so we end up needing this "side code path" for graphics only and it
// doesn't fit in the "all pipeline" templated flow.
bool GpuShaderInstrumentor::PreCallRecordPipelineCreationShaderInstrumentationGPL(
    const VkAllocationCallbacks *pAllocator, vvl::Pipeline &pipeline_state,
    vku::safe_VkGraphicsPipelineCreateInfo &modified_pipeline_ci, const Location &loc,
    std::vector<chassis::ShaderInstrumentationMetadata> &shader_instrumentation_metadata) {
    // Init here instead of in chassis so we don't pay cost when GPU-AV is not used
    const size_t total_stages = pipeline_state.stage_states.size();
    shader_instrumentation_metadata.resize(total_stages);

    InstrumentationDescriptorSetLayouts instrumentation_dsl;
    BuildDescriptorSetLayoutInfo(pipeline_state, instrumentation_dsl);

    auto modified_pipeline_lib_ci = const_cast<VkPipelineLibraryCreateInfoKHR *>(
        vku::FindStructInPNextChain<VkPipelineLibraryCreateInfoKHR>(modified_pipeline_ci.pNext));

    // the "pStages[]" is spread across libraries, so build it up in the double for loop
    uint32_t shader_i = 0;

    // This outer loop is the main difference between the GPL and non-GPL version and why its hard to merge them
    for (uint32_t modified_lib_i = 0; modified_lib_i < modified_pipeline_lib_ci->libraryCount; ++modified_lib_i) {
        const auto modified_lib = Get<vvl::Pipeline>(modified_pipeline_lib_ci->pLibraries[modified_lib_i]);
        if (!modified_lib) continue;
        if (modified_lib->stage_states.empty()) continue;

        vku::safe_VkGraphicsPipelineCreateInfo modified_pipeline_ci(modified_lib->GraphicsCreateInfo());
        // If the application supplied pipeline might be interested in failing to be created
        // if the driver does not find it in its cache, GPU-AV needs to succeed in the instrumented pipeline library
        // creation process no matter caching state.
        modified_pipeline_ci.flags &= ~VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT;

        for (uint32_t stage_state_i = 0; stage_state_i < static_cast<uint32_t>(modified_lib->stage_states.size());
             ++stage_state_i) {
            const ShaderStageState &modified_stage_state = modified_lib->stage_states[stage_state_i];
            auto modified_module_state = std::const_pointer_cast<vvl::ShaderModule>(modified_stage_state.module_state);
            ASSERT_AND_CONTINUE(modified_module_state);
            chassis::ShaderInstrumentationMetadata &instrumentation_metadata = shader_instrumentation_metadata[shader_i++];

            // Check pNext for inlined SPIR-V
            // ---
            vku::safe_VkShaderModuleCreateInfo *modified_shader_module_ci = nullptr;
            {
                vku::safe_VkPipelineShaderStageCreateInfo *modified_stage_ci = nullptr;
                const VkShaderStageFlagBits stage = modified_stage_state.GetStage();
                for (uint32_t i = 0; i < modified_pipeline_ci.stageCount; ++i) {
                    if (modified_pipeline_ci.pStages[i].stage == stage) {
                        modified_stage_ci = &modified_pipeline_ci.pStages[i];
                    }
                }
                assert(modified_stage_ci);

                modified_shader_module_ci =
                    const_cast<vku::safe_VkShaderModuleCreateInfo *>(reinterpret_cast<const vku::safe_VkShaderModuleCreateInfo *>(
                        vku::FindStructInPNextChain<VkShaderModuleCreateInfo>(modified_stage_ci->pNext)));

                if (gpuav_settings.select_instrumented_shaders) {
                    if (modified_shader_module_ci && !IsSelectiveInstrumentationEnabled(modified_shader_module_ci->pNext)) {
                        continue;
                    } else if (selected_instrumented_shaders.find(modified_module_state->VkHandle()) ==
                               selected_instrumented_shaders.end()) {
                        continue;
                    }
                }
            }

            // Instrument shader
            // ---
            std::vector<uint32_t> instrumented_spirv;
            const uint32_t unique_shader_id = unique_shader_module_id_++;
            const bool is_shader_instrumented = InstrumentShader(modified_module_state->spirv->words_, unique_shader_id,
                                                                 instrumentation_dsl, loc, instrumented_spirv);
            if (is_shader_instrumented) {
                instrumentation_metadata.unique_shader_id = unique_shader_id;
                if (modified_module_state->VkHandle() != VK_NULL_HANDLE) {
                    // If the user used vkCreateShaderModule, we create a new VkShaderModule to replace with the instrumented
                    // shader
                    VkShaderModule instrumented_shader_module;
                    VkShaderModuleCreateInfo create_info = vku::InitStructHelper();
                    create_info.pCode = instrumented_spirv.data();
                    create_info.codeSize = instrumented_spirv.size() * sizeof(uint32_t);
                    VkResult result = DispatchCreateShaderModule(device, &create_info, pAllocator, &instrumented_shader_module);
                    if (result == VK_SUCCESS) {
                        modified_pipeline_ci.pStages[stage_state_i] = *modified_stage_state.pipeline_create_info;
                        modified_pipeline_ci.pStages[stage_state_i].module = instrumented_shader_module;

                        modified_lib->instrumentation_data.instrumented_shader_modules.emplace_back(
                            std::pair<uint32_t, VkShaderModule>{unique_shader_id, instrumented_shader_module});
                    } else {
                        InternalError(device, loc, "Unable to replace non-instrumented shader with instrumented one.");
                        return false;
                    }
                } else if (modified_shader_module_ci) {
                    // The user is inlining the Shader Module into the pipeline, so just need to update the spirv
                    instrumentation_metadata.passed_in_shader_stage_ci = true;
                    // TODO - This makes a copy, but could save on Chassis stack instead (then remove function from VUL).
                    // The core issue is we always use std::vector<uint32_t> but Safe Struct manages its own version of the pCode
                    // memory. It would be much harder to change everything from std::vector and instead to adjust Safe Struct to
                    // not double-free the memory on us. If making any changes, we have to consider a case where the user inlines
                    // the fragment shader, but use a normal VkShaderModule in the vertex shader.
                    modified_shader_module_ci->SetCode(instrumented_spirv);
                } else {
                    assert(false);
                    return false;
                }
            }
        }

        // Create instrumented pipeline library
        // ---
        VkPipeline instrumented_pipeline_lib = VK_NULL_HANDLE;
        const VkResult result = DispatchCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, modified_pipeline_ci.ptr(), pAllocator,
                                                                &instrumented_pipeline_lib);
        if (result != VK_SUCCESS || instrumented_pipeline_lib == VK_NULL_HANDLE) {
            InternalError(device, loc, "Failed to create instrumentde pipeline library.");
            return false;
        }

        if (modified_lib->active_shaders & VK_SHADER_STAGE_FRAGMENT_BIT) {
            pipeline_state.instrumentation_data.frag_out_lib = instrumented_pipeline_lib;
        } else {
            pipeline_state.instrumentation_data.pre_raster_lib = instrumented_pipeline_lib;
        }

        const_cast<VkPipeline *>(modified_pipeline_lib_ci->pLibraries)[modified_lib_i] = instrumented_pipeline_lib;
    }
    return true;
}

void GpuShaderInstrumentor::PostCallRecordPipelineCreationShaderInstrumentationGPL(
    vvl::Pipeline &pipeline_state, const VkAllocationCallbacks *pAllocator,
    std::vector<chassis::ShaderInstrumentationMetadata> &shader_instrumentation_metadata) {
    // if we return early from NeedPipelineCreationShaderInstrumentation, will need to skip at this point in PostCall
    if (shader_instrumentation_metadata.empty()) return;

    uint32_t shader_index = 0;
    // This outer loop is the main difference between the GPL and non-GPL version and why its hard to merge them
    for (uint32_t library_i = 0; library_i < pipeline_state.library_create_info->libraryCount; ++library_i) {
        const auto lib = Get<vvl::Pipeline>(pipeline_state.library_create_info->pLibraries[library_i]);
        if (!lib) continue;
        if (lib->stage_states.empty()) continue;

        vku::safe_VkGraphicsPipelineCreateInfo new_lib_pipeline_ci(lib->GraphicsCreateInfo());

        for (uint32_t stage_state_i = 0; stage_state_i < static_cast<uint32_t>(lib->stage_states.size()); ++stage_state_i) {
            auto &instrumentation_metadata = shader_instrumentation_metadata[shader_index++];

            // if the shader for some reason was not instrumented, there is nothing to save
            if (!instrumentation_metadata.IsInstrumented()) continue;

            pipeline_state.instrumentation_data.was_instrumented = true;

            const auto &stage_state = lib->stage_states[stage_state_i];
            auto &module_state = stage_state.module_state;

            // We currently need to store a copy of the original, non-instrumented shader so if there is debug information,
            // we can reference it by the instruction number printed out in the shader. Since the application can destroy the
            // original VkShaderModule, there is a chance this will be gone, we need to copy it now.
            // TODO - in the instrumentation, instead of printing the instruction number only, if we print out debug info, we
            // can remove this copy
            std::vector<uint32_t> code;
            if (module_state && module_state->spirv) code = module_state->spirv->words_;

            VkShaderModule shader_module_handle = module_state->VkHandle();
            if (shader_module_handle == VK_NULL_HANDLE && instrumentation_metadata.passed_in_shader_stage_ci) {
                shader_module_handle = kPipelineStageInfoHandle;
            }

            instrumented_shaders_map_.insert_or_assign(instrumentation_metadata.unique_shader_id, lib->VkHandle(),
                                                       shader_module_handle, VK_NULL_HANDLE, std::move(code));
        }
    }
}

static bool GpuValidateShader(const std::vector<uint32_t> &input, bool SetRelaxBlockLayout, bool SetScalarBlockLayout,
                              spv_target_env target_env, std::string &error) {
    // Use SPIRV-Tools validator to try and catch any issues with the module
    spv_context ctx = spvContextCreate(target_env);
    spv_const_binary_t binary{input.data(), input.size()};
    spv_diagnostic diag = nullptr;
    spv_validator_options options = spvValidatorOptionsCreate();
    spvValidatorOptionsSetRelaxBlockLayout(options, SetRelaxBlockLayout);
    spvValidatorOptionsSetScalarBlockLayout(options, SetScalarBlockLayout);
    spv_result_t result = spvValidateWithOptions(ctx, options, &binary, &diag);
    if (result != SPV_SUCCESS && diag) error = diag->error;
    return (result == SPV_SUCCESS);
}

// Call the SPIR-V Optimizer to run the instrumentation pass on the shader.
bool GpuShaderInstrumentor::InstrumentShader(const vvl::span<const uint32_t> &input_spirv, uint32_t unique_shader_id,
                                             const InstrumentationDescriptorSetLayouts &instrumentation_dsl, const Location &loc,
                                             std::vector<uint32_t> &out_instrumented_spirv) {
    if (input_spirv[0] != spv::MagicNumber) return false;

    if (gpuav_settings.debug_dump_instrumented_shaders) {
        std::string file_name = "dump_" + std::to_string(unique_shader_id) + "_before.spv";
        DumpSpirvToFile(file_name, reinterpret_cast<const char *>(input_spirv.data()), input_spirv.size() * sizeof(uint32_t));
    }

    spirv::Settings module_settings{};
    // Use the unique_shader_id as a shader ID so we can look up its handle later in the shader_map.
    module_settings.shader_id = unique_shader_id;
    module_settings.output_buffer_descriptor_set = instrumentation_desc_set_bind_index_;
    module_settings.print_debug_info = gpuav_settings.debug_print_instrumentation_info;
    module_settings.max_instrumentations_count = gpuav_settings.debug_max_instrumentations_count;
    module_settings.support_non_semantic_info = IsExtEnabled(extensions.vk_khr_shader_non_semantic_info);
    module_settings.has_bindless_descriptors = instrumentation_dsl.has_bindless_descriptors;

    spirv::Module module(input_spirv, debug_report, module_settings, enabled_features,
                         instrumentation_dsl.set_index_to_bindings_layout_lut);

    bool modified = false;

    // If descriptor indexing is enabled, enable length checks and updated descriptor checks
    if (gpuav_settings.shader_instrumentation.descriptor_checks) {
        // Will wrap descriptor indexing with if/else to prevent crashing if OOB
        spirv::DescriptorIndexingOOBPass oob_pass(module);
        modified |= oob_pass.Run();

        // Depending on the DescriptorClass, will add dedicated check
        spirv::DescriptorClassGeneralBufferPass general_buffer_pass(module);
        modified |= general_buffer_pass.Run();
        spirv::DescriptorClassTexelBufferPass texel_buffer_pass(module);
        modified |= texel_buffer_pass.Run();
    }

    if (gpuav_settings.shader_instrumentation.buffer_device_address) {
        spirv::BufferDeviceAddressPass pass(module);
        modified |= pass.Run();
    }

    if (gpuav_settings.shader_instrumentation.ray_query) {
        spirv::RayQueryPass pass(module);
        modified |= pass.Run();
    }

    // Post Process instrumentation passes assume the things inside are valid, but putting at the end, things above will wrap checks
    // in a if/else, this means they will be gaurded as if they were inside the above passes
    if (gpuav_settings.shader_instrumentation.post_process_descriptor_index) {
        spirv::PostProcessDescriptorIndexingPass pass(module);
        modified |= pass.Run();
    }

    if (gpuav_settings.shader_instrumentation.vertex_attribute_fetch_oob) {
        spirv::VertexAttributeFetchOob pass(module);
        modified |= pass.Run();
    }

    // If there were GLSL written function injected, we will grab them and link them in here
    for (const auto &info : module.link_info_) {
        module.LinkFunction(info);
    }

    // DebugPrintf goes at the end for 2 reasons:
    // 1. We use buffer device address in it and we don't want to validate the inside of this pass
    // 2. We might want to debug the above passes and want to inject our own debug printf calls
    if (gpuav_settings.debug_printf_enabled) {
        // binding slot allows debug printf to be slotted in the same set as GPU-AV if needed
        spirv::DebugPrintfPass pass(module, intenral_only_debug_printf_, glsl::kBindingInstDebugPrintf);
        modified |= pass.Run();
    }

    // If nothing was instrumented, leave early to save time
    if (!modified) {
        return false;
    }

    // some small cleanup to make sure SPIR-V is legal
    module.PostProcess();
    // translate internal representation of SPIR-V into legal SPIR-V binary
    module.ToBinary(out_instrumented_spirv);

    if (gpuav_settings.debug_dump_instrumented_shaders) {
        std::string file_name = "dump_" + std::to_string(unique_shader_id) + "_after.spv";
        DumpSpirvToFile(file_name, reinterpret_cast<const char *>(out_instrumented_spirv.data()),
                        out_instrumented_spirv.size() * sizeof(uint32_t));
    }

    spv_target_env target_env = PickSpirvEnv(api_version, IsExtEnabled(extensions.vk_khr_spirv_1_4));
    // (Maybe) validate the instrumented and linked shader
    if (gpuav_settings.debug_validate_instrumented_shaders) {
        std::string spirv_val_error;
        if (!GpuValidateShader(out_instrumented_spirv, extensions.vk_khr_relaxed_block_layout,
                               extensions.vk_ext_scalar_block_layout, target_env, spirv_val_error)) {
            std::ostringstream strm;
            strm << "Instrumented shader (id " << unique_shader_id << ") is invalid, spirv-val error:\n"
                 << spirv_val_error << " Proceeding with non instrumented shader.";
            InternalError(device, loc, strm.str().c_str());
            return false;
        }
    }

    return true;
}

void GpuShaderInstrumentor::InternalError(LogObjectList objlist, const Location &loc, const char *const specific_message) const {
    aborted_ = true;
    std::string error_message = specific_message;

    char const *layer_name = gpuav_settings.debug_printf_only ? "DebugPrintf" : "GPU-AV";
    char const *vuid = gpuav_settings.debug_printf_only ? "UNASSIGNED-DEBUG-PRINTF" : "UNASSIGNED-GPU-Assisted-Validation";

    LogError(vuid, objlist, loc, "Internal Error, %s is being disabled. Details:\n%s", layer_name, error_message.c_str());

    // Once we encounter an internal issue disconnect everything.
    // This prevents need to check "if (aborted)" (which is awful when we easily forget to check somewhere and the user gets spammed
    // with errors making it hard to see the first error with the real source of the problem).
    dispatch_device_->ReleaseValidationObject(LayerObjectTypeGpuAssisted);
}

void GpuShaderInstrumentor::InternalWarning(LogObjectList objlist, const Location &loc, const char *const specific_message) const {
    char const *vuid = gpuav_settings.debug_printf_only ? "WARNING-DEBUG-PRINTF" : "WARNING-GPU-Assisted-Validation";
    LogWarning(vuid, objlist, loc, "Internal Warning: %s", specific_message);
}

void GpuShaderInstrumentor::InternalInfo(LogObjectList objlist, const Location &loc, const char *const specific_message) const {
    char const *vuid = gpuav_settings.debug_printf_only ? "INFO-DEBUG-PRINTF" : "INFO-GPU-Assisted-Validation";
    LogInfo(vuid, objlist, loc, "Internal Info: %s", specific_message);
}

// The lock (debug_output_mutex) is held by the caller,
// because the latter has code paths that make multiple calls of this function,
// and all such calls have to access the same debug reporting state to ensure consistency of output information.
static std::string LookupDebugUtilsNameNoLock(const DebugReport *debug_report, const uint64_t object) {
    auto object_label = debug_report->GetUtilsObjectNameNoLock(object);
    if (object_label != "") {
        object_label = "(" + object_label + ")";
    }
    return object_label;
}

// Generate the stage-specific part of the message.
static void GenerateStageMessage(std::ostringstream &ss, uint32_t stage_id, uint32_t stage_info_0, uint32_t stage_info_1,
                                 uint32_t stage_info_2, const std::vector<uint32_t> &instrumented_spirv) {
    switch (stage_id) {
        case glsl::kHeaderStageIdMultiEntryPoint: {
            ss << "Stage has multiple OpEntryPoint (";
            ::spirv::GetExecutionModelNames(instrumented_spirv, ss);
            ss << ") and could not detect stage. ";
        } break;
        case spv::ExecutionModelVertex: {
            ss << "Stage = Vertex. Vertex Index = " << stage_info_0 << " Instance Index = " << stage_info_1 << ". ";
        } break;
        case spv::ExecutionModelTessellationControl: {
            ss << "Stage = Tessellation Control.  Invocation ID = " << stage_info_0 << ", Primitive ID = " << stage_info_1;
        } break;
        case spv::ExecutionModelTessellationEvaluation: {
            ss << "Stage = Tessellation Eval.  Primitive ID = " << stage_info_0 << ", TessCoord (u, v) = (" << stage_info_1 << ", "
               << stage_info_2 << "). ";
        } break;
        case spv::ExecutionModelGeometry: {
            ss << "Stage = Geometry.  Primitive ID = " << stage_info_0 << " Invocation ID = " << stage_info_1 << ". ";
        } break;
        case spv::ExecutionModelFragment: {
            // Should use std::bit_cast but requires c++20
            float x_coord;
            float y_coord;
            std::memcpy(&x_coord, &stage_info_0, sizeof(float));
            std::memcpy(&y_coord, &stage_info_1, sizeof(float));
            ss << "Stage = Fragment.  Fragment coord (x,y) = (" << x_coord << ", " << y_coord << "). ";
        } break;
        case spv::ExecutionModelGLCompute: {
            ss << "Stage = Compute.  Global invocation ID (x, y, z) = (" << stage_info_0 << ", " << stage_info_1 << ", "
               << stage_info_2 << ")";
        } break;
        case spv::ExecutionModelRayGenerationKHR: {
            ss << "Stage = Ray Generation.  Global Launch ID (x,y,z) = (" << stage_info_0 << ", " << stage_info_1 << ", "
               << stage_info_2 << "). ";
        } break;
        case spv::ExecutionModelIntersectionKHR: {
            ss << "Stage = Intersection.  Global Launch ID (x,y,z) = (" << stage_info_0 << ", " << stage_info_1 << ", "
               << stage_info_2 << "). ";
        } break;
        case spv::ExecutionModelAnyHitKHR: {
            ss << "Stage = Any Hit.  Global Launch ID (x,y,z) = (" << stage_info_0 << ", " << stage_info_1 << ", " << stage_info_2
               << "). ";
        } break;
        case spv::ExecutionModelClosestHitKHR: {
            ss << "Stage = Closest Hit.  Global Launch ID (x,y,z) = (" << stage_info_0 << ", " << stage_info_1 << ", "
               << stage_info_2 << "). ";
        } break;
        case spv::ExecutionModelMissKHR: {
            ss << "Stage = Miss.  Global Launch ID (x,y,z) = (" << stage_info_0 << ", " << stage_info_1 << ", " << stage_info_2
               << "). ";
        } break;
        case spv::ExecutionModelCallableKHR: {
            ss << "Stage = Callable.  Global Launch ID (x,y,z) = (" << stage_info_0 << ", " << stage_info_1 << ", " << stage_info_2
               << "). ";
        } break;
        case spv::ExecutionModelTaskEXT: {
            ss << "Stage = TaskEXT. Global invocation ID (x, y, z) = (" << stage_info_0 << ", " << stage_info_1 << ", "
               << stage_info_2 << " )";
        } break;
        case spv::ExecutionModelMeshEXT: {
            ss << "Stage = MeshEXT. Global invocation ID (x, y, z) = (" << stage_info_0 << ", " << stage_info_1 << ", "
               << stage_info_2 << " )";
        } break;
        case spv::ExecutionModelTaskNV: {
            ss << "Stage = TaskNV. Global invocation ID (x, y, z) = (" << stage_info_0 << ", " << stage_info_1 << ", "
               << stage_info_2 << " )";
        } break;
        case spv::ExecutionModelMeshNV: {
            ss << "Stage = MeshNV. Global invocation ID (x, y, z) = (" << stage_info_0 << ", " << stage_info_1 << ", "
               << stage_info_2 << " )";
        } break;
        default: {
            ss << "Internal Error (unexpected stage = " << stage_id << "). ";
            assert(false);
        } break;
    }
    ss << '\n';
}

// There are 2 ways to inject source into a shader:
// 1. The "old" way using OpLine/OpSource
// 2. The "new" way using NonSemantic Shader DebugInfo
static std::string FindShaderSource(std::ostringstream &ss, const std::vector<uint32_t> &instrumented_spirv,
                                    uint32_t instruction_position, bool debug_printf_only) {
    ss << "SPIR-V Instruction Index = " << instruction_position << '\n';

    const uint32_t last_line_inst_offset = ::spirv::GetDebugLineOffset(instrumented_spirv, instruction_position);
    if (last_line_inst_offset != 0) {
        Instruction last_line_inst(instrumented_spirv.data() + last_line_inst_offset);
        ss << (debug_printf_only ? "Debug shader printf message generated " : "Shader validation error occurred ");
        GetShaderSourceInfo(ss, instrumented_spirv, last_line_inst);
    } else {
        ss << "Unable to source. Build shader with debug info to get source information.\n";
    }

    return ss.str();
}

// Where we build up the error message with all the useful debug information about where the error occured
std::string GpuShaderInstrumentor::GenerateDebugInfoMessage(VkCommandBuffer commandBuffer, uint32_t stage_id, uint32_t stage_info_0,
                                                            uint32_t stage_info_1, uint32_t stage_info_2,
                                                            uint32_t instruction_position,
                                                            const InstrumentedShader *instrumented_shader, uint32_t shader_id,
                                                            VkPipelineBindPoint pipeline_bind_point,
                                                            uint32_t operation_index) const {
    std::ostringstream ss;
    if (!instrumented_shader || instrumented_shader->instrumented_spirv.empty()) {
        ss << "[Internal Error] - Can't get instructions from shader_map\n";
        return ss.str();
    }

    GenerateStageMessage(ss, stage_id, stage_info_0, stage_info_1, stage_info_2, instrumented_shader->instrumented_spirv);

    ss << std::hex << std::showbase;
    if (instrumented_shader->shader_module == VK_NULL_HANDLE && instrumented_shader->shader_object == VK_NULL_HANDLE) {
        std::unique_lock<std::mutex> lock(debug_report->debug_output_mutex);
        ss << "[Internal Error] - Unable to locate shader/pipeline handles used in command buffer "
           << LookupDebugUtilsNameNoLock(debug_report, HandleToUint64(commandBuffer)) << "(" << HandleToUint64(commandBuffer)
           << ")\n";
        assert(true);
    } else {
        std::unique_lock<std::mutex> lock(debug_report->debug_output_mutex);
        ss << "Command buffer " << LookupDebugUtilsNameNoLock(debug_report, HandleToUint64(commandBuffer)) << "("
           << HandleToUint64(commandBuffer) << ")\n";
        ss << std::dec << std::noshowbase;
        ss << '\t';  // helps to show that the index is expressed with respect to the command buffer
        if (pipeline_bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
            ss << "Draw ";
        } else if (pipeline_bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) {
            ss << "Compute Dispatch ";
        } else if (pipeline_bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR) {
            ss << "Ray Trace ";
        } else {
            assert(false);
            ss << "Unknown Pipeline Operation ";
        }
        ss << "Index " << operation_index << '\n';
        ss << std::hex << std::noshowbase;

        if (instrumented_shader->shader_module == VK_NULL_HANDLE) {
            ss << "Shader Object " << LookupDebugUtilsNameNoLock(debug_report, HandleToUint64(instrumented_shader->shader_object))
               << "(" << HandleToUint64(instrumented_shader->shader_object) << ") (internal ID " << shader_id << ")\n";
        } else {
            ss << "Pipeline " << LookupDebugUtilsNameNoLock(debug_report, HandleToUint64(instrumented_shader->pipeline)) << "("
               << HandleToUint64(instrumented_shader->pipeline) << ")";
            if (instrumented_shader->shader_module == kPipelineStageInfoHandle) {
                ss << " (internal ID " << shader_id
                   << ")\nShader Module was passed in via VkPipelineShaderStageCreateInfo::pNext\n";
            } else {
                ss << "\nShader Module "
                   << LookupDebugUtilsNameNoLock(debug_report, HandleToUint64(instrumented_shader->shader_module)) << "("
                   << HandleToUint64(instrumented_shader->shader_module) << ") (internal ID " << shader_id << ")\n";
            }
        }
    }
    ss << std::dec << std::noshowbase;

    FindShaderSource(ss, instrumented_shader->instrumented_spirv, instruction_position, gpuav_settings.debug_printf_only);

    return ss.str();
}

}  // namespace gpuav
