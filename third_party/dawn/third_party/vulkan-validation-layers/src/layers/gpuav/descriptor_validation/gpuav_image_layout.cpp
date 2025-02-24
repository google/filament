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

#include "gpuav/descriptor_validation/gpuav_image_layout.h"

#include "gpuav/core/gpuav.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "gpuav/validation_cmd/gpuav_copy_buffer_to_image.h"
#include "utils/image_layout_utils.h"
#include "drawdispatch/drawdispatch_vuids.h"
#include "error_message/error_strings.h"

#include "state_tracker/render_pass_state.h"

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

namespace gpuav {

static void RecordTransitionImageLayout(Validator &gpuav, vvl::CommandBuffer &cb_state,
                                        const sync_utils::ImageBarrier &mem_barrier) {
    if (gpuav.enabled_features.synchronization2) {
        if (mem_barrier.oldLayout == mem_barrier.newLayout) {
            return;
        }
    }
    auto image_state = gpuav.Get<vvl::Image>(mem_barrier.image);
    if (!image_state) return;

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

static void TransitionImageLayouts(Validator &gpuav, vvl::CommandBuffer &cb_state, uint32_t barrier_count,
                                   const VkImageMemoryBarrier2 *image_barriers) {
    for (uint32_t i = 0; i < barrier_count; i++) {
        const sync_utils::ImageBarrier barrier(image_barriers[i]);
        RecordTransitionImageLayout(gpuav, cb_state, barrier);
    }
}

static void TransitionImageLayouts(Validator &gpuav, vvl::CommandBuffer &cb_state, uint32_t barrier_count,
                                   const VkImageMemoryBarrier *image_barriers, VkPipelineStageFlags src_stage_mask,
                                   VkPipelineStageFlags dst_stage_mask) {
    for (uint32_t i = 0; i < barrier_count; i++) {
        const sync_utils::ImageBarrier barrier(image_barriers[i], src_stage_mask, dst_stage_mask);
        RecordTransitionImageLayout(gpuav, cb_state, barrier);
    }
}

static void TransitionAttachmentRefLayout(vvl::CommandBuffer &cb_state, const vku::safe_VkAttachmentReference2 &ref) {
    if (ref.attachment != VK_ATTACHMENT_UNUSED) {
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
}

template <typename RangeFactory>
static bool VerifyImageLayoutRange(const Validator &gpuav, const vvl::CommandBuffer &cb_state, const vvl::Image &image_state,
                                   VkImageAspectFlags aspect_mask, VkImageLayout explicit_layout, const RangeFactory &range_factory,
                                   const Location &loc, const char *mismatch_layout_vuid, bool *error) {
    bool skip = false;
    if (!gpuav.gpuav_settings.validate_image_layout) return skip;

    const auto image_layout_registry = cb_state.GetImageLayoutRegistry(image_state.VkHandle());
    if (!image_layout_registry) {
        return skip;
    }

    // TODO - things like ANGLE might have external images which have their layouts transitioned implicitly
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8940
    if (image_state.external_memory_handle_types != 0) {
        return skip;
    }

    const auto &layout_map = image_layout_registry->GetLayoutMap();
    const auto *global_range_map = image_state.layout_range_map.get();
    GlobalImageLayoutRangeMap empty_map(1);
    assert(global_range_map);
    auto global_range_map_guard = global_range_map->ReadLock();

    auto pos = layout_map.begin();
    const auto end = layout_map.end();
    sparse_container::parallel_iterator<const GlobalImageLayoutRangeMap> current_layout(empty_map, *global_range_map,
                                                                                        pos->first.begin);
    while (pos != end) {
        const VkImageLayout initial_layout = pos->second.initial_layout;
        ASSERT_AND_CONTINUE(initial_layout != image_layout_map::kInvalidLayout);

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
            const auto aspect_mask = image_state.subresource_encoder.Decode(intersected_range.begin).aspectMask;
            const bool matches = ImageLayoutMatches(aspect_mask, image_layout, initial_layout);
            if (!matches) {
                // We can report all the errors for the intersected range directly
                for (auto index : sparse_container::range_view<decltype(intersected_range)>(intersected_range)) {
                    const auto subresource = image_state.subresource_encoder.Decode(index);
                    const LogObjectList objlist(cb_state.Handle(), image_state.Handle());
                    // TODO - We need a way to map the action command to which caused this error
                    const vvl::DrawDispatchVuid &vuid = GetDrawDispatchVuid(vvl::Func::vkCmdDraw);
                    skip |= gpuav.LogError(vuid.image_layout_09600, objlist, loc,
                                           "command buffer %s expects %s (subresource: %s) to be in layout %s--instead, current "
                                           "layout is %s. (Detected from GPU-AV)",
                                           gpuav.FormatHandle(cb_state).c_str(), gpuav.FormatHandle(image_state).c_str(),
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
    return skip;
}

template <typename RegionType>
static void RecordCmdBlitImage(Validator &gpuav, VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                               VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType *pRegions,
                               VkFilter filter) {
    auto cb_state_ptr = gpuav.GetWrite<vvl::CommandBuffer>(commandBuffer);
    auto src_image_state = gpuav.Get<vvl::Image>(srcImage);
    auto dst_image_state = gpuav.Get<vvl::Image>(dstImage);
    if (cb_state_ptr && src_image_state && dst_image_state) {
        // Make sure that all image slices are updated to correct layout
        for (uint32_t i = 0; i < regionCount; ++i) {
            cb_state_ptr->SetImageInitialLayout(*src_image_state, pRegions[i].srcSubresource, srcImageLayout);
            cb_state_ptr->SetImageInitialLayout(*dst_image_state, pRegions[i].dstSubresource, dstImageLayout);
        }
    }
}

static void RecordCmdWaitEvents2(Validator &gpuav, VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                 const VkDependencyInfo *pDependencyInfos) {
    // don't hold read lock during the base class method
    auto cb_state = gpuav.GetWrite<vvl::CommandBuffer>(commandBuffer);
    for (uint32_t i = 0; i < eventCount; i++) {
        const auto &dep_info = pDependencyInfos[i];
        TransitionImageLayouts(gpuav, *cb_state, dep_info.imageMemoryBarrierCount, dep_info.pImageMemoryBarriers);
    }
}

void UpdateCmdBufImageLayouts(Validator &gpuav, const vvl::CommandBuffer &cb_state) {
    for (const auto &[image, image_layout_registry] : cb_state.image_layout_map) {
        if (!image_layout_registry) continue;
        auto image_state = gpuav.Get<vvl::Image>(image);
        if (image_state && image_state->GetId() == image_layout_registry->GetImageId()) {
            auto guard = image_state->layout_range_map->WriteLock();
            sparse_container::splice(*image_state->layout_range_map, image_layout_registry->GetLayoutMap(), GlobalLayoutUpdater());
        }
    }
}

void TransitionSubpassLayouts(vvl::CommandBuffer &cb_state, const vvl::RenderPass &render_pass_state, const int subpass_index) {
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
void TransitionBeginRenderPassLayouts(vvl::CommandBuffer &cb_state, const vvl::RenderPass &render_pass_state) {
    // First record expected initialLayout as a potential initial layout usage.
    auto const rpci = render_pass_state.create_info.ptr();
    for (uint32_t i = 0; i < rpci->attachmentCount; ++i) {
        auto *view_state = cb_state.GetActiveAttachmentImageViewState(i);
        if (view_state) {
            vvl::Image *image_state = view_state->image_state.get();
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
                cb_state.SetImageInitialLayout(*image_state, view_state->normalized_subresource_range, initial_layout);
            }
        }
    }
    // Now transition for first subpass (index 0)
    TransitionSubpassLayouts(cb_state, render_pass_state, 0);
}

void TransitionFinalSubpassLayouts(vvl::CommandBuffer &cb_state) {
    auto render_pass_state = cb_state.active_render_pass.get();
    auto framebuffer_state = cb_state.activeFramebuffer.get();
    if (!render_pass_state || !framebuffer_state) {
        return;
    }

    const VkRenderPassCreateInfo2 *render_pass_info = render_pass_state->create_info.ptr();
    for (uint32_t i = 0; i < render_pass_info->attachmentCount; ++i) {
        auto *view_state = cb_state.GetActiveAttachmentImageViewState(i);
        if (view_state) {
            VkImageLayout stencil_layout = kInvalidLayout;
            const auto *attachment_description_stencil_layout =
                vku::FindStructInPNextChain<VkAttachmentDescriptionStencilLayout>(render_pass_info->pAttachments[i].pNext);
            if (attachment_description_stencil_layout) {
                stencil_layout = attachment_description_stencil_layout->stencilFinalLayout;
            }
            cb_state.SetImageViewLayout(*view_state, render_pass_info->pAttachments[i].finalLayout, stencil_layout);
        }
    }
}

bool Validator::VerifyImageLayout(const vvl::CommandBuffer &cb_state, const vvl::ImageView &image_view_state,
                                  VkImageLayout explicit_layout, const Location &loc, const char *mismatch_layout_vuid,
                                  bool *error) const {
    if (disabled[image_layout_validation]) return false;
    // Possible the image state was destroyed and we didn't see it waiting for the queue submit callback
    if (!image_view_state.image_state) return false;
    auto range_factory = [&image_view_state](const ImageLayoutRegistry &registry) {
        return image_layout_map::RangeGenerator(image_view_state.range_generator);
    };

    return VerifyImageLayoutRange(*this, cb_state, *image_view_state.image_state,
                                  image_view_state.create_info.subresourceRange.aspectMask, explicit_layout, range_factory, loc,
                                  mismatch_layout_vuid, error);
}

// Validates the buffer is allowed to be protected
bool Validator::ValidateProtectedBuffer(const vvl::CommandBuffer &cb_state, const vvl::Buffer &buffer_state,
                                        const Location &buffer_loc, const char *vuid, const char *more_message) const {
    bool skip = false;

    // if driver supports protectedNoFault the operation is valid, just has undefined values
    if ((!phys_dev_props_core11.protectedNoFault) && (cb_state.unprotected == true) && (buffer_state.unprotected == false)) {
        const LogObjectList objlist(cb_state.Handle(), buffer_state.Handle());
        skip |= LogError(vuid, objlist, buffer_loc, "(%s) is a protected buffer, but command buffer (%s) is unprotected.%s",
                         FormatHandle(buffer_state).c_str(), FormatHandle(cb_state).c_str(), more_message);
    }
    return skip;
}

// Validates the buffer is allowed to be unprotected
bool Validator::ValidateUnprotectedBuffer(const vvl::CommandBuffer &cb_state, const vvl::Buffer &buffer_state,
                                          const Location &buffer_loc, const char *vuid, const char *more_message) const {
    bool skip = false;

    // if driver supports protectedNoFault the operation is valid, just has undefined values
    if ((!phys_dev_props_core11.protectedNoFault) && (cb_state.unprotected == false) && (buffer_state.unprotected == true)) {
        const LogObjectList objlist(cb_state.Handle(), buffer_state.Handle());
        skip |= LogError(vuid, objlist, buffer_loc, "(%s) is an unprotected buffer, but command buffer (%s) is protected.%s",
                         FormatHandle(buffer_state).c_str(), FormatHandle(cb_state).c_str(), more_message);
    }
    return skip;
}

// Validates the image is allowed to be protected
bool Validator::ValidateProtectedImage(const vvl::CommandBuffer &cb_state, const vvl::Image &image_state, const Location &loc,
                                       const char *vuid, const char *more_message) const {
    bool skip = false;

    // if driver supports protectedNoFault the operation is valid, just has undefined values
    if ((!phys_dev_props_core11.protectedNoFault) && (cb_state.unprotected == true) && (image_state.unprotected == false)) {
        const LogObjectList objlist(cb_state.Handle(), image_state.Handle());
        skip |= LogError(vuid, objlist, loc, "(%s) is a protected image, but command buffer (%s) is unprotected.%s",
                         FormatHandle(image_state).c_str(), FormatHandle(cb_state).c_str(), more_message);
    }
    return skip;
}

// Validates the image is allowed to be unprotected
bool Validator::ValidateUnprotectedImage(const vvl::CommandBuffer &cb_state, const vvl::Image &image_state, const Location &loc,
                                         const char *vuid, const char *more_message) const {
    bool skip = false;

    // if driver supports protectedNoFault the operation is valid, just has undefined values
    if ((!phys_dev_props_core11.protectedNoFault) && (cb_state.unprotected == false) && (image_state.unprotected == true)) {
        const LogObjectList objlist(cb_state.Handle(), image_state.Handle());
        skip |= LogError(vuid, objlist, loc, "(%s) is an unprotected image, but command buffer (%s) is protected.%s",
                         FormatHandle(image_state).c_str(), FormatHandle(cb_state).c_str(), more_message);
    }
    return skip;
}

void Validator::PostCallRecordCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo,
                                          const VkAllocationCallbacks *pAllocator, VkImage *pImage,
                                          const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;

    BaseClass::PostCallRecordCreateImage(device, pCreateInfo, pAllocator, pImage, record_obj);
    if ((pCreateInfo->flags & VK_IMAGE_CREATE_SPARSE_BINDING_BIT) != 0) {
        // non-sparse images set up their layout maps when memory is bound
        if (auto image_state = Get<vvl::Image>(*pImage)) {
            image_state->SetInitialLayoutMap();
        }
    }
}

void Validator::PreCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks *pAllocator,
                                          const RecordObject &record_obj) {
    // Clean up validation specific data
    // Clean up generic image state
    BaseClass::PreCallRecordDestroyImage(device, image, pAllocator, record_obj);
}

void Validator::PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                const VkClearColorValue *pColor, uint32_t rangeCount,
                                                const VkImageSubresourceRange *pRanges, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges, record_obj);

    auto cb_state_ptr = GetWrite<vvl::CommandBuffer>(commandBuffer);
    auto image_state = Get<vvl::Image>(image);
    if (cb_state_ptr && image_state) {
        for (uint32_t i = 0; i < rangeCount; ++i) {
            cb_state_ptr->SetImageInitialLayout(image, pRanges[i], imageLayout);
        }
    }
}

void Validator::PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                       const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount,
                                                       const VkImageSubresourceRange *pRanges, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges,
                                                      record_obj);

    auto cb_state_ptr = GetWrite<vvl::CommandBuffer>(commandBuffer);
    auto image_state = Get<vvl::Image>(image);
    if (cb_state_ptr && image_state) {
        for (uint32_t i = 0; i < rangeCount; ++i) {
            cb_state_ptr->SetImageInitialLayout(image, pRanges[i], imageLayout);
        }
    }
}

void Validator::PreCallRecordCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                 const VkClearAttachment *pAttachments, uint32_t rectCount,
                                                 const VkClearRect *pRects, const RecordObject &record_obj) {
    // TODO???
}

