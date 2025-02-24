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
#include <vector>

#include <vulkan/vk_enum_string_helper.h>
#include "core_validation.h"
#include "cc_vuid_maps.h"
#include "error_message/error_strings.h"
#include "generated/error_location_helper.h"
#include "sync/sync_vuid_maps.h"
#include "utils/image_layout_utils.h"
#include "state_tracker/image_state.h"
#include "state_tracker/render_pass_state.h"
#include "drawdispatch/drawdispatch_vuids.h"

bool IsValidAspectMaskForFormat(VkImageAspectFlags aspect_mask, VkFormat format);

using LayoutRange = image_layout_map::ImageLayoutRegistry::RangeType;
using LayoutEntry = image_layout_map::ImageLayoutRegistry::LayoutEntry;

// Utility type for checking Image layouts
struct LayoutUseCheckAndMessage {
    const static VkImageAspectFlags kDepthOrStencil = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    const VkImageLayout expected_layout;
    const VkImageAspectFlags aspect_mask;
    const char *message;
    VkImageLayout layout;

    LayoutUseCheckAndMessage() = delete;
    LayoutUseCheckAndMessage(VkImageLayout expected, const VkImageAspectFlags aspect_mask_ = 0)
        : expected_layout{expected}, aspect_mask{aspect_mask_}, message(nullptr), layout(kInvalidLayout) {}
    bool Check(const LayoutEntry &layout_entry) {
        message = nullptr;
        layout = kInvalidLayout;  // Success status
        if (layout_entry.current_layout != kInvalidLayout) {
            if (!ImageLayoutMatches(aspect_mask, expected_layout, layout_entry.current_layout)) {
                message = "previous known";
                layout = layout_entry.current_layout;
            }
        } else if (layout_entry.initial_layout != kInvalidLayout) {
            if (!ImageLayoutMatches(aspect_mask, expected_layout, layout_entry.initial_layout)) {
                assert(layout_entry.state);  // If we have an initial layout, we better have a state for it
                if (!((layout_entry.state->aspect_mask & kDepthOrStencil) &&
                      ImageLayoutMatches(layout_entry.state->aspect_mask, expected_layout, layout_entry.initial_layout))) {
                    message = "previously used";
                    layout = layout_entry.initial_layout;
                }
            }
        }
        return layout == kInvalidLayout;
    }
};

template <typename RangeFactory>
bool CoreChecks::VerifyImageLayoutRange(const vvl::CommandBuffer &cb_state, const vvl::Image &image_state,
                                        VkImageAspectFlags aspect_mask, VkImageLayout explicit_layout,
                                        const RangeFactory &range_factory, const Location &loc, const char *mismatch_layout_vuid,
                                        bool *error) const {
    bool skip = false;
    const auto image_layout_registry = cb_state.GetImageLayoutRegistry(image_state.VkHandle());
    if (!image_layout_registry) return skip;

    LayoutUseCheckAndMessage layout_check(explicit_layout, aspect_mask);
    skip |= image_layout_registry->AnyInRange(
        range_factory(*image_layout_registry),
        [this, image_layout_registry, &cb_state, &image_state, &layout_check, mismatch_layout_vuid, loc, error](
            const LayoutRange &range, const LayoutEntry &state) {
            bool subres_skip = false;
            if (!layout_check.Check(state)) {
                *error = true;
                auto subres = image_layout_registry->Decode(range.begin);
                const LogObjectList objlist(cb_state.Handle(), image_state.Handle());
                subres_skip |= LogError(mismatch_layout_vuid, objlist, loc,
                                        "Cannot use %s (layer=%" PRIu32 " mip=%" PRIu32
                                        ") with specific layout %s that doesn't match the "
                                        "%s layout %s.",
                                        FormatHandle(image_state).c_str(), subres.arrayLayer, subres.mipLevel,
                                        string_VkImageLayout(layout_check.expected_layout), layout_check.message,
                                        string_VkImageLayout(layout_check.layout));
            }
            return subres_skip;
        });

    return skip;
}

bool CoreChecks::VerifyImageLayoutSubresource(const vvl::CommandBuffer &cb_state, const vvl::Image &image_state,
                                              const VkImageSubresourceLayers &subresource_layers, VkImageLayout explicit_layout,
                                              const Location &loc, const char *vuid) const {
    if (disabled[image_layout_validation]) return false;
    bool skip = false;

    const VkImageSubresourceRange range = RangeFromLayers(subresource_layers);

    VkImageSubresourceRange normalized_isr = image_state.NormalizeSubresourceRange(range);
    auto range_factory = [&normalized_isr](const ImageLayoutRegistry &registry) { return registry.RangeGen(normalized_isr); };
    bool unused_error = false;
    skip |= VerifyImageLayoutRange(cb_state, image_state, normalized_isr.aspectMask, explicit_layout, range_factory, loc, vuid,
                                   &unused_error);
    return skip;
}

bool CoreChecks::VerifyImageLayout(const vvl::CommandBuffer &cb_state, const vvl::ImageView &image_view_state,
                                   VkImageLayout explicit_layout, const Location &loc, const char *mismatch_layout_vuid,
                                   bool *error) const {
    if (disabled[image_layout_validation]) return false;
    assert(image_view_state.image_state);
    auto range_factory = [&image_view_state](const ImageLayoutRegistry &registry) {
        return image_layout_map::RangeGenerator(image_view_state.range_generator);
    };

    return VerifyImageLayoutRange(cb_state, *image_view_state.image_state, image_view_state.create_info.subresourceRange.aspectMask,
                                  explicit_layout, range_factory, loc, mismatch_layout_vuid, error);
}

bool CoreChecks::VerifyImageLayout(const vvl::CommandBuffer &cb_state, const vvl::Image &image_state,
                                   const VkImageSubresourceRange &range, VkImageLayout explicit_layout, const Location &loc,
                                   const char *mismatch_layout_vuid, bool *error) const {
    if (disabled[image_layout_validation]) return false;

    auto range_factory = [&range](const ImageLayoutRegistry &registry) { return registry.RangeGen(range); };

    return VerifyImageLayoutRange(cb_state, image_state, range.aspectMask, explicit_layout, range_factory, loc,
                                  mismatch_layout_vuid, error);
}

void CoreChecks::TransitionFinalSubpassLayouts(vvl::CommandBuffer &cb_state) {
    auto render_pass_state = cb_state.active_render_pass.get();
    auto framebuffer_state = cb_state.activeFramebuffer.get();
    if (!render_pass_state || !framebuffer_state) {
        return;
    }

    const VkRenderPassCreateInfo2 *render_pass_info = render_pass_state->create_info.ptr();
    for (uint32_t i = 0; i < render_pass_info->attachmentCount; ++i) {
        auto *view_state = cb_state.GetActiveAttachmentImageViewState(i);
        if (!view_state) continue;

        VkImageLayout stencil_layout = kInvalidLayout;
        const auto *attachment_description_stencil_layout =
            vku::FindStructInPNextChain<VkAttachmentDescriptionStencilLayout>(render_pass_info->pAttachments[i].pNext);
        if (attachment_description_stencil_layout) {
            stencil_layout = attachment_description_stencil_layout->stencilFinalLayout;
        }
        cb_state.SetImageViewLayout(*view_state, render_pass_info->pAttachments[i].finalLayout, stencil_layout);
    }
}

