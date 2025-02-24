/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
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

#include "error_message/error_location.h"
#include "stateless/stateless_validation.h"
#include "generated/enum_flag_bits.h"
#include "generated/dispatch_functions.h"
#include "stateless/sl_vuid_maps.h"

namespace stateless {

bool Device::manual_PreCallValidateCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo,
                                                      const VkAllocationCallbacks *pAllocator, VkShaderModule *pShaderModule,
                                                      const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    constexpr std::array allowed_structs = {VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
                                            VK_STRUCTURE_TYPE_SHADER_MODULE_VALIDATION_CACHE_CREATE_INFO_EXT};

    skip |= context.ValidateStructPnext(error_obj.location.dot(Field::pCreateInfo), pCreateInfo->pNext, allowed_structs.size(),
                                        allowed_structs.data(), GeneratedVulkanHeaderVersion,
                                        "VUID-vkCreateShaderModule-pCreateInfo-06904",
                                        "VUID-vkCreateShaderModule-pCreateInfo-06904", true);
    return skip;
}

bool Device::manual_PreCallValidateCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                                        const VkAllocationCallbacks *pAllocator, VkPipelineLayout *pPipelineLayout,
                                                        const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    // Validate layout count against device physical limit
    if (pCreateInfo->setLayoutCount > device_limits.maxBoundDescriptorSets) {
        skip |= LogError("VUID-VkPipelineLayoutCreateInfo-setLayoutCount-00286", device, create_info_loc.dot(Field::setLayoutCount),
                         "(%" PRIu32 ") exceeds physical device maxBoundDescriptorSets limit (%" PRIu32 ").",
                         pCreateInfo->setLayoutCount, device_limits.maxBoundDescriptorSets);
    }

    if (!IsExtEnabled(extensions.vk_ext_graphics_pipeline_library)) {
        for (uint32_t i = 0; i < pCreateInfo->setLayoutCount; ++i) {
            if (!pCreateInfo->pSetLayouts[i]) {
                // TODO - Combine with other check in CoreChecks
                skip |= LogError("VUID-VkPipelineLayoutCreateInfo-graphicsPipelineLibrary-06753", device,
                                 create_info_loc.dot(Field::pSetLayouts, i),
                                 "is VK_NULL_HANDLE, but VK_EXT_graphics_pipeline_library is not enabled.");
            }
        }
    }

    skip |= ValidatePushConstantRange(pCreateInfo->pushConstantRangeCount, pCreateInfo->pPushConstantRanges, create_info_loc);

    return skip;
}

bool Device::ValidatePushConstantRange(uint32_t push_constant_range_count, const VkPushConstantRange *push_constant_ranges,
                                       const Location &loc) const {
    bool skip = false;

    for (uint32_t i = 0; i < push_constant_range_count; ++i) {
        const Location pc_loc = loc.dot(Field::pPushConstantRanges, i);
        const uint32_t offset = push_constant_ranges[i].offset;
        const uint32_t size = push_constant_ranges[i].size;
        const uint32_t max_push_constants_size = device_limits.maxPushConstantsSize;
        // Check that offset + size don't exceed the max.
        // Prevent arithetic overflow here by avoiding addition and testing in this order.
        if (offset >= max_push_constants_size) {
            skip |= LogError("VUID-VkPushConstantRange-offset-00294", device, pc_loc.dot(Field::offset),
                             "(%" PRIu32 ") is greater than maxPushConstantSize (%" PRIu32 ").", offset, max_push_constants_size);
        }
        if (size > max_push_constants_size - offset) {
            skip |= LogError("VUID-VkPushConstantRange-size-00298", device, pc_loc.dot(Field::size),
                             "(%" PRIu32 ") plus offset (%" PRIu32 ") is greater than maxPushConstantSize (%" PRIu32 ").", size,
                             offset, max_push_constants_size);
        }

        // size needs to be non-zero and a multiple of 4.
        if (size == 0) {
            skip |= LogError("VUID-VkPushConstantRange-size-00296", device, pc_loc.dot(Field::size),
                             "(%" PRIu32 ") is not greater than zero.", size);
        }
        if (size & 0x3) {
            skip |= LogError("VUID-VkPushConstantRange-size-00297", device, pc_loc.dot(Field::size),
                             "(%" PRIu32 ") is not a multiple of 4.", size);
        }

        // offset needs to be a multiple of 4.
        if ((offset & 0x3) != 0) {
            skip |= LogError("VUID-VkPushConstantRange-offset-00295", device, pc_loc.dot(Field::offset),
                             "(%" PRIu32 ") is not a multiple of 4.", offset);
        }
    }

    // As of 1.0.28, there is a VU that states that a stage flag cannot appear more than once in the list of push constant ranges.
    for (uint32_t i = 0; i < push_constant_range_count; ++i) {
        for (uint32_t j = i + 1; j < push_constant_range_count; ++j) {
            if (0 != (push_constant_ranges[i].stageFlags & push_constant_ranges[j].stageFlags)) {
                const char *vuid = loc.function == Func::vkCreatePipelineLayout
                                       ? "VUID-VkPipelineLayoutCreateInfo-pPushConstantRanges-00292"
                                       : "VUID-VkShaderCreateInfoEXT-pPushConstantRanges-10063";
                skip |= LogError(vuid, device, loc,
                                 "pPushConstantRanges[%" PRIu32
                                 "].stageFlags is the same stage (%s) as pPushConstantRanges[%" PRIu32 "].stageFlags.",
                                 i, string_VkShaderStageFlags(push_constant_ranges[i].stageFlags).c_str(), j);
                break;  // Only need to report the first range mismatch
            }
        }
    }

    return skip;
}

// Called from graphics, compute, raytracing, etc
bool Device::ValidatePipelineShaderStageCreateInfoCommon(const Context &context, const VkPipelineShaderStageCreateInfo &create_info,
                                                         const Location &loc) const {
    bool skip = false;

    if (create_info.pName) {
        skip |= context.ValidateString(loc.dot(Field::pName), "VUID-VkPipelineShaderStageCreateInfo-pName-parameter",
                                       create_info.pName);
    }

    if (vku::FindStructInPNextChain<VkPipelineShaderStageRequiredSubgroupSizeCreateInfo>(create_info.pNext)) {
        if ((create_info.flags & VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT) != 0) {
            skip |= LogError("VUID-VkPipelineShaderStageCreateInfo-pNext-02754", device, loc.dot(Field::flags),
                             "(%s) includes VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT while "
                             "VkPipelineShaderStageRequiredSubgroupSizeCreateInfo is included in the pNext chain.",
                             string_VkPipelineShaderStageCreateFlags(create_info.flags).c_str());
        }
    }
    return skip;
}

// Called from graphics, compute, raytracing, etc
bool Device::ValidatePipelineBinaryInfo(const void *next, VkPipelineCreateFlags flags, VkPipelineCache pipelineCache,
                                        const Location &loc) const {
    bool skip = false;
    const auto *temp_flags_2 = vku::FindStructInPNextChain<VkPipelineCreateFlags2CreateInfo>(next);
    const VkPipelineCreateFlags2KHR create_flags_2 =
        temp_flags_2 ? temp_flags_2->flags : static_cast<VkPipelineCreateFlags2KHR>(flags);
    const Location flag_loc =
        temp_flags_2 ? loc.pNext(Struct::VkPipelineCreateFlags2CreateInfo, Field::flags) : loc.dot(Field::flags);

    if (temp_flags_2 && (create_flags_2 & VK_PIPELINE_CREATE_2_CAPTURE_DATA_BIT_KHR) && (pipelineCache != VK_NULL_HANDLE)) {
        skip |= LogError(GetPipelineBinaryInfoVUID(flag_loc, vvl::PipelineBinaryInfoError::PNext_09617), device, flag_loc,
                         "(%s) includes VK_PIPELINE_CREATE_2_CAPTURE_DATA_BIT_KHR while "
                         "pipelineCache is not VK_NULL_HANDLE.",
                         string_VkPipelineCreateFlags2(create_flags_2).c_str());
    }

    const auto binary_info = vku::FindStructInPNextChain<VkPipelineBinaryInfoKHR>(next);

    if (binary_info && (binary_info->binaryCount > 0)) {
        if (pipelineCache != VK_NULL_HANDLE) {
            skip |= LogError(GetPipelineBinaryInfoVUID(flag_loc, vvl::PipelineBinaryInfoError::PNext_09616), device, loc.pNext(Struct::VkPipelineBinaryInfoKHR, Field::binaryCount),
                             "(%" PRIu32 ") is greated than zero while  "
                             "pipelineCache is not VK_NULL_HANDLE.",
                             binary_info->binaryCount);
        }

        const auto creation_feedback = vku::FindStructInPNextChain<VkPipelineCreationFeedbackCreateInfo>(next);
        if (creation_feedback) {
            if (creation_feedback->pPipelineCreationFeedback->flags &
                VK_PIPELINE_CREATION_FEEDBACK_APPLICATION_PIPELINE_CACHE_HIT_BIT) {
                skip |= LogError(
                    GetPipelineBinaryInfoVUID(flag_loc, vvl::PipelineBinaryInfoError::BinaryCount_09620), device,
                    loc.pNext(Struct::VkPipelineCreationFeedbackCreateInfo, Field::pPipelineCreationFeedback).dot(Field::flags),
                    "(%s) includes VK_PIPELINE_CREATION_FEEDBACK_APPLICATION_PIPELINE_CACHE_HIT_BIT while "
                    "binaryCount is greater than zero.",
                    string_VkPipelineCreateFlags2(creation_feedback->pPipelineCreationFeedback->flags).c_str());
            }

            if (creation_feedback->pPipelineCreationFeedback->flags &
                VK_PIPELINE_CREATION_FEEDBACK_BASE_PIPELINE_ACCELERATION_BIT) {
                skip |= LogError(
                    GetPipelineBinaryInfoVUID(flag_loc, vvl::PipelineBinaryInfoError::BinaryCount_09621), device,
                    loc.pNext(Struct::VkPipelineCreationFeedbackCreateInfo, Field::pPipelineCreationFeedback).dot(Field::flags),
                    "(%s) includes VK_PIPELINE_CREATION_FEEDBACK_BASE_PIPELINE_ACCELERATION_BIT while "
                    "binaryCount is greater than zero.",
                    string_VkPipelineCreateFlags2(creation_feedback->pPipelineCreationFeedback->flags).c_str());
            }
        }

        if (create_flags_2 & VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT) {
            skip |= LogError(GetPipelineBinaryInfoVUID(flag_loc, vvl::PipelineBinaryInfoError::BinaryCount_09622), device, flag_loc,
                             "(%s) includes VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT while "
                             "binaryCount is greater than zero.",
                             string_VkPipelineCreateFlags2(flags).c_str());
        }
    }

    return skip;
}