void Validator::PostCallRecordTransitionImageLayout(VkDevice device, uint32_t transitionCount,
                                                    const VkHostImageLayoutTransitionInfo *pTransitions,
                                                    const RecordObject &record_obj) {
    BaseClass::PostCallRecordTransitionImageLayout(device, transitionCount, pTransitions, record_obj);

    if (VK_SUCCESS != record_obj.result) return;

    for (uint32_t i = 0; i < transitionCount; ++i) {
        auto &transition = pTransitions[i];
        auto image_state = Get<vvl::Image>(transition.image);
        if (!image_state) continue;
        image_state->SetImageLayout(transition.subresourceRange, transition.newLayout);
    }
}

void Validator::PostCallRecordTransitionImageLayoutEXT(VkDevice device, uint32_t transitionCount,
                                                       const VkHostImageLayoutTransitionInfoEXT *pTransitions,
                                                       const RecordObject &record_obj) {
    PostCallRecordTransitionImageLayout(device, transitionCount, pTransitions, record_obj);
}

void Validator::PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                          VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                          const VkImageCopy *pRegions, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions,
                                         record_obj);
    auto cb_state_ptr = GetWrite<vvl::CommandBuffer>(commandBuffer);
    auto src_image_state = Get<vvl::Image>(srcImage);
    auto dst_image_state = Get<vvl::Image>(dstImage);
    if (cb_state_ptr && src_image_state && dst_image_state) {
        // Make sure that all image slices are updated to correct layout
        for (uint32_t i = 0; i < regionCount; ++i) {
            cb_state_ptr->SetImageInitialLayout(*src_image_state, pRegions[i].srcSubresource, srcImageLayout);
            cb_state_ptr->SetImageInitialLayout(*dst_image_state, pRegions[i].dstSubresource, dstImageLayout);
        }
    }
}

