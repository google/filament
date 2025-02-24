/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
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

#include "best_practices/best_practices_validation.h"
#include "best_practices/bp_state.h"
#include "state_tracker/render_pass_state.h"
#include "chassis/chassis_modification_state.h"

static inline bool FormatHasFullThroughputBlendingArm(VkFormat format) {
    switch (format) {
        case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
        case VK_FORMAT_R16_SFLOAT:
        case VK_FORMAT_R16G16_SFLOAT:
        case VK_FORMAT_R16G16B16_SFLOAT:
        case VK_FORMAT_R16G16B16A16_SFLOAT:
        case VK_FORMAT_R32_SFLOAT:
        case VK_FORMAT_R32G32_SFLOAT:
        case VK_FORMAT_R32G32B32_SFLOAT:
        case VK_FORMAT_R32G32B32A32_SFLOAT:
            return false;

        default:
            return true;
    }
}

bool BestPractices::ValidateMultisampledBlendingArm(const VkGraphicsPipelineCreateInfo& create_info,
                                                    const Location& create_info_loc) const {
    bool skip = false;

    if (!create_info.pColorBlendState || !create_info.pMultisampleState ||
        create_info.pMultisampleState->rasterizationSamples == VK_SAMPLE_COUNT_1_BIT ||
        create_info.pMultisampleState->sampleShadingEnable) {
        return skip;
    }

    auto rp_state = Get<vvl::RenderPass>(create_info.renderPass);
    if (!rp_state) return skip;

    const auto& subpass = rp_state->create_info.pSubpasses[create_info.subpass];

    // According to spec, pColorBlendState must be ignored if subpass does not have color attachments.
    uint32_t num_color_attachments = std::min(subpass.colorAttachmentCount, create_info.pColorBlendState->attachmentCount);

    for (uint32_t j = 0; j < num_color_attachments; j++) {
        const auto& blend_att = create_info.pColorBlendState->pAttachments[j];
        uint32_t att = subpass.pColorAttachments[j].attachment;

        if (att != VK_ATTACHMENT_UNUSED && blend_att.blendEnable && blend_att.colorWriteMask) {
            if (!FormatHasFullThroughputBlendingArm(rp_state->create_info.pAttachments[att].format)) {
                skip |= LogPerformanceWarning("BestPractices-Arm-vkCreatePipelines-multisampled-blending", device, create_info_loc,
                                              "%s Pipeline is multisampled and "
                                              "color attachment #%u makes use "
                                              "of a format which cannot be blended at full throughput when using MSAA.",
                                              VendorSpecificTag(kBPVendorArm), j);
            }
        }
    }

    return skip;
}

void BestPractices::ManualPostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache,
                                                               uint32_t createInfoCount,
                                                               const VkComputePipelineCreateInfo* pCreateInfos,
                                                               const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                               const RecordObject& record_obj, PipelineStates& pipeline_states,
                                                               chassis::CreateComputePipelines& chassis_state) {
    // AMD best practice
    pipeline_cache_ = pipelineCache;
}

