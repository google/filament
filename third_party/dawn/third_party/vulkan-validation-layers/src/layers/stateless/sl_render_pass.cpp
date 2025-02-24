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

#include "stateless/stateless_validation.h"
#include "utils/convert_utils.h"
#include "error_message/error_strings.h"

namespace stateless {

bool Device::ValidateSubpassGraphicsFlags(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo, uint32_t subpass,
                                          VkPipelineStageFlags2 stages, const char *vuid, const Location &loc) const {
    bool skip = false;
    // make sure we consider all of the expanded and un-expanded graphics bits to be valid
    const auto kExcludeStages = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT | VK_PIPELINE_STAGE_2_COPY_BIT |
                                VK_PIPELINE_STAGE_2_RESOLVE_BIT | VK_PIPELINE_STAGE_2_BLIT_BIT | VK_PIPELINE_STAGE_2_CLEAR_BIT;
    const auto kMetaGraphicsStages = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT |
                                     VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT;
    const auto kGraphicsStages =
        (sync_utils::ExpandPipelineStages(VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_QUEUE_GRAPHICS_BIT) | kMetaGraphicsStages) &
        ~kExcludeStages;

    const auto IsPipeline = [pCreateInfo](uint32_t subpass, const VkPipelineBindPoint stage) {
        if (subpass == VK_SUBPASS_EXTERNAL || subpass >= pCreateInfo->subpassCount) {
            return false;
        } else {
            return pCreateInfo->pSubpasses[subpass].pipelineBindPoint == stage;
        }
    };

    const bool is_all_graphics_stages = (stages & ~kGraphicsStages) == 0;
    if (IsPipeline(subpass, VK_PIPELINE_BIND_POINT_GRAPHICS) && !is_all_graphics_stages) {
        skip |= LogError(vuid, device, loc,
                         "dependency contains a stage mask (%s) that are not part "
                         "of the Graphics pipeline",
                         sync_utils::StringPipelineStageFlags(stages & ~kGraphicsStages).c_str());
    }

    return skip;
}

bool Device::ValidateCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo,
                                      const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                      const ErrorObject &error_obj) const {
    bool skip = false;
    uint32_t max_color_attachments = device_limits.maxColorAttachments;
    const bool use_rp2 = error_obj.location.function != Func::vkCreateRenderPass;
    const char *vuid = nullptr;
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);

    VkBool32 android_external_format_resolve_feature = false;
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    android_external_format_resolve_feature = enabled_features.externalFormatResolve;
#endif

    for (uint32_t i = 0; i < pCreateInfo->attachmentCount; ++i) {
        const Location &attachment_loc = create_info_loc.dot(Field::pAttachments, i);

        // if not null, also confirms rp2 is being used
        const void *pNext =
            (use_rp2) ? reinterpret_cast<VkAttachmentDescription2 const *>(&pCreateInfo->pAttachments[i])->pNext : nullptr;
        const auto *attachment_description_stencil_layout =
            (use_rp2) ? vku::FindStructInPNextChain<VkAttachmentDescriptionStencilLayout>(pNext) : nullptr;

        const VkFormat attachment_format = pCreateInfo->pAttachments[i].format;
        const VkImageLayout initial_layout = pCreateInfo->pAttachments[i].initialLayout;
        const VkImageLayout final_layout = pCreateInfo->pAttachments[i].finalLayout;
        if (attachment_format == VK_FORMAT_UNDEFINED) {
            if (use_rp2 && android_external_format_resolve_feature) {
                if (GetExternalFormat(pNext) == 0) {
                    skip |= LogError("VUID-VkAttachmentDescription2-format-09334", device, attachment_loc.dot(Field::format),
                                     "is VK_FORMAT_UNDEFINED.");
                }
            } else {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-09332" : "VUID-VkAttachmentDescription-format-06698";
                skip |= LogError(vuid, device, attachment_loc.dot(Field::format), "is VK_FORMAT_UNDEFINED.");
            }
        }
        if (final_layout == VK_IMAGE_LAYOUT_UNDEFINED || final_layout == VK_IMAGE_LAYOUT_PREINITIALIZED) {
            vuid = use_rp2 ? "VUID-VkAttachmentDescription2-finalLayout-00843" : "VUID-VkAttachmentDescription-finalLayout-00843";
            skip |= LogError(vuid, device, attachment_loc.dot(Field::finalLayout), "is %s.", string_VkImageLayout(final_layout));
        }
        if (!enabled_features.separateDepthStencilLayouts) {
            if (IsImageLayoutDepthOnly(initial_layout) || IsImageLayoutStencilOnly(initial_layout)) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-separateDepthStencilLayouts-03284"
                               : "VUID-VkAttachmentDescription-separateDepthStencilLayouts-03284";
                skip |= LogError(vuid, device, attachment_loc.dot(Field::initialLayout), "is %s.",
                                 string_VkImageLayout(initial_layout));
            }
            if (IsImageLayoutDepthOnly(final_layout) || IsImageLayoutStencilOnly(final_layout)) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-separateDepthStencilLayouts-03285"
                               : "VUID-VkAttachmentDescription-separateDepthStencilLayouts-03285";
                skip |=
                    LogError(vuid, device, attachment_loc.dot(Field::finalLayout), "is %s.", string_VkImageLayout(final_layout));
            }
        }
        if (!enabled_features.attachmentFeedbackLoopLayout) {
            if (initial_layout == VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-attachmentFeedbackLoopLayout-07309"
                               : "VUID-VkAttachmentDescription-attachmentFeedbackLoopLayout-07309";
                skip |= LogError(vuid, device, attachment_loc.dot(Field::initialLayout),
                                 "is VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT but the "
                                 "attachmentFeedbackLoopLayout feature is not enabled.");
            }
            if (final_layout == VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-attachmentFeedbackLoopLayout-07310"
                               : "VUID-VkAttachmentDescription-attachmentFeedbackLoopLayout-07310";
                skip |= LogError(vuid, device, attachment_loc.dot(Field::finalLayout),
                                 "is VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT but the "
                                 "attachmentFeedbackLoopLayout feature is not enabled.");
            }
        }
        if (!enabled_features.synchronization2) {
            if (initial_layout == VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL || initial_layout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-synchronization2-06908"
                               : "VUID-VkAttachmentDescription-synchronization2-06908";
                skip |= LogError(vuid, device, attachment_loc.dot(Field::initialLayout),
                                 "is %s but the synchronization2 feature is not enabled.", string_VkImageLayout(initial_layout));
            }
            if (final_layout == VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL || final_layout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-synchronization2-06909"
                               : "VUID-VkAttachmentDescription-synchronization2-06909";
                skip |= LogError(vuid, device, attachment_loc.dot(Field::finalLayout),
                                 "is %s but the synchronization2 feature is not enabled.", string_VkImageLayout(final_layout));
            }
        }
        if (!enabled_features.dynamicRenderingLocalRead) {
            if (initial_layout == VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-dynamicRenderingLocalRead-09544"
                               : "VUID-VkAttachmentDescription-dynamicRenderingLocalRead-09544";
                skip |= LogError(vuid, device, attachment_loc.dot(Field::initialLayout),
                                 "is VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ but the "
                                 "dynamicRenderingLocalRead feature is not enabled.");
            }
            if (final_layout == VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-dynamicRenderingLocalRead-09545"
                               : "VUID-VkAttachmentDescription-dynamicRenderingLocalRead-09545";
                skip |= LogError(vuid, device, attachment_loc.dot(Field::finalLayout),
                                 "is VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ but the "
                                 "dynamicRenderingLocalRead feature is not enabled.");
            }
        }
        if (!vkuFormatIsDepthOrStencil(attachment_format)) {  // color format
            if (IsImageLayoutDepthOnly(initial_layout) || IsImageLayoutStencilOnly(initial_layout)) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-03286" : "VUID-VkAttachmentDescription-format-03286";
                skip |= LogError(vuid, device, attachment_loc.dot(Field::initialLayout), "is %s.",
                                 string_VkImageLayout(initial_layout));
            }
            if (IsImageLayoutDepthOnly(final_layout) || IsImageLayoutStencilOnly(final_layout)) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-03287" : "VUID-VkAttachmentDescription-format-03287";
                skip |=
                    LogError(vuid, device, attachment_loc.dot(Field::finalLayout), "is %s.", string_VkImageLayout(final_layout));
            }
        } else if (vkuFormatIsDepthAndStencil(attachment_format)) {
            if (IsImageLayoutStencilOnly(initial_layout)) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-06906" : "VUID-VkAttachmentDescription-format-06906";
                skip |= LogError(vuid, device, attachment_loc.dot(Field::initialLayout), "is %s.",
                                 string_VkImageLayout(initial_layout));
            }
            if (IsImageLayoutStencilOnly(final_layout)) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-06907" : "VUID-VkAttachmentDescription-format-06907";
                skip |=
                    LogError(vuid, device, attachment_loc.dot(Field::finalLayout), "is %s.", string_VkImageLayout(final_layout));
            }

            if (!attachment_description_stencil_layout) {
                if (IsImageLayoutDepthOnly(initial_layout)) {
                    vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-06249" : "VUID-VkAttachmentDescription-format-06242";
                    skip |= LogError(vuid, device, attachment_loc.dot(Field::initialLayout),
                                     "is %s but no VkAttachmentDescriptionStencilLayout provided.",
                                     string_VkImageLayout(initial_layout));
                }
                if (IsImageLayoutDepthOnly(final_layout)) {
                    vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-06250" : "VUID-VkAttachmentDescription-format-06243";
                    skip |=
                        LogError(vuid, device, attachment_loc.dot(Field::finalLayout),
                                 "is %s but no VkAttachmentDescriptionStencilLayout provided.", string_VkImageLayout(final_layout));
                }
            }
        } else if (vkuFormatIsDepthOnly(attachment_format)) {
            if (IsImageLayoutStencilOnly(initial_layout)) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-03290" : "VUID-VkAttachmentDescription-format-03290";
                skip |= LogError(vuid, device, attachment_loc.dot(Field::initialLayout), "is %s.",
                                 string_VkImageLayout(initial_layout));
            }
            if (IsImageLayoutStencilOnly(final_layout)) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-03291" : "VUID-VkAttachmentDescription-format-03291";
                skip |=
                    LogError(vuid, device, attachment_loc.dot(Field::finalLayout), "is %s.", string_VkImageLayout(final_layout));
            }
        } else if (vkuFormatIsStencilOnly(attachment_format) && !attachment_description_stencil_layout) {
            if (IsImageLayoutDepthOnly(initial_layout)) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-06247" : "VUID-VkAttachmentDescription-format-03292";
                skip |= LogError(vuid, device, attachment_loc.dot(Field::initialLayout), "is %s.",
                                 string_VkImageLayout(initial_layout));
            }
            if (IsImageLayoutDepthOnly(final_layout)) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-06248" : "VUID-VkAttachmentDescription-format-03293";
                skip |=
                    LogError(vuid, device, attachment_loc.dot(Field::finalLayout), "is %s.", string_VkImageLayout(final_layout));
            }
        }
        if (attachment_description_stencil_layout) {
            const VkImageLayout stencil_initial_layout = attachment_description_stencil_layout->stencilInitialLayout;
            const VkImageLayout stencil_final_layout = attachment_description_stencil_layout->stencilFinalLayout;

            if (stencil_initial_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ||
                stencil_initial_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ||
                stencil_initial_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL ||
                stencil_initial_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
                stencil_initial_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL ||
                stencil_initial_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL ||
                stencil_initial_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL) {
                skip |= LogError("VUID-VkAttachmentDescriptionStencilLayout-stencilInitialLayout-03308", device,
                                 attachment_loc.pNext(Struct::VkAttachmentDescriptionStencilLayout, Field::stencilInitialLayout),
                                 "is %s.", string_VkImageLayout(stencil_initial_layout));
            }
            if (stencil_final_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ||
                stencil_final_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ||
                stencil_final_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL ||
                stencil_final_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
                stencil_final_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL ||
                stencil_final_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL ||
                stencil_final_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL) {
                skip |= LogError("VUID-VkAttachmentDescriptionStencilLayout-stencilFinalLayout-03309", device,
                                 attachment_loc.pNext(Struct::VkAttachmentDescriptionStencilLayout, Field::stencilFinalLayout),
                                 "is %s.", string_VkImageLayout(stencil_final_layout));
            }
            if (stencil_final_layout == VK_IMAGE_LAYOUT_UNDEFINED || stencil_final_layout == VK_IMAGE_LAYOUT_PREINITIALIZED) {
                skip |= LogError("VUID-VkAttachmentDescriptionStencilLayout-stencilFinalLayout-03310", device,
                                 attachment_loc.pNext(Struct::VkAttachmentDescriptionStencilLayout, Field::stencilFinalLayout),
                                 "is %s.", string_VkImageLayout(stencil_final_layout));
            }
        }

        if (vkuFormatIsDepthOrStencil(attachment_format)) {
            if (initial_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-03281" : "VUID-VkAttachmentDescription-format-03281";
                skip |= LogError(vuid, device, attachment_loc.dot(Field::initialLayout),
                                 "must not be VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL when using a Depth or Stencil format (%s)",
                                 string_VkFormat(attachment_format));
            }
            if (final_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-03283" : "VUID-VkAttachmentDescription-format-03283";
                skip |= LogError(vuid, device, attachment_loc.dot(Field::finalLayout),
                                 "must not be VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL when using a Depth or Stencil format (%s)",
                                 string_VkFormat(attachment_format));
            }
        }
        if (vkuFormatIsColor(attachment_format)) {
            if (initial_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
                initial_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-03280" : "VUID-VkAttachmentDescription-format-03280";
                skip |= LogError(vuid, device, attachment_loc.dot(Field::initialLayout), "is %s, but using a Color format (%s)",
                                 string_VkImageLayout(initial_layout), string_VkFormat(attachment_format));

            } else if (initial_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL ||
                       initial_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-06487" : "VUID-VkAttachmentDescription-format-06487";
                skip |= LogError(vuid, device, attachment_loc.dot(Field::initialLayout), "is %s, but using a Color format (%s)",
                                 string_VkImageLayout(initial_layout), string_VkFormat(attachment_format));
            }
            if (final_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
                final_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-03282" : "VUID-VkAttachmentDescription-format-03282";
                skip |= LogError(vuid, device, attachment_loc.dot(Field::finalLayout), "is %s, but using a Color format (%s)",
                                 string_VkImageLayout(final_layout), string_VkFormat(attachment_format));
            } else if (final_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL ||
                       final_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-06488" : "VUID-VkAttachmentDescription-format-06488";
                skip |= LogError(vuid, device, attachment_loc.dot(Field::finalLayout), "is %s, but using a Color format (%s)",
                                 string_VkImageLayout(final_layout), string_VkFormat(attachment_format));
            }
        }
        if (vkuFormatIsColor(attachment_format) || vkuFormatHasDepth(attachment_format)) {
            if (pCreateInfo->pAttachments[i].loadOp == VK_ATTACHMENT_LOAD_OP_LOAD && initial_layout == VK_IMAGE_LAYOUT_UNDEFINED) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-06699" : "VUID-VkAttachmentDescription-format-06699";
                skip |= LogError(
                    vuid, device, attachment_loc,
                    "format is %s and loadOp is VK_ATTACHMENT_LOAD_OP_LOAD, but initialLayout is VK_IMAGE_LAYOUT_UNDEFINED.",
                    string_VkFormat(attachment_format));
            }
        }
        if (vkuFormatHasStencil(attachment_format) && pCreateInfo->pAttachments[i].stencilLoadOp == VK_ATTACHMENT_LOAD_OP_LOAD) {
            if (initial_layout == VK_IMAGE_LAYOUT_UNDEFINED) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-pNext-06704" : "VUID-VkAttachmentDescription-format-06700";
                skip |= LogError(vuid, device, attachment_loc,
                                 "format (%s) includes stencil aspect and stencilLoadOp is VK_ATTACHMENT_LOAD_OP_LOAD, but "
                                 "the initialLayout is VK_IMAGE_LAYOUT_UNDEFINED.",
                                 string_VkFormat(attachment_format));
            }

            // rp2 can have seperate depth/stencil layout and need to look in pNext
            if (attachment_description_stencil_layout) {
                if (attachment_description_stencil_layout->stencilInitialLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
                    skip |=
                        LogError("VUID-VkAttachmentDescription2-pNext-06705", device, attachment_loc,
                                 "format includes stencil aspect and stencilLoadOp is VK_ATTACHMENT_LOAD_OP_LOAD, but "
                                 "the VkAttachmentDescriptionStencilLayout::stencilInitialLayout is VK_IMAGE_LAYOUT_UNDEFINED.");
                }
            }
        }
    }

    for (uint32_t i = 0; i < pCreateInfo->subpassCount; ++i) {
        if (pCreateInfo->pSubpasses[i].colorAttachmentCount > max_color_attachments) {
            vuid = use_rp2 ? "VUID-VkSubpassDescription2-colorAttachmentCount-03063"
                           : "VUID-VkSubpassDescription-colorAttachmentCount-00845";
            skip |= LogError(vuid, device, create_info_loc.dot(Field::pSubpasses, i),
                             "cannot be used to create a render pass. maxColorAttachments is %d.", max_color_attachments);
        }
    }

    for (uint32_t i = 0; i < pCreateInfo->dependencyCount; ++i) {
        const auto &dependency = pCreateInfo->pDependencies[i];

        // Need to check first so layer doesn't segfault from out of bound array access
        // src subpass bound check
        if ((dependency.srcSubpass != VK_SUBPASS_EXTERNAL) && (dependency.srcSubpass >= pCreateInfo->subpassCount)) {
            vuid = use_rp2 ? "VUID-VkRenderPassCreateInfo2-srcSubpass-02526" : "VUID-VkRenderPassCreateInfo-pDependencies-06866";
            skip |= LogError(vuid, device, create_info_loc.dot(Field::pDependencies, i).dot(Field::srcSubpass),
                             "index (%" PRIu32 ") has to be less than subpassCount (%" PRIu32 ")", dependency.srcSubpass,
                             pCreateInfo->subpassCount);
        }

        // dst subpass bound check
        if ((dependency.dstSubpass != VK_SUBPASS_EXTERNAL) && (dependency.dstSubpass >= pCreateInfo->subpassCount)) {
            vuid = use_rp2 ? "VUID-VkRenderPassCreateInfo2-dstSubpass-02527" : "VUID-VkRenderPassCreateInfo-pDependencies-06867";
            skip |= LogError(vuid, device, create_info_loc.dot(Field::pDependencies, i).dot(Field::dstSubpass),
                             "index (%" PRIu32 ") has to be less than subpassCount (%" PRIu32 ")", dependency.dstSubpass,
                             pCreateInfo->subpassCount);
        }

        VkPipelineStageFlags2 srcStageMask = dependency.srcStageMask;
        VkPipelineStageFlags2 dstStageMask = dependency.dstStageMask;
        if (const auto barrier = vku::FindStructInPNextChain<VkMemoryBarrier2>(pCreateInfo->pDependencies[i].pNext); barrier) {
            srcStageMask = barrier->srcStageMask;
            dstStageMask = barrier->dstStageMask;
        }

        // Spec currently only supports Graphics pipeline in render pass -- so only that pipeline is currently checked
        vuid = use_rp2 ? "VUID-VkRenderPassCreateInfo2-pDependencies-03054" : "VUID-VkRenderPassCreateInfo-pDependencies-00837";
        skip |= ValidateSubpassGraphicsFlags(device, pCreateInfo, dependency.srcSubpass, srcStageMask, vuid,
                                             create_info_loc.dot(Field::pDependencies, i).dot(Field::srcSubpass));

        vuid = use_rp2 ? "VUID-VkRenderPassCreateInfo2-pDependencies-03055" : "VUID-VkRenderPassCreateInfo-pDependencies-00838";
        skip |= ValidateSubpassGraphicsFlags(device, pCreateInfo, dependency.dstSubpass, dstStageMask, vuid,
                                             create_info_loc.dot(Field::pDependencies, i).dot(Field::dstSubpass));
    }

    return skip;
}