bool Device::ValidatePipelineRenderingCreateInfo(const Context &context, const VkPipelineRenderingCreateInfo &rendering_struct,
                                                 const Location &loc) const {
    bool skip = false;

    if ((rendering_struct.depthAttachmentFormat != VK_FORMAT_UNDEFINED)) {
        skip |= context.ValidateRangedEnum(loc.pNext(Struct::VkPipelineRenderingCreateInfo, Field::depthAttachmentFormat),
                                           vvl::Enum::VkFormat, rendering_struct.depthAttachmentFormat,
                                           "VUID-VkGraphicsPipelineCreateInfo-renderPass-06583");

        if (!vkuFormatHasDepth(rendering_struct.depthAttachmentFormat)) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-renderPass-06587", device,
                             loc.pNext(Struct::VkPipelineRenderingCreateInfo, Field::depthAttachmentFormat),
                             "(%s) does not have a depth aspect (need to use a depth format).",
                             string_VkFormat(rendering_struct.depthAttachmentFormat));
        }
    }

    if ((rendering_struct.stencilAttachmentFormat != VK_FORMAT_UNDEFINED)) {
        skip |= context.ValidateRangedEnum(loc.pNext(Struct::VkPipelineRenderingCreateInfo, Field::stencilAttachmentFormat),
                                           vvl::Enum::VkFormat, rendering_struct.stencilAttachmentFormat,
                                           "VUID-VkGraphicsPipelineCreateInfo-renderPass-06584");
        if (!vkuFormatHasStencil(rendering_struct.stencilAttachmentFormat)) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-renderPass-06588", device,
                             loc.pNext(Struct::VkPipelineRenderingCreateInfo, Field::stencilAttachmentFormat),
                             "(%s) does not have a stencil aspect (need to use a stencil format).",
                             string_VkFormat(rendering_struct.stencilAttachmentFormat));
        }
    }

    if (rendering_struct.colorAttachmentCount != 0) {
        skip |= context.ValidateRangedEnumArray(
            loc.pNext(Struct::VkPipelineRenderingCreateInfo, Field::colorAttachmentCount),
            loc.pNext(Struct::VkPipelineRenderingCreateInfo, Field::pColorAttachmentFormats), vvl::Enum::VkFormat,
            rendering_struct.colorAttachmentCount, rendering_struct.pColorAttachmentFormats, true, true,
            "VUID-VkGraphicsPipelineCreateInfo-renderPass-06579", "VUID-VkGraphicsPipelineCreateInfo-renderPass-06579");
        if (rendering_struct.colorAttachmentCount > device_limits.maxColorAttachments) {
            skip |= LogError("VUID-VkPipelineRenderingCreateInfo-colorAttachmentCount-09533", device,
                             loc.pNext(Struct::VkPipelineRenderingCreateInfo, Field::colorAttachmentCount),
                             "(%" PRIu32 ") is larger than maxColorAttachments (%" PRIu32 ").",
                             rendering_struct.colorAttachmentCount, device_limits.maxColorAttachments);
        }
    }

    if (rendering_struct.pColorAttachmentFormats) {
        for (uint32_t j = 0; j < rendering_struct.colorAttachmentCount; ++j) {
            skip |= context.ValidateRangedEnum(loc.pNext(Struct::VkPipelineRenderingCreateInfo, Field::pColorAttachmentFormats, j),
                                               vvl::Enum::VkFormat, rendering_struct.pColorAttachmentFormats[j],
                                               "VUID-VkGraphicsPipelineCreateInfo-renderPass-06580");
        }
    }

    return skip;
}

bool Device::ValidateCreateGraphicsPipelinesFlags(const VkPipelineCreateFlags2KHR flags, const Location &flags_loc) const {
    bool skip = false;
    if ((flags & VK_PIPELINE_CREATE_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT) != 0 &&
        (flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) == 0) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-flags-09245", device, flags_loc, "is (%s).",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }

    if ((flags & VK_PIPELINE_CREATE_INDIRECT_BINDABLE_BIT_NV) != 0) {
        if (!enabled_features.deviceGeneratedCommands) {
            skip |=
                LogError("VUID-VkGraphicsPipelineCreateInfo-flags-02877", device, flags_loc,
                         "(%s) contains VK_PIPELINE_CREATE_INDIRECT_BINDABLE_BIT_NV, but deviceGeneratedCommands was not enabled.",
                         string_VkPipelineCreateFlags2(flags).c_str());
        }
    }

    if ((flags & VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT) != 0) {
        if (!enabled_features.deviceGeneratedCommands) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-flags-11000", device, flags_loc,
                             "(%s) contains VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT, but "
                             "VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT::deviceGeneratedCommands is not enabled.",
                             string_VkPipelineCreateFlags2(flags).c_str());
        }
    }

    if ((flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) != 0) {
        if (!enabled_features.graphicsPipelineLibrary) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-graphicsPipelineLibrary-06606", device, flags_loc,
                             "(%s) contains VK_PIPELINE_CREATE_LIBRARY_BIT_KHR, but graphicsPipelineLibrary was not enabled.",
                             string_VkPipelineCreateFlags2(flags).c_str());
        }
    }

    if ((flags & VK_PIPELINE_CREATE_DISPATCH_BASE) != 0) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-flags-00764", device, flags_loc,
                         "(%s) must not include "
                         "VK_PIPELINE_CREATE_DISPATCH_BASE.",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }
    if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR) != 0) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-flags-03372", device, flags_loc,
                         "(%s) must not include VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR.",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }
    if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR) != 0) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-flags-03373", device, flags_loc,
                         "(%s) must not include VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR.",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }
    if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_MISS_SHADERS_BIT_KHR) != 0) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-flags-03374", device, flags_loc,
                         "(%s) must not include VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_MISS_SHADERS_BIT_KHR.",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }
    if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR) != 0) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-flags-03375", device, flags_loc,
                         "(%s) must not include VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR.",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }
    if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR) != 0) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-flags-03376", device, flags_loc,
                         "(%s) must not include VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR.",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }
    if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR) != 0) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-flags-03377", device, flags_loc,
                         "(%s) must not include VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR.",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }
    if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR) != 0) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-flags-03577", device, flags_loc,
                         "(%s) must not include VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR.",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }
    if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_ALLOW_MOTION_BIT_NV) != 0) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-flags-04947", device, flags_loc,
                         "(%s) must not include VK_PIPELINE_CREATE_RAY_TRACING_ALLOW_MOTION_BIT_NV.",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }
    if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_OPACITY_MICROMAP_BIT_EXT) != 0) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-flags-07401", device, flags_loc,
                         "(%s) must not include VK_PIPELINE_CREATE_RAY_TRACING_OPACITY_MICROMAP_BIT_EXT.",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }
    if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_DISPLACEMENT_MICROMAP_BIT_NV) != 0) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-flags-07997", device, flags_loc,
                         "(%s) must not include VK_PIPELINE_CREATE_RAY_TRACING_DISPLACEMENT_MICROMAP_BIT_NV.",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }
    return skip;
}

