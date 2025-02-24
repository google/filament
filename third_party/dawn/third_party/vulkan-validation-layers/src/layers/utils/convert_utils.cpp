/* Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
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

#include "convert_utils.h"

#include <vector>

#include <vulkan/utility/vk_format_utils.h>

#include <vulkan/utility/vk_struct_helper.hpp>

static vku::safe_VkAttachmentDescription2 ToV2KHR(const VkAttachmentDescription& in_struct) {
    vku::safe_VkAttachmentDescription2 v2;
    v2.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
    v2.pNext = nullptr;
    v2.flags = in_struct.flags;
    v2.format = in_struct.format;
    v2.samples = in_struct.samples;
    v2.loadOp = in_struct.loadOp;
    v2.storeOp = in_struct.storeOp;
    v2.stencilLoadOp = in_struct.stencilLoadOp;
    v2.stencilStoreOp = in_struct.stencilStoreOp;
    v2.initialLayout = in_struct.initialLayout;
    v2.finalLayout = in_struct.finalLayout;

    return v2;
}

static vku::safe_VkAttachmentReference2 ToV2KHR(const VkAttachmentReference& in_struct, const VkImageAspectFlags aspectMask = 0) {
    vku::safe_VkAttachmentReference2 v2;
    v2.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
    v2.pNext = nullptr;
    v2.attachment = in_struct.attachment;
    v2.layout = in_struct.layout;
    v2.aspectMask = aspectMask;

    return v2;
}

static vku::safe_VkSubpassDescription2 ToV2KHR(const VkSubpassDescription& in_struct, const uint32_t viewMask,
                                              const VkImageAspectFlags* color_attachment_aspect_masks,
                                              const VkImageAspectFlags ds_attachment_aspect_mask,
                                              const VkImageAspectFlags* input_attachment_aspect_masks) {
    vku::safe_VkSubpassDescription2 v2;
    v2.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
    v2.pNext = nullptr;
    v2.flags = in_struct.flags;
    v2.pipelineBindPoint = in_struct.pipelineBindPoint;
    v2.viewMask = viewMask;
    v2.inputAttachmentCount = in_struct.inputAttachmentCount;
    v2.pInputAttachments = nullptr;  // to be filled
    v2.colorAttachmentCount = in_struct.colorAttachmentCount;
    v2.pColorAttachments = nullptr;        // to be filled
    v2.pResolveAttachments = nullptr;      // to be filled
    v2.pDepthStencilAttachment = nullptr;  // to be filled
    v2.preserveAttachmentCount = in_struct.preserveAttachmentCount;
    v2.pPreserveAttachments = nullptr;  // to be filled

    if (v2.inputAttachmentCount && in_struct.pInputAttachments) {
        v2.pInputAttachments = new vku::safe_VkAttachmentReference2[v2.inputAttachmentCount];
        for (uint32_t i = 0; i < v2.inputAttachmentCount; ++i) {
            v2.pInputAttachments[i] = ToV2KHR(in_struct.pInputAttachments[i], input_attachment_aspect_masks[i]);
        }
    }
    if (v2.colorAttachmentCount && in_struct.pColorAttachments) {
        v2.pColorAttachments = new vku::safe_VkAttachmentReference2[v2.colorAttachmentCount];
        for (uint32_t i = 0; i < v2.colorAttachmentCount; ++i) {
            v2.pColorAttachments[i] = ToV2KHR(in_struct.pColorAttachments[i], color_attachment_aspect_masks[i]);
        }
    }
    if (v2.colorAttachmentCount && in_struct.pResolveAttachments) {
        v2.pResolveAttachments = new vku::safe_VkAttachmentReference2[v2.colorAttachmentCount];
        for (uint32_t i = 0; i < v2.colorAttachmentCount; ++i) {
            v2.pResolveAttachments[i] = ToV2KHR(in_struct.pResolveAttachments[i]);
        }
    }
    if (in_struct.pDepthStencilAttachment) {
        v2.pDepthStencilAttachment = new vku::safe_VkAttachmentReference2();
        *v2.pDepthStencilAttachment = ToV2KHR(*in_struct.pDepthStencilAttachment, ds_attachment_aspect_mask);
    }
    if (v2.preserveAttachmentCount && in_struct.pPreserveAttachments) {
        auto preserve_attachments = new uint32_t[v2.preserveAttachmentCount];
        for (uint32_t i = 0; i < v2.preserveAttachmentCount; ++i) {
            preserve_attachments[i] = in_struct.pPreserveAttachments[i];
        }
        v2.pPreserveAttachments = preserve_attachments;
    }

    return v2;
}

static vku::safe_VkSubpassDependency2 ToV2KHR(const VkSubpassDependency& in_struct, int32_t viewOffset = 0) {
    vku::safe_VkSubpassDependency2 v2;
    v2.sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;
    v2.pNext = nullptr;
    v2.srcSubpass = in_struct.srcSubpass;
    v2.dstSubpass = in_struct.dstSubpass;
    v2.srcStageMask = in_struct.srcStageMask;
    v2.dstStageMask = in_struct.dstStageMask;
    v2.srcAccessMask = in_struct.srcAccessMask;
    v2.dstAccessMask = in_struct.dstAccessMask;
    v2.dependencyFlags = in_struct.dependencyFlags;
    v2.viewOffset = viewOffset;

    return v2;
}

vku::safe_VkRenderPassCreateInfo2 ConvertVkRenderPassCreateInfoToV2KHR(const VkRenderPassCreateInfo& create_info) {
    vku::safe_VkRenderPassCreateInfo2 out_struct;
    const auto multiview_info = vku::FindStructInPNextChain<VkRenderPassMultiviewCreateInfo>(create_info.pNext);
    const auto* input_attachment_aspect_info = vku::FindStructInPNextChain<VkRenderPassInputAttachmentAspectCreateInfo>(create_info.pNext);
    const auto fragment_density_map_info = vku::FindStructInPNextChain<VkRenderPassFragmentDensityMapCreateInfoEXT>(create_info.pNext);

    out_struct.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2;

    // Fixup RPCI2 pNext chain.  Only FDM2 is valid on both chains.
    if (fragment_density_map_info) {
        out_struct.pNext = vku::SafePnextCopy(fragment_density_map_info);
        auto base_struct = reinterpret_cast<const VkBaseOutStructure*>(out_struct.pNext);
        const_cast<VkBaseOutStructure*>(base_struct)->pNext = nullptr;
    } else {
        out_struct.pNext = nullptr;
    }

    out_struct.flags = create_info.flags;
    out_struct.attachmentCount = create_info.attachmentCount;
    out_struct.pAttachments = nullptr;  // to be filled
    out_struct.subpassCount = create_info.subpassCount;
    out_struct.pSubpasses = nullptr;  // to be filled
    out_struct.dependencyCount = create_info.dependencyCount;
    out_struct.pDependencies = nullptr;  // to be filled
    out_struct.correlatedViewMaskCount = multiview_info ? multiview_info->correlationMaskCount : 0;
    out_struct.pCorrelatedViewMasks = nullptr;  // to be filled

    // TODO: This should support VkRenderPassFragmentDensityMapCreateInfoEXT somehow
    // see https://github.com/KhronosGroup/Vulkan-Docs/issues/1027

    if (out_struct.attachmentCount && create_info.pAttachments) {
        out_struct.pAttachments = new vku::safe_VkAttachmentDescription2[out_struct.attachmentCount];
        for (uint32_t i = 0; i < out_struct.attachmentCount; ++i) {
            out_struct.pAttachments[i] = ToV2KHR(create_info.pAttachments[i]);
        }
    }

    // translate VkRenderPassInputAttachmentAspectCreateInfo into vector
    std::vector<std::vector<VkImageAspectFlags>> color_attachment_aspect_masks(out_struct.subpassCount);
    std::vector<VkImageAspectFlags> depth_stencil_attachment_aspect_masks(out_struct.subpassCount);
    std::vector<std::vector<VkImageAspectFlags>> input_attachment_aspect_masks(out_struct.subpassCount);

    const auto GetAspectFromFormat = [](VkFormat format) {
        VkImageAspectFlags aspect = 0u;
        if (vkuFormatIsColor(format)) aspect |= VK_IMAGE_ASPECT_COLOR_BIT;
        if (vkuFormatHasDepth(format)) aspect |= VK_IMAGE_ASPECT_DEPTH_BIT;
        if (vkuFormatHasStencil(format)) aspect |= VK_IMAGE_ASPECT_STENCIL_BIT;
        if (vkuFormatPlaneCount(format) > 1) {
            aspect |= VK_IMAGE_ASPECT_PLANE_0_BIT;
            aspect |= VK_IMAGE_ASPECT_PLANE_1_BIT;
        }
        if (vkuFormatPlaneCount(format) > 2) aspect |= VK_IMAGE_ASPECT_PLANE_2_BIT;
        return aspect;
    };
    // set defaults
    for (uint32_t si = 0; si < out_struct.subpassCount; ++si) {
        if (create_info.pSubpasses) {
            const auto& subpass = create_info.pSubpasses[si];
            color_attachment_aspect_masks[si].resize(subpass.colorAttachmentCount, 0);
            input_attachment_aspect_masks[si].resize(subpass.inputAttachmentCount, 0);

            for (uint32_t cai = 0; cai < subpass.colorAttachmentCount; ++cai) {
                if (out_struct.pAttachments && subpass.pColorAttachments) {
                    const auto& color_attachment = subpass.pColorAttachments[cai];
                    if (color_attachment.attachment != VK_ATTACHMENT_UNUSED) {
                        const auto format = out_struct.pAttachments[color_attachment.attachment].format;
                        color_attachment_aspect_masks[si][cai] = GetAspectFromFormat(format);
                    }
                }
            }

            if (out_struct.pAttachments && subpass.pDepthStencilAttachment) {
                const auto& ds_attachment = *subpass.pDepthStencilAttachment;
                if (ds_attachment.attachment != VK_ATTACHMENT_UNUSED) {
                    const auto format = out_struct.pAttachments[ds_attachment.attachment].format;
                    depth_stencil_attachment_aspect_masks[si] = GetAspectFromFormat(format);
                }
            }

            for (uint32_t iai = 0; iai < subpass.inputAttachmentCount; ++iai) {
                if (out_struct.pAttachments && subpass.pInputAttachments) {
                    const auto& input_attachment = subpass.pInputAttachments[iai];
                    if (input_attachment.attachment != VK_ATTACHMENT_UNUSED) {
                        const auto format = out_struct.pAttachments[input_attachment.attachment].format;
                        input_attachment_aspect_masks[si][iai] = GetAspectFromFormat(format);
                    }
                }
            }
        }
    }
    // translate VkRenderPassInputAttachmentAspectCreateInfo
    if (input_attachment_aspect_info && input_attachment_aspect_info->pAspectReferences) {
        for (uint32_t i = 0; i < input_attachment_aspect_info->aspectReferenceCount; ++i) {
            const uint32_t subpass = input_attachment_aspect_info->pAspectReferences[i].subpass;
            const uint32_t input_attachment = input_attachment_aspect_info->pAspectReferences[i].inputAttachmentIndex;
            const VkImageAspectFlags aspect_mask = input_attachment_aspect_info->pAspectReferences[i].aspectMask;

            if (subpass < input_attachment_aspect_masks.size() &&
                input_attachment < input_attachment_aspect_masks[subpass].size()) {
                input_attachment_aspect_masks[subpass][input_attachment] = aspect_mask;
            }
        }
    }

    const bool has_view_mask = multiview_info && multiview_info->subpassCount && multiview_info->pViewMasks;
    if (out_struct.subpassCount && create_info.pSubpasses) {
        out_struct.pSubpasses = new vku::safe_VkSubpassDescription2[out_struct.subpassCount];
        for (uint32_t i = 0; i < out_struct.subpassCount; ++i) {
            const uint32_t view_mask = has_view_mask ? multiview_info->pViewMasks[i] : 0;
            out_struct.pSubpasses[i] = ToV2KHR(create_info.pSubpasses[i], view_mask, color_attachment_aspect_masks[i].data(),
                                               depth_stencil_attachment_aspect_masks[i], input_attachment_aspect_masks[i].data());
        }
    }

    const bool has_view_offset = multiview_info && multiview_info->dependencyCount && multiview_info->pViewOffsets;
    if (out_struct.dependencyCount && create_info.pDependencies) {
        out_struct.pDependencies = new vku::safe_VkSubpassDependency2[out_struct.dependencyCount];
        for (uint32_t i = 0; i < out_struct.dependencyCount; ++i) {
            const int32_t view_offset = has_view_offset ? multiview_info->pViewOffsets[i] : 0;
            out_struct.pDependencies[i] = ToV2KHR(create_info.pDependencies[i], view_offset);
        }
    }

    if (out_struct.correlatedViewMaskCount && multiview_info->pCorrelationMasks) {
        auto correlated_view_masks = new uint32_t[out_struct.correlatedViewMaskCount];
        for (uint32_t i = 0; i < out_struct.correlatedViewMaskCount; ++i) {
            correlated_view_masks[i] = multiview_info->pCorrelationMasks[i];
        }
        out_struct.pCorrelatedViewMasks = correlated_view_masks;
    }
    return out_struct;
}

vku::safe_VkImageMemoryBarrier2 ConvertVkImageMemoryBarrierToV2(const VkImageMemoryBarrier& barrier,
                                                               VkPipelineStageFlags2 srcStageMask,
                                                               VkPipelineStageFlags2 dstStageMask) {
    VkImageMemoryBarrier2 barrier2 = vku::InitStructHelper();

    // As of Vulkan 1.3.153, the VkImageMemoryBarrier2 supports the same pNext structs as VkImageMemoryBarrier
    // (VkExternalMemoryAcquireUnmodifiedEXT and VkSampleLocationsInfoEXT). It means we can copy entire pNext
    // chain (as part of vku::safe_VkImageMemoryBarrier2 constructor) without analyzing which pNext structures are supported in V2.
    barrier2.pNext = barrier.pNext;

    barrier2.srcStageMask = srcStageMask;
    barrier2.srcAccessMask = barrier.srcAccessMask;
    barrier2.dstStageMask = dstStageMask;
    barrier2.dstAccessMask = barrier.dstAccessMask;
    barrier2.oldLayout = barrier.oldLayout;
    barrier2.newLayout = barrier.newLayout;
    barrier2.srcQueueFamilyIndex = barrier.srcQueueFamilyIndex;
    barrier2.dstQueueFamilyIndex = barrier.dstQueueFamilyIndex;
    barrier2.image = barrier.image;
    barrier2.subresourceRange = barrier.subresourceRange;
    return vku::safe_VkImageMemoryBarrier2(&barrier2);
}

SubmitInfoConverter::SubmitInfoConverter(const VkSubmitInfo* submit_infos, uint32_t count) {
    size_t wait_count = 0;
    size_t cb_count = 0;
    size_t signal_count = 0;

    for (uint32_t batch = 0; batch < count; batch++) {
        const VkSubmitInfo& info = submit_infos[batch];
        wait_count += info.waitSemaphoreCount;
        cb_count += info.commandBufferCount;
        signal_count += info.signalSemaphoreCount;
    }
    wait_infos.resize(wait_count);
    auto* current_wait = wait_infos.data();

    cb_infos.resize(cb_count);
    auto* current_cb = cb_infos.data();

    signal_infos.resize(signal_count);
    auto* current_signal = signal_infos.data();

    submit_infos2.resize(count);
    for (uint32_t batch = 0; batch < count; batch++) {
        const VkSubmitInfo& info = submit_infos[batch];
        const auto* timeline_values = vku::FindStructInPNextChain<VkTimelineSemaphoreSubmitInfo>(info.pNext);

        VkSubmitInfo2& info2 = submit_infos2[batch];
        info2 = vku::InitStructHelper();

        if (info.waitSemaphoreCount) {
            info2.waitSemaphoreInfoCount = info.waitSemaphoreCount;
            info2.pWaitSemaphoreInfos = current_wait;
            for (uint32_t i = 0; i < info.waitSemaphoreCount; i++, current_wait++) {
                *current_wait = vku::InitStructHelper();
                current_wait->semaphore = info.pWaitSemaphores[i];
                current_wait->stageMask = info.pWaitDstStageMask[i];
                if (timeline_values) {
                    if (i >= timeline_values->waitSemaphoreValueCount) {
                        continue;  // [core validation check]
                    }
                    current_wait->value = timeline_values->pWaitSemaphoreValues[i];
                }
            }
        }
        if (info.commandBufferCount) {
            info2.commandBufferInfoCount = info.commandBufferCount;
            info2.pCommandBufferInfos = current_cb;
            for (uint32_t i = 0; i < info.commandBufferCount; i++, current_cb++) {
                *current_cb = vku::InitStructHelper();
                current_cb->commandBuffer = info.pCommandBuffers[i];
            }
        }
        if (info.signalSemaphoreCount) {
            info2.signalSemaphoreInfoCount = info.signalSemaphoreCount;
            info2.pSignalSemaphoreInfos = current_signal;
            for (uint32_t i = 0; i < info.signalSemaphoreCount; i++, current_signal++) {
                *current_signal = vku::InitStructHelper();
                current_signal->semaphore = info.pSignalSemaphores[i];
                current_signal->stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
                if (timeline_values) {
                    if (i >= timeline_values->signalSemaphoreValueCount) {
                        continue;  // [core validation check]
                    }
                    current_signal->value = timeline_values->pSignalSemaphoreValues[i];
                }
            }
        }
    }
}