bool BestPractices::ValidateCreateGraphicsPipeline(const VkGraphicsPipelineCreateInfo& create_info, const vvl::Pipeline& pipeline,
                                                   const Location create_info_loc) const {
    bool skip = false;
    if (!(pipeline.active_shaders & VK_SHADER_STAGE_MESH_BIT_EXT) && create_info.pVertexInputState) {
        const auto& vertex_input = *create_info.pVertexInputState;
        uint32_t count = 0;
        for (uint32_t j = 0; j < vertex_input.vertexBindingDescriptionCount; j++) {
            if (vertex_input.pVertexBindingDescriptions[j].inputRate == VK_VERTEX_INPUT_RATE_INSTANCE) {
                count++;
            }
        }
        if (count > kMaxInstancedVertexBuffers) {
            skip |= LogPerformanceWarning(
                "BestPractices-vkCreateGraphicsPipelines-too-many-instanced-vertex-buffers", device, create_info_loc,
                "The pipeline is using %u instanced vertex buffers (current limit: %u), but this can be inefficient on the "
                "GPU. If using instanced vertex attributes prefer interleaving them in a single buffer.",
                count, kMaxInstancedVertexBuffers);
        }
    }

    if ((create_info.pRasterizationState) && (create_info.pRasterizationState->depthBiasEnable) &&
        (create_info.pRasterizationState->depthBiasConstantFactor == 0.0f) &&
        (create_info.pRasterizationState->depthBiasSlopeFactor == 0.0f) && VendorCheckEnabled(kBPVendorArm)) {
        skip |=
            LogPerformanceWarning("BestPractices-Arm-vkCreatePipelines-depthbias-zero", device, create_info_loc,
                                  "%s This vkCreateGraphicsPipelines call is created with depthBiasEnable set to true "
                                  "and both depthBiasConstantFactor and depthBiasSlopeFactor are set to 0. This can cause reduced "
                                  "efficiency during rasterization. Consider disabling depthBias or increasing either "
                                  "depthBiasConstantFactor or depthBiasSlopeFactor.",
                                  VendorSpecificTag(kBPVendorArm));
    }

    const auto* graphics_lib_info = vku::FindStructInPNextChain<VkGraphicsPipelineLibraryCreateInfoEXT>(create_info.pNext);
    if (create_info.renderPass == VK_NULL_HANDLE &&
        !vku::FindStructInPNextChain<VkPipelineRenderingCreateInfo>(create_info.pNext) &&
        (!graphics_lib_info ||
         (graphics_lib_info->flags & (VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT |
                                      VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT)) != 0)) {
        skip |= LogWarning(
            "BestPractices-Pipeline-NoRendering", device, create_info_loc,
            "renderPass is VK_NULL_HANDLE and pNext chain does not contain an instance of VkPipelineRenderingCreateInfo.");
    }

    if (VendorCheckEnabled(kBPVendorArm)) {
        skip |= ValidateMultisampledBlendingArm(create_info, create_info_loc);
    }

    if (VendorCheckEnabled(kBPVendorAMD)) {
        if (create_info.pInputAssemblyState && create_info.pInputAssemblyState->primitiveRestartEnable) {
            skip |= LogPerformanceWarning("BestPractices-AMD-CreatePipelines-AvoidPrimitiveRestart", device, create_info_loc,
                                          "%s Use of primitive restart is not recommended", VendorSpecificTag(kBPVendorAMD));
        }

        // TODO: this might be too aggressive of a check
        if (create_info.pDynamicState && create_info.pDynamicState->dynamicStateCount > kDynamicStatesWarningLimitAMD) {
            skip |= LogPerformanceWarning("BestPractices-AMD-CreatePipelines-MinimizeNumDynamicStates", device, create_info_loc,
                                          "%s Dynamic States usage incurs a performance cost. Ensure that they are truly needed",
                                          VendorSpecificTag(kBPVendorAMD));
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                           const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                           const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                           const ErrorObject& error_obj, PipelineStates& pipeline_states,
                                                           chassis::CreateGraphicsPipelines& chassis_state) const {
    bool skip = BaseClass::PreCallValidateCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator,
                                                                  pPipelines, error_obj, pipeline_states, chassis_state);
    if (skip) {
        return skip;
    }

    if ((createInfoCount > 1) && (!pipelineCache)) {
        skip |=
            LogPerformanceWarning("BestPractices-vkCreateGraphicsPipelines-multiple-pipelines-no-cache", device, error_obj.location,
                                  "creating multiple pipelines (createInfoCount is %" PRIu32
                                  ") but is not using a "
                                  "pipeline cache, which may help with performance",
                                  createInfoCount);
    }

    for (uint32_t i = 0; i < createInfoCount; i++) {
        const auto* pipeline = pipeline_states[i].get();
        ASSERT_AND_CONTINUE(pipeline);
        skip |= ValidateCreateGraphicsPipeline(pCreateInfos[i], *pipeline, error_obj.location.dot(Field::pCreateInfos, i));
    }

    if (VendorCheckEnabled(kBPVendorAMD) || VendorCheckEnabled(kBPVendorNVIDIA)) {
        auto prev_pipeline = pipeline_cache_.load();
        if (pipelineCache && prev_pipeline && pipelineCache != prev_pipeline) {
            skip |= LogPerformanceWarning("BestPractices-vkCreatePipelines-multiple-pipelines-caches", device, error_obj.location,
                                          "%s %s A second pipeline cache is in use. "
                                          "Consider using only one pipeline cache to improve cache hit rate.",
                                          VendorSpecificTag(kBPVendorAMD), VendorSpecificTag(kBPVendorNVIDIA));
        }
    }
    if (VendorCheckEnabled(kBPVendorAMD)) {
        const uint32_t pso_count = num_pso_.load();
        if (pso_count > kMaxRecommendedNumberOfPSOAMD) {
            skip |= LogPerformanceWarning("BestPractices-AMD-CreatePipelines-TooManyPipelines", device, error_obj.location,
                                          "%s Too many pipelines created (%" PRIu32 " but max recommended is %" PRIu32
                                          "), consider consolidation",
                                          VendorSpecificTag(kBPVendorAMD), pso_count, kMaxRecommendedNumberOfPSOAMD);
        }
    }

    return skip;
}

static std::vector<bp_state::AttachmentInfo> GetAttachmentAccess(bp_state::Pipeline& pipe_state) {
    std::vector<bp_state::AttachmentInfo> result;
    auto rp = pipe_state.RenderPassState();
    if (!rp || rp->UsesDynamicRendering()) {
        return result;
    }
    const auto& create_info = pipe_state.GraphicsCreateInfo();
    const auto& subpass = rp->create_info.pSubpasses[create_info.subpass];

    // NOTE: see PIPELINE_LAYOUT and vku::safe_VkGraphicsPipelineCreateInfo constructors. pColorBlendState and pDepthStencilState
    // are only non-null if they are enabled.
    if (create_info.pColorBlendState && !(pipe_state.ignore_color_attachments)) {
        // According to spec, pColorBlendState must be ignored if subpass does not have color attachments.
        uint32_t num_color_attachments = std::min(subpass.colorAttachmentCount, create_info.pColorBlendState->attachmentCount);
        for (uint32_t j = 0; j < num_color_attachments; j++) {
            if (create_info.pColorBlendState->pAttachments[j].colorWriteMask != 0) {
                uint32_t attachment = subpass.pColorAttachments[j].attachment;
                if (attachment != VK_ATTACHMENT_UNUSED) {
                    result.emplace_back(attachment, VK_IMAGE_ASPECT_COLOR_BIT);
                }
            }
        }
    }

    if (create_info.pDepthStencilState &&
        (create_info.pDepthStencilState->depthTestEnable || create_info.pDepthStencilState->depthBoundsTestEnable ||
         create_info.pDepthStencilState->stencilTestEnable)) {
        uint32_t attachment = subpass.pDepthStencilAttachment ? subpass.pDepthStencilAttachment->attachment : VK_ATTACHMENT_UNUSED;
        if (attachment != VK_ATTACHMENT_UNUSED) {
            VkImageAspectFlags aspects = 0;
            if (create_info.pDepthStencilState->depthTestEnable || create_info.pDepthStencilState->depthBoundsTestEnable) {
                aspects |= VK_IMAGE_ASPECT_DEPTH_BIT;
            }
            if (create_info.pDepthStencilState->stencilTestEnable) {
                aspects |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
            result.emplace_back(attachment, aspects);
        }
    }
    return result;
}

bp_state::Pipeline::Pipeline(const vvl::Device& state_data, const VkGraphicsPipelineCreateInfo* create_info,
                             std::shared_ptr<const vvl::PipelineCache>&& pipe_cache,
                             std::shared_ptr<const vvl::RenderPass>&& rpstate, std::shared_ptr<const vvl::PipelineLayout>&& layout)
    : vvl::Pipeline(state_data, create_info, std::move(pipe_cache), std::move(rpstate), std::move(layout), nullptr),
      access_framebuffer_attachments(GetAttachmentAccess(*this)) {}

std::shared_ptr<vvl::Pipeline> BestPractices::CreateGraphicsPipelineState(
    const VkGraphicsPipelineCreateInfo* create_info, std::shared_ptr<const vvl::PipelineCache> pipeline_cache,
    std::shared_ptr<const vvl::RenderPass>&& render_pass, std::shared_ptr<const vvl::PipelineLayout>&& layout,
    spirv::StatelessData stateless_data[kCommonMaxGraphicsShaderStages]) const {
    return std::static_pointer_cast<vvl::Pipeline>(std::make_shared<bp_state::Pipeline>(
        *this, create_info, std::move(pipeline_cache), std::move(render_pass), std::move(layout)));
}

void BestPractices::ManualPostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                                const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                                const RecordObject& record_obj, PipelineStates& pipeline_states,
                                                                chassis::CreateGraphicsPipelines& chassis_state) {
    // AMD best practice
    pipeline_cache_ = pipelineCache;
}

bool BestPractices::PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                          const VkComputePipelineCreateInfo* pCreateInfos,
                                                          const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                          const ErrorObject& error_obj, PipelineStates& pipeline_states,
                                                          chassis::CreateComputePipelines& chassis_state) const {
    bool skip = BaseClass::PreCallValidateCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator,
                                                                 pPipelines, error_obj, pipeline_states, chassis_state);

    if ((createInfoCount > 1) && (!pipelineCache)) {
        skip |=
            LogPerformanceWarning("BestPractices-vkCreateComputePipelines-multiple-pipelines-no-cache", device, error_obj.location,
                                  "creating multiple pipelines (createInfoCount is %" PRIu32
                                  ") but is not using a "
                                  "pipeline cache, which may help with performance",
                                  createInfoCount);
    }

    if (VendorCheckEnabled(kBPVendorAMD)) {
        auto prev_pipeline = pipeline_cache_.load();
        if (pipelineCache && prev_pipeline && pipelineCache != prev_pipeline) {
            skip |= LogPerformanceWarning("BestPractices-vkCreateComputePipelines-multiple-cache", device, error_obj.location,
                                          "%s A second pipeline cache is in use. Consider using only one pipeline cache to "
                                          "improve cache hit rate",
                                          VendorSpecificTag(kBPVendorAMD));
        }
    }

    for (uint32_t i = 0; i < createInfoCount; i++) {
        const Location create_info_loc = error_obj.location.dot(Field::pCreateInfos, i);
        const VkComputePipelineCreateInfo& create_info = pCreateInfos[i];
        if (VendorCheckEnabled(kBPVendorArm)) {
            skip |= ValidateCreateComputePipelineArm(create_info, create_info_loc);
        }

        if (VendorCheckEnabled(kBPVendorAMD)) {
            skip |= ValidateCreateComputePipelineAmd(create_info, create_info_loc);
        }

        if (IsExtEnabled(extensions.vk_khr_maintenance4)) {
            auto module_state = Get<vvl::ShaderModule>(create_info.stage.module);
            if (module_state &&
                module_state->spirv->static_data_.has_builtin_workgroup_size) {  // No module if creating from module identifier
                skip |= LogWarning("BestPractices-SpirvDeprecated_WorkgroupSize", device, create_info_loc,
                                   "is using the SPIR-V Workgroup built-in which SPIR-V 1.6 deprecated. When using "
                                   "VK_KHR_maintenance4 or Vulkan 1.3+, the new SPIR-V LocalSizeId execution mode should be used "
                                   "instead. This can be done by recompiling your shader and targeting Vulkan 1.3+.");
            }
        }
    }

    return skip;
}