bool Device::manual_PreCallValidateCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo,
                                                    const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                                    const Context &context) const {
    vku::safe_VkRenderPassCreateInfo2 create_info_2 = ConvertVkRenderPassCreateInfoToV2KHR(*pCreateInfo);
    return ValidateCreateRenderPass(device, create_info_2.ptr(), pAllocator, pRenderPass, context.error_obj);
}

bool Device::manual_PreCallValidateCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo,
                                                     const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                                     const Context &context) const {
    vku::safe_VkRenderPassCreateInfo2 create_info_2(pCreateInfo);
    return ValidateCreateRenderPass(device, create_info_2.ptr(), pAllocator, pRenderPass, context.error_obj);
}

void Device::RecordRenderPass(VkRenderPass renderPass, const VkRenderPassCreateInfo2 *pCreateInfo) {
    std::unique_lock<std::mutex> lock(renderpass_map_mutex);
    auto &renderpass_state = renderpasses_states[renderPass];
    lock.unlock();

    for (uint32_t subpass = 0; subpass < pCreateInfo->subpassCount; ++subpass) {
        bool uses_color = false;

        for (uint32_t i = 0; i < pCreateInfo->pSubpasses[subpass].colorAttachmentCount && !uses_color; ++i) {
            if (pCreateInfo->pSubpasses[subpass].pColorAttachments[i].attachment != VK_ATTACHMENT_UNUSED) uses_color = true;
        }

        bool uses_depthstencil = false;
        if (pCreateInfo->pSubpasses[subpass].pDepthStencilAttachment) {
            if (pCreateInfo->pSubpasses[subpass].pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED) {
                uses_depthstencil = true;
            }
        }

        if (uses_color) renderpass_state.subpasses_using_color_attachment.insert(subpass);
        if (uses_depthstencil) renderpass_state.subpasses_using_depthstencil_attachment.insert(subpass);
    }
}
void Device::PostCallRecordCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                            const RecordObject &record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    vku::safe_VkRenderPassCreateInfo2 create_info_2 = ConvertVkRenderPassCreateInfoToV2KHR(*pCreateInfo);
    RecordRenderPass(*pRenderPass, create_info_2.ptr());
}