static GlobalImageLayoutRangeMap *GetLayoutRangeMap(GlobalImageLayoutMap &global_image_layout_map, const vvl::Image &image_state) {
    // This approach allows for a single hash lookup or/create new
    auto &layout_map = global_image_layout_map[&image_state];
    if (!layout_map) {
        layout_map.emplace(image_state.subresource_encoder.SubresourceCount());
    }
    return &(*layout_map);
}

// Helper to update the Global or Overlay layout map
struct GlobalLayoutUpdater {
    bool update(VkImageLayout &dst, const image_layout_map::ImageLayoutRegistry::LayoutEntry &src) const {
        if (src.current_layout != image_layout_map::kInvalidLayout && dst != src.current_layout) {
            dst = src.current_layout;
            return true;
        }
        return false;
    }

    std::optional<VkImageLayout> insert(const image_layout_map::ImageLayoutRegistry::LayoutEntry &src) const {
        std::optional<VkImageLayout> result;
        if (src.current_layout != image_layout_map::kInvalidLayout) {
            result.emplace(src.current_layout);
        }
        return result;
    }
};

// This validates that the initial layout specified in the command buffer for the IMAGE is the same as the global IMAGE layout
bool CoreChecks::ValidateCmdBufImageLayouts(const Location &loc, const vvl::CommandBuffer &cb_state,
                                            GlobalImageLayoutMap &global_image_layout_map) const {
    if (disabled[image_layout_validation]) return false;
    bool skip = false;
    // Iterate over the layout maps for each referenced image
    GlobalImageLayoutRangeMap empty_map(1);
    for (const auto &[image, image_layout_registry] : cb_state.image_layout_map) {
        if (!image_layout_registry) continue;
        const auto image_state = Get<vvl::Image>(image);
        if (!image_state) continue;

        // TODO - things like ANGLE might have external images which have their layouts transitioned implicitly
        // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8940
        if (image_state->external_memory_handle_types != 0) {
            continue;
        }

        const auto &layout_map = image_layout_registry->GetLayoutMap();
        // Validate the initial_uses for each subresource referenced
        if (layout_map.empty()) continue;

        auto *overlay_map = GetLayoutRangeMap(global_image_layout_map, *image_state);
        const auto *global_range_map = image_state->layout_range_map.get();
        ASSERT_AND_CONTINUE(global_range_map);
        auto global_range_map_guard = global_range_map->ReadLock();

        // Note: don't know if it would matter
        // if (global_range_map->empty() && overlay_map->empty()) // skip this next loop...;

        auto pos = layout_map.begin();
        const auto end = layout_map.end();
        sparse_container::parallel_iterator<const GlobalImageLayoutRangeMap> current_layout(*overlay_map, *global_range_map,
                                                                                            pos->first.begin);
        while (pos != end) {
            VkImageLayout initial_layout = pos->second.initial_layout;
            if (initial_layout == image_layout_map::kInvalidLayout) {
                continue;
            }

            VkImageLayout image_layout = kInvalidLayout;

            if (current_layout->range.empty()) break;  // When we are past the end of data in overlay and global... stop looking
            if (current_layout->pos_A->valid) {        // pos_A denotes the overlay map in the parallel iterator
                image_layout = current_layout->pos_A->lower_bound->second;
            } else if (current_layout->pos_B->valid) {  // pos_B denotes the global map in the parallel iterator
                image_layout = current_layout->pos_B->lower_bound->second;
            }
            const auto intersected_range = pos->first & current_layout->range;
            if (initial_layout == VK_IMAGE_LAYOUT_UNDEFINED) {
                // TODO: Set memory invalid which is in mem_tracker currently
            } else if (image_layout != initial_layout) {
                const auto aspect_mask = image_state->subresource_encoder.Decode(intersected_range.begin).aspectMask;
                const bool matches = ImageLayoutMatches(aspect_mask, image_layout, initial_layout);
                if (!matches) {
                    // We can report all the errors for the intersected range directly
                    for (auto index : sparse_container::range_view<decltype(intersected_range)>(intersected_range)) {
                        const auto subresource = image_state->subresource_encoder.Decode(index);
                        const LogObjectList objlist(cb_state.Handle(), image_state->Handle());
                        // TODO - We need a way to map the action command to which caused this error
                        const vvl::DrawDispatchVuid &vuid = GetDrawDispatchVuid(vvl::Func::vkCmdDraw);
                        skip |= LogError(
                            vuid.image_layout_09600, objlist, loc,
                            "command buffer %s expects %s (subresource: %s) to be in layout %s--instead, current layout is %s.",
                            FormatHandle(cb_state).c_str(), FormatHandle(*image_state).c_str(),
                            string_VkImageSubresource(subresource).c_str(), string_VkImageLayout(initial_layout),
                            string_VkImageLayout(image_layout));
                    }
                }
            }
            if (pos->first.includes(intersected_range.end)) {
                current_layout.seek(intersected_range.end);
            } else {
                ++pos;
                if (pos != end) {
                    current_layout.seek(pos->first.begin);
                }
            }
        }
        // Update all layout set operations (which will be a subset of the initial_layouts)
        sparse_container::splice(*overlay_map, layout_map, GlobalLayoutUpdater());
    }

    return skip;
}

void CoreChecks::UpdateCmdBufImageLayouts(const vvl::CommandBuffer &cb_state) {
    for (const auto &[image, image_layout_registry] : cb_state.image_layout_map) {
        const auto image_state = Get<vvl::Image>(image);
        if (image_state && image_layout_registry && image_state->GetId() == image_layout_registry->GetImageId()) {
            auto guard = image_state->layout_range_map->WriteLock();
            sparse_container::splice(*image_state->layout_range_map, image_layout_registry->GetLayoutMap(), GlobalLayoutUpdater());
        }
    }
}

