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
#include "sync/sync_utils.h"
#include "best_practices/bp_state.h"
#include "state_tracker/queue_state.h"

bool BestPractices::CheckPipelineStageFlags(const LogObjectList& objlist, const Location& loc, VkPipelineStageFlags flags) const {
    bool skip = false;

    if (flags & VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT) {
        skip |= LogWarning("BestPractices-pipeline-stage-flags-graphics", objlist, loc, "using VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT");
    } else if (flags & VK_PIPELINE_STAGE_ALL_COMMANDS_BIT) {
        skip |= LogWarning("BestPractices-pipeline-stage-flags-compute", objlist, loc, "using VK_PIPELINE_STAGE_ALL_COMMANDS_BIT");
    }

    return skip;
}

bool BestPractices::CheckPipelineStageFlags(const LogObjectList& objlist, const Location& loc,
                                            VkPipelineStageFlags2KHR flags) const {
    bool skip = false;

    if (flags & VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT) {
        skip |=
            LogWarning("BestPractices-pipeline-stage-flags2-graphics", objlist, loc, "using VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT");
    } else if (flags & VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT) {
        skip |=
            LogWarning("BestPractices-pipeline-stage-flags2-compute", objlist, loc, "using VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT");
    }

    return skip;
}

bool BestPractices::CheckDependencyInfo(const LogObjectList& objlist, const Location& dep_loc,
                                        const VkDependencyInfo& dep_info) const {
    bool skip = false;
    auto stage_masks = sync_utils::GetGlobalStageMasks(dep_info);

    skip |= CheckPipelineStageFlags(objlist, dep_loc, stage_masks.src);
    skip |= CheckPipelineStageFlags(objlist, dep_loc, stage_masks.dst);
    for (uint32_t i = 0; i < dep_info.imageMemoryBarrierCount; ++i) {
        skip |= ValidateImageMemoryBarrier(dep_loc.dot(Field::pImageMemoryBarriers, i), dep_info.pImageMemoryBarriers[i].image,
                                           dep_info.pImageMemoryBarriers[i].oldLayout, dep_info.pImageMemoryBarriers[i].newLayout,
                                           dep_info.pImageMemoryBarriers[i].srcAccessMask,
                                           dep_info.pImageMemoryBarriers[i].dstAccessMask,
                                           dep_info.pImageMemoryBarriers[i].subresourceRange.aspectMask);
    }

    return skip;
}

bool BestPractices::CheckEventSignalingState(const bp_state::CommandBuffer& command_buffer, VkEvent event,
                                             const Location& cb_loc) const {
    bool skip = false;
    if (auto* signaling_info = vvl::Find(command_buffer.event_signaling_state, event); signaling_info && signaling_info->signaled) {
        const LogObjectList objlist(command_buffer.VkHandle(), event);
        skip |= LogWarning("BestPractices-Event-SignalSignaledEvent", objlist, cb_loc,
                           "%s sets event %s which was already set (in this command buffer or in the executed secondary command "
                           "buffers). If this is not the desired behavior, the event must be reset before it is set again.",
                           FormatHandle(command_buffer.VkHandle()).c_str(), FormatHandle(event).c_str());
    }
    return skip;
}

void BestPractices::RecordCmdSetEvent(bp_state::CommandBuffer& command_buffer, VkEvent event) {
    if (auto* signaling_info = vvl::Find(command_buffer.event_signaling_state, event)) {
        signaling_info->signaled = true;
    } else {
        command_buffer.event_signaling_state.emplace(event, bp_state::CommandBuffer::SignalingInfo(true));
    }
}

void BestPractices::RecordCmdResetEvent(bp_state::CommandBuffer& command_buffer, VkEvent event) {
    if (auto* signaling_info = vvl::Find(command_buffer.event_signaling_state, event)) {
        signaling_info->signaled = false;
    } else {
        command_buffer.event_signaling_state.emplace(event, bp_state::CommandBuffer::SignalingInfo(false));
    }
}