void Device::PostCallRecordCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                                const RecordObject &record_obj) {
    // Track the state necessary for checking vkCreateGraphicsPipeline (subpass usage of depth and color attachments)
    if (record_obj.result != VK_SUCCESS) return;
    vku::safe_VkRenderPassCreateInfo2 create_info_2(pCreateInfo);
    RecordRenderPass(*pRenderPass, create_info_2.ptr());
}

void Device::PostCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks *pAllocator,
                                             const RecordObject &record_obj) {
    // Track the state necessary for checking vkCreateGraphicsPipeline (subpass usage of depth and color attachments)
    std::unique_lock<std::mutex> lock(renderpass_map_mutex);
    renderpasses_states.erase(renderPass);
}

bool Device::ValidateRenderPassStripeBeginInfo(VkCommandBuffer commandBuffer, const void *pNext, const VkRect2D render_area,
                                               const Location &loc) const {
    bool skip = false;
    const auto rp_stripe_begin = vku::FindStructInPNextChain<VkRenderPassStripeBeginInfoARM>(pNext);
    if (!rp_stripe_begin) {
        return skip;
    }

    if (rp_stripe_begin->stripeInfoCount > phys_dev_ext_props.renderpass_striped_props.maxRenderPassStripes) {
        skip |= LogError("VUID-VkRenderPassStripeBeginInfoARM-stripeInfoCount-09450", commandBuffer,
                         loc.pNext(Struct::VkRenderPassStripeBeginInfoARM, Field::stripeInfoCount),
                         "(%" PRIu32 ") is greater than maxRenderPassStripes (%" PRIu32 ").", rp_stripe_begin->stripeInfoCount,
                         phys_dev_ext_props.renderpass_striped_props.maxRenderPassStripes);
    }

    const uint32_t width_granularity = phys_dev_ext_props.renderpass_striped_props.renderPassStripeGranularity.width;
    const uint32_t height_granularity = phys_dev_ext_props.renderpass_striped_props.renderPassStripeGranularity.height;
    const uint32_t last_stripe_index = (rp_stripe_begin->stripeInfoCount - 1);
    uint32_t total_stripe_area = 0;
    bool has_overlapping_stripes = false;

    for (uint32_t i = 0; i < rp_stripe_begin->stripeInfoCount; ++i) {
        const Location &stripe_info_loc = loc.pNext(Struct::VkRenderPassStripeBeginInfoARM, Field::pStripeInfos, i);
        const VkRect2D stripe_area = rp_stripe_begin->pStripeInfos[i].stripeArea;
        total_stripe_area += (stripe_area.extent.width * stripe_area.extent.height);

        // Check overlapping stripes, report only first overlapping stripe info.
        for (uint32_t index = i + 1; (!has_overlapping_stripes && i != last_stripe_index && index <= last_stripe_index); ++index) {
            const auto rect = rp_stripe_begin->pStripeInfos[index].stripeArea;
            has_overlapping_stripes =
                RangesIntersect(rect.offset.x, rect.extent.width, stripe_area.offset.x, stripe_area.extent.width);
            has_overlapping_stripes &=
                RangesIntersect(rect.offset.y, rect.extent.height, stripe_area.offset.y, stripe_area.extent.height);

            if (has_overlapping_stripes) {
                skip |= LogError("VUID-VkRenderPassStripeBeginInfoARM-stripeArea-09451", commandBuffer, stripe_info_loc,
                                 "(offset{%s} extent{%s}) is overlapping with pStripeInfos[%" PRIu32 "] (offset{%s} extent {%s}).",
                                 string_VkOffset2D(stripe_area.offset).c_str(), string_VkExtent2D(stripe_area.extent).c_str(),
                                 index, string_VkOffset2D(rect.offset).c_str(), string_VkExtent2D(rect.extent).c_str());
                break;
            }
        }

        if (width_granularity > 0 && (stripe_area.offset.x % width_granularity) != 0) {
            skip |= LogError("VUID-VkRenderPassStripeInfoARM-stripeArea-09452", commandBuffer,
                             stripe_info_loc.dot(Field::stripeArea).dot(Field::offset).dot(Field::x),
                             "(%" PRIu32 ") is not a multiple of %" PRIu32 ".", stripe_area.offset.x, width_granularity);
        }

        if (width_granularity > 0 && (stripe_area.extent.width % width_granularity) != 0 &&
            ((stripe_area.extent.width + stripe_area.offset.x) != render_area.extent.width)) {
            skip |= LogError("VUID-VkRenderPassStripeInfoARM-stripeArea-09453", commandBuffer,
                             stripe_info_loc.dot(Field::stripeArea).dot(Field::extent).dot(Field::width),
                             "(%" PRIu32 ") is not a multiple of %" PRIu32
                             ", or when added to the stripeArea.offset.x is not equal render area width (%" PRIu32 ")",
                             stripe_area.extent.width, width_granularity, render_area.extent.width);
        }

        if (height_granularity > 0 && (stripe_area.offset.y % height_granularity) != 0) {
            skip |= LogError("VUID-VkRenderPassStripeInfoARM-stripeArea-09454", commandBuffer,
                             stripe_info_loc.dot(Field::stripeArea).dot(Field::offset).dot(Field::y),
                             "(%" PRIu32 ") is not a multiple of %" PRIu32 ".", stripe_area.offset.y, height_granularity);
        }

        if (height_granularity > 0 && (stripe_area.extent.height % height_granularity) != 0 &&
            (stripe_area.extent.height + stripe_area.offset.y) != render_area.extent.height) {
            skip |= LogError("VUID-VkRenderPassStripeInfoARM-stripeArea-09455", commandBuffer,
                             stripe_info_loc.dot(Field::stripeArea).dot(Field::extent).dot(Field::height),
                             "(%" PRIu32 ") is not a multiple of %" PRIu32
                             ", or when added to the stripeArea.offset.y is not equal to render area height (%" PRIu32 ")",
                             stripe_area.extent.height, height_granularity, render_area.extent.height);
        }
    }

    // Check render area coverage if there is no overlapping stripe.
    const uint32_t total_render_area = render_area.extent.width * render_area.extent.height;
    if (!has_overlapping_stripes && (total_stripe_area != total_render_area)) {
        const std::string vuid = (loc.function == Func::vkCmdBeginRenderPass) ? "VUID-VkRenderPassBeginInfo-pNext-09539"
                                                                              : "VUID-VkRenderingInfo-pNext-09535";
        skip |= LogError(vuid.data(), commandBuffer, loc.pNext(Struct::VkRenderPassStripeBeginInfoARM, Field::pStripeInfos),
                         "has total of stripe area of %" PRIu32 " is not covering whole render area of %" PRIu32 " (%s).",
                         total_stripe_area, total_render_area, string_VkExtent2D(render_area.extent).c_str());
    }

    return skip;
}

