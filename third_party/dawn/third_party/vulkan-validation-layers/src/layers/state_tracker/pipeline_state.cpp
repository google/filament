/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
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
#include "state_tracker/pipeline_state.h"
#include "generated/dynamic_state_helper.h"
#include "state_tracker/descriptor_sets.h"
#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/render_pass_state.h"
#include "state_tracker/state_object.h"
#include "generated/enum_flag_bits.h"
#include "state_tracker/shader_object_state.h"
#include "chassis/chassis_modification_state.h"

namespace vvl {

static vku::safe_VkGraphicsPipelineCreateInfo MakeGraphicsCreateInfo(const VkGraphicsPipelineCreateInfo &ci,
                                                                     std::shared_ptr<const vvl::RenderPass> rpstate,
                                                                     const Device &state_data) {
    bool use_color = false;
    bool use_depth_stencil = false;

    if (ci.renderPass == VK_NULL_HANDLE) {
        auto dynamic_rendering = vku::FindStructInPNextChain<VkPipelineRenderingCreateInfo>(ci.pNext);
        if (dynamic_rendering) {
            use_color = (dynamic_rendering->colorAttachmentCount > 0);
            use_depth_stencil = (dynamic_rendering->depthAttachmentFormat != VK_FORMAT_UNDEFINED) ||
                                (dynamic_rendering->stencilAttachmentFormat != VK_FORMAT_UNDEFINED);
        } else {
            // if this is true, will be caught later by VU
            use_color = ci.pColorBlendState && ci.pColorBlendState->attachmentCount > 0;
        }
    } else if (rpstate) {
        use_color = rpstate->UsesColorAttachment(ci.subpass);
        use_depth_stencil = rpstate->UsesDepthStencilAttachment(ci.subpass);
    }

    vku::PNextCopyState copy_state = {
        [&state_data, &ci](VkBaseOutStructure *safe_struct, const VkBaseOutStructure *in_struct) -> bool {
            return Pipeline::PnextRenderingInfoCustomCopy(&state_data, ci, safe_struct, in_struct);
        }};
    return vku::safe_VkGraphicsPipelineCreateInfo(&ci, use_color, use_depth_stencil, &copy_state);
}

// static
std::vector<ShaderStageState> Pipeline::GetStageStates(const Device &state_data, const Pipeline &pipe_state,
                                                       spirv::StatelessData *stateless_data) {
    std::vector<ShaderStageState> stage_states;

    std::vector<VkShaderStageFlagBits> lookup_in_library_stages = {VK_SHADER_STAGE_VERTEX_BIT,
                                                                   VK_SHADER_STAGE_FRAGMENT_BIT,
                                                                   VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                                                   VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
                                                                   VK_SHADER_STAGE_GEOMETRY_BIT,
                                                                   VK_SHADER_STAGE_TASK_BIT_EXT,
                                                                   VK_SHADER_STAGE_MESH_BIT_EXT};

    for (size_t stage_index = 0; stage_index < pipe_state.shader_stages_ci.size(); ++stage_index) {
        if (pipe_state.pipeline_type == VK_PIPELINE_BIND_POINT_GRAPHICS &&
            !pipe_state.OwnsSubState(pipe_state.fragment_shader_state) && !pipe_state.OwnsSubState(pipe_state.pre_raster_state)) {
            continue;  // pStages are ignored if not using one of these sub-states
        }

        const auto &stage_ci = pipe_state.shader_stages_ci[stage_index];
        auto module_state = state_data.Get<vvl::ShaderModule>(stage_ci.module);
        if (!module_state && pipe_state.pipeline_cache) {
            // Attempt to look up the pipeline cache for shader module data
            module_state = pipe_state.pipeline_cache->GetStageModule(pipe_state, stage_index);
        }
        if (!module_state) {
            // See if the module is referenced in a library sub state
            module_state = pipe_state.GetSubStateShader(stage_ci.stage);
        }

        if (!module_state || !module_state->spirv) {
            // If module is null and there is a VkShaderModuleCreateInfo in the pNext chain of the stage info, then this
            // module is part of a library and the state must be created
            // This support was also added in VK_KHR_maintenance5
            if (const auto shader_ci = vku::FindStructInPNextChain<VkShaderModuleCreateInfo>(stage_ci.pNext)) {
                // don't need to worry about GroupDecoration in GPL
                auto spirv_module = std::make_shared<spirv::Module>(shader_ci->codeSize, shader_ci->pCode, stateless_data);
                module_state = std::make_shared<vvl::ShaderModule>(VK_NULL_HANDLE, spirv_module);
                if (stateless_data) {
                    stateless_data->pipeline_pnext_module = spirv_module;
                }
            } else {
                // VK_EXT_shader_module_identifier could legally provide a null module handle
                module_state = std::make_shared<vvl::ShaderModule>();
            }
        }

        stage_states.emplace_back(&stage_ci, nullptr, module_state, module_state->spirv);

        // If stage was found, do not try to look for it in library
        auto found_stage = std::find(lookup_in_library_stages.begin(), lookup_in_library_stages.end(), stage_ci.stage);
        if (found_stage != lookup_in_library_stages.end()) {
            const size_t last_library_stage_i = lookup_in_library_stages.size() - 1;
            std::swap(*found_stage, lookup_in_library_stages[last_library_stage_i]);
            lookup_in_library_stages.resize(last_library_stage_i);
        }
    }

    for (const auto &stage_flag : lookup_in_library_stages) {
        // Check if stage has been supplied by a library
        std::shared_ptr<const vvl::ShaderModule> module_state = nullptr;
        const vku::safe_VkPipelineShaderStageCreateInfo *stage_ci = nullptr;
        switch (stage_flag) {
            case VK_SHADER_STAGE_VERTEX_BIT:
                if (pipe_state.pre_raster_state && pipe_state.pre_raster_state->vertex_shader) {
                    module_state = pipe_state.pre_raster_state->vertex_shader;
                    stage_ci = pipe_state.pre_raster_state->vertex_shader_ci;
                }
                break;
            case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
                if (pipe_state.pre_raster_state && pipe_state.pre_raster_state->tessc_shader) {
                    module_state = pipe_state.pre_raster_state->tessc_shader;
                    stage_ci = pipe_state.pre_raster_state->tessc_shader_ci;
                }
                break;
            case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
                if (pipe_state.pre_raster_state && pipe_state.pre_raster_state->tesse_shader) {
                    module_state = pipe_state.pre_raster_state->tesse_shader;
                    stage_ci = pipe_state.pre_raster_state->tesse_shader_ci;
                }
                break;
            case VK_SHADER_STAGE_GEOMETRY_BIT:
                if (pipe_state.pre_raster_state && pipe_state.pre_raster_state->geometry_shader) {
                    module_state = pipe_state.pre_raster_state->geometry_shader;
                    stage_ci = pipe_state.pre_raster_state->geometry_shader_ci;
                }
                break;
            case VK_SHADER_STAGE_TASK_BIT_EXT:
                if (pipe_state.pre_raster_state && pipe_state.pre_raster_state->task_shader) {
                    module_state = pipe_state.pre_raster_state->task_shader;
                    stage_ci = pipe_state.pre_raster_state->task_shader_ci;
                }
                break;
            case VK_SHADER_STAGE_MESH_BIT_EXT:
                if (pipe_state.pre_raster_state && pipe_state.pre_raster_state->mesh_shader) {
                    module_state = pipe_state.pre_raster_state->mesh_shader;
                    stage_ci = pipe_state.pre_raster_state->mesh_shader_ci;
                }
                break;
            case VK_SHADER_STAGE_FRAGMENT_BIT:
                if (pipe_state.fragment_shader_state && pipe_state.fragment_shader_state->fragment_shader) {
                    module_state = pipe_state.fragment_shader_state->fragment_shader;
                    stage_ci = pipe_state.fragment_shader_state->fragment_shader_ci.get();
                }
                break;
            default:
                // no-op
                break;
        }
        if (!stage_ci) {
            continue;
        }

        stage_states.emplace_back(stage_ci, nullptr, module_state, module_state->spirv);
    }
    return stage_states;
}

static uint32_t GetCreateInfoShaders(const Pipeline &pipe_state) {
    uint32_t result = 0;
    if (pipe_state.pipeline_type == VK_PIPELINE_BIND_POINT_GRAPHICS && !pipe_state.OwnsSubState(pipe_state.fragment_shader_state) &&
        !pipe_state.OwnsSubState(pipe_state.pre_raster_state)) {
        return result;  // pStages are ignored if not using one of these substates
    }

    for (const auto &stage_ci : pipe_state.shader_stages_ci) {
        result |= stage_ci.stage;
    }
    return result;
}

static uint32_t GetLinkingShaders(const VkPipelineLibraryCreateInfoKHR *link_info, const Device &state_data) {
    uint32_t result = 0;
    if (link_info) {
        for (uint32_t i = 0; i < link_info->libraryCount; ++i) {
            const auto &state = state_data.Get<vvl::Pipeline>(link_info->pLibraries[i]);
            if (state) {
                result |= state->active_shaders;
            }
        }
    }
    return result;
}

static CBDynamicFlags GetGraphicsDynamicState(Pipeline &pipe_state) {
    CBDynamicFlags flags = 0;

    // "Dynamic state values set via pDynamicState must be ignored if the state they correspond to is not otherwise statically set
    // by one of the state subsets used to create the pipeline."
    //
    // we only care here if the pipeline was created with the subset, not linked
    const bool has_vertex_input_state = pipe_state.OwnsSubState(pipe_state.vertex_input_state);
    const bool has_pre_raster_state = pipe_state.OwnsSubState(pipe_state.pre_raster_state);
    const bool has_fragment_shader_state = pipe_state.OwnsSubState(pipe_state.fragment_shader_state);
    const bool has_fragment_output_state = pipe_state.OwnsSubState(pipe_state.fragment_output_state);

    const auto *dynamic_state_ci = pipe_state.GraphicsCreateInfo().pDynamicState;
    if (dynamic_state_ci) {
        for (const VkDynamicState vk_dynamic_state :
             vvl::make_span(dynamic_state_ci->pDynamicStates, dynamic_state_ci->dynamicStateCount)) {
            // Check if should ignore or not before converting and adding
            switch (vk_dynamic_state) {
                // VkPipelineVertexInputStateCreateInfo
                case VK_DYNAMIC_STATE_VERTEX_INPUT_EXT:
                case VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE:
                // VkPipelineInputAssemblyStateCreateInfo
                case VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY:
                case VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE: {
                    if (has_vertex_input_state) {
                        flags.set(ConvertToCBDynamicState(vk_dynamic_state));
                    }
                    break;
                }

                // VkPipelineViewportStateCreateInfo
                case VK_DYNAMIC_STATE_VIEWPORT:
                case VK_DYNAMIC_STATE_SCISSOR:
                case VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT:
                case VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT:
                case VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV:
                case VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV:
                case VK_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV:
                case VK_DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER_NV:
                case VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV:
                case VK_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV:
                case VK_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT:
                case VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_ENABLE_NV:
                case VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV:
                // VkPipelineRasterizationStateCreateInfo
                case VK_DYNAMIC_STATE_LINE_WIDTH:
                case VK_DYNAMIC_STATE_DEPTH_BIAS:
                case VK_DYNAMIC_STATE_CULL_MODE:
                case VK_DYNAMIC_STATE_FRONT_FACE:
                case VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE:
                case VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE:
                case VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT:
                case VK_DYNAMIC_STATE_POLYGON_MODE_EXT:
                case VK_DYNAMIC_STATE_LINE_STIPPLE:
                case VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT:
                case VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT:
                case VK_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT:
                case VK_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT:
                case VK_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT:
                case VK_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT:
                case VK_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT:
                case VK_DYNAMIC_STATE_DEPTH_CLAMP_RANGE_EXT:
                // VkPipelineTessellationStateCreateInfo
                case VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT:
                case VK_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT:
                // VkPipelineDiscardRectangleStateCreateInfoEXT
                case VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT:
                case VK_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT:
                case VK_DYNAMIC_STATE_DISCARD_RECTANGLE_MODE_EXT: {
                    if (has_pre_raster_state) {
                        flags.set(ConvertToCBDynamicState(vk_dynamic_state));
                    }
                    break;
                }

                // VkPipelineFragmentShadingRateStateCreateInfoKHR
                case VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR: {
                    if (has_pre_raster_state || has_fragment_shader_state) {
                        flags.set(ConvertToCBDynamicState(vk_dynamic_state));
                    }
                    break;
                }

                // VkPipelineDepthStencilStateCreateInfo
                case VK_DYNAMIC_STATE_DEPTH_BOUNDS:
                case VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE:
                case VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK:
                case VK_DYNAMIC_STATE_STENCIL_WRITE_MASK:
                case VK_DYNAMIC_STATE_STENCIL_REFERENCE:
                case VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE:
                case VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE:
                case VK_DYNAMIC_STATE_DEPTH_COMPARE_OP:
                case VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE:
                case VK_DYNAMIC_STATE_STENCIL_OP:
                // VkPipelineRepresentativeFragmentTestStateCreateInfoNV
                case VK_DYNAMIC_STATE_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV: {
                    if (has_fragment_shader_state) {
                        flags.set(ConvertToCBDynamicState(vk_dynamic_state));
                    }
                    break;
                }

                /// VkPipelineColorBlendStateCreateInfo
                case VK_DYNAMIC_STATE_BLEND_CONSTANTS:
                case VK_DYNAMIC_STATE_LOGIC_OP_EXT:
                case VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT:
                case VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT:
                case VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT:
                case VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT:
                case VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT:
                case VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT: {
                    if (has_fragment_output_state) {
                        flags.set(ConvertToCBDynamicState(vk_dynamic_state));
                    }
                    break;
                }

                // VkPipelineMultisampleStateCreateInfo
                case VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT:
                case VK_DYNAMIC_STATE_SAMPLE_MASK_EXT:
                case VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT:
                case VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT:
                case VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT:
                case VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT:
                case VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV:
                case VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV:
                case VK_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV:
                case VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV:
                case VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_NV:
                case VK_DYNAMIC_STATE_COVERAGE_REDUCTION_MODE_NV: {
                    if (has_fragment_shader_state || has_fragment_output_state) {
                        flags.set(ConvertToCBDynamicState(vk_dynamic_state));
                    }
                    break;
                }

                // VkRenderPass and subpass parameter
                case VK_DYNAMIC_STATE_ATTACHMENT_FEEDBACK_LOOP_ENABLE_EXT: {
                    if (has_pre_raster_state || has_fragment_shader_state || has_fragment_output_state) {
                        flags.set(ConvertToCBDynamicState(vk_dynamic_state));
                    }
                    break;
                }

                // not valid and shouldn't see
                case VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR:
                case VK_DYNAMIC_STATE_MAX_ENUM:
                    assert(false);
                    break;
            }
        }
    }

    // apply linked library's dynamic state
    if (!has_vertex_input_state && pipe_state.vertex_input_state) {
        flags |= pipe_state.vertex_input_state->parent.dynamic_state;
    }
    if (!has_pre_raster_state && pipe_state.pre_raster_state) {
        flags |= pipe_state.pre_raster_state->parent.dynamic_state;
    }
    if (!has_fragment_shader_state && pipe_state.fragment_shader_state) {
        flags |= pipe_state.fragment_shader_state->parent.dynamic_state;
    }
    if (!has_fragment_output_state && pipe_state.fragment_output_state) {
        flags |= pipe_state.fragment_output_state->parent.dynamic_state;
    }
    return flags;
}

static CBDynamicFlags GetRayTracingDynamicState(Pipeline &pipe_state) {
    CBDynamicFlags flags = 0;

    const auto *dynamic_state_ci = pipe_state.RayTracingCreateInfo().pDynamicState;
    if (dynamic_state_ci) {
        for (const VkDynamicState vk_dynamic_state :
             vvl::make_span(dynamic_state_ci->pDynamicStates, dynamic_state_ci->dynamicStateCount)) {
            switch (vk_dynamic_state) {
                case VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR:
                    flags.set(ConvertToCBDynamicState(vk_dynamic_state));
                    break;
                default:
                    break;
            }
        }
    }

    return flags;
}

static bool UsesPipelineRobustness(const void *pNext, const Pipeline &pipe_state) {
    bool result = false;
    const auto robustness_info = vku::FindStructInPNextChain<VkPipelineRobustnessCreateInfo>(pNext);
    if (!robustness_info) {
        return false;
    }
    result |= (robustness_info->storageBuffers == VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_2) ||
              (robustness_info->storageBuffers == VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS);
    result |= (robustness_info->uniformBuffers == VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_2) ||
              (robustness_info->uniformBuffers == VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS);
    if (!result) {
        for (const auto &stage_ci : pipe_state.shader_stages_ci) {
            const auto stage_robustness_info = vku::FindStructInPNextChain<VkPipelineRobustnessCreateInfo>(stage_ci.pNext);
            if (stage_robustness_info) {
                result |=
                    (stage_robustness_info->storageBuffers == VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_2) ||
                    (stage_robustness_info->storageBuffers == VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS);
                result |=
                    (stage_robustness_info->uniformBuffers == VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_2) ||
                    (stage_robustness_info->uniformBuffers == VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS);
            }
        }
    }
    return result;
}

static bool UsesPipelineVertexRobustness(const void *pNext, const Pipeline &pipe_state) {
    bool result = false;
    const auto robustness_info = vku::FindStructInPNextChain<VkPipelineRobustnessCreateInfo>(pNext);
    if (!robustness_info) {
        return false;
    }
    result |= (robustness_info->vertexInputs == VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_2) ||
              (robustness_info->vertexInputs == VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS);
    if (!result) {
        for (const auto &stage_ci : pipe_state.shader_stages_ci) {
            const auto stage_robustness_info = vku::FindStructInPNextChain<VkPipelineRobustnessCreateInfo>(stage_ci.pNext);
            if (stage_robustness_info) {
                result |= (stage_robustness_info->vertexInputs == VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_2) ||
                          (stage_robustness_info->vertexInputs == VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS);
            }
        }
    }
    return result;
}

static bool IgnoreColorAttachments(const Device &state_data, Pipeline &pipe_state) {
    // If the libraries used to create this pipeline are ignoring color attachments, this pipeline should as well
    if (pipe_state.library_create_info) {
        for (uint32_t i = 0; i < pipe_state.library_create_info->libraryCount; i++) {
            const auto lib = state_data.Get<vvl::Pipeline>(pipe_state.library_create_info->pLibraries[i]);
            if (lib->ignore_color_attachments) return true;
        }
    }
    // According to the spec, pAttachments is to be ignored if the pipeline is created with
    // VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT, VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT, VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT
    // and VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT dynamic states set
    return pipe_state.ColorBlendState() && (pipe_state.IsDynamic(CB_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT) &&
                                            pipe_state.IsDynamic(CB_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT) &&
                                            pipe_state.IsDynamic(CB_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT) &&
                                            pipe_state.IsDynamic(CB_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT));
}

static bool UsesShaderModuleId(const Pipeline &pipe_state) {
    for (const auto &stage_ci : pipe_state.shader_stages_ci) {
        // if using GPL, can have null pStages
        if (stage_ci.ptr()) {
            const auto module_id_info =
                vku::FindStructInPNextChain<VkPipelineShaderStageModuleIdentifierCreateInfoEXT>(stage_ci.pNext);
            if (module_id_info && (module_id_info->identifierSize > 0)) {
                return true;
            }
        }
    }
    return false;
}

static vvl::unordered_set<uint32_t> GetFSOutputLocations(const std::vector<ShaderStageState> &stage_states) {
    vvl::unordered_set<uint32_t> result;
    for (const auto &stage_state : stage_states) {
        if (!stage_state.entrypoint) {
            continue;
        }
        if (stage_state.GetStage() == VK_SHADER_STAGE_FRAGMENT_BIT) {
            for (const auto *variable : stage_state.entrypoint->user_defined_interface_variables) {
                if ((variable->storage_class != spv::StorageClassOutput) || variable->interface_slots.empty()) {
                    continue;  // not an output interface
                }
                // It is not allowed to have Block Fragment or 64-bit vectors output in Frag shader
                // This means all Locations in slots will be the same
                result.insert(variable->interface_slots[0].Location());
            }
            break;  // found
        }
    }
    return result;
}

static VkPrimitiveTopology GetTopologyAtRasterizer(const Pipeline &pipeline) {
    auto result = (pipeline.InputAssemblyState()) ? pipeline.InputAssemblyState()->topology : VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;

    // When dealing with GPL, if there is no Vertex Input stage, check if the topology was linked in, but don't go searching in
    // shaders, otherwise a pre-raster GPL stage can have a mismatched topology
    if (!pipeline.OwnsSubState(pipeline.vertex_input_state)) {
        return result;
    }

    for (const auto &stage : pipeline.stage_states) {
        if (!stage.entrypoint) {
            continue;
        }
        auto stage_topo = stage.spirv_state->GetTopology(*stage.entrypoint);
        if (stage_topo) {
            result = *stage_topo;
        }
    }
    return result;
}

static VkPipelineCreateFlags2KHR GetPipelineCreateFlags(const void *pNext, VkPipelineCreateFlags flags) {
    const auto flags2 = vku::FindStructInPNextChain<VkPipelineCreateFlags2CreateInfo>(pNext);
    if (flags2) {
        return flags2->flags;
    }
    return flags;
}

// static
std::shared_ptr<VertexInputState> Pipeline::CreateVertexInputState(const Pipeline &p, const Device &state,
                                                                   const vku::safe_VkGraphicsPipelineCreateInfo &create_info) {
    const auto lib_type = GetGraphicsLibType(create_info);
    if (lib_type & VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT) {  // Vertex input graphics library
        return std::make_shared<VertexInputState>(p, create_info);
    }

    if (p.library_create_info) {
        auto ss = GetLibSubState<VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT>(state, *p.library_create_info);
        if (ss) {
            return ss;
        }
    } else {
        if (lib_type == static_cast<VkGraphicsPipelineLibraryFlagsEXT>(0)) {  // Not a graphics library
            return std::make_shared<VertexInputState>(p, create_info);
        }
    }

    // We shouldn't get here...
    return {};
}

// static
std::shared_ptr<PreRasterState> Pipeline::CreatePreRasterState(
    const Pipeline &p, const Device &state, const vku::safe_VkGraphicsPipelineCreateInfo &create_info,
    const std::shared_ptr<const vvl::RenderPass> &rp, spirv::StatelessData stateless_data[kCommonMaxGraphicsShaderStages]) {
    const auto lib_type = GetGraphicsLibType(create_info);
    if (lib_type & VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT) {  // Pre-raster graphics library
        return std::make_shared<PreRasterState>(p, state, create_info, rp, stateless_data);
    }

    if (p.library_create_info) {
        auto ss = GetLibSubState<VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT>(state, *p.library_create_info);
        if (ss) {
            return ss;
        }
    } else {
        if (lib_type == static_cast<VkGraphicsPipelineLibraryFlagsEXT>(0)) {  // Not a graphics library
            return std::make_shared<PreRasterState>(p, state, create_info, rp, stateless_data);
        }
    }

    // We shouldn't get here...
    return {};
}

// static
std::shared_ptr<FragmentShaderState> Pipeline::CreateFragmentShaderState(
    const Pipeline &p, const Device &state, const VkGraphicsPipelineCreateInfo &create_info,
    const vku::safe_VkGraphicsPipelineCreateInfo &safe_create_info, const std::shared_ptr<const vvl::RenderPass> &rp,
    spirv::StatelessData stateless_data[kCommonMaxGraphicsShaderStages]) {
    const auto lib_type = GetGraphicsLibType(create_info);

    if (lib_type & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT) {  // Fragment shader graphics library
        return std::make_shared<FragmentShaderState>(p, state, create_info, rp, stateless_data);
    }

    if (p.library_create_info) {
        auto ss = GetLibSubState<VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT>(state, *p.library_create_info);
        if (ss && EnablesRasterizationStates(p.pre_raster_state)) {
            return ss;
        }
    } else {
        if ((lib_type == static_cast<VkGraphicsPipelineLibraryFlagsEXT>(0)) &&  // Not a graphics library
            EnablesRasterizationStates(p.pre_raster_state)) {
            // No fragment shader _should_ imply no fragment shader state, however, for historical (GL) reasons, a pipeline _can_
            // be created with a VS but no FS and still have valid fragment shader state.
            // See https://gitlab.khronos.org/vulkan/vulkan/-/issues/3178 for more details.
            return std::make_shared<FragmentShaderState>(p, state, safe_create_info, rp, stateless_data);
        }
    }

    // The conditions for containing FS state were not met, so return null
    return {};
}

// static
// Pointers that should be ignored have been set to null in safe_create_info, but if this is a graphics library we need the "raw"
// create_info.
std::shared_ptr<FragmentOutputState> Pipeline::CreateFragmentOutputState(
    const Pipeline &p, const Device &state, const VkGraphicsPipelineCreateInfo &create_info,
    const vku::safe_VkGraphicsPipelineCreateInfo &safe_create_info, const std::shared_ptr<const vvl::RenderPass> &rp) {
    // If this pipeline is being created a non-executable (i.e., does not contain complete state) pipeline with FO state, then
    // unconditionally set this pipeline's FO state.
    const auto lib_type = GetGraphicsLibType(create_info);
    if (lib_type & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT) {  // Fragment output graphics library
        return std::make_shared<FragmentOutputState>(p, create_info, rp);
    }

    if (p.library_create_info) {
        auto ss = GetLibSubState<VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT>(state, *p.library_create_info);
        // If this pipeline is linking in a library that contains FO state, check to see if the FO state is valid before creating it
        // for this pipeline
        if (ss && EnablesRasterizationStates(p.pre_raster_state)) {
            return ss;
        }
    } else {
        // This is a complete pipeline that does not link to any graphics libraries. Check its create info to see if it has valid FO
        // state
        if ((lib_type == static_cast<VkGraphicsPipelineLibraryFlagsEXT>(0)) &&  // Not a graphics library
            EnablesRasterizationStates(p.pre_raster_state)) {
            return std::make_shared<FragmentOutputState>(p, safe_create_info, rp);
        }
    }

    // The conditions for containing FO state were not met, so return null
    return {};
}

std::vector<std::shared_ptr<const vvl::PipelineLayout>> Pipeline::PipelineLayoutStateUnion() const {
    std::vector<std::shared_ptr<const vvl::PipelineLayout>> ret;
    ret.reserve(2);
    // Only need to check pre-raster _or_ fragment shader layout; if either one is not merged_graphics_layout, then
    // merged_graphics_layout is a union
    if (pre_raster_state) {
        if (pre_raster_state->pipeline_layout != fragment_shader_state->pipeline_layout) {
            return {pre_raster_state->pipeline_layout, fragment_shader_state->pipeline_layout};
        } else {
            return {pre_raster_state->pipeline_layout};
        }
    }
    return {merged_graphics_layout};
}

// TODO (ncesario) this needs to be automated. As a first step, need to leverage SubState::ValidShaderStages()
// Currently will return vvl::ShaderModule with no SPIR-V
std::shared_ptr<const vvl::ShaderModule> Pipeline::GetSubStateShader(VkShaderStageFlagBits state) const {
    switch (state) {
        case VK_SHADER_STAGE_VERTEX_BIT: {
            const auto sub_state = Pipeline::GetSubState<VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT>(*this);
            return (sub_state) ? sub_state->vertex_shader : nullptr;
            break;
        }
        case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: {
            const auto sub_state = Pipeline::GetSubState<VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT>(*this);
            return (sub_state) ? sub_state->tessc_shader : nullptr;
            break;
        }
        case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: {
            const auto sub_state = Pipeline::GetSubState<VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT>(*this);
            return (sub_state) ? sub_state->tesse_shader : nullptr;
            break;
        }
        case VK_SHADER_STAGE_GEOMETRY_BIT: {
            const auto sub_state = Pipeline::GetSubState<VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT>(*this);
            return (sub_state) ? sub_state->geometry_shader : nullptr;
            break;
        }
        case VK_SHADER_STAGE_TASK_BIT_EXT: {
            const auto sub_state = Pipeline::GetSubState<VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT>(*this);
            return (sub_state) ? sub_state->task_shader : nullptr;
            break;
        }
        case VK_SHADER_STAGE_MESH_BIT_EXT: {
            const auto sub_state = Pipeline::GetSubState<VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT>(*this);
            return (sub_state) ? sub_state->mesh_shader : nullptr;
            break;
        }
        case VK_SHADER_STAGE_FRAGMENT_BIT: {
            const auto sub_state = Pipeline::GetSubState<VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT>(*this);
            return (sub_state) ? sub_state->fragment_shader : nullptr;
            break;
        };
        default:
            return {};
    }
}

Pipeline::Pipeline(const Device &state_data, const VkGraphicsPipelineCreateInfo *pCreateInfo,
                   std::shared_ptr<const vvl::PipelineCache> &&pipe_cache, std::shared_ptr<const vvl::RenderPass> &&rpstate,
                   std::shared_ptr<const vvl::PipelineLayout> &&layout,
                   spirv::StatelessData stateless_data[kCommonMaxGraphicsShaderStages])
    : StateObject(static_cast<VkPipeline>(VK_NULL_HANDLE), kVulkanObjectTypePipeline),
      rp_state(rpstate),
      create_info(MakeGraphicsCreateInfo(*pCreateInfo, rpstate, state_data)),
      pipeline_cache(std::move(pipe_cache)),
      rendering_create_info(vku::FindStructInPNextChain<VkPipelineRenderingCreateInfo>(GraphicsCreateInfo().pNext)),
      library_create_info(vku::FindStructInPNextChain<VkPipelineLibraryCreateInfoKHR>(GraphicsCreateInfo().pNext)),
      graphics_lib_type(GetGraphicsLibType(GraphicsCreateInfo())),
      pipeline_type(VK_PIPELINE_BIND_POINT_GRAPHICS),
      create_flags(GetPipelineCreateFlags(GraphicsCreateInfo().pNext, GraphicsCreateInfo().flags)),
      shader_stages_ci(GraphicsCreateInfo().pStages, GraphicsCreateInfo().stageCount),
      uses_shader_module_id(UsesShaderModuleId(*this)),
      vertex_input_state(CreateVertexInputState(*this, state_data, GraphicsCreateInfo())),
      pre_raster_state(CreatePreRasterState(*this, state_data, GraphicsCreateInfo(), rpstate, stateless_data)),
      fragment_shader_state(
          CreateFragmentShaderState(*this, state_data, *pCreateInfo, GraphicsCreateInfo(), rpstate, stateless_data)),
      fragment_output_state(CreateFragmentOutputState(*this, state_data, *pCreateInfo, GraphicsCreateInfo(), rpstate)),
      stage_states(GetStageStates(state_data, *this, stateless_data)),
      create_info_shaders(GetCreateInfoShaders(*this)),
      linking_shaders(GetLinkingShaders(library_create_info, state_data)),
      active_shaders(create_info_shaders | linking_shaders),
      fragmentShader_writable_output_location_list(GetFSOutputLocations(stage_states)),
      active_slots(GetActiveSlots(stage_states)),
      max_active_slot(GetMaxActiveSlot(active_slots)),
      dynamic_state(GetGraphicsDynamicState(*this)),
      topology_at_rasterizer(GetTopologyAtRasterizer(*this)),
      descriptor_buffer_mode((create_flags & VK_PIPELINE_CREATE_2_DESCRIPTOR_BUFFER_BIT_EXT) != 0),
      uses_pipeline_robustness(UsesPipelineRobustness(GraphicsCreateInfo().pNext, *this)),
      uses_pipeline_vertex_robustness(UsesPipelineVertexRobustness(GraphicsCreateInfo().pNext, *this)),
      ignore_color_attachments(IgnoreColorAttachments(state_data, *this)) {
    if (library_create_info) {
        const auto &exe_layout_state = state_data.Get<vvl::PipelineLayout>(GraphicsCreateInfo().layout);
        const auto *exe_layout = exe_layout_state.get();
        const auto *pre_raster_layout =
            (pre_raster_state && pre_raster_state->pipeline_layout) ? pre_raster_state->pipeline_layout.get() : nullptr;
        const auto *fragment_shader_layout = (fragment_shader_state && fragment_shader_state->pipeline_layout)
                                                 ? fragment_shader_state->pipeline_layout.get()
                                                 : nullptr;
        std::array<decltype(exe_layout), 3> layouts;
        layouts[0] = exe_layout;
        layouts[1] = fragment_shader_layout;
        layouts[2] = pre_raster_layout;
        merged_graphics_layout = std::make_shared<vvl::PipelineLayout>(layouts);

        // TODO Could store the graphics_lib_type in the sub-state rather than searching for it again here.
        //      Or, could store a pointer back to the owning Pipeline.
        for (uint32_t i = 0; i < library_create_info->libraryCount; ++i) {
            const auto &state = state_data.Get<vvl::Pipeline>(library_create_info->pLibraries[i]);
            if (state) {
                graphics_lib_type |= state->graphics_lib_type;
            }
        }
    }
}

Pipeline::Pipeline(const Device &state_data, const VkComputePipelineCreateInfo *pCreateInfo,
                   std::shared_ptr<const vvl::PipelineCache> &&pipe_cache, std::shared_ptr<const vvl::PipelineLayout> &&layout,
                   spirv::StatelessData *stateless_data)
    : StateObject(static_cast<VkPipeline>(VK_NULL_HANDLE), kVulkanObjectTypePipeline),
      create_info(pCreateInfo),
      pipeline_cache(std::move(pipe_cache)),
      pipeline_type(VK_PIPELINE_BIND_POINT_COMPUTE),
      create_flags(GetPipelineCreateFlags(ComputeCreateInfo().pNext, ComputeCreateInfo().flags)),
      shader_stages_ci(&ComputeCreateInfo().stage, 1),
      uses_shader_module_id(UsesShaderModuleId(*this)),
      stage_states(GetStageStates(state_data, *this, stateless_data)),
      create_info_shaders(GetCreateInfoShaders(*this)),
      active_shaders(create_info_shaders),  // compute has no linking shaders
      active_slots(GetActiveSlots(stage_states)),
      max_active_slot(GetMaxActiveSlot(active_slots)),
      dynamic_state(0),  // compute has no dynamic state
      descriptor_buffer_mode((create_flags & VK_PIPELINE_CREATE_2_DESCRIPTOR_BUFFER_BIT_EXT) != 0),
      uses_pipeline_robustness(UsesPipelineRobustness(ComputeCreateInfo().pNext, *this)),
      uses_pipeline_vertex_robustness(false),
      ignore_color_attachments(IgnoreColorAttachments(state_data, *this)),
      merged_graphics_layout(layout) {
    assert(active_shaders == VK_SHADER_STAGE_COMPUTE_BIT);
}

Pipeline::Pipeline(const Device &state_data, const VkRayTracingPipelineCreateInfoKHR *pCreateInfo,
                   std::shared_ptr<const vvl::PipelineCache> &&pipe_cache, std::shared_ptr<const vvl::PipelineLayout> &&layout,
                   spirv::StatelessData *stateless_data)
    : StateObject(static_cast<VkPipeline>(VK_NULL_HANDLE), kVulkanObjectTypePipeline),
      create_info(pCreateInfo),
      pipeline_cache(std::move(pipe_cache)),
      pipeline_type(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
      create_flags(GetPipelineCreateFlags(RayTracingCreateInfo().pNext, RayTracingCreateInfo().flags)),
      shader_stages_ci(RayTracingCreateInfo().pStages, RayTracingCreateInfo().stageCount),
      ray_tracing_library_ci(RayTracingCreateInfo().pLibraryInfo),
      uses_shader_module_id(UsesShaderModuleId(*this)),
      stage_states(GetStageStates(state_data, *this, stateless_data)),
      create_info_shaders(GetCreateInfoShaders(*this)),
      active_shaders(create_info_shaders),  // RTX has no linking shaders
      active_slots(GetActiveSlots(stage_states)),
      max_active_slot(GetMaxActiveSlot(active_slots)),
      dynamic_state(GetRayTracingDynamicState(*this)),
      descriptor_buffer_mode((RayTracingCreateInfo().flags & VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT) != 0),
      uses_pipeline_robustness(UsesPipelineRobustness(RayTracingCreateInfo().pNext, *this)),
      uses_pipeline_vertex_robustness(false),
      ignore_color_attachments(IgnoreColorAttachments(state_data, *this)),
      merged_graphics_layout(std::move(layout)) {
    assert(0 == (active_shaders & ~(kShaderStageAllRayTracing)));
}

Pipeline::Pipeline(const Device &state_data, const VkRayTracingPipelineCreateInfoNV *pCreateInfo,
                   std::shared_ptr<const vvl::PipelineCache> &&pipe_cache, std::shared_ptr<const vvl::PipelineLayout> &&layout,
                   spirv::StatelessData *stateless_data)
    : StateObject(static_cast<VkPipeline>(VK_NULL_HANDLE), kVulkanObjectTypePipeline),
      create_info(pCreateInfo),
      pipeline_cache(std::move(pipe_cache)),
      pipeline_type(VK_PIPELINE_BIND_POINT_RAY_TRACING_NV),
      create_flags(GetPipelineCreateFlags(RayTracingCreateInfo().pNext, RayTracingCreateInfo().flags)),
      shader_stages_ci(RayTracingCreateInfo().pStages, RayTracingCreateInfo().stageCount),
      ray_tracing_library_ci(RayTracingCreateInfo().pLibraryInfo),
      uses_shader_module_id(UsesShaderModuleId(*this)),
      stage_states(GetStageStates(state_data, *this, stateless_data)),
      create_info_shaders(GetCreateInfoShaders(*this)),
      active_shaders(create_info_shaders),  // RTX has no linking shaders
      active_slots(GetActiveSlots(stage_states)),
      max_active_slot(GetMaxActiveSlot(active_slots)),
      dynamic_state(GetRayTracingDynamicState(*this)),
      descriptor_buffer_mode((RayTracingCreateInfo().flags & VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT) != 0),
      uses_pipeline_robustness(UsesPipelineRobustness(RayTracingCreateInfo().pNext, *this)),
      uses_pipeline_vertex_robustness(false),
      ignore_color_attachments(IgnoreColorAttachments(state_data, *this)),
      merged_graphics_layout(std::move(layout)) {
    assert(0 == (active_shaders & ~(kShaderStageAllRayTracing)));
}

}  // namespace vvl

void LastBound::UnbindAndResetPushDescriptorSet(std::shared_ptr<vvl::DescriptorSet> &&ds) {
    if (push_descriptor_set) {
        for (auto &ds_slot : ds_slots) {
            if (ds_slot.ds_state == push_descriptor_set) {
                cb_state.RemoveChild(ds_slot.ds_state);
                ds_slot.ds_state.reset();
            }
        }
    }
    cb_state.AddChild(ds);
    push_descriptor_set = std::move(ds);
}

void LastBound::Reset() {
    pipeline_state = nullptr;
    desc_set_pipeline_layout = VK_NULL_HANDLE;
    if (push_descriptor_set) {
        cb_state.RemoveChild(push_descriptor_set);
        push_descriptor_set->Destroy();
    }
    push_descriptor_set.reset();
    ds_slots.clear();
}

bool LastBound::IsDepthTestEnable() const {
    if (!pipeline_state || pipeline_state->IsDynamic(CB_DYNAMIC_STATE_DEPTH_TEST_ENABLE)) {
        if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_DEPTH_TEST_ENABLE)) {
            return cb_state.dynamic_state_value.depth_test_enable;
        }
    } else {
        if (pipeline_state->DepthStencilState()) {
            return pipeline_state->DepthStencilState()->depthTestEnable;
        }
    }
    return false;
}