bool BestPractices::PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                               const ErrorObject& error_obj) const {
    bool skip = false;

    skip |= CheckPipelineStageFlags(commandBuffer, error_obj.location.dot(Field::stageMask), stageMask);
    auto cb_state = Get<bp_state::CommandBuffer>(commandBuffer);
    skip |= CheckEventSignalingState(*cb_state, event, error_obj.location.dot(Field::commandBuffer));
    return skip;
}

void BestPractices::PreCallRecordCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                             const RecordObject& record_obj) {
    BaseClass::PreCallRecordCmdSetEvent(commandBuffer, event, stageMask, record_obj);
    auto cb_state = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    RecordCmdSetEvent(*cb_state, event);
}

bool BestPractices::PreCallValidateCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                   const VkDependencyInfoKHR* pDependencyInfo, const ErrorObject& error_obj) const {
    return PreCallValidateCmdSetEvent2(commandBuffer, event, pDependencyInfo, error_obj);
}

bool BestPractices::PreCallValidateCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event,
                                                const VkDependencyInfo* pDependencyInfo, const ErrorObject& error_obj) const {
    bool skip = false;
    skip |= CheckDependencyInfo(commandBuffer, error_obj.location.dot(Field::pDependencyInfo), *pDependencyInfo);
    auto cb_state = Get<bp_state::CommandBuffer>(commandBuffer);
    skip |= CheckEventSignalingState(*cb_state, event, error_obj.location.dot(Field::commandBuffer));
    return skip;
}

void BestPractices::PreCallRecordCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                 const VkDependencyInfoKHR* pDependencyInfo, const RecordObject& record_obj) {
    PreCallRecordCmdSetEvent2(commandBuffer, event, pDependencyInfo, record_obj);
}

void BestPractices::PreCallRecordCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo* pDependencyInfo,
                                              const RecordObject& record_obj) {
    BaseClass::PreCallRecordCmdSetEvent2(commandBuffer, event, pDependencyInfo, record_obj);
    auto cb_state = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    RecordCmdSetEvent(*cb_state, event);
}

bool BestPractices::PreCallValidateCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                                 const ErrorObject& error_obj) const {
    bool skip = false;
    skip |= CheckPipelineStageFlags(commandBuffer, error_obj.location.dot(Field::stageMask), stageMask);
    return skip;
}

void BestPractices::PreCallRecordCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                               const RecordObject& record_obj) {
    BaseClass::PreCallRecordCmdResetEvent(commandBuffer, event, stageMask, record_obj);
    auto cb_state = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    RecordCmdResetEvent(*cb_state, event);
}

bool BestPractices::PreCallValidateCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                     VkPipelineStageFlags2KHR stageMask, const ErrorObject& error_obj) const {
    return PreCallValidateCmdResetEvent2(commandBuffer, event, stageMask, error_obj);
}

bool BestPractices::PreCallValidateCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                                  const ErrorObject& error_obj) const {
    bool skip = false;
    skip |= CheckPipelineStageFlags(commandBuffer, error_obj.location.dot(Field::stageMask), stageMask);
    return skip;
}

void BestPractices::PreCallRecordCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2KHR stageMask,
                                                   const RecordObject& record_obj) {
    PreCallRecordCmdResetEvent2(commandBuffer, event, stageMask, record_obj);
}

void BestPractices::PreCallRecordCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                                const RecordObject& record_obj) {
    BaseClass::PreCallRecordCmdResetEvent2(commandBuffer, event, stageMask, record_obj);
    auto cb_state = GetWrite<bp_state::CommandBuffer>(commandBuffer);
    RecordCmdResetEvent(*cb_state, event);
}

bool BestPractices::PreCallValidateCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                                 VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                                 uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                                 uint32_t bufferMemoryBarrierCount,
                                                 const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                                 uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers,
                                                 const ErrorObject& error_obj) const {
    bool skip = false;

    skip |= CheckPipelineStageFlags(commandBuffer, error_obj.location.dot(Field::srcStageMask), srcStageMask);
    skip |= CheckPipelineStageFlags(commandBuffer, error_obj.location.dot(Field::dstStageMask), dstStageMask);

    return skip;
}