bool Device::ValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *const rp_begin,
                                        const ErrorObject &error_obj) const {
    bool skip = false;
    if ((rp_begin->clearValueCount != 0) && !rp_begin->pClearValues) {
        const LogObjectList objlist(commandBuffer, rp_begin->renderPass);
        skip |= LogError("VUID-VkRenderPassBeginInfo-clearValueCount-04962", objlist,
                         error_obj.location.dot(Field::pRenderPassBegin).dot(Field::clearValueCount),
                         "(%" PRIu32 ") is not zero, but pRenderPassBegin->pClearValues is NULL.", rp_begin->clearValueCount);
    }

    const Location loc = error_obj.location.dot(Field::pRenderPassBegin);
    skip |= ValidateRenderPassStripeBeginInfo(commandBuffer, rp_begin->pNext, rp_begin->renderArea, loc);

    return skip;
}

bool Device::manual_PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                      VkSubpassContents, const Context &context) const {
    return ValidateCmdBeginRenderPass(commandBuffer, pRenderPassBegin, context.error_obj);
}

bool Device::manual_PreCallValidateCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                       const VkSubpassBeginInfo *, const Context &context) const {
    return ValidateCmdBeginRenderPass(commandBuffer, pRenderPassBegin, context.error_obj);
}

static bool UniqueRenderingInfoImageViews(const VkRenderingInfo &rendering_info, VkImageView image_view) {
    bool unique_views = true;
    for (uint32_t i = 0; i < rendering_info.colorAttachmentCount; ++i) {
        if (rendering_info.pColorAttachments[i].imageView == image_view) {
            unique_views = false;
        }

        if (rendering_info.pColorAttachments[i].resolveImageView == image_view) {
            unique_views = false;
        }
    }

    if (rendering_info.pDepthAttachment) {
        if (rendering_info.pDepthAttachment->imageView == image_view) {
            unique_views = false;
        }

        if (rendering_info.pDepthAttachment->resolveImageView == image_view) {
            unique_views = false;
        }
    }

    if (rendering_info.pStencilAttachment) {
        if (rendering_info.pStencilAttachment->imageView == image_view) {
            unique_views = false;
        }

        if (rendering_info.pStencilAttachment->resolveImageView == image_view) {
            unique_views = false;
        }
    }
    return unique_views;
}

