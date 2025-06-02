/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "VulkanBuffer.h"
#include "VulkanMemory.h"

#include <utils/Panic.h>

using namespace bluevk;

namespace filament::backend {

VulkanBuffer::VulkanBuffer(VmaAllocator allocator, VulkanStagePool& stagePool,
        VkBufferUsageFlags usage, uint32_t numBytes)
    : mAllocator(allocator),
      mStagePool(stagePool),
      mUsage(usage),
      mUpdatedOffset(0),
      mUpdatedBytes(0) {
    // for now make sure that only 1 bit is set in usage
    // (because loadFromCpu() assumes that somewhat)
    assert_invariant(usage && !(usage & (usage - 1)));

    // Create the VkBuffer.
    VkBufferCreateInfo bufferInfo {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = numBytes,
        .usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT
    };

    VmaAllocationCreateInfo allocInfo { .usage = VMA_MEMORY_USAGE_GPU_ONLY };
    vmaCreateBuffer(mAllocator, &bufferInfo, &allocInfo, &mGpuBuffer, &mGpuMemory, nullptr);
}

VulkanBuffer::~VulkanBuffer() {
    vmaDestroyBuffer(mAllocator, mGpuBuffer, mGpuMemory);
}

void VulkanBuffer::loadFromCpu(VkCommandBuffer cmdbuf, const void* cpuData, uint32_t byteOffset,
        uint32_t numBytes) {
    VulkanStage const* stage = mStagePool.acquireStage(numBytes);
    void* mapped;
    vmaMapMemory(mAllocator, stage->memory, &mapped);
    memcpy(mapped, cpuData, numBytes);
    vmaUnmapMemory(mAllocator, stage->memory);
    vmaFlushAllocation(mAllocator, stage->memory, 0, numBytes);

    // If there was a previous update, then we need to make sure the following write is properly
    // synced with the previous read.
    if (mUpdatedBytes > 0 &&
            (byteOffset >= mUpdatedOffset && byteOffset <= (mUpdatedOffset + mUpdatedBytes))) {
        VkAccessFlags srcAccess = 0;
        VkPipelineStageFlags srcStage = 0;
        if (mUsage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) {
            srcAccess = VK_ACCESS_SHADER_READ_BIT;
            srcStage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else if (mUsage & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) {
            srcAccess = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
            srcStage = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
        } else if (mUsage & VK_BUFFER_USAGE_INDEX_BUFFER_BIT) {
            srcAccess = VK_ACCESS_INDEX_READ_BIT;
            srcStage = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
        }

        VkBufferMemoryBarrier barrier = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            .srcAccessMask = srcAccess,
            .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = mGpuBuffer,
            .offset = byteOffset,
            .size = numBytes,
        };
        vkCmdPipelineBarrier(cmdbuf, srcStage, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1,
                &barrier, 0, nullptr);
    }

    VkBufferCopy region = {
        .srcOffset = 0,
        .dstOffset = byteOffset,
        .size = numBytes,
    };
    vkCmdCopyBuffer(cmdbuf, stage->buffer, mGpuBuffer, 1, &region);

    mUpdatedOffset = byteOffset;
    mUpdatedBytes = numBytes;

    // Firstly, ensure that the copy finishes before the next draw call.
    // Secondly, in case the user decides to upload another chunk (without ever using the first one)
    // we need to ensure that this upload completes first (hence
    // dstStageMask=VK_PIPELINE_STAGE_TRANSFER_BIT).
    VkAccessFlags dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;

    if (mUsage & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) {
        dstAccessMask |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
        dstStageMask |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
    } else if (mUsage & VK_BUFFER_USAGE_INDEX_BUFFER_BIT) {
        dstAccessMask |= VK_ACCESS_INDEX_READ_BIT;
        dstStageMask |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
    } else if (mUsage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) {
        dstAccessMask |= VK_ACCESS_SHADER_READ_BIT;
        dstStageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    } else if (mUsage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) {
        // TODO: implement me
    }

    VkBufferMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = dstAccessMask,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = mGpuBuffer,
        .offset = byteOffset,
        .size = numBytes,
    };

    vkCmdPipelineBarrier(cmdbuf, VK_PIPELINE_STAGE_TRANSFER_BIT, dstStageMask, 0, 0, nullptr, 1,
            &barrier, 0, nullptr);
}

} // namespace filament::backend