bool LastBound::IsDepthBoundTestEnable() const {
    if (!pipeline_state || pipeline_state->IsDynamic(CB_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE)) {
        if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE)) {
            return cb_state.dynamic_state_value.depth_bounds_test_enable;
        }
    } else {
        if (pipeline_state->DepthStencilState()) {
            return pipeline_state->DepthStencilState()->depthBoundsTestEnable;
        }
    }
    return false;
}

bool LastBound::IsDepthWriteEnable() const {
    // "Depth writes are always disabled when depthTestEnable is VK_FALSE"
    if (!IsDepthTestEnable()) {
        return false;
    }
    if (!pipeline_state || pipeline_state->IsDynamic(CB_DYNAMIC_STATE_DEPTH_WRITE_ENABLE)) {
        if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_DEPTH_WRITE_ENABLE)) {
            return cb_state.dynamic_state_value.depth_write_enable;
        }
    } else {
        if (pipeline_state->DepthStencilState()) {
            return pipeline_state->DepthStencilState()->depthWriteEnable;
        }
    }
    return false;
}

bool LastBound::IsDepthBiasEnable() const {
    if (!pipeline_state || pipeline_state->IsDynamic(CB_DYNAMIC_STATE_DEPTH_BIAS_ENABLE)) {
        if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_DEPTH_BIAS_ENABLE)) {
            return cb_state.dynamic_state_value.depth_bias_enable;
        }
    } else {
        if (pipeline_state->RasterizationState()) {
            return pipeline_state->RasterizationState()->depthBiasEnable;
        }
    }
    return false;
}