bool BestPractices::PreCallValidateCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                                     const VkDependencyInfoKHR* pDependencyInfos,
                                                     const ErrorObject& error_obj) const {
    return PreCallValidateCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos, error_obj);
}

bool BestPractices::PreCallValidateCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                                  const VkDependencyInfo* pDependencyInfos, const ErrorObject& error_obj) const {
    bool skip = false;
    for (uint32_t i = 0; i < eventCount; i++) {
        skip |= CheckDependencyInfo(commandBuffer, error_obj.location.dot(Field::pDependencyInfos, i), pDependencyInfos[i]);
    }

    return skip;
}

bool BestPractices::ValidateAccessLayoutCombination(const Location& loc, VkImage image, VkAccessFlags2 access, VkImageLayout layout,
                                                    VkImageAspectFlags aspect) const {
    bool skip = false;

    const VkAccessFlags2 all = vvl::kU64Max;  // core validation is responsible for detecting undefined flags.
    VkAccessFlags2 allowed = 0;

    // Combinations taken from https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/2918
    switch (layout) {
        case VK_IMAGE_LAYOUT_UNDEFINED:
            allowed = all;
            break;
        case VK_IMAGE_LAYOUT_GENERAL:
            allowed = all;
            break;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            allowed = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                      VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            allowed = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
            allowed = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                      VK_ACCESS_2_SHADER_SAMPLED_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT |
                      VK_ACCESS_2_SHADER_BINDING_TABLE_READ_BIT_KHR;
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            allowed = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                      VK_ACCESS_2_SHADER_SAMPLED_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT |
                      VK_ACCESS_2_SHADER_BINDING_TABLE_READ_BIT_KHR;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            allowed = VK_ACCESS_TRANSFER_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            allowed = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_PREINITIALIZED:
            allowed = VK_ACCESS_HOST_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
            if (aspect & VK_IMAGE_ASPECT_DEPTH_BIT) {
                allowed |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                           VK_ACCESS_2_SHADER_SAMPLED_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT |
                           VK_ACCESS_2_SHADER_BINDING_TABLE_READ_BIT_KHR;
            }
            if (aspect & VK_IMAGE_ASPECT_STENCIL_BIT) {
                allowed |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            }
            break;
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
            if (aspect & VK_IMAGE_ASPECT_DEPTH_BIT) {
                allowed |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            }
            if (aspect & VK_IMAGE_ASPECT_STENCIL_BIT) {
                allowed |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                           VK_ACCESS_2_SHADER_SAMPLED_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT |
                           VK_ACCESS_2_SHADER_BINDING_TABLE_READ_BIT_KHR;
            }
            break;
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
            allowed = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
            allowed = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                      VK_ACCESS_2_SHADER_SAMPLED_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT |
                      VK_ACCESS_2_SHADER_BINDING_TABLE_READ_BIT_KHR;
            break;
        case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
            allowed = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
            allowed = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                      VK_ACCESS_2_SHADER_SAMPLED_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT |
                      VK_ACCESS_2_SHADER_BINDING_TABLE_READ_BIT_KHR;
            break;
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            allowed = VK_ACCESS_NONE;  // PR table says "Must be 0"
            break;
        case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR:
            allowed = all;
            break;
        // alias VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV
        case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
            // alias VK_ACCESS_SHADING_RATE_IMAGE_READ_BIT_NV
            allowed = VK_ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR;
            break;
        case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT:
            allowed = VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT;
            break;
        default:
            // If a new layout is added, will need to manually add it
            return false;
    }

    if ((allowed | access) != allowed) {
        skip |= LogWarning("BestPractices-ImageBarrierAccessLayout", image, loc,
                           "image is %s and accessMask is %s, but for layout %s expected accessMask are %s.",
                           FormatHandle(image).c_str(), string_VkAccessFlags2(access).c_str(), string_VkImageLayout(layout),
                           string_VkAccessFlags2(allowed).c_str());
    }

    return skip;
}