bool Device::manual_PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                           const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                           const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                           const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (!pCreateInfos) {
        return skip;
    }

    for (uint32_t i = 0; i < createInfoCount; ++i) {
        const Location create_info_loc = error_obj.location.dot(Field::pCreateInfos, i);
        bool has_pre_raster_state = true;
        // Create a copy of create_info and set non-included sub-state to null
        VkGraphicsPipelineCreateInfo create_info = pCreateInfos[i];

        const auto *create_flags_2 = vku::FindStructInPNextChain<VkPipelineCreateFlags2CreateInfo>(create_info.pNext);
        const VkPipelineCreateFlags2KHR flags =
            create_flags_2 ? create_flags_2->flags : static_cast<VkPipelineCreateFlags2KHR>(create_info.flags);
        const Location flags_loc = create_flags_2 ? create_info_loc.pNext(Struct::VkPipelineCreateFlags2CreateInfo, Field::flags)
                                                  : create_info_loc.dot(Field::flags);
        if (!create_flags_2) {
            skip |= context.ValidateFlags(flags_loc, vvl::FlagBitmask::VkPipelineCreateFlagBits, AllVkPipelineCreateFlagBits,
                                          create_info.flags, kOptionalFlags, "VUID-VkGraphicsPipelineCreateInfo-None-09497");
        }
        skip |= ValidateCreateGraphicsPipelinesFlags(flags, flags_loc);

        const auto *graphics_lib_info = vku::FindStructInPNextChain<VkGraphicsPipelineLibraryCreateInfoEXT>(create_info.pNext);
        if (graphics_lib_info) {
            if (!(graphics_lib_info->flags & VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT)) {
                create_info.pVertexInputState = nullptr;
                create_info.pInputAssemblyState = nullptr;
            }
            if (!(graphics_lib_info->flags & VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT)) {
                has_pre_raster_state = false;
                create_info.pViewportState = nullptr;
                create_info.pRasterizationState = nullptr;
                create_info.pTessellationState = nullptr;
            }
            if (!(graphics_lib_info->flags & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT)) {
                create_info.pDepthStencilState = nullptr;
            }
            if (!(graphics_lib_info->flags & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT)) {
                create_info.pColorBlendState = nullptr;
            }
            if (!(graphics_lib_info->flags & (VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT |
                                              VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT))) {
                create_info.pMultisampleState = nullptr;
            }
            if (!(graphics_lib_info->flags & (VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT |
                                              VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT))) {
                create_info.layout = VK_NULL_HANDLE;
            }
            if (!(graphics_lib_info->flags & (VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT |
                                              VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT |
                                              VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT))) {
                create_info.renderPass = VK_NULL_HANDLE;
                create_info.subpass = 0;
            }
        }

        if (!create_info.renderPass) {
            // Pipeline has fragment output state
            if ((flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) == 0 ||
                (create_info.pColorBlendState && create_info.pMultisampleState)) {
                const auto rendering_struct = vku::FindStructInPNextChain<VkPipelineRenderingCreateInfo>(create_info.pNext);
                if (rendering_struct) {
                    skip |= ValidatePipelineRenderingCreateInfo(context, *rendering_struct, create_info_loc);
                }

                // VkAttachmentSampleCountInfoAMD == VkAttachmentSampleCountInfoNV
                auto attachment_sample_count_info = vku::FindStructInPNextChain<VkAttachmentSampleCountInfoAMD>(create_info.pNext);
                if (attachment_sample_count_info && attachment_sample_count_info->pColorAttachmentSamples) {
                    for (uint32_t j = 0; j < attachment_sample_count_info->colorAttachmentCount; ++j) {
                        skip |= context.ValidateFlags(
                            create_info_loc.pNext(Struct::VkAttachmentSampleCountInfoAMD, Field::pColorAttachmentSamples),
                            vvl::FlagBitmask::VkSampleCountFlagBits, AllVkSampleCountFlagBits,
                            attachment_sample_count_info->pColorAttachmentSamples[j], kRequiredFlags,
                            "VUID-VkGraphicsPipelineCreateInfo-pColorAttachmentSamples-06592");
                    }
                }
            }
        }

        if (!IsExtEnabled(extensions.vk_ext_graphics_pipeline_library)) {
            if (create_info.stageCount == 0) {
                // Because not using GPL, this will always have a complete state and require a shader
                skip |=
                    LogError("VUID-VkGraphicsPipelineCreateInfo-stageCount-09530", device, create_info_loc.dot(Field::stageCount),
                             "is 0, but %s is not enabled", VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
            }

            skip |= context.ValidateStructTypeArray(
                create_info_loc.dot(Field::stageCount), create_info_loc.dot(Field::pStages), create_info.stageCount,
                create_info.pStages, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, true, true,
                "VUID-VkPipelineShaderStageCreateInfo-sType-sType", "VUID-VkGraphicsPipelineCreateInfo-pStages-06600",
                "VUID-VkGraphicsPipelineCreateInfo-pStages-06600");
            // Can be null with enough dynamic states
            skip |= context.ValidateStructType(create_info_loc.dot(Field::pRasterizationState), create_info.pRasterizationState,
                                               VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, false,
                                               "VUID-VkGraphicsPipelineCreateInfo-pRasterizationState-09040",
                                               "VUID-VkPipelineRasterizationStateCreateInfo-sType-sType");
        }

        if (graphics_lib_info && (graphics_lib_info->flags & (VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT))) {
            skip |= context.ValidateArray(create_info_loc.dot(Field::stageCount), create_info_loc.dot(Field::pStages),
                                          create_info.stageCount, &create_info.pStages, true, true,
                                          "VUID-VkGraphicsPipelineCreateInfo-flags-06644",
                                          "VUID-VkGraphicsPipelineCreateInfo-flags-06640");
        }

        // <VkDynamicState, index in pDynamicStates, hash for enum key>
        vvl::unordered_map<VkDynamicState, uint32_t, std::hash<int>> dynamic_state_map;
        // TODO probably should check dynamic state from graphics libraries, at least when creating an "executable pipeline"
        if (create_info.pDynamicState != nullptr) {
            const auto &dynamic_state_info = *create_info.pDynamicState;
            for (uint32_t state_index = 0; state_index < dynamic_state_info.dynamicStateCount; ++state_index) {
                const VkDynamicState dynamic_state = dynamic_state_info.pDynamicStates[state_index];

                if (vvl::Contains(dynamic_state_map, dynamic_state)) {
                    skip |= LogError("VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442", device,
                                     create_info_loc.dot(Field::pDynamicState),
                                     "has %s at both pDynamicStates[%" PRIu32 "] and pDynamicStates[%" PRIu32 "].",
                                     string_VkDynamicState(dynamic_state), dynamic_state_map[dynamic_state], state_index);
                }

                dynamic_state_map[dynamic_state] = state_index;
            }
        }

        if (vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR)) {
            // Not allowed for graphics pipelines
            skip |=
                LogError("VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-03578", device,
                         create_info_loc.dot(Field::pDynamicState)
                             .dot(Field::pDynamicStates, dynamic_state_map[VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR]),
                         "is VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR.");
        }

        if (has_pre_raster_state && vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT) &&
            vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_VIEWPORT)) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04132", device, create_info_loc,
                             "pDynamicStates[%" PRIu32 "] is VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT but pDynamicStates[%" PRIu32
                             "] is VK_DYNAMIC_STATE_VIEWPORT.",
                             dynamic_state_map[VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT], dynamic_state_map[VK_DYNAMIC_STATE_VIEWPORT]);
        }

        if (has_pre_raster_state && vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT) &&
            vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_SCISSOR)) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04133", device, create_info_loc,
                             "pDynamicStates[%" PRIu32 "] is VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT but pDynamicStates[%" PRIu32
                             "] is VK_DYNAMIC_STATE_SCISSOR.",
                             dynamic_state_map[VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT], dynamic_state_map[VK_DYNAMIC_STATE_SCISSOR]);
        }

        if (vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT) &&
            discard_rectangles_extension_version < 2) {
            skip |= LogError(
                "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-07855", device,
                create_info_loc.dot(Field::pDynamicState)
                    .dot(Field::pDynamicStates, dynamic_state_map[VK_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT]),
                "is VK_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT without support for version 2 of VK_EXT_discard_rectangles.");
        }
        if (vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_DISCARD_RECTANGLE_MODE_EXT) &&
            discard_rectangles_extension_version < 2) {
            skip |= LogError(
                "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-07856", device,
                create_info_loc.dot(Field::pDynamicState)
                    .dot(Field::pDynamicStates, dynamic_state_map[VK_DYNAMIC_STATE_DISCARD_RECTANGLE_MODE_EXT]),
                "is VK_DYNAMIC_STATE_DISCARD_RECTANGLE_MODE_EXT without support for version 2 of VK_EXT_discard_rectangles.");
        }
        if (vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_ENABLE_NV) &&
            scissor_exclusive_extension_version < 2) {
            skip |= LogError(
                "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-07854", device,
                create_info_loc.dot(Field::pDynamicState)
                    .dot(Field::pDynamicStates, dynamic_state_map[VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_ENABLE_NV]),
                "is VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_ENABLE_NV without support for version 2 of VK_NV_scissor_exclusive.");
        }

        auto feedback_struct = vku::FindStructInPNextChain<VkPipelineCreationFeedbackCreateInfo>(create_info.pNext);
        if ((feedback_struct != nullptr) && (feedback_struct->pipelineStageCreationFeedbackCount != 0 &&
                                             feedback_struct->pipelineStageCreationFeedbackCount != create_info.stageCount)) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pipelineStageCreationFeedbackCount-06594", device,
                             create_info_loc.pNext(Struct::VkPipelineCreationFeedback, Field::pipelineStageCreationFeedbackCount),
                             "(%" PRIu32 ") is different than stageCount(%" PRIu32 ").",
                             feedback_struct->pipelineStageCreationFeedbackCount, create_info.stageCount);
        }

        // helpers for bool used multiple times below
        const bool has_dynamic_viewport = vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_VIEWPORT);
        const bool has_dynamic_scissor = vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_SCISSOR);
        const bool has_dynamic_viewport_w_scaling_nv = vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV);
        const bool has_dynamic_exclusive_scissor_nv = vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV);
        const bool has_dynamic_viewport_with_count = vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
        const bool has_dynamic_scissor_with_count = vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT);

        // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml

        // Collect active stages and other information
        // Only want to loop through pStages once
        uint32_t active_shaders = 0;
        if (create_info.pStages != nullptr) {
            for (uint32_t stage_index = 0; stage_index < create_info.stageCount; ++stage_index) {
                active_shaders |= create_info.pStages[stage_index].stage;
                const Location stage_loc = create_info_loc.dot(Field::pStages, stage_index);

                skip |= context.ValidateStructType(stage_loc, &create_info.pStages[stage_index],
                                                   VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, false, kVUIDUndefined,
                                                   "VUID-VkPipelineShaderStageCreateInfo-sType-sType");

                // special graphics-only generated call
                skip |= ValidatePipelineShaderStageCreateInfo(context, create_info.pStages[stage_index], stage_loc);
                skip |= ValidatePipelineShaderStageCreateInfoCommon(context, create_info.pStages[stage_index], stage_loc);
            }
        }

        if (has_pre_raster_state && (active_shaders & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) &&
            (active_shaders & VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)) {
            if (create_info.pTessellationState) {
                skip |= ValidatePipelineTessellationStateCreateInfo(context, *create_info.pTessellationState,
                                                                    create_info_loc.dot(Field::pTessellationState));

                const bool has_dynamic_patch_control_points =
                    vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT);
                if (!has_dynamic_patch_control_points &&
                    (create_info.pTessellationState->patchControlPoints == 0 ||
                     create_info.pTessellationState->patchControlPoints > device_limits.maxTessellationPatchSize)) {
                    skip |= LogError("VUID-VkPipelineTessellationStateCreateInfo-patchControlPoints-01214", device,
                                     create_info_loc.dot(Field::pTessellationState).dot(Field::patchControlPoints),
                                     "is %" PRIu32 ", but should be between 0 and maxTessellationPatchSize (%" PRIu32 ").",
                                     create_info.pTessellationState->patchControlPoints, device_limits.maxTessellationPatchSize);
                }
            }
        }

        if (!(active_shaders & VK_SHADER_STAGE_MESH_BIT_EXT) && (create_info.pInputAssemblyState != nullptr)) {
            skip |= ValidatePipelineInputAssemblyStateCreateInfo(context, *create_info.pInputAssemblyState,
                                                                 create_info_loc.dot(Field::pInputAssemblyState));
        }

        const bool has_dynamic_vertex_input = vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_VERTEX_INPUT_EXT);
        if (!has_dynamic_vertex_input && !(active_shaders & VK_SHADER_STAGE_MESH_BIT_EXT) &&
            (create_info.pVertexInputState != nullptr)) {
            auto const &vertex_input_state = create_info.pVertexInputState;
            const Location vertex_loc = create_info_loc.dot(Field::pVertexInputState);
            skip |= ValidatePipelineVertexInputStateCreateInfo(context, *vertex_input_state, vertex_loc);

            if (vertex_input_state->vertexBindingDescriptionCount > device_limits.maxVertexInputBindings) {
                skip |= LogError("VUID-VkPipelineVertexInputStateCreateInfo-vertexBindingDescriptionCount-00613", device,
                                 vertex_loc.dot(Field::vertexBindingDescriptionCount),
                                 "(%" PRIu32 ") is larger than maxVertexInputBindings (%" PRIu32 ").",
                                 vertex_input_state->vertexBindingDescriptionCount, device_limits.maxVertexInputBindings);
            }

            if (vertex_input_state->vertexAttributeDescriptionCount > device_limits.maxVertexInputAttributes) {
                skip |= LogError("VUID-VkPipelineVertexInputStateCreateInfo-vertexAttributeDescriptionCount-00614", device,
                                 vertex_loc.dot(Field::vertexAttributeDescriptionCount),
                                 "(%" PRIu32 ") is larger than maxVertexInputAttributes (%" PRIu32 ").",
                                 vertex_input_state->vertexAttributeDescriptionCount, device_limits.maxVertexInputAttributes);
            }

            const bool has_dynamic_binding_stride = vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE);
            vvl::unordered_set<uint32_t> vertex_bindings(vertex_input_state->vertexBindingDescriptionCount);
            for (uint32_t d = 0; d < vertex_input_state->vertexBindingDescriptionCount; ++d) {
                const Location binding_loc = vertex_loc.dot(Field::pVertexBindingDescriptions);
                auto const &vertex_bind_desc = vertex_input_state->pVertexBindingDescriptions[d];
                auto const &binding_it = vertex_bindings.find(vertex_bind_desc.binding);
                if (binding_it != vertex_bindings.cend()) {
                    skip |= LogError("VUID-VkPipelineVertexInputStateCreateInfo-pVertexBindingDescriptions-00616", device,
                                     binding_loc.dot(Field::binding),
                                     "(%" PRIu32 ") is already in pVertexBindingDescription[%" PRIu32 "].",
                                     vertex_bind_desc.binding, *binding_it);
                }
                vertex_bindings.insert(vertex_bind_desc.binding);

                if (vertex_bind_desc.binding >= device_limits.maxVertexInputBindings) {
                    skip |= LogError("VUID-VkVertexInputBindingDescription-binding-00618", device, binding_loc.dot(Field::binding),
                                     "(%" PRIu32
                                     ") is larger than or equal to VkPhysicalDeviceLimits::maxVertexInputBindings (%" PRIu32 ").",
                                     vertex_bind_desc.binding, device_limits.maxVertexInputBindings);
                }

                if (!has_dynamic_binding_stride && vertex_bind_desc.stride > device_limits.maxVertexInputBindingStride) {
                    skip |= LogError("VUID-VkVertexInputBindingDescription-stride-00619", device, binding_loc.dot(Field::stride),
                                     "(%" PRIu32
                                     ") is larger "
                                     "than maxVertexInputBindingStride (%" PRIu32 ").",
                                     vertex_bind_desc.stride, device_limits.maxVertexInputBindingStride);
                }
            }

            vvl::unordered_set<uint32_t> attribute_locations(vertex_input_state->vertexAttributeDescriptionCount);
            for (uint32_t d = 0; d < vertex_input_state->vertexAttributeDescriptionCount; ++d) {
                const Location attribute_loc = vertex_loc.dot(Field::pVertexAttributeDescriptions);
                auto const &vertex_attrib_desc = vertex_input_state->pVertexAttributeDescriptions[d];
                auto const &location_it = attribute_locations.find(vertex_attrib_desc.location);
                if (location_it != attribute_locations.cend()) {
                    skip |= LogError("VUID-VkPipelineVertexInputStateCreateInfo-pVertexAttributeDescriptions-00617", device,
                                     attribute_loc.dot(Field::location),
                                     "(%" PRIu32 ") is already in pVertexAttributeDescriptions[%" PRIu32 "].",
                                     vertex_attrib_desc.location, *location_it);
                }
                attribute_locations.insert(vertex_attrib_desc.location);

                auto const &binding_it = vertex_bindings.find(vertex_attrib_desc.binding);
                if (binding_it == vertex_bindings.cend()) {
                    skip |= LogError("VUID-VkPipelineVertexInputStateCreateInfo-binding-00615", device,
                                     attribute_loc.dot(Field::binding),
                                     "(%" PRIu32 ") does not exist in pVertexBindingDescription.", vertex_attrib_desc.binding);
                }

                if (vertex_attrib_desc.location >= device_limits.maxVertexInputAttributes) {
                    skip |= LogError("VUID-VkVertexInputAttributeDescription-location-00620", device,
                                     attribute_loc.dot(Field::location),
                                     "(%" PRIu32 ") is larger than or equal to maxVertexInputAttributes (%" PRIu32 ").",
                                     vertex_attrib_desc.location, device_limits.maxVertexInputAttributes);
                }

                if (vertex_attrib_desc.binding >= device_limits.maxVertexInputBindings) {
                    skip |=
                        LogError("VUID-VkVertexInputAttributeDescription-binding-00621", device, attribute_loc.dot(Field::binding),
                                 "(%" PRIu32 ") is larger than or equal to maxVertexInputBindings (%" PRIu32 ").",
                                 vertex_attrib_desc.binding, device_limits.maxVertexInputBindings);
                }

                if (vertex_attrib_desc.offset > device_limits.maxVertexInputAttributeOffset) {
                    skip |=
                        LogError("VUID-VkVertexInputAttributeDescription-offset-00622", device, attribute_loc.dot(Field::offset),
                                 "(%" PRIu32 ") is larger than maxVertexInputAttributeOffset (%" PRIu32 ").",
                                 vertex_attrib_desc.offset, device_limits.maxVertexInputAttributeOffset);
                }

                if (vkuFormatIsDepthOrStencil(vertex_attrib_desc.format)) {
                    // Should never hopefully get here, but there are known driver advertising the wrong feature flags
                    // see https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/4849
                    skip |= LogError("UNASSIGNED-VkVertexInputAttributeDescription-depthStencil-format", device,
                                     attribute_loc.dot(Field::format),
                                     "is a depth/stencil format (%s) but depth/stencil formats do not have a defined sizes for "
                                     "alignment, replace with a color format.",
                                     string_VkFormat(vertex_attrib_desc.format));
                }

                VkFormatProperties properties;
                DispatchGetPhysicalDeviceFormatProperties(physical_device, vertex_attrib_desc.format, &properties);
                if ((properties.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT) == 0) {
                    skip |=
                        LogError("VUID-VkVertexInputAttributeDescription-format-00623", device, attribute_loc.dot(Field::format),
                                 "(%s) doesn't support VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT.\n"
                                 "(supported bufferFeatures: %s)",
                                 string_VkFormat(vertex_attrib_desc.format),
                                 string_VkFormatFeatureFlags2(properties.bufferFeatures).c_str());
                }
            }
        }

        // pViewportState, pMultisampleState, pDepthStencilState, and pColorBlendState ignored when rasterization is disabled
        if ((create_info.pRasterizationState != nullptr) &&
            (create_info.pRasterizationState->rasterizerDiscardEnable == VK_FALSE)) {
            // Everything in here has a pre-rasterization shader state
            if (create_info.pViewportState) {
                const auto &viewport_state = *create_info.pViewportState;
                const Location viewport_loc = create_info_loc.dot(Field::pViewportState);
                skip |= ValidatePipelineViewportStateCreateInfo(context, *create_info.pViewportState, viewport_loc);

                const auto *exclusive_scissor_struct =
                    vku::FindStructInPNextChain<VkPipelineViewportExclusiveScissorStateCreateInfoNV>(viewport_state.pNext);
                const auto *shading_rate_image_struct =
                    vku::FindStructInPNextChain<VkPipelineViewportShadingRateImageStateCreateInfoNV>(viewport_state.pNext);
                const auto *coarse_sample_order_struct =
                    vku::FindStructInPNextChain<VkPipelineViewportCoarseSampleOrderStateCreateInfoNV>(viewport_state.pNext);
                const auto *vp_swizzle_struct =
                    vku::FindStructInPNextChain<VkPipelineViewportSwizzleStateCreateInfoNV>(viewport_state.pNext);
                const auto *vp_w_scaling_struct =
                    vku::FindStructInPNextChain<VkPipelineViewportWScalingStateCreateInfoNV>(viewport_state.pNext);
                const auto *depth_clip_control_struct =
                    vku::FindStructInPNextChain<VkPipelineViewportDepthClipControlCreateInfoEXT>(viewport_state.pNext);
                const auto *depth_clamp_control_struct =
                    vku::FindStructInPNextChain<VkPipelineViewportDepthClampControlCreateInfoEXT>(viewport_state.pNext);

                if (!enabled_features.multiViewport) {
                    if (exclusive_scissor_struct && (exclusive_scissor_struct->exclusiveScissorCount != 0 &&
                                                     exclusive_scissor_struct->exclusiveScissorCount != 1)) {
                        skip |=
                            LogError("VUID-VkPipelineViewportExclusiveScissorStateCreateInfoNV-exclusiveScissorCount-02027", device,
                                     viewport_loc.pNext(Struct::VkPipelineViewportExclusiveScissorStateCreateInfoNV,
                                                        Field::exclusiveScissorCount),
                                     "is %" PRIu32 " but multiViewport feature is not enabled.",
                                     exclusive_scissor_struct->exclusiveScissorCount);
                    }

                    if (shading_rate_image_struct &&
                        (shading_rate_image_struct->viewportCount != 0 && shading_rate_image_struct->viewportCount != 1)) {
                        skip |= LogError(
                            "VUID-VkPipelineViewportShadingRateImageStateCreateInfoNV-viewportCount-02054", device,
                            viewport_loc.pNext(Struct::VkPipelineViewportShadingRateImageStateCreateInfoNV, Field::viewportCount),
                            "is %" PRIu32 " but multiViewport feature is not enabled.", shading_rate_image_struct->viewportCount);
                    }
                }

                // Viewport count
                if (viewport_state.viewportCount > device_limits.maxViewports) {
                    skip |=
                        LogError("VUID-VkPipelineViewportStateCreateInfo-viewportCount-01218", device,
                                 viewport_loc.dot(Field::viewportCount), "(%" PRIu32 ") is larger than maxViewports (%" PRIu32 ").",
                                 viewport_state.viewportCount, device_limits.maxViewports);
                }
                if (has_dynamic_viewport_with_count) {
                    if (viewport_state.viewportCount != 0) {
                        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-03379", device,
                                         viewport_loc.dot(Field::viewportCount),
                                         "(%" PRIu32 ") must be zero when VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT is used.",
                                         viewport_state.viewportCount);
                    }
                } else {
                    if (viewport_state.viewportCount == 0) {
                        skip |= LogError("VUID-VkPipelineViewportStateCreateInfo-viewportCount-04135", device,
                                         viewport_loc.dot(Field::viewportCount),
                                         "can't be 0 unless VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT is used.");
                    }

                    if (!enabled_features.multiViewport && (viewport_state.viewportCount > 1)) {
                        skip |= LogError("VUID-VkPipelineViewportStateCreateInfo-viewportCount-01216", device,
                                         viewport_loc.dot(Field::viewportCount),
                                         "is %" PRIu32 " but multiViewport feature is not enabled.", viewport_state.viewportCount);
                    }
                }

                // Scissor count
                if (viewport_state.scissorCount > device_limits.maxViewports) {
                    skip |=
                        LogError("VUID-VkPipelineViewportStateCreateInfo-scissorCount-01219", device,
                                 viewport_loc.dot(Field::scissorCount), "(%" PRIu32 ") is larger than maxViewports (%" PRIu32 ").",
                                 viewport_state.scissorCount, device_limits.maxViewports);
                }
                if (has_dynamic_scissor_with_count) {
                    if (viewport_state.scissorCount != 0) {
                        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-03380", device,
                                         viewport_loc.dot(Field::scissorCount),
                                         "(%" PRIu32 ") must be zero when VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT is used.",
                                         viewport_state.viewportCount);
                    }
                } else {
                    if (viewport_state.scissorCount == 0) {
                        skip |= LogError("VUID-VkPipelineViewportStateCreateInfo-scissorCount-04136", device,
                                         viewport_loc.dot(Field::scissorCount),
                                         "can't be 0 unless VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT is used.");
                    }

                    if (!enabled_features.multiViewport && (viewport_state.scissorCount > 1)) {
                        skip |= LogError("VUID-VkPipelineViewportStateCreateInfo-scissorCount-01217", device,
                                         viewport_loc.dot(Field::scissorCount),
                                         "is %" PRIu32 " but multiViewport feature is not enabled.", viewport_state.scissorCount);
                    }
                }

                if (!has_dynamic_scissor && viewport_state.pScissors) {
                    for (uint32_t scissor_i = 0; scissor_i < viewport_state.scissorCount; ++scissor_i) {
                        const Location &scissor_loc = create_info_loc.dot(Field::pScissors, scissor_i);
                        const auto &scissor = viewport_state.pScissors[scissor_i];

                        if (scissor.offset.x < 0) {
                            skip |= LogError("VUID-VkPipelineViewportStateCreateInfo-x-02821", device,
                                             scissor_loc.dot(Field::offset).dot(Field::x), "(%" PRId32 ") is negative.",
                                             scissor.offset.x);
                        }

                        if (scissor.offset.y < 0) {
                            skip |= LogError("VUID-VkPipelineViewportStateCreateInfo-x-02821", device,
                                             scissor_loc.dot(Field::offset).dot(Field::y), "(%" PRId32 ") is negative.",
                                             scissor.offset.y);
                        }

                        const int64_t x_sum = static_cast<int64_t>(scissor.offset.x) + static_cast<int64_t>(scissor.extent.width);
                        if (x_sum > std::numeric_limits<int32_t>::max()) {
                            skip |= LogError("VUID-VkPipelineViewportStateCreateInfo-offset-02822", device, scissor_loc,
                                             "offset.x (%" PRId32 ") + extent.width (%" PRId32 ") is %" PRIi64
                                             " which will overflow int32_t.",
                                             scissor.offset.x, scissor.extent.width, x_sum);
                        }

                        const int64_t y_sum = static_cast<int64_t>(scissor.offset.y) + static_cast<int64_t>(scissor.extent.height);
                        if (y_sum > std::numeric_limits<int32_t>::max()) {
                            skip |= LogError("VUID-VkPipelineViewportStateCreateInfo-offset-02823", device, scissor_loc,
                                             "offset.y (%" PRId32 ") + extent.height (%" PRId32 ") is %" PRIi64
                                             " which will overflow int32_t.",
                                             scissor.offset.y, scissor.extent.height, y_sum);
                        }
                    }
                }

                if (exclusive_scissor_struct && exclusive_scissor_struct->exclusiveScissorCount > device_limits.maxViewports) {
                    skip |= LogError("VUID-VkPipelineViewportExclusiveScissorStateCreateInfoNV-exclusiveScissorCount-02028", device,
                                     viewport_loc.pNext(Struct::VkPipelineViewportExclusiveScissorStateCreateInfoNV,
                                                        Field::exclusiveScissorCount),
                                     "(%" PRIu32 ") is larger than maxViewports (%" PRIu32 ").",
                                     exclusive_scissor_struct->exclusiveScissorCount, device_limits.maxViewports);
                }

                if (shading_rate_image_struct && shading_rate_image_struct->viewportCount > device_limits.maxViewports) {
                    skip |= LogError(
                        "VUID-VkPipelineViewportShadingRateImageStateCreateInfoNV-viewportCount-02055", device,
                        viewport_loc.pNext(Struct::VkPipelineViewportShadingRateImageStateCreateInfoNV, Field::viewportCount),
                        "(%" PRIu32 ") is larger than maxViewports (%" PRIu32 ").", shading_rate_image_struct->viewportCount,
                        device_limits.maxViewports);
                }

                if (viewport_state.scissorCount != viewport_state.viewportCount) {
                    if (!IsExtEnabled(extensions.vk_ext_extended_dynamic_state) ||
                        (!has_dynamic_viewport_with_count && !has_dynamic_scissor_with_count)) {
                        skip |= LogError("VUID-VkPipelineViewportStateCreateInfo-scissorCount-04134", device, viewport_loc,
                                         "scissorCount (%" PRIu32 ") is different to viewportCount (%" PRIu32 ").",
                                         viewport_state.scissorCount, viewport_state.viewportCount);
                    }
                }

                if (exclusive_scissor_struct && exclusive_scissor_struct->exclusiveScissorCount != 0 &&
                    exclusive_scissor_struct->exclusiveScissorCount != viewport_state.viewportCount) {
                    skip |= LogError("VUID-VkPipelineViewportExclusiveScissorStateCreateInfoNV-exclusiveScissorCount-02029", device,
                                     viewport_loc.pNext(Struct::VkPipelineViewportExclusiveScissorStateCreateInfoNV,
                                                        Field::exclusiveScissorCount),
                                     "is %" PRIu32 " and viewportCount is (%" PRIu32 ").",
                                     exclusive_scissor_struct->exclusiveScissorCount, viewport_state.viewportCount);
                }

                if (shading_rate_image_struct && shading_rate_image_struct->shadingRateImageEnable &&
                    shading_rate_image_struct->viewportCount > viewport_state.viewportCount) {
                    skip |= LogError(
                        "VUID-VkPipelineViewportShadingRateImageStateCreateInfoNV-shadingRateImageEnable-02056", device,
                        viewport_loc.pNext(Struct::VkPipelineViewportShadingRateImageStateCreateInfoNV, Field::viewportCount),
                        "(%" PRIu32 ") is greater than viewportCount (%" PRIu32 ").", shading_rate_image_struct->viewportCount,
                        viewport_state.viewportCount);
                }

                if (!has_dynamic_viewport && !has_dynamic_viewport_with_count && viewport_state.viewportCount > 0 &&
                    viewport_state.pViewports == nullptr) {
                    skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04130", device,
                                     viewport_loc.dot(Field::pViewports), "is NULL, but the viewport state is not dynamic.");
                }

                if (!has_dynamic_scissor && !has_dynamic_scissor_with_count && viewport_state.scissorCount > 0 &&
                    viewport_state.pScissors == nullptr) {
                    skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04131", device,
                                     viewport_loc.dot(Field::pScissors), "is NULL, but the scissor state is not dynamic.");
                }

                if (!has_dynamic_exclusive_scissor_nv && exclusive_scissor_struct &&
                    exclusive_scissor_struct->exclusiveScissorCount > 0 &&
                    exclusive_scissor_struct->pExclusiveScissors == nullptr) {
                    skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04056", device, viewport_loc,
                                     "The exclusive scissor state is static (pCreateInfos[%" PRIu32
                                     "].pDynamicState->pDynamicStates does not contain VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV), but "
                                     "pCreateInfos[%" PRIu32 "] pExclusiveScissors is NULL.",
                                     i, i);
                }

                if (!vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV) &&
                    shading_rate_image_struct && shading_rate_image_struct->viewportCount > 0 &&
                    shading_rate_image_struct->pShadingRatePalettes == nullptr) {
                    skip |= LogError(
                        "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04057", device, viewport_loc,
                        "The shading rate palette state is static (pCreateInfos[%" PRIu32
                        "].pDynamicState->pDynamicStates does not contain VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV), "
                        "but pCreateInfos[%" PRIu32 "] pShadingRatePalettes is NULL.",
                        i, i);
                }

                if (vp_swizzle_struct) {
                    const auto swizzle_viewport_count = vp_swizzle_struct->viewportCount;
                    const auto viewport_count = viewport_state.viewportCount;
                    if (swizzle_viewport_count < viewport_count) {
                        skip |= LogError(
                            "VUID-VkPipelineViewportSwizzleStateCreateInfoNV-viewportCount-01215", device,
                            viewport_loc.pNext(Struct::VkPipelineViewportSwizzleStateCreateInfoNV, Field::viewportCount),
                            "(%" PRIu32 ") less than viewportCount (%" PRIu32 ").", swizzle_viewport_count, viewport_count);
                    }
                }

                // validate the VkViewports
                if (!has_dynamic_viewport && viewport_state.pViewports) {
                    for (uint32_t viewport_i = 0; viewport_i < viewport_state.viewportCount; ++viewport_i) {
                        const auto &viewport = viewport_state.pViewports[viewport_i];  // will crash on invalid ptr
                        skip |= ValidateViewport(viewport, VkCommandBuffer(0), viewport_loc.dot(Field::pViewports, viewport_i));
                    }
                }

                if (coarse_sample_order_struct) {
                    const Location coarse_sample_loc =
                        viewport_loc.pNext(Struct::VkPipelineViewportCoarseSampleOrderStateCreateInfoNV);
                    if (coarse_sample_order_struct->sampleOrderType != VK_COARSE_SAMPLE_ORDER_TYPE_CUSTOM_NV &&
                        coarse_sample_order_struct->customSampleOrderCount != 0) {
                        skip |= LogError("VUID-VkPipelineViewportCoarseSampleOrderStateCreateInfoNV-sampleOrderType-02072", device,
                                         coarse_sample_loc.dot(Field::sampleOrderType),
                                         "is not VK_COARSE_SAMPLE_ORDER_TYPE_CUSTOM_NV and customSampleOrderCount is not 0.");
                    }

                    for (uint32_t order_i = 0; order_i < coarse_sample_order_struct->customSampleOrderCount; ++order_i) {
                        skip |= ValidateCoarseSampleOrderCustomNV(coarse_sample_order_struct->pCustomSampleOrders[order_i],
                                                                  coarse_sample_loc.dot(Field::pCustomSampleOrders, order_i));
                    }
                }

                if (vp_w_scaling_struct && (vp_w_scaling_struct->viewportWScalingEnable == VK_TRUE)) {
                    if (vp_w_scaling_struct->viewportCount != viewport_state.viewportCount) {
                        skip |=
                            LogError("VUID-VkPipelineViewportStateCreateInfo-viewportWScalingEnable-01726", device,
                                     viewport_loc.pNext(Struct::VkPipelineViewportWScalingStateCreateInfoNV, Field::viewportCount),
                                     "(%" PRIu32 ") is not equal to viewportCount (%" PRIu32 ").",
                                     vp_w_scaling_struct->viewportCount, viewport_state.viewportCount);
                    }
                    if (!has_dynamic_viewport_w_scaling_nv && !vp_w_scaling_struct->pViewportWScalings) {
                        skip |= LogError(
                            "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-01715", device,
                            viewport_loc.pNext(Struct::VkPipelineViewportWScalingStateCreateInfoNV, Field::pViewportWScalings),
                            "is NULL.");
                    }
                }

                const bool has_dynamic_depth_clamp_range = vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_DEPTH_CLAMP_RANGE_EXT);
                if (!has_dynamic_depth_clamp_range && depth_clamp_control_struct &&
                    depth_clamp_control_struct->depthClampMode == VK_DEPTH_CLAMP_MODE_USER_DEFINED_RANGE_EXT) {
                    if (!depth_clamp_control_struct->pDepthClampRange) {
                        skip |= LogError(
                            "VUID-VkPipelineViewportDepthClampControlCreateInfoEXT-pDepthClampRange-09646", device,
                            viewport_loc.pNext(Struct::VkPipelineViewportDepthClampControlCreateInfoEXT, Field::pDepthClampRange),
                            "is NULL.");
                    } else {
                        skip |= ValidateDepthClampRange(
                            *depth_clamp_control_struct->pDepthClampRange,
                            viewport_loc.pNext(Struct::VkPipelineViewportDepthClampControlCreateInfoEXT, Field::pDepthClampRange));
                    }
                }

                if (depth_clip_control_struct) {
                    if (depth_clip_control_struct->negativeOneToOne && !enabled_features.depthClipControl) {
                        skip |= LogError(
                            "VUID-VkPipelineViewportDepthClipControlCreateInfoEXT-negativeOneToOne-06470", device,
                            viewport_loc.pNext(Struct::VkPhysicalDeviceDepthClipControlFeaturesEXT, Field::negativeOneToOne),
                            "is VK_TRUE but the depthClipControl feature was not enabled.");
                    }
                }
            } else if ((!has_dynamic_viewport_with_count || !has_dynamic_scissor_with_count ||
                        !IsExtEnabled(extensions.vk_ext_extended_dynamic_state3)) &&
                       (vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE) ||
                        (!vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE) &&
                         !create_info.pRasterizationState->rasterizerDiscardEnable))) {
                skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-rasterizerDiscardEnable-09024", device, create_info_loc,
                                 "Rasterization is enabled (pCreateInfos[%" PRIu32
                                 "].pRasterizationState->rasterizerDiscardEnable is VK_FALSE), but pCreateInfos[%" PRIu32
                                 "].pViewportState is NULL.",
                                 i, i);
            }

            // It is possible for pCreateInfos[i].pMultisampleState to be null when creating a graphics library
            if (create_info.pMultisampleState) {
                const Location ms_loc = create_info_loc.dot(Field::pMultisampleState);
                skip |= ValidatePipelineMultisampleStateCreateInfo(context, *create_info.pMultisampleState, ms_loc);

                if (create_info.pMultisampleState->sampleShadingEnable == VK_TRUE) {
                    if (!enabled_features.sampleRateShading) {
                        skip |= LogError("VUID-VkPipelineMultisampleStateCreateInfo-sampleShadingEnable-00784", device,
                                         ms_loc.dot(Field::sampleShadingEnable),
                                         "is VK_TRUE but the sampleRateShading feature was not enabled.");
                    }
                    // TODO Add documentation issue about when minSampleShading must be in range and when it is ignored
                    // For now a "least noise" test *only* when sampleShadingEnable is VK_TRUE.
                    if (!IsBetweenInclusive(create_info.pMultisampleState->minSampleShading, 0.F, 1.0F)) {
                        skip |= LogError("VUID-VkPipelineMultisampleStateCreateInfo-minSampleShading-00786", device,
                                         ms_loc.dot(Field::minSampleShading), "is %f.",
                                         create_info.pMultisampleState->minSampleShading);
                    }
                }
            }

            bool uses_color_attachment = false;
            bool uses_depthstencil_attachment = false;
            {
                std::unique_lock<std::mutex> lock(renderpass_map_mutex);
                const auto subpasses_uses_it = renderpasses_states.find(create_info.renderPass);
                if (subpasses_uses_it != renderpasses_states.end()) {
                    const auto &subpasses_uses = subpasses_uses_it->second;
                    if (subpasses_uses.subpasses_using_color_attachment.count(create_info.subpass)) {
                        uses_color_attachment = true;
                    }
                    if (subpasses_uses.subpasses_using_depthstencil_attachment.count(create_info.subpass)) {
                        uses_depthstencil_attachment = true;
                    }
                }
                lock.unlock();
            }

            if (create_info.pDepthStencilState != nullptr && uses_depthstencil_attachment) {
                const Location ds_loc = create_info_loc.dot(Field::pDepthStencilState);
                auto const &ds_state = *create_info.pDepthStencilState;
                skip |= ValidatePipelineDepthStencilStateCreateInfo(context, ds_state, ds_loc);
            }

            if (create_info.pColorBlendState != nullptr && uses_color_attachment) {
                const Location color_loc = create_info_loc.dot(Field::pColorBlendState);
                auto const &color_blend_state = *create_info.pColorBlendState;
                skip |= ValidatePipelineColorBlendStateCreateInfo(context, color_blend_state, color_loc);

                // If logicOpEnable is VK_TRUE, logicOp must be a valid VkLogicOp value
                if (color_blend_state.logicOpEnable == VK_TRUE) {
                    skip |=
                        context.ValidateRangedEnum(color_loc.dot(Field::logicOp), vvl::Enum::VkLogicOp, color_blend_state.logicOp,
                                                   "VUID-VkPipelineColorBlendStateCreateInfo-logicOpEnable-00607");
                }

                const bool dynamic_not_set = (!vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT) ||
                                              !vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT) ||
                                              !vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT) ||
                                              !vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT));

                // If any of the dynamic states are not set still need a valid array
                if ((color_blend_state.attachmentCount > 0) && dynamic_not_set) {
                    skip |= context.ValidateArray(color_loc.dot(Field::attachmentCount), color_loc.dot(Field::pAttachments),
                                                  color_blend_state.attachmentCount, &color_blend_state.pAttachments, false, true,
                                                  kVUIDUndefined, "VUID-VkPipelineColorBlendStateCreateInfo-pAttachments-07353");
                }

                auto color_write = vku::FindStructInPNextChain<VkPipelineColorWriteCreateInfoEXT>(color_blend_state.pNext);
                if (color_write && (color_write->attachmentCount != color_blend_state.attachmentCount) && dynamic_not_set) {
                    skip |= LogError("VUID-VkPipelineColorWriteCreateInfoEXT-attachmentCount-07608", device,
                                     color_loc.pNext(Struct::VkPipelineColorWriteCreateInfoEXT, Field::attachmentCount),
                                     "(%" PRIu32 ") is different than pColorBlendState.attachmentCount (%" PRIu32 ").",
                                     color_write->attachmentCount, color_blend_state.attachmentCount);
                }
            }
        }

        if (create_info.pRasterizationState) {
            const Location rasterization_loc = create_info_loc.dot(Field::pRasterizationState);
            skip |= ValidatePipelineRasterizationStateCreateInfo(context, *create_info.pRasterizationState, rasterization_loc);

            if (!IsExtEnabled(extensions.vk_nv_fill_rectangle)) {
                if (create_info.pRasterizationState->polygonMode == VK_POLYGON_MODE_FILL_RECTANGLE_NV) {
                    skip |= LogError("VUID-VkPipelineRasterizationStateCreateInfo-polygonMode-01414", device,
                                     rasterization_loc.dot(Field::polygonMode),
                                     "is VK_POLYGON_MODE_FILL_RECTANGLE_NV, but "
                                     "the extension VK_NV_fill_rectangle is not enabled.");
                }
            }

            if ((create_info.pRasterizationState->polygonMode != VK_POLYGON_MODE_FILL) &&
                (create_info.pRasterizationState->polygonMode != VK_POLYGON_MODE_FILL_RECTANGLE_NV) &&
                (!enabled_features.fillModeNonSolid)) {
                skip |= LogError("VUID-VkPipelineRasterizationStateCreateInfo-polygonMode-01507", device,
                                 rasterization_loc.dot(Field::polygonMode), "is %s, but fillModeNonSolid feature is not enabled.",
                                 string_VkPolygonMode(create_info.pRasterizationState->polygonMode));
            }

            if (!vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_LINE_WIDTH) && !enabled_features.wideLines &&
                (create_info.pRasterizationState->lineWidth != 1.0f)) {
                skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-00749", device,
                                 rasterization_loc.dot(Field::lineWidth),
                                 "is %f, but the line width state is static (pCreateInfos[%" PRIu32
                                 "].pDynamicState->pDynamicStates does not contain VK_DYNAMIC_STATE_LINE_WIDTH) and "
                                 "wideLines feature was not enabled.",
                                 create_info.pRasterizationState->lineWidth, i);
            }

            const auto *line_state =
                vku::FindStructInPNextChain<VkPipelineRasterizationLineStateCreateInfo>(create_info.pRasterizationState->pNext);
            const bool dynamic_line_raster_mode = vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT);
            const bool dynamic_line_stipple_enable = vvl::Contains(dynamic_state_map, VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT);
            if (line_state) {
                if (line_state->stippledLineEnable && !dynamic_line_stipple_enable) {
                    if (line_state->lineStippleFactor < 1 || line_state->lineStippleFactor > 256) {
                        skip |= LogError(
                            "VUID-VkGraphicsPipelineCreateInfo-stippledLineEnable-02767", device,
                            rasterization_loc.pNext(Struct::VkPipelineRasterizationLineStateCreateInfo, Field::lineStippleFactor),
                            "is %" PRIu32 ".", line_state->lineStippleFactor);
                    }
                }

                if (!dynamic_line_raster_mode) {
                    if (create_info.pMultisampleState &&
                        (line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_BRESENHAM ||
                         line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH)) {
                        if (create_info.pMultisampleState->alphaToCoverageEnable) {
                            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-lineRasterizationMode-02766", device,
                                             rasterization_loc.pNext(Struct::VkPipelineRasterizationLineStateCreateInfo,
                                                                     Field::lineStippleFactor),
                                             "is %s, but pCreateInfos[%" PRIu32
                                             "].pMultisampleState->alphaToCoverageEnable is VK_TRUE.",
                                             string_VkLineRasterizationMode(line_state->lineRasterizationMode), i);
                        }
                        if (create_info.pMultisampleState->alphaToOneEnable) {
                            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-lineRasterizationMode-02766", device,
                                             rasterization_loc.pNext(Struct::VkPipelineRasterizationLineStateCreateInfo,
                                                                     Field::lineStippleFactor),
                                             "is %s, but pCreateInfos[%" PRIu32 "].pMultisampleState->alphaToOneEnable is VK_TRUE.",
                                             string_VkLineRasterizationMode(line_state->lineRasterizationMode), i);
                        }
                        if (create_info.pMultisampleState->sampleShadingEnable) {
                            skip |=
                                LogError("VUID-VkGraphicsPipelineCreateInfo-lineRasterizationMode-02766", device,
                                         rasterization_loc.pNext(Struct::VkPipelineRasterizationLineStateCreateInfo,
                                                                 Field::lineStippleFactor),
                                         "is %s, but pCreateInfos[%" PRIu32 "].pMultisampleState->sampleShadingEnable is VK_TRUE.",
                                         string_VkLineRasterizationMode(line_state->lineRasterizationMode), i);
                        }
                    }

                    if (line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR &&
                        (!enabled_features.rectangularLines)) {
                        skip |=
                            LogError("VUID-VkPipelineRasterizationLineStateCreateInfo-lineRasterizationMode-02768", device,
                                     rasterization_loc.pNext(Struct::VkPhysicalDeviceLineRasterizationFeatures,
                                                             Field::lineRasterizationMode),
                                     "is VK_LINE_RASTERIZATION_MODE_RECTANGULAR but the rectangularLines feature was not enabled.");
                    }
                    if (line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_BRESENHAM &&
                        (!enabled_features.bresenhamLines)) {
                        skip |= LogError("VUID-VkPipelineRasterizationLineStateCreateInfo-lineRasterizationMode-02769", device,
                                         rasterization_loc.pNext(Struct::VkPhysicalDeviceLineRasterizationFeatures,
                                                                 Field::lineRasterizationMode),
                                         "is VK_LINE_RASTERIZATION_MODE_BRESENHAM but the bresenhamLines feature was not enabled.");
                    }
                    if (line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH &&
                        (!enabled_features.smoothLines)) {
                        skip |= LogError("VUID-VkPipelineRasterizationLineStateCreateInfo-lineRasterizationMode-02770", device,
                                         rasterization_loc.pNext(Struct::VkPhysicalDeviceLineRasterizationFeatures,
                                                                 Field::lineRasterizationMode),
                                         "is VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH but the smoothLines feature was not "
                                         "enabled.");
                    }
                    if (line_state->stippledLineEnable && !dynamic_line_stipple_enable) {
                        if (line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR &&
                            (!enabled_features.stippledRectangularLines)) {
                            skip |= LogError("VUID-VkPipelineRasterizationLineStateCreateInfo-stippledLineEnable-02771", device,
                                             rasterization_loc.pNext(Struct::VkPhysicalDeviceLineRasterizationFeatures,
                                                                     Field::lineRasterizationMode),
                                             "is VK_LINE_RASTERIZATION_MODE_RECTANGULAR and stippledLineEnable was VK_TRUE, "
                                             "but the stippledRectangularLines feature was not enabled.");
                        }
                        if (line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_BRESENHAM &&
                            (!enabled_features.stippledBresenhamLines)) {
                            skip |= LogError("VUID-VkPipelineRasterizationLineStateCreateInfo-stippledLineEnable-02772", device,
                                             rasterization_loc.pNext(Struct::VkPhysicalDeviceLineRasterizationFeatures,
                                                                     Field::lineRasterizationMode),
                                             "is VK_LINE_RASTERIZATION_MODE_BRESENHAM and stippledLineEnable was VK_TRUE, but "
                                             "the stippledBresenhamLines feature was not enabled.");
                        }
                        if (line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH &&
                            (!enabled_features.stippledSmoothLines)) {
                            skip |= LogError("VUID-VkPipelineRasterizationLineStateCreateInfo-stippledLineEnable-02773", device,
                                             rasterization_loc.pNext(Struct::VkPhysicalDeviceLineRasterizationFeatures,
                                                                     Field::lineRasterizationMode),
                                             "is VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH and stippledLineEnable was "
                                             "VK_TRUE, but the stippledSmoothLines feature was not enabled.");
                        }
                        if (line_state->lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_DEFAULT &&
                            (!enabled_features.stippledRectangularLines || !device_limits.strictLines)) {
                            skip |= LogError("VUID-VkPipelineRasterizationLineStateCreateInfo-stippledLineEnable-02774", device,
                                             rasterization_loc.pNext(Struct::VkPhysicalDeviceLineRasterizationFeatures,
                                                                     Field::lineRasterizationMode),
                                             "is VK_LINE_RASTERIZATION_MODE_DEFAULT and stippledLineEnable was VK_TRUE, but "
                                             "the stippledRectangularLines feature was not enabled.");
                        }
                    }
                }
            }
        }

        if (flags & VK_PIPELINE_CREATE_DERIVATIVE_BIT) {
            if (create_info.basePipelineHandle != VK_NULL_HANDLE) {
                if (create_info.basePipelineIndex != -1) {
                    skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-flags-07986", device,
                                     create_info_loc.dot(Field::basePipelineIndex),
                                     "%" PRIu32 " and basePipelineHandle is not VK_NULL_HANDLE.", create_info.basePipelineIndex);
                }
            } else if (static_cast<uint32_t>(create_info.basePipelineIndex) >= createInfoCount) {
                skip |=
                    LogError("VUID-VkGraphicsPipelineCreateInfo-flags-07985", device, create_info_loc.dot(Field::basePipelineIndex),
                             "(%" PRIu32 ") is greater than or equal to createInfoCount %" PRIu32 ".",
                             create_info.basePipelineIndex, createInfoCount);
            }
        }

        skip |= ValidatePipelineBinaryInfo(create_info.pNext, create_info.flags, pipelineCache, create_info_loc);
    }

    return skip;
}