bool LastBound::IsDepthClampEnable() const {
    if (!pipeline_state || pipeline_state->IsDynamic(CB_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT)) {
        if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT)) {
            return cb_state.dynamic_state_value.depth_clamp_enable;
        }
    } else {
        if (pipeline_state->RasterizationState()) {
            return pipeline_state->RasterizationState()->depthClampEnable;
        }
    }
    return false;
}

bool LastBound::IsStencilTestEnable() const {
    if (!pipeline_state || pipeline_state->IsDynamic(CB_DYNAMIC_STATE_STENCIL_TEST_ENABLE)) {
        if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_STENCIL_TEST_ENABLE)) {
            return cb_state.dynamic_state_value.stencil_test_enable;
        }
    } else {
        if (pipeline_state->DepthStencilState()) {
            return pipeline_state->DepthStencilState()->stencilTestEnable;
        }
    }
    return false;
}

VkStencilOpState LastBound::GetStencilOpStateFront() const {
    VkStencilOpState front = {};
    if (pipeline_state) {
        front = pipeline_state->DepthStencilState()->front;
    }
    if (!pipeline_state || pipeline_state->IsDynamic(CB_DYNAMIC_STATE_STENCIL_WRITE_MASK)) {
        front.writeMask = cb_state.dynamic_state_value.write_mask_front;
    }
    if (!pipeline_state || pipeline_state->IsDynamic(CB_DYNAMIC_STATE_STENCIL_OP)) {
        front.failOp = cb_state.dynamic_state_value.fail_op_front;
        front.passOp = cb_state.dynamic_state_value.pass_op_front;
        front.depthFailOp = cb_state.dynamic_state_value.depth_fail_op_front;
    }
    return front;
}