void Validator::PreCallRecordCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR *pCopyImageInfo,
                                              const RecordObject &record_obj) {
    PreCallRecordCmdCopyImage2(commandBuffer, pCopyImageInfo, record_obj);
}

void Validator::PreCallRecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo,
                                           const RecordObject &record_obj) {
    auto cb_state_ptr = GetWrite<vvl::CommandBuffer>(commandBuffer);
    auto src_image_state = Get<vvl::Image>(pCopyImageInfo->srcImage);
    auto dst_image_state = Get<vvl::Image>(pCopyImageInfo->dstImage);
    if (cb_state_ptr && src_image_state && dst_image_state) {
        // Make sure that all image slices are updated to correct layout
        for (uint32_t i = 0; i < pCopyImageInfo->regionCount; ++i) {
            cb_state_ptr->SetImageInitialLayout(*src_image_state, pCopyImageInfo->pRegions[i].srcSubresource,
                                                pCopyImageInfo->srcImageLayout);
            cb_state_ptr->SetImageInitialLayout(*dst_image_state, pCopyImageInfo->pRegions[i].dstSubresource,
                                                pCopyImageInfo->dstImageLayout);
        }
    }
}

void Validator::PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                  VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy *pRegions,
                                                  const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions,
                                                 record_obj);

    auto cb_state_ptr = GetWrite<vvl::CommandBuffer>(commandBuffer);
    auto src_image_state = Get<vvl::Image>(srcImage);
    if (cb_state_ptr && src_image_state) {
        // Make sure that all image slices record referenced layout
        for (uint32_t i = 0; i < regionCount; ++i) {
            cb_state_ptr->SetImageInitialLayout(*src_image_state, pRegions[i].imageSubresource, srcImageLayout);
        }
    }
}