bool Device::ValidateCreateComputePipelinesFlags(const VkPipelineCreateFlags2KHR flags, const Location &flags_loc) const {
    bool skip = false;
    if ((flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) != 0) {
        if (!enabled_features.shaderEnqueue) {
            skip |=
                LogError("VUID-VkComputePipelineCreateInfo-shaderEnqueue-09177", device, flags_loc,
                         "%s must not include VK_PIPELINE_CREATE_LIBRARY_BIT_KHR.", string_VkPipelineCreateFlags2(flags).c_str());
        }
    }
    if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR) != 0) {
        skip |= LogError("VUID-VkComputePipelineCreateInfo-flags-03365", device, flags_loc,
                         "(%s) must not include VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR.",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }
    if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR) != 0) {
        skip |= LogError("VUID-VkComputePipelineCreateInfo-flags-03366", device, flags_loc,
                         "(%s) must not include VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR.",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }
    if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_MISS_SHADERS_BIT_KHR) != 0) {
        skip |= LogError("VUID-VkComputePipelineCreateInfo-flags-03367", device, flags_loc,
                         "(%s) must not include VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_MISS_SHADERS_BIT_KHR.",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }
    if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR) != 0) {
        skip |= LogError("VUID-VkComputePipelineCreateInfo-flags-03368", device, flags_loc,
                         "(%s) must not include VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR.",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }
    if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR) != 0) {
        skip |= LogError("VUID-VkComputePipelineCreateInfo-flags-03369", device, flags_loc,
                         "(%s) must not include VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR.",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }
    if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR) != 0) {
        skip |= LogError("VUID-VkComputePipelineCreateInfo-flags-03370", device, flags_loc,
                         "(%s) must not include VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR.",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }
    if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR) != 0) {
        skip |= LogError("VUID-VkComputePipelineCreateInfo-flags-03576", device, flags_loc,
                         "(%s) must not include VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR.",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }
    if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_ALLOW_MOTION_BIT_NV) != 0) {
        skip |= LogError("VUID-VkComputePipelineCreateInfo-flags-04945", device, flags_loc,
                         "(%s) must not include VK_PIPELINE_CREATE_RAY_TRACING_ALLOW_MOTION_BIT_NV.",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }
    if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_OPACITY_MICROMAP_BIT_EXT) != 0) {
        skip |= LogError("VUID-VkComputePipelineCreateInfo-flags-07367", device, flags_loc,
                         "(%s) must not include VK_PIPELINE_CREATE_RAY_TRACING_OPACITY_MICROMAP_BIT_EXT.",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }
    if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_DISPLACEMENT_MICROMAP_BIT_NV) != 0) {
        skip |= LogError("VUID-VkComputePipelineCreateInfo-flags-07996", device, flags_loc,
                         "(%s) must not include VK_PIPELINE_CREATE_RAY_TRACING_DISPLACEMENT_MICROMAP_BIT_NV.",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }
    if ((flags & VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT) != 0) {
        if (!enabled_features.deviceGeneratedCommands) {
            skip |= LogError("VUID-VkComputePipelineCreateInfo-flags-11007", device, flags_loc,
                             "(%s) contains VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT, but "
                             "VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT::deviceGeneratedCommands is not enabled.",
                             string_VkPipelineCreateFlags2(flags).c_str());
        }
    }
    return skip;
}