VkStencilOpState LastBound::GetStencilOpStateBack() const {
    VkStencilOpState back = {};
    if (pipeline_state) {
        back = pipeline_state->DepthStencilState()->back;
    }
    if (!pipeline_state || pipeline_state->IsDynamic(CB_DYNAMIC_STATE_STENCIL_WRITE_MASK)) {
        back.writeMask = cb_state.dynamic_state_value.write_mask_back;
    }
    if (!pipeline_state || pipeline_state->IsDynamic(CB_DYNAMIC_STATE_STENCIL_OP)) {
        back.failOp = cb_state.dynamic_state_value.fail_op_back;
        back.passOp = cb_state.dynamic_state_value.pass_op_back;
        back.depthFailOp = cb_state.dynamic_state_value.depth_fail_op_back;
    }
    return back;
}

VkSampleCountFlagBits LastBound::GetRasterizationSamples() const {
    // For given pipeline, return number of MSAA samples, or one if MSAA disabled
    VkSampleCountFlagBits rasterization_samples = VK_SAMPLE_COUNT_1_BIT;
    if (!pipeline_state || pipeline_state->IsDynamic(CB_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT)) {
        rasterization_samples = cb_state.dynamic_state_value.rasterization_samples;
    } else {
        if (auto ms_state = pipeline_state->MultisampleState()) {
            rasterization_samples = ms_state->rasterizationSamples;
        }
    }
    return rasterization_samples;
}