void Validator::PreCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                      const VkCopyImageToBufferInfo2KHR *pCopyImageToBufferInfo,
                                                      const RecordObject &record_obj) {
    PreCallRecordCmdCopyImageToBuffer2(commandBuffer, pCopyImageToBufferInfo, record_obj);
}

void Validator::PreCallRecordCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                                                   const VkCopyImageToBufferInfo2 *pCopyImageToBufferInfo,
                                                   const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdCopyImageToBuffer2(commandBuffer, pCopyImageToBufferInfo, record_obj);

    auto cb_state_ptr = GetWrite<vvl::CommandBuffer>(commandBuffer);
    auto src_image_state = Get<vvl::Image>(pCopyImageToBufferInfo->srcImage);
    if (cb_state_ptr && src_image_state) {
        // Make sure that all image slices record referenced layout
        for (uint32_t i = 0; i < pCopyImageToBufferInfo->regionCount; ++i) {
            cb_state_ptr->SetImageInitialLayout(*src_image_state, pCopyImageToBufferInfo->pRegions[i].imageSubresource,
                                                pCopyImageToBufferInfo->srcImageLayout);
        }
    }
}

void Validator::PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                  VkImageLayout dstImageLayout, uint32_t regionCount,
                                                  const VkBufferImageCopy *pRegions, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions,
                                                 record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);

    if (auto dst_image_state = Get<vvl::Image>(dstImage)) {
        // Make sure that all image slices are record referenced layout
        for (uint32_t i = 0; i < regionCount; ++i) {
            cb_state->SetImageInitialLayout(*dst_image_state, pRegions[i].imageSubresource, dstImageLayout);
        }
    }

    std::vector<VkBufferImageCopy2> regions_2(regionCount);
    for (const auto [i, region] : vvl::enumerate(pRegions, regionCount)) {
        regions_2[i].bufferOffset = region.bufferOffset;
        regions_2[i].bufferRowLength = region.bufferRowLength;
        regions_2[i].bufferImageHeight = region.bufferImageHeight;
        regions_2[i].imageSubresource = region.imageSubresource;
        regions_2[i].imageOffset = region.imageOffset;
        regions_2[i].imageExtent = region.imageExtent;
    }

    VkCopyBufferToImageInfo2 copy_buffer_to_image_info = vku::InitStructHelper();
    copy_buffer_to_image_info.srcBuffer = srcBuffer;
    copy_buffer_to_image_info.dstImage = dstImage;
    copy_buffer_to_image_info.dstImageLayout = dstImageLayout;
    copy_buffer_to_image_info.regionCount = regionCount;
    copy_buffer_to_image_info.pRegions = regions_2.data();

    InsertCopyBufferToImageValidation(*this, record_obj.location, *cb_state, &copy_buffer_to_image_info);
}

