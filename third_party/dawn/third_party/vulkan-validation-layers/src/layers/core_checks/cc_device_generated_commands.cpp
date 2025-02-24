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
 */

#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>
#include <spirv/unified1/spirv.hpp>
#include "core_validation.h"
#include "error_message/error_strings.h"
#include "generated/dispatch_functions.h"
#include "state_tracker/device_generated_commands_state.h"
#include "state_tracker/pipeline_layout_state.h"
#include "state_tracker/descriptor_sets.h"
#include "state_tracker/render_pass_state.h"
#include "state_tracker/shader_object_state.h"
#include "state_tracker/shader_module.h"
#include "cc_buffer_address.h"

static inline bool IsActionCommand(VkIndirectCommandsTokenTypeEXT type) {
    return IsValueIn(
        type, {VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_INDEXED_EXT, VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT,
               VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_INDEXED_COUNT_EXT, VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_COUNT_EXT,
               VK_INDIRECT_COMMANDS_TOKEN_TYPE_DISPATCH_EXT, VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_NV_EXT,
               VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_COUNT_NV_EXT, VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_EXT,
               VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_COUNT_EXT, VK_INDIRECT_COMMANDS_TOKEN_TYPE_TRACE_RAYS2_EXT});
}

// Make sure sub_range is contained inside the full_range
bool PushConstantRangesContained(VkPushConstantRange full_range, VkPushConstantRange sub_range) {
    const uint32_t start = full_range.offset;
    const uint32_t end = start + full_range.size;
    return (start >= sub_range.offset && end <= (sub_range.offset + sub_range.size));
}

