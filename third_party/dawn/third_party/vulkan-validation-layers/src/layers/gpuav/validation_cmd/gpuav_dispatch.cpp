/* Copyright (c) 2018-2024 The Khronos Group Inc.
 * Copyright (c) 2018-2024 Valve Corporation
 * Copyright (c) 2018-2024 LunarG, Inc.
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
#include "gpuav/resources/gpuav_state_trackers.h"
#include "gpuav/shaders/gpuav_error_header.h"
#include "gpuav/shaders/gpuav_shaders_constants.h"
#include "generated/validation_cmd_dispatch_comp.h"

namespace gpuav {

// See gpuav/shaders/validation_cmd/dispatch.comp
constexpr uint32_t kPushConstantDWords = 4u;

struct SharedDispatchValidationResources final {
    VkDescriptorSetLayout ds_layout = VK_NULL_HANDLE;
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkShaderEXT shader_object = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;

    SharedDispatchValidationResources(Validator &gpuav, VkDescriptorSetLayout error_output_desc_set, bool use_shader_objects,
                                      const Location &loc)
        : device(gpuav.device) {
        VkResult result = VK_SUCCESS;
        std::vector<VkDescriptorSetLayoutBinding> bindings = {
            {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},  // indirect buffer
        };

        VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
        ds_layout_ci.bindingCount = static_cast<uint32_t>(bindings.size());
        ds_layout_ci.pBindings = bindings.data();
        result = DispatchCreateDescriptorSetLayout(device, &ds_layout_ci, nullptr, &ds_layout);
        if (result != VK_SUCCESS) {
            gpuav.InternalError(device, loc, "Unable to create descriptor set layout.");
            return;
        }

        VkPushConstantRange push_constant_range = {};
        push_constant_range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        push_constant_range.offset = 0;
        push_constant_range.size = kPushConstantDWords * sizeof(uint32_t);

        std::array<VkDescriptorSetLayout, 2> set_layouts = {{error_output_desc_set, ds_layout}};
        VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
        pipeline_layout_ci.pushConstantRangeCount = 1;
        pipeline_layout_ci.pPushConstantRanges = &push_constant_range;
        pipeline_layout_ci.setLayoutCount = static_cast<uint32_t>(set_layouts.size());
        pipeline_layout_ci.pSetLayouts = set_layouts.data();
        result = DispatchCreatePipelineLayout(device, &pipeline_layout_ci, nullptr, &pipeline_layout);
        if (result != VK_SUCCESS) {
            gpuav.InternalError(device, loc, "Unable to create pipeline layout.");
            return;
        }

        if (use_shader_objects) {
            VkShaderCreateInfoEXT shader_ci = vku::InitStructHelper();
            shader_ci.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            shader_ci.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
            shader_ci.codeSize = validation_cmd_dispatch_comp_size * sizeof(uint32_t);
            shader_ci.pCode = validation_cmd_dispatch_comp;
            shader_ci.pName = "main";
            shader_ci.setLayoutCount = pipeline_layout_ci.setLayoutCount;
            shader_ci.pSetLayouts = pipeline_layout_ci.pSetLayouts;
            shader_ci.pushConstantRangeCount = pipeline_layout_ci.pushConstantRangeCount;
            shader_ci.pPushConstantRanges = pipeline_layout_ci.pPushConstantRanges;
            result = DispatchCreateShadersEXT(device, 1u, &shader_ci, nullptr, &shader_object);
            if (result != VK_SUCCESS) {
                gpuav.InternalError(device, loc, "Unable to create shader object.");
                return;
            }
        }

        {
            VkShaderModuleCreateInfo shader_module_ci = vku::InitStructHelper();
            shader_module_ci.codeSize = validation_cmd_dispatch_comp_size * sizeof(uint32_t);
            shader_module_ci.pCode = validation_cmd_dispatch_comp;
            VkShaderModule validation_shader = VK_NULL_HANDLE;
            result = DispatchCreateShaderModule(device, &shader_module_ci, nullptr, &validation_shader);
            if (result != VK_SUCCESS) {
                gpuav.InternalError(device, loc, "Unable to create shader module.");
                return;
            }

            // Create pipeline
            VkPipelineShaderStageCreateInfo pipeline_stage_ci = vku::InitStructHelper();
            pipeline_stage_ci.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            pipeline_stage_ci.module = validation_shader;
            pipeline_stage_ci.pName = "main";

            VkComputePipelineCreateInfo pipeline_ci = vku::InitStructHelper();
            pipeline_ci.stage = pipeline_stage_ci;
            pipeline_ci.layout = pipeline_layout;

            result = DispatchCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);

            DispatchDestroyShaderModule(device, validation_shader, nullptr);

            if (result != VK_SUCCESS) {
                gpuav.InternalError(device, loc, "Failed to create compute pipeline for dispatch validation.");
                return;
            }
        }
    }

    ~SharedDispatchValidationResources() {
        if (ds_layout != VK_NULL_HANDLE) {
            DispatchDestroyDescriptorSetLayout(device, ds_layout, nullptr);
            ds_layout = VK_NULL_HANDLE;
        }
        if (pipeline_layout != VK_NULL_HANDLE) {
            DispatchDestroyPipelineLayout(device, pipeline_layout, nullptr);
            pipeline_layout = VK_NULL_HANDLE;
        }
        if (pipeline != VK_NULL_HANDLE) {
            DispatchDestroyPipeline(device, pipeline, nullptr);
            pipeline = VK_NULL_HANDLE;
        }
        if (shader_object != VK_NULL_HANDLE) {
            DispatchDestroyShaderEXT(device, shader_object, nullptr);
            shader_object = VK_NULL_HANDLE;
        }
    }

    bool IsValid() const {
        return ds_layout != VK_NULL_HANDLE && pipeline_layout != VK_NULL_HANDLE &&
               (pipeline != VK_NULL_HANDLE || shader_object != VK_NULL_HANDLE) && device != VK_NULL_HANDLE;
    }
};

void InsertIndirectDispatchValidation(Validator &gpuav, const Location &loc, CommandBuffer &cb_state, VkBuffer indirect_buffer,
                                      VkDeviceSize indirect_offset) {
    if (!gpuav.gpuav_settings.validate_indirect_dispatches_buffers) {
        return;
    }

    // Insert a dispatch that can examine some device memory right before the dispatch we're validating
    //
    // NOTE that this validation does not attempt to abort invalid api calls as most other validation does. A crash
    // or DEVICE_LOST resulting from the invalid call will prevent preceding validation errors from being reported.

    const auto lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_COMPUTE);
    auto const &last_bound = cb_state.lastBound[lv_bind_point];
    const auto *pipeline_state = last_bound.pipeline_state;
    const bool use_shader_objects = pipeline_state == nullptr;

    auto &shared_dispatch_resources = gpuav.shared_resources_manager.Get<SharedDispatchValidationResources>(
        gpuav, cb_state.GetErrorLoggingDescSetLayout(), use_shader_objects, loc);

    assert(shared_dispatch_resources.IsValid());
    if (!shared_dispatch_resources.IsValid()) {
        return;
    }

    VkDescriptorSet indirect_buffer_desc_set =
        cb_state.gpu_resources_manager.GetManagedDescriptorSet(shared_dispatch_resources.ds_layout);
    if (indirect_buffer_desc_set == VK_NULL_HANDLE) {
        gpuav.InternalError(cb_state.VkHandle(), loc, "Unable to allocate descriptor set.");
        return;
    }

    VkDescriptorBufferInfo desc_buffer_info{};
    // Indirect buffer
    desc_buffer_info.buffer = indirect_buffer;
    desc_buffer_info.offset = 0;
    desc_buffer_info.range = VK_WHOLE_SIZE;

    VkWriteDescriptorSet desc_write{};
    desc_write = vku::InitStructHelper();
    desc_write.dstBinding = 0;
    desc_write.descriptorCount = 1;
    desc_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    desc_write.pBufferInfo = &desc_buffer_info;
    desc_write.dstSet = indirect_buffer_desc_set;

    DispatchUpdateDescriptorSets(gpuav.device, 1, &desc_write, 0, nullptr);

    // Save current graphics pipeline state
    RestorablePipelineState restorable_state(cb_state, VK_PIPELINE_BIND_POINT_COMPUTE);

    // Insert diagnostic dispatch
    if (use_shader_objects) {
        VkShaderStageFlagBits stage = VK_SHADER_STAGE_COMPUTE_BIT;
        DispatchCmdBindShadersEXT(cb_state.VkHandle(), 1u, &stage, &shared_dispatch_resources.shader_object);
    } else {
        DispatchCmdBindPipeline(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, shared_dispatch_resources.pipeline);
    }
    uint32_t push_constants[kPushConstantDWords] = {};
    push_constants[0] = gpuav.phys_dev_props.limits.maxComputeWorkGroupCount[0];
    push_constants[1] = gpuav.phys_dev_props.limits.maxComputeWorkGroupCount[1];
    push_constants[2] = gpuav.phys_dev_props.limits.maxComputeWorkGroupCount[2];
    push_constants[3] = static_cast<uint32_t>((indirect_offset / sizeof(uint32_t)));
    DispatchCmdPushConstants(cb_state.VkHandle(), shared_dispatch_resources.pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0,
                             sizeof(push_constants), push_constants);
    BindErrorLoggingDescSet(gpuav, cb_state, VK_PIPELINE_BIND_POINT_COMPUTE, shared_dispatch_resources.pipeline_layout,
                            cb_state.compute_index, static_cast<uint32_t>(cb_state.per_command_error_loggers.size()));
    DispatchCmdBindDescriptorSets(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, shared_dispatch_resources.pipeline_layout,
                                  glsl::kDiagPerCmdDescriptorSet, 1, &indirect_buffer_desc_set, 0, nullptr);
    DispatchCmdDispatch(cb_state.VkHandle(), 1, 1, 1);

    CommandBuffer::ErrorLoggerFunc error_logger = [loc](Validator &gpuav, const CommandBuffer &, const uint32_t *error_record,
                                                        const LogObjectList &objlist, const std::vector<std::string> &) {
        bool skip = false;
        using namespace glsl;

        if (error_record[kHeaderErrorGroupOffset] != kErrorGroupGpuPreDispatch) {
            return skip;
        }

        switch (error_record[kHeaderErrorSubCodeOffset]) {
            case kErrorSubCodePreDispatchCountLimitX: {
                uint32_t count = error_record[kPreActionParamOffset_0];
                skip |= gpuav.LogError("VUID-VkDispatchIndirectCommand-x-00417", objlist, loc,
                                       "Indirect dispatch VkDispatchIndirectCommand::x of %" PRIu32
                                       " would exceed maxComputeWorkGroupCount[0] limit of %" PRIu32 ".",
                                       count, gpuav.phys_dev_props.limits.maxComputeWorkGroupCount[0]);
                break;
            }
            case kErrorSubCodePreDispatchCountLimitY: {
                uint32_t count = error_record[kPreActionParamOffset_0];
                skip |= gpuav.LogError("VUID-VkDispatchIndirectCommand-y-00418", objlist, loc,
                                       "Indirect dispatch VkDispatchIndirectCommand::y of %" PRIu32
                                       " would exceed maxComputeWorkGroupCount[1] limit of %" PRIu32 ".",
                                       count, gpuav.phys_dev_props.limits.maxComputeWorkGroupCount[1]);
                break;
            }
            case kErrorSubCodePreDispatchCountLimitZ: {
                uint32_t count = error_record[kPreActionParamOffset_0];
                skip |= gpuav.LogError("VUID-VkDispatchIndirectCommand-z-00419", objlist, loc,
                                       "Indirect dispatch VkDispatchIndirectCommand::z of %" PRIu32
                                       " would exceed maxComputeWorkGroupCount[2] limit of %" PRIu32 ".",
                                       count, gpuav.phys_dev_props.limits.maxComputeWorkGroupCount[0]);
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
