/* Copyright (c) 2018-2025 The Khronos Group Inc.
 * Copyright (c) 2018-2025 Valve Corporation
 * Copyright (c) 2018-2025 LunarG, Inc.
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

#include "gpuav/core/gpuav.h"
#include "gpuav/validation_cmd/gpuav_validation_cmd_common.h"
#include "gpuav/resources/gpuav_vulkan_objects.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "gpuav/shaders/gpuav_error_header.h"
#include "generated/validation_cmd_trace_rays_rgen.h"
#include "error_message/error_strings.h"

// See gpuav/shaders/validation_cmd/trace_rays.rgen
constexpr uint32_t kPushConstantDWords = 6u;

namespace gpuav {

struct SharedTraceRaysValidationResources final {
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;
    VmaPool sbt_pool = VK_NULL_HANDLE;
    VkBuffer sbt_buffer = VK_NULL_HANDLE;
    VmaAllocation sbt_allocation = {};
    VkDeviceAddress sbt_address = 0;
    uint32_t shader_group_handle_size_aligned = 0;
    VmaAllocator vma_allocator;
    VkDevice device = VK_NULL_HANDLE;
    bool valid = false;

    SharedTraceRaysValidationResources(Validator& gpuav, VkDescriptorSetLayout error_output_desc_layout, const Location& loc)
        : vma_allocator(gpuav.vma_allocator_), device(gpuav.device), valid(false) {
        VkResult result = VK_SUCCESS;

        VkPushConstantRange push_constant_range = {};
        push_constant_range.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        push_constant_range.offset = 0;
        push_constant_range.size = kPushConstantDWords * sizeof(uint32_t);

        VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
        pipeline_layout_ci.pushConstantRangeCount = 1;
        pipeline_layout_ci.pPushConstantRanges = &push_constant_range;
        pipeline_layout_ci.setLayoutCount = 1;
        pipeline_layout_ci.pSetLayouts = &error_output_desc_layout;
        result = DispatchCreatePipelineLayout(gpuav.device, &pipeline_layout_ci, nullptr, &pipeline_layout);
        if (result != VK_SUCCESS) {
            gpuav.InternalError(gpuav.device, loc, "Unable to create pipeline layout.");
            return;
        }

        VkShaderModuleCreateInfo shader_module_ci = vku::InitStructHelper();
        shader_module_ci.codeSize = validation_cmd_trace_rays_rgen_size * sizeof(uint32_t);
        shader_module_ci.pCode = validation_cmd_trace_rays_rgen;
        VkShaderModule validation_shader = VK_NULL_HANDLE;
        result = DispatchCreateShaderModule(gpuav.device, &shader_module_ci, nullptr, &validation_shader);
        if (result != VK_SUCCESS) {
            gpuav.InternalError(gpuav.device, loc, "Unable to create ray tracing shader module.");
            return;
        }

        // Create pipeline
        VkPipelineShaderStageCreateInfo pipeline_stage_ci = vku::InitStructHelper();
        pipeline_stage_ci.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        pipeline_stage_ci.module = validation_shader;
        pipeline_stage_ci.pName = "main";
        VkRayTracingShaderGroupCreateInfoKHR raygen_group_ci = vku::InitStructHelper();
        raygen_group_ci.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        raygen_group_ci.generalShader = 0;
        raygen_group_ci.closestHitShader = VK_SHADER_UNUSED_KHR;
        raygen_group_ci.anyHitShader = VK_SHADER_UNUSED_KHR;
        raygen_group_ci.intersectionShader = VK_SHADER_UNUSED_KHR;

        VkRayTracingPipelineCreateInfoKHR rt_pipeline_create_info{};
        rt_pipeline_create_info.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
        rt_pipeline_create_info.stageCount = 1;
        rt_pipeline_create_info.pStages = &pipeline_stage_ci;
        rt_pipeline_create_info.groupCount = 1;
        rt_pipeline_create_info.pGroups = &raygen_group_ci;
        rt_pipeline_create_info.maxPipelineRayRecursionDepth = 1;
        rt_pipeline_create_info.layout = pipeline_layout;
        result = DispatchCreateRayTracingPipelinesKHR(gpuav.device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rt_pipeline_create_info,
                                                      nullptr, &pipeline);

        DispatchDestroyShaderModule(gpuav.device, validation_shader, nullptr);

        if (result != VK_SUCCESS) {
            gpuav.InternalError(gpuav.device, loc, "Failed to create ray tracing pipeline for pre trace rays validation.");
            return;
        }

        VkPhysicalDeviceRayTracingPipelinePropertiesKHR rt_pipeline_props = vku::InitStructHelper();
        VkPhysicalDeviceProperties2 props2 = vku::InitStructHelper(&rt_pipeline_props);
        DispatchGetPhysicalDeviceProperties2(gpuav.physical_device, &props2);

        // Get shader group handles to fill shader binding table (SBT)
        const uint32_t shader_group_size_aligned =
            Align(rt_pipeline_props.shaderGroupHandleSize, rt_pipeline_props.shaderGroupHandleAlignment);
        const uint32_t sbt_size = 1 * shader_group_size_aligned;
        std::vector<uint8_t> sbt_host_storage(sbt_size);
        result = DispatchGetRayTracingShaderGroupHandlesKHR(gpuav.device, pipeline, 0, rt_pipeline_create_info.groupCount, sbt_size,
                                                            sbt_host_storage.data());
        if (result != VK_SUCCESS) {
            gpuav.InternalError(gpuav.device, loc, "Failed to call vkGetRayTracingShaderGroupHandlesKHR.");
            return;
        }

        // Allocate buffer to store SBT, and fill it with sbt_host_storage
        VkBufferCreateInfo buffer_info = vku::InitStructHelper();
        buffer_info.size = 4096;
        buffer_info.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR |
                            VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        VmaAllocationCreateInfo alloc_info = {};
        alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        uint32_t mem_type_index = 0;
        vmaFindMemoryTypeIndexForBufferInfo(vma_allocator, &buffer_info, &alloc_info, &mem_type_index);
        VmaPoolCreateInfo pool_create_info = {};
        pool_create_info.memoryTypeIndex = mem_type_index;
        pool_create_info.blockSize = 0;
        pool_create_info.maxBlockCount = 0;
        pool_create_info.flags = VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT;
        result = vmaCreatePool(vma_allocator, &pool_create_info, &sbt_pool);
        if (result != VK_SUCCESS) {
            gpuav.InternalVmaError(gpuav.device, loc, "Unable to create VMA memory pool for SBT.");
            return;
        }

        alloc_info.pool = sbt_pool;
        result = vmaCreateBuffer(vma_allocator, &buffer_info, &alloc_info, &sbt_buffer, &sbt_allocation, nullptr);
        if (result != VK_SUCCESS) {
            gpuav.InternalVmaError(gpuav.device, loc, "Unable to allocate device memory for shader binding table.");
            return;
        }

        uint8_t* mapped_sbt = nullptr;
        result = vmaMapMemory(vma_allocator, sbt_allocation, reinterpret_cast<void**>(&mapped_sbt));

        if (result != VK_SUCCESS) {
            gpuav.InternalVmaError(gpuav.device, loc,
                                   "Failed to map shader binding table when creating trace rays validation resources.");
            return;
        }

        std::memcpy(mapped_sbt, sbt_host_storage.data(), rt_pipeline_props.shaderGroupHandleSize);

        vmaUnmapMemory(vma_allocator, sbt_allocation);

        shader_group_handle_size_aligned = shader_group_size_aligned;

        // Retrieve SBT address
        const VkDeviceAddress sbt_address = gpuav.GetBufferDeviceAddressHelper(sbt_buffer);
        assert(sbt_address != 0);
        if (sbt_address == 0) {
            gpuav.InternalError(gpuav.device, loc, "Retrieved SBT buffer device address is null.");
            return;
        }
        assert(sbt_address == Align(sbt_address, static_cast<VkDeviceAddress>(rt_pipeline_props.shaderGroupBaseAlignment)));
        this->sbt_address = sbt_address;

        valid = true;
    }

    ~SharedTraceRaysValidationResources() {
        if (pipeline_layout != VK_NULL_HANDLE) {
            DispatchDestroyPipelineLayout(device, pipeline_layout, nullptr);
            pipeline_layout = VK_NULL_HANDLE;
        }
        if (pipeline != VK_NULL_HANDLE) {
            DispatchDestroyPipeline(device, pipeline, nullptr);
            pipeline = VK_NULL_HANDLE;
        }
        if (sbt_buffer != VK_NULL_HANDLE) {
            vmaDestroyBuffer(vma_allocator, sbt_buffer, sbt_allocation);
            sbt_buffer = VK_NULL_HANDLE;
            sbt_allocation = VK_NULL_HANDLE;
            sbt_address = 0;
        }
        if (sbt_pool) {
            vmaDestroyPool(vma_allocator, sbt_pool);
            sbt_pool = VK_NULL_HANDLE;
        }
    }

    bool IsValid() const { return valid; }
};

void InsertIndirectTraceRaysValidation(Validator& gpuav, const Location& loc, CommandBuffer& cb_state,
                                       VkDeviceAddress indirect_data_address) {
    if (!gpuav.gpuav_settings.validate_indirect_trace_rays_buffers) {
        return;
    }

    if (!gpuav.enabled_features.shaderInt64) {
        return;
    }

    auto& shared_trace_rays_resources =
        gpuav.shared_resources_manager.Get<SharedTraceRaysValidationResources>(gpuav, cb_state.GetErrorLoggingDescSetLayout(), loc);

    assert(shared_trace_rays_resources.IsValid());
    if (!shared_trace_rays_resources.IsValid()) {
        return;
    }

    // Save current ray tracing pipeline state
    RestorablePipelineState restorable_state(cb_state, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);

    // Push info needed for validation:
    // - the device address indirect data is read from
    // - the limits to check against
    uint32_t push_constants[kPushConstantDWords] = {};
    const uint64_t ray_query_dimension_max_width = static_cast<uint64_t>(gpuav.phys_dev_props.limits.maxComputeWorkGroupCount[0]) *
                                                   static_cast<uint64_t>(gpuav.phys_dev_props.limits.maxComputeWorkGroupSize[0]);
    const uint64_t ray_query_dimension_max_height = static_cast<uint64_t>(gpuav.phys_dev_props.limits.maxComputeWorkGroupCount[1]) *
                                                    static_cast<uint64_t>(gpuav.phys_dev_props.limits.maxComputeWorkGroupSize[1]);
    const uint64_t ray_query_dimension_max_depth = static_cast<uint64_t>(gpuav.phys_dev_props.limits.maxComputeWorkGroupCount[2]) *
                                                   static_cast<uint64_t>(gpuav.phys_dev_props.limits.maxComputeWorkGroupSize[2]);
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rt_pipeline_props = vku::InitStructHelper();
    VkPhysicalDeviceProperties2 props2 = vku::InitStructHelper(&rt_pipeline_props);
    DispatchGetPhysicalDeviceProperties2(gpuav.physical_device, &props2);
    // Need to put the buffer reference first otherwise it is incorrect, probably an alignment issue
    push_constants[0] = static_cast<uint32_t>(indirect_data_address) & vvl::kU32Max;
    push_constants[1] = static_cast<uint32_t>(indirect_data_address >> 32) & vvl::kU32Max;
    push_constants[2] = static_cast<uint32_t>(std::min<uint64_t>(ray_query_dimension_max_width, vvl::kU32Max));
    push_constants[3] = static_cast<uint32_t>(std::min<uint64_t>(ray_query_dimension_max_height, vvl::kU32Max));
    push_constants[4] = static_cast<uint32_t>(std::min<uint64_t>(ray_query_dimension_max_depth, vvl::kU32Max));
    push_constants[5] = rt_pipeline_props.maxRayDispatchInvocationCount;

    DispatchCmdBindPipeline(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, shared_trace_rays_resources.pipeline);
    BindErrorLoggingDescSet(gpuav, cb_state, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, shared_trace_rays_resources.pipeline_layout,
                            cb_state.trace_rays_index, static_cast<uint32_t>(cb_state.per_command_error_loggers.size()));
    DispatchCmdPushConstants(cb_state.VkHandle(), shared_trace_rays_resources.pipeline_layout, VK_SHADER_STAGE_RAYGEN_BIT_KHR, 0,
                             sizeof(push_constants), push_constants);
    VkStridedDeviceAddressRegionKHR ray_gen_sbt{};
    assert(shared_trace_rays_resources.sbt_address != 0);
    ray_gen_sbt.deviceAddress = shared_trace_rays_resources.sbt_address;
    ray_gen_sbt.stride = shared_trace_rays_resources.shader_group_handle_size_aligned;
    ray_gen_sbt.size = shared_trace_rays_resources.shader_group_handle_size_aligned;

    VkStridedDeviceAddressRegionKHR empty_sbt{};
    DispatchCmdTraceRaysKHR(cb_state.VkHandle(), &ray_gen_sbt, &empty_sbt, &empty_sbt, &empty_sbt, 1, 1, 1);

    CommandBuffer::ErrorLoggerFunc error_logger = [loc](Validator& gpuav, const CommandBuffer&, const uint32_t* error_record,
                                                        const LogObjectList& objlist, const std::vector<std::string>&) {
        bool skip = false;

        using namespace glsl;

        if (error_record[kHeaderErrorGroupOffset] != kErrorGroupGpuPreTraceRays) {
            return skip;
        }

        switch (error_record[kHeaderErrorSubCodeOffset]) {
            case kErrorSubCodePreTraceRaysLimitWidth: {
                const uint32_t width = error_record[kPreActionParamOffset_0];
                skip |= gpuav.LogError("VUID-VkTraceRaysIndirectCommandKHR-width-03638", objlist, loc,
                                       "Indirect trace rays of VkTraceRaysIndirectCommandKHR::width of %" PRIu32
                                       " would exceed VkPhysicalDeviceLimits::maxComputeWorkGroupCount[0] * "
                                       "VkPhysicalDeviceLimits::maxComputeWorkGroupSize[0] limit of %" PRIu64 ".",
                                       width,
                                       static_cast<uint64_t>(gpuav.phys_dev_props.limits.maxComputeWorkGroupCount[0]) *
                                           static_cast<uint64_t>(gpuav.phys_dev_props.limits.maxComputeWorkGroupSize[0]));
                break;
            }
            case kErrorSubCodePreTraceRaysLimitHeight: {
                const uint32_t height = error_record[kPreActionParamOffset_0];
                skip |= gpuav.LogError("VUID-VkTraceRaysIndirectCommandKHR-height-03639", objlist, loc,
                                       "Indirect trace rays of VkTraceRaysIndirectCommandKHR::height of %" PRIu32
                                       " would exceed VkPhysicalDeviceLimits::maxComputeWorkGroupCount[1] * "
                                       "VkPhysicalDeviceLimits::maxComputeWorkGroupSize[1] limit of %" PRIu64 ".",
                                       height,
                                       static_cast<uint64_t>(gpuav.phys_dev_props.limits.maxComputeWorkGroupCount[1]) *
                                           static_cast<uint64_t>(gpuav.phys_dev_props.limits.maxComputeWorkGroupSize[1]));
                break;
            }
            case kErrorSubCodePreTraceRaysLimitDepth: {
                const uint32_t depth = error_record[kPreActionParamOffset_0];
                skip |= gpuav.LogError("VUID-VkTraceRaysIndirectCommandKHR-depth-03640", objlist, loc,
                                       "Indirect trace rays of VkTraceRaysIndirectCommandKHR::height of %" PRIu32
                                       " would exceed VkPhysicalDeviceLimits::maxComputeWorkGroupCount[2] * "
                                       "VkPhysicalDeviceLimits::maxComputeWorkGroupSize[2] limit of %" PRIu64 ".",
                                       depth,
                                       static_cast<uint64_t>(gpuav.phys_dev_props.limits.maxComputeWorkGroupCount[2]) *
                                           static_cast<uint64_t>(gpuav.phys_dev_props.limits.maxComputeWorkGroupSize[2]));
                break;
            }
            case kErrorSubCodePreTraceRaysLimitVolume: {
                VkPhysicalDeviceRayTracingPipelinePropertiesKHR rt_pipeline_props = vku::InitStructHelper();
                VkPhysicalDeviceProperties2 props2 = vku::InitStructHelper(&rt_pipeline_props);
                DispatchGetPhysicalDeviceProperties2(gpuav.physical_device, &props2);

                const VkExtent3D trace_rays_extent = {error_record[kPreActionParamOffset_0], error_record[kPreActionParamOffset_1],
                                                      error_record[kPreActionParamOffset_2]};
                const uint64_t rays_volume = trace_rays_extent.width * trace_rays_extent.height * trace_rays_extent.depth;
                skip |= gpuav.LogError(
                    "VUID-VkTraceRaysIndirectCommandKHR-width-03641", objlist, loc,
                    "Indirect trace rays of volume %" PRIu64
                    " (%s) would exceed VkPhysicalDeviceRayTracingPipelinePropertiesKHR::maxRayDispatchInvocationCount "
                    "limit of %" PRIu32 ".",
                    rays_volume, string_VkExtent3D(trace_rays_extent).c_str(), rt_pipeline_props.maxRayDispatchInvocationCount);
                break;
            }
            default:
                break;
        }

        return skip;
    };

    cb_state.per_command_error_loggers.emplace_back(std::move(error_logger));
}

}  // namespace gpuav