bool LastBound::IsRasterizationDisabled() const {
    if (!pipeline_state || pipeline_state->IsDynamic(CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
        if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
            return cb_state.dynamic_state_value.rasterizer_discard_enable;
        }
    } else {
        return (pipeline_state->RasterizationDisabled());
    }
    return false;
}

bool LastBound::IsLogicOpEnabled() const {
    if (!pipeline_state || pipeline_state->IsDynamic(CB_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT)) {
        if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT)) {
            return cb_state.dynamic_state_value.logic_op_enable;
        }
    } else {
        return pipeline_state->ColorBlendState() && pipeline_state->ColorBlendState()->logicOpEnable;
    }
    return false;
}

VkColorComponentFlags LastBound::GetColorWriteMask(uint32_t i) const {
    if (!pipeline_state || pipeline_state->IsDynamic(CB_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT)) {
        if (i < cb_state.dynamic_state_value.color_write_masks.size()) {
            return cb_state.dynamic_state_value.color_write_masks[i];
        }
    } else {
        if (pipeline_state->ColorBlendState() && i < pipeline_state->ColorBlendState()->attachmentCount) {
            return pipeline_state->ColorBlendState()->pAttachments[i].colorWriteMask;
        }
    }
    return (VkColorComponentFlags)0u;
}