bool BestPractices::ValidateImageMemoryBarrier(const Location& loc, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout,
                                               VkAccessFlags2 srcAccessMask, VkAccessFlags2 dstAccessMask,
                                               VkImageAspectFlags aspectMask) const {
    bool skip = false;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && IsImageLayoutReadOnly(newLayout)) {
        skip |= LogWarning("BestPractices-ImageMemoryBarrier-TransitionUndefinedToReadOnly", image, loc,
                           "VkImageMemoryBarrier is being submitted with oldLayout VK_IMAGE_LAYOUT_UNDEFINED and the contents "
                           "may be discarded, but the newLayout is %s, which is read only.",
                           string_VkImageLayout(newLayout));
    }

    skip |= ValidateAccessLayoutCombination(loc, image, srcAccessMask, oldLayout, aspectMask);
    skip |= ValidateAccessLayoutCombination(loc, image, dstAccessMask, newLayout, aspectMask);

    return skip;
}

bool BestPractices::PreCallValidateCmdPipelineBarrier(
    VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
    VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
    const VkImageMemoryBarrier* pImageMemoryBarriers, const ErrorObject& error_obj) const {
    bool skip = false;

    skip |= CheckPipelineStageFlags(commandBuffer, error_obj.location.dot(Field::srcStageMask), srcStageMask);
    skip |= CheckPipelineStageFlags(commandBuffer, error_obj.location.dot(Field::dstStageMask), dstStageMask);

    for (uint32_t i = 0; i < imageMemoryBarrierCount; ++i) {
        skip |= ValidateImageMemoryBarrier(error_obj.location.dot(Field::pImageMemoryBarriers, i), pImageMemoryBarriers[i].image,
                                           pImageMemoryBarriers[i].oldLayout, pImageMemoryBarriers[i].newLayout,
                                           pImageMemoryBarriers[i].srcAccessMask, pImageMemoryBarriers[i].dstAccessMask,
                                           pImageMemoryBarriers[i].subresourceRange.aspectMask);
    }

    if (VendorCheckEnabled(kBPVendorAMD)) {
        const uint32_t num = num_barriers_objects_.load();
        const uint32_t total_barriers = num + imageMemoryBarrierCount + bufferMemoryBarrierCount;
        if (total_barriers > kMaxRecommendedBarriersSizeAMD) {
            skip |= LogPerformanceWarning("BestPractices-AMD-CmdBuffer-highBarrierCount", commandBuffer, error_obj.location,
                                          "%s In this frame, %" PRIu32 " barriers were already submitted (%" PRIu32
                                          " if you include image and buffer barriers too). Barriers have a high cost and can "
                                          "stall the GPU. "
                                          "Total recommended max is %" PRIu32
                                          ". "
                                          "Consider consolidating and re-organizing the frame to use fewer barriers.",
                                          VendorSpecificTag(kBPVendorAMD), num, total_barriers, kMaxRecommendedBarriersSizeAMD);
        }
    }
    if (VendorCheckEnabled(kBPVendorAMD) || VendorCheckEnabled(kBPVendorNVIDIA)) {
        static constexpr std::array<VkImageLayout, 3> read_layouts = {
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        };

        for (uint32_t i = 0; i < imageMemoryBarrierCount; i++) {
            // read to read barriers
            const auto& image_barrier = pImageMemoryBarriers[i];
            const bool old_is_read_layout =
                std::find(read_layouts.begin(), read_layouts.end(), image_barrier.oldLayout) != read_layouts.end();
            const bool new_is_read_layout =
                std::find(read_layouts.begin(), read_layouts.end(), image_barrier.newLayout) != read_layouts.end();

            if (old_is_read_layout && new_is_read_layout) {
                skip |= LogPerformanceWarning("BestPractices-PipelineBarrier-readToReadBarrier", commandBuffer, error_obj.location,
                                              "%s %s Don't issue read-to-read barriers. "
                                              "Get the resource in the right state the first time you use it.",
                                              VendorSpecificTag(kBPVendorAMD), VendorSpecificTag(kBPVendorNVIDIA));
            }

            // general with no storage
            if (VendorCheckEnabled(kBPVendorAMD) && image_barrier.newLayout == VK_IMAGE_LAYOUT_GENERAL) {
                auto image_state = Get<vvl::Image>(pImageMemoryBarriers[i].image);
                if (image_state && !(image_state->create_info.usage & VK_IMAGE_USAGE_STORAGE_BIT)) {
                    const LogObjectList objlist(commandBuffer, pImageMemoryBarriers[i].image);
                    skip |= LogPerformanceWarning("BestPractices-AMD-vkImage-AvoidGeneral", objlist,
                                                  error_obj.location.dot(Field::pImageMemoryBarriers, i).dot(Field::image),
                                                  "%s VK_IMAGE_LAYOUT_GENERAL should only be used with "
                                                  "VK_IMAGE_USAGE_STORAGE_BIT images.",
                                                  VendorSpecificTag(kBPVendorAMD));
                }
            }
        }
    }

    for (uint32_t i = 0; i < imageMemoryBarrierCount; ++i) {
        skip |= ValidateCmdPipelineBarrierImageBarrier(commandBuffer, pImageMemoryBarriers[i],
                                                       error_obj.location.dot(Field::pImageMemoryBarriers, i));
    }

    return skip;
}