bool Device::manual_PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                          const VkComputePipelineCreateInfo *pCreateInfos,
                                                          const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                          const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    for (uint32_t i = 0; i < createInfoCount; i++) {
        const Location create_info_loc = error_obj.location.dot(Field::pCreateInfos, i);
        const VkComputePipelineCreateInfo &create_info = pCreateInfos[i];

        skip |= context.ValidateString(create_info_loc.dot(Field::stage).dot(Field::pName),
                                       "VUID-VkPipelineShaderStageCreateInfo-pName-parameter", create_info.stage.pName);
        auto feedback_struct = vku::FindStructInPNextChain<VkPipelineCreationFeedbackCreateInfo>(create_info.pNext);
        if (feedback_struct) {
            const uint32_t feedback_count = feedback_struct->pipelineStageCreationFeedbackCount;
            if ((feedback_count != 0) && (feedback_count != 1)) {
                skip |= LogError(
                    "VUID-VkComputePipelineCreateInfo-pipelineStageCreationFeedbackCount-06566", device,
                    create_info_loc.pNext(Struct::VkPipelineCreationFeedbackCreateInfo, Field::pipelineStageCreationFeedbackCount),
                    "is %" PRIu32 ".", feedback_count);
            }
        }

        const auto *create_flags_2 = vku::FindStructInPNextChain<VkPipelineCreateFlags2CreateInfo>(create_info.pNext);
        const VkPipelineCreateFlags2KHR flags =
            create_flags_2 ? create_flags_2->flags : static_cast<VkPipelineCreateFlags2KHR>(create_info.flags);
        const Location flags_loc = create_flags_2 ? create_info_loc.pNext(Struct::VkPipelineCreateFlags2CreateInfo, Field::flags)
                                                  : create_info_loc.dot(Field::flags);
        if (!create_flags_2) {
            skip |= context.ValidateFlags(flags_loc, vvl::FlagBitmask::VkPipelineCreateFlagBits, AllVkPipelineCreateFlagBits,
                                          create_info.flags, kOptionalFlags, "VUID-VkComputePipelineCreateInfo-None-09497");
        }
        skip |= ValidateCreateComputePipelinesFlags(flags, flags_loc);

        // Make sure compute stage is selected
        if (create_info.stage.stage != VK_SHADER_STAGE_COMPUTE_BIT) {
            skip |= LogError("VUID-VkComputePipelineCreateInfo-stage-00701", device,
                             create_info_loc.dot(Field::stage).dot(Field::stage), "is %s.",
                             string_VkShaderStageFlagBits(create_info.stage.stage));
        }

        if (flags & VK_PIPELINE_CREATE_DERIVATIVE_BIT) {
            if (create_info.basePipelineHandle != VK_NULL_HANDLE) {
                if (create_info.basePipelineIndex != -1) {
                    skip |= LogError("VUID-VkComputePipelineCreateInfo-flags-07986", device,
                                     create_info_loc.dot(Field::basePipelineIndex),
                                     "(%" PRIu32 ") and basePipelineHandle is not VK_NULL_HANDLE.", create_info.basePipelineIndex);
                }
            } else {
                if (static_cast<uint32_t>(create_info.basePipelineIndex) >= createInfoCount) {
                    skip |= LogError("VUID-VkComputePipelineCreateInfo-flags-07985", device,
                                     create_info_loc.dot(Field::basePipelineIndex),
                                     "(%" PRIu32 ") is greater than or equal to createInfoCount %" PRIu32 ".",
                                     create_info.basePipelineIndex, createInfoCount);
                }
            }
        }

        skip |= ValidatePipelineShaderStageCreateInfoCommon(context, create_info.stage, create_info_loc.dot(Field::stage));

        skip |= ValidatePipelineBinaryInfo(create_info.pNext, create_info.flags, pipelineCache, create_info_loc);
    }
    return skip;
}