bool LastBound::IsColorWriteEnabled(uint32_t i) const {
    if (!pipeline_state || pipeline_state->IsDynamic(CB_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT)) {
        if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT)) {
            return cb_state.dynamic_state_value.color_write_enabled[i];
        }
    } else {
        if (pipeline_state->ColorBlendState()) {
            auto color_write =
                vku::FindStructInPNextChain<VkPipelineColorWriteCreateInfoEXT>(pipeline_state->ColorBlendState()->pNext);
            if (color_write && i < color_write->attachmentCount) {
                return color_write->pColorWriteEnables[i];
            }
        }
    }
    return true;
}

VkPrimitiveTopology LastBound::GetPrimitiveTopology() const {
    if (!pipeline_state) {
        return GetShaderState(ShaderObjectStage::VERTEX)->GetTopology();
    } else if (pipeline_state->IsDynamic(CB_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY)) {
        return cb_state.dynamic_state_value.primitive_topology;
    } else {
        return pipeline_state->topology_at_rasterizer;
    }
}

VkCullModeFlags LastBound::GetCullMode() const {
    if (!pipeline_state || pipeline_state->IsDynamic(CB_DYNAMIC_STATE_CULL_MODE)) {
        if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_CULL_MODE)) {
            return cb_state.dynamic_state_value.cull_mode;
        }
    } else {
        if (auto raster_state = pipeline_state->RasterizationState()) {
            return raster_state->cullMode;
        }
    }
    return VK_CULL_MODE_NONE;
}

VkConservativeRasterizationModeEXT LastBound::GetConservativeRasterizationMode() const {
    if (!pipeline_state || pipeline_state->IsDynamic(CB_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT)) {
        if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT)) {
            return cb_state.dynamic_state_value.conservative_rasterization_mode;
        }
    } else {
        if (const auto rasterization_conservative_state_ci =
                vku::FindStructInPNextChain<VkPipelineRasterizationConservativeStateCreateInfoEXT>(
                    pipeline_state->RasterizationStatePNext())) {
            return rasterization_conservative_state_ci->conservativeRasterizationMode;
        }
    }
    return VK_CONSERVATIVE_RASTERIZATION_MODE_DISABLED_EXT;
}

bool LastBound::IsSampleLocationsEnable() const {
    if (!pipeline_state || pipeline_state->IsDynamic(CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT)) {
        if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT)) {
            return cb_state.dynamic_state_value.sample_locations_enable;
        }
    } else {
        if (auto ms_state = pipeline_state->MultisampleState()) {
            if (const auto *sample_location_state = vku::FindStructInPNextChain<VkPipelineSampleLocationsStateCreateInfoEXT>(ms_state->pNext)) {
                return sample_location_state->sampleLocationsEnable;
            }
        }
    }
    return false;
}

bool LastBound::IsExclusiveScissorEnabled() const {
    if (!pipeline_state || pipeline_state->IsDynamic(CB_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_ENABLE_NV)) {
        if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_ENABLE_NV)) {
            for (uint32_t i = 0; i < cb_state.dynamic_state_value.exclusive_scissor_enable_count; ++i) {
                if (cb_state.dynamic_state_value
                        .exclusive_scissor_enables[cb_state.dynamic_state_value.exclusive_scissor_enable_first + i]) {
                    return true;
                }
            }
        }
    } else {
        return true;  // no pipeline state, but if not dynamic, defaults to being enabled
    }
    return false;
}

bool LastBound::IsCoverageToColorEnabled() const {
    if (!pipeline_state || pipeline_state->IsDynamic(CB_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV)) {
        if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV)) {
            return cb_state.dynamic_state_value.coverage_to_color_enable;
        }
    } else {
        if (auto ms_state = pipeline_state->MultisampleState()) {
            if (const auto *coverage_to_color_state =
                    vku::FindStructInPNextChain<VkPipelineCoverageToColorStateCreateInfoNV>(ms_state->pNext)) {
                return coverage_to_color_state->coverageToColorEnable;
            }
        }
    }
    return false;
}