void Validator::PreCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                      const VkCopyBufferToImageInfo2KHR *pCopyBufferToImageInfo2KHR,
                                                      const RecordObject &record_obj) {
    PreCallRecordCmdCopyBufferToImage2(commandBuffer, pCopyBufferToImageInfo2KHR, record_obj);
}

void Validator::PreCallRecordCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                                                   const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo,
                                                   const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdCopyBufferToImage2(commandBuffer, pCopyBufferToImageInfo, record_obj);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);

    if (auto dst_image_state = Get<vvl::Image>(pCopyBufferToImageInfo->dstImage)) {
        // Make sure that all image slices are record referenced layout
        for (uint32_t i = 0; i < pCopyBufferToImageInfo->regionCount; ++i) {
            cb_state->SetImageInitialLayout(*dst_image_state, pCopyBufferToImageInfo->pRegions[i].imageSubresource,
                                            pCopyBufferToImageInfo->dstImageLayout);
        }
    }

    InsertCopyBufferToImageValidation(*this, record_obj.location, *cb_state, pCopyBufferToImageInfo);
}

void Validator::PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                          VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                          const VkImageBlit *pRegions, VkFilter filter, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions,
                                         filter, record_obj);
    RecordCmdBlitImage(*this, commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
}

void Validator::PreCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR *pBlitImageInfo,
                                              const RecordObject &record_obj) {
    PreCallRecordCmdBlitImage2(commandBuffer, pBlitImageInfo, record_obj);
}

