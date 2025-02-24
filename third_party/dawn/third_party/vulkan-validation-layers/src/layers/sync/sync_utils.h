/*
 * Copyright (c) 2019-2025 Valve Corporation
 * Copyright (c) 2019-2025 LunarG, Inc.
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

#pragma once
#include "generated/sync_validation_types.h"
#include "generated/vk_object_types.h"
#include <vulkan/vulkan.h>
#include <string>

// Remove Windows trojan macro that prevents usage of this name in any scope of the program.
// For example, nested namespace type sync_utils::MemoryBarrier won't compile because of this.
#if defined(VK_USE_PLATFORM_WIN32_KHR) && defined(MemoryBarrier)
#undef MemoryBarrier
#endif

struct DeviceFeatures;
struct DeviceExtensions;

namespace vvl {
class Image;
class Buffer;
}  // namespace vvl

namespace sync_utils {

static constexpr VkQueueFlags kAllQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
static constexpr VkAccessFlags2 kAllAccesses = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;

VkPipelineStageFlags2 DisabledPipelineStages(const DeviceFeatures& features, const DeviceExtensions& device_extensions);

// Expand all pipeline stage bits. If queue_flags and disabled_feature_mask is provided, the expansion of ALL_COMMANDS_BIT
// and ALL_GRAPHICS_BIT will be limited to what is supported.
VkPipelineStageFlags2 ExpandPipelineStages(VkPipelineStageFlags2 stage_mask, VkQueueFlags queue_flags = kAllQueueTypes,
                                           const VkPipelineStageFlags2 disabled_feature_mask = 0);

VkAccessFlags2 ExpandAccessFlags(VkAccessFlags2 access_mask);

VkAccessFlags2 CompatibleAccessMask(VkPipelineStageFlags2 stage_mask);

VkPipelineStageFlags2 WithEarlierPipelineStages(VkPipelineStageFlags2 stage_mask);

VkPipelineStageFlags2 WithLaterPipelineStages(VkPipelineStageFlags2 stage_mask);

std::string StringPipelineStageFlags(VkPipelineStageFlags2 mask);

std::string StringAccessFlags(VkAccessFlags2 mask);

// If mask contains ALL of expand_bits, then clear these bits and add a meta_mask
void ReplaceExpandBitsWithMetaMask(VkFlags64& mask, VkFlags64 expand_bits, VkFlags64 meta_mask);

struct ExecScopes {
    VkPipelineStageFlags2 src;
    VkPipelineStageFlags2 dst;
};
ExecScopes GetGlobalStageMasks(const VkDependencyInfo& dep_info);

struct ShaderStageAccesses {
    SyncAccessIndex sampled_read;
    SyncAccessIndex storage_read;
    SyncAccessIndex storage_write;
    SyncAccessIndex uniform_read;
};
ShaderStageAccesses GetShaderStageAccesses(VkShaderStageFlagBits shader_stage);

struct MemoryBarrier {
    VkPipelineStageFlags2 srcStageMask;
    VkAccessFlags2 srcAccessMask;
    VkPipelineStageFlags2 dstStageMask;
    VkAccessFlags2 dstAccessMask;

    explicit MemoryBarrier(const VkMemoryBarrier2& barrier)
        : srcStageMask(barrier.srcStageMask),
          srcAccessMask(barrier.srcAccessMask),
          dstStageMask(barrier.dstStageMask),
          dstAccessMask(barrier.dstAccessMask) {}
    explicit MemoryBarrier(const VkBufferMemoryBarrier2& barrier)
        : srcStageMask(barrier.srcStageMask),
          srcAccessMask(barrier.srcAccessMask),
          dstStageMask(barrier.dstStageMask),
          dstAccessMask(barrier.dstAccessMask) {}
    explicit MemoryBarrier(const VkImageMemoryBarrier2& barrier)
        : srcStageMask(barrier.srcStageMask),
          srcAccessMask(barrier.srcAccessMask),
          dstStageMask(barrier.dstStageMask),
          dstAccessMask(barrier.dstAccessMask) {}
    MemoryBarrier(const VkMemoryBarrier& barrier, VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask)
        : srcStageMask(src_stage_mask),
          srcAccessMask(barrier.srcAccessMask),
          dstStageMask(dst_stage_mask),
          dstAccessMask(barrier.dstAccessMask) {}
    MemoryBarrier(const VkBufferMemoryBarrier& barrier, VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask)
        : srcStageMask(src_stage_mask),
          srcAccessMask(barrier.srcAccessMask),
          dstStageMask(dst_stage_mask),
          dstAccessMask(barrier.dstAccessMask) {}
    MemoryBarrier(const VkImageMemoryBarrier& barrier, VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask)
        : srcStageMask(src_stage_mask),
          srcAccessMask(barrier.srcAccessMask),
          dstStageMask(dst_stage_mask),
          dstAccessMask(barrier.dstAccessMask) {}
};

enum class OwnershipTransferOp { none, release, acquire };

// OwnershipTransferBarrier is not a standalone barrier type; it is part of a buffer/image barrier.
// Similar to MemoryBarrier, it can be used when buffer/image specific information is not needed.
struct OwnershipTransferBarrier : MemoryBarrier {
    uint32_t srcQueueFamilyIndex;
    uint32_t dstQueueFamilyIndex;

    OwnershipTransferBarrier(const VkBufferMemoryBarrier2& barrier)
        : MemoryBarrier(barrier),
          srcQueueFamilyIndex(barrier.srcQueueFamilyIndex),
          dstQueueFamilyIndex(barrier.dstQueueFamilyIndex) {}
    OwnershipTransferBarrier(const VkBufferMemoryBarrier& barrier, VkPipelineStageFlags src_stage_mask,
                       VkPipelineStageFlags dst_stage_mask)
        : MemoryBarrier(barrier, src_stage_mask, dst_stage_mask),
          srcQueueFamilyIndex(barrier.srcQueueFamilyIndex),
          dstQueueFamilyIndex(barrier.dstQueueFamilyIndex) {}
    OwnershipTransferBarrier(const VkImageMemoryBarrier2& barrier)
        : MemoryBarrier(barrier),
          srcQueueFamilyIndex(barrier.srcQueueFamilyIndex),
          dstQueueFamilyIndex(barrier.dstQueueFamilyIndex) {}
    OwnershipTransferBarrier(const VkImageMemoryBarrier& barrier, VkPipelineStageFlags src_stage_mask,
                       VkPipelineStageFlags dst_stage_mask)
        : MemoryBarrier(barrier, src_stage_mask, dst_stage_mask),
          srcQueueFamilyIndex(barrier.srcQueueFamilyIndex),
          dstQueueFamilyIndex(barrier.dstQueueFamilyIndex) {}

    OwnershipTransferOp TransferOp(uint32_t command_pool_queue_family) const {
        if (srcQueueFamilyIndex != dstQueueFamilyIndex) {
            if (command_pool_queue_family == srcQueueFamilyIndex) {
                return OwnershipTransferOp::release;
            } else if (command_pool_queue_family == dstQueueFamilyIndex) {
                return OwnershipTransferOp::acquire;
            }
        }
        return OwnershipTransferOp::none;
    }
};

struct BufferBarrier : OwnershipTransferBarrier {
    VkBuffer buffer;
    VkDeviceSize offset;
    VkDeviceSize size;

    explicit BufferBarrier(const VkBufferMemoryBarrier2& barrier)
        : OwnershipTransferBarrier(barrier), buffer(barrier.buffer), offset(barrier.offset), size(barrier.size) {}
    BufferBarrier(const VkBufferMemoryBarrier& barrier, VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask)
        : OwnershipTransferBarrier(barrier, src_stage_mask, dst_stage_mask),
          buffer(barrier.buffer),
          offset(barrier.offset),
          size(barrier.size) {}
};

struct ImageBarrier : OwnershipTransferBarrier {
    VkImageLayout oldLayout;
    VkImageLayout newLayout;
    VkImage image;
    VkImageSubresourceRange subresourceRange;

    explicit ImageBarrier(const VkImageMemoryBarrier2& barrier)
        : OwnershipTransferBarrier(barrier),
          oldLayout(barrier.oldLayout),
          newLayout(barrier.newLayout),
          image(barrier.image),
          subresourceRange(barrier.subresourceRange) {}
    ImageBarrier(const VkImageMemoryBarrier& barrier, VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask)
        : OwnershipTransferBarrier(barrier, src_stage_mask, dst_stage_mask),
          oldLayout(barrier.oldLayout),
          newLayout(barrier.newLayout),
          image(barrier.image),
          subresourceRange(barrier.subresourceRange) {}
};

}  // namespace sync_utils