bool Device::ValidateDepthClampRange(const VkDepthClampRangeEXT &depth_clamp_range, const Location &loc) const {
    bool skip = false;
    if (depth_clamp_range.minDepthClamp > depth_clamp_range.maxDepthClamp) {
        skip |=
            LogError("VUID-VkDepthClampRangeEXT-pDepthClampRange-00999", device, loc.dot(Field::minDepthClamp),
                     "(%f) is greater than maxDepthClamp (%f).", depth_clamp_range.minDepthClamp, depth_clamp_range.maxDepthClamp);
    }

    if (!IsExtEnabled(extensions.vk_ext_depth_range_unrestricted)) {
        if (depth_clamp_range.minDepthClamp < 0.0) {
            skip |= LogError("VUID-VkDepthClampRangeEXT-pDepthClampRange-09648", device, loc.dot(Field::minDepth),
                             "(%f) is below 0.0 (and VK_EXT_depth_range_unrestricted is not enabled).",
                             depth_clamp_range.minDepthClamp);
        }
        if (depth_clamp_range.maxDepthClamp > 1.0) {
            skip |= LogError("VUID-VkDepthClampRangeEXT-pDepthClampRange-09649", device, loc.dot(Field::maxDepthClamp),
                             "(%f)  is above 1.0 (and VK_EXT_depth_range_unrestricted is not enabled).",
                             depth_clamp_range.maxDepthClamp);
        }
    }
    return skip;
}