bool BestPractices::ValidateCreateComputePipelineArm(const VkComputePipelineCreateInfo& create_info,
                                                     const Location& create_info_loc) const {
    bool skip = false;
    auto module_state = Get<vvl::ShaderModule>(create_info.stage.module);
    if (!module_state || !module_state->spirv) {
        return false;  // No module if creating from module identifier
    }

    // Generate warnings about work group sizes based on active resources.
    auto entrypoint = module_state->spirv->FindEntrypoint(create_info.stage.pName, create_info.stage.stage);
    if (!entrypoint) return false;

    uint32_t x = {}, y = {}, z = {};
    if (!module_state->spirv->FindLocalSize(*entrypoint, x, y, z)) {
        return false;
    }

    const uint32_t thread_count = x * y * z;

    // Generate a priori warnings about work group sizes.
    if (thread_count > kMaxEfficientWorkGroupThreadCountArm) {
        skip |= LogPerformanceWarning(
            "BestPractices-Arm-vkCreateComputePipelines-compute-work-group-size", device, create_info_loc,
            "%s compute shader with work group dimensions (%u, %u, "
            "%u) (%u threads total), has more threads than advised in a single work group. It is advised to use work "
            "groups with less than %u threads, especially when using barrier() or shared memory.",
            VendorSpecificTag(kBPVendorArm), x, y, z, thread_count, kMaxEfficientWorkGroupThreadCountArm);
    }

    if (thread_count == 1 || ((x > 1) && (x & (kThreadGroupDispatchCountAlignmentArm - 1))) ||
        ((y > 1) && (y & (kThreadGroupDispatchCountAlignmentArm - 1))) ||
        ((z > 1) && (z & (kThreadGroupDispatchCountAlignmentArm - 1)))) {
        skip |= LogPerformanceWarning(
            "BestPractices-Arm-vkCreateComputePipelines-compute-thread-group-alignment", device, create_info_loc,
            "%s compute shader with work group dimensions (%u, "
            "%u, %u) is not aligned to %u "
            "threads. On Arm Mali architectures, not aligning work group sizes to %u may "
            "leave threads idle on the shader "
            "core.",
            VendorSpecificTag(kBPVendorArm), x, y, z, kThreadGroupDispatchCountAlignmentArm, kThreadGroupDispatchCountAlignmentArm);
    }

    unsigned dimensions = 0;
    if (x > 1) dimensions++;
    if (y > 1) dimensions++;
    if (z > 1) dimensions++;
    // Here the dimension will really depend on the dispatch grid, but assume it's 1D.
    dimensions = std::max(dimensions, 1u);

    // If we're accessing images, we almost certainly want to have a 2D workgroup for cache reasons.
    // There are some false positives here. We could simply have a shader that does this within a 1D grid,
    // or we may have a linearly tiled image, but these cases are quite unlikely in practice.
    bool accesses_2d = false;
    for (const auto& variable : entrypoint->resource_interface_variables) {
        if (variable.info.image_dim != spv::Dim1D && variable.info.image_dim != spv::DimBuffer) {
            accesses_2d = true;
            break;
        }
    }

    if (accesses_2d && dimensions < 2) {
        LogPerformanceWarning("BestPractices-Arm-vkCreateComputePipelines-compute-spatial-locality", device, create_info_loc,
                              "%s compute shader has work group dimensions (%u, %u, %u), which "
                              "suggests a 1D dispatch, but the shader is accessing 2D or 3D images. The shader may be "
                              "exhibiting poor spatial locality with respect to one or more shader resources.",
                              VendorSpecificTag(kBPVendorArm), x, y, z);
    }

    return skip;
}