bool BestPractices::PreCallValidateCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfoKHR* pDependencyInfo,
                                                          const ErrorObject& error_obj) const {
    return PreCallValidateCmdPipelineBarrier2(commandBuffer, pDependencyInfo, error_obj);
}

bool BestPractices::PreCallValidateCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                                       const ErrorObject& error_obj) const {
    bool skip = false;

    const Location dep_info_loc = error_obj.location.dot(Field::pDependencyInfo);
    skip |= CheckDependencyInfo(commandBuffer, dep_info_loc, *pDependencyInfo);

    for (uint32_t i = 0; i < pDependencyInfo->imageMemoryBarrierCount; ++i) {
        skip |= ValidateCmdPipelineBarrierImageBarrier(commandBuffer, pDependencyInfo->pImageMemoryBarriers[i],
                                                       dep_info_loc.dot(Field::pImageMemoryBarriers, i));
    }

    return skip;
}

template <typename ImageMemoryBarrier>
bool BestPractices::ValidateCmdPipelineBarrierImageBarrier(VkCommandBuffer commandBuffer, const ImageMemoryBarrier& barrier,
                                                           const Location& loc) const {
    bool skip = false;

    const auto cb_state = GetRead<bp_state::CommandBuffer>(commandBuffer);

    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        if (barrier.oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && barrier.newLayout != VK_IMAGE_LAYOUT_UNDEFINED) {
            skip |= ValidateZcull(*cb_state, barrier.image, barrier.subresourceRange, loc);
        }
    }

    return skip;
}

template <typename Func>
static void ForEachSubresource(const vvl::Image& image, const VkImageSubresourceRange& range, Func&& func) {
    const uint32_t layer_count =
        (range.layerCount == VK_REMAINING_ARRAY_LAYERS) ? (image.full_range.layerCount - range.baseArrayLayer) : range.layerCount;
    const uint32_t level_count =
        (range.levelCount == VK_REMAINING_MIP_LEVELS) ? (image.full_range.levelCount - range.baseMipLevel) : range.levelCount;

    for (uint32_t i = 0; i < layer_count; ++i) {
        const uint32_t layer = range.baseArrayLayer + i;
        for (uint32_t j = 0; j < level_count; ++j) {
            const uint32_t level = range.baseMipLevel + j;
            func(layer, level);
        }
    }
}