bool Device::manual_PreCallValidateCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo *pCreateInfo,
                                                       const VkAllocationCallbacks *pAllocator, VkPipelineCache *pPipelineCache,
                                                       const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    const bool has_externally_sync = (pCreateInfo->flags & VK_PIPELINE_CACHE_CREATE_EXTERNALLY_SYNCHRONIZED_BIT) != 0;
    const bool has_internally_sync = (pCreateInfo->flags & VK_PIPELINE_CACHE_CREATE_INTERNALLY_SYNCHRONIZED_MERGE_BIT_KHR) != 0;

    if (!enabled_features.pipelineCreationCacheControl && has_externally_sync) {
        skip |= LogError("VUID-VkPipelineCacheCreateInfo-pipelineCreationCacheControl-02892", device,
                         error_obj.location.dot(Field::pCreateInfo).dot(Field::flags),
                         "includes VK_PIPELINE_CACHE_CREATE_EXTERNALLY_SYNCHRONIZED_BIT, but pipelineCreationCacheControl "
                         "feature was not enabled");
    }

    if (!enabled_features.maintenance8 && has_internally_sync) {
        skip |= LogError(
            "VUID-VkPipelineCacheCreateInfo-maintenance8-10200", device,
            error_obj.location.dot(Field::pCreateInfo).dot(Field::flags),
            "includes VK_PIPELINE_CACHE_CREATE_INTERNALLY_SYNCHRONIZED_MERGE_BIT_KHR, but maintenance8 feature was not enabled.");
    }
    if (has_externally_sync && has_internally_sync) {
        skip |= LogError("VUID-VkPipelineCacheCreateInfo-flags-10201", device,
                         error_obj.location.dot(Field::pCreateInfo).dot(Field::flags),
                         "includes both VK_PIPELINE_CACHE_CREATE_INTERNALLY_SYNCHRONIZED_MERGE_BIT_KHR and "
                         "VK_PIPELINE_CACHE_CREATE_EXTERNALLY_SYNCHRONIZED_BIT_EXT");
    }
    return skip;
}