bool LastBound::IsCoverageModulationTableEnable() const {
    if (!pipeline_state || pipeline_state->IsDynamic(CB_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV)) {
        if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV)) {
            return cb_state.dynamic_state_value.coverage_modulation_table_enable;
        }
    } else {
        if (auto ms_state = pipeline_state->MultisampleState()) {
            if (const auto *coverage_modulation_state =
                    vku::FindStructInPNextChain<VkPipelineCoverageModulationStateCreateInfoNV>(ms_state->pNext)) {
                return coverage_modulation_state->coverageModulationTableEnable;
            }
        }
    }
    return false;
}

bool LastBound::IsStippledLineEnable() const {
    if (!pipeline_state || pipeline_state->IsDynamic(CB_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT)) {
        if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT)) {
            return cb_state.dynamic_state_value.stippled_line_enable;
        }
    } else {
        if (const auto line_state_ci = vku::FindStructInPNextChain<VkPipelineRasterizationLineStateCreateInfo>(
                pipeline_state->RasterizationStatePNext())) {
            return line_state_ci->stippledLineEnable;
        }
    }
    return false;
}

bool LastBound::IsDiscardRectangleEnable() const {
    if (!pipeline_state || pipeline_state->IsDynamic(CB_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT)) {
        if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT)) {
            return cb_state.dynamic_state_value.discard_rectangle_enable;
        }
    } else {
        // VK_EXT_discard_rectangles had a special v2 added right away to give it dynamic state
        // "If the VK_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT dynamic state is not enabled for the pipeline the presence of this
        // structure in the VkGraphicsPipelineCreateInfo chain, and a discardRectangleCount greater than zero, implicitly enables
        // discard rectangles in the pipeline"
        const void *pipeline_pnext = pipeline_state->GetCreateInfoPNext();
        if (const auto *discard_rectangle_state =
                vku::FindStructInPNextChain<VkPipelineDiscardRectangleStateCreateInfoEXT>(pipeline_pnext)) {
            return discard_rectangle_state->discardRectangleCount > 0;
        }
    }
    return false;
}

bool LastBound::IsShadingRateImageEnable() const {
    if (!pipeline_state || pipeline_state->IsDynamic(CB_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV)) {
        if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV)) {
            return cb_state.dynamic_state_value.shading_rate_image_enable;
        }
    } else {
        if (auto viewport_state = pipeline_state->ViewportState()) {
            if (const auto *shading_rate_image_state =
                    vku::FindStructInPNextChain<VkPipelineViewportShadingRateImageStateCreateInfoNV>(viewport_state->pNext)) {
                return shading_rate_image_state->shadingRateImageEnable;
            }
        }
    }
    return false;
}

bool LastBound::IsViewportWScalingEnable() const {
    if (!pipeline_state || pipeline_state->IsDynamic(CB_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV)) {
        if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV)) {
            return cb_state.dynamic_state_value.viewport_w_scaling_enable;
        }
    } else {
        if (auto viewport_state = pipeline_state->ViewportState()) {
            if (const auto *viewport_w_scaling_state =
                    vku::FindStructInPNextChain<VkPipelineViewportWScalingStateCreateInfoNV>(viewport_state->pNext)) {
                return viewport_w_scaling_state->viewportWScalingEnable;
            }
        }
    }
    return false;
}

bool LastBound::IsPrimitiveRestartEnable() const {
    if (!pipeline_state || pipeline_state->IsDynamic(CB_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE)) {
        if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE)) {
            return cb_state.dynamic_state_value.primitive_restart_enable;
        }
    } else {
        if (auto ia_state = pipeline_state->InputAssemblyState()) {
            return ia_state->primitiveRestartEnable == VK_TRUE;
        }
    }
    return false;
}

VkCoverageModulationModeNV LastBound::GetCoverageModulationMode() const {
    if (!pipeline_state || pipeline_state->IsDynamic(CB_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV)) {
        if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV)) {
            return cb_state.dynamic_state_value.coverage_modulation_mode;
        }
    } else {
        if (auto ms_state = pipeline_state->MultisampleState()) {
            if (const auto *coverage_modulation_state =
                    vku::FindStructInPNextChain<VkPipelineCoverageModulationStateCreateInfoNV>(ms_state->pNext)) {
                return coverage_modulation_state->coverageModulationMode;
            }
        }
    }
    return VK_COVERAGE_MODULATION_MODE_NONE_NV;
}

bool LastBound::ValidShaderObjectCombination(const VkPipelineBindPoint bind_point, const DeviceFeatures &device_features) const {
    if (bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) {
        if (!IsValidShaderOrNullBound(ShaderObjectStage::COMPUTE) ||
            !IsValidShaderOrNullBound(ShaderObjectStage::COMPUTE))
            return false;
    } else {
        if (!IsValidShaderOrNullBound(ShaderObjectStage::VERTEX)) return false;
        if (device_features.tessellationShader && !IsValidShaderOrNullBound(ShaderObjectStage::TESSELLATION_CONTROL)) return false;
        if (device_features.tessellationShader && !IsValidShaderOrNullBound(ShaderObjectStage::TESSELLATION_EVALUATION))
            return false;
        if (device_features.geometryShader && !IsValidShaderOrNullBound(ShaderObjectStage::GEOMETRY)) return false;
        if (!IsValidShaderOrNullBound(ShaderObjectStage::FRAGMENT)) return false;
        if (device_features.taskShader && !IsValidShaderOrNullBound(ShaderObjectStage::TASK)) return false;
        if (device_features.meshShader && !IsValidShaderOrNullBound(ShaderObjectStage::MESH)) return false;
        if (GetShader(ShaderObjectStage::VERTEX) == VK_NULL_HANDLE &&
            (!device_features.meshShader || GetShader(ShaderObjectStage::MESH) == VK_NULL_HANDLE))
            return false;
    }
    return true;
}

VkShaderEXT LastBound::GetShader(ShaderObjectStage stage) const {
    if (!IsValidShaderBound(stage) || GetShaderState(stage) == nullptr) return VK_NULL_HANDLE;
    return shader_object_states[static_cast<uint32_t>(stage)]->VkHandle();
}

vvl::ShaderObject *LastBound::GetShaderState(ShaderObjectStage stage) const {
    return shader_object_states[static_cast<uint32_t>(stage)];
}

const vvl::ShaderObject *LastBound::GetShaderStateIfValid(ShaderObjectStage stage) const {
    if (!shader_object_bound[static_cast<uint32_t>(stage)]) {
        return nullptr;
    }
    return shader_object_states[static_cast<uint32_t>(stage)];
}

const vvl::ShaderObject *LastBound::GetFirstShader(VkPipelineBindPoint bind_point) const {
    if (bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) {
        return GetShaderStateIfValid(ShaderObjectStage::COMPUTE);
    } else if (bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
        if (const vvl::ShaderObject *vs = GetShaderStateIfValid(ShaderObjectStage::VERTEX)) {
            return vs;
        }

        if (const vvl::ShaderObject *ms = GetShaderStateIfValid(ShaderObjectStage::MESH)) {
            return ms;
        }
    }

    return nullptr;
}

bool LastBound::HasShaderObjects() const {
    for (uint32_t i = 0; i < kShaderObjectStageCount; ++i) {
        if (GetShader(static_cast<ShaderObjectStage>(i)) != VK_NULL_HANDLE) {
            return true;
        }
    }
    return false;
}

bool LastBound::IsValidShaderBound(ShaderObjectStage stage) const { return GetShaderStateIfValid(stage) != nullptr; }

bool LastBound::IsValidShaderOrNullBound(ShaderObjectStage stage) const {
    return shader_object_bound[static_cast<uint32_t>(stage)];
}