template <typename ImageMemoryBarrier>
void BestPractices::RecordCmdPipelineBarrierImageBarrier(VkCommandBuffer commandBuffer, const ImageMemoryBarrier& barrier) {
    auto cb_state = Get<bp_state::CommandBuffer>(commandBuffer);

    // Is a queue ownership acquisition barrier
    if (barrier.srcQueueFamilyIndex != barrier.dstQueueFamilyIndex &&
        barrier.dstQueueFamilyIndex == cb_state->command_pool->queueFamilyIndex) {
        auto image = Get<bp_state::Image>(barrier.image);
        ASSERT_AND_RETURN(image);
        auto subresource_range = barrier.subresourceRange;
        cb_state->queue_submit_functions.emplace_back([image, subresource_range](const vvl::Device& vst, const vvl::Queue& qs,
                                                                                 const vvl::CommandBuffer& cbs) -> bool {
            ForEachSubresource(*image, subresource_range, [&](uint32_t layer, uint32_t level) {
                // Update queue family index without changing usage, signifying a correct queue family transfer
                image->UpdateUsage(layer, level, image->GetUsageType(layer, level), qs.queue_family_index);
            });
            return false;
        });
    }

    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        RecordResetZcullDirection(*cb_state, barrier.image, barrier.subresourceRange);
    }
}

void BestPractices::PostCallRecordCmdPipelineBarrier(
    VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
    VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
    const VkImageMemoryBarrier* pImageMemoryBarriers, const RecordObject& record_obj) {
    BaseClass::PostCallRecordCmdPipelineBarrier(
        commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount,
        pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers, record_obj);

    num_barriers_objects_ += (memoryBarrierCount + imageMemoryBarrierCount + bufferMemoryBarrierCount);

    for (uint32_t i = 0; i < imageMemoryBarrierCount; ++i) {
        RecordCmdPipelineBarrierImageBarrier(commandBuffer, pImageMemoryBarriers[i]);
    }
}

void BestPractices::PostCallRecordCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                                      const RecordObject& record_obj) {
    BaseClass::PostCallRecordCmdPipelineBarrier2(commandBuffer, pDependencyInfo, record_obj);

    for (uint32_t i = 0; i < pDependencyInfo->imageMemoryBarrierCount; ++i) {
        RecordCmdPipelineBarrierImageBarrier(commandBuffer, pDependencyInfo->pImageMemoryBarriers[i]);
    }
}

void BestPractices::PostCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                                         const RecordObject& record_obj) {
    PostCallRecordCmdPipelineBarrier2(commandBuffer, pDependencyInfo, record_obj);
}

bool BestPractices::PreCallValidateCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore,
                                                   const ErrorObject& error_obj) const {
    bool skip = false;
    if (VendorCheckEnabled(kBPVendorAMD) || VendorCheckEnabled(kBPVendorNVIDIA)) {
        const size_t count = Count<vvl::Semaphore>();
        if (count > kMaxRecommendedSemaphoreObjectsSizeAMD) {
            skip |= LogPerformanceWarning("BestPractices-SyncObjects-HighNumberOfSemaphores", device, error_obj.location,
                                          "%s %s High number of vkSemaphore objects created. "
                                          "%zu created, but recommended max is %" PRIu32
                                          ". "
                                          "Minimize the amount of queue synchronization that is used. "
                                          "Each semaphore has a CPU and GPU overhead cost with it.",
                                          VendorSpecificTag(kBPVendorAMD), VendorSpecificTag(kBPVendorNVIDIA), count,
                                          kMaxRecommendedSemaphoreObjectsSizeAMD);
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkFence* pFence,
                                               const ErrorObject& error_obj) const {
    bool skip = false;
    if (VendorCheckEnabled(kBPVendorAMD) || VendorCheckEnabled(kBPVendorNVIDIA)) {
        const size_t count = Count<vvl::Fence>();
        if (count > kMaxRecommendedFenceObjectsSizeAMD) {
            skip |= LogPerformanceWarning("BestPractices-SyncObjects-HighNumberOfFences", device, error_obj.location,
                                          "%s %s High number of VkFence objects created. "
                                          "%zu created, but recommended max is %" PRIu32
                                          ". "
                                          "Minimize the amount of CPU-GPU synchronization that is used. "
                                          "Each fence has a CPU and GPU overhead cost with it.",
                                          VendorSpecificTag(kBPVendorAMD), VendorSpecificTag(kBPVendorNVIDIA), count,
                                          kMaxRecommendedFenceObjectsSizeAMD);
        }
    }

    return skip;
}