bool Device::manual_PreCallValidateCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo *pRenderingInfo,
                                                     const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    const Location rendering_info_loc = error_obj.location.dot(Field::pRenderingInfo);

    if (!enabled_features.dynamicRendering) {
        skip |= LogError("VUID-vkCmdBeginRendering-dynamicRendering-06446", commandBuffer, error_obj.location,
                         "dynamicRendering is not enabled.");
    }

    if (pRenderingInfo->viewMask == 0 && pRenderingInfo->layerCount == 0) {
        skip |= LogError("VUID-VkRenderingInfo-viewMask-06069", commandBuffer, rendering_info_loc,
                         "viewMask and layerCount are both zero");
    }

    if (pRenderingInfo->colorAttachmentCount > device_limits.maxColorAttachments) {
        skip |= LogError("VUID-VkRenderingInfo-colorAttachmentCount-06106", commandBuffer,
                         rendering_info_loc.dot(Field::colorAttachmentCount),
                         "(%" PRIu32
                         ") must be less than or equal to "
                         "maxColorAttachments (%" PRIu32 ").",
                         pRenderingInfo->colorAttachmentCount, device_limits.maxColorAttachments);
    }

    if ((pRenderingInfo->flags & VK_RENDERING_CONTENTS_INLINE_BIT_KHR) != 0 && !enabled_features.nestedCommandBuffer &&
        !enabled_features.maintenance7) {
        skip |= LogError("VUID-VkRenderingInfo-flags-10012", commandBuffer, rendering_info_loc.dot(Field::flags),
                         "is %s, but nestedCommandBuffer and maintenance7 feature were not enabled.",
                         string_VkRenderingFlags(pRenderingInfo->flags).c_str());
    }
    if (pRenderingInfo->layerCount > device_limits.maxFramebufferLayers) {
        skip |= LogError("VUID-VkRenderingInfo-layerCount-07817", commandBuffer, rendering_info_loc.dot(Field::layerCount),
                         "(%" PRIu32 ") is greater than maxFramebufferLayers (%" PRIu32 ").", pRenderingInfo->layerCount,
                         device_limits.maxFramebufferLayers);
    }

    if (!enabled_features.multiview && (pRenderingInfo->viewMask != 0)) {
        skip |= LogError("VUID-VkRenderingInfo-multiview-06127", commandBuffer, rendering_info_loc.dot(Field::viewMask),
                         "is %" PRId32 " but the multiview feature is not enabled.", pRenderingInfo->viewMask);
    }

    const auto rendering_fsr_attachment_info =
        vku::FindStructInPNextChain<VkRenderingFragmentShadingRateAttachmentInfoKHR>(pRenderingInfo->pNext);
    if (rendering_fsr_attachment_info) {
        skip |= ValidateBeginRenderingFragmentShadingRateAttachment(commandBuffer, *pRenderingInfo, *rendering_fsr_attachment_info,
                                                                    rendering_info_loc);
    }

    const auto fragment_density_map_attachment_info =
        vku::FindStructInPNextChain<VkRenderingFragmentDensityMapAttachmentInfoEXT>(pRenderingInfo->pNext);
    if (fragment_density_map_attachment_info && (fragment_density_map_attachment_info->imageView != VK_NULL_HANDLE)) {
        if (UniqueRenderingInfoImageViews(*pRenderingInfo, fragment_density_map_attachment_info->imageView) == false) {
            skip |= LogError("VUID-VkRenderingInfo-imageView-06116", commandBuffer,
                             rendering_info_loc.pNext(Struct::VkRenderingFragmentDensityMapAttachmentInfoEXT, Field::imageView),
                             "is %s.", FormatHandle(fragment_density_map_attachment_info->imageView).c_str());
        }

        if (fragment_density_map_attachment_info->imageLayout != VK_IMAGE_LAYOUT_GENERAL &&
            fragment_density_map_attachment_info->imageLayout != VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT) {
            skip |= LogError("VUID-VkRenderingFragmentDensityMapAttachmentInfoEXT-imageView-06157", commandBuffer,
                             rendering_info_loc.pNext(Struct::VkRenderingFragmentDensityMapAttachmentInfoEXT, Field::imageView),
                             "is %s, but "
                             "VkRenderingFragmentDensityMapAttachmentInfoEXT::imageLayout is %s.",
                             FormatHandle(fragment_density_map_attachment_info->imageView).c_str(),
                             string_VkImageLayout(fragment_density_map_attachment_info->imageLayout));
        }

        if (rendering_fsr_attachment_info &&
            (rendering_fsr_attachment_info->imageView == fragment_density_map_attachment_info->imageView)) {
            skip |= LogError("VUID-VkRenderingInfo-imageView-06126", commandBuffer,
                             rendering_info_loc.pNext(Struct::VkRenderingFragmentDensityMapAttachmentInfoEXT, Field::imageView),
                             "and VkRenderingFragmentShadingRateAttachmentInfoKHR::imageView are the same (%s).",
                             FormatHandle(fragment_density_map_attachment_info->imageView).c_str());
        }
    }

    skip |= ValidateRenderPassStripeBeginInfo(commandBuffer, pRenderingInfo->pNext, pRenderingInfo->renderArea, rendering_info_loc);
    skip |= ValidateBeginRenderingColorAttachment(commandBuffer, *pRenderingInfo, rendering_info_loc);
    skip |= ValidateBeginRenderingDepthAttachment(commandBuffer, *pRenderingInfo, rendering_info_loc);
    skip |= ValidateBeginRenderingStencilAttachment(commandBuffer, *pRenderingInfo, rendering_info_loc);
    return skip;
}