std::vector<vvl::ShaderObject *> LastBound::GetAllBoundGraphicsShaders() {
    std::vector<vvl::ShaderObject *> shaders;

    if (IsValidShaderBound(ShaderObjectStage::VERTEX)) {
        shaders.emplace_back(shader_object_states[static_cast<uint32_t>(ShaderObjectStage::VERTEX)]);
    }
    if (IsValidShaderBound(ShaderObjectStage::TESSELLATION_CONTROL)) {
        shaders.emplace_back(shader_object_states[static_cast<uint32_t>(ShaderObjectStage::TESSELLATION_CONTROL)]);
    }
    if (IsValidShaderBound(ShaderObjectStage::TESSELLATION_EVALUATION)) {
        shaders.emplace_back(shader_object_states[static_cast<uint32_t>(ShaderObjectStage::TESSELLATION_EVALUATION)]);
    }
    if (IsValidShaderBound(ShaderObjectStage::GEOMETRY)) {
        shaders.emplace_back(shader_object_states[static_cast<uint32_t>(ShaderObjectStage::GEOMETRY)]);
    }
    if (IsValidShaderBound(ShaderObjectStage::FRAGMENT)) {
        shaders.emplace_back(shader_object_states[static_cast<uint32_t>(ShaderObjectStage::FRAGMENT)]);
    }
    if (IsValidShaderBound(ShaderObjectStage::TASK)) {
        shaders.emplace_back(shader_object_states[static_cast<uint32_t>(ShaderObjectStage::TASK)]);
    }
    if (IsValidShaderBound(ShaderObjectStage::MESH)) {
        shaders.emplace_back(shader_object_states[static_cast<uint32_t>(ShaderObjectStage::MESH)]);
    }

    return shaders;
}

bool LastBound::IsAnyGraphicsShaderBound() const {
    return IsValidShaderBound(ShaderObjectStage::VERTEX) ||
        IsValidShaderBound(ShaderObjectStage::TESSELLATION_CONTROL) ||
        IsValidShaderBound(ShaderObjectStage::TESSELLATION_EVALUATION) ||
        IsValidShaderBound(ShaderObjectStage::GEOMETRY) ||
        IsValidShaderBound(ShaderObjectStage::FRAGMENT) ||
        IsValidShaderBound(ShaderObjectStage::TASK) ||
        IsValidShaderBound(ShaderObjectStage::MESH);
}

VkShaderStageFlags LastBound::GetAllActiveBoundStages() const {
    if (pipeline_state) {
        return pipeline_state->active_shaders;
    }
    // else shader object
    VkShaderStageFlags stages = 0;
    if (IsValidShaderBound(ShaderObjectStage::VERTEX)) {
        stages |= VK_SHADER_STAGE_VERTEX_BIT;
    }
    if (IsValidShaderBound(ShaderObjectStage::TESSELLATION_CONTROL)) {
        stages |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    }
    if (IsValidShaderBound(ShaderObjectStage::TESSELLATION_EVALUATION)) {
        stages |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    }
    if (IsValidShaderBound(ShaderObjectStage::GEOMETRY)) {
        stages |= VK_SHADER_STAGE_GEOMETRY_BIT;
    }
    if (IsValidShaderBound(ShaderObjectStage::FRAGMENT)) {
        stages |= VK_SHADER_STAGE_FRAGMENT_BIT;
    }
    if (IsValidShaderBound(ShaderObjectStage::COMPUTE)) {
        stages |= VK_SHADER_STAGE_COMPUTE_BIT;
    }
    if (IsValidShaderBound(ShaderObjectStage::TASK)) {
        stages |= VK_SHADER_STAGE_TASK_BIT_EXT;
    }
    if (IsValidShaderBound(ShaderObjectStage::MESH)) {
        stages |= VK_SHADER_STAGE_MESH_BIT_EXT;
    }
    return stages;
}

bool LastBound::IsBoundSetCompatible(uint32_t set, const vvl::PipelineLayout &pipeline_layout) const {
    if ((set >= ds_slots.size()) || (set >= pipeline_layout.set_compat_ids.size())) {
        return false;
    }
    return (*(ds_slots[set].compat_id_for_set) == *(pipeline_layout.set_compat_ids[set]));
}

bool LastBound::IsBoundSetCompatible(uint32_t set, const vvl::ShaderObject &shader_object_state) const {
    if ((set >= ds_slots.size()) || (set >= shader_object_state.set_compat_ids.size())) {
        return false;
    }
    return (*(ds_slots[set].compat_id_for_set) == *(shader_object_state.set_compat_ids[set]));
};

std::string LastBound::DescribeNonCompatibleSet(uint32_t set, const vvl::PipelineLayout &pipeline_layout) const {
    std::ostringstream ss;
    if (set >= ds_slots.size()) {
        ss << "The set (" << set << ") is out of bounds for the number of sets bound (" << ds_slots.size() << ")\n";
    } else if (set >= pipeline_layout.set_compat_ids.size()) {
        ss << "The set (" << set << ") is out of bounds for the number of sets in the non-compatible VkPipelineLayout ("
           << pipeline_layout.set_compat_ids.size() << ")\n";
    } else {
        return ds_slots[set].compat_id_for_set->DescribeDifference(*(pipeline_layout.set_compat_ids[set]));
    }
    return ss.str();
}

std::string LastBound::DescribeNonCompatibleSet(uint32_t set, const vvl::ShaderObject &shader_object_state) const {
    std::ostringstream ss;
    if (set >= ds_slots.size()) {
        ss << "The set (" << set << ") is out of bounds for the number of sets bound (" << ds_slots.size() << ")\n";
    } else if (set >= shader_object_state.set_compat_ids.size()) {
        ss << "The set (" << set << ") is out of bounds for the number of sets in the non-compatible VkDescriptorSetLayout ("
           << shader_object_state.set_compat_ids.size() << ")\n";
    } else {
        return ds_slots[set].compat_id_for_set->DescribeDifference(*(shader_object_state.set_compat_ids[set]));
    }
    return ss.str();
}

bool IsPipelineLayoutSetCompatible(uint32_t set, const vvl::PipelineLayout *a, const vvl::PipelineLayout *b) {
    if (!a || !b) {
        return false;
    }
    if ((set >= a->set_compat_ids.size()) || (set >= b->set_compat_ids.size())) {
        return false;
    }
    return *(a->set_compat_ids[set]) == *(b->set_compat_ids[set]);
}

std::string DescribePipelineLayoutSetNonCompatible(uint32_t set, const vvl::PipelineLayout *a, const vvl::PipelineLayout *b) {
    std::ostringstream ss;
    if (!a || !b) {
        ss << "The set (" << set << ") has a null VkPipelineLayout object\n";
    } else if (set >= a->set_compat_ids.size()) {
        ss << "The set (" << set << ") is out of bounds for the number of sets in the non-compatible VkDescriptorSetLayout ("
           << a->set_compat_ids.size() << ")\n";
    } else if (set >= b->set_compat_ids.size()) {
        ss << "The set (" << set << ") is out of bounds for the number of sets in the non-compatible VkDescriptorSetLayout ("
           << b->set_compat_ids.size() << ")\n";
    } else {
        return a->set_compat_ids[set]->DescribeDifference(*(b->set_compat_ids[set]));
    }
    return ss.str();
}

const spirv::EntryPoint *LastBound::GetVertexEntryPoint() const {
    if (pipeline_state) {
        for (const ShaderStageState &shader_stage_state : pipeline_state->stage_states) {
            if (shader_stage_state.GetStage() != VK_SHADER_STAGE_VERTEX_BIT) {
                continue;
            }
            return shader_stage_state.entrypoint.get();
        }
        return nullptr;
    } else if (const auto *shader_object = GetShaderState(ShaderObjectStage::VERTEX)) {
        return shader_object->entrypoint.get();
    }
    return nullptr;
}

const spirv::EntryPoint *LastBound::GetFragmentEntryPoint() const {
    if (pipeline_state && pipeline_state->fragment_shader_state) {
        return pipeline_state->fragment_shader_state->fragment_entry_point.get();
    } else if (const auto *shader_object = GetShaderState(ShaderObjectStage::FRAGMENT)) {
        return shader_object->entrypoint.get();
    }
    return nullptr;
}

bool LastBound::WasInstrumented() const {
    if (pipeline_state) {
        return pipeline_state->instrumentation_data.was_instrumented;
    }
    for (uint32_t i = 0; i < kShaderObjectStageCount; ++i) {
        const auto stage = static_cast<ShaderObjectStage>(i);
        if (!IsValidShaderBound(stage)) continue;
        const vvl::ShaderObject *shader = GetShaderState(stage);
        if (shader && shader->instrumentation_data.was_instrumented) {
            return true;
        }
    }
    return false;
}