bool Device::manual_PreCallValidateMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount,
                                                       const VkPipelineCache *pSrcCaches, const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    if (pSrcCaches) {
        for (uint32_t index0 = 0; index0 < srcCacheCount; ++index0) {
            if (pSrcCaches[index0] == dstCache) {
                skip |= LogError("VUID-vkMergePipelineCaches-dstCache-00770", device, error_obj.location.dot(Field::dstCache),
                                 "%s is in pSrcCaches list.", FormatHandle(dstCache).c_str());
                break;
            }
        }
    }
    return skip;
}

bool Device::manual_PreCallValidateGetPipelinePropertiesEXT(VkDevice device, const VkPipelineInfoEXT *pPipelineInfo,
                                                            VkBaseOutStructure *pPipelineProperties, const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (!enabled_features.pipelinePropertiesIdentifier) {
        skip |= LogError("VUID-vkGetPipelinePropertiesEXT-None-06766", device, error_obj.location,
                         "the pipelinePropertiesIdentifier feature was not enabled.");
    }

    const Location &pipeline_properties_loc = error_obj.location.dot(Field::pPipelineProperties);
    if (pPipelineProperties) {
        if (pPipelineProperties->sType != VK_STRUCTURE_TYPE_PIPELINE_PROPERTIES_IDENTIFIER_EXT) {
            skip |=
                LogError("VUID-VkPipelinePropertiesIdentifierEXT-sType-sType", device, pipeline_properties_loc.dot(Field::sType),
                         "is not VK_STRUCTURE_TYPE_PIPELINE_PROPERTIES_IDENTIFIER_EXT.");
        }
        if (pPipelineProperties->pNext != nullptr) {
            skip |= LogError("VUID-VkPipelinePropertiesIdentifierEXT-pNext-pNext", device,
                             pipeline_properties_loc.dot(Field::pNext), "is not NULL.");
        }
    } else {
        skip |= LogError("VUID-vkGetPipelinePropertiesEXT-pPipelineProperties-06739", device, pipeline_properties_loc, "is NULL.");
    }
    return skip;
}
}  // namespace stateless