bool Device::ValidateBeginRenderingColorAttachment(VkCommandBuffer commandBuffer, const VkRenderingInfo &rendering_info,
                                                   const Location &rendering_info_loc) const {
    bool skip = false;
    for (uint32_t i = 0; i < rendering_info.colorAttachmentCount; ++i) {
        const VkRenderingAttachmentInfo &color_attachment = rendering_info.pColorAttachments[i];
        if (color_attachment.imageView == VK_NULL_HANDLE) continue;
        const Location color_attachment_loc = rendering_info_loc.dot(Field::pColorAttachments, i);

        const VkImageLayout image_layout = color_attachment.imageLayout;
        if (image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
            image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) {
            skip |= LogError("VUID-VkRenderingInfo-colorAttachmentCount-06090", commandBuffer,
                             color_attachment_loc.dot(Field::imageLayout), "is %s.", string_VkImageLayout(image_layout));
        }

        const VkResolveModeFlagBits resolve_mode = color_attachment.resolveMode;
        const VkImageLayout resolve_image_layout = color_attachment.resolveImageLayout;
        if (resolve_mode != VK_RESOLVE_MODE_NONE) {
            if (resolve_image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
                resolve_image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) {
                skip |= LogError("VUID-VkRenderingInfo-colorAttachmentCount-06091", commandBuffer,
                                 color_attachment_loc.dot(Field::resolveImageLayout), "is %s and resolveMode is %s.",
                                 string_VkImageLayout(resolve_image_layout), string_VkResolveModeFlagBits(resolve_mode));
            }
        }

        if (IsExtEnabled(extensions.vk_khr_maintenance2)) {
            if (image_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL ||
                image_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL) {
                skip |= LogError("VUID-VkRenderingInfo-colorAttachmentCount-06096", commandBuffer,
                                 color_attachment_loc.dot(Field::imageLayout), "is %s.", string_VkImageLayout(image_layout));
            }

            if (resolve_mode != VK_RESOLVE_MODE_NONE) {
                if (resolve_image_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL ||
                    resolve_image_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL) {
                    skip |= LogError("VUID-VkRenderingInfo-colorAttachmentCount-06097", commandBuffer,
                                     color_attachment_loc.dot(Field::resolveImageLayout), "is %s and resolveMode is %s.",
                                     string_VkImageLayout(resolve_image_layout), string_VkResolveModeFlagBits(resolve_mode));
                }
            }
        }

        if (IsImageLayoutDepthOnly(image_layout) || IsImageLayoutStencilOnly(image_layout)) {
            skip |= LogError("VUID-VkRenderingInfo-colorAttachmentCount-06100", commandBuffer,
                             color_attachment_loc.dot(Field::imageLayout), "is %s.", string_VkImageLayout(image_layout));
        }

        if (resolve_mode != VK_RESOLVE_MODE_NONE) {
            if (resolve_image_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ||
                resolve_image_layout == VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL) {
                skip |= LogError("VUID-VkRenderingInfo-colorAttachmentCount-06101", commandBuffer,
                                 color_attachment_loc.dot(Field::resolveImageLayout), "is %s and resolveMode is %s.",
                                 string_VkImageLayout(resolve_image_layout), string_VkResolveModeFlagBits(resolve_mode));
            }
        }
    }

    return skip;
}

