/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
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

#include "stateless/stateless_validation.h"
#include "generated/enum_flag_bits.h"
#include "utils/vk_layer_utils.h"

namespace stateless {
static inline bool IsMeshCommand(VkIndirectCommandsTokenTypeEXT type) {
    return IsValueIn(
        type,
        {VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_EXT, VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_COUNT_EXT,
         VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_NV_EXT, VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_COUNT_NV_EXT});
}

static inline bool IsComputeCommand(VkIndirectCommandsTokenTypeEXT type) {
    return IsValueIn(type, {VK_INDIRECT_COMMANDS_TOKEN_TYPE_DISPATCH_EXT, VK_INDIRECT_COMMANDS_TOKEN_TYPE_EXECUTION_SET_EXT,
                            VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_CONSTANT_EXT, VK_INDIRECT_COMMANDS_TOKEN_TYPE_SEQUENCE_INDEX_EXT});
}

static inline bool IsRayTracingCommand(VkIndirectCommandsTokenTypeEXT type) {
    return IsValueIn(type, {VK_INDIRECT_COMMANDS_TOKEN_TYPE_TRACE_RAYS2_EXT, VK_INDIRECT_COMMANDS_TOKEN_TYPE_EXECUTION_SET_EXT,
                            VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_CONSTANT_EXT, VK_INDIRECT_COMMANDS_TOKEN_TYPE_SEQUENCE_INDEX_EXT});
}

bool Device::ValidateIndirectExecutionSetPipelineInfo(const Context& context,
                                                      const VkIndirectExecutionSetPipelineInfoEXT& pipeline_info,
                                                      const Location& pipeline_info_loc) const {
    bool skip = false;

    const auto& props = phys_dev_ext_props.device_generated_commands_props;
    if (pipeline_info.maxPipelineCount == 0) {
        skip |= LogError("VUID-VkIndirectExecutionSetPipelineInfoEXT-maxPipelineCount-11018", device,
                         pipeline_info_loc.dot(Field::maxPipelineCount), "is zero.");
    } else if (pipeline_info.maxPipelineCount > props.maxIndirectPipelineCount) {
        skip |= LogError("VUID-VkIndirectExecutionSetPipelineInfoEXT-maxPipelineCount-11018", device,
                         pipeline_info_loc.dot(Field::maxPipelineCount),
                         "(%" PRIu32 ") is larger than maxIndirectPipelineCount (%" PRIu32 ").", pipeline_info.maxPipelineCount,
                         props.maxIndirectPipelineCount);
    }

    skip |= ValidateIndirectExecutionSetPipelineInfoEXT(context, pipeline_info, pipeline_info_loc);

    return skip;
}

bool Device::ValidateIndirectExecutionSetShaderInfo(const Context& context, const VkIndirectExecutionSetShaderInfoEXT& shader_info,
                                                    const Location& shader_info_loc) const {
    bool skip = false;

    const auto& props = phys_dev_ext_props.device_generated_commands_props;
    if (shader_info.maxShaderCount == 0) {
        skip |= LogError("VUID-VkIndirectExecutionSetShaderInfoEXT-maxShaderCount-11021", device,
                         shader_info_loc.dot(Field::maxShaderCount), "is zero.");
    } else if (shader_info.maxShaderCount > props.maxIndirectShaderObjectCount) {
        skip |= LogError("VUID-VkIndirectExecutionSetShaderInfoEXT-maxShaderCount-11022", device,
                         shader_info_loc.dot(Field::maxShaderCount),
                         "(%" PRIu32 ") is larger than maxIndirectShaderObjectCount (%" PRIu32 ").", shader_info.maxShaderCount,
                         props.maxIndirectShaderObjectCount);
    } else if (shader_info.maxShaderCount < shader_info.shaderCount) {
        skip |= LogError("VUID-VkIndirectExecutionSetShaderInfoEXT-maxShaderCount-11036", device,
                         shader_info_loc.dot(Field::maxShaderCount), "(%" PRIu32 ") is less than shaderCount (%" PRIu32 ").",
                         shader_info.maxShaderCount, shader_info.shaderCount);
    }

    // implicit checks - done manually as code gen is hard to get correct
    skip |= context.ValidateStructType(shader_info_loc, &shader_info, VK_STRUCTURE_TYPE_INDIRECT_EXECUTION_SET_SHADER_INFO_EXT,
                                       false, kVUIDUndefined, "VUID-VkIndirectExecutionSetShaderInfoEXT-sType-sType");
    skip |= context.ValidateStructTypeArray(shader_info_loc.dot(Field::shaderCount), shader_info_loc.dot(Field::pSetLayoutInfos),
                                            shader_info.shaderCount, shader_info.pSetLayoutInfos,
                                            VK_STRUCTURE_TYPE_INDIRECT_EXECUTION_SET_SHADER_LAYOUT_INFO_EXT, true, false,
                                            "VUID-VkIndirectExecutionSetShaderLayoutInfoEXT-sType-sType",
                                            "VUID-VkIndirectExecutionSetShaderInfoEXT-pSetLayoutInfos-parameter",
                                            "VUID-VkIndirectExecutionSetShaderInfoEXT-shaderCount-arraylength");

    // Validate shaderCount once above
    skip |= context.ValidateArray(shader_info_loc.dot(Field::shaderCount), shader_info_loc.dot(Field::pInitialShaders),
                                  shader_info.shaderCount, &shader_info.pInitialShaders, false, true, kVUIDUndefined,
                                  "VUID-VkIndirectExecutionSetShaderInfoEXT-pInitialShaders-parameter");
    skip |=
        context.ValidateArray(shader_info_loc.dot(Field::pushConstantRangeCount), shader_info_loc.dot(Field::pPushConstantRanges),
                              shader_info.pushConstantRangeCount, &shader_info.pPushConstantRanges, false, true, kVUIDUndefined,
                              "VUID-VkIndirectExecutionSetShaderInfoEXT-pPushConstantRanges-parameter");

    if (shader_info.pPushConstantRanges != nullptr) {
        for (uint32_t i = 0; i < shader_info.pushConstantRangeCount; ++i) {
            const Location pc_range_loc = shader_info_loc.dot(Field::pPushConstantRanges, i);
            skip |= context.ValidateFlags(pc_range_loc.dot(Field::stageFlags), vvl::FlagBitmask::VkShaderStageFlagBits,
                                          AllVkShaderStageFlagBits, shader_info.pPushConstantRanges[i].stageFlags, kRequiredFlags,
                                          "VUID-VkPushConstantRange-stageFlags-parameter",
                                          "VUID-VkPushConstantRange-stageFlags-requiredbitmask");
        }
    }

    return skip;
}

bool Device::manual_PreCallValidateCreateIndirectExecutionSetEXT(VkDevice device,
                                                                 const VkIndirectExecutionSetCreateInfoEXT* pCreateInfo,
                                                                 const VkAllocationCallbacks* pAllocator,
                                                                 VkIndirectExecutionSetEXT* pIndirectExecutionSet,
                                                                 const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;

    if (!enabled_features.deviceGeneratedCommands) {
        skip |= LogError("VUID-vkCreateIndirectExecutionSetEXT-deviceGeneratedCommands-11013", device, error_obj.location,
                         "deviceGeneratedCommands feature was not enabled.");
    }

    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    const Location info_loc = create_info_loc.dot(Field::info);

    if (pCreateInfo->type == VK_INDIRECT_EXECUTION_SET_INFO_TYPE_PIPELINES_EXT) {
        if (!pCreateInfo->info.pPipelineInfo) {
            skip |= LogError("VUID-VkIndirectExecutionSetCreateInfoEXT-pPipelineInfo-parameter", device,
                             create_info_loc.dot(Field::type),
                             "is VK_INDIRECT_EXECUTION_SET_INFO_TYPE_PIPELINES_EXT, but info.pPipelineInfo is null.");
        } else {
            skip |= ValidateIndirectExecutionSetPipelineInfo(context, *pCreateInfo->info.pPipelineInfo,
                                                             info_loc.dot(Field::pPipelineInfo));
        }
    } else if (pCreateInfo->type == VK_INDIRECT_EXECUTION_SET_INFO_TYPE_SHADER_OBJECTS_EXT) {
        if (!enabled_features.shaderObject) {
            skip |=
                LogError("VUID-VkIndirectExecutionSetCreateInfoEXT-maxIndirectShaderObjectCount-11014", device,
                         create_info_loc.dot(Field::type),
                         "is VK_INDIRECT_EXECUTION_SET_INFO_TYPE_SHADER_OBJECTS_EXT but the shaderObject feature was not enabled.");
        } else if (phys_dev_ext_props.device_generated_commands_props.maxIndirectShaderObjectCount == 0) {
            skip |= LogError("VUID-VkIndirectExecutionSetCreateInfoEXT-maxIndirectShaderObjectCount-11014", device,
                             create_info_loc.dot(Field::type),
                             "is VK_INDIRECT_EXECUTION_SET_INFO_TYPE_SHADER_OBJECTS_EXT but maxIndirectShaderObjectCount is zero "
                             "(so is no support for device generated commands via shader object).");
        }

        if (!pCreateInfo->info.pShaderInfo) {
            skip |=
                LogError("VUID-VkIndirectExecutionSetCreateInfoEXT-pShaderInfo-parameter", device, create_info_loc.dot(Field::type),
                         "is VK_INDIRECT_EXECUTION_SET_INFO_TYPE_SHADER_OBJECTS_EXT, but info.pShaderInfo is null.");
        } else {
            skip |=
                ValidateIndirectExecutionSetShaderInfo(context, *pCreateInfo->info.pShaderInfo, info_loc.dot(Field::pShaderInfo));
        }
    }

    return skip;
}

bool Device::ValidateIndirectCommandsPushConstantToken(const Context& context,
                                                       const VkIndirectCommandsPushConstantTokenEXT& push_constant_token,
                                                       VkIndirectCommandsTokenTypeEXT token_type,
                                                       const Location& push_constant_token_loc) const {
    bool skip = false;
    skip |= context.ValidateFlags(
        push_constant_token_loc.dot(Field::updateRange).dot(Field::stageFlags), vvl::FlagBitmask::VkShaderStageFlagBits,
        AllVkShaderStageFlagBits, push_constant_token.updateRange.stageFlags, kRequiredFlags,
        "VUID-VkPushConstantRange-stageFlags-parameter", "VUID-VkPushConstantRange-stageFlags-requiredbitmask");

    if (token_type == VK_INDIRECT_COMMANDS_TOKEN_TYPE_SEQUENCE_INDEX_EXT && push_constant_token.updateRange.size != 4) {
        skip |= LogError("VUID-VkIndirectCommandsPushConstantTokenEXT-size-11133", device,
                         push_constant_token_loc.dot(Field::updateRange).dot(Field::size),
                         "is %" PRIu32 ", but needs to be 4 when using VK_INDIRECT_COMMANDS_TOKEN_TYPE_SEQUENCE_INDEX_EXT.",
                         push_constant_token.updateRange.size);
    }

    return skip;
}

bool Device::ValidateIndirectCommandsIndexBufferToken(const Context& context,
                                                      const VkIndirectCommandsIndexBufferTokenEXT& index_buffer_token,
                                                      const Location& index_buffer_token_loc) const {
    bool skip = false;
    skip |= context.ValidateFlags(index_buffer_token_loc.dot(Field::mode), vvl::FlagBitmask::VkIndirectCommandsInputModeFlagBitsEXT,
                                  AllVkIndirectCommandsInputModeFlagBitsEXT, index_buffer_token.mode, kRequiredSingleBit,
                                  "VUID-VkIndirectCommandsIndexBufferTokenEXT-mode-parameter",
                                  "VUID-VkIndirectCommandsIndexBufferTokenEXT-mode-11135");

    const auto& props = phys_dev_ext_props.device_generated_commands_props;
    if ((index_buffer_token.mode & props.supportedIndirectCommandsInputModes) == 0) {
        skip |= LogError("VUID-VkIndirectCommandsIndexBufferTokenEXT-mode-11136", device, index_buffer_token_loc.dot(Field::mode),
                         "is %s, but that is not supported by supportedIndirectCommandsInputModes (%s).",
                         string_VkIndirectCommandsInputModeFlagBitsEXT(index_buffer_token.mode),
                         string_VkIndirectCommandsInputModeFlagsEXT(props.supportedIndirectCommandsInputModes).c_str());
    }
    return skip;
}

bool Device::ValidateIndirectCommandsExecutionSetToken(const Context& context,
                                                       const VkIndirectCommandsExecutionSetTokenEXT& exe_set_token,
                                                       const Location& exe_set_token_loc) const {
    bool skip = false;
    skip |= context.ValidateRangedEnum(exe_set_token_loc.dot(Field::type), vvl::Enum::VkIndirectExecutionSetInfoTypeEXT,
                                       exe_set_token.type, "VUID-VkIndirectCommandsExecutionSetTokenEXT-type-parameter");

    skip |= context.ValidateFlags(exe_set_token_loc.dot(Field::shaderStages), vvl::FlagBitmask::VkShaderStageFlagBits,
                                  AllVkShaderStageFlagBits, exe_set_token.shaderStages, kRequiredFlags,
                                  "VUID-VkIndirectCommandsExecutionSetTokenEXT-shaderStages-parameter",
                                  "VUID-VkIndirectCommandsExecutionSetTokenEXT-shaderStages-requiredbitmask");

    const auto& props = phys_dev_ext_props.device_generated_commands_props;
    if ((exe_set_token.shaderStages & (props.supportedIndirectCommandsShaderStagesPipelineBinding |
                                       props.supportedIndirectCommandsShaderStagesShaderBinding)) == 0) {
        skip |= LogError("VUID-VkIndirectCommandsExecutionSetTokenEXT-shaderStages-11137", device,
                         exe_set_token_loc.dot(Field::shaderStages),
                         "is %s, but that is not supported by supportedIndirectCommandsShaderStagesPipelineBinding (%s) or "
                         "supportedIndirectCommandsShaderStagesShaderBinding (%s).",
                         string_VkShaderStageFlags(exe_set_token.shaderStages).c_str(),
                         string_VkShaderStageFlags(props.supportedIndirectCommandsShaderStagesPipelineBinding).c_str(),
                         string_VkShaderStageFlags(props.supportedIndirectCommandsShaderStagesShaderBinding).c_str());
    }

    return skip;
}

bool Device::ValidateIndirectCommandsLayoutToken(const Context& context, const VkIndirectCommandsLayoutTokenEXT& token,
                                                 const Location& token_loc) const {
    bool skip = false;

    const Location data_loc = token_loc.dot(Field::data);
    switch (token.type) {
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_CONSTANT_EXT:
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_SEQUENCE_INDEX_EXT:
            if (!token.data.pPushConstant) {
                skip |=
                    LogError("VUID-VkIndirectCommandsLayoutTokenEXT-pPushConstant-parameter", device, token_loc.dot(Field::type),
                             "is %s, but data.pPushConstant is null.", string_VkIndirectCommandsTokenTypeEXT(token.type));
            } else {
                skip |= ValidateIndirectCommandsPushConstantToken(context, *token.data.pPushConstant, token.type,
                                                                  data_loc.dot(Field::pPushConstant));
            }
            break;
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_VERTEX_BUFFER_EXT:
            if (!token.data.pVertexBuffer) {
                skip |=
                    LogError("VUID-VkIndirectCommandsLayoutTokenEXT-pVertexBuffer-parameter", device, token_loc.dot(Field::type),
                             "is VK_INDIRECT_COMMANDS_TOKEN_TYPE_VERTEX_BUFFER_EXT, but data.pVertexBuffer is null.");
            }
            break;
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_INDEX_BUFFER_EXT:
            if (!token.data.pIndexBuffer) {
                skip |= LogError("VUID-VkIndirectCommandsLayoutTokenEXT-pIndexBuffer-parameter", device, token_loc.dot(Field::type),
                                 "is VK_INDIRECT_COMMANDS_TOKEN_TYPE_INDEX_BUFFER_EXT, but data.pIndexBuffer is null.");
            } else {
                skip |=
                    ValidateIndirectCommandsIndexBufferToken(context, *token.data.pIndexBuffer, data_loc.dot(Field::pIndexBuffer));
            }
            break;
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_EXECUTION_SET_EXT:
            if (!token.data.pExecutionSet) {
                skip |=
                    LogError("VUID-VkIndirectCommandsLayoutTokenEXT-pExecutionSet-parameter", device, token_loc.dot(Field::type),
                             "is VK_INDIRECT_COMMANDS_TOKEN_TYPE_EXECUTION_SET_EXT, but data.pExecutionSet is null.");
            } else {
                skip |= ValidateIndirectCommandsExecutionSetToken(context, *token.data.pExecutionSet,
                                                                  data_loc.dot(Field::pExecutionSet));
            }
            break;
        default:
            break;
    }

    const auto& props = phys_dev_ext_props.device_generated_commands_props;
    if (IsMeshCommand(token.type)) {
        if (!enabled_features.meshShader || !enabled_features.taskShader) {
            skip |= LogError("VUID-VkIndirectCommandsLayoutTokenEXT-meshShader-11126", device, token_loc.dot(Field::type),
                             "is %s but meshShader and taskShader features are disabled.",
                             string_VkIndirectCommandsTokenTypeEXT(token.type));
        } else if (!props.deviceGeneratedCommandsMultiDrawIndirectCount) {
            if (token.type == VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_COUNT_EXT) {
                skip |= LogError("VUID-VkIndirectCommandsLayoutTokenEXT-deviceGeneratedCommandsMultiDrawIndirectCount-11130",
                                 device, token_loc.dot(Field::type),
                                 "is VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_COUNT_EXT but "
                                 "deviceGeneratedCommandsMultiDrawIndirectCount is not supported.");
            } else if (token.type == VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_COUNT_NV_EXT) {
                skip |= LogError("VUID-VkIndirectCommandsLayoutTokenEXT-deviceGeneratedCommandsMultiDrawIndirectCount-11131",
                                 device, token_loc.dot(Field::type),
                                 "is VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_COUNT_NV_EXT but "
                                 "deviceGeneratedCommandsMultiDrawIndirectCount is not supported.");
            }
        }
    } else if (token.type == VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_INDEXED_COUNT_EXT ||
               token.type == VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_COUNT_EXT) {
        if (!props.deviceGeneratedCommandsMultiDrawIndirectCount) {
            skip |=
                LogError("VUID-VkIndirectCommandsLayoutTokenEXT-deviceGeneratedCommandsMultiDrawIndirectCount-11129", device,
                         token_loc.dot(Field::type), "is %s but deviceGeneratedCommandsMultiDrawIndirectCount is not supported.",
                         string_VkIndirectCommandsTokenTypeEXT(token.type));
        }
    } else if (token.type == VK_INDIRECT_COMMANDS_TOKEN_TYPE_TRACE_RAYS2_EXT && !enabled_features.rayTracingMaintenance1) {
        skip |= LogError("VUID-VkIndirectCommandsLayoutTokenEXT-rayTracingMaintenance1-11128", device, token_loc.dot(Field::type),
                         "is VK_INDIRECT_COMMANDS_TOKEN_TYPE_TRACE_RAYS2_EXT but rayTracingMaintenance1 was not enabled.");
    }

    // offset is ignored for SEQUENCE_INDEX
    if (token.type != VK_INDIRECT_COMMANDS_TOKEN_TYPE_SEQUENCE_INDEX_EXT) {
        if (token.offset > props.maxIndirectCommandsTokenOffset) {
            skip |= LogError("VUID-VkIndirectCommandsLayoutTokenEXT-offset-11124", device, token_loc.dot(Field::offset),
                             "(%" PRIu32 ") is greater than maxIndirectCommandsTokenOffset (%" PRIu32 ")", token.offset,
                             props.maxIndirectCommandsTokenOffset);
        }
        if (SafeModulo(token.offset, 4) != 0) {
            skip |= LogError("VUID-VkIndirectCommandsLayoutTokenEXT-offset-11125", device, token_loc.dot(Field::offset),
                             "(%" PRIu32 ") is not aligned to 4.", token.offset);
        }
    }

    return skip;
}

bool Device::ValidateIndirectCommandsLayoutStage(const Context& context, const VkIndirectCommandsLayoutTokenEXT& token,
                                                 const Location& token_loc, VkShaderStageFlags shader_stages,
                                                 bool has_stage_graphics, bool has_stage_compute, bool has_stage_ray_tracing,
                                                 bool has_stage_mesh) const {
    bool skip = false;
    // Check if type has supported shaderStage
    if (!has_stage_graphics &&
        IsValueIn(
            token.type,
            {VK_INDIRECT_COMMANDS_TOKEN_TYPE_INDEX_BUFFER_EXT, VK_INDIRECT_COMMANDS_TOKEN_TYPE_VERTEX_BUFFER_EXT,
             VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_INDEXED_EXT, VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT,
             VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_INDEXED_COUNT_EXT, VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_COUNT_EXT,
             VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_NV_EXT, VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_COUNT_NV_EXT,
             VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_EXT, VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_COUNT_EXT})) {
        skip |= LogError("VUID-VkIndirectCommandsLayoutCreateInfoEXT-pTokens-11104", device, token_loc.dot(Field::type),
                         "is %s, but shaderStages (%s) doesn't contain graphics stages.",
                         string_VkIndirectCommandsTokenTypeEXT(token.type), string_VkShaderStageFlags(shader_stages).c_str());
    }
    if (!has_stage_compute && token.type == VK_INDIRECT_COMMANDS_TOKEN_TYPE_DISPATCH_EXT) {
        skip |= LogError("VUID-VkIndirectCommandsLayoutCreateInfoEXT-pTokens-11105", device, token_loc.dot(Field::type),
                         "is VK_INDIRECT_COMMANDS_TOKEN_TYPE_DISPATCH_EXT, but shaderStages (%s) doesn't contain "
                         "VK_SHADER_STAGE_COMPUTE_BIT.",
                         string_VkShaderStageFlags(shader_stages).c_str());
    }
    if (!has_stage_mesh && IsValueIn(token.type, {VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_EXT,
                                                  VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_COUNT_EXT})) {
        skip |= LogError("VUID-VkIndirectCommandsLayoutCreateInfoEXT-pTokens-11106", device, token_loc.dot(Field::type),
                         "is %s, but shaderStages (%s) doesn't contain mesh stages.",
                         string_VkIndirectCommandsTokenTypeEXT(token.type), string_VkShaderStageFlags(shader_stages).c_str());
    }
    if (!has_stage_mesh && IsValueIn(token.type, {VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_NV_EXT,
                                                  VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_COUNT_NV_EXT})) {
        skip |= LogError("VUID-VkIndirectCommandsLayoutCreateInfoEXT-pTokens-11107", device, token_loc.dot(Field::type),
                         "is %s, but shaderStages (%s) doesn't contain mesh stages.",
                         string_VkIndirectCommandsTokenTypeEXT(token.type), string_VkShaderStageFlags(shader_stages).c_str());
    }
    if (!has_stage_ray_tracing && token.type == VK_INDIRECT_COMMANDS_TOKEN_TYPE_TRACE_RAYS2_EXT) {
        skip |= LogError(
            "VUID-VkIndirectCommandsLayoutCreateInfoEXT-pTokens-11108", device, token_loc.dot(Field::type),
            "is VK_INDIRECT_COMMANDS_TOKEN_TYPE_TRACE_RAYS2_EXT, but shaderStages (%s) doesn't contain ray tracing stages.",
            string_VkShaderStageFlags(shader_stages).c_str());
    }

    // Check if certain stages, it is limited to certain tokens
    if (has_stage_graphics &&
        IsValueIn(token.type, {VK_INDIRECT_COMMANDS_TOKEN_TYPE_TRACE_RAYS2_EXT, VK_INDIRECT_COMMANDS_TOKEN_TYPE_DISPATCH_EXT})) {
        skip |= LogError("VUID-VkIndirectCommandsLayoutCreateInfoEXT-shaderStages-11109", device, token_loc.dot(Field::type),
                         "is %s but shaderStages is graphics (%s).", string_VkIndirectCommandsTokenTypeEXT(token.type),
                         string_VkShaderStageFlags(shader_stages).c_str());
    } else if (has_stage_compute && !IsComputeCommand(token.type)) {
        skip |=
            LogError("VUID-VkIndirectCommandsLayoutCreateInfoEXT-shaderStages-11110", device, token_loc.dot(Field::type),
                     "is %s but shaderStages is VK_SHADER_STAGE_COMPUTE_BIT.", string_VkIndirectCommandsTokenTypeEXT(token.type));
    } else if (has_stage_ray_tracing && !IsRayTracingCommand(token.type)) {
        skip |= LogError("VUID-VkIndirectCommandsLayoutCreateInfoEXT-shaderStages-11111", device, token_loc.dot(Field::type),
                         "is %s but shaderStages is ray tracing (%s).", string_VkIndirectCommandsTokenTypeEXT(token.type),
                         string_VkShaderStageFlags(shader_stages).c_str());
    }

    return skip;
}

bool Device::manual_PreCallValidateCreateIndirectCommandsLayoutEXT(VkDevice device,
                                                                   const VkIndirectCommandsLayoutCreateInfoEXT* pCreateInfo,
                                                                   const VkAllocationCallbacks* pAllocator,
                                                                   VkIndirectCommandsLayoutEXT* pIndirectCommandsLayout,
                                                                   const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;

    if (!enabled_features.deviceGeneratedCommands) {
        skip |= LogError("VUID-vkCreateIndirectCommandsLayoutEXT-deviceGeneratedCommands-11089", device, error_obj.location,
                         "deviceGeneratedCommands feature was not enabled.");
    }

    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    const auto& props = phys_dev_ext_props.device_generated_commands_props;
    if (pCreateInfo->indirectStride > props.maxIndirectCommandsIndirectStride) {
        skip |= LogError("VUID-VkIndirectCommandsLayoutCreateInfoEXT-indirectStride-11090", device,
                         create_info_loc.dot(Field::indirectStride),
                         "(%" PRIu32 ") is greater than maxIndirectCommandsIndirectStride (%" PRIu32 ")",
                         pCreateInfo->indirectStride, props.maxIndirectCommandsIndirectStride);
    }
    const VkShaderStageFlags shader_stages = pCreateInfo->shaderStages;
    if (shader_stages & ~props.supportedIndirectCommandsShaderStages) {
        skip |= LogError("VUID-VkIndirectCommandsLayoutCreateInfoEXT-shaderStages-11091", device,
                         create_info_loc.dot(Field::shaderStages),
                         "is %s which contain stages not found in supportedIndirectCommandsShaderStages (%s)",
                         string_VkShaderStageFlags(shader_stages).c_str(),
                         string_VkShaderStageFlags(props.supportedIndirectCommandsShaderStages).c_str());
    }
    if (pCreateInfo->tokenCount > props.maxIndirectCommandsTokenCount) {
        skip |=
            LogError("VUID-VkIndirectCommandsLayoutCreateInfoEXT-tokenCount-11092", device, create_info_loc.dot(Field::tokenCount),
                     "(%" PRIu32 ") is greater than maxIndirectCommandsTokenCount (%" PRIu32 ")", pCreateInfo->tokenCount,
                     props.maxIndirectCommandsTokenCount);
    }

    if (shader_stages & VK_SHADER_STAGE_FRAGMENT_BIT) {
        if (!(shader_stages & (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_MESH_BIT_EXT))) {
            skip |= LogError("VUID-VkIndirectCommandsLayoutCreateInfoEXT-shaderStages-11113", device,
                             create_info_loc.dot(Field::shaderStages),
                             "(%s) contains VK_SHADER_STAGE_FRAGMENT_BIT but does not contains VK_SHADER_STAGE_VERTEX_BIT or "
                             "VK_SHADER_STAGE_MESH_BIT_EXT.",
                             string_VkShaderStageFlags(shader_stages).c_str());
        }
    }

    const bool has_stage_graphics = shader_stages & kShaderStageAllGraphics;
    const bool has_stage_compute = shader_stages & VK_SHADER_STAGE_COMPUTE_BIT;
    const bool has_stage_ray_tracing = shader_stages & kShaderStageAllRayTracing;
    const bool has_stage_mesh = shader_stages & VK_SHADER_STAGE_MESH_BIT_EXT;  // also VK_SHADER_STAGE_MESH_BIT_NV
    {
        // Mesh can/will have a fragment stage in it
        const VkShaderStageFlags all_mesh_stages =
            VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_FRAGMENT_BIT;

        bool valid_stages = true;
        if (has_stage_graphics && ((shader_stages | kShaderStageAllGraphics) != kShaderStageAllGraphics)) {
            valid_stages = false;
        } else if (has_stage_compute && ((shader_stages | VK_SHADER_STAGE_COMPUTE_BIT) != VK_SHADER_STAGE_COMPUTE_BIT)) {
            valid_stages = false;
        } else if (has_stage_ray_tracing && ((shader_stages | kShaderStageAllRayTracing) != kShaderStageAllRayTracing)) {
            valid_stages = false;
        } else if (has_stage_mesh && ((shader_stages | all_mesh_stages) != all_mesh_stages)) {
            valid_stages = false;
        }
        if (!valid_stages) {
            skip |= LogError("VUID-VkIndirectCommandsLayoutCreateInfoEXT-shaderStages-11112", device,
                             create_info_loc.dot(Field::shaderStages),
                             "is %s but you can't mix graphics/compute/mesh/rayTracing stages with each other.",
                             string_VkShaderStageFlags(shader_stages).c_str());
        }
    }

    ASSERT_AND_RETURN_SKIP(pCreateInfo->pTokens);

    uint32_t current_token_offset = 0;

    for (uint32_t i = 0; i < pCreateInfo->tokenCount; ++i) {
        const Location token_loc = create_info_loc.dot(Field::pTokens, i);
        const auto& token = pCreateInfo->pTokens[i];
        skip |= ValidateIndirectCommandsLayoutToken(context, token, token_loc);
        skip |= ValidateIndirectCommandsLayoutStage(context, token, token_loc, shader_stages, has_stage_graphics, has_stage_compute,
                                                    has_stage_ray_tracing, has_stage_mesh);

        if (token.type != VK_INDIRECT_COMMANDS_TOKEN_TYPE_SEQUENCE_INDEX_EXT) {
            if (token.offset < current_token_offset) {
                skip |= LogError("VUID-VkIndirectCommandsLayoutCreateInfoEXT-pTokens-11103", device, token_loc.dot(Field::offset),
                                 "(%" PRIu32 ") is less than pTokens[%" PRIu32 "].offset (%" PRIu32 ")", token.offset, i - 1,
                                 pCreateInfo->pTokens[i - 1].offset);
            }
            // is a monotonic increasing value so can give previous value
            current_token_offset = token.offset;
        }
    }

    return skip;
}

bool Device::ValidateGeneratedCommandsInfo(VkCommandBuffer command_buffer,
                                           const VkGeneratedCommandsInfoEXT& generated_commands_info,
                                           const Location& info_loc) const {
    bool skip = false;

    if (generated_commands_info.sequenceCountAddress != 0) {
        if (SafeModulo(generated_commands_info.sequenceCountAddress, 4) != 0) {
            skip |= LogError("VUID-VkGeneratedCommandsInfoEXT-sequenceCountAddress-11073", command_buffer,
                             info_loc.dot(Field::sequenceCountAddress), "(%" PRIuLEAST64 ") is not aligned to 4.",
                             generated_commands_info.sequenceCountAddress);
        }
    }
    if (generated_commands_info.maxSequenceCount == 0) {
        skip |= LogError("VUID-VkGeneratedCommandsInfoEXT-maxSequenceCount-10246", command_buffer,
                         info_loc.dot(Field::maxSequenceCount), "is zero.");
    }

    if (generated_commands_info.indirectAddress == 0) {
        skip |= LogError("VUID-VkGeneratedCommandsInfoEXT-indirectAddress-11076", command_buffer,
                         info_loc.dot(Field::indirectAddress), "is NULL.");
    } else if (SafeModulo(generated_commands_info.indirectAddress, 4) != 0) {
        skip |=
            LogError("VUID-VkGeneratedCommandsInfoEXT-indirectAddress-11074", command_buffer, info_loc.dot(Field::indirectAddress),
                     "(%" PRIuLEAST64 ") is not aligned to 4.", generated_commands_info.indirectAddress);
    }

    if (generated_commands_info.indirectAddressSize == 0) {
        skip |= LogError("VUID-VkGeneratedCommandsInfoEXT-indirectAddressSize-11077", command_buffer,
                         info_loc.dot(Field::indirectAddressSize), "is zero.");
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdPreprocessGeneratedCommandsEXT(VkCommandBuffer commandBuffer,
                                                                     const VkGeneratedCommandsInfoEXT* pGeneratedCommandsInfo,
                                                                     VkCommandBuffer stateCommandBuffer,
                                                                     const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;
    if (!enabled_features.deviceGeneratedCommands) {
        skip |= LogError("VUID-vkCmdPreprocessGeneratedCommandsEXT-deviceGeneratedCommands-11087", device, error_obj.location,
                         "deviceGeneratedCommands feature was not enabled.");
    }

    const Location info_loc = error_obj.location.dot(Field::pGeneratedCommandsInfo);
    if (pGeneratedCommandsInfo->shaderStages &
        ~phys_dev_ext_props.device_generated_commands_props.supportedIndirectCommandsShaderStages) {
        skip |= LogError(
            "VUID-vkCmdPreprocessGeneratedCommandsEXT-supportedIndirectCommandsShaderStages-11088", commandBuffer,
            info_loc.dot(Field::shaderStages), "(%s) contains stages not found in supportedIndirectCommandsShaderStages (%s).",
            string_VkShaderStageFlags(pGeneratedCommandsInfo->shaderStages).c_str(),
            string_VkShaderStageFlags(phys_dev_ext_props.device_generated_commands_props.supportedIndirectCommandsShaderStages)
                .c_str());
    }

    skip |= ValidateGeneratedCommandsInfo(commandBuffer, *pGeneratedCommandsInfo, info_loc);

    return skip;
}

bool Device::manual_PreCallValidateCmdExecuteGeneratedCommandsEXT(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
                                                                  const VkGeneratedCommandsInfoEXT* pGeneratedCommandsInfo,
                                                                  const Context& context) const {
    bool skip = false;
    const auto& error_obj = context.error_obj;

    if (!enabled_features.deviceGeneratedCommands) {
        skip |= LogError("VUID-vkCmdExecuteGeneratedCommandsEXT-deviceGeneratedCommands-11059", device, error_obj.location,
                         "deviceGeneratedCommands feature was not enabled.");
    }

    const Location info_loc = error_obj.location.dot(Field::pGeneratedCommandsInfo);
    if (pGeneratedCommandsInfo->shaderStages &
        ~phys_dev_ext_props.device_generated_commands_props.supportedIndirectCommandsShaderStages) {
        skip |= LogError(
            "VUID-vkCmdExecuteGeneratedCommandsEXT-supportedIndirectCommandsShaderStages-11061", commandBuffer,
            info_loc.dot(Field::shaderStages), "(%s) contains stages not found in supportedIndirectCommandsShaderStages (%s).",
            string_VkShaderStageFlags(pGeneratedCommandsInfo->shaderStages).c_str(),
            string_VkShaderStageFlags(phys_dev_ext_props.device_generated_commands_props.supportedIndirectCommandsShaderStages)
                .c_str());
    }

    skip |= ValidateGeneratedCommandsInfo(commandBuffer, *pGeneratedCommandsInfo, info_loc);

    return skip;
}
}  // namespace stateless