bool BestPractices::ValidateCreateComputePipelineAmd(const VkComputePipelineCreateInfo& create_info,
                                                     const Location& create_info_loc) const {
    bool skip = false;
    auto module_state = Get<vvl::ShaderModule>(create_info.stage.module);
    if (!module_state || !module_state->spirv) {
        return false;
    }
    auto entrypoint = module_state->spirv->FindEntrypoint(create_info.stage.pName, create_info.stage.stage);
    if (!entrypoint) {
        return false;
    }

    uint32_t x = {}, y = {}, z = {};
    if (!module_state->spirv->FindLocalSize(*entrypoint, x, y, z)) {
        return false;
    }

    const uint32_t thread_count = x * y * z;

    const bool multiple_64 = ((thread_count % 64) == 0);

    if (!multiple_64) {
        skip |= LogPerformanceWarning("BestPractices-AMD-LocalWorkgroup-Multiple64", device, create_info_loc,
                                      "%s compute shader with work group dimensions (%" PRIu32 ", %" PRIu32 ", %" PRIu32
                                      "), workgroup size (%" PRIu32
                                      "), is not a multiple of 64. Make the workgroup size a multiple of 64 to obtain best "
                                      "performance across all AMD GPU generations.",
                                      VendorSpecificTag(kBPVendorAMD), x, y, z, thread_count);
    }

    return skip;
}