bool Device::ValidateBeginRenderingDepthAttachment(VkCommandBuffer commandBuffer, const VkRenderingInfo &rendering_info,
                                                   const Location &rendering_info_loc) const {
    bool skip = false;
    if (!rendering_info.pDepthAttachment || rendering_info.pDepthAttachment->imageView == VK_NULL_HANDLE) return skip;

    const VkRenderingAttachmentInfo &depth_attachment = *rendering_info.pDepthAttachment;
    const Location attachment_loc = rendering_info_loc.dot(Field::pDepthAttachment);

    if (depth_attachment.imageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        skip |= LogError("VUID-VkRenderingInfo-pDepthAttachment-06092", commandBuffer, attachment_loc.dot(Field::imageLayout),
                         "is VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL.");
    } else if (IsImageLayoutStencilOnly(depth_attachment.imageLayout)) {
        skip |= LogError("VUID-VkRenderingInfo-pDepthAttachment-07732", commandBuffer, attachment_loc.dot(Field::imageLayout),
                         "is %s.", string_VkImageLayout(depth_attachment.imageLayout));
    }

    if (depth_attachment.resolveMode != VK_RESOLVE_MODE_NONE) {
        const VkImageLayout resolve_layout = depth_attachment.resolveImageLayout;
        if (resolve_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
            skip |= LogError("VUID-VkRenderingInfo-pDepthAttachment-06093", commandBuffer,
                             attachment_loc.dot(Field::resolveImageLayout),
                             "is VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL and resolveMode is %s.",
                             string_VkResolveModeFlagBits(depth_attachment.resolveMode));
        } else if (IsImageLayoutStencilOnly(resolve_layout)) {
            skip |= LogError("VUID-VkRenderingInfo-pDepthAttachment-07733", commandBuffer,
                             attachment_loc.dot(Field::resolveImageLayout), "is %s and resolveMode is %s.",
                             string_VkImageLayout(resolve_layout), string_VkResolveModeFlagBits(depth_attachment.resolveMode));
        }

        if (IsExtEnabled(extensions.vk_khr_maintenance2) &&
            resolve_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL) {
            skip |= LogError("VUID-VkRenderingInfo-pDepthAttachment-06098", commandBuffer,
                             attachment_loc.dot(Field::resolveImageLayout),
                             "is VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL.");
        }

        if (!(depth_attachment.resolveMode & phys_dev_props_core12.supportedDepthResolveModes)) {
            skip |=
                LogError("VUID-VkRenderingInfo-pDepthAttachment-06102", commandBuffer, attachment_loc.dot(Field::resolveMode),
                         "is %s, but supportedDepthResolveModes is %s.", string_VkResolveModeFlagBits(depth_attachment.resolveMode),
                         string_VkResolveModeFlags(phys_dev_props_core12.supportedDepthResolveModes).c_str());
        }
    }

    return skip;
}

bool Device::ValidateBeginRenderingStencilAttachment(VkCommandBuffer commandBuffer, const VkRenderingInfo &rendering_info,
                                                     const Location &rendering_info_loc) const {
    bool skip = false;
    if (!rendering_info.pStencilAttachment || rendering_info.pStencilAttachment->imageView == VK_NULL_HANDLE) return skip;

    const VkRenderingAttachmentInfo &stencil_attachment = *rendering_info.pStencilAttachment;
    const Location attachment_loc = rendering_info_loc.dot(Field::pStencilAttachment);

    if (stencil_attachment.imageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        skip |= LogError("VUID-VkRenderingInfo-pStencilAttachment-06094", commandBuffer, attachment_loc.dot(Field::imageLayout),
                         "is VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL.");
    } else if (IsImageLayoutDepthOnly(stencil_attachment.imageLayout)) {
        skip |= LogError("VUID-VkRenderingInfo-pStencilAttachment-07734", commandBuffer, attachment_loc.dot(Field::imageLayout),
                         "is %s.", string_VkImageLayout(stencil_attachment.imageLayout));
    }

    if (stencil_attachment.resolveMode != VK_RESOLVE_MODE_NONE) {
        const VkImageLayout resolve_layout = stencil_attachment.resolveImageLayout;
        if (resolve_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
            skip |= LogError("VUID-VkRenderingInfo-pStencilAttachment-06095", commandBuffer,
                             attachment_loc.dot(Field::resolveImageLayout),
                             "is VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL and resolveMode is %s.",
                             string_VkResolveModeFlagBits(stencil_attachment.resolveMode));
        } else if (IsImageLayoutDepthOnly(resolve_layout)) {
            skip |= LogError("VUID-VkRenderingInfo-pStencilAttachment-07735", commandBuffer,
                             attachment_loc.dot(Field::resolveImageLayout), "is %s and resolveMode is %s.",
                             string_VkImageLayout(resolve_layout), string_VkResolveModeFlagBits(stencil_attachment.resolveMode));
        }

        if (IsExtEnabled(extensions.vk_khr_maintenance2) &&
            resolve_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL) {
            skip |= LogError("VUID-VkRenderingInfo-pStencilAttachment-06099", commandBuffer,
                             attachment_loc.dot(Field::resolveImageLayout),
                             "is VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL.");
        }

        if (!(stencil_attachment.resolveMode & phys_dev_props_core12.supportedStencilResolveModes)) {
            skip |= LogError("VUID-VkRenderingInfo-pStencilAttachment-06103", commandBuffer, attachment_loc.dot(Field::resolveMode),
                             "is %s, but supportedStencilResolveModes is %s.",
                             string_VkResolveModeFlagBits(stencil_attachment.resolveMode),
                             string_VkResolveModeFlags(phys_dev_props_core12.supportedStencilResolveModes).c_str());
        }
    }

    return skip;
}