void Validator::PreCallRecordCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR *pBlitImageInfo,
                                           const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdBlitImage2(commandBuffer, pBlitImageInfo, record_obj);
    RecordCmdBlitImage(*this, commandBuffer, pBlitImageInfo->srcImage, pBlitImageInfo->srcImageLayout, pBlitImageInfo->dstImage,
                       pBlitImageInfo->dstImageLayout, pBlitImageInfo->regionCount, pBlitImageInfo->pRegions,
                       pBlitImageInfo->filter);
}

void Validator::PostCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                              const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    BaseClass::PostCallRecordBindImageMemory(device, image, memory, memoryOffset, record_obj);

    if (auto image_state = Get<vvl::Image>(image)) {
        image_state->SetInitialLayoutMap();
    }
}

void Validator::PostCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo *pBindInfos,
                                               const RecordObject &record_obj) {
    // Don't check |record_obj.result| as some binds might still be valid
    BaseClass::PostCallRecordBindImageMemory2(device, bindInfoCount, pBindInfos, record_obj);

    for (uint32_t i = 0; i < bindInfoCount; i++) {
        if (auto image_state = Get<vvl::Image>(pBindInfos[i].image)) {
            // Need to protect if some VkBindMemoryStatus are not VK_SUCCESS
            if (!image_state->HasBeenBound()) continue;

            image_state->SetInitialLayoutMap();
        }
    }
}

void Validator::PostCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo *pBindInfos,
                                                  const RecordObject &record_obj) {
    PostCallRecordBindImageMemory2(device, bindInfoCount, pBindInfos, record_obj);
}

void Validator::PreCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                           VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                           uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                           uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                           uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers,
                                           const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdWaitEvents(commandBuffer, eventCount, pEvents, sourceStageMask, dstStageMask, memoryBarrierCount,
                                          pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount,
                                          pImageMemoryBarriers, record_obj);
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    TransitionImageLayouts(*this, *cb_state, imageMemoryBarrierCount, pImageMemoryBarriers, sourceStageMask, dstStageMask);
}

void Validator::PreCallRecordCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                               const VkDependencyInfoKHR *pDependencyInfos, const RecordObject &record_obj) {
    PreCallRecordCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos, record_obj);
}

void Validator::PreCallRecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                            const VkDependencyInfo *pDependencyInfos, const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos, record_obj);
    RecordCmdWaitEvents2(*this, commandBuffer, eventCount, pEvents, pDependencyInfos);
}

void Validator::PreCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                                VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                                uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                                uint32_t bufferMemoryBarrierCount,
                                                const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                                uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers,
                                                const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount,
                                               pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers,
                                               imageMemoryBarrierCount, pImageMemoryBarriers, record_obj);

    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    TransitionImageLayouts(*this, *cb_state, imageMemoryBarrierCount, pImageMemoryBarriers, srcStageMask, dstStageMask);
}

void Validator::PreCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfoKHR *pDependencyInfo,
                                                    const RecordObject &record_obj) {
    PreCallRecordCmdPipelineBarrier2(commandBuffer, pDependencyInfo, record_obj);
}

void Validator::PreCallRecordCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo *pDependencyInfo,
                                                 const RecordObject &record_obj) {
    BaseClass::PreCallRecordCmdPipelineBarrier2(commandBuffer, pDependencyInfo, record_obj);

    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    TransitionImageLayouts(*this, *cb_state, pDependencyInfo->imageMemoryBarrierCount, pDependencyInfo->pImageMemoryBarriers);
}
}  // namespace gpuav