void BestPractices::PostCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                  VkPipeline pipeline, const RecordObject& record_obj) {
    BaseClass::PostCallRecordCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline, record_obj);

    // AMD best practice
    PipelineUsedInFrame(pipeline);

    if (pipelineBindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS) {
        // check for depth/blend state tracking
        if (auto pipeline_state = Get<bp_state::Pipeline>(pipeline)) {
            auto cb_state = GetWrite<bp_state::CommandBuffer>(commandBuffer);
            auto& render_pass_state = cb_state->render_pass_state;

            render_pass_state.nextDrawTouchesAttachments = pipeline_state->access_framebuffer_attachments;
            render_pass_state.drawTouchAttachments = true;

            const auto* blend_state = pipeline_state->ColorBlendState();
            const auto* stencil_state = pipeline_state->DepthStencilState();

            if (blend_state && !(pipeline_state->ignore_color_attachments)) {
                // assume the pipeline is depth-only unless any of the attachments have color writes enabled
                render_pass_state.depthOnly = true;
                for (size_t i = 0; i < blend_state->attachmentCount; i++) {
                    if (blend_state->pAttachments[i].colorWriteMask != 0) {
                        render_pass_state.depthOnly = false;
                    }
                }
            }

            // check for depth value usage
            render_pass_state.depthEqualComparison = false;

            if (stencil_state && stencil_state->depthTestEnable) {
                switch (stencil_state->depthCompareOp) {
                    case VK_COMPARE_OP_EQUAL:
                    case VK_COMPARE_OP_GREATER_OR_EQUAL:
                    case VK_COMPARE_OP_LESS_OR_EQUAL:
                        render_pass_state.depthEqualComparison = true;
                        break;
                    default:
                        break;
                }
            }

            if (VendorCheckEnabled(kBPVendorNVIDIA)) {
                using TessGeometryMeshState = bp_state::CommandBufferStateNV::TessGeometryMesh::State;
                auto& tgm = cb_state->nv.tess_geometry_mesh;

                // Make sure the message is only signaled once per command buffer
                tgm.threshold_signaled = tgm.num_switches >= kNumBindPipelineTessGeometryMeshSwitchesThresholdNVIDIA;

                // Track pipeline switches with tessellation, geometry, and/or mesh shaders enabled, and disabled
                auto tgm_stages = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT |
                                  VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT;
                auto new_tgm_state = (pipeline_state->active_shaders & tgm_stages) != 0 ? TessGeometryMeshState::Enabled
                                                                                        : TessGeometryMeshState::Disabled;
                if (tgm.state != new_tgm_state && tgm.state != TessGeometryMeshState::Unknown) {
                    tgm.num_switches++;
                }
                tgm.state = new_tgm_state;

                // Track depthTestEnable and depthCompareOp
                auto& pipeline_create_info = pipeline_state->GraphicsCreateInfo();
                auto depth_stencil_state = pipeline_create_info.pDepthStencilState;
                auto dynamic_state = pipeline_create_info.pDynamicState;
                if (depth_stencil_state && dynamic_state) {
                    auto dynamic_state_begin = dynamic_state->pDynamicStates;
                    auto dynamic_state_end = dynamic_state->pDynamicStates + dynamic_state->dynamicStateCount;

                    const bool dynamic_depth_test_enable =
                        std::find(dynamic_state_begin, dynamic_state_end, VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE) != dynamic_state_end;
                    const bool dynamic_depth_func =
                        std::find(dynamic_state_begin, dynamic_state_end, VK_DYNAMIC_STATE_DEPTH_COMPARE_OP) != dynamic_state_end;

                    if (!dynamic_depth_test_enable) {
                        RecordSetDepthTestState(*cb_state, cb_state->nv.depth_compare_op,
                                                depth_stencil_state->depthTestEnable != VK_FALSE);
                    }
                    if (!dynamic_depth_func) {
                        RecordSetDepthTestState(*cb_state, depth_stencil_state->depthCompareOp, cb_state->nv.depth_test_enable);
                    }
                }
            }
        }
    }
}