// ValidateLayoutVsAttachmentDescription is a general function where we can validate various state associated with the
// VkAttachmentDescription structs that are used by the sub-passes of a renderpass. Initial check is to make sure that READ_ONLY
// layout attachments don't have CLEAR as their loadOp.
bool CoreChecks::ValidateLayoutVsAttachmentDescription(const VkImageLayout first_layout, const uint32_t attachment,
                                                       const VkAttachmentDescription2 &attachment_description,
                                                       const Location &layout_loc) const {
    bool skip = false;
    const bool use_rp2 = layout_loc.function != Func::vkCreateRenderPass;

    // Verify that initial loadOp on READ_ONLY attachments is not CLEAR
    // for both loadOp and stencilLoaOp rp2 has it in 1 VU while rp1 has it in 2 VU with half behind Maintenance2 extension
    // Each is VUID is below in following order: rp2 -> rp1 with Maintenance2 -> rp1 with no extenstion
    if (attachment_description.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) {
        if (use_rp2 && ((first_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) ||
                        (first_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) ||
                        (first_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL))) {
            skip |= LogError("VUID-VkRenderPassCreateInfo2-pAttachments-02522", device, layout_loc,
                             "(%s) is an invalid for pAttachments[%d] (first attachment to have LOAD_OP_CLEAR).",
                             string_VkImageLayout(first_layout), attachment);
        } else if ((use_rp2 == false) && IsExtEnabled(extensions.vk_khr_maintenance2) &&
                   (first_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL)) {
            skip |= LogError("VUID-VkRenderPassCreateInfo-pAttachments-01566", device, layout_loc,
                             "(%s) is an invalid for pAttachments[%d] (first attachment to have LOAD_OP_CLEAR).",
                             string_VkImageLayout(first_layout), attachment);
        } else if ((use_rp2 == false) && ((first_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) ||
                                          (first_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))) {
            skip |= LogError("VUID-VkRenderPassCreateInfo-pAttachments-00836", device, layout_loc,
                             "(%s) is an invalid for pAttachments[%d] (first attachment to have LOAD_OP_CLEAR).",
                             string_VkImageLayout(first_layout), attachment);
        }
    }

    // Same as above for loadOp, but for stencilLoadOp
    if (attachment_description.stencilLoadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) {
        if (use_rp2 && ((first_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) ||
                        (first_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) ||
                        (first_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL))) {
            skip |= LogError("VUID-VkRenderPassCreateInfo2-pAttachments-02523", device, layout_loc,
                             "(%s) is an invalid for pAttachments[%d] (first attachment to have LOAD_OP_CLEAR).",
                             string_VkImageLayout(first_layout), attachment);
        } else if ((use_rp2 == false) && IsExtEnabled(extensions.vk_khr_maintenance2) &&
                   (first_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL)) {
            skip |= LogError("VUID-VkRenderPassCreateInfo-pAttachments-01567", device, layout_loc,
                             "(%s) is an invalid for pAttachments[%d] (first attachment to have LOAD_OP_CLEAR).",
                             string_VkImageLayout(first_layout), attachment);
        } else if ((use_rp2 == false) && ((first_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) ||
                                          (first_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))) {
            skip |= LogError("VUID-VkRenderPassCreateInfo-pAttachments-02511", device, layout_loc,
                             "(%s) is an invalid for pAttachments[%d] (first attachment to have LOAD_OP_CLEAR).",
                             string_VkImageLayout(first_layout), attachment);
        }
    }

    return skip;
}

bool CoreChecks::ValidateMultipassRenderedToSingleSampledSampleCount(VkFramebuffer framebuffer, VkRenderPass renderpass,
                                                                     vvl::Image &image_state, VkSampleCountFlagBits msrtss_samples,
                                                                     const Location &rasterization_samples_loc) const {
    bool skip = false;
    const auto image_create_info = image_state.create_info;
    if (!image_state.image_format_properties.sampleCounts) {
        skip |= GetPhysicalDeviceImageFormatProperties(image_state, "VUID-VkRenderPassAttachmentBeginInfo-pAttachments-07010",
                                                       rasterization_samples_loc);
    }
    if (!(image_state.image_format_properties.sampleCounts & msrtss_samples)) {
        const LogObjectList objlist(renderpass, framebuffer, image_state.Handle());
        skip |= LogError("VUID-VkRenderPassAttachmentBeginInfo-pAttachments-07010", objlist, rasterization_samples_loc,
                         "is %s but is not supported with image (%s) created with\n"
                         "format: %s\n"
                         "imageType: %s\n"
                         "tiling: %s\n"
                         "usage: %s\n"
                         "flags: %s\n",
                         string_VkSampleCountFlagBits(msrtss_samples), FormatHandle(image_state).c_str(),
                         string_VkFormat(image_create_info.format), string_VkImageType(image_create_info.imageType),
                         string_VkImageTiling(image_create_info.tiling), string_VkImageUsageFlags(image_create_info.usage).c_str(),
                         string_VkImageCreateFlags(image_create_info.flags).c_str());
    }
    return skip;
}