bool Device::ValidateBeginRenderingFragmentShadingRateAttachment(
    VkCommandBuffer commandBuffer, const VkRenderingInfo &rendering_info,
    const VkRenderingFragmentShadingRateAttachmentInfoKHR &rendering_fsr_attachment_info,
    const Location &rendering_info_loc) const {
    bool skip = false;
    if (rendering_fsr_attachment_info.imageView == VK_NULL_HANDLE) return skip;

    if (UniqueRenderingInfoImageViews(rendering_info, rendering_fsr_attachment_info.imageView) == false) {
        skip |= LogError("VUID-VkRenderingInfo-imageView-06125", commandBuffer,
                         rendering_info_loc.pNext(Struct::VkRenderingFragmentShadingRateAttachmentInfoKHR, Field::imageView),
                         "is %s.", FormatHandle(rendering_fsr_attachment_info.imageView).c_str());
    }

    const VkImageLayout image_layout = rendering_fsr_attachment_info.imageLayout;
    if (image_layout != VK_IMAGE_LAYOUT_GENERAL && image_layout != VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR) {
        skip |= LogError("VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06147", commandBuffer,
                         rendering_info_loc.pNext(Struct::VkRenderingFragmentShadingRateAttachmentInfoKHR, Field::layout),
                         "is (%s).", string_VkImageLayout(image_layout));
    }

    if (!IsPowerOfTwo(rendering_fsr_attachment_info.shadingRateAttachmentTexelSize.width)) {
        skip |= LogError(
            "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06149", commandBuffer,
            rendering_info_loc.pNext(Struct::VkRenderingFragmentShadingRateAttachmentInfoKHR, Field::shadingRateAttachmentTexelSize)
                .dot(Field::width),
            "(%" PRIu32 ") must be a power of two.", rendering_fsr_attachment_info.shadingRateAttachmentTexelSize.width);
    }

    const uint32_t max_frs_attach_texel_width =
        phys_dev_ext_props.fragment_shading_rate_props.maxFragmentShadingRateAttachmentTexelSize.width;
    if (rendering_fsr_attachment_info.shadingRateAttachmentTexelSize.width > max_frs_attach_texel_width) {
        skip |= LogError(
            "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06150", commandBuffer,
            rendering_info_loc.pNext(Struct::VkRenderingFragmentShadingRateAttachmentInfoKHR, Field::shadingRateAttachmentTexelSize)
                .dot(Field::width),
            "(%" PRIu32
            ") must be less than or equal to "
            "maxFragmentShadingRateAttachmentTexelSize.width (%" PRIu32 ").",
            rendering_fsr_attachment_info.shadingRateAttachmentTexelSize.width, max_frs_attach_texel_width);
    }

    const uint32_t min_frs_attach_texel_width =
        phys_dev_ext_props.fragment_shading_rate_props.minFragmentShadingRateAttachmentTexelSize.width;
    if (rendering_fsr_attachment_info.shadingRateAttachmentTexelSize.width < min_frs_attach_texel_width) {
        skip |= LogError(
            "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06151", commandBuffer,
            rendering_info_loc.pNext(Struct::VkRenderingFragmentShadingRateAttachmentInfoKHR, Field::shadingRateAttachmentTexelSize)
                .dot(Field::width),
            "(%" PRIu32
            ") must be greater than or equal to "
            "minFragmentShadingRateAttachmentTexelSize.width (%" PRIu32 ").",
            rendering_fsr_attachment_info.shadingRateAttachmentTexelSize.width, min_frs_attach_texel_width);
    }

    if (!IsPowerOfTwo(rendering_fsr_attachment_info.shadingRateAttachmentTexelSize.height)) {
        skip |= LogError(
            "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06152", commandBuffer,
            rendering_info_loc.pNext(Struct::VkRenderingFragmentShadingRateAttachmentInfoKHR, Field::shadingRateAttachmentTexelSize)
                .dot(Field::height),
            "(%" PRIu32 ") must be a power of two.", rendering_fsr_attachment_info.shadingRateAttachmentTexelSize.height);
    }

    const uint32_t max_frs_attach_texel_height =
        phys_dev_ext_props.fragment_shading_rate_props.maxFragmentShadingRateAttachmentTexelSize.height;
    if (rendering_fsr_attachment_info.shadingRateAttachmentTexelSize.height > max_frs_attach_texel_height) {
        skip |= LogError(
            "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06153", commandBuffer,
            rendering_info_loc.pNext(Struct::VkRenderingFragmentShadingRateAttachmentInfoKHR, Field::shadingRateAttachmentTexelSize)
                .dot(Field::height),
            "(%" PRIu32
            ") must be less than or equal to "
            "maxFragmentShadingRateAttachmentTexelSize.height (%" PRIu32 ").",
            rendering_fsr_attachment_info.shadingRateAttachmentTexelSize.height, max_frs_attach_texel_height);
    }

    const uint32_t min_frs_attach_texel_height =
        phys_dev_ext_props.fragment_shading_rate_props.minFragmentShadingRateAttachmentTexelSize.height;
    if (rendering_fsr_attachment_info.shadingRateAttachmentTexelSize.height < min_frs_attach_texel_height) {
        skip |= LogError(
            "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06154", commandBuffer,
            rendering_info_loc.pNext(Struct::VkRenderingFragmentShadingRateAttachmentInfoKHR, Field::shadingRateAttachmentTexelSize)
                .dot(Field::height),
            "(%" PRIu32
            ") must be greater than or equal to "
            "minFragmentShadingRateAttachmentTexelSize.height (%" PRIu32 ").",
            rendering_fsr_attachment_info.shadingRateAttachmentTexelSize.height, min_frs_attach_texel_height);
    }

    const uint32_t max_frs_attach_texel_aspect_ratio =
        phys_dev_ext_props.fragment_shading_rate_props.maxFragmentShadingRateAttachmentTexelSizeAspectRatio;
    if ((rendering_fsr_attachment_info.shadingRateAttachmentTexelSize.width /
         rendering_fsr_attachment_info.shadingRateAttachmentTexelSize.height) > max_frs_attach_texel_aspect_ratio) {
        skip |= LogError("VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06155", commandBuffer,
                         rendering_info_loc.pNext(Struct::VkRenderingFragmentShadingRateAttachmentInfoKHR,
                                                  Field::shadingRateAttachmentTexelSize),
                         "the quotient of width (%" PRIu32 ") and height (%" PRIu32
                         ") "
                         "must be less than or equal to maxFragmentShadingRateAttachmentTexelSizeAspectRatio (%" PRIu32 ").",
                         rendering_fsr_attachment_info.shadingRateAttachmentTexelSize.width,
                         rendering_fsr_attachment_info.shadingRateAttachmentTexelSize.height, max_frs_attach_texel_aspect_ratio);
    }

    if ((rendering_fsr_attachment_info.shadingRateAttachmentTexelSize.height /
         rendering_fsr_attachment_info.shadingRateAttachmentTexelSize.width) > max_frs_attach_texel_aspect_ratio) {
        skip |= LogError("VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06156", commandBuffer,
                         rendering_info_loc.pNext(Struct::VkRenderingFragmentShadingRateAttachmentInfoKHR,
                                                  Field::shadingRateAttachmentTexelSize),
                         "the quotient of height (%" PRIu32 ") and width (%" PRIu32
                         ") "
                         "must be less than or equal to maxFragmentShadingRateAttachmentTexelSizeAspectRatio (%" PRIu32 ").",
                         rendering_fsr_attachment_info.shadingRateAttachmentTexelSize.height,
                         rendering_fsr_attachment_info.shadingRateAttachmentTexelSize.width, max_frs_attach_texel_aspect_ratio);
    }

    return skip;
}
}  // namespace stateless