void BestPractices::PreCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                         const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                         const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                         const RecordObject& record_obj) {
    BaseClass::PreCallRecordCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator,
                                                                 pPipelines, record_obj);
    // AMD best practice
    num_pso_ += createInfoCount;
}

bool BestPractices::PreCallValidateCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout,
                                                        const ErrorObject& error_obj) const {
    bool skip = false;
    if (VendorCheckEnabled(kBPVendorAMD)) {
        uint32_t descriptor_size = enabled_features.robustBufferAccess ? 4 : 2;
        // Descriptor sets cost 1 DWORD each.
        // Dynamic buffers cost 2 DWORDs each when robust buffer access is OFF.
        // Dynamic buffers cost 4 DWORDs each when robust buffer access is ON.
        // Push constants cost 1 DWORD per 4 bytes in the Push constant range.
        uint32_t pipeline_size = pCreateInfo->setLayoutCount;  // in DWORDS
        for (uint32_t i = 0; i < pCreateInfo->setLayoutCount; i++) {
            auto descriptor_set_layout_state = Get<vvl::DescriptorSetLayout>(pCreateInfo->pSetLayouts[i]);
            if (!descriptor_set_layout_state) continue;
            pipeline_size += descriptor_set_layout_state->GetDynamicDescriptorCount() * descriptor_size;
        }

        for (uint32_t i = 0; i < pCreateInfo->pushConstantRangeCount; i++) {
            pipeline_size += pCreateInfo->pPushConstantRanges[i].size / 4;
        }

        if (pipeline_size > kPipelineLayoutSizeWarningLimitAMD) {
            skip |= LogPerformanceWarning("BestPractices-AMD-CreatePipelinesLayout-KeepLayoutSmall", device, error_obj.location,
                                          "%s pipeline layout size is too large. Prefer smaller pipeline layouts."
                                          "Descriptor sets cost 1 DWORD each. "
                                          "Dynamic buffers cost 2 DWORDs each when robust buffer access is OFF. "
                                          "Dynamic buffers cost 4 DWORDs each when robust buffer access is ON. "
                                          "Push constants cost 1 DWORD per 4 bytes in the Push constant range. ",
                                          VendorSpecificTag(kBPVendorAMD));
        }
    }

    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        bool has_separate_sampler = false;
        size_t fast_space_usage = 0;

        for (uint32_t i = 0; i < pCreateInfo->setLayoutCount; ++i) {
            auto descriptor_set_layout_state = Get<vvl::DescriptorSetLayout>(pCreateInfo->pSetLayouts[i]);
            if (!descriptor_set_layout_state) continue;
            for (const auto& binding : descriptor_set_layout_state->GetBindings()) {
                if (binding.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER) {
                    has_separate_sampler = true;
                }

                if ((descriptor_set_layout_state->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT) ==
                    0U) {
                    size_t descriptor_type_size = 0;

                    switch (binding.descriptorType) {
                        case VK_DESCRIPTOR_TYPE_SAMPLER:
                        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                            descriptor_type_size = 4;
                            break;
                        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
                        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
                            descriptor_type_size = 8;
                            break;
                        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                        case VK_DESCRIPTOR_TYPE_MUTABLE_EXT:
                            descriptor_type_size = 16;
                            break;
                        case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:
                            descriptor_type_size = 1;
                            break;
                        default:
                            // Unknown type.
                            break;
                    }

                    size_t descriptor_size = descriptor_type_size * binding.descriptorCount;
                    fast_space_usage += descriptor_size;
                }
            }
        }

        if (has_separate_sampler) {
            skip |= LogPerformanceWarning(
                "BestPractices-NVIDIA-CreatePipelineLayout-SeparateSampler", device, error_obj.location,
                "%s Consider using combined image samplers instead of separate samplers for marginally better performance.",
                VendorSpecificTag(kBPVendorNVIDIA));
        }

        if (fast_space_usage > kPipelineLayoutFastDescriptorSpaceNVIDIA) {
            skip |= LogPerformanceWarning(
                "BestPractices-NVIDIA-CreatePipelineLayout-LargePipelineLayout", device, error_obj.location,
                "%s Pipeline layout size is too large, prefer using pipeline-specific descriptor set layouts. "
                "Aim for consuming less than %" PRIu32
                " bytes to allow fast reads for all non-bindless descriptors. "
                "Samplers, textures, texel buffers, and combined image samplers consume 4 bytes each. "
                "Uniform buffers and acceleration structures consume 8 bytes. "
                "Storage buffers consume 16 bytes. "
                "Push constants do not consume space.",
                VendorSpecificTag(kBPVendorNVIDIA), kPipelineLayoutFastDescriptorSpaceNVIDIA);
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                   VkPipeline pipeline, const ErrorObject& error_obj) const {
    bool skip = false;

    if (VendorCheckEnabled(kBPVendorAMD) || VendorCheckEnabled(kBPVendorNVIDIA)) {
        if (IsPipelineUsedInFrame(pipeline)) {
            skip |= LogPerformanceWarning(
                "BestPractices-Pipeline-SortAndBind", commandBuffer, error_obj.location,
                "%s %s Pipeline %s was bound twice in the frame. "
                "Keep pipeline state changes to a minimum, for example, by sorting draw calls by pipeline.",
                VendorSpecificTag(kBPVendorAMD), VendorSpecificTag(kBPVendorNVIDIA), FormatHandle(pipeline).c_str());
        }
    }
    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        auto cb_state = Get<bp_state::CommandBuffer>(commandBuffer);
        const auto& tgm = cb_state->nv.tess_geometry_mesh;
        if (tgm.num_switches >= kNumBindPipelineTessGeometryMeshSwitchesThresholdNVIDIA && !tgm.threshold_signaled) {
            LogPerformanceWarning("BestPractices-NVIDIA-BindPipeline-SwitchTessGeometryMesh", commandBuffer, error_obj.location,
                                  "%s Avoid switching between pipelines with and without tessellation, geometry, task, "
                                  "and/or mesh shaders. Group draw calls using these shader stages together.",
                                  VendorSpecificTag(kBPVendorNVIDIA));
            // Do not set 'skip' so the number of switches gets properly counted after the message.
        }
    }

    return skip;
}