bool CoreChecks::PreCallValidateCreateIndirectCommandsLayoutEXT(VkDevice device,
                                                                const VkIndirectCommandsLayoutCreateInfoEXT* pCreateInfo,
                                                                const VkAllocationCallbacks* pAllocator,
                                                                VkIndirectCommandsLayoutEXT* pIndirectCommandsLayout,
                                                                const ErrorObject& error_obj) const {
    bool skip = false;

    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);

    auto pipeline_layout_state = Get<vvl::PipelineLayout>(pCreateInfo->pipelineLayout);
    auto* dynamic_layout_create = vku::FindStructInPNextChain<VkPipelineLayoutCreateInfo>(pCreateInfo->pNext);

    const uint32_t kNotFound = vvl::kU32Max;
    uint32_t execution_set_token_index = kNotFound;
    uint32_t vertex_buffer_token_index = kNotFound;
    uint32_t index_buffer_token_index = kNotFound;
    uint32_t sequence_index_token_index = kNotFound;

    vvl::unordered_map<uint32_t, uint32_t> vertex_binding_unit_unique;
    vvl::unordered_map<uint32_t, VkPushConstantRange> token_ranges;

    for (uint32_t i = 0; i < pCreateInfo->tokenCount; i++) {
        const Location token_loc = create_info_loc.dot(Field::pTokens, i);
        const auto& token = pCreateInfo->pTokens[i];

        // Validate contents of VkIndirectCommandsTokenDataEXT
        const Location data_loc = token_loc.dot(Field::data);
        if (token.type == VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_CONSTANT_EXT) {
            const VkIndirectCommandsPushConstantTokenEXT* push_constant_token = token.data.pPushConstant;
            const Location pc_token_loc = data_loc.dot(Field::pPushConstant);
            const Location update_range_loc = pc_token_loc.dot(Field::updateRange);
            const VkPushConstantRange& token_range = push_constant_token->updateRange;

            if (!dynamic_layout_create && pipeline_layout_state) {
                const auto& layout_ranges = *pipeline_layout_state->push_constant_ranges_layout;
                for (const auto& layout_range : layout_ranges) {
                    if (!PushConstantRangesContained(token_range, layout_range)) {
                        skip |= LogError("VUID-VkIndirectCommandsPushConstantTokenEXT-updateRange-11132",
                                         pipeline_layout_state->Handle(), update_range_loc,
                                         "is %s but the push constant range in "
                                         "VkIndirectCommandsLayoutCreateInfoEXT::pipelineLayout is %s.",
                                         string_VkPushConstantRange(token_range).c_str(),
                                         string_VkPushConstantRange(layout_range).c_str());
                    }
                    break;
                }

            } else if (dynamic_layout_create) {
                // TODO - Because of custom PushConstantRangesId, can't share logic when using dynamicGeneratedPipelineLayout
                for (uint32_t pc_index = 0; pc_index < dynamic_layout_create->pushConstantRangeCount; pc_index++) {
                    const VkPushConstantRange& layout_range = dynamic_layout_create->pPushConstantRanges[pc_index];
                    if (!PushConstantRangesContained(token_range, layout_range)) {
                        skip |= LogError("VUID-VkIndirectCommandsPushConstantTokenEXT-updateRange-11132", device, update_range_loc,
                                         "is %s but the push constant range in "
                                         "VkPipelineLayoutCreateInfo::pPushConstantRanges[%" PRIu32 "] is %s.",
                                         string_VkPushConstantRange(token_range).c_str(), pc_index,
                                         string_VkPushConstantRange(layout_range).c_str());
                    }
                    break;
                }
            }
        } else if (token.type == VK_INDIRECT_COMMANDS_TOKEN_TYPE_VERTEX_BUFFER_EXT) {
            const VkIndirectCommandsVertexBufferTokenEXT* vertex_buffer_token = token.data.pVertexBuffer;
            const uint32_t vertex_binding_uint = vertex_buffer_token->vertexBindingUnit;
            if (vertex_binding_unit_unique.find(vertex_binding_uint) != vertex_binding_unit_unique.end()) {
                skip |= LogError("VUID-VkIndirectCommandsLayoutCreateInfoEXT-pTokens-11097", device,
                                 data_loc.dot(Field::pVertexBuffer).dot(Field::vertexBindingUnit),
                                 "(%" PRIu32 ") is the same as pTokens[%" PRIu32 "].data.pVertexBuffer.vertexBindingUnit",
                                 vertex_binding_uint, vertex_binding_unit_unique[vertex_binding_uint]);
            } else {
                vertex_binding_unit_unique[vertex_binding_uint] = i;
            }
        }

        // Check for duplicate tokens
        if (token.type == VK_INDIRECT_COMMANDS_TOKEN_TYPE_EXECUTION_SET_EXT) {
            // currently these 2 VUs overlap but can be helpful to catch in different times
            if (execution_set_token_index == kNotFound) {
                execution_set_token_index = i;
                if (i != 0) {
                    skip |= LogError(
                        "VUID-VkIndirectCommandsLayoutCreateInfoEXT-pTokens-11139", device, token_loc.dot(Field::type),
                        "is VK_INDIRECT_COMMANDS_TOKEN_TYPE_EXECUTION_SET_EXT but it can only be the first token at pTokens[0].");
                }
            } else {
                skip |= LogError("VUID-VkIndirectCommandsLayoutCreateInfoEXT-pTokens-11093", device, token_loc.dot(Field::type),
                                 "is VK_INDIRECT_COMMANDS_TOKEN_TYPE_EXECUTION_SET_EXT but also was pTokens[%" PRIu32
                                 "] (can only list token once).",
                                 execution_set_token_index);
            }
        } else if (token.type == VK_INDIRECT_COMMANDS_TOKEN_TYPE_INDEX_BUFFER_EXT) {
            if (index_buffer_token_index == kNotFound) {
                index_buffer_token_index = i;
            } else {
                skip |= LogError("VUID-VkIndirectCommandsLayoutCreateInfoEXT-pTokens-11094", device, token_loc.dot(Field::type),
                                 "is VK_INDIRECT_COMMANDS_TOKEN_TYPE_INDEX_BUFFER_EXT but also was pTokens[%" PRIu32
                                 "] (can only list token once).",
                                 index_buffer_token_index);
            }
        } else if (token.type == VK_INDIRECT_COMMANDS_TOKEN_TYPE_SEQUENCE_INDEX_EXT) {
            if (sequence_index_token_index == kNotFound) {
                sequence_index_token_index = i;
            } else {
                skip |= LogError("VUID-VkIndirectCommandsLayoutCreateInfoEXT-pTokens-11145", device, token_loc.dot(Field::type),
                                 "is VK_INDIRECT_COMMANDS_TOKEN_TYPE_SEQUENCE_INDEX_EXT but also was pTokens[%" PRIu32
                                 "] (can only list token once).",
                                 sequence_index_token_index);
            }
        } else if (token.type == VK_INDIRECT_COMMANDS_TOKEN_TYPE_VERTEX_BUFFER_EXT) {
            vertex_buffer_token_index = i;
        }

        if (token.type == VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_CONSTANT_EXT ||
            token.type == VK_INDIRECT_COMMANDS_TOKEN_TYPE_SEQUENCE_INDEX_EXT) {
            if (pCreateInfo->pipelineLayout == VK_NULL_HANDLE) {
                if (!enabled_features.dynamicGeneratedPipelineLayout) {
                    skip |= LogError("VUID-VkIndirectCommandsLayoutCreateInfoEXT-pTokens-11101", device, token_loc.dot(Field::type),
                                     "is %s, pipelineLayout is VK_NULL_HANDLE, but the "
                                     "dynamicGeneratedPipelineLayout feature was not enabled.",
                                     string_VkIndirectCommandsTokenTypeEXT(token.type));
                } else if (!dynamic_layout_create) {
                    skip |= LogError("VUID-VkIndirectCommandsLayoutCreateInfoEXT-pTokens-11102", device, token_loc.dot(Field::type),
                                     "is %s, pipelineLayout is VK_NULL_HANDLE, but no "
                                     "there is no VkPipelineLayoutCreateInfo structure attached to the pNext chain.",
                                     string_VkIndirectCommandsTokenTypeEXT(token.type));
                }
            }

            const VkIndirectCommandsPushConstantTokenEXT* push_constant_token = token.data.pPushConstant;
            const VkPushConstantRange& token_range = push_constant_token->updateRange;
            for (const auto& [past_index, past_range] : token_ranges) {
                if (RangesIntersect(past_range.offset, past_range.size, token_range.offset, token_range.size)) {
                    skip |= LogError("VUID-VkIndirectCommandsLayoutCreateInfoEXT-pTokens-11099", device,
                                     data_loc.dot(Field::pPushConstant).dot(Field::updateRange),
                                     "is %s which overlaps with pTokens[%" PRIu32 "].data.pPushConstant->updateRange (%s).",
                                     string_VkPushConstantRange(token_range).c_str(), past_index,
                                     string_VkPushConstantRange(past_range).c_str());
                    break;
                }
            }
            token_ranges[i] = token_range;
        }
    }

    const uint32_t final_token_index = pCreateInfo->tokenCount - 1;
    const VkIndirectCommandsTokenTypeEXT final_token_type = pCreateInfo->pTokens[final_token_index].type;
    if (!IsActionCommand(final_token_type)) {
        skip |= LogError("VUID-VkIndirectCommandsLayoutCreateInfoEXT-pTokens-11100", device,
                         create_info_loc.dot(Field::pTokens, final_token_index).dot(Field::type),
                         "is %s, but needs to be an action command (ex. Draw, Dispatch, Trace Rays, etc ).",
                         string_VkIndirectCommandsTokenTypeEXT(final_token_type));
    } else {
        if (!IsValueIn(final_token_type, {VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_INDEXED_EXT,
                                          VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_INDEXED_COUNT_EXT}) &&
            index_buffer_token_index != kNotFound) {
            skip |= LogError("VUID-VkIndirectCommandsLayoutCreateInfoEXT-pTokens-11095", device,
                             create_info_loc.dot(Field::pTokens, final_token_index).dot(Field::type),
                             "is %s (not an index draw token), but pTokens[%" PRIu32
                             "].type is VK_INDIRECT_COMMANDS_TOKEN_TYPE_INDEX_BUFFER_EXT.",
                             string_VkIndirectCommandsTokenTypeEXT(final_token_type), index_buffer_token_index);
        }
        if (!IsValueIn(final_token_type,
                       {VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_INDEXED_EXT, VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT,
                        VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_INDEXED_COUNT_EXT, VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_COUNT_EXT}) &&
            vertex_buffer_token_index != kNotFound) {
            skip |= LogError("VUID-VkIndirectCommandsLayoutCreateInfoEXT-pTokens-11096", device,
                             create_info_loc.dot(Field::pTokens, final_token_index).dot(Field::type),
                             "is %s (not a non-mesh draw token), but pTokens[%" PRIu32
                             "].type is VK_INDIRECT_COMMANDS_TOKEN_TYPE_VERTEX_BUFFER_EXT.",
                             string_VkIndirectCommandsTokenTypeEXT(final_token_type), vertex_buffer_token_index);
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateDestroyIndirectCommandsLayoutEXT(VkDevice device,
                                                                 VkIndirectCommandsLayoutEXT indirectCommandsLayout,
                                                                 const VkAllocationCallbacks* pAllocator,
                                                                 const ErrorObject& error_obj) const {
    bool skip = false;
    if (auto indirect_commands_layout = Get<vvl::IndirectCommandsLayout>(indirectCommandsLayout)) {
        skip |= ValidateObjectNotInUse(indirect_commands_layout.get(), error_obj.location,
                                       "VUID-vkDestroyIndirectCommandsLayoutEXT-indirectCommandsLayout-11114");
    }
    return skip;
}

bool CoreChecks::ValidateIndirectExecutionSetPipelineInfo(const VkIndirectExecutionSetPipelineInfoEXT& pipeline_info,
                                                          const Location& pipeline_info_loc) const {
    bool skip = false;

    const auto initial_pipeline = Get<vvl::Pipeline>(pipeline_info.initialPipeline);
    ASSERT_AND_RETURN_SKIP(initial_pipeline);

    const auto& props = phys_dev_ext_props.device_generated_commands_props;

    if ((initial_pipeline->create_flags & VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT) == 0) {
        skip |= LogError("VUID-VkIndirectExecutionSetPipelineInfoEXT-initialPipeline-11153", initial_pipeline->Handle(),
                         pipeline_info_loc.dot(Field::initialPipeline),
                         "is missing VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT, was created with flags %s. (Make sure you "
                         "set it with VkPipelineCreateFlags2CreateInfo)",
                         string_VkPipelineCreateFlags2(initial_pipeline->create_flags).c_str());
    }

    if (initial_pipeline->pipeline_type == VK_PIPELINE_BIND_POINT_COMPUTE &&
        ((props.supportedIndirectCommandsShaderStagesPipelineBinding & VK_SHADER_STAGE_COMPUTE_BIT) == 0)) {
        skip |= LogError(
            "VUID-VkIndirectExecutionSetPipelineInfoEXT-supportedIndirectCommandsShaderStagesPipelineBinding-11015",
            initial_pipeline->Handle(), pipeline_info_loc.dot(Field::initialPipeline),
            "is type VK_PIPELINE_BIND_POINT_COMPUTE, but supportedIndirectCommandsShaderStagesPipelineBinding (%s) doesn't "
            "contain compute stage support.",
            string_VkShaderStageFlags(props.supportedIndirectCommandsShaderStagesPipelineBinding).c_str());
    } else if (initial_pipeline->pipeline_type == VK_PIPELINE_BIND_POINT_GRAPHICS &&
               ((props.supportedIndirectCommandsShaderStagesPipelineBinding & VK_SHADER_STAGE_FRAGMENT_BIT) == 0)) {
        skip |= LogError(
            "VUID-VkIndirectExecutionSetPipelineInfoEXT-supportedIndirectCommandsShaderStagesPipelineBinding-11016",
            initial_pipeline->Handle(), pipeline_info_loc.dot(Field::initialPipeline),
            "is type VK_PIPELINE_BIND_POINT_GRAPHICS, but supportedIndirectCommandsShaderStagesPipelineBinding (%s) doesn't "
            "contain VK_SHADER_STAGE_FRAGMENT_BIT support.",
            string_VkShaderStageFlags(props.supportedIndirectCommandsShaderStagesPipelineBinding).c_str());
    } else if (initial_pipeline->pipeline_type == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR &&
               ((props.supportedIndirectCommandsShaderStagesPipelineBinding & kShaderStageAllRayTracing) == 0)) {
        skip |= LogError(
            "VUID-VkIndirectExecutionSetPipelineInfoEXT-supportedIndirectCommandsShaderStagesPipelineBinding-11017",
            initial_pipeline->Handle(), pipeline_info_loc.dot(Field::initialPipeline),
            "is type VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, but supportedIndirectCommandsShaderStagesPipelineBinding (%s) doesn't "
            "contain ray tracing stage support.",
            string_VkShaderStageFlags(props.supportedIndirectCommandsShaderStagesPipelineBinding).c_str());
    }

    auto pipeline_layout = initial_pipeline->PipelineLayoutState();
    ASSERT_AND_RETURN_SKIP(pipeline_layout);
    for (uint32_t i = 0; i < pipeline_layout->set_layouts.size(); i++) {
        if (pipeline_layout->set_layouts[i] == nullptr) continue;
        const auto& bindings = pipeline_layout->set_layouts[i]->GetBindings();
        for (uint32_t j = 0; j < bindings.size(); j++) {
            if (bindings[j].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
                bindings[j].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC) {
                const LogObjectList objlist(pipeline_layout->Handle(), pipeline_layout->set_layouts[i]->Handle());
                skip |= LogError("VUID-VkIndirectExecutionSetPipelineInfoEXT-initialPipeline-11019", objlist,
                                 pipeline_info_loc.dot(Field::initialPipeline),
                                 "was created with a VkPipelineLayout that contains a descriptor type of %s in pSetLayouts[%" PRIu32
                                 "], VkDescriptorSetLayoutCreateInfo::pBindings[%" PRIu32 "].",
                                 string_VkDescriptorType(bindings[j].descriptorType), i, j);
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateIndirectExecutionSetShaderInfo(const VkIndirectExecutionSetShaderInfoEXT& shader_info,
                                                        const Location& shader_info_loc) const {
    bool skip = false;

    const auto& props = phys_dev_ext_props.device_generated_commands_props;
    vvl::unordered_map<VkShaderStageFlagBits, uint32_t> unique_stages;
    for (uint32_t i = 0; i < shader_info.shaderCount; i++) {
        const VkShaderEXT shader_handle = shader_info.pInitialShaders[i];
        const auto shader_object = Get<vvl::ShaderObject>(shader_handle);
        ASSERT_AND_CONTINUE(shader_object);
        if ((shader_object->create_info.flags & VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT) == 0) {
            skip |= LogError("VUID-VkIndirectExecutionSetShaderInfoEXT-pInitialShaders-11154", shader_object->Handle(),
                             shader_info_loc.dot(Field::pInitialShaders, i),
                             "is missing VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT, was created with flags %s.",
                             string_VkShaderCreateFlagsEXT(shader_object->create_info.flags).c_str());
        }

        const VkShaderStageFlagBits stage = shader_object->create_info.stage;
        if ((stage & props.supportedIndirectCommandsShaderStagesShaderBinding) == 0) {
            skip |= LogError(
                "VUID-VkIndirectExecutionSetShaderInfoEXT-pInitialShaders-11020", shader_handle,
                shader_info_loc.dot(Field::pInitialShaders, i),
                "was created with stage %s, but supportedIndirectCommandsShaderStagesShaderBinding (%s) doesn't indicate support.",
                string_VkShaderStageFlagBits(stage),
                string_VkShaderStageFlags(props.supportedIndirectCommandsShaderStagesShaderBinding).c_str());
        }

        if (unique_stages.find(stage) != unique_stages.end()) {
            skip |=
                LogError("VUID-VkIndirectExecutionSetShaderInfoEXT-stage-11023", shader_handle,
                         shader_info_loc.dot(Field::pInitialShaders, i), "is the same stage as pInitialShaders[%" PRIu32 "] (%s)",
                         unique_stages[stage], string_VkShaderStageFlagBits(stage));
        } else {
            unique_stages[stage] = i;
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCreateIndirectExecutionSetEXT(VkDevice device,
                                                              const VkIndirectExecutionSetCreateInfoEXT* pCreateInfo,
                                                              const VkAllocationCallbacks* pAllocator,
                                                              VkIndirectExecutionSetEXT* pIndirectExecutionSet,
                                                              const ErrorObject& error_obj) const {
    bool skip = false;

    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    const Location info_loc = create_info_loc.dot(Field::info);

    if (pCreateInfo->type == VK_INDIRECT_EXECUTION_SET_INFO_TYPE_PIPELINES_EXT) {
        ASSERT_AND_RETURN_SKIP(pCreateInfo->info.pPipelineInfo);  // caught in stateless validation
        skip |= ValidateIndirectExecutionSetPipelineInfo(*pCreateInfo->info.pPipelineInfo, info_loc.dot(Field::pPipelineInfo));
    } else if (pCreateInfo->type == VK_INDIRECT_EXECUTION_SET_INFO_TYPE_SHADER_OBJECTS_EXT) {
        ASSERT_AND_RETURN_SKIP(pCreateInfo->info.pShaderInfo);  // caught in stateless validation
        skip |= ValidateIndirectExecutionSetShaderInfo(*pCreateInfo->info.pShaderInfo, info_loc.dot(Field::pShaderInfo));
    }

    return skip;
}

bool CoreChecks::PreCallValidateDestroyIndirectExecutionSetEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet,
                                                               const VkAllocationCallbacks* pAllocator,
                                                               const ErrorObject& error_obj) const {
    bool skip = false;
    if (auto indirect_execution_set = Get<vvl::IndirectExecutionSet>(indirectExecutionSet)) {
        skip |= ValidateObjectNotInUse(indirect_execution_set.get(), error_obj.location,
                                       "VUID-vkDestroyIndirectExecutionSetEXT-indirectExecutionSet-11025");
    }
    return skip;
}

bool CoreChecks::ValidateGeneratedCommandsInfo(const vvl::CommandBuffer& cb_state,
                                               const vvl::IndirectCommandsLayout& indirect_commands_layout,
                                               const VkGeneratedCommandsInfoEXT& generated_commands_info, bool preprocessed,
                                               const Location& info_loc) const {
    bool skip = false;

    if (indirect_commands_layout.has_vertex_buffer_token) {
        // If had vertex buffer token, it had to be graphic bind point (else would hit error earlier)
        const auto pipeline = cb_state.GetCurrentPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS);
        if (pipeline && !pipeline->IsDynamic(CB_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE)) {
            const LogObjectList objlist(cb_state.Handle(), pipeline->Handle());
            skip |= LogError("VUID-VkGeneratedCommandsInfoEXT-indirectCommandsLayout-11079", objlist,
                             info_loc.dot(Field::indirectCommandsLayout),
                             "contains a VK_INDIRECT_COMMANDS_TOKEN_TYPE_VERTEX_BUFFER_EXT token but the last bound graphics "
                             "pipeline did not have VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE set.");
        }
    }

    if (indirect_commands_layout.has_multi_draw_count_token) {
        // Use 64-bit to catch overflow
        const uint64_t limit = 1 << 24;
        const uint64_t count = (uint64_t)generated_commands_info.maxDrawCount * (uint64_t)generated_commands_info.maxSequenceCount;
        if (count >= limit) {
            skip |= LogError(
                "VUID-VkGeneratedCommandsInfoEXT-maxDrawCount-11078", cb_state.Handle(), info_loc.dot(Field::maxDrawCount),
                "(%" PRIu32 ") time maxSequenceCount (%" PRIu32 ") is %" PRIu64 " which is over the limit of 2^24 (16777216)",
                generated_commands_info.maxDrawCount, generated_commands_info.maxSequenceCount, count);
        }
    }

    bool valid_dispatch = true;
    auto* pipeline_info = vku::FindStructInPNextChain<VkGeneratedCommandsPipelineInfoEXT>(generated_commands_info.pNext);
    auto* shader_info = vku::FindStructInPNextChain<VkGeneratedCommandsShaderInfoEXT>(generated_commands_info.pNext);
    if (generated_commands_info.indirectExecutionSet == VK_NULL_HANDLE) {
        if (!pipeline_info && !shader_info) {
            skip |= LogError(
                "VUID-VkGeneratedCommandsInfoEXT-indirectExecutionSet-11080", cb_state.Handle(),
                info_loc.dot(Field::indirectExecutionSet),
                "is VK_NULL_HANDLE but the pNext chain does not contain an instance of VkGeneratedCommandsPipelineInfoEXT or "
                "VkGeneratedCommandsShaderInfoEXT.");
            valid_dispatch = false;
        } else if (indirect_commands_layout.has_execution_set_token) {
            skip |= LogError("VUID-VkGeneratedCommandsInfoEXT-indirectCommandsLayout-11083", indirect_commands_layout.Handle(),
                             info_loc.dot(Field::indirectExecutionSet),
                             "is VK_NULL_HANDLE but indirectCommandsLayout was created with a "
                             "VK_INDIRECT_COMMANDS_TOKEN_TYPE_EXECUTION_SET_EXT token.");
            valid_dispatch = false;
        }
    } else {
        if (!indirect_commands_layout.has_execution_set_token) {
            skip |= LogError("VUID-VkGeneratedCommandsInfoEXT-indirectCommandsLayout-10241", indirect_commands_layout.Handle(),
                             info_loc.dot(Field::indirectExecutionSet),
                             "is not VK_NULL_HANDLE but indirectCommandsLayout was not created with a "
                             "VK_INDIRECT_COMMANDS_TOKEN_TYPE_EXECUTION_SET_EXT token.");
            valid_dispatch = false;
        } else {
            auto indirect_execution_set = Get<vvl::IndirectExecutionSet>(generated_commands_info.indirectExecutionSet);
            ASSERT_AND_RETURN_SKIP(indirect_execution_set);
            if (indirect_execution_set->shader_stage_flags != indirect_commands_layout.execution_set_token_shader_stage_flags) {
                skip |=
                    LogError("VUID-VkGeneratedCommandsInfoEXT-indirectCommandsLayout-11002", indirect_commands_layout.Handle(),
                             info_loc.dot(Field::indirectExecutionSet),
                             "was created with shader stage %s but indirectCommandsLayout was created with shader stage %s.",
                             string_VkShaderStageFlags(indirect_execution_set->shader_stage_flags).c_str(),
                             string_VkShaderStageFlags(indirect_commands_layout.execution_set_token_shader_stage_flags).c_str());
                valid_dispatch = false;
            }
        }
    }

    // Only dispatch if we know this is valid
    if (valid_dispatch) {
        VkGeneratedCommandsMemoryRequirementsInfoEXT req_info = vku::InitStructHelper();
        req_info.maxSequenceCount = generated_commands_info.maxSequenceCount;
        req_info.indirectCommandsLayout = generated_commands_info.indirectCommandsLayout;
        req_info.indirectExecutionSet = generated_commands_info.indirectExecutionSet;
        if (generated_commands_info.indirectExecutionSet == VK_NULL_HANDLE) {
            req_info.pNext = pipeline_info ? (void*)pipeline_info : (void*)shader_info;
        }
        VkMemoryRequirements2 mem_reqs = vku::InitStructHelper();
        DispatchGetGeneratedCommandsMemoryRequirementsEXT(device, &req_info, &mem_reqs);

        if (generated_commands_info.preprocessAddress == 0 && mem_reqs.memoryRequirements.size != 0) {
            skip |= LogError("VUID-VkGeneratedCommandsInfoEXT-preprocessAddress-11063", cb_state.Handle(),
                             info_loc.dot(Field::preprocessAddress),
                             "is NULL but vkGetGeneratedCommandsMemoryRequirementsEXT returned a non-zero size of %" PRIu64 ".",
                             mem_reqs.memoryRequirements.size);
        }
        if (generated_commands_info.preprocessSize < mem_reqs.memoryRequirements.size) {
            skip |= LogError(
                "VUID-VkGeneratedCommandsInfoEXT-preprocessSize-11071", cb_state.Handle(), info_loc.dot(Field::preprocessSize),
                "(%" PRIu64 ") is less then the size returned from vkGetGeneratedCommandsMemoryRequirementsEXT (%" PRIu64 ").",
                generated_commands_info.preprocessSize, mem_reqs.memoryRequirements.size);
        }
    }

    if (generated_commands_info.maxSequenceCount > phys_dev_ext_props.device_generated_commands_props.maxIndirectSequenceCount) {
        skip |= LogError(
            "VUID-VkGeneratedCommandsInfoEXT-maxSequenceCount-11067", cb_state.Handle(), info_loc.dot(Field::maxSequenceCount),
            "(%" PRIu32 ") is larger than maxIndirectSequenceCount (%" PRIu32 ").", generated_commands_info.maxSequenceCount,
            phys_dev_ext_props.device_generated_commands_props.maxIndirectSequenceCount);
    }

    const auto preprocess_buffer_states = GetBuffersByAddress(generated_commands_info.preprocessAddress);
    if (!preprocess_buffer_states.empty()) {
        BufferAddressValidation<2> buffer_address_validator = {{{
            {"VUID-VkGeneratedCommandsInfoEXT-preprocessAddress-11069",
             [](vvl::Buffer* const buffer_state, std::string* out_error_msg) {
                 if ((buffer_state->usage & VK_BUFFER_USAGE_2_PREPROCESS_BUFFER_BIT_EXT) == 0) {
                     if (out_error_msg) {
                         *out_error_msg += "buffer has usage " + string_VkBufferUsageFlags2(buffer_state->usage);
                     }
                     return false;
                 }
                 return true;
             },
             []() { return "The following buffers are missing VK_BUFFER_USAGE_2_PREPROCESS_BUFFER_BIT_EXT"; }},
            {"VUID-VkGeneratedCommandsInfoEXT-preprocessAddress-11070",
             [this](vvl::Buffer* const buffer_state, std::string* out_error_msg) {
                 return BufferAddressValidation<1>::ValidateMemoryBoundToBuffer(*this, buffer_state, out_error_msg);
             },
             []() { return BufferAddressValidation<1>::ValidateMemoryBoundToBufferErrorMsgHeader(); }},
        }}};

        skip |= buffer_address_validator.LogErrorsIfNoValidBuffer(
            *this, preprocess_buffer_states, info_loc.dot(Field::preprocessAddress), LogObjectList(cb_state.Handle()),
            generated_commands_info.preprocessAddress);
    }

    const auto sequence_buffer_states = GetBuffersByAddress(generated_commands_info.sequenceCountAddress);
    if (!sequence_buffer_states.empty()) {
        BufferAddressValidation<2> buffer_address_validator = {{{
            {"VUID-VkGeneratedCommandsInfoEXT-sequenceCountAddress-11072",
             [](vvl::Buffer* const buffer_state, std::string* out_error_msg) {
                 if ((buffer_state->usage & VK_BUFFER_USAGE_2_INDIRECT_BUFFER_BIT) == 0) {
                     if (out_error_msg) {
                         *out_error_msg += "buffer has usage " + string_VkBufferUsageFlags2(buffer_state->usage);
                     }
                     return false;
                 }
                 return true;
             },
             []() { return "The following buffers are missing VK_BUFFER_USAGE_2_INDIRECT_BUFFER_BIT"; }},
            {"VUID-VkGeneratedCommandsInfoEXT-sequenceCountAddress-11075",
             [this](vvl::Buffer* const buffer_state, std::string* out_error_msg) {
                 return BufferAddressValidation<1>::ValidateMemoryBoundToBuffer(*this, buffer_state, out_error_msg);
             },
             []() { return BufferAddressValidation<1>::ValidateMemoryBoundToBufferErrorMsgHeader(); }},
        }}};

        skip |= buffer_address_validator.LogErrorsIfNoValidBuffer(
            *this, sequence_buffer_states, info_loc.dot(Field::sequenceCountAddress), LogObjectList(cb_state.Handle()),
            generated_commands_info.sequenceCountAddress);
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdExecuteGeneratedCommandsEXT(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
                                                               const VkGeneratedCommandsInfoEXT* pGeneratedCommandsInfo,
                                                               const ErrorObject& error_obj) const {
    bool skip = false;

    const auto& cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (!cb_state.unprotected) {
        skip |= LogError("VUID-vkCmdExecuteGeneratedCommandsEXT-commandBuffer-11045", commandBuffer,
                         error_obj.location.dot(Field::commandBuffer), "is protected.");
    }

    const auto& props = phys_dev_ext_props.device_generated_commands_props;
    if (cb_state.transform_feedback_active) {
        if (!props.deviceGeneratedCommandsTransformFeedback) {
            skip |= LogError("VUID-vkCmdExecuteGeneratedCommandsEXT-deviceGeneratedCommandsTransformFeedback-11057", commandBuffer,
                             error_obj.location.dot(Field::commandBuffer),
                             "has Transform Feedback active, but deviceGeneratedCommandsTransformFeedback is not supported.");
        }
        if (pGeneratedCommandsInfo->indirectExecutionSet != VK_NULL_HANDLE) {
            skip |=
                LogError("VUID-vkCmdExecuteGeneratedCommandsEXT-indirectExecutionSet-11058", commandBuffer,
                         error_obj.location.dot(Field::commandBuffer),
                         "has Transform Feedback active, but pGeneratedCommandsInfo->indirectExecutionSet is not VK_NULL_HANDLE.");
        }
    }

    if (cb_state.beginInfo.flags & VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT) {
        LogError("VUID-vkCmdExecuteGeneratedCommandsEXT-commandBuffer-11143", commandBuffer,
                 error_obj.location.dot(Field::commandBuffer), "was created with VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT.");
    }

    const Location info_loc = error_obj.location.dot(Field::pGeneratedCommandsInfo);
    const auto indirect_commands_layout = Get<vvl::IndirectCommandsLayout>(pGeneratedCommandsInfo->indirectCommandsLayout);
    ASSERT_AND_RETURN_SKIP(indirect_commands_layout);

    const bool preprocess_usage_flag =
        (indirect_commands_layout->create_info.flags & VK_INDIRECT_COMMANDS_LAYOUT_USAGE_EXPLICIT_PREPROCESS_BIT_EXT) != 0;
    if (isPreprocessed && !preprocess_usage_flag) {
        const LogObjectList objlist(commandBuffer, indirect_commands_layout->Handle());
        skip |= LogError(
            "VUID-vkCmdExecuteGeneratedCommandsEXT-isPreprocessed-11047", objlist, info_loc.dot(Field::indirectCommandsLayout),
            "was not created with VK_INDIRECT_COMMANDS_LAYOUT_USAGE_EXPLICIT_PREPROCESS_BIT_EXT but isPreprocessed is VK_TRUE.");
    } else if (!isPreprocessed && preprocess_usage_flag) {
        const LogObjectList objlist(commandBuffer, indirect_commands_layout->Handle());
        skip |= LogError(
            "VUID-vkCmdExecuteGeneratedCommandsEXT-indirectCommandsLayout-11141", objlist,
            info_loc.dot(Field::indirectCommandsLayout),
            "was created with VK_INDIRECT_COMMANDS_LAYOUT_USAGE_EXPLICIT_PREPROCESS_BIT_EXT but isPreprocessed is VK_FALSE.");
    }

    if (const vvl::RenderPass* rp_state = cb_state.active_render_pass.get()) {
        uint32_t view_mask = 0;
        if (rp_state->UsesDynamicRendering()) {
            view_mask = rp_state->dynamic_rendering_begin_rendering_info.viewMask;
        } else {
            const auto* render_pass_info = rp_state->create_info.ptr();
            const auto subpass_desc = render_pass_info->pSubpasses[cb_state.GetActiveSubpass()];
            view_mask = subpass_desc.viewMask;
        }
        if (view_mask != 0) {
            skip |= LogError("VUID-vkCmdExecuteGeneratedCommandsEXT-None-11062", commandBuffer, error_obj.location,
                             "The active render pass contains a non-zero view mask (%" PRIu32 ").", view_mask);
        }
    }

    if (auto indirect_execution_set = Get<vvl::IndirectExecutionSet>(pGeneratedCommandsInfo->indirectExecutionSet)) {
        const LogObjectList objlist(commandBuffer, indirect_commands_layout->Handle(), indirect_execution_set->Handle());
        skip |= ValidateGeneratedCommandsInitialShaderState(cb_state, *indirect_commands_layout, *indirect_execution_set,
                                                            pGeneratedCommandsInfo->shaderStages, objlist,
                                                            error_obj.location.dot(Field::commandBuffer));
    }

    const auto lv_bind_point = ConvertToLvlBindPoint(indirect_commands_layout->bind_point);
    const auto& last_bound_state = cb_state.lastBound[lv_bind_point];
    VkShaderStageFlags bound_stages = last_bound_state.GetAllActiveBoundStages();
    if ((bound_stages | props.supportedIndirectCommandsShaderStages) != props.supportedIndirectCommandsShaderStages) {
        skip |= LogError("VUID-vkCmdExecuteGeneratedCommandsEXT-supportedIndirectCommandsShaderStages-11060",
                         cb_state.GetObjectList(indirect_commands_layout->bind_point), error_obj.location.dot(Field::commandBuffer),
                         "is using stages (%s) but supportedIndirectCommandsShaderStages only supports %s.",
                         string_VkShaderStageFlags(bound_stages).c_str(),
                         string_VkShaderStageFlags(props.supportedIndirectCommandsShaderStages).c_str());
    }

    skip |= ValidateGeneratedCommandsInfo(cb_state, *indirect_commands_layout, *pGeneratedCommandsInfo, isPreprocessed, info_loc);

    return skip;
}

bool CoreChecks::ValidateGeneratedCommandsInitialShaderState(const vvl::CommandBuffer& cb_state,
                                                             const vvl::IndirectCommandsLayout& indirect_commands_layout,
                                                             const vvl::IndirectExecutionSet& indirect_execution_set,
                                                             VkShaderStageFlags shader_stage_flags, const LogObjectList& objlist,
                                                             const Location cb_loc) const {
    bool skip = false;
    if (!indirect_commands_layout.has_execution_set_token) return skip;
    const char* vuid = (cb_loc.function == Func::vkCmdPreprocessGeneratedCommandsEXT)
                           ? "VUID-vkCmdPreprocessGeneratedCommandsEXT-indirectCommandsLayout-11084"
                           : "VUID-vkCmdExecuteGeneratedCommandsEXT-indirectCommandsLayout-11053";

    const VkPipelineBindPoint bind_point = ConvertToPipelineBindPoint(shader_stage_flags);
    const auto lv_bind_point = ConvertToLvlBindPoint(bind_point);
    const LastBound& last_bound = cb_state.lastBound[lv_bind_point];

    if (indirect_execution_set.is_pipeline) {
        const vvl::Pipeline* pipeline = last_bound.pipeline_state;
        if (!pipeline) {
            skip |= LogError(vuid, objlist, cb_loc, "has not had a pipeline bound for %s.", string_VkPipelineBindPoint(bind_point));
        } else if (pipeline->Handle() != indirect_execution_set.initial_pipeline->Handle()) {
            skip |= LogError(vuid, objlist, cb_loc,
                             "bound %s at %s does not match the VkIndirectExecutionSetPipelineInfoEXT::initialPipeline (%s).",
                             FormatHandle(*pipeline).c_str(), string_VkPipelineBindPoint(bind_point),
                             FormatHandle(*indirect_execution_set.initial_pipeline).c_str());
        }
    } else if (indirect_execution_set.is_shader_objects) {
        // Shader Objects only has compute or graphics
        if (bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) {
            if (!last_bound.IsValidShaderBound(ShaderObjectStage::COMPUTE)) {
                skip |= LogError(vuid, objlist, cb_loc, "has not had a compute VkShaderEXT bound yet.");
            }
        } else if (bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
            if (!last_bound.IsAnyGraphicsShaderBound()) {
                skip |= LogError(vuid, objlist, cb_loc, "has not had a graphics VkShaderEXT bound yet.");
            }
        }
    }

    return skip;
}

// To prevent having "cb_state" and "state_cb_state", we isolate the logic here
bool CoreChecks::ValidatePreprocessGeneratedCommandsStateCommandBuffer(const vvl::CommandBuffer& command_buffer,
                                                                       const vvl::CommandBuffer& state_command_buffer,
                                                                       const vvl::IndirectCommandsLayout& indirect_commands_layout,
                                                                       const VkGeneratedCommandsInfoEXT& generated_commands_info,
                                                                       const Location loc) const {
    bool skip = false;

    if (state_command_buffer.state == CbState::InvalidComplete || state_command_buffer.state == CbState::InvalidIncomplete) {
        skip |= ReportInvalidCommandBuffer(state_command_buffer, loc.dot(Field::stateCommandBuffer),
                                           "VUID-vkCmdPreprocessGeneratedCommandsEXT-stateCommandBuffer-11138");
    } else if (CbState::Recording != state_command_buffer.state) {
        const LogObjectList objlist(command_buffer.Handle(), state_command_buffer.Handle());
        skip |= LogError("VUID-vkCmdPreprocessGeneratedCommandsEXT-stateCommandBuffer-11138", objlist,
                         loc.dot(Field::stateCommandBuffer), "is not in a recording state.");
    }

    if (auto indirect_execution_set = Get<vvl::IndirectExecutionSet>(generated_commands_info.indirectExecutionSet)) {
        const LogObjectList objlist(command_buffer.Handle(), state_command_buffer.Handle(), indirect_commands_layout.Handle(),
                                    indirect_execution_set->Handle());
        skip |= ValidateGeneratedCommandsInitialShaderState(state_command_buffer, indirect_commands_layout, *indirect_execution_set,
                                                            generated_commands_info.shaderStages, objlist,
                                                            loc.dot(Field::stateCommandBuffer));
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdPreprocessGeneratedCommandsEXT(VkCommandBuffer commandBuffer,
                                                                  const VkGeneratedCommandsInfoEXT* pGeneratedCommandsInfo,
                                                                  VkCommandBuffer stateCommandBuffer,
                                                                  const ErrorObject& error_obj) const {
    bool skip = false;

    const auto& cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (!cb_state.unprotected) {
        skip |= LogError("VUID-vkCmdPreprocessGeneratedCommandsEXT-commandBuffer-11081", commandBuffer,
                         error_obj.location.dot(Field::commandBuffer), "is protected.");
    }

    const Location info_loc = error_obj.location.dot(Field::pGeneratedCommandsInfo);
    const auto indirect_commands_layout = Get<vvl::IndirectCommandsLayout>(pGeneratedCommandsInfo->indirectCommandsLayout);
    ASSERT_AND_RETURN_SKIP(indirect_commands_layout);

    if ((indirect_commands_layout->create_info.flags & VK_INDIRECT_COMMANDS_LAYOUT_USAGE_EXPLICIT_PREPROCESS_BIT_EXT) == 0) {
        const LogObjectList objlist(commandBuffer, indirect_commands_layout->Handle());
        skip |= LogError("VUID-vkCmdPreprocessGeneratedCommandsEXT-pGeneratedCommandsInfo-11082", objlist,
                         info_loc.dot(Field::indirectCommandsLayout),
                         "was not created with VK_INDIRECT_COMMANDS_LAYOUT_USAGE_EXPLICIT_PREPROCESS_BIT_EXT.");
    }

    // Unfortunate variable name, but it is in line with the spec
    const auto state_cb_state = GetRead<vvl::CommandBuffer>(stateCommandBuffer);
    ASSERT_AND_RETURN_SKIP(state_cb_state);
    skip |= ValidatePreprocessGeneratedCommandsStateCommandBuffer(cb_state, *state_cb_state, *indirect_commands_layout,
                                                                  *pGeneratedCommandsInfo, error_obj.location);

    skip |= ValidateGeneratedCommandsInfo(cb_state, *indirect_commands_layout, *pGeneratedCommandsInfo, false, info_loc);

    return skip;
}

bool CoreChecks::PreCallValidateGetGeneratedCommandsMemoryRequirementsEXT(VkDevice device,
                                                                          const VkGeneratedCommandsMemoryRequirementsInfoEXT* pInfo,
                                                                          VkMemoryRequirements2* pMemoryRequirements,
                                                                          const ErrorObject& error_obj) const {
    bool skip = false;

    const Location info_loc = error_obj.location.dot(Field::pInfo);
    if (pInfo->maxSequenceCount > phys_dev_ext_props.device_generated_commands_props.maxIndirectSequenceCount) {
        skip |=
            LogError("VUID-VkGeneratedCommandsMemoryRequirementsInfoEXT-maxSequencesCount-11009", device,
                     info_loc.dot(Field::maxSequenceCount), "(%" PRIu32 ") is larger than maxIndirectSequenceCount (%" PRIu32 ").",
                     pInfo->maxSequenceCount, phys_dev_ext_props.device_generated_commands_props.maxIndirectSequenceCount);
    }

    const auto indirect_commands_layout = Get<vvl::IndirectCommandsLayout>(pInfo->indirectCommandsLayout);
    ASSERT_AND_RETURN_SKIP(indirect_commands_layout);

    if (indirect_commands_layout->has_multi_draw_count_token) {
        // Use 64-bit to catch overflow
        const uint64_t limit = 1 << 24;
        const uint64_t count = (uint64_t)pInfo->maxDrawCount * (uint64_t)pInfo->maxSequenceCount;
        if (count >= limit) {
            skip |= LogError(
                "VUID-VkGeneratedCommandsMemoryRequirementsInfoEXT-maxDrawCount-11146", device, info_loc.dot(Field::maxDrawCount),
                "(%" PRIu32 ") time maxSequenceCount (%" PRIu32 ") is %" PRIu64 " which is over the limit of 2^24 (16777216)",
                pInfo->maxDrawCount, pInfo->maxSequenceCount, count);
        }
    }

    if (pInfo->indirectExecutionSet == VK_NULL_HANDLE) {
        if (indirect_commands_layout->has_execution_set_token) {
            skip |= LogError("VUID-VkGeneratedCommandsMemoryRequirementsInfoEXT-indirectCommandsLayout-11010",
                             indirect_commands_layout->Handle(), info_loc.dot(Field::indirectExecutionSet),
                             "is VK_NULL_HANDLE but indirectCommandsLayout was created with a "
                             "VK_INDIRECT_COMMANDS_TOKEN_TYPE_EXECUTION_SET_EXT token.");
        }

        auto* pipeline_info = vku::FindStructInPNextChain<VkGeneratedCommandsPipelineInfoEXT>(pInfo->pNext);
        auto* shader_info = vku::FindStructInPNextChain<VkGeneratedCommandsShaderInfoEXT>(pInfo->pNext);
        if (!pipeline_info && !shader_info) {
            skip |= LogError(
                "VUID-VkGeneratedCommandsMemoryRequirementsInfoEXT-indirectExecutionSet-11012", indirect_commands_layout->Handle(),
                info_loc.dot(Field::indirectExecutionSet),
                "is VK_NULL_HANDLE but the pNext chain does not contain an instance of VkGeneratedCommandsPipelineInfoEXT or "
                "VkGeneratedCommandsShaderInfoEXT.");
        }
    } else {
        if (!indirect_commands_layout->has_execution_set_token) {
            skip |= LogError("VUID-VkGeneratedCommandsMemoryRequirementsInfoEXT-indirectCommandsLayout-11011",
                             indirect_commands_layout->Handle(), info_loc.dot(Field::indirectExecutionSet),
                             "is not VK_NULL_HANDLE but indirectCommandsLayout was not created with a "
                             "VK_INDIRECT_COMMANDS_TOKEN_TYPE_EXECUTION_SET_EXT token.");
        } else {
            auto indirect_execution_set = Get<vvl::IndirectExecutionSet>(pInfo->indirectExecutionSet);
            ASSERT_AND_RETURN_SKIP(indirect_execution_set);
            if (indirect_execution_set->shader_stage_flags != indirect_commands_layout->execution_set_token_shader_stage_flags) {
                skip |=
                    LogError("VUID-VkGeneratedCommandsMemoryRequirementsInfoEXT-indirectCommandsLayout-11151",
                             indirect_commands_layout->Handle(), info_loc.dot(Field::indirectExecutionSet),
                             "was created with shader stage %s but indirectCommandsLayout was created with shader stage %s.",
                             string_VkShaderStageFlags(indirect_execution_set->shader_stage_flags).c_str(),
                             string_VkShaderStageFlags(indirect_commands_layout->execution_set_token_shader_stage_flags).c_str());
            }
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateUpdateIndirectExecutionSetPipelineEXT(
    VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet, uint32_t executionSetWriteCount,
    const VkWriteIndirectExecutionSetPipelineEXT* pExecutionSetWrites, const ErrorObject& error_obj) const {
    bool skip = false;
    auto indirect_execution_set = Get<vvl::IndirectExecutionSet>(indirectExecutionSet);
    ASSERT_AND_RETURN_SKIP(indirect_execution_set);
    if (!indirect_execution_set->is_pipeline) {
        skip |= LogError("VUID-vkUpdateIndirectExecutionSetPipelineEXT-indirectExecutionSet-11035",
                         indirect_execution_set->Handle(), error_obj.location.dot(Field::indirectExecutionSet),
                         "was not created with type VK_INDIRECT_EXECUTION_SET_INFO_TYPE_PIPELINES_EXT.");
        return skip;  // something is wrong and the rest will probably be garbage
    }

    if (executionSetWriteCount > indirect_execution_set->max_pipeline_count) {
        skip |= LogError("VUID-vkUpdateIndirectExecutionSetPipelineEXT-executionSetWriteCount-11037",
                         indirect_execution_set->Handle(), error_obj.location.dot(Field::executionSetWriteCount),
                         "(%" PRIu32 ") is larger than the maxPipelineCount used to create indirectExecutionSet (%" PRIu32 ").",
                         executionSetWriteCount, indirect_execution_set->max_pipeline_count);
    }

    vvl::unordered_map<uint32_t, uint32_t> unique_indexes;
    const auto& props = phys_dev_ext_props.device_generated_commands_props;
    for (uint32_t i = 0; i < executionSetWriteCount; i++) {
        const VkWriteIndirectExecutionSetPipelineEXT& set_pipeline = pExecutionSetWrites[i];
        const Location set_write_loc = error_obj.location.dot(Field::pExecutionSetWrites, i);

        if (unique_indexes.find(set_pipeline.index) != unique_indexes.end()) {
            skip |=
                LogError("VUID-vkUpdateIndirectExecutionSetPipelineEXT-pExecutionSetWrites-11042", device,
                         set_write_loc.dot(Field::index), "(%" PRIu32 ") has same value as pExecutionSetWrites[%" PRIu32 "].index",
                         set_pipeline.index, unique_indexes[set_pipeline.index]);
        } else {
            unique_indexes[set_pipeline.index] = i;
        }

        if (set_pipeline.index >= indirect_execution_set->max_pipeline_count) {
            skip |=
                LogError("VUID-VkWriteIndirectExecutionSetPipelineEXT-index-11026", indirect_execution_set->Handle(),
                         set_write_loc.dot(Field::index),
                         "(%" PRIu32 ") is not less than the maxPipelineCount used to create indirectExecutionSet (%" PRIu32 ").",
                         set_pipeline.index, indirect_execution_set->max_pipeline_count);
        }

        auto update_pipeline = Get<vvl::Pipeline>(set_pipeline.pipeline);
        ASSERT_AND_CONTINUE(update_pipeline);
        if ((update_pipeline->create_flags & VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT) == 0) {
            // TODO - This seems to not be possible to hit without first hitting 11153
            skip |= LogError("VUID-VkWriteIndirectExecutionSetPipelineEXT-pipeline-11027", update_pipeline->Handle(),
                             set_write_loc.dot(Field::pipeline),
                             "is missing VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT, was created with flags %s. (Make sure you "
                             "set it with VkPipelineCreateFlags2CreateInfo)",
                             string_VkPipelineCreateFlags2(update_pipeline->create_flags).c_str());
        }

        if ((update_pipeline->active_shaders | props.supportedIndirectCommandsShaderStagesPipelineBinding) !=
            props.supportedIndirectCommandsShaderStagesPipelineBinding) {
            skip |= LogError("VUID-VkWriteIndirectExecutionSetPipelineEXT-pipeline-11030", update_pipeline->Handle(),
                             set_write_loc.dot(Field::pipeline),
                             "is using stages (%s) but supportedIndirectCommandsShaderStagesPipelineBinding only supports %s.",
                             string_VkShaderStageFlags(update_pipeline->active_shaders).c_str(),
                             string_VkShaderStageFlags(props.supportedIndirectCommandsShaderStagesPipelineBinding).c_str());
        }

        if (const auto initial_pipeline = indirect_execution_set->initial_pipeline) {
            // Want to throw this error first, in case one is missing a fragment shader, might have extra errors follow below
            if (initial_pipeline->active_shaders != update_pipeline->active_shaders) {
                LogObjectList objlist(initial_pipeline->Handle(), update_pipeline->Handle());
                skip |= LogError("VUID-vkUpdateIndirectExecutionSetPipelineEXT-initialPipeline-11152", objlist,
                                 set_write_loc.dot(Field::pipeline),
                                 "was created with shader stages different from the ones specified in "
                                 "initialPipeline.\ninitialPipeline: %s\nupdate "
                                 "pipeline: %s\n",
                                 string_VkShaderStageFlags(initial_pipeline->active_shaders).c_str(),
                                 string_VkShaderStageFlags(update_pipeline->active_shaders).c_str());
            }

            if (initial_pipeline->dynamic_state != update_pipeline->dynamic_state) {
                LogObjectList objlist(initial_pipeline->Handle(), update_pipeline->Handle());
                skip |=
                    LogError("VUID-vkUpdateIndirectExecutionSetPipelineEXT-None-11040", objlist, set_write_loc.dot(Field::pipeline),
                             "was created with dynamic states different from the ones specified in "
                             "initialPipeline.\ninitialPipeline: %s\nupdate "
                             "pipeline: %s\n",
                             DynamicStatesToString(initial_pipeline->dynamic_state).c_str(),
                             DynamicStatesToString(update_pipeline->dynamic_state).c_str());
            }

            if (initial_pipeline->fragmentShader_writable_output_location_list !=
                update_pipeline->fragmentShader_writable_output_location_list) {
                LogObjectList objlist(initial_pipeline->Handle(), update_pipeline->Handle());
                skip |= LogError("VUID-vkUpdateIndirectExecutionSetPipelineEXT-initialPipeline-11147", objlist,
                                 set_write_loc.dot(Field::pipeline),
                                 "was created with a fragment shader interface different from the one specified in "
                                 "initialPipeline. The fragment shaders statically write to different locations.");
            }

            // Default to false incase there is not fragment shader
            bool initial_pipeline_frag_depth = false;
            bool initial_pipeline_sample_mask = false;
            bool initial_pipeline_stencil_export = false;
            if (initial_pipeline->fragment_shader_state && initial_pipeline->fragment_shader_state->fragment_entry_point) {
                auto shader_state = initial_pipeline->fragment_shader_state;
                initial_pipeline_frag_depth = shader_state->fragment_entry_point->HasBuiltIn(spv::BuiltInFragDepth);
                initial_pipeline_sample_mask = shader_state->fragment_entry_point->HasBuiltIn(spv::BuiltInSampleMask);
                if (shader_state->fragment_shader && shader_state->fragment_shader->spirv) {
                    initial_pipeline_stencil_export =
                        shader_state->fragment_shader->spirv->HasCapability(spv::CapabilityStencilExportEXT);
                }
            }

            // Indirect Execution Set
            bool ies_pipeline_frag_depth = false;
            bool ies_pipeline_sample_mask = false;
            bool ies_pipeline_stencil_export = false;
            if (update_pipeline->fragment_shader_state && update_pipeline->fragment_shader_state->fragment_entry_point) {
                auto shader_state = update_pipeline->fragment_shader_state;
                ies_pipeline_frag_depth = shader_state->fragment_entry_point->HasBuiltIn(spv::BuiltInFragDepth);
                ies_pipeline_sample_mask = shader_state->fragment_entry_point->HasBuiltIn(spv::BuiltInSampleMask);
                if (shader_state->fragment_shader && shader_state->fragment_shader->spirv) {
                    ies_pipeline_stencil_export =
                        shader_state->fragment_shader->spirv->HasCapability(spv::CapabilityStencilExportEXT);
                }
            }

            if (initial_pipeline_frag_depth != ies_pipeline_frag_depth) {
                LogObjectList objlist(initial_pipeline->Handle(), update_pipeline->Handle());
                skip |= LogError("VUID-vkUpdateIndirectExecutionSetPipelineEXT-initialPipeline-11098", objlist,
                                 set_write_loc.dot(Field::pipeline),
                                 "was %screated with FragDepth, but the initialPipeline was %screated with FragDepth.",
                                 ies_pipeline_frag_depth ? "" : "not ", initial_pipeline_frag_depth ? "" : "not ");
            }
            if (initial_pipeline_sample_mask != ies_pipeline_sample_mask) {
                LogObjectList objlist(initial_pipeline->Handle(), update_pipeline->Handle());
                skip |= LogError("VUID-vkUpdateIndirectExecutionSetPipelineEXT-initialPipeline-11086", objlist,
                                 set_write_loc.dot(Field::pipeline),
                                 "was %screated with SampleMask, but the initialPipeline was %screated with SampleMask.",
                                 ies_pipeline_sample_mask ? "" : "not ", initial_pipeline_sample_mask ? "" : "not ");
            }
            if (initial_pipeline_stencil_export != ies_pipeline_stencil_export) {
                LogObjectList objlist(initial_pipeline->Handle(), update_pipeline->Handle());
                skip |=
                    LogError("VUID-vkUpdateIndirectExecutionSetPipelineEXT-initialPipeline-11085", objlist,
                             set_write_loc.dot(Field::pipeline),
                             "was %screated with StencilExportEXT, but the initialPipeline was %screated with StencilExportEXT.",
                             ies_pipeline_stencil_export ? "" : "not ", initial_pipeline_stencil_export ? "" : "not ");
            }

            const auto initial_pipeline_layout = initial_pipeline->PipelineLayoutState();
            const auto update_pipeline_layout = update_pipeline->PipelineLayoutState();
            if (initial_pipeline_layout && update_pipeline_layout) {
                const uint32_t set_count = (uint32_t)std::min(initial_pipeline_layout->set_compat_ids.size(),
                                                              update_pipeline_layout->set_compat_ids.size());
                for (uint32_t set = 0; set < set_count; set++) {
                    if (!IsPipelineLayoutSetCompatible(set, initial_pipeline_layout.get(), update_pipeline_layout.get())) {
                        LogObjectList objlist(initial_pipeline->Handle(), initial_pipeline_layout->Handle(),
                                              update_pipeline->Handle(), update_pipeline_layout->Handle());
                        skip |= LogError(
                            "VUID-vkUpdateIndirectExecutionSetPipelineEXT-None-11039", objlist, set_write_loc.dot(Field::pipeline),
                            "%s was created with a layout %s which is not compatible with the initialPipeline layout %s for set "
                            "%" PRIu32 ".\n%s",
                            FormatHandle(update_pipeline->VkHandle()).c_str(),
                            FormatHandle(update_pipeline_layout->VkHandle()).c_str(),
                            FormatHandle(initial_pipeline_layout->VkHandle()).c_str(), set,
                            DescribePipelineLayoutSetNonCompatible(set, initial_pipeline_layout.get(), update_pipeline_layout.get())
                                .c_str());
                        break;
                    }
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateUpdateIndirectExecutionSetShaderEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet,
                                                                    uint32_t executionSetWriteCount,
                                                                    const VkWriteIndirectExecutionSetShaderEXT* pExecutionSetWrites,
                                                                    const ErrorObject& error_obj) const {
    bool skip = false;
    auto indirect_execution_set = Get<vvl::IndirectExecutionSet>(indirectExecutionSet);
    ASSERT_AND_RETURN_SKIP(indirect_execution_set);
    if (!indirect_execution_set->is_shader_objects) {
        skip |= LogError("VUID-vkUpdateIndirectExecutionSetShaderEXT-indirectExecutionSet-11041", indirect_execution_set->Handle(),
                         error_obj.location.dot(Field::indirectExecutionSet),
                         "was not created with type VK_INDIRECT_EXECUTION_SET_INFO_TYPE_SHADER_OBJECTS_EXT.");
        return skip;  // something is wrong and the rest will probably be garbage
    }

    vvl::unordered_map<uint32_t, uint32_t> unique_indexes;
    for (uint32_t i = 0; i < executionSetWriteCount; i++) {
        const VkWriteIndirectExecutionSetShaderEXT& set_shader = pExecutionSetWrites[i];
        const Location set_write_loc = error_obj.location.dot(Field::pExecutionSetWrites, i);

        if (unique_indexes.find(set_shader.index) != unique_indexes.end()) {
            skip |=
                LogError("VUID-vkUpdateIndirectExecutionSetShaderEXT-pExecutionSetWrites-11043", device,
                         set_write_loc.dot(Field::index), "(%" PRIu32 ") has same value as pExecutionSetWrites[%" PRIu32 "].index",
                         set_shader.index, unique_indexes[set_shader.index]);
        } else {
            unique_indexes[set_shader.index] = i;
        }

        if (set_shader.index >= indirect_execution_set->max_shader_count) {
            skip |= LogError("VUID-VkWriteIndirectExecutionSetShaderEXT-index-11031", device, set_write_loc.dot(Field::index),
                             "(%" PRIu32
                             ") is not less then the sum of VkIndirectExecutionSetShaderInfoEXT::maxShaderCount (%" PRIu32 ").",
                             set_shader.index, indirect_execution_set->max_shader_count);
        }

        auto update_shader_object = Get<vvl::ShaderObject>(set_shader.shader);
        ASSERT_AND_CONTINUE(update_shader_object);
        if ((update_shader_object->create_info.flags & VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT) == 0) {
            // TODO - This seems to not be possible to hit without first hitting 11154
            skip |= LogError("VUID-VkWriteIndirectExecutionSetShaderEXT-shader-11032", update_shader_object->Handle(),
                             set_write_loc.dot(Field::shader),
                             "is missing VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT, was created with flags %s.",
                             string_VkShaderCreateFlagsEXT(update_shader_object->create_info.flags).c_str());
        }

        if ((update_shader_object->create_info.stage & indirect_execution_set->shader_stage_flags) == 0) {
            skip |= LogError(
                "VUID-VkWriteIndirectExecutionSetShaderEXT-pInitialShaders-11033", update_shader_object->Handle(),
                set_write_loc.dot(Field::shader),
                "was created with %s but none of the VkIndirectExecutionSetShaderInfoEXT::pShaderStages contained that stage (%s).",
                string_VkShaderStageFlagBits(update_shader_object->create_info.stage),
                string_VkShaderStageFlags(indirect_execution_set->shader_stage_flags).c_str());
        }

        const auto initial_fragment_shader_object = indirect_execution_set->initial_fragment_shader_object;
        if (initial_fragment_shader_object && update_shader_object->create_info.stage == VK_SHADER_STAGE_FRAGMENT_BIT) {
            if (update_shader_object->entrypoint && initial_fragment_shader_object->entrypoint) {
                vvl::unordered_set<uint32_t> update_shader_output_locations;
                vvl::unordered_set<uint32_t> initial_shader_output_locations;

                // From GetFSOutputLocations()
                for (const auto* variable : update_shader_object->entrypoint->user_defined_interface_variables) {
                    if ((variable->storage_class != spv::StorageClassOutput) || variable->interface_slots.empty()) {
                        continue;  // not an output interface
                    }
                    update_shader_output_locations.insert(variable->interface_slots[0].Location());
                }
                for (const auto* variable : initial_fragment_shader_object->entrypoint->user_defined_interface_variables) {
                    if ((variable->storage_class != spv::StorageClassOutput) || variable->interface_slots.empty()) {
                        continue;  // not an output interface
                    }
                    initial_shader_output_locations.insert(variable->interface_slots[0].Location());
                }

                if (update_shader_output_locations != initial_shader_output_locations) {
                    LogObjectList objlist(initial_fragment_shader_object->Handle(), update_shader_object->Handle());
                    skip |=
                        LogError("VUID-vkUpdateIndirectExecutionSetShaderEXT-None-11148", objlist, set_write_loc.dot(Field::shader),
                                 "was created with a fragment shader interface different from the initial Fragment VkShaderEXT. "
                                 "The fragment shaders statically write to different locations.");
                }

                const bool initial_shader_frag_depth =
                    initial_fragment_shader_object->entrypoint->HasBuiltIn(spv::BuiltInFragDepth);
                const bool initial_shader_sample_mask =
                    initial_fragment_shader_object->entrypoint->HasBuiltIn(spv::BuiltInSampleMask);
                const bool initial_shader_stencil_export =
                    initial_fragment_shader_object->spirv &&
                    initial_fragment_shader_object->spirv->HasCapability(spv::CapabilityStencilExportEXT);

                const bool ies_shader_frag_depth = update_shader_object->entrypoint->HasBuiltIn(spv::BuiltInFragDepth);
                const bool ies_shader_sample_mask = update_shader_object->entrypoint->HasBuiltIn(spv::BuiltInSampleMask);
                const bool ies_shader_stencil_export =
                    update_shader_object->spirv && update_shader_object->spirv->HasCapability(spv::CapabilityStencilExportEXT);

                if (initial_shader_frag_depth != ies_shader_frag_depth) {
                    LogObjectList objlist(initial_fragment_shader_object->Handle(), update_shader_object->Handle());
                    skip |= LogError(
                        "VUID-vkUpdateIndirectExecutionSetShaderEXT-FragDepth-11054", objlist, set_write_loc.dot(Field::shader),
                        "was %screated with FragDepth, but the initial Fragment VkShaderEXT was %screated with FragDepth.",
                        ies_shader_frag_depth ? "" : "not ", initial_shader_frag_depth ? "" : "not ");
                }
                if (initial_shader_sample_mask != ies_shader_sample_mask) {
                    LogObjectList objlist(initial_fragment_shader_object->Handle(), update_shader_object->Handle());
                    skip |= LogError(
                        "VUID-vkUpdateIndirectExecutionSetShaderEXT-SampleMask-11050", objlist, set_write_loc.dot(Field::shader),
                        "was %screated with SampleMask, but the initial Fragment VkShaderEXT was %screated with SampleMask.",
                        ies_shader_sample_mask ? "" : "not ", initial_shader_sample_mask ? "" : "not ");
                }
                if (initial_shader_stencil_export != ies_shader_stencil_export) {
                    LogObjectList objlist(initial_fragment_shader_object->Handle(), update_shader_object->Handle());
                    skip |= LogError("VUID-vkUpdateIndirectExecutionSetShaderEXT-StencilExportEXT-11003", objlist,
                                     set_write_loc.dot(Field::shader),
                                     "was %screated with StencilExportEXT, but the initial Fragment VkShaderEXT was %screated with "
                                     "StencilExportEXT.",
                                     ies_shader_stencil_export ? "" : "not ", initial_shader_stencil_export ? "" : "not ");
                }
            }
        }
    }

    return skip;
}