bool CoreChecks::ValidateRenderPassLayoutAgainstFramebufferImageUsage(VkImageLayout layout, const vvl::ImageView &image_view_state,
                                                                      VkFramebuffer framebuffer, VkRenderPass renderpass,
                                                                      uint32_t attachment_index, const Location &rp_loc,
                                                                      const Location &attachment_reference_loc) const {
    bool skip = false;
    const auto &image_view = image_view_state.Handle();
    const auto *image_state = image_view_state.image_state.get();
    if (!image_state) {
        return skip;  // validated at VUID-VkRenderPassBeginInfo-framebuffer-parameter
    }
    const auto &image = image_state->Handle();
    const bool use_rp2 = rp_loc.function != Func::vkCmdBeginRenderPass;

    auto image_usage = image_state->create_info.usage;
    const auto stencil_usage_info = vku::FindStructInPNextChain<VkImageStencilUsageCreateInfo>(image_state->create_info.pNext);
    if (stencil_usage_info) {
        image_usage |= stencil_usage_info->stencilUsage;
    }

    const char *vuid = kVUIDUndefined;

    // Check for layouts that mismatch image usages in the framebuffer
    if (layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && !(image_usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)) {
        vuid = use_rp2 ? "VUID-vkCmdBeginRenderPass2-initialLayout-03094" : "VUID-vkCmdBeginRenderPass-initialLayout-00895";
        skip = true;
    } else if (layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL &&
               !(image_usage & (VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT))) {
        vuid = use_rp2 ? "VUID-vkCmdBeginRenderPass2-initialLayout-03097" : "VUID-vkCmdBeginRenderPass-initialLayout-00897";
        skip = true;
    } else if (layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && !(image_usage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)) {
        vuid = use_rp2 ? "VUID-vkCmdBeginRenderPass2-initialLayout-03098" : "VUID-vkCmdBeginRenderPass-initialLayout-00898";
        skip = true;
    } else if (layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && !(image_usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
        vuid = use_rp2 ? "VUID-vkCmdBeginRenderPass2-initialLayout-03099" : "VUID-vkCmdBeginRenderPass-initialLayout-00899";
        skip = true;
    } else if (layout == VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT) {
        if (((image_usage & (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) == 0) ||
            ((image_usage & (VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)) == 0)) {
            vuid = use_rp2 ? "VUID-vkCmdBeginRenderPass2-initialLayout-07002" : "VUID-vkCmdBeginRenderPass-initialLayout-07000";
            skip = true;
        } else if (!(image_usage & VK_IMAGE_USAGE_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT)) {
            vuid = use_rp2 ? "VUID-vkCmdBeginRenderPass2-initialLayout-07003" : "VUID-vkCmdBeginRenderPass-initialLayout-07001";
            skip = true;
        }
    } else if ((layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL ||
                layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL ||
                layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
                layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) &&
               !(image_usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
        vuid = use_rp2 ? "VUID-vkCmdBeginRenderPass2-initialLayout-03096" : "VUID-vkCmdBeginRenderPass-initialLayout-01758";
        skip = true;
    } else if (layout == VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ && !IsShaderTileImageUsageValid(image_usage)) {
        vuid = use_rp2 ? "VUID-vkCmdBeginRenderPass2-initialLayout-09538" : "VUID-vkCmdBeginRenderPass-initialLayout-09537";
        skip = true;
    } else if ((IsImageLayoutDepthOnly(layout) || IsImageLayoutStencilOnly(layout)) &&
               !(image_usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
        vuid = use_rp2 ? "VUID-vkCmdBeginRenderPass2-initialLayout-02844" : "VUID-vkCmdBeginRenderPass-initialLayout-02842";
        skip = true;
    }

    if (skip) {
        std::stringstream stencil_usage_message;
        if (stencil_usage_info) {
            stencil_usage_message << " (which includes " << string_VkImageUsageFlags(stencil_usage_info->stencilUsage)
                                  << "from VkImageStencilUsageCreateInfo)";
        }
        const LogObjectList objlist(image, renderpass, framebuffer, image_view);
        return LogError(vuid, objlist, rp_loc,
                        "(%s) was created with %s = %s, but %s pAttachments[%" PRIu32 "] (%s) usage is %s%s.",
                        FormatHandle(renderpass).c_str(), attachment_reference_loc.Fields().c_str(), string_VkImageLayout(layout),
                        FormatHandle(framebuffer).c_str(), attachment_index, FormatHandle(image_view).c_str(),
                        string_VkImageUsageFlags(image_usage).c_str(), stencil_usage_message.str().c_str());
    }

    return skip;
}

bool CoreChecks::ValidateRenderPassStencilLayoutAgainstFramebufferImageUsage(VkImageLayout layout,
                                                                             const vvl::ImageView &image_view_state,
                                                                             VkFramebuffer framebuffer, VkRenderPass renderpass,
                                                                             const Location &layout_loc) const {
    bool skip = false;
    const auto &image_view = image_view_state.Handle();
    const auto *image_state = image_view_state.image_state.get();
    const auto &image = image_state->Handle();
    const bool use_rp2 = layout_loc.function != Func::vkCmdBeginRenderPass;

    if (!image_state) {
        return skip;  // validated at VUID-VkRenderPassBeginInfo-framebuffer-parameter
    }
    auto image_usage = image_state->create_info.usage;
    if (const auto stencil_usage_info =
            vku::FindStructInPNextChain<VkImageStencilUsageCreateInfo>(image_state->create_info.pNext)) {
        image_usage |= stencil_usage_info->stencilUsage;
    }

    if (IsImageLayoutStencilOnly(layout) && !(image_usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
        const char *vuid = use_rp2 ? "VUID-vkCmdBeginRenderPass2-stencilInitialLayout-02845"
                                   : "VUID-vkCmdBeginRenderPass-stencilInitialLayout-02843";
        const LogObjectList objlist(image, renderpass, framebuffer, image_view);
        skip |= LogError(vuid, objlist, layout_loc,
                         "is %s but the image attached to %s via %s"
                         " was created with %s (not VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT).",
                         string_VkImageLayout(layout), FormatHandle(framebuffer).c_str(), FormatHandle(image_view).c_str(),
                         string_VkImageUsageFlags(image_usage).c_str());
    }

    return skip;
}

bool CoreChecks::VerifyFramebufferAndRenderPassLayouts(const vvl::CommandBuffer &cb_state, const VkRenderPassBeginInfo &begin_info,
                                                       const vvl::RenderPass &render_pass_state,
                                                       const vvl::Framebuffer &framebuffer_state,
                                                       const Location &rp_begin_loc) const {
    bool skip = false;
    const auto *render_pass_info = render_pass_state.create_info.ptr();
    const VkRenderPass render_pass = render_pass_state.VkHandle();
    auto const &framebuffer_info = framebuffer_state.create_info;
    const VkImageView *attachments = framebuffer_info.pAttachments;

    const VkFramebuffer framebuffer = framebuffer_state.VkHandle();

    if (render_pass_info->attachmentCount != framebuffer_info.attachmentCount) {
        const LogObjectList objlist(render_pass, framebuffer_state.Handle());
        // VU bieng worked on at https://gitlab.khronos.org/vulkan/vulkan/-/issues/2267
        skip |= LogError("UNASSIGNED-CoreValidation-DrawState-InvalidRenderpass", objlist, rp_begin_loc,
                         "You cannot start a render pass using a framebuffer with a different number of attachments (%" PRIu32
                         " vs %" PRIu32 ").",
                         render_pass_info->attachmentCount, framebuffer_info.attachmentCount);
    }

    const auto *attachment_info = vku::FindStructInPNextChain<VkRenderPassAttachmentBeginInfo>(begin_info.pNext);
    if (((framebuffer_info.flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT) != 0) && attachment_info != nullptr) {
        attachments = attachment_info->pAttachments;
    }

    if (attachments == nullptr) {
        return skip;
    }

    // Have the location where the VkRenderPass is reference, and where in it's creation the error occured
    const Location rp_loc = rp_begin_loc.dot(Field::renderPass);
    // only printing Fields, but use same Function to make getting correct VUID easier
    const Location rp_create_info(rp_begin_loc.function, Field::pCreateInfo);

    for (uint32_t i = 0; i < render_pass_info->attachmentCount && i < framebuffer_info.attachmentCount; ++i) {
        const Location attachment_loc = rp_create_info.dot(Field::pAttachments, i);
        auto image_view = attachments[i];
        auto view_state = Get<vvl::ImageView>(image_view);

        if (!view_state) {
            const LogObjectList objlist(render_pass, framebuffer_state.Handle(), image_view);
            skip |= LogError("VUID-VkRenderPassBeginInfo-framebuffer-parameter", objlist, attachment_loc, "%s is invalid.",
                             FormatHandle(image_view).c_str());
            continue;
        }

        const VkImage image = view_state->create_info.image;
        const auto *image_state = view_state->image_state.get();

        if (!image_state) {
            const LogObjectList objlist(render_pass, framebuffer_state.Handle(), image_view, image);
            skip |= LogError("VUID-VkRenderPassBeginInfo-framebuffer-parameter", objlist, attachment_loc,
                             "%s references invalid image (%s).", FormatHandle(image_view).c_str(), FormatHandle(image).c_str());
            continue;
        }
        if (image_state->IsSwapchainImage() && image_state->owned_by_swapchain && !image_state->bind_swapchain) {
            const LogObjectList objlist(render_pass, framebuffer_state.Handle(), image_view, image);
            skip |= LogError("VUID-VkRenderPassBeginInfo-framebuffer-parameter", objlist, attachment_loc,
                             "%s references a swapchain image (%s) from a swapchain that has been destroyed.",
                             FormatHandle(image_view).c_str(), FormatHandle(image).c_str());
            continue;
        }
        auto attachment_initial_layout = render_pass_info->pAttachments[i].initialLayout;
        auto attachment_final_layout = render_pass_info->pAttachments[i].finalLayout;

        // Default to expecting stencil in the same layout.
        auto attachment_stencil_initial_layout = attachment_initial_layout;

        // If a separate layout is specified, look for that.
        const auto *attachment_desc_stencil_layout =
            vku::FindStructInPNextChain<VkAttachmentDescriptionStencilLayout>(render_pass_info->pAttachments[i].pNext);
        if (const auto *attachment_description_stencil_layout =
                vku::FindStructInPNextChain<VkAttachmentDescriptionStencilLayout>(render_pass_info->pAttachments[i].pNext);
            attachment_description_stencil_layout) {
            attachment_stencil_initial_layout = attachment_description_stencil_layout->stencilInitialLayout;
        }

        std::shared_ptr<const ImageLayoutRegistry> image_layout_registry;
        bool has_queried_map = false;

        for (uint32_t aspect_index = 0; aspect_index < 32; aspect_index++) {
            VkImageAspectFlags test_aspect = 1u << aspect_index;
            if ((view_state->normalized_subresource_range.aspectMask & test_aspect) == 0) {
                continue;
            }

            // Allow for differing depth and stencil layouts
            VkImageLayout check_layout = attachment_initial_layout;
            if (test_aspect == VK_IMAGE_ASPECT_STENCIL_BIT) {
                check_layout = attachment_stencil_initial_layout;
            }

            // If no layout information for image yet, will be checked at QueueSubmit time
            if (check_layout == VK_IMAGE_LAYOUT_UNDEFINED) {
                continue;
            }
            if (!has_queried_map) {
                image_layout_registry = cb_state.GetImageLayoutRegistry(image_state->VkHandle());
                has_queried_map = true;
            }
            if (!image_layout_registry) {
                // If no layout information for image yet, will be checked at QueueSubmit time
                continue;
            }
            auto normalized_range = view_state->normalized_subresource_range;
            normalized_range.aspectMask = test_aspect;
            LayoutUseCheckAndMessage layout_check(check_layout, test_aspect);

            skip |= image_layout_registry->AnyInRange(
                normalized_range, [this, &layout_check, i, cb = cb_state.Handle(), render_pass = render_pass,
                                   framebuffer = framebuffer_state.Handle(), image = view_state->image_state->Handle(),
                                   image_view = view_state->Handle(), attachment_loc,
                                   rp_begin_loc](const LayoutRange &range, const LayoutEntry &state) {
                    bool subres_skip = false;
                    if (!layout_check.Check(state)) {
                        const LogObjectList objlist(cb, render_pass, framebuffer, image, image_view);
                        const char *vuid = rp_begin_loc.function != Func::vkCmdBeginRenderPass
                                               ? "VUID-vkCmdBeginRenderPass2-initialLayout-03100"
                                               : "VUID-vkCmdBeginRenderPass-initialLayout-00900";
                        subres_skip |= LogError(
                            vuid, objlist, attachment_loc,
                            "You cannot start a render pass using attachment %" PRIu32
                            " where the render pass initial layout is %s and the %s layout of the attachment is %s. The layouts "
                            "must match, or the render pass initial layout for the attachment must be VK_IMAGE_LAYOUT_UNDEFINED.",
                            i, string_VkImageLayout(layout_check.expected_layout), layout_check.message,
                            string_VkImageLayout(layout_check.layout));
                    }
                    return subres_skip;
                });
        }
        skip |= ValidateRenderPassLayoutAgainstFramebufferImageUsage(
            attachment_initial_layout, *view_state, framebuffer, render_pass, i, rp_loc, attachment_loc.dot(Field::initialLayout));

        skip |= ValidateRenderPassLayoutAgainstFramebufferImageUsage(attachment_final_layout, *view_state, framebuffer, render_pass,
                                                                     i, rp_loc, attachment_loc.dot(Field::finalLayout));

        if (attachment_desc_stencil_layout != nullptr) {
            skip |= ValidateRenderPassStencilLayoutAgainstFramebufferImageUsage(
                attachment_desc_stencil_layout->stencilInitialLayout, *view_state, framebuffer, render_pass,
                attachment_loc.pNext(Struct::VkAttachmentDescriptionStencilLayout, Field::stencilInitialLayout));
            skip |= ValidateRenderPassStencilLayoutAgainstFramebufferImageUsage(
                attachment_desc_stencil_layout->stencilFinalLayout, *view_state, framebuffer, render_pass,
                attachment_loc.pNext(Struct::VkAttachmentDescriptionStencilLayout, Field::stencilFinalLayout));
        }
    }

    for (uint32_t j = 0; j < render_pass_info->subpassCount; ++j) {
        const Location subpass_loc = rp_create_info.dot(Field::pSubpasses, j);
        auto &subpass = render_pass_info->pSubpasses[j];
        const auto *ms_rendered_to_single_sampled =
            vku::FindStructInPNextChain<VkMultisampledRenderToSingleSampledInfoEXT>(render_pass_info->pSubpasses[j].pNext);
        for (uint32_t k = 0; k < render_pass_info->pSubpasses[j].inputAttachmentCount; ++k) {
            auto &attachment_ref = subpass.pInputAttachments[k];
            if (attachment_ref.attachment == VK_ATTACHMENT_UNUSED) continue;
            const Location input_loc = subpass_loc.dot(Field::pInputAttachments, k);
            auto image_view = attachments[attachment_ref.attachment];

            if (auto view_state = Get<vvl::ImageView>(image_view)) {
                skip |= ValidateRenderPassLayoutAgainstFramebufferImageUsage(attachment_ref.layout, *view_state, framebuffer,
                                                                             render_pass, attachment_ref.attachment, rp_loc,
                                                                             input_loc.dot(Field::layout));

                if (ms_rendered_to_single_sampled && ms_rendered_to_single_sampled->multisampledRenderToSingleSampledEnable) {
                    if (render_pass_info->pAttachments[attachment_ref.attachment].samples == VK_SAMPLE_COUNT_1_BIT) {
                        skip |= ValidateMultipassRenderedToSingleSampledSampleCount(
                            framebuffer, render_pass, *view_state->image_state, ms_rendered_to_single_sampled->rasterizationSamples,
                            subpass_loc.pNext(Struct::VkMultisampledRenderToSingleSampledInfoEXT, Field::rasterizationSamples));
                    }
                }
            }
        }

        for (uint32_t k = 0; k < render_pass_info->pSubpasses[j].colorAttachmentCount; ++k) {
            auto &attachment_ref = subpass.pColorAttachments[k];
            if (attachment_ref.attachment == VK_ATTACHMENT_UNUSED) continue;
            const Location color_attachment_loc = subpass_loc.dot(Field::pColorAttachments, k);
            auto image_view = attachments[attachment_ref.attachment];

            if (auto view_state = Get<vvl::ImageView>(image_view)) {
                skip |= ValidateRenderPassLayoutAgainstFramebufferImageUsage(attachment_ref.layout, *view_state, framebuffer,
                                                                             render_pass, attachment_ref.attachment, rp_loc,
                                                                             color_attachment_loc.dot(Field::layout));
                if (subpass.pResolveAttachments) {
                    skip |= ValidateRenderPassLayoutAgainstFramebufferImageUsage(
                        attachment_ref.layout, *view_state, framebuffer, render_pass, attachment_ref.attachment, rp_loc,
                        subpass_loc.dot(Field::pResolveAttachments, k).dot(Field::layout));
                }

                if (ms_rendered_to_single_sampled && ms_rendered_to_single_sampled->multisampledRenderToSingleSampledEnable) {
                    if (render_pass_info->pAttachments[attachment_ref.attachment].samples == VK_SAMPLE_COUNT_1_BIT) {
                        skip |= ValidateMultipassRenderedToSingleSampledSampleCount(
                            framebuffer, render_pass, *view_state->image_state, ms_rendered_to_single_sampled->rasterizationSamples,
                            subpass_loc.pNext(Struct::VkMultisampledRenderToSingleSampledInfoEXT, Field::rasterizationSamples));
                    }
                }
            }
        }

        if (render_pass_info->pSubpasses[j].pDepthStencilAttachment) {
            auto &attachment_ref = *subpass.pDepthStencilAttachment;
            if (attachment_ref.attachment == VK_ATTACHMENT_UNUSED) continue;
            const Location ds_loc = subpass_loc.dot(Field::pDepthStencilAttachment);
            auto image_view = attachments[attachment_ref.attachment];

            if (auto view_state = Get<vvl::ImageView>(image_view)) {
                skip |= ValidateRenderPassLayoutAgainstFramebufferImageUsage(attachment_ref.layout, *view_state, framebuffer,
                                                                             render_pass, attachment_ref.attachment, rp_loc,
                                                                             ds_loc.dot(Field::layout));

                if (const auto *stencil_layout =
                        vku::FindStructInPNextChain<VkAttachmentReferenceStencilLayout>(attachment_ref.pNext);
                    stencil_layout != nullptr) {
                    skip |= ValidateRenderPassStencilLayoutAgainstFramebufferImageUsage(
                        stencil_layout->stencilLayout, *view_state, framebuffer, render_pass,
                        ds_loc.pNext(Struct::VkAttachmentReferenceStencilLayout, Field::stencilLayout));
                }

                if (ms_rendered_to_single_sampled && ms_rendered_to_single_sampled->multisampledRenderToSingleSampledEnable) {
                    if (render_pass_info->pAttachments[attachment_ref.attachment].samples == VK_SAMPLE_COUNT_1_BIT) {
                        skip |= ValidateMultipassRenderedToSingleSampledSampleCount(
                            framebuffer, render_pass, *view_state->image_state, ms_rendered_to_single_sampled->rasterizationSamples,
                            subpass_loc.pNext(Struct::VkMultisampledRenderToSingleSampledInfoEXT, Field::rasterizationSamples));
                    }
                }
            }
        }
    }
    return skip;
}

void CoreChecks::TransitionAttachmentRefLayout(vvl::CommandBuffer &cb_state, const vku::safe_VkAttachmentReference2 &ref) {
    if (ref.attachment == VK_ATTACHMENT_UNUSED) return;
    vvl::ImageView *image_view = cb_state.GetActiveAttachmentImageViewState(ref.attachment);
    if (image_view) {
        VkImageLayout stencil_layout = kInvalidLayout;
        const auto *attachment_reference_stencil_layout =
            vku::FindStructInPNextChain<VkAttachmentReferenceStencilLayout>(ref.pNext);
        if (attachment_reference_stencil_layout) {
            stencil_layout = attachment_reference_stencil_layout->stencilLayout;
        }

        cb_state.SetImageViewLayout(*image_view, ref.layout, stencil_layout);
    }
}

void CoreChecks::TransitionSubpassLayouts(vvl::CommandBuffer &cb_state, const vvl::RenderPass &render_pass_state,
                                          const int subpass_index) {
    auto const &subpass = render_pass_state.create_info.pSubpasses[subpass_index];
    for (uint32_t j = 0; j < subpass.inputAttachmentCount; ++j) {
        TransitionAttachmentRefLayout(cb_state, subpass.pInputAttachments[j]);
    }
    for (uint32_t j = 0; j < subpass.colorAttachmentCount; ++j) {
        TransitionAttachmentRefLayout(cb_state, subpass.pColorAttachments[j]);
    }
    if (subpass.pDepthStencilAttachment) {
        TransitionAttachmentRefLayout(cb_state, *subpass.pDepthStencilAttachment);
    }
}

// Transition the layout state for renderpass attachments based on the BeginRenderPass() call. This includes:
// 1. Transition into initialLayout state
// 2. Transition from initialLayout to layout used in subpass 0
void CoreChecks::TransitionBeginRenderPassLayouts(vvl::CommandBuffer &cb_state, const vvl::RenderPass &render_pass_state) {
    // First record expected initialLayout as a potential initial layout usage.
    auto const rpci = render_pass_state.create_info.ptr();
    for (uint32_t i = 0; i < rpci->attachmentCount; ++i) {
        auto *view_state = cb_state.GetActiveAttachmentImageViewState(i);
        if (!view_state) continue;

        vvl::Image *image_state = view_state->image_state.get();
        ASSERT_AND_CONTINUE(image_state);

        const auto initial_layout = rpci->pAttachments[i].initialLayout;
        const auto *attachment_description_stencil_layout =
            vku::FindStructInPNextChain<VkAttachmentDescriptionStencilLayout>(rpci->pAttachments[i].pNext);
        if (attachment_description_stencil_layout) {
            const auto stencil_initial_layout = attachment_description_stencil_layout->stencilInitialLayout;
            VkImageSubresourceRange sub_range = view_state->normalized_subresource_range;
            sub_range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            cb_state.SetImageInitialLayout(*image_state, sub_range, initial_layout);
            sub_range.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
            cb_state.SetImageInitialLayout(*image_state, sub_range, stencil_initial_layout);
        } else {
            // If layoutStencil is kInvalidLayout (meaning no separate depth/stencil layout), image view format has both depth
            // and stencil aspects, and subresource has only one of aspect out of depth or stencil, then the missing aspect will
            // also be transitioned and thus must be included explicitly
            auto subresource_range = view_state->normalized_subresource_range;
            if (const VkFormat format = view_state->create_info.format; vkuFormatIsDepthAndStencil(format)) {
                if (subresource_range.aspectMask & (VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT)) {
                    subresource_range.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
                }
            }
            cb_state.SetImageInitialLayout(*image_state, subresource_range, initial_layout);
        }
    }
    // Now transition for first subpass (index 0)
    TransitionSubpassLayouts(cb_state, render_pass_state, 0);
}

bool CoreChecks::VerifyClearImageLayout(const vvl::CommandBuffer &cb_state, const vvl::Image &image_state,
                                        const VkImageSubresourceRange &range, VkImageLayout dest_image_layout,
                                        const Location &loc) const {
    bool skip = false;
    if (loc.function == Func::vkCmdClearDepthStencilImage) {
        if ((dest_image_layout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) && (dest_image_layout != VK_IMAGE_LAYOUT_GENERAL)) {
            LogObjectList objlist(cb_state.Handle(), image_state.Handle());
            skip |= LogError("VUID-vkCmdClearDepthStencilImage-imageLayout-00012", objlist, loc,
                             "Layout for cleared image is %s but can only be TRANSFER_DST_OPTIMAL or GENERAL.",
                             string_VkImageLayout(dest_image_layout));
        }

    } else if (loc.function == Func::vkCmdClearColorImage) {
        if ((dest_image_layout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) && (dest_image_layout != VK_IMAGE_LAYOUT_GENERAL) &&
            (dest_image_layout != VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR)) {
            LogObjectList objlist(cb_state.Handle(), image_state.Handle());
            skip |= LogError("VUID-vkCmdClearColorImage-imageLayout-01394", objlist, loc,
                             "Layout for cleared image is %s but can only be TRANSFER_DST_OPTIMAL, SHARED_PRESENT_KHR, or GENERAL.",
                             string_VkImageLayout(dest_image_layout));
        }
    }

    // Cast to const to prevent creation at validate time.
    const auto image_layout_registry = cb_state.GetImageLayoutRegistry(image_state.VkHandle());
    if (image_layout_registry) {
        LayoutUseCheckAndMessage layout_check(dest_image_layout);
        auto normalized_isr = image_state.NormalizeSubresourceRange(range);
        // IncrementInterval skips over all the subresources that have the same state as we just checked, incrementing to
        // the next "constant value" range
        skip |= image_layout_registry->AnyInRange(normalized_isr, [this, &cb_state, &layout_check, loc,
                                                                   image = image_state.Handle()](const LayoutRange &range,
                                                                                                 const LayoutEntry &state) {
            bool subres_skip = false;
            if (!layout_check.Check(state)) {
                const char *vuid = (loc.function == Func::vkCmdClearDepthStencilImage)
                                       ? "VUID-vkCmdClearDepthStencilImage-imageLayout-00011"
                                       : "VUID-vkCmdClearColorImage-imageLayout-00004";
                LogObjectList objlist(cb_state.Handle(), image);
                subres_skip |=
                    LogError(vuid, objlist, loc, "Cannot clear an image whose layout is %s and doesn't match the %s layout %s.",
                             string_VkImageLayout(layout_check.expected_layout), layout_check.message,
                             string_VkImageLayout(layout_check.layout));
            }
            return subres_skip;
        });
    }

    return skip;
}

bool CoreChecks::UpdateCommandBufferImageLayoutMap(const vvl::CommandBuffer &cb_state, const Location &image_loc,
                                                   const ImageBarrier &img_barrier, const vvl::CommandBuffer::ImageLayoutMap &current_map,
                                                   vvl::CommandBuffer::ImageLayoutMap &layout_updates) const {
    bool skip = false;
    auto image_state = Get<vvl::Image>(img_barrier.image);
    ASSERT_AND_RETURN_SKIP(image_state);

    std::shared_ptr<ImageLayoutRegistry> image_layout_registry;
    auto iter = layout_updates.find(image_state->VkHandle());
    bool new_write = false;
    if (iter == layout_updates.end()) {
        image_layout_registry = std::make_shared<ImageLayoutRegistry>(*image_state);
        new_write = true;
        layout_updates.emplace(image_state->VkHandle(), image_layout_registry);
    } else if (iter->second->GetImageId() != image_state->GetId()) {
        image_layout_registry = std::make_shared<ImageLayoutRegistry>(*image_state);
        iter->second = image_layout_registry;
        new_write = true;
    } else {
        image_layout_registry = iter->second;
    }
    const auto &current_subresource_map = current_map.find(image_state->VkHandle());
    const auto read_subresource_map =
        (new_write && current_subresource_map != current_map.end()) ? current_subresource_map->second : image_layout_registry;
    // Validate aspects in isolation.
    // This is required when handling separate depth-stencil layouts.
    for (uint32_t aspect_index = 0; aspect_index < 32; aspect_index++) {
        VkImageAspectFlags test_aspect = 1u << aspect_index;
        if ((img_barrier.subresourceRange.aspectMask & test_aspect) == 0) {
            continue;
        }
        auto old_layout = NormalizeSynchronization2Layout(img_barrier.subresourceRange.aspectMask, img_barrier.oldLayout);

        LayoutUseCheckAndMessage layout_check(old_layout, test_aspect);
        auto normalized_isr = image_state->NormalizeSubresourceRange(img_barrier.subresourceRange);
        normalized_isr.aspectMask = test_aspect;
        skip |=
            read_subresource_map->AnyInRange(normalized_isr, [this, read_subresource_map, &cb_state, &layout_check, &image_loc,
                                                              &img_barrier](const LayoutRange &range, const LayoutEntry &state) {
                bool subres_skip = false;
                if (!layout_check.Check(state)) {
                    const auto &vuid = GetImageBarrierVUID(image_loc, sync_vuid_maps::ImageError::kConflictingLayout);
                    auto subres = read_subresource_map->Decode(range.begin);
                    const LogObjectList objlist(cb_state.Handle(), img_barrier.image);
                    subres_skip =
                        LogError(vuid, objlist, image_loc,
                                 "(%s) cannot transition the layout of aspect=%" PRIu32 ", level=%" PRIu32 ", layer=%" PRIu32
                                 " from %s when the "
                                 "%s layout is %s.",
                                 FormatHandle(img_barrier.image).c_str(), subres.aspectMask, subres.mipLevel, subres.arrayLayer,
                                 string_VkImageLayout(img_barrier.oldLayout), layout_check.message,
                                 string_VkImageLayout(layout_check.layout));
                }
                return subres_skip;
            });
        image_layout_registry->SetSubresourceRangeLayout(cb_state, normalized_isr, img_barrier.newLayout);
    }
    return skip;
}

bool CoreChecks::FindLayouts(const vvl::Image &image_state, std::vector<VkImageLayout> &layouts) const {
    const auto *layout_range_map = image_state.layout_range_map.get();
    if (!layout_range_map) return false;

    auto guard = layout_range_map->ReadLock();
    // TODO: FindLayouts function should mutate into a ValidatePresentableLayout with the loop wrapping the LogError
    //       from the caller. You can then use decode to add the subresource of the range::begin to the error message.

    // TODO: what is this test and what is it supposed to do?! -- the logic doesn't match the comment below?!

    // TODO: Make this robust for >1 aspect mask. Now it will just say ignore potential errors in this case.
    if (layout_range_map->size() >= (image_state.create_info.arrayLayers * image_state.create_info.mipLevels + 1)) {
        return false;
    }

    for (const auto &entry : *layout_range_map) {
        layouts.emplace_back(entry.second);
    }
    return true;
}

void CoreChecks::RecordTransitionImageLayout(vvl::CommandBuffer &cb_state, const ImageBarrier &mem_barrier) {
    if (enabled_features.synchronization2) {
        if (mem_barrier.oldLayout == mem_barrier.newLayout) {
            return;
        }
    }
    auto image_state = Get<vvl::Image>(mem_barrier.image);
    ASSERT_AND_RETURN(image_state);

    auto normalized_isr = image_state->NormalizeSubresourceRange(mem_barrier.subresourceRange);

    VkImageLayout initial_layout = NormalizeSynchronization2Layout(mem_barrier.subresourceRange.aspectMask, mem_barrier.oldLayout);
    VkImageLayout new_layout = NormalizeSynchronization2Layout(mem_barrier.subresourceRange.aspectMask, mem_barrier.newLayout);

    // Layout transitions in external instance are not tracked, so don't validate initial layout.
    if (IsQueueFamilyExternal(mem_barrier.srcQueueFamilyIndex)) {
        initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    }

    // For ownership transfers, the barrier is specified twice; as a release
    // operation on the yielding queue family, and as an acquire operation
    // on the acquiring queue family. This barrier may also include a layout
    // transition, which occurs 'between' the two operations. For validation
    // purposes it doesn't seem important which side performs the layout
    // transition, but it must not be performed twice. We'll arbitrarily
    // choose to perform it as part of the acquire operation.
    //
    // However, we still need to record initial layout for the "initial layout" validation
    if (cb_state.IsReleaseOp(mem_barrier)) {
        cb_state.SetImageInitialLayout(*image_state, normalized_isr, initial_layout);
    } else {
        cb_state.SetImageLayout(*image_state, normalized_isr, new_layout, initial_layout);
    }
}

void CoreChecks::TransitionImageLayouts(vvl::CommandBuffer &cb_state, uint32_t barrier_count,
                                        const VkImageMemoryBarrier2 *image_barriers) {
    for (uint32_t i = 0; i < barrier_count; i++) {
        const ImageBarrier barrier(image_barriers[i]);
        RecordTransitionImageLayout(cb_state, barrier);
    }
}

void CoreChecks::TransitionImageLayouts(vvl::CommandBuffer &cb_state, uint32_t barrier_count,
                                        const VkImageMemoryBarrier *image_barriers, VkPipelineStageFlags src_stage_mask,
                                        VkPipelineStageFlags dst_stage_mask) {
    for (uint32_t i = 0; i < barrier_count; i++) {
        const ImageBarrier barrier(image_barriers[i], src_stage_mask, dst_stage_mask);
        RecordTransitionImageLayout(cb_state, barrier);
    }
}

bool CoreChecks::IsCompliantSubresourceRange(const VkImageSubresourceRange &subres_range, const vvl::Image &image_state) const {
    if (!(subres_range.layerCount) || !(subres_range.levelCount)) return false;
    if (subres_range.baseMipLevel + subres_range.levelCount > image_state.create_info.mipLevels) return false;
    if ((subres_range.baseArrayLayer + subres_range.layerCount) > image_state.create_info.arrayLayers) {
        return false;
    }
    if (!IsValidAspectMaskForFormat(subres_range.aspectMask, image_state.create_info.format)) return false;
    if (((vkuFormatPlaneCount(image_state.create_info.format) < 3) && (subres_range.aspectMask & VK_IMAGE_ASPECT_PLANE_2_BIT)) ||
        ((vkuFormatPlaneCount(image_state.create_info.format) < 2) && (subres_range.aspectMask & VK_IMAGE_ASPECT_PLANE_1_BIT))) {
        return false;
    }
    if (subres_range.aspectMask & VK_IMAGE_ASPECT_METADATA_BIT ||
        subres_range.aspectMask & VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT ||
        subres_range.aspectMask & VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT ||
        subres_range.aspectMask & VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT ||
        subres_range.aspectMask & VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT) {
        return false;
    }
    return true;
}

bool CoreChecks::ValidateHostCopyCurrentLayout(const VkImageLayout expected_layout, const VkImageSubresourceLayers &subres_layers,
                                               const vvl::Image &image_state, const Location &loc) const {
    return ValidateHostCopyCurrentLayout(expected_layout, RangeFromLayers(subres_layers), image_state, loc);
}

bool CoreChecks::ValidateHostCopyCurrentLayout(const VkImageLayout expected_layout, const VkImageSubresourceRange &validate_range,
                                               const vvl::Image &image_state, const Location &loc) const {
    bool skip = false;
    if (disabled[image_layout_validation]) return false;
    if (!(image_state.layout_range_map)) return false;
    const VkImageSubresourceRange subres_range = image_state.NormalizeSubresourceRange(validate_range);
    // RangeGenerator doesn't tolerate degenerate or invalid ranges. The error will be found and logged elsewhere
    if (!IsCompliantSubresourceRange(subres_range, image_state)) return false;

    GlobalImageLayoutRangeMap::RangeGenerator range_gen(image_state.subresource_encoder, subres_range);

    struct CheckState {
        const VkImageLayout expected_layout;
        VkImageAspectFlags aspect_mask;
        GlobalImageLayoutRangeMap::key_type found_range;
        VkImageLayout found_layout;
        CheckState(VkImageLayout expected_layout_, VkImageAspectFlags aspect_mask_)
            : expected_layout(expected_layout_),
              aspect_mask(aspect_mask_),
              found_range({0, 0}),
              found_layout(VK_IMAGE_LAYOUT_MAX_ENUM) {}
    };

    CheckState check_state(expected_layout, subres_range.aspectMask);

    auto guard = image_state.layout_range_map->ReadLock();
    image_state.layout_range_map->AnyInRange(
        range_gen, [&check_state](const GlobalImageLayoutRangeMap::key_type &range, const VkImageLayout &layout) {
            bool mismatch = false;
            if (!ImageLayoutMatches(check_state.aspect_mask, layout, check_state.expected_layout)) {
                check_state.found_range = range;
                check_state.found_layout = layout;
                mismatch = true;
            }
            return mismatch;
        });

    if (check_state.found_range.non_empty()) {
        const VkImageSubresource subres = image_state.subresource_encoder.IndexToVkSubresource(check_state.found_range.begin);
        skip |= LogError(vvl::GetImageImageLayoutVUID(loc), image_state.Handle(), loc,
                         "is currently %s but expected to be %s for %s (subresource: %s)",
                         string_VkImageLayout(check_state.found_layout), string_VkImageLayout(expected_layout),
                         debug_report->FormatHandle(image_state.Handle()).c_str(), string_VkImageSubresource(subres).c_str());
    }
    return skip;
}
