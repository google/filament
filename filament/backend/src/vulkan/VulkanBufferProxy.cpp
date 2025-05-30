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

#include "VulkanBufferProxy.h"

#include "VulkanBufferCache.h"
#include "VulkanMemory.h"

using namespace bluevk;

namespace filament::backend {

VulkanBufferProxy::VulkanBufferProxy(VmaAllocator allocator, VulkanStagePool& stagePool,
        VulkanBufferCache& bufferCache, VulkanBufferUsage usage, uint32_t numBytes)
    : mAllocator(allocator),
      mStagePool(stagePool),
      mBufferCache(bufferCache),
      mBuffer(mBufferCache.acquire(usage, numBytes)),
      mUpdatedOffset(0),
      mUpdatedBytes(0) {}

void VulkanBufferProxy::loadFromCpu(VkCommandBuffer cmdbuf, const void* cpuData,
        uint32_t byteOffset, uint32_t numBytes) {
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
        if (getUsage() == VulkanBufferUsage::UNIFORM) {
            srcAccess = VK_ACCESS_SHADER_READ_BIT;
            srcStage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else if (getUsage() == VulkanBufferUsage::VERTEX) {
            srcAccess = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
            srcStage = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
        } else if (getUsage() == VulkanBufferUsage::INDEX) {
            srcAccess = VK_ACCESS_INDEX_READ_BIT;
            srcStage = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
        }

        VkBufferMemoryBarrier barrier = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            .srcAccessMask = srcAccess,
            .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = getVkBuffer(),
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
    vkCmdCopyBuffer(cmdbuf, stage->buffer, getVkBuffer(), 1, &region);

    mUpdatedOffset = byteOffset;
    mUpdatedBytes = numBytes;

    // Firstly, ensure that the copy finishes before the next draw call.
    // Secondly, in case the user decides to upload another chunk (without ever using the first one)
    // we need to ensure that this upload completes first (hence
    // dstStageMask=VK_PIPELINE_STAGE_TRANSFER_BIT).
    VkAccessFlags dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;

    if (getUsage() == VulkanBufferUsage::VERTEX) {
        dstAccessMask |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
        dstStageMask |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
    } else if (getUsage() == VulkanBufferUsage::INDEX) {
        dstAccessMask |= VK_ACCESS_INDEX_READ_BIT;
        dstStageMask |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
    } else if (getUsage() == VulkanBufferUsage::UNIFORM) {
        dstAccessMask |= VK_ACCESS_SHADER_READ_BIT;
        dstStageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    } else if (getUsage() == VulkanBufferUsage::SHADER_STORAGE) {
        // TODO: implement me
    }

    VkBufferMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = dstAccessMask,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = getVkBuffer(),
        .offset = byteOffset,
        .size = numBytes,
    };

    vkCmdPipelineBarrier(cmdbuf, VK_PIPELINE_STAGE_TRANSFER_BIT, dstStageMask, 0, 0, nullptr, 1,
            &barrier, 0, nullptr);
}

VkBuffer VulkanBufferProxy::getVkBuffer() const noexcept {
    return mBuffer->getGpuBuffer()->vkbuffer;
}

VulkanBufferUsage VulkanBufferProxy::getUsage() const noexcept {
    return mBuffer->getGpuBuffer()->usage;
}

}// namespace filament::backend
