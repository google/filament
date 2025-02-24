/* Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
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

#include <vulkan/vk_enum_string_helper.h>
#include "core_validation.h"
#include "chassis/chassis_modification_state.h"

bool CoreChecks::PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                       const VkComputePipelineCreateInfo *pCreateInfos,
                                                       const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                       const ErrorObject &error_obj, PipelineStates &pipeline_states,
                                                       chassis::CreateComputePipelines &chassis_state) const {
    bool skip = BaseClass::PreCallValidateCreateComputePipelines(device, pipelineCache, count, pCreateInfos, pAllocator, pPipelines,
                                                                 error_obj, pipeline_states, chassis_state);

    skip |= ValidateDeviceQueueSupport(error_obj.location);
    for (uint32_t i = 0; i < count; i++) {
        const vvl::Pipeline *pipeline = pipeline_states[i].get();
        ASSERT_AND_CONTINUE(pipeline);

        const Location create_info_loc = error_obj.location.dot(Field::pCreateInfos, i);
        const Location stage_info = create_info_loc.dot(Field::stage);
        const auto &stage_state = pipeline->stage_states[0];
        skip |= ValidateShaderStage(stage_state, pipeline, stage_info);
        if (stage_state.pipeline_create_info) {
            skip |= ValidatePipelineShaderStage(*pipeline, *stage_state.pipeline_create_info, pCreateInfos[i].pNext, stage_info);
        }

        skip |= ValidateComputePipelineDerivatives(pipeline_states, i, create_info_loc);
        skip |= ValidatePipelineCacheControlFlags(pipeline->create_flags, create_info_loc.dot(Field::flags),
                                                  "VUID-VkComputePipelineCreateInfo-pipelineCreationCacheControl-02875");
        skip |= ValidatePipelineIndirectBindableFlags(pipeline->create_flags, create_info_loc.dot(Field::flags),
                                                      "VUID-VkComputePipelineCreateInfo-flags-09007");

        if (const auto *pipeline_robustness_info =
                vku::FindStructInPNextChain<VkPipelineRobustnessCreateInfo>(pCreateInfos[i].pNext)) {
            skip |= ValidatePipelineRobustnessCreateInfo(*pipeline, *pipeline_robustness_info, create_info_loc);
        }

        // From dumping traces, we found almost all apps only create one pipeline at a time. To greatly simplify the logic, only
        // check the stateless validation in the pNext chain for the first pipeline. (The core issue is because we parse the SPIR-V
        // at state tracking time, and we state track pipelines first)
        if (i == 0 && chassis_state.stateless_data.pipeline_pnext_module) {
            skip |= ValidateSpirvStateless(*chassis_state.stateless_data.pipeline_pnext_module, chassis_state.stateless_data,
                                           create_info_loc.dot(Field::stage).pNext(Struct::VkShaderModuleCreateInfo, Field::pCode));
        }
    }
    return skip;
}

bool CoreChecks::ValidateComputePipelineDerivatives(PipelineStates &pipeline_states, uint32_t pipe_index,
                                                    const Location &loc) const {
    bool skip = false;
    const auto &pipeline = *pipeline_states[pipe_index].get();
    // If create derivative bit is set, check that we've specified a base
    // pipeline correctly, and that the base pipeline was created to allow
    // derivatives.
    if (pipeline.create_flags & VK_PIPELINE_CREATE_2_DERIVATIVE_BIT) {
        std::shared_ptr<const vvl::Pipeline> base_pipeline;
        const auto &pipeline_ci = pipeline.ComputeCreateInfo();
        const VkPipeline base_handle = pipeline_ci.basePipelineHandle;
        const int32_t base_index = pipeline_ci.basePipelineIndex;
        if (base_index != -1 && base_index < static_cast<int32_t>(pipeline_states.size())) {
            if (static_cast<uint32_t>(base_index) >= pipe_index) {
                skip |= LogError("VUID-vkCreateComputePipelines-flags-00695", base_handle, loc,
                                 "base pipeline (index %" PRId32
                                 ") must occur earlier in array than derivative pipeline (index %" PRIu32 ").",
                                 base_index, pipe_index);
            } else {
                base_pipeline = pipeline_states[base_index];
            }
        } else if (base_handle != VK_NULL_HANDLE) {
            base_pipeline = Get<vvl::Pipeline>(base_handle);
        }

        if (base_pipeline && !(base_pipeline->create_flags & VK_PIPELINE_CREATE_2_ALLOW_DERIVATIVES_BIT)) {
            skip |= LogError("VUID-vkCreateComputePipelines-flags-00696", base_pipeline->Handle(), loc,
                             "base pipeline does not allow derivatives.");
        }
    }
    return skip;
}
