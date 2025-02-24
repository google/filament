/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
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

#include <assert.h>
#include <string>
#include <sstream>

#include <vulkan/vk_enum_string_helper.h>
#include "state_tracker/image_state.h"
#include "state_tracker/buffer_state.h"
#include "state_tracker/render_pass_state.h"
#include "state_tracker/shader_module.h"
#include "core_validation.h"
#include "generated/enum_flag_bits.h"

bool CoreChecks::ReportInvalidCommandBuffer(const vvl::CommandBuffer &cb_state, const Location &loc, const char *vuid) const {
    bool skip = false;
    for (const auto &entry : cb_state.broken_bindings) {
        const auto &obj = entry.first;
        const char *cause_str = (obj.type == kVulkanObjectTypeDescriptorSet)   ? " or updated"
                                : (obj.type == kVulkanObjectTypeCommandBuffer) ? " or rerecorded"
                                                                               : "";
        auto objlist = entry.second;  // intentional copy
        objlist.add(cb_state.Handle());
        skip |= LogError(vuid, objlist, loc, "was called in %s which is invalid because bound %s was destroyed%s.",
                         FormatHandle(cb_state).c_str(), FormatHandle(obj).c_str(), cause_str);
    }
    return skip;
}

bool CoreChecks::PreCallValidateFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                                   const VkCommandBuffer *pCommandBuffers, const ErrorObject &error_obj) const {
    bool skip = false;
    for (uint32_t i = 0; i < commandBufferCount; i++) {
        auto cb_state = GetRead<vvl::CommandBuffer>(pCommandBuffers[i]);
        // Delete CB information structure, and remove from commandBufferMap
        if (cb_state && cb_state->InUse()) {
            const LogObjectList objlist(pCommandBuffers[i], commandPool);
            skip |= LogError("VUID-vkFreeCommandBuffers-pCommandBuffers-00047", objlist,
                             error_obj.location.dot(Field::pCommandBuffers, i), "(%s) is in use.",
                             FormatHandle(pCommandBuffers[i]).c_str());
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo *pBeginInfo,
                                                   const ErrorObject &error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) return false;
    bool skip = false;
    if (cb_state->InUse()) {
        skip |= LogError("VUID-vkBeginCommandBuffer-commandBuffer-00049", commandBuffer, error_obj.location,
                         "on active %s before it has completed. You must check "
                         "command buffer fence before this call.",
                         FormatHandle(commandBuffer).c_str());
    }
    const Location begin_info_loc = error_obj.location.dot(Field::pBeginInfo);
    if (cb_state->IsPrimary()) {
        const VkCommandBufferUsageFlags invalid_usage =
            (VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
        if ((pBeginInfo->flags & invalid_usage) == invalid_usage) {
            skip |= LogError("VUID-vkBeginCommandBuffer-commandBuffer-02840", commandBuffer, begin_info_loc.dot(Field::flags),
                             "is %s for Primary %s (can't have both ONE_TIME_SUBMIT and SIMULTANEOUS_USE).",
                             string_VkCommandBufferUsageFlags(pBeginInfo->flags).c_str(), FormatHandle(commandBuffer).c_str());
        }
    } else {
        const VkCommandBufferInheritanceInfo *info = pBeginInfo->pInheritanceInfo;
        const Location inheritance_loc = begin_info_loc.dot(Field::pInheritanceInfo);
        if (!info) {
            skip |= LogError("VUID-vkBeginCommandBuffer-commandBuffer-00051", commandBuffer, inheritance_loc,
                             "is null for Secondary %s.", FormatHandle(commandBuffer).c_str());
        } else {
            auto p_inherited_rendering_info = vku::FindStructInPNextChain<VkCommandBufferInheritanceRenderingInfo>(info->pNext);

            if (pBeginInfo->flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT) {
                if (info->renderPass != VK_NULL_HANDLE) {
                    if (auto framebuffer = Get<vvl::Framebuffer>(info->framebuffer)) {
                        if (framebuffer->create_info.renderPass != info->renderPass) {
                            if (auto render_pass = Get<vvl::RenderPass>(info->renderPass)) {
                                // renderPass that framebuffer was created with must be compatible with local renderPass
                                skip |= ValidateRenderPassCompatibility(framebuffer->Handle(), *framebuffer->rp_state.get(),
                                                                        cb_state->Handle(), *render_pass.get(), inheritance_loc,
                                                                        "VUID-VkCommandBufferBeginInfo-flags-00055");
                            }
                        }
                    }

                    auto render_pass = Get<vvl::RenderPass>(info->renderPass);
                    if (!render_pass) {
                        skip |= LogError("VUID-VkCommandBufferBeginInfo-flags-06000", commandBuffer,
                                         inheritance_loc.dot(Field::renderPass), "is not a valid VkRenderPass.");
                    } else {
                        if (info->subpass >= render_pass->create_info.subpassCount) {
                            skip |= LogError("VUID-VkCommandBufferBeginInfo-flags-06001", commandBuffer,
                                             inheritance_loc.dot(Field::subpass),
                                             "(%" PRIu32 ") is not valid, renderPass was created with subpassCount %" PRIu32 ".",
                                             info->subpass, render_pass->create_info.subpassCount);
                        }
                    }
                } else {
                    if (!p_inherited_rendering_info) {
                        skip |=
                            LogError("VUID-VkCommandBufferBeginInfo-flags-06002", commandBuffer, inheritance_loc.dot(Field::pNext),
                                     "chain must include a VkCommandBufferInheritanceRenderingInfo structure.");
                    }
                }
            }

            if (p_inherited_rendering_info) {
                auto p_attachment_sample_count_info_amd = vku::FindStructInPNextChain<VkAttachmentSampleCountInfoAMD>(info->pNext);
                if (p_attachment_sample_count_info_amd &&
                    p_attachment_sample_count_info_amd->colorAttachmentCount != p_inherited_rendering_info->colorAttachmentCount) {
                    skip |= LogError(
                        "VUID-VkCommandBufferBeginInfo-flags-06003", commandBuffer,
                        inheritance_loc.pNext(Struct::VkAttachmentSampleCountInfoAMD, Field::colorAttachmentCount),
                        "(%" PRIu32 ") must equal VkCommandBufferInheritanceRenderingInfo::colorAttachmentCount (%" PRIu32 ").",
                        p_attachment_sample_count_info_amd->colorAttachmentCount, p_inherited_rendering_info->colorAttachmentCount);
                }

                if ((p_inherited_rendering_info->colorAttachmentCount != 0) &&
                    (p_inherited_rendering_info->rasterizationSamples & AllVkSampleCountFlagBits) == 0) {
                    skip |= LogError(
                        "VUID-VkCommandBufferInheritanceRenderingInfo-colorAttachmentCount-06004", commandBuffer,
                        inheritance_loc.pNext(Struct::VkCommandBufferInheritanceRenderingInfo, Field::colorAttachmentCount),
                        "(%" PRIu32 ") is not 0, so rasterizationSamples (0x%" PRIx32
                        ") must be a valid VkSampleCountFlagBits value.",
                        p_inherited_rendering_info->colorAttachmentCount, p_inherited_rendering_info->rasterizationSamples);
                }

                if ((!enabled_features.variableMultisampleRate) &&
                    (p_inherited_rendering_info->rasterizationSamples & AllVkSampleCountFlagBits) == 0) {
                    skip |= LogError(
                        "VUID-VkCommandBufferInheritanceRenderingInfo-variableMultisampleRate-06005", commandBuffer,
                        inheritance_loc.pNext(Struct::VkCommandBufferInheritanceRenderingInfo, Field::rasterizationSamples),
                        "is not valid (0x%" PRIx32 ") and the variableMultisampleRate feature was not enabled.",
                        p_inherited_rendering_info->rasterizationSamples);
                }

                for (uint32_t i = 0; i < p_inherited_rendering_info->colorAttachmentCount; ++i) {
                    if (p_inherited_rendering_info->pColorAttachmentFormats != nullptr) {
                        const VkFormat attachment_format = p_inherited_rendering_info->pColorAttachmentFormats[i];
                        if (attachment_format != VK_FORMAT_UNDEFINED) {
                            const VkFormatFeatureFlags2 potential_format_features = GetPotentialFormatFeatures(attachment_format);
                            if ((potential_format_features & (VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BIT |
                                                              VK_FORMAT_FEATURE_2_LINEAR_COLOR_ATTACHMENT_BIT_NV)) == 0) {
                                skip |= LogError("VUID-VkCommandBufferInheritanceRenderingInfo-pColorAttachmentFormats-06492",
                                                 commandBuffer,
                                                 inheritance_loc.pNext(Struct::VkCommandBufferInheritanceRenderingInfo,
                                                                       Field::pColorAttachmentFormats, i),
                                                 "is %s with potential format features %s.", string_VkFormat(attachment_format),
                                                 string_VkFormatFeatureFlags2(potential_format_features).c_str());
                            }
                        }
                    }
                }

                const VkFormatFeatureFlags2 valid_depth_stencil_format = VK_FORMAT_FEATURE_2_DEPTH_STENCIL_ATTACHMENT_BIT;
                const VkFormat depth_format = p_inherited_rendering_info->depthAttachmentFormat;
                if (depth_format != VK_FORMAT_UNDEFINED) {
                    const VkFormatFeatureFlags2 potential_format_features = GetPotentialFormatFeatures(depth_format);
                    if ((potential_format_features & valid_depth_stencil_format) == 0) {
                        skip |= LogError(
                            "VUID-VkCommandBufferInheritanceRenderingInfo-depthAttachmentFormat-06007", commandBuffer,
                            inheritance_loc.pNext(Struct::VkCommandBufferInheritanceRenderingInfo, Field::depthAttachmentFormat),
                            "is %s with potential format features %s.", string_VkFormat(depth_format),
                            string_VkFormatFeatureFlags2(potential_format_features).c_str());
                    }
                    if (!vkuFormatHasDepth(depth_format)) {
                        skip |= LogError(
                            "VUID-VkCommandBufferInheritanceRenderingInfo-depthAttachmentFormat-06540", commandBuffer,
                            inheritance_loc.pNext(Struct::VkCommandBufferInheritanceRenderingInfo, Field::depthAttachmentFormat),
                            "%s is not a depth format.", string_VkFormat(depth_format));
                    }
                }

                const VkFormat stencil_format = p_inherited_rendering_info->stencilAttachmentFormat;
                if (stencil_format != VK_FORMAT_UNDEFINED) {
                    const VkFormatFeatureFlags2 potential_format_features = GetPotentialFormatFeatures(stencil_format);
                    if ((potential_format_features & valid_depth_stencil_format) == 0) {
                        skip |= LogError(
                            "VUID-VkCommandBufferInheritanceRenderingInfo-stencilAttachmentFormat-06199", commandBuffer,
                            inheritance_loc.pNext(Struct::VkCommandBufferInheritanceRenderingInfo, Field::stencilAttachmentFormat),
                            "is %s with potential format features %s.", string_VkFormat(stencil_format),
                            string_VkFormatFeatureFlags2(potential_format_features).c_str());
                    }
                    if (!vkuFormatHasStencil(stencil_format)) {
                        skip |= LogError(
                            "VUID-VkCommandBufferInheritanceRenderingInfo-stencilAttachmentFormat-06541", commandBuffer,
                            inheritance_loc.pNext(Struct::VkCommandBufferInheritanceRenderingInfo, Field::stencilAttachmentFormat),
                            "%s is not a stencil format.", string_VkFormat(stencil_format));
                    }
                }

                if ((depth_format != VK_FORMAT_UNDEFINED && stencil_format != VK_FORMAT_UNDEFINED) &&
                    (depth_format != stencil_format)) {
                    skip |= LogError(
                        "VUID-VkCommandBufferInheritanceRenderingInfo-depthAttachmentFormat-06200", commandBuffer,
                        inheritance_loc.pNext(Struct::VkCommandBufferInheritanceRenderingInfo, Field::depthAttachmentFormat),
                        "(%s) is not the same as stencilAttachmentFormat (%s).", string_VkFormat(depth_format),
                        string_VkFormat(stencil_format));
                }

                if ((enabled_features.multiview == VK_FALSE) && (p_inherited_rendering_info->viewMask != 0)) {
                    skip |= LogError("VUID-VkCommandBufferInheritanceRenderingInfo-multiview-06008", commandBuffer,
                                     inheritance_loc.pNext(Struct::VkCommandBufferInheritanceRenderingInfo, Field::viewMask),
                                     "is %" PRIu32 ", but the multiview feature was not enabled.",
                                     p_inherited_rendering_info->viewMask);
                }

                if (MostSignificantBit(p_inherited_rendering_info->viewMask) >=
                    static_cast<int32_t>(phys_dev_props_core11.maxMultiviewViewCount)) {
                    skip |=
                        LogError("VUID-VkCommandBufferInheritanceRenderingInfo-viewMask-06009", commandBuffer,
                                 inheritance_loc.pNext(Struct::VkCommandBufferInheritanceRenderingInfo, Field::viewMask),
                                 "(0x%" PRIx32 ") most significant bit is greater or equal to maxMultiviewViewCount (%" PRIu32 ").",
                                 p_inherited_rendering_info->viewMask, phys_dev_props_core11.maxMultiviewViewCount);
                }
            }
        }

        if (info) {
            if ((info->occlusionQueryEnable == VK_FALSE || enabled_features.occlusionQueryPrecise == VK_FALSE) &&
                (info->queryFlags & VK_QUERY_CONTROL_PRECISE_BIT)) {
                skip |= LogError("VUID-vkBeginCommandBuffer-commandBuffer-00052", commandBuffer, inheritance_loc,
                                 "Secondary %s must not have VK_QUERY_CONTROL_PRECISE_BIT if "
                                 "occulusionQuery is disabled or the device does not support precise occlusion queries.",
                                 FormatHandle(commandBuffer).c_str());
            }
            auto p_inherited_viewport_scissor_info =
                vku::FindStructInPNextChain<VkCommandBufferInheritanceViewportScissorInfoNV>(info->pNext);
            if (p_inherited_viewport_scissor_info != nullptr && p_inherited_viewport_scissor_info->viewportScissor2D) {
                if (!enabled_features.inheritedViewportScissor2D) {
                    skip |= LogError(
                        "VUID-VkCommandBufferInheritanceViewportScissorInfoNV-viewportScissor2D-04782", commandBuffer,
                        inheritance_loc.pNext(Struct::VkCommandBufferInheritanceViewportScissorInfoNV, Field::viewportScissor2D),
                        "is VK_TRUE, but the inheritedViewportScissor2D feature was not enabled.");
                }
                if (!(pBeginInfo->flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT)) {
                    skip |=
                        LogError("VUID-VkCommandBufferInheritanceViewportScissorInfoNV-viewportScissor2D-04786", commandBuffer,
                                 begin_info_loc.dot(Field::flags), "is %s for Secondary %s (and viewportScissor2D is VK_TRUE).",
                                 string_VkCommandBufferUsageFlags(pBeginInfo->flags).c_str(), FormatHandle(commandBuffer).c_str());
                }
                if (p_inherited_viewport_scissor_info->viewportDepthCount == 0) {
                    skip |= LogError(
                        "VUID-VkCommandBufferInheritanceViewportScissorInfoNV-viewportScissor2D-04784", commandBuffer,
                        inheritance_loc.pNext(Struct::VkCommandBufferInheritanceViewportScissorInfoNV, Field::viewportDepthCount),
                        "is zero (but viewportScissor2D is VK_TRUE).");
                }
            }

            // Check for dynamic rendering feature enabled or 1.3
            if ((api_version < VK_API_VERSION_1_3) && (!enabled_features.dynamicRendering)) {
                if (pBeginInfo->flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT) {
                    if (info->renderPass == VK_NULL_HANDLE) {
                        skip |=
                            LogError("VUID-VkCommandBufferBeginInfo-flags-09240", commandBuffer, begin_info_loc.dot(Field::flags),
                                     "includes VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT "
                                     "but the renderpass member of pInheritanceInfo is VK_NULL_HANDLE.");
                    }
                }
            }
        }
    }
    if (CbState::Recording == cb_state->state) {
        skip |= LogError("VUID-vkBeginCommandBuffer-commandBuffer-00049", commandBuffer, error_obj.location,
                         "Cannot call Begin on %s in the RECORDING state. Must first call "
                         "vkEndCommandBuffer().",
                         FormatHandle(commandBuffer).c_str());
    } else if (CbState::Recorded == cb_state->state || CbState::InvalidComplete == cb_state->state) {
        VkCommandPool cmd_pool = cb_state->allocate_info.commandPool;
        const auto *pool = cb_state->command_pool;
        if (!(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT & pool->createFlags)) {
            const LogObjectList objlist(commandBuffer, cmd_pool);
            skip |= LogError("VUID-vkBeginCommandBuffer-commandBuffer-00050", objlist, error_obj.location,
                             "%s attempts to implicitly reset cmdBuffer created from "
                             "%s that does NOT have the VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT bit set.",
                             FormatHandle(commandBuffer).c_str(), FormatHandle(cmd_pool).c_str());
        }
    }
    auto chained_device_group_struct = vku::FindStructInPNextChain<VkDeviceGroupCommandBufferBeginInfo>(pBeginInfo->pNext);
    if (chained_device_group_struct) {
        const LogObjectList objlist(commandBuffer);
        skip |= ValidateDeviceMaskToPhysicalDeviceCount(
            chained_device_group_struct->deviceMask, objlist,
            begin_info_loc.pNext(Struct::VkDeviceGroupCommandBufferBeginInfo, Field::deviceMask),
            "VUID-VkDeviceGroupCommandBufferBeginInfo-deviceMask-00106");
        skip |= ValidateDeviceMaskToZero(chained_device_group_struct->deviceMask, objlist,
                                         begin_info_loc.pNext(Struct::VkDeviceGroupCommandBufferBeginInfo, Field::deviceMask),
                                         "VUID-VkDeviceGroupCommandBufferBeginInfo-deviceMask-00107");
    }
    if ((pBeginInfo->flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT) != 0) {
        if ((cb_state->command_pool->queue_flags & VK_QUEUE_GRAPHICS_BIT) == 0) {
            const LogObjectList objlist(commandBuffer, cb_state->command_pool->Handle());
            skip |= LogError("VUID-VkCommandBufferBeginInfo-flags-09123", objlist, begin_info_loc.dot(Field::flags),
                             "contain VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT, but the command pool (created with "
                             "queueFamilyIndex %" PRIu32 ") the command buffer %s was allocated from only supports %s.",
                             cb_state->command_pool->queueFamilyIndex, FormatHandle(commandBuffer).c_str(),
                             string_VkQueueFlags(cb_state->command_pool->queue_flags).c_str());
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateEndCommandBuffer(VkCommandBuffer commandBuffer, const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state_ptr = GetRead<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state_ptr) {
        return skip;
    }
    const vvl::CommandBuffer &cb_state = *cb_state_ptr;
    if (cb_state.IsPrimary() || !(cb_state.beginInfo.flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT)) {
        skip |= InsideRenderPass(cb_state, error_obj.location, "VUID-vkEndCommandBuffer-commandBuffer-00060");
    }

    if (cb_state.state == CbState::InvalidComplete || cb_state.state == CbState::InvalidIncomplete) {
        skip |= ReportInvalidCommandBuffer(cb_state, error_obj.location, "VUID-vkEndCommandBuffer-commandBuffer-00059");
    } else if (CbState::Recording != cb_state.state) {
        skip |= LogError("VUID-vkEndCommandBuffer-commandBuffer-00059", commandBuffer, error_obj.location,
                         "Cannot call End on %s when not in the RECORDING state. Must first call vkBeginCommandBuffer().",
                         FormatHandle(commandBuffer).c_str());
    }

    for (const auto &query_obj : cb_state.activeQueries) {
        skip |= LogError("VUID-vkEndCommandBuffer-commandBuffer-00061", commandBuffer, error_obj.location,
                         "Ending command buffer with in progress query: %s, query %d.", FormatHandle(query_obj.pool).c_str(),
                         query_obj.slot);
    }
    if (cb_state.conditional_rendering_active) {
        skip |= LogError("VUID-vkEndCommandBuffer-None-01978", commandBuffer, error_obj.location,
                         "Ending command buffer with active conditional rendering.");
    }

    skip |= InsideVideoCodingScope(cb_state, error_obj.location, "VUID-vkEndCommandBuffer-None-06991");

    return skip;
}

bool CoreChecks::PreCallValidateResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags,
                                                   const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) return false;
    VkCommandPool cmd_pool = cb_state->allocate_info.commandPool;
    const auto *pool = cb_state->command_pool;

    if (!(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT & pool->createFlags)) {
        const LogObjectList objlist(commandBuffer, cmd_pool);
        skip |= LogError("VUID-vkResetCommandBuffer-commandBuffer-00046", objlist, error_obj.location,
                         "%s was created from %s  which was created with %s.", FormatHandle(commandBuffer).c_str(),
                         FormatHandle(cmd_pool).c_str(), string_VkCommandPoolCreateFlags(pool->createFlags).c_str());
    }

    if (cb_state->InUse()) {
        const LogObjectList objlist(commandBuffer, cmd_pool);
        skip |= LogError("VUID-vkResetCommandBuffer-commandBuffer-00045", objlist, error_obj.location, "(%s) is in use.",
                         FormatHandle(commandBuffer).c_str());
    }

    return skip;
}

bool CoreChecks::ValidateCmdBindIndexBuffer(const vvl::CommandBuffer &cb_state, VkBuffer buffer, VkDeviceSize offset,
                                            VkIndexType indexType, const Location &loc) const {
    bool skip = false;
    const bool is_2 = loc.function == Func::vkCmdBindIndexBuffer2KHR || loc.function == Func::vkCmdBindIndexBuffer2;
    const char *vuid;

    auto buffer_state = Get<vvl::Buffer>(buffer);
    if (!buffer_state) return skip;  // if using nullDescriptors
    const LogObjectList objlist(cb_state.Handle(), buffer);

    vuid = is_2 ? "VUID-vkCmdBindIndexBuffer2-buffer-08784" : "VUID-vkCmdBindIndexBuffer-buffer-08784";
    skip |= ValidateBufferUsageFlags(objlist, *buffer_state, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, true, vuid, loc.dot(Field::buffer));
    vuid = is_2 ? "VUID-vkCmdBindIndexBuffer2-buffer-08785" : "VUID-vkCmdBindIndexBuffer-buffer-08785";
    skip |= ValidateMemoryIsBoundToBuffer(cb_state.Handle(), *buffer_state, loc.dot(Field::buffer), vuid);

    const VkDeviceSize offset_align = static_cast<VkDeviceSize>(GetIndexAlignment(indexType));
    if (!IsIntegerMultipleOf(offset, offset_align)) {
        vuid = is_2 ? "VUID-vkCmdBindIndexBuffer2-offset-08783" : "VUID-vkCmdBindIndexBuffer-offset-08783";
        skip |= LogError(vuid, objlist, loc.dot(Field::offset), "(%" PRIu64 ") does not fall on alignment (%s) boundary.", offset,
                         string_VkIndexType(indexType));
    }
    if (offset >= buffer_state->create_info.size) {
        vuid = is_2 ? "VUID-vkCmdBindIndexBuffer2-offset-08782" : "VUID-vkCmdBindIndexBuffer-offset-08782";
        skip |= LogError(vuid, objlist, loc.dot(Field::offset), "(%" PRIu64 ") is not less than the size (%" PRIu64 ").", offset,
                         buffer_state->create_info.size);
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   VkIndexType indexType, const ErrorObject &error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateCmd(*cb_state, error_obj.location);
    skip |= ValidateCmdBindIndexBuffer(*cb_state, buffer, offset, indexType, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdBindIndexBuffer2(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                    VkDeviceSize size, VkIndexType indexType, const ErrorObject &error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateCmd(*cb_state, error_obj.location);
    skip |= ValidateCmdBindIndexBuffer(*cb_state, buffer, offset, indexType, error_obj.location);

    if (size != VK_WHOLE_SIZE && buffer != VK_NULL_HANDLE) {
        auto buffer_state = Get<vvl::Buffer>(buffer);
        if (!buffer_state) return skip;  // if using nullDescriptors

        const VkDeviceSize offset_align = static_cast<VkDeviceSize>(GetIndexAlignment(indexType));
        if (!IsIntegerMultipleOf(size, offset_align)) {
            const LogObjectList objlist(commandBuffer, buffer);
            skip |= LogError("VUID-vkCmdBindIndexBuffer2-size-08767", objlist, error_obj.location.dot(Field::size),
                             "(%" PRIu64 ") does not fall on alignment (%s) boundary.", size, string_VkIndexType(indexType));
        }
        if ((offset + size) > buffer_state->create_info.size) {
            const LogObjectList objlist(commandBuffer, buffer);
            skip |= LogError("VUID-vkCmdBindIndexBuffer2-size-08768", objlist, error_obj.location.dot(Field::size),
                             "(%" PRIu64 ") + offset (%" PRIu64 ") is larger than the buffer size (%" PRIu64 ").", size, offset,
                             buffer_state->create_info.size);
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdBindIndexBuffer2KHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkDeviceSize size, VkIndexType indexType,
                                                       const ErrorObject &error_obj) const {
    return PreCallValidateCmdBindIndexBuffer2(commandBuffer, buffer, offset, size, indexType, error_obj);
}

bool CoreChecks::PreCallValidateCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                     const VkBuffer *pBuffers, const VkDeviceSize *pOffsets,
                                                     const ErrorObject &error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);

    bool skip = false;
    skip |= ValidateCmd(*cb_state, error_obj.location);
    for (uint32_t i = 0; i < bindingCount; ++i) {
        auto buffer_state = Get<vvl::Buffer>(pBuffers[i]);
        if (!buffer_state) continue;  // if using nullDescriptors

        const LogObjectList objlist(commandBuffer, buffer_state->Handle());
        skip |= ValidateBufferUsageFlags(objlist, *buffer_state, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, true,
                                         "VUID-vkCmdBindVertexBuffers-pBuffers-00627", error_obj.location.dot(Field::pBuffers, i));
        skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *buffer_state, error_obj.location.dot(Field::pBuffers, i),
                                              "VUID-vkCmdBindVertexBuffers-pBuffers-00628");
        if (pOffsets[i] >= buffer_state->create_info.size) {
            skip |= LogError("VUID-vkCmdBindVertexBuffers-pOffsets-00626", objlist, error_obj.location.dot(Field::pOffsets, i),
                             "(%" PRIu64 ") is larger than the buffer size (%" PRIu64 ").", pOffsets[i],
                             buffer_state->create_info.size);
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                VkDeviceSize dataSize, const void *pData, const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    auto dst_buffer_state = Get<vvl::Buffer>(dstBuffer);
    ASSERT_AND_RETURN_SKIP(dst_buffer_state);

    const LogObjectList objlist(commandBuffer, dstBuffer);
    const Location buffer_loc = error_obj.location.dot(Field::dstBuffer);

    skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *dst_buffer_state, buffer_loc, "VUID-vkCmdUpdateBuffer-dstBuffer-00035");
    // Validate that DST buffer has correct usage flags set
    skip |= ValidateBufferUsageFlags(objlist, *dst_buffer_state, VK_BUFFER_USAGE_TRANSFER_DST_BIT, true,
                                     "VUID-vkCmdUpdateBuffer-dstBuffer-00034", buffer_loc);
    skip |= ValidateCmd(*cb_state, error_obj.location);
    skip |= ValidateProtectedBuffer(*cb_state, *dst_buffer_state, buffer_loc, "VUID-vkCmdUpdateBuffer-commandBuffer-01813");
    skip |= ValidateUnprotectedBuffer(*cb_state, *dst_buffer_state, buffer_loc, "VUID-vkCmdUpdateBuffer-commandBuffer-01814");
    if (dstOffset >= dst_buffer_state->create_info.size) {
        skip |= LogError("VUID-vkCmdUpdateBuffer-dstOffset-00032", objlist, error_obj.location.dot(Field::dstOffset),
                         "(%" PRIu64 ") is not less than the size (%" PRIu64 ").", dstOffset, dst_buffer_state->create_info.size);
    } else if (dataSize > dst_buffer_state->create_info.size - dstOffset) {
        skip |= LogError("VUID-vkCmdUpdateBuffer-dataSize-00033", objlist, error_obj.location.dot(Field::dataSize),
                         "(%" PRIu64 ") is not less than the buffer size (%" PRIu64 ") minus dstOffset (%" PRIu64 ").", dataSize,
                         dst_buffer_state->create_info.size, dstOffset);
    }
    return skip;
}

bool CoreChecks::ValidatePrimaryCommandBuffer(const vvl::CommandBuffer &cb_state, const Location &loc, const char *vuid) const {
    bool skip = false;
    if (cb_state.IsSecondary()) {
        skip |= LogError(vuid, cb_state.Handle(), loc, "command can't be executed on a secondary command buffer.");
    }
    return skip;
}

bool CoreChecks::ValidateSecondaryCommandBufferState(const vvl::CommandBuffer &cb_state, const vvl::CommandBuffer &sub_cb_state,
                                                     const Location &cb_loc) const {
    bool skip = false;

    vvl::unordered_set<int> active_types;
    if (!disabled[query_validation]) {
        for (const auto &query_object : cb_state.activeQueries) {
            auto query_pool_state = Get<vvl::QueryPool>(query_object.pool);
            if (!query_pool_state) continue;
            if (query_pool_state->create_info.queryType == VK_QUERY_TYPE_PIPELINE_STATISTICS &&
                sub_cb_state.beginInfo.pInheritanceInfo) {
                VkQueryPipelineStatisticFlags cmd_buf_statistics = sub_cb_state.beginInfo.pInheritanceInfo->pipelineStatistics;
                if ((cmd_buf_statistics & query_pool_state->create_info.pipelineStatistics) !=
                    query_pool_state->create_info.pipelineStatistics) {
                    const LogObjectList objlist(cb_state.Handle(), sub_cb_state.Handle(), query_object.pool);
                    skip |= LogError(
                        "VUID-vkCmdExecuteCommands-commandBuffer-00104", objlist, cb_loc,
                        "was created with pInheritanceInfo::pipelineStatistics %s but the active query pool (%s) was "
                        "created with %s.",
                        string_VkQueryPipelineStatisticFlags(cmd_buf_statistics).c_str(), FormatHandle(query_object.pool).c_str(),
                        string_VkQueryPipelineStatisticFlags(query_pool_state->create_info.pipelineStatistics).c_str());
                }
            }
            active_types.insert(query_pool_state->create_info.queryType);
        }
        for (const auto &query_object : sub_cb_state.startedQueries) {
            auto query_pool_state = Get<vvl::QueryPool>(query_object.pool);
            if (!query_pool_state) continue;
            if (active_types.count(query_pool_state->create_info.queryType)) {
                const LogObjectList objlist(cb_state.Handle(), sub_cb_state.Handle(), query_object.pool);
                skip |= LogError("VUID-vkCmdExecuteCommands-pCommandBuffers-00105", objlist, cb_loc,
                                 "called with invalid %s which has invalid active %s"
                                 " of type %s but a query of that type has been started on secondary command buffer %s.",
                                 FormatHandle(cb_state).c_str(), FormatHandle(query_object.pool).c_str(),
                                 string_VkQueryType(query_pool_state->create_info.queryType), FormatHandle(sub_cb_state).c_str());
            }
        }
    }
    const auto primary_pool = cb_state.command_pool;
    const auto secondary_pool = sub_cb_state.command_pool;
    if (primary_pool && secondary_pool && (primary_pool->queueFamilyIndex != secondary_pool->queueFamilyIndex)) {
        const LogObjectList objlist(sub_cb_state.Handle(), cb_state.Handle());
        skip |= LogError("VUID-vkCmdExecuteCommands-pCommandBuffers-00094", objlist, cb_loc,
                         "Primary command buffer %s created in queue family %" PRIu32
                         " has secondary command buffer %s created in queue family %" PRIu32 ".",
                         FormatHandle(cb_state).c_str(), primary_pool->queueFamilyIndex, FormatHandle(sub_cb_state).c_str(),
                         secondary_pool->queueFamilyIndex);
    }

    return skip;
}

// Object that simulates the inherited viewport/scissor state as the device executes the called secondary command buffers.
// Visit the calling primary command buffer first, then the called secondaries in order.
// Contact David Zhao Akeley <dakeley@nvidia.com> for clarifications and bug fixes.
class CoreChecks::ViewportScissorInheritanceTracker {
    static_assert(4 == sizeof(vvl::CommandBuffer::viewportMask), "Adjust max_viewports to match viewportMask bit width");
    static constexpr uint32_t kMaxViewports = 32, kNotTrashed = uint32_t(-2), kTrashedByPrimary = uint32_t(-1);

    const vvl::Device &validation_;
    const vvl::CommandBuffer *primary_state_ = nullptr;
    uint32_t viewport_mask_;
    uint32_t scissor_mask_;
    uint32_t viewport_trashed_by_[kMaxViewports];  // filled in VisitPrimary.
    uint32_t scissor_trashed_by_[kMaxViewports];
    VkViewport viewports_to_inherit_[kMaxViewports];
    uint32_t viewport_count_to_inherit_;  // 0 if viewport count (EXT state) has never been defined (but not trashed)
    uint32_t scissor_count_to_inherit_;   // 0 if scissor count (EXT state) has never been defined (but not trashed)
    uint32_t viewport_count_trashed_by_;
    uint32_t scissor_count_trashed_by_;

  public:
    ViewportScissorInheritanceTracker(const vvl::Device &validation) : validation_(validation) {}

    bool VisitPrimary(const vvl::CommandBuffer &primary_state) {
        assert(!primary_state_);
        primary_state_ = &primary_state;

        viewport_mask_ = primary_state.viewportMask | primary_state.viewportWithCountMask;
        scissor_mask_ = primary_state.scissorMask | primary_state.scissorWithCountMask;

        for (uint32_t n = 0; n < kMaxViewports; ++n) {
            uint32_t bit = uint32_t(1) << n;
            viewport_trashed_by_[n] = primary_state.trashedViewportMask & bit ? kTrashedByPrimary : kNotTrashed;
            scissor_trashed_by_[n] = primary_state.trashedScissorMask & bit ? kTrashedByPrimary : kNotTrashed;
            if (n < primary_state.dynamic_state_value.viewports.size() && viewport_mask_ & bit) {
                viewports_to_inherit_[n] = primary_state.dynamic_state_value.viewports[n];
            }
        }

        viewport_count_to_inherit_ = primary_state.dynamic_state_value.viewport_count;
        scissor_count_to_inherit_ = primary_state.dynamic_state_value.scissor_count;
        viewport_count_trashed_by_ = primary_state.trashedViewportCount ? kTrashedByPrimary : kNotTrashed;
        scissor_count_trashed_by_ = primary_state.trashedScissorCount ? kTrashedByPrimary : kNotTrashed;
        return false;
    }

    bool VisitSecondary(uint32_t cmd_buffer_idx, const Location &cb_loc, const vvl::CommandBuffer &secondary_state) {
        bool skip = false;
        if (secondary_state.inheritedViewportDepths.empty()) {
            skip |= VisitSecondaryNoInheritance(cmd_buffer_idx, secondary_state);
        } else {
            skip |= VisitSecondaryInheritance(cmd_buffer_idx, cb_loc, secondary_state);
        }

        // See note at end of VisitSecondaryNoInheritance.
        if (secondary_state.trashedViewportCount) {
            viewport_count_trashed_by_ = cmd_buffer_idx;
        }
        if (secondary_state.trashedScissorCount) {
            scissor_count_trashed_by_ = cmd_buffer_idx;
        }
        return skip;
    }

  private:
    // Track state inheritance as specified by VK_NV_inherited_scissor_viewport, including states
    // overwritten to undefined value by bound pipelines with non-dynamic state.
    bool VisitSecondaryNoInheritance(uint32_t cmd_buffer_idx, const vvl::CommandBuffer &secondary_state) {
        viewport_mask_ |= secondary_state.viewportMask | secondary_state.viewportWithCountMask;
        scissor_mask_ |= secondary_state.scissorMask | secondary_state.scissorWithCountMask;

        for (uint32_t n = 0; n < kMaxViewports; ++n) {
            uint32_t bit = uint32_t(1) << n;
            if ((secondary_state.viewportMask | secondary_state.viewportWithCountMask) & bit) {
                if (n < secondary_state.dynamic_state_value.viewports.size()) {
                    viewports_to_inherit_[n] = secondary_state.dynamic_state_value.viewports[n];
                }
                viewport_trashed_by_[n] = kNotTrashed;
            }
            if ((secondary_state.scissorMask | secondary_state.scissorWithCountMask) & bit) {
                scissor_trashed_by_[n] = kNotTrashed;
            }
            if (secondary_state.dynamic_state_value.viewport_count != 0) {
                viewport_count_to_inherit_ = secondary_state.dynamic_state_value.viewport_count;
                viewport_count_trashed_by_ = kNotTrashed;
            }
            if (secondary_state.dynamic_state_value.scissor_count != 0) {
                scissor_count_to_inherit_ = secondary_state.dynamic_state_value.scissor_count;
                scissor_count_trashed_by_ = kNotTrashed;
            }
            // Order of above vs below matters here.
            if (secondary_state.trashedViewportMask & bit) {
                viewport_trashed_by_[n] = cmd_buffer_idx;
            }
            if (secondary_state.trashedScissorMask & bit) {
                scissor_trashed_by_[n] = cmd_buffer_idx;
            }
            // Check trashing dynamic viewport/scissor count in VisitSecondary (at end) as even secondary command buffers enabling
            // viewport/scissor state inheritance may define this state statically in bound graphics pipelines.
        }
        return false;
    }

    // Validate needed inherited state as specified by VK_NV_inherited_scissor_viewport.
    bool VisitSecondaryInheritance(uint32_t cmd_buffer_idx, const Location &cb_loc, const vvl::CommandBuffer &secondary_state) {
        bool skip = false;
        uint32_t check_viewport_count = 0, check_scissor_count = 0;

        // Common code for reporting missing inherited state (for a myriad of reasons).
        auto check_missing_inherit = [&](uint32_t was_ever_defined, uint32_t trashed_by, VkDynamicState state, uint32_t index = 0,
                                         uint32_t static_use_count = 0, const VkViewport *inherited_viewport = nullptr,
                                         const VkViewport *expected_viewport_depth = nullptr) {
            if (was_ever_defined && trashed_by == kNotTrashed) {
                if (state != VK_DYNAMIC_STATE_VIEWPORT) return false;

                assert(inherited_viewport != nullptr && expected_viewport_depth != nullptr);
                if (inherited_viewport->minDepth != expected_viewport_depth->minDepth ||
                    inherited_viewport->maxDepth != expected_viewport_depth->maxDepth) {
                    return validation_.LogError("VUID-vkCmdDraw-None-07850", primary_state_->Handle(), cb_loc,
                                                "(%s) consume inherited viewport %" PRIu32
                                                " %s"
                                                "but this state was not inherited as its depth range [%f, %f] does not match "
                                                "pViewportDepths[%" PRIu32 "] = [%f, %f]",
                                                validation_.FormatHandle(secondary_state.Handle()).c_str(), unsigned(index),
                                                index >= static_use_count ? "(with count) " : "", inherited_viewport->minDepth,
                                                inherited_viewport->maxDepth, unsigned(cmd_buffer_idx),
                                                expected_viewport_depth->minDepth, expected_viewport_depth->maxDepth);
                    // akeley98 note: This VUID is not ideal; however, there isn't a more relevant VUID as
                    // it isn't illegal in itself to have mismatched inherited viewport depths.
                    // The error only occurs upon attempting to consume the viewport.
                } else {
                    return false;
                }
            }

            const char *state_name;
            bool format_index = false;

            switch (state) {
                case VK_DYNAMIC_STATE_SCISSOR:
                    state_name = "scissor";
                    format_index = true;
                    break;
                case VK_DYNAMIC_STATE_VIEWPORT:
                    state_name = "viewport";
                    format_index = true;
                    break;
                case VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT:
                    state_name = "dynamic viewport count";
                    break;
                case VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT:
                    state_name = "dynamic scissor count";
                    break;
                default:
                    assert(false);
                    state_name = "<unknown state, report bug>";
                    break;
            }

            std::stringstream ss;
            ss << "(" << validation_.FormatHandle(secondary_state.Handle()).c_str() << ") consume inherited " << state_name << " ";
            if (format_index) {
                if (index >= static_use_count) {
                    ss << "(with count) ";
                }
                ss << index << " ";
            }
            ss << "but this state ";
            if (!was_ever_defined) {
                ss << "was never defined.";
            } else if (trashed_by == kTrashedByPrimary) {
                ss << "was left undefined after vkCmdExecuteCommands or vkCmdBindPipeline (with non-dynamic state) in "
                      "the calling primary command buffer.";
            } else {
                ss << "was left undefined after vkCmdBindPipeline (with non-dynamic state) in pCommandBuffers[" << trashed_by
                   << "].";
            }
            return validation_.LogError("VUID-vkCmdDraw-None-07850", primary_state_->Handle(), cb_loc, "%s", ss.str().c_str());
        };

        // Check if secondary command buffer uses viewport/scissor-with-count state, and validate this state if so.
        if (secondary_state.usedDynamicViewportCount) {
            if (viewport_count_to_inherit_ == 0 || viewport_count_trashed_by_ != kNotTrashed) {
                skip |= check_missing_inherit(viewport_count_to_inherit_, viewport_count_trashed_by_,
                                              VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
            } else {
                check_viewport_count = viewport_count_to_inherit_;
            }
        }
        if (secondary_state.usedDynamicScissorCount) {
            if (scissor_count_to_inherit_ == 0 || scissor_count_trashed_by_ != kNotTrashed) {
                skip |= check_missing_inherit(scissor_count_to_inherit_, scissor_count_trashed_by_,
                                              VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT);
            } else {
                check_scissor_count = scissor_count_to_inherit_;
            }
        }

        // Check the maximum of (viewports used by pipelines with static viewport count, "" dynamic viewport count)
        // but limit to length of inheritedViewportDepths array and uint32_t bit width (validation layer limit).
        check_viewport_count = std::min(std::min(kMaxViewports, uint32_t(secondary_state.inheritedViewportDepths.size())),
                                        std::max(check_viewport_count, secondary_state.usedViewportScissorCount));
        check_scissor_count = std::min(kMaxViewports, std::max(check_scissor_count, secondary_state.usedViewportScissorCount));

        if (secondary_state.usedDynamicViewportCount &&
            viewport_count_to_inherit_ > secondary_state.inheritedViewportDepths.size()) {
            skip |= validation_.LogError(
                "VUID-vkCmdDraw-None-07850", primary_state_->Handle(), cb_loc,
                "(%s) consume inherited dynamic viewport with count state "
                "but the dynamic viewport count (%" PRIu32 ") exceeds the inheritance limit (viewportDepthCount=%" PRIu32 ").",
                validation_.FormatHandle(secondary_state.Handle()).c_str(), unsigned(viewport_count_to_inherit_),
                unsigned(secondary_state.inheritedViewportDepths.size()));
        }

        for (uint32_t n = 0; n < check_viewport_count; ++n) {
            skip |= check_missing_inherit(viewport_mask_ & uint32_t(1) << n, viewport_trashed_by_[n], VK_DYNAMIC_STATE_VIEWPORT, n,
                                          secondary_state.usedViewportScissorCount, &viewports_to_inherit_[n],
                                          &secondary_state.inheritedViewportDepths[n]);
        }

        for (uint32_t n = 0; n < check_scissor_count; ++n) {
            skip |= check_missing_inherit(scissor_mask_ & uint32_t(1) << n, scissor_trashed_by_[n], VK_DYNAMIC_STATE_SCISSOR, n,
                                          secondary_state.usedViewportScissorCount);
        }
        return skip;
    }
};

constexpr uint32_t CoreChecks::ViewportScissorInheritanceTracker::kMaxViewports;
constexpr uint32_t CoreChecks::ViewportScissorInheritanceTracker::kNotTrashed;
constexpr uint32_t CoreChecks::ViewportScissorInheritanceTracker::kTrashedByPrimary;

bool CoreChecks::PreCallValidateCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBuffersCount,
                                                   const VkCommandBuffer *pCommandBuffers, const ErrorObject &error_obj) const {
    const auto &cb_state = *GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    vvl::unordered_set<const vvl::CommandBuffer *> linked_command_buffers;
    ViewportScissorInheritanceTracker viewport_scissor_inheritance{*this};

    if (enabled_features.inheritedViewportScissor2D) {
        skip |= viewport_scissor_inheritance.VisitPrimary(cb_state);
    }

    if (!cb_state.IsPrimary()) {
        if (!enabled_features.nestedCommandBuffer) {
            skip |= LogError("VUID-vkCmdExecuteCommands-commandBuffer-09375", commandBuffer,
                             error_obj.location.dot(Field::commandBuffer),
                             "is a secondary command buffer (and the nestedCommandBuffer feature was not enabled).");
        } else if (!enabled_features.nestedCommandBufferRendering &&
                   ((cb_state.beginInfo.flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT) != 0)) {
            skip |= LogError(
                "VUID-vkCmdExecuteCommands-nestedCommandBufferRendering-09377", commandBuffer,
                error_obj.location.dot(Field::commandBuffer),
                "is a secondary command buffer and was recorded with VkCommandBufferBeginInfo::flag including "
                "VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT, but the nestedCommandBufferRendering feature was not enabled.");
        }
    }

    const QueryObject *active_occlusion_query = nullptr;
    for (const auto &active_query : cb_state.activeQueries) {
        auto query_pool_state = Get<vvl::QueryPool>(active_query.pool);
        if (!query_pool_state) continue;
        const auto queryType = query_pool_state->create_info.queryType;
        if (queryType == VK_QUERY_TYPE_OCCLUSION) {
            active_occlusion_query = &active_query;
        }
        if (queryType != VK_QUERY_TYPE_OCCLUSION && queryType != VK_QUERY_TYPE_PIPELINE_STATISTICS) {
            skip |= LogError("VUID-vkCmdExecuteCommands-commandBuffer-07594", commandBuffer, error_obj.location,
                             "query with type %s is active.", string_VkQueryType(queryType));
        }
    }

    const vvl::RenderPass *rp_state = cb_state.active_render_pass.get();
    if (rp_state) {
        if (!rp_state->UsesDynamicRendering() && cb_state.IsPrimary()) {
            // check if first subpass
            if (cb_state.activeSubpassContents != VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS &&
                cb_state.activeSubpassContents != VK_SUBPASS_CONTENTS_INLINE_AND_SECONDARY_COMMAND_BUFFERS_KHR) {
                if (cb_state.GetActiveSubpass() == 0) {
                    const LogObjectList objlist(commandBuffer, rp_state->Handle());
                    skip |= LogError("VUID-vkCmdExecuteCommands-contents-09680", objlist, error_obj.location,
                                     "contents must be set to VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS or "
                                     "VK_SUBPASS_CONTENTS_INLINE_AND_SECONDARY_COMMAND_BUFFERS_KHR"
                                     "when calling vkCmdExecuteCommands() within the first subpass.");
                } else {
                    const LogObjectList objlist(commandBuffer, rp_state->Handle());
                    skip |=
                        LogError("VUID-vkCmdExecuteCommands-None-09681", objlist, error_obj.location,
                                 "contents must be set to VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS or "
                                 "VK_SUBPASS_CONTENTS_INLINE_AND_SECONDARY_COMMAND_BUFFERS_KHR"
                                 "when calling vkCmdExecuteCommands() within a non-first subpass (currently subpass %" PRIu32 ").",
                                 cb_state.GetActiveSubpass());
                }
            }
        }

        if (cb_state.hasRenderPassInstance && rp_state->UsesDynamicRendering() &&
            !((rp_state->use_dynamic_rendering &&
               (rp_state->dynamic_rendering_begin_rendering_info.flags & VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT)) ||
              (rp_state->use_dynamic_rendering_inherited &&
               (rp_state->inheritance_rendering_info.flags & VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT)))) {
            const LogObjectList objlist(commandBuffer, rp_state->Handle());
            skip |= LogError("VUID-vkCmdExecuteCommands-flags-06024", objlist, error_obj.location,
                             "VkRenderingInfo::flags must include "
                             "VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT when calling vkCmdExecuteCommands() within a "
                             "render pass instance begun with vkCmdBeginRendering().");
        }
    }

    for (uint32_t i = 0; i < commandBuffersCount; i++) {
        const auto &sub_cb_state = *GetRead<vvl::CommandBuffer>(pCommandBuffers[i]);
        const Location cb_loc = error_obj.location.dot(Field::pCommandBuffers, i);
        const vvl::RenderPass *secondary_rp_state = sub_cb_state.active_render_pass.get();

        if (enabled_features.inheritedViewportScissor2D) {
            skip |= viewport_scissor_inheritance.VisitSecondary(i, cb_loc, sub_cb_state);
        }

        if (!sub_cb_state.IsSecondary()) {
            const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
            skip |= LogError("VUID-vkCmdExecuteCommands-pCommandBuffers-00088", objlist, cb_loc,
                             "(%s) is not VK_COMMAND_BUFFER_LEVEL_SECONDARY.", FormatHandle(pCommandBuffers[i]).c_str());
        } else if (!rp_state) {
            if (sub_cb_state.beginInfo.flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT) {
                const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
                skip |= LogError("VUID-vkCmdExecuteCommands-pCommandBuffers-00100", objlist, cb_loc,
                                 "(%s) is executed outside a render pass "
                                 "instance scope, but the Secondary Command Buffer does have the "
                                 "VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT set in VkCommandBufferBeginInfo::flags when "
                                 "the vkBeginCommandBuffer() was called.",
                                 FormatHandle(pCommandBuffers[i]).c_str());
            }
        } else if (rp_state) {
            if (cb_state.hasRenderPassInstance && rp_state->UsesDynamicRendering() && secondary_rp_state &&
                secondary_rp_state->UsesDynamicRendering()) {
                const auto *location_info = vku::FindStructInPNextChain<VkRenderingAttachmentLocationInfo>(
                    secondary_rp_state->inheritance_rendering_info.pNext);

                if (location_info) {
                    const std::string vuid_090504 = "VUID-vkCmdExecuteCommands-pCommandBuffers-09504";
                    const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
                    skip |= ValidateRenderingAttachmentLocations(*location_info, objlist, cb_loc.dot(Field::pNext));

                    if (location_info->colorAttachmentCount != cb_state.rendering_attachments.color_indexes.size()) {
                        skip |= LogError(vuid_090504, objlist,
                                         cb_loc.pNext(Struct::VkRenderingAttachmentLocationInfo, Field::colorAttachmentCount),
                                         "(%" PRIu32
                                         ") does not match the implicit or explicit state in the primary command buffer ("
                                         "%" PRIu32 ").",
                                         location_info->colorAttachmentCount,
                                         unsigned(cb_state.rendering_attachments.color_indexes.size()));
                    } else {
                        for (uint32_t idx = 0; idx < location_info->colorAttachmentCount; idx++) {
                            if (location_info->pColorAttachmentLocations &&
                                location_info->pColorAttachmentLocations[idx] !=
                                    cb_state.rendering_attachments.color_locations[idx]) {
                                skip |= LogError(
                                    vuid_090504, objlist,
                                    cb_loc.pNext(Struct::VkRenderingAttachmentLocationInfo, Field::pColorAttachmentInputIndices,
                                                 idx),
                                    "(%" PRIu32
                                    ") does not match the implicit or explicit state in the primary command buffer (%" PRIu32 ").",
                                    location_info->pColorAttachmentLocations[idx],
                                    cb_state.rendering_attachments.color_locations[idx]);
                            }
                        }
                    }
                }

                const auto *index_info = vku::FindStructInPNextChain<VkRenderingInputAttachmentIndexInfo>(
                    secondary_rp_state->inheritance_rendering_info.pNext);

                if (index_info) {
                    const std::string vuid_090505 = "VUID-vkCmdExecuteCommands-pCommandBuffers-09505";
                    const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
                    skip |= ValidateRenderingInputAttachmentIndices(*index_info, objlist, cb_loc.dot(Field::pNext));

                    if (index_info->colorAttachmentCount != cb_state.rendering_attachments.color_indexes.size()) {
                        skip |= LogError(vuid_090505, objlist,
                                         cb_loc.pNext(Struct::VkRenderingInputAttachmentIndexInfo, Field::colorAttachmentCount),
                                         "(%" PRIu32
                                         ") does not match the implicit or explicit state in the primary command buffer ("
                                         "%" PRIu32 ").",
                                         index_info->colorAttachmentCount,
                                         unsigned(cb_state.rendering_attachments.color_indexes.size()));
                    } else {
                        for (uint32_t idx = 0; idx < index_info->colorAttachmentCount; idx++) {
                            if (index_info->pColorAttachmentInputIndices && cb_state.rendering_attachments.color_indexes[idx] !=
                                                                                index_info->pColorAttachmentInputIndices[idx]) {
                                skip |= LogError(vuid_090505, objlist,
                                                 cb_loc.pNext(Struct::VkRenderingInputAttachmentIndexInfo,
                                                              Field::pColorAttachmentInputIndices, idx),
                                                 "(%" PRIu32
                                                 ") does not match the implicit or explicit state in the primary command "
                                                 "buffer (%" PRIu32 ").",
                                                 index_info->pColorAttachmentInputIndices[idx],
                                                 cb_state.rendering_attachments.color_indexes[idx]);
                            }
                        }
                    }

                    if (cb_state.rendering_attachments.depth_index && index_info->pDepthInputAttachmentIndex &&
                        *cb_state.rendering_attachments.depth_index != *index_info->pDepthInputAttachmentIndex) {
                        skip |=
                            LogError(vuid_090505, objlist,
                                     cb_loc.pNext(Struct::VkRenderingInputAttachmentIndexInfo, Field::pDepthInputAttachmentIndex),
                                     "(%" PRIu32
                                     ") does not match the implicit or explicit state in the primary command buffer ("
                                     "%" PRIu32 ").",
                                     *index_info->pDepthInputAttachmentIndex, *cb_state.rendering_attachments.depth_index);
                    }

                    if (cb_state.rendering_attachments.stencil_index && index_info->pStencilInputAttachmentIndex &&
                        *cb_state.rendering_attachments.stencil_index != *index_info->pStencilInputAttachmentIndex) {
                        skip |=
                            LogError(vuid_090505, objlist,
                                     cb_loc.pNext(Struct::VkRenderingInputAttachmentIndexInfo, Field::pStencilInputAttachmentIndex),
                                     "(%" PRIu32
                                     ") does not match the implicit or explicit state in the primary command buffer"
                                     "(%" PRIu32 ").",
                                     *index_info->pStencilInputAttachmentIndex, *cb_state.rendering_attachments.stencil_index);
                    }
                }
            }
            if (sub_cb_state.beginInfo.pInheritanceInfo != nullptr) {
                const uint32_t inheritance_subpass = sub_cb_state.beginInfo.pInheritanceInfo->subpass;
                const VkRenderPass inheritance_render_pass = sub_cb_state.beginInfo.pInheritanceInfo->renderPass;
                if (!(sub_cb_state.beginInfo.flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT)) {
                    const LogObjectList objlist(pCommandBuffers[i], rp_state->Handle());
                    skip |= LogError("VUID-vkCmdExecuteCommands-pCommandBuffers-00096", objlist, cb_loc,
                                     "(%s) is executed within a %s "
                                     "instance scope, but the Secondary Command Buffer does not have the "
                                     "VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT set in VkCommandBufferBeginInfo::flags when "
                                     "the vkBeginCommandBuffer() was called.",
                                     FormatHandle(pCommandBuffers[i]).c_str(), FormatHandle(rp_state->Handle()).c_str());
                } else if (sub_cb_state.beginInfo.flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT) {
                    if (!rp_state->UsesDynamicRendering()) {
                        // Make sure render pass is compatible with parent command buffer pass if secondary command buffer has
                        // "render pass continue" usage flag
                        auto inherit_rp_state = Get<vvl::RenderPass>(inheritance_render_pass);
                        if (inherit_rp_state && (rp_state->VkHandle() != inherit_rp_state->VkHandle())) {
                            skip |= ValidateRenderPassCompatibility(cb_state.Handle(), *rp_state, inherit_rp_state->Handle(),
                                                                    *inherit_rp_state.get(), cb_loc,
                                                                    "VUID-vkCmdExecuteCommands-pBeginInfo-06020");
                        }
                        //  If framebuffer for secondary CB is not NULL, then it must match active FB from primaryCB
                        skip |= ValidateInheritanceInfoFramebuffer(commandBuffer, cb_state, pCommandBuffers[i], sub_cb_state,
                                                                   error_obj.location);
                    }
                    // Inherit primary's activeFramebuffer, or null if using dynamic rendering,
                    // and while running validate functions
                    for (auto &function : sub_cb_state.cmd_execute_commands_functions) {
                        skip |= function(sub_cb_state, &cb_state, cb_state.activeFramebuffer.get());
                    }
                }

                if ((sub_cb_state.beginInfo.flags & VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT) &&
                    !enabled_features.nestedCommandBufferSimultaneousUse) {
                    skip |=
                        LogError("VUID-vkCmdExecuteCommands-nestedCommandBufferSimultaneousUse-09378", pCommandBuffers[i], cb_loc,
                                 "(%s) was recorded with VkCommandBufferBeginInfo::flag including "
                                 "VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, but the nestedCommandBufferSimultaneousUse feature "
                                 "was not enabled.",
                                 FormatHandle(pCommandBuffers[i]).c_str());
                }

                if (!rp_state->UsesDynamicRendering() && (cb_state.GetActiveSubpass() != inheritance_subpass)) {
                    const LogObjectList objlist(commandBuffer, pCommandBuffers[i], rp_state->Handle());
                    skip |= LogError("VUID-vkCmdExecuteCommands-pCommandBuffers-06019", objlist, cb_loc,
                                     "(%s) is executed within a %s "
                                     "instance scope begun by vkCmdBeginRenderPass(), but "
                                     "VkCommandBufferInheritanceInfo::subpass (%" PRIu32
                                     ") does not "
                                     "match the current subpass (%" PRIu32 ").",
                                     FormatHandle(pCommandBuffers[i]).c_str(), FormatHandle(rp_state->Handle()).c_str(),
                                     inheritance_subpass, cb_state.GetActiveSubpass());
                } else if (rp_state->UsesDynamicRendering()) {
                    if (inheritance_render_pass != VK_NULL_HANDLE) {
                        const LogObjectList objlist(commandBuffer, pCommandBuffers[i], rp_state->Handle());
                        skip |= LogError("VUID-vkCmdExecuteCommands-pBeginInfo-06025", objlist, cb_loc,
                                         "(%s) is executed within a dynamic renderpass instance scope begun "
                                         "by vkCmdBeginRendering(), but "
                                         "VkCommandBufferInheritanceInfo::pInheritanceInfo::renderPass is not VK_NULL_HANDLE.",
                                         FormatHandle(pCommandBuffers[i]).c_str());
                    }

                    if (rp_state->use_dynamic_rendering && secondary_rp_state &&
                        secondary_rp_state->use_dynamic_rendering_inherited) {
                        const auto rendering_info = rp_state->dynamic_rendering_begin_rendering_info;
                        const auto inheritance_rendering_info = secondary_rp_state->inheritance_rendering_info;
                        if ((inheritance_rendering_info.flags &
                             ~(VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT | VK_RENDERING_CONTENTS_INLINE_BIT_KHR)) !=
                            (rendering_info.flags &
                             ~(VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT | VK_RENDERING_CONTENTS_INLINE_BIT_KHR))) {
                            const LogObjectList objlist(commandBuffer, pCommandBuffers[i], rp_state->Handle());
                            skip |=
                                LogError("VUID-vkCmdExecuteCommands-flags-06026", objlist, cb_loc,
                                         "(%s) is executed within a dynamic renderpass instance scope begun "
                                         "by vkCmdBeginRendering(), but VkCommandBufferInheritanceRenderingInfo::flags (%s) does "
                                         "not match VkRenderingInfo::flags (%s) (excluding "
                                         "VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT or "
                                         "VK_RENDERING_CONTENTS_INLINE_BIT_KHR).",
                                         FormatHandle(pCommandBuffers[i]).c_str(),
                                         string_VkRenderingFlags(inheritance_rendering_info.flags).c_str(),
                                         string_VkRenderingFlags(rendering_info.flags).c_str());
                        }

                        if (inheritance_rendering_info.colorAttachmentCount != rendering_info.colorAttachmentCount) {
                            const LogObjectList objlist(commandBuffer, pCommandBuffers[i], rp_state->Handle());
                            skip |= LogError("VUID-vkCmdExecuteCommands-colorAttachmentCount-06027", objlist, cb_loc,
                                             "(%s) is executed within a dynamic renderpass instance scope begun "
                                             "by vkCmdBeginRendering(), but "
                                             "VkCommandBufferInheritanceRenderingInfo::colorAttachmentCount (%" PRIu32
                                             ") does "
                                             "not match VkRenderingInfo::colorAttachmentCount (%" PRIu32 ").",
                                             FormatHandle(pCommandBuffers[i]).c_str(),
                                             inheritance_rendering_info.colorAttachmentCount, rendering_info.colorAttachmentCount);
                        }

                        for (uint32_t color_i = 0, count = std::min(inheritance_rendering_info.colorAttachmentCount,
                                                                    rendering_info.colorAttachmentCount);
                             color_i < count; color_i++) {
                            if (rendering_info.pColorAttachments[color_i].imageView == VK_NULL_HANDLE) {
                                if (inheritance_rendering_info.pColorAttachmentFormats[color_i] != VK_FORMAT_UNDEFINED) {
                                    const LogObjectList objlist(commandBuffer, pCommandBuffers[i], rp_state->Handle());
                                    skip |= LogError("VUID-vkCmdExecuteCommands-imageView-07606", objlist, cb_loc,
                                                     "(%s) is executed within a dynamic render pass instance "
                                                     "scope begun "
                                                     "by vkCmdBeginRendering(), VkRenderingInfo::pColorAttachments[%" PRIu32
                                                     "].imageView is VK_NULL_HANDLE but "
                                                     "VkCommandBufferInheritanceRenderingInfo::pColorAttachmentFormats[%" PRIu32
                                                     "] is %s.",
                                                     FormatHandle(pCommandBuffers[i]).c_str(), color_i, color_i,
                                                     string_VkFormat(inheritance_rendering_info.pColorAttachmentFormats[color_i]));
                                }
                            } else {
                                auto image_view_state = Get<vvl::ImageView>(rendering_info.pColorAttachments[color_i].imageView);

                                if (image_view_state && image_view_state->create_info.format !=
                                                            inheritance_rendering_info.pColorAttachmentFormats[color_i]) {
                                    const LogObjectList objlist(commandBuffer, pCommandBuffers[i], rp_state->Handle());
                                    skip |= LogError("VUID-vkCmdExecuteCommands-imageView-06028", objlist, cb_loc,
                                                     "(%s) is executed within a dynamic render pass instance "
                                                     "scope begun "
                                                     "by vkCmdBeginRendering(), VkRenderingInfo::pColorAttachments[%" PRIu32
                                                     "].imageView format is %s but "
                                                     "VkCommandBufferInheritanceRenderingInfo::pColorAttachmentFormats[%" PRIu32
                                                     "] is %s.",
                                                     FormatHandle(pCommandBuffers[i]).c_str(), color_i,
                                                     string_VkFormat(image_view_state->create_info.format), color_i,
                                                     string_VkFormat(inheritance_rendering_info.pColorAttachmentFormats[color_i]));
                                }
                            }
                        }

                        if ((rendering_info.pDepthAttachment != nullptr) &&
                            rendering_info.pDepthAttachment->imageView != VK_NULL_HANDLE) {
                            auto image_view_state = Get<vvl::ImageView>(rendering_info.pDepthAttachment->imageView);

                            if (image_view_state &&
                                image_view_state->create_info.format != inheritance_rendering_info.depthAttachmentFormat) {
                                const LogObjectList objlist(commandBuffer, pCommandBuffers[i], rp_state->Handle());
                                skip |= LogError("VUID-vkCmdExecuteCommands-pDepthAttachment-06029", objlist, cb_loc,
                                                 "(%s) is executed within a dynamic renderpass "
                                                 "instance scope begun "
                                                 "by vkCmdBeginRendering(), but "
                                                 "VkCommandBufferInheritanceRenderingInfo::depthAttachmentFormat does "
                                                 "not match the format of the imageView in VkRenderingInfo::pDepthAttachment.",
                                                 FormatHandle(pCommandBuffers[i]).c_str());
                            }
                        }

                        if ((rendering_info.pStencilAttachment != nullptr) &&
                            rendering_info.pStencilAttachment->imageView != VK_NULL_HANDLE) {
                            auto image_view_state = Get<vvl::ImageView>(rendering_info.pStencilAttachment->imageView);

                            if (image_view_state &&
                                image_view_state->create_info.format != inheritance_rendering_info.stencilAttachmentFormat) {
                                const LogObjectList objlist(commandBuffer, pCommandBuffers[i], rp_state->Handle());
                                skip |= LogError("VUID-vkCmdExecuteCommands-pStencilAttachment-06030", objlist, cb_loc,
                                                 "(%s) is executed within a dynamic renderpass "
                                                 "instance scope begun "
                                                 "by vkCmdBeginRendering(), but "
                                                 "VkCommandBufferInheritanceRenderingInfo::stencilAttachmentFormat does "
                                                 "not match the format of the imageView in VkRenderingInfo::pStencilAttachment.",
                                                 FormatHandle(pCommandBuffers[i]).c_str());
                            }
                        }

                        if (rendering_info.pDepthAttachment == nullptr ||
                            rendering_info.pDepthAttachment->imageView == VK_NULL_HANDLE) {
                            VkFormat format = inheritance_rendering_info.depthAttachmentFormat;
                            if (format != VK_FORMAT_UNDEFINED) {
                                const LogObjectList objlist(commandBuffer, pCommandBuffers[i], rp_state->Handle());
                                skip |= LogError("VUID-vkCmdExecuteCommands-pDepthAttachment-06774", objlist, cb_loc,
                                                 "(%s) is executed within a dynamic renderpass "
                                                 "instance scope begun by vkCmdBeginRendering(), and "
                                                 "VkRenderingInfo::pDepthAttachment does not define an "
                                                 "image view but VkCommandBufferInheritanceRenderingInfo::depthAttachmentFormat "
                                                 "is %s instead of VK_FORMAT_UNDEFINED.",
                                                 FormatHandle(pCommandBuffers[i]).c_str(), string_VkFormat(format));
                            }
                        }

                        if (rendering_info.pStencilAttachment == nullptr ||
                            rendering_info.pStencilAttachment->imageView == VK_NULL_HANDLE) {
                            VkFormat format = inheritance_rendering_info.stencilAttachmentFormat;
                            if (format != VK_FORMAT_UNDEFINED) {
                                const LogObjectList objlist(commandBuffer, pCommandBuffers[i], rp_state->Handle());
                                skip |= LogError("VUID-vkCmdExecuteCommands-pStencilAttachment-06775", objlist, cb_loc,
                                                 "(%s) is executed within a dynamic renderpass "
                                                 "instance scope begun by vkCmdBeginRendering(), and "
                                                 "VkRenderingInfo::pStencilAttachment does not define an "
                                                 "image view but VkCommandBufferInheritanceRenderingInfo::stencilAttachmentFormat "
                                                 "is %s instead of VK_FORMAT_UNDEFINED.",
                                                 FormatHandle(pCommandBuffers[i]).c_str(), string_VkFormat(format));
                            }
                        }

                        if (rendering_info.viewMask != inheritance_rendering_info.viewMask) {
                            const LogObjectList objlist(commandBuffer, pCommandBuffers[i], rp_state->Handle());
                            skip |= LogError("VUID-vkCmdExecuteCommands-viewMask-06031", objlist, cb_loc,
                                             "(%s) is executed within a dynamic renderpass instance scope begun "
                                             "by vkCmdBeginRendering(), but "
                                             "VkCommandBufferInheritanceRenderingInfo::viewMask (%" PRIu32
                                             ") does "
                                             "not match VkRenderingInfo::viewMask (%" PRIu32 ").",
                                             FormatHandle(pCommandBuffers[i]).c_str(), inheritance_rendering_info.viewMask,
                                             rendering_info.viewMask);
                        }

                        // VkAttachmentSampleCountInfoAMD == VkAttachmentSampleCountInfoNV
                        const auto amd_sample_count =
                            vku::FindStructInPNextChain<VkAttachmentSampleCountInfoAMD>(inheritance_rendering_info.pNext);

                        if (amd_sample_count) {
                            for (uint32_t index = 0; index < rendering_info.colorAttachmentCount; index++) {
                                if (rendering_info.pColorAttachments[index].imageView == VK_NULL_HANDLE) {
                                    continue;
                                }
                                auto image_view_state = Get<vvl::ImageView>(rendering_info.pColorAttachments[index].imageView);

                                if (image_view_state &&
                                    image_view_state->samples != amd_sample_count->pColorAttachmentSamples[index]) {
                                    const LogObjectList objlist(commandBuffer, pCommandBuffers[i], rp_state->Handle());
                                    skip |= LogError(
                                        "VUID-vkCmdExecuteCommands-pNext-06032", objlist, cb_loc,
                                        "(%s) is executed within a dynamic renderpass instance "
                                        "scope begun "
                                        "by vkCmdBeginRenderingKHR(), but "
                                        "VkAttachmentSampleCountInfo(AMD/NV)::pColorAttachmentSamples at index (%" PRIu32
                                        ") "
                                        "does "
                                        "not match the sample count of the imageView in VkRenderingInfo::pColorAttachments.",
                                        FormatHandle(pCommandBuffers[i]).c_str(), index);
                                }
                            }

                            if ((rendering_info.pDepthAttachment != nullptr) &&
                                rendering_info.pDepthAttachment->imageView != VK_NULL_HANDLE) {
                                auto image_view_state = Get<vvl::ImageView>(rendering_info.pDepthAttachment->imageView);

                                if (image_view_state &&
                                    image_view_state->samples != amd_sample_count->depthStencilAttachmentSamples) {
                                    const LogObjectList objlist(commandBuffer, pCommandBuffers[i], rp_state->Handle());
                                    skip |= LogError(
                                        "VUID-vkCmdExecuteCommands-pNext-06033", objlist, cb_loc,
                                        "(%s) is executed within a dynamic renderpass instance "
                                        "scope begun "
                                        "by vkCmdBeginRenderingKHR(), but "
                                        "VkAttachmentSampleCountInfo(AMD/NV)::depthStencilAttachmentSamples does "
                                        "not match the sample count of the imageView in VkRenderingInfo::pDepthAttachment.",
                                        FormatHandle(pCommandBuffers[i]).c_str());
                                }
                            }

                            if ((rendering_info.pStencilAttachment != nullptr) &&
                                rendering_info.pStencilAttachment->imageView != VK_NULL_HANDLE) {
                                auto image_view_state = Get<vvl::ImageView>(rendering_info.pStencilAttachment->imageView);

                                if (image_view_state &&
                                    image_view_state->samples != amd_sample_count->depthStencilAttachmentSamples) {
                                    const LogObjectList objlist(commandBuffer, pCommandBuffers[i], rp_state->Handle());
                                    skip |= LogError(
                                        "VUID-vkCmdExecuteCommands-pNext-06034", objlist, cb_loc,
                                        "(%s) is executed within a dynamic renderpass instance "
                                        "scope begun "
                                        "by vkCmdBeginRenderingKHR(), but "
                                        "VkAttachmentSampleCountInfo(AMD/NV)::depthStencilAttachmentSamples does "
                                        "not match the sample count of the imageView in VkRenderingInfo::pStencilAttachment.",
                                        FormatHandle(pCommandBuffers[i]).c_str());
                                }
                            }
                        } else {
                            for (uint32_t index = 0; index < rendering_info.colorAttachmentCount; index++) {
                                if (rendering_info.pColorAttachments[index].imageView == VK_NULL_HANDLE) {
                                    continue;
                                }
                                auto image_view_state = Get<vvl::ImageView>(rendering_info.pColorAttachments[index].imageView);

                                if (image_view_state &&
                                    image_view_state->samples != inheritance_rendering_info.rasterizationSamples) {
                                    const LogObjectList objlist(commandBuffer, pCommandBuffers[i], rp_state->Handle());
                                    skip |= LogError(
                                        "VUID-vkCmdExecuteCommands-pNext-06035", objlist, cb_loc,
                                        "(%s) is executed within a dynamic renderpass instance "
                                        "scope begun "
                                        "by vkCmdBeginRenderingKHR(), but the sample count of the image view at index (%" PRIu32
                                        ") of "
                                        "VkRenderingInfo::pColorAttachments does not match "
                                        "VkCommandBufferInheritanceRenderingInfo::rasterizationSamples.",
                                        FormatHandle(pCommandBuffers[i]).c_str(), index);
                                }
                            }

                            if ((rendering_info.pDepthAttachment != nullptr) &&
                                rendering_info.pDepthAttachment->imageView != VK_NULL_HANDLE) {
                                auto image_view_state = Get<vvl::ImageView>(rendering_info.pDepthAttachment->imageView);

                                if (image_view_state &&
                                    image_view_state->samples != inheritance_rendering_info.rasterizationSamples) {
                                    const LogObjectList objlist(commandBuffer, pCommandBuffers[i], rp_state->Handle());
                                    skip |= LogError("VUID-vkCmdExecuteCommands-pNext-06036", objlist, cb_loc,
                                                     "(%s) is executed within a dynamic renderpass "
                                                     "instance scope begun "
                                                     "by vkCmdBeginRenderingKHR(), but the sample count of the image view for "
                                                     "VkRenderingInfo::pDepthAttachment does not match "
                                                     "VkCommandBufferInheritanceRenderingInfo::rasterizationSamples.",
                                                     FormatHandle(pCommandBuffers[i]).c_str());
                                }
                            }

                            if ((rendering_info.pStencilAttachment != nullptr) &&
                                rendering_info.pStencilAttachment->imageView != VK_NULL_HANDLE) {
                                auto image_view_state = Get<vvl::ImageView>(rendering_info.pStencilAttachment->imageView);

                                if (image_view_state &&
                                    image_view_state->samples != inheritance_rendering_info.rasterizationSamples) {
                                    const LogObjectList objlist(commandBuffer, pCommandBuffers[i], rp_state->Handle());
                                    skip |= LogError("VUID-vkCmdExecuteCommands-pNext-06037", objlist, cb_loc,
                                                     "(%s) is executed within a dynamic renderpass "
                                                     "instance scope begun "
                                                     "by vkCmdBeginRenderingKHR(), but the sample count of the image view for "
                                                     "VkRenderingInfo::pStencilAttachment does not match "
                                                     "VkCommandBufferInheritanceRenderingInfo::rasterizationSamples.",
                                                     FormatHandle(pCommandBuffers[i]).c_str());
                                }
                            }
                        }
                    }
                }

                // spec: "A maxCommandBufferNestingLevel of UINT32_MAX means there is no limit to the nesting level"
                if (enabled_features.nestedCommandBuffer && cb_state.IsSecondary() &&
                    phys_dev_ext_props.nested_command_buffer_props.maxCommandBufferNestingLevel != UINT32_MAX) {
                    if (sub_cb_state.nesting_level >= phys_dev_ext_props.nested_command_buffer_props.maxCommandBufferNestingLevel) {
                        skip |= LogError("VUID-vkCmdExecuteCommands-nestedCommandBuffer-09376", pCommandBuffers[i], cb_loc,
                                         "(%s) has a nesting level of %" PRIu32
                                         " which is not less then maxCommandBufferNestingLevel (%" PRIu32 ").",
                                         FormatHandle(pCommandBuffers[i]).c_str(), sub_cb_state.nesting_level,
                                         phys_dev_ext_props.nested_command_buffer_props.maxCommandBufferNestingLevel);
                    }
                }
            }
        }

        // TODO(mlentine): Move more logic into this method
        skip |= ValidateSecondaryCommandBufferState(cb_state, sub_cb_state, cb_loc);
        skip |= ValidateCommandBufferState(sub_cb_state, cb_loc, 0, "VUID-vkCmdExecuteCommands-pCommandBuffers-00089");
        if (!(sub_cb_state.beginInfo.flags & VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT)) {
            if (sub_cb_state.InUse()) {
                const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
                skip |= LogError("VUID-vkCmdExecuteCommands-pCommandBuffers-00091", objlist, cb_loc,
                                 "Cannot execute pending %s without VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT set.",
                                 FormatHandle(pCommandBuffers[i]).c_str());
            }
            // We use an const_cast, because one cannot query a container keyed on a non-const pointer using a const pointer
            if (cb_state.linkedCommandBuffers.count(const_cast<vvl::CommandBuffer *>(&sub_cb_state))) {
                const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
                skip |= LogError("VUID-vkCmdExecuteCommands-pCommandBuffers-00092", objlist, cb_loc,
                                 "Cannot execute %s without VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT "
                                 "set if previously executed in %s",
                                 FormatHandle(pCommandBuffers[i]).c_str(), FormatHandle(commandBuffer).c_str());
            }

            const auto insert_pair = linked_command_buffers.insert(&sub_cb_state);
            if (!insert_pair.second) {
                const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
                skip |= LogError("VUID-vkCmdExecuteCommands-pCommandBuffers-00093", objlist, cb_loc,
                                 "Cannot duplicate %s in pCommandBuffers without "
                                 "VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT set.",
                                 FormatHandle(commandBuffer).c_str());
            }
        }
        if (!cb_state.activeQueries.empty() && !enabled_features.inheritedQueries) {
            const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
            skip |= LogError("VUID-vkCmdExecuteCommands-commandBuffer-00101", objlist, cb_loc,
                             "cannot be submitted with a query in flight and "
                             "inherited queries not supported on this device.");
        }
        // Validate initial layout uses vs. the primary cmd buffer state
        // Novel Valid usage: "UNASSIGNED-vkCmdExecuteCommands-commandBuffer-00001"
        // initial layout usage of secondary command buffers resources must match parent command buffer
        for (const auto &sub_layout_map_entry : sub_cb_state.image_layout_map) {
            const VkImage image = sub_layout_map_entry.first;

            const auto cb_image_layout_registry = cb_state.GetImageLayoutRegistry(image);
            // Const getter can be null in which case we have nothing to check against for this image...
            if (!cb_image_layout_registry) continue;
            if (!sub_layout_map_entry.second) continue;

            const auto &sub_layout_map = sub_layout_map_entry.second->GetLayoutMap();
            const auto &cb_layout_map = cb_image_layout_registry->GetLayoutMap();
            for (sparse_container::parallel_iterator<const ImageLayoutRegistry::LayoutMap> iter(sub_layout_map, cb_layout_map, 0);
                 !iter->range.empty(); ++iter) {
                VkImageLayout cb_layout = kInvalidLayout, sub_layout = kInvalidLayout;
                const char *layout_type;

                if (!iter->pos_A->valid || !iter->pos_B->valid) continue;

                // pos_A denotes the sub CB map in the parallel iterator
                sub_layout = iter->pos_A->lower_bound->second.initial_layout;
                if (VK_IMAGE_LAYOUT_UNDEFINED == sub_layout) continue;  // secondary doesn't care about current or initial

                // pos_B denotes the main CB map in the parallel iterator
                const auto &cb_layout_state = iter->pos_B->lower_bound->second;
                if (cb_layout_state.current_layout != kInvalidLayout) {
                    layout_type = "current";
                    cb_layout = cb_layout_state.current_layout;
                } else if (cb_layout_state.initial_layout != kInvalidLayout) {
                    layout_type = "initial";
                    cb_layout = cb_layout_state.initial_layout;
                } else {
                    continue;
                }
                if (sub_layout != cb_layout) {
                    // We can report all the errors for the intersected range directly
                    for (auto index = iter->range.begin; index < iter->range.end; index++) {
                        const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
                        const auto image_state = Get<vvl::Image>(image);
                        if (!image_state) continue;
                        const auto subresource = image_state->subresource_encoder.Decode(index);
                        // VU being worked on https://gitlab.khronos.org/vulkan/vulkan/-/issues/2456
                        skip |= LogError("UNASSIGNED-vkCmdExecuteCommands-commandBuffer-00001", objlist, cb_loc,
                                         "was executed using %s (subresource: aspectMask %s, array layer %" PRIu32
                                         ", mip level %" PRIu32 ") which expects layout %s--instead, image %s layout is %s.",
                                         FormatHandle(image).c_str(), string_VkImageAspectFlags(subresource.aspectMask).c_str(),
                                         subresource.arrayLayer, subresource.mipLevel, string_VkImageLayout(sub_layout),
                                         layout_type, string_VkImageLayout(cb_layout));
                    }
                }
            }
        }

        // All commands buffers involved must be protected or unprotected
        if ((cb_state.unprotected == false) && (sub_cb_state.unprotected == true)) {
            const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
            skip |= LogError("VUID-vkCmdExecuteCommands-commandBuffer-01820", objlist, cb_loc,
                             "(%s) is a unprotected while primary command buffer (%s) is protected.",
                             FormatHandle(pCommandBuffers[i]).c_str(), FormatHandle(commandBuffer).c_str());
        } else if ((cb_state.unprotected == true) && (sub_cb_state.unprotected == false)) {
            const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
            skip |= LogError("VUID-vkCmdExecuteCommands-commandBuffer-01821", objlist, cb_loc,
                             "(%s) is a protected while primary command buffer (%s) is unprotected.",
                             FormatHandle(pCommandBuffers[i]).c_str(), FormatHandle(commandBuffer).c_str());
        }
        if (active_occlusion_query) {
            if (sub_cb_state.inheritanceInfo.occlusionQueryEnable != VK_TRUE) {
                const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
                skip |= LogError("VUID-vkCmdExecuteCommands-commandBuffer-00102", objlist, cb_loc,
                                 "(%s) was recorded with VkCommandBufferInheritanceInfo::occlusionQueryEnable set to VK_FALSE, but "
                                 "primary command buffer %s has an active occlusion query",
                                 FormatHandle(pCommandBuffers[i]).c_str(), FormatHandle(commandBuffer).c_str());
            }
            if ((sub_cb_state.inheritanceInfo.queryFlags & active_occlusion_query->control_flags) !=
                active_occlusion_query->control_flags) {
                const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
                skip |= LogError("VUID-vkCmdExecuteCommands-commandBuffer-00103", objlist, cb_loc,
                                 "(%s) was recorded with VkCommandBufferInheritanceInfo::queryFlags %s, but primary command buffer "
                                 "%s has an active occlusion query with VkQueryControlFlags %s.",
                                 FormatHandle(pCommandBuffers[i]).c_str(),
                                 string_VkQueryControlFlags(sub_cb_state.inheritanceInfo.queryFlags).c_str(),
                                 FormatHandle(commandBuffer).c_str(),
                                 string_VkQueryControlFlags(active_occlusion_query->control_flags).c_str());
            }
        }
    }

    if (cb_state.transform_feedback_active) {
        skip |=
            LogError("VUID-vkCmdExecuteCommands-None-02286", commandBuffer, error_obj.location, "transform feedback is active.");
    }

    skip |= ValidateCmd(cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT *pMarkerInfo,
                                                       const ErrorObject &error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateCmd(*cb_state, error_obj.location);
}

bool CoreChecks::PreCallValidateCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer, const ErrorObject &error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateCmd(*cb_state, error_obj.location);
}

bool CoreChecks::ValidateCmdDrawStrideWithStruct(const vvl::CommandBuffer &cb_state, const std::string &vuid, const uint32_t stride,
                                                 Struct struct_name, const uint32_t struct_size, const Location &loc) const {
    bool skip = false;
    static const int condition_multiples = 0b0011;
    if ((stride & condition_multiples) || (stride < struct_size)) {
        skip |= LogError(vuid, cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), loc.dot(Field::stride),
                         "%" PRIu32 " is invalid or less than sizeof(%s) %" PRIu32 ".", stride, String(struct_name), struct_size);
    }
    return skip;
}

bool CoreChecks::ValidateCmdDrawStrideWithBuffer(const vvl::CommandBuffer &cb_state, const std::string &vuid, const uint32_t stride,
                                                 Struct struct_name, const uint32_t struct_size, const uint32_t drawCount,
                                                 const VkDeviceSize offset, const vvl::Buffer &buffer_state,
                                                 const Location &loc) const {
    bool skip = false;
    uint64_t validation_value = stride * (drawCount - 1) + offset + struct_size;
    if (validation_value > buffer_state.create_info.size) {
        LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS);
        objlist.add(buffer_state.Handle());
        skip |=
            LogError(vuid, objlist, loc,
                     "stride (%" PRIu32 ") * [drawCount (%" PRIu32 ") - 1] + offset (%" PRIu64 ") + sizeof(%s) (%" PRIu32
                     ") is %" PRIu64 ", which is greater than the buffer size (%" PRIu64 ").",
                     stride, drawCount, offset, String(struct_name), struct_size, validation_value, buffer_state.create_info.size);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                                   uint32_t bindingCount, const VkBuffer *pBuffers,
                                                                   const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes,
                                                                   const ErrorObject &error_obj) const {
    bool skip = false;

    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    if (cb_state->transform_feedback_active) {
        skip |= LogError("VUID-vkCmdBindTransformFeedbackBuffersEXT-None-02365", commandBuffer, error_obj.location,
                         "transform feedback is active.");
    }

    for (uint32_t i = 0; i < bindingCount; ++i) {
        const Location buffer_loc = error_obj.location.dot(Field::pBuffers, i);
        auto buffer_state = Get<vvl::Buffer>(pBuffers[i]);
        ASSERT_AND_CONTINUE(buffer_state);

        if (pOffsets[i] >= buffer_state->create_info.size) {
            const LogObjectList objlist(commandBuffer, pBuffers[i]);
            skip |= LogError("VUID-vkCmdBindTransformFeedbackBuffersEXT-pOffsets-02358", objlist,
                             error_obj.location.dot(Field::pOffsets, i),
                             "(%" PRIu64 ") is greater than or equal to the size of pBuffers[%" PRIu32 "] (%" PRIu64 ").",
                             pOffsets[i], i, buffer_state->create_info.size);
        }

        if ((buffer_state->usage & VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT) == 0) {
            const LogObjectList objlist(commandBuffer, pBuffers[i]);
            skip |= LogError("VUID-vkCmdBindTransformFeedbackBuffersEXT-pBuffers-02360", objlist, buffer_loc,
                             "was created with %s.", string_VkBufferUsageFlags2(buffer_state->usage).c_str());
        }

        // pSizes is optional and may be nullptr. Also might be VK_WHOLE_SIZE which VU don't apply
        if ((pSizes != nullptr) && (pSizes[i] != VK_WHOLE_SIZE)) {
            // only report one to prevent redundant error if the size is larger since adding offset will be as well
            if (pSizes[i] > buffer_state->create_info.size) {
                const LogObjectList objlist(commandBuffer, pBuffers[i]);
                skip |= LogError("VUID-vkCmdBindTransformFeedbackBuffersEXT-pSizes-02362", objlist,
                                 error_obj.location.dot(Field::pSizes, i),
                                 "(%" PRIu64 ") is greater than the size of pBuffers[%" PRIu32 "](%" PRIu64 ").", pSizes[i], i,
                                 buffer_state->create_info.size);
            } else if (pOffsets[i] + pSizes[i] > buffer_state->create_info.size) {
                const LogObjectList objlist(commandBuffer, pBuffers[i]);
                skip |= LogError("VUID-vkCmdBindTransformFeedbackBuffersEXT-pOffsets-02363", objlist, error_obj.location,
                                 "The sum of pOffsets[%" PRIu32 "] (%" PRIu64 ") and pSizes[%" PRIu32 "] (%" PRIu64
                                 ") is greater than the size of pBuffers[%" PRIu32 "] (%" PRIu64 ").",
                                 i, pOffsets[i], i, pSizes[i], i, buffer_state->create_info.size);
            }
        }

        skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *buffer_state, buffer_loc,
                                              "VUID-vkCmdBindTransformFeedbackBuffersEXT-pBuffers-02364");
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                             uint32_t counterBufferCount, const VkBuffer *pCounterBuffers,
                                                             const VkDeviceSize *pCounterBufferOffsets,
                                                             const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateCmd(*cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    const auto *pipe = cb_state->lastBound[VK_PIPELINE_BIND_POINT_GRAPHICS].pipeline_state;
    if (!pipe && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdBeginTransformFeedbackEXT-None-06233", commandBuffer, error_obj.location,
                         "No graphics pipeline has been bound yet.");
    } else if (pipe && pipe->pre_raster_state) {
        for (const auto &stage_state : pipe->stage_states) {
            if (!stage_state.entrypoint || stage_state.GetStage() != pipe->pre_raster_state->last_stage) {
                continue;
            }
            if (!stage_state.entrypoint->execution_mode.Has(spirv::ExecutionModeSet::xfb_bit)) {
                const LogObjectList objlist(commandBuffer, pipe->Handle());
                skip |= LogError("VUID-vkCmdBeginTransformFeedbackEXT-None-04128", objlist, error_obj.location,
                                 "The last bound pipeline (%s) has no Xfb Execution Mode for stage %s.",
                                 FormatHandle(pipe->Handle()).c_str(),
                                 string_VkShaderStageFlagBits(pipe->pre_raster_state->last_stage));
            }
        }
    }

    if (cb_state->transform_feedback_active) {
        skip |= LogError("VUID-vkCmdBeginTransformFeedbackEXT-None-02367", commandBuffer, error_obj.location,
                         "transform feedback is active.");
    }

    if (cb_state->active_render_pass) {
        const auto &rp_ci = cb_state->active_render_pass->create_info;
        for (uint32_t i = 0; i < rp_ci.subpassCount; ++i) {
            // When a subpass uses a non-zero view mask, multiview functionality is considered to be enabled
            if (rp_ci.pSubpasses[i].viewMask > 0) {
                skip |= LogError("VUID-vkCmdBeginTransformFeedbackEXT-None-02373", commandBuffer, error_obj.location,
                                 "active render pass (%s) has multiview enabled.",
                                 FormatHandle(cb_state->active_render_pass->Handle()).c_str());
                break;
            }
        }
    }

    if ((counterBufferCount + firstCounterBuffer) > cb_state->transform_feedback_buffers_bound) {
        skip |= LogError("VUID-vkCmdBeginTransformFeedbackEXT-firstCounterBuffer-09630", commandBuffer,
                         error_obj.location.dot(Field::firstCounterBuffer),
                         "is %" PRIu32 " and counterBufferCount is %" PRIu32
                         " but vkCmdBindTransformFeedbackBuffersEXT only bound %" PRIu32 " buffers.",
                         firstCounterBuffer, counterBufferCount, cb_state->transform_feedback_buffers_bound);
    }

    // pCounterBuffers and pCounterBufferOffsets are optional and may be nullptr. Additionaly, pCounterBufferOffsets must be nullptr
    // if pCounterBuffers is nullptr.
    if (pCounterBuffers == nullptr) {
        if (pCounterBufferOffsets != nullptr) {
            skip |= LogError("VUID-vkCmdBeginTransformFeedbackEXT-pCounterBuffer-02371", commandBuffer, error_obj.location,
                             "pCounterBuffers is NULL and pCounterBufferOffsets is not NULL.");
        }
    } else {
        for (uint32_t i = 0; i < counterBufferCount; ++i) {
            if (pCounterBuffers[i] == VK_NULL_HANDLE) {
                continue;
            }
            auto buffer_state = Get<vvl::Buffer>(pCounterBuffers[i]);
            ASSERT_AND_CONTINUE(buffer_state);

            if (pCounterBufferOffsets != nullptr && pCounterBufferOffsets[i] + 4 > buffer_state->create_info.size) {
                const LogObjectList objlist(commandBuffer, pCounterBuffers[i]);
                skip |= LogError("VUID-vkCmdBeginTransformFeedbackEXT-pCounterBufferOffsets-02370", objlist,
                                 error_obj.location.dot(Field::pCounterBuffers, i),
                                 "is not large enough to hold 4 bytes at pCounterBufferOffsets[%" PRIu32 "](0x%" PRIx64 ").", i,
                                 pCounterBufferOffsets[i]);
            }

            if ((buffer_state->usage & VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT) == 0) {
                const LogObjectList objlist(commandBuffer, pCounterBuffers[i]);
                skip |= LogError("VUID-vkCmdBeginTransformFeedbackEXT-pCounterBuffers-02372", objlist,
                                 error_obj.location.dot(Field::pCounterBuffers, i), "was created with %s.",
                                 string_VkBufferUsageFlags2(buffer_state->usage).c_str());
            }
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                           uint32_t counterBufferCount, const VkBuffer *pCounterBuffers,
                                                           const VkDeviceSize *pCounterBufferOffsets,
                                                           const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state->transform_feedback_active) {
        skip |= LogError("VUID-vkCmdEndTransformFeedbackEXT-None-02375", commandBuffer, error_obj.location,
                         "transform feedback is not active.");
    }

    if (pCounterBuffers) {
        for (uint32_t i = 0; i < counterBufferCount; ++i) {
            if (pCounterBuffers[i] == VK_NULL_HANDLE) {
                continue;
            }
            auto buffer_state = Get<vvl::Buffer>(pCounterBuffers[i]);
            ASSERT_AND_CONTINUE(buffer_state);

            if (pCounterBufferOffsets != nullptr && pCounterBufferOffsets[i] + 4 > buffer_state->create_info.size) {
                const LogObjectList objlist(commandBuffer, pCounterBuffers[i]);
                skip |= LogError("VUID-vkCmdEndTransformFeedbackEXT-pCounterBufferOffsets-02378", objlist,
                                 error_obj.location.dot(Field::pCounterBuffers, i),
                                 "is not large enough to hold 4 bytes at pCounterBufferOffsets[%" PRIu32 "](0x%" PRIx64 ").", i,
                                 pCounterBufferOffsets[i]);
            }

            if ((buffer_state->usage & VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT) == 0) {
                const LogObjectList objlist(commandBuffer, pCounterBuffers[i]);
                skip |= LogError("VUID-vkCmdEndTransformFeedbackEXT-pCounterBuffers-02380", objlist,
                                 error_obj.location.dot(Field::pCounterBuffers, i), "was created with %s.",
                                 string_VkBufferUsageFlags2(buffer_state->usage).c_str());
            }
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                      const VkBuffer *pBuffers, const VkDeviceSize *pOffsets,
                                                      const VkDeviceSize *pSizes, const VkDeviceSize *pStrides,
                                                      const ErrorObject &error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);

    bool skip = false;
    skip |= ValidateCmd(*cb_state, error_obj.location);
    for (uint32_t i = 0; i < bindingCount; ++i) {
        auto buffer_state = Get<vvl::Buffer>(pBuffers[i]);
        if (!buffer_state) continue;  // if using nullDescriptors

        const LogObjectList objlist(commandBuffer, pBuffers[i]);
        const Location buffer_loc = error_obj.location.dot(Field::pBuffers, i);
        skip |= ValidateBufferUsageFlags(objlist, *buffer_state, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, true,
                                         "VUID-vkCmdBindVertexBuffers2-pBuffers-03359", buffer_loc);
        skip |=
            ValidateMemoryIsBoundToBuffer(commandBuffer, *buffer_state, buffer_loc, "VUID-vkCmdBindVertexBuffers2-pBuffers-03360");

        const VkDeviceSize offset = pOffsets[i];
        if (pSizes) {
            if (offset >= buffer_state->create_info.size) {
                skip |= LogError("VUID-vkCmdBindVertexBuffers2-pOffsets-03357", objlist, error_obj.location.dot(Field::pOffsets, i),
                                 "(0x%" PRIu64 ") is beyond the end of the buffer of size (%" PRIu64 ").", offset,
                                 buffer_state->create_info.size);
            }
            const VkDeviceSize size = pSizes[i];
            if (size == VK_WHOLE_SIZE) {
                if (!enabled_features.maintenance5) {
                    skip |= LogError("VUID-vkCmdBindVertexBuffers2-pSizes-03358", objlist, error_obj.location.dot(Field::pSizes, i),
                                     "is VK_WHOLE_SIZE, which is not valid in this context. This can be fixed by enabling the "
                                     "maintenance5 feature.");
                }
            } else if (offset + size > buffer_state->create_info.size) {
                skip |= LogError("VUID-vkCmdBindVertexBuffers2-pSizes-03358", objlist, error_obj.location.dot(Field::pOffsets, i),
                                 "(%" PRIu64 ") + pSizes[%" PRIu32 "] (%" PRIu64
                                 ") is beyond the end of the buffer of size (%" PRIu64 ").",
                                 offset, i, size, buffer_state->create_info.size);
            }
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                         uint32_t bindingCount, const VkBuffer *pBuffers,
                                                         const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes,
                                                         const VkDeviceSize *pStrides, const ErrorObject &error_obj) const {
    return PreCallValidateCmdBindVertexBuffers2(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides,
                                                error_obj);
}

bool CoreChecks::PreCallValidateCmdBeginConditionalRenderingEXT(
    VkCommandBuffer commandBuffer, const VkConditionalRenderingBeginInfoEXT *pConditionalRenderingBegin,
    const ErrorObject &error_obj) const {
    bool skip = false;

    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    if (cb_state && cb_state->conditional_rendering_active) {
        skip |= LogError("VUID-vkCmdBeginConditionalRenderingEXT-None-01980", commandBuffer, error_obj.location,
                         "Conditional rendering is already active.");
    }

    if (pConditionalRenderingBegin) {
        if (auto buffer_state = Get<vvl::Buffer>(pConditionalRenderingBegin->buffer)) {
            const Location conditional_loc = error_obj.location.dot(Field::pConditionalRenderingBegin);
            skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *buffer_state, conditional_loc.dot(Field::buffer),
                                                  "VUID-VkConditionalRenderingBeginInfoEXT-buffer-01981");

            if ((buffer_state->usage & VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT) == 0) {
                const LogObjectList objlist(commandBuffer, buffer_state->Handle());
                skip |=
                    LogError("VUID-VkConditionalRenderingBeginInfoEXT-buffer-01982", objlist, conditional_loc.dot(Field::buffer),
                             "(%s) was created with %s.", FormatHandle(pConditionalRenderingBegin->buffer).c_str(),
                             string_VkBufferUsageFlags2(buffer_state->usage).c_str());
            }
            if (pConditionalRenderingBegin->offset + 4 > buffer_state->create_info.size) {
                const LogObjectList objlist(commandBuffer, buffer_state->Handle());
                skip |= LogError(
                    "VUID-VkConditionalRenderingBeginInfoEXT-offset-01983", objlist, conditional_loc.dot(Field::offset),
                    "(%" PRIu64 ") + 4 bytes is not less than the size of pConditionalRenderingBegin->buffer (%" PRIu64 ").",
                    pConditionalRenderingBegin->offset, buffer_state->create_info.size);
            }
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer, const ErrorObject &error_obj) const {
    bool skip = false;

    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) {
        return skip;
    }

    if (!cb_state->conditional_rendering_active) {
        skip |= LogError("VUID-vkCmdEndConditionalRenderingEXT-None-01985", commandBuffer, error_obj.location,
                         "Conditional rendering is not active.");
    }
    const bool in_render_pass = cb_state->active_render_pass != nullptr;
    if (!cb_state->conditional_rendering_inside_render_pass && in_render_pass) {
        skip |= LogError("VUID-vkCmdEndConditionalRenderingEXT-None-01986", commandBuffer, error_obj.location,
                         "Conditional rendering was begun outside outside of a render "
                         "pass instance, but a render pass instance is currently active in the command buffer.");
    }
    if (cb_state->conditional_rendering_inside_render_pass && in_render_pass &&
        cb_state->conditional_rendering_subpass != cb_state->GetActiveSubpass()) {
        skip |= LogError("VUID-vkCmdEndConditionalRenderingEXT-None-01987", commandBuffer, error_obj.location,
                         "Conditional rendering was begun in subpass %" PRIu32 ", but the current subpass is %" PRIu32 ".",
                         cb_state->conditional_rendering_subpass, cb_state->GetActiveSubpass());
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView,
                                                          VkImageLayout imageLayout, const ErrorObject &error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;

    skip |= ValidateCmd(*cb_state, error_obj.location);

    if (!enabled_features.shadingRateImage) {
        skip |= LogError("VUID-vkCmdBindShadingRateImageNV-None-02058", commandBuffer, error_obj.location,
                         "The shadingRateImage feature is disabled.");
    }

    if (imageView == VK_NULL_HANDLE) {
        return skip;
    }
    auto view_state = Get<vvl::ImageView>(imageView);
    if (!view_state) {
        const LogObjectList objlist(commandBuffer, imageView);
        skip |= LogError("VUID-vkCmdBindShadingRateImageNV-imageView-02059", objlist, error_obj.location,
                         "If imageView is not VK_NULL_HANDLE, it must be a valid "
                         "VkImageView handle.");
        return skip;
    }
    const auto &ivci = view_state->create_info;
    if (ivci.viewType != VK_IMAGE_VIEW_TYPE_2D && ivci.viewType != VK_IMAGE_VIEW_TYPE_2D_ARRAY) {
        const LogObjectList objlist(commandBuffer, imageView);
        skip |= LogError("VUID-vkCmdBindShadingRateImageNV-imageView-02059", objlist, error_obj.location,
                         "If imageView is not VK_NULL_HANDLE, it must be a valid "
                         "VkImageView handle of type VK_IMAGE_VIEW_TYPE_2D or VK_IMAGE_VIEW_TYPE_2D_ARRAY.");
    }

    if (ivci.format != VK_FORMAT_R8_UINT) {
        const LogObjectList objlist(commandBuffer, imageView);
        skip |= LogError("VUID-vkCmdBindShadingRateImageNV-imageView-02060", objlist, error_obj.location,
                         "If imageView is not VK_NULL_HANDLE, it must have a format of "
                         "VK_FORMAT_R8_UINT but is %s.",
                         string_VkFormat(ivci.format));
    }

    const auto *image_state = view_state->image_state.get();
    auto usage = image_state->create_info.usage;
    if (!(usage & VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV)) {
        const LogObjectList objlist(commandBuffer, imageView);
        skip |= LogError("VUID-vkCmdBindShadingRateImageNV-imageView-02061", objlist, error_obj.location,
                         "If imageView is not VK_NULL_HANDLE, the image must have been "
                         "created with VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV set.");
    }

    // XXX TODO: While the VUID says "each subresource", only the base mip level is
    // actually used. Since we don't have an existing convenience function to iterate
    // over all mip levels, just don't bother with non-base levels.
    const VkImageSubresourceRange &range = view_state->normalized_subresource_range;
    VkImageSubresourceLayers subresource = {range.aspectMask, range.baseMipLevel, range.baseArrayLayer, range.layerCount};

    if (image_state) {
        if (imageLayout != VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV && imageLayout != VK_IMAGE_LAYOUT_GENERAL) {
            const LogObjectList objlist(cb_state->Handle(), image_state->Handle());
            skip |=
                LogError("VUID-vkCmdBindShadingRateImageNV-imageLayout-02063", objlist, error_obj.location.dot(Field::imageView),
                         "(%s) layout is %s.", FormatHandle(image_state->Handle()).c_str(), string_VkImageLayout(imageLayout));
        }
        skip |= VerifyImageLayoutSubresource(*cb_state, *image_state, subresource, imageLayout,
                                             error_obj.location.dot(Field::imageView),
                                             "VUID-vkCmdBindShadingRateImageNV-imageView-02062");
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const ErrorObject &error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;

    if (cb_state->IsPrimary() || enabled_features.nestedCommandBuffer) {
        return skip;
    }

    if (cb_state->GetLabelStackDepth() < 1) {
        skip |= LogError("VUID-vkCmdEndDebugUtilsLabelEXT-commandBuffer-01913", commandBuffer, error_obj.location,
                         "called without a corresponding vkCmdBeginDebugUtilsLabelEXT first");
    }
    return skip;
}

bool CoreChecks::ValidateVkConvertCooperativeVectorMatrixInfoNV(const LogObjectList &objlist,
                                                                const VkConvertCooperativeVectorMatrixInfoNV &info,
                                                                const Location &info_loc) const {
    bool skip = false;

    auto const supported_matrix_type = [&](VkComponentTypeKHR component_type) {
        if (component_type == VK_COMPONENT_TYPE_FLOAT32_KHR) {
            return true;
        }
        for (size_t i = 0; i < cooperative_vector_properties_nv.size(); ++i) {
            if (cooperative_vector_properties_nv[i].matrixInterpretation == component_type) {
                return true;
            }
        }
        return false;
    };

    if (!supported_matrix_type(info.srcComponentType)) {
        skip |= LogError("VUID-VkConvertCooperativeVectorMatrixInfoNV-srcComponentType-10079", objlist,
                         info_loc.dot(Field::srcComponentType), "(%s) must be float32 or a supported matrixInterpretation",
                         string_VkComponentTypeKHR(info.srcComponentType));
    }
    if (!supported_matrix_type(info.dstComponentType)) {
        skip |= LogError("VUID-VkConvertCooperativeVectorMatrixInfoNV-dstComponentType-10080", objlist,
                         info_loc.dot(Field::dstComponentType), "(%s) must be float32 or a supported matrixInterpretation",
                         string_VkComponentTypeKHR(info.dstComponentType));
    }

    return skip;
}

bool CoreChecks::PreCallValidateConvertCooperativeVectorMatrixNV(VkDevice device,
                                                                 const VkConvertCooperativeVectorMatrixInfoNV *pInfo,
                                                                 const ErrorObject &error_obj) const {
    bool skip = false;

    const Location info_loc = error_obj.location.dot(Field::pInfo);

    skip |= ValidateVkConvertCooperativeVectorMatrixInfoNV(device, *pInfo, info_loc);

    return skip;
}

bool CoreChecks::PreCallValidateCmdConvertCooperativeVectorMatrixNV(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                                    const VkConvertCooperativeVectorMatrixInfoNV *pInfos,
                                                                    const ErrorObject &error_obj) const {
    bool skip = false;

    for (uint32_t i = 0; i < infoCount; ++i) {
        auto const &info = pInfos[i];
        auto src_buffers = GetBuffersByAddress(info.srcData.deviceAddress);
        auto dst_buffers = GetBuffersByAddress(info.dstData.deviceAddress);

        const Location info_loc = error_obj.location.dot(Field::pInfos, i);

        if (src_buffers.empty()) {
            skip |= LogError("VUID-vkCmdConvertCooperativeVectorMatrixNV-pInfo-10083", commandBuffer,
                             info_loc.dot(Field::srcData).dot(Field::deviceAddress), "(0x%" PRIx64 ") does not belong to a buffer",
                             info.srcData.deviceAddress);
        }
        if (dst_buffers.empty()) {
            skip |= LogError("VUID-vkCmdConvertCooperativeVectorMatrixNV-pInfo-10083", commandBuffer,
                             info_loc.dot(Field::dstData).dot(Field::deviceAddress), "(0x%" PRIx64 ") does not belong to a buffer",
                             info.dstData.deviceAddress);
        }

        skip |= ValidateVkConvertCooperativeVectorMatrixInfoNV(commandBuffer, info, info_loc);
    }

    return skip;
}
