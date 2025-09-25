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
#include "VulkanCommands.h"
#include "VulkanContext.h"
#include "VulkanHandles.h"
#include "VulkanMemory.h"

#include <backend/DriverEnums.h>

using namespace bluevk;

namespace filament::backend {

VulkanBufferProxy::VulkanBufferProxy(VulkanContext const& context, VmaAllocator allocator,
        VulkanStagePool& stagePool, VulkanBufferCache& bufferCache, VulkanBufferBinding binding,
        BufferUsage usage, uint32_t numBytes)
    : mStagingBufferBypassEnabled(context.stagingBufferBypassEnabled()),
      mAllocator(allocator),
      mStagePool(stagePool),
      mBufferCache(bufferCache),
      mBuffer(mBufferCache.acquire(binding, numBytes)),
      mLastReadAge(0),
      mUsage(usage) {}

void VulkanBufferProxy::loadFromCpu(VulkanCommandBuffer& commands, const void* cpuData,
        uint32_t byteOffset, uint32_t numBytes) {

    // This means that we're recording a write into a command buffer without a previous read, so it
    // should be safe to
    //   1) Do a direct memcpy in UMA mode
    //   2) Skip adding a barrier (to protect the write from writing over a read).
    bool const isAvailable = commands.age() != mLastReadAge;

    // Keep track of the VulkanBuffer usage
    commands.acquire(mBuffer);

    // Check if we can just memcpy directly to the GPU memory.
    bool const isMemcopyable = mBuffer->getGpuBuffer()->allocationInfo.pMappedData != nullptr;
    
    // In the case of UNIFORMS, check that is available to see to know if a memcpy is possible.
    // This works regardless if it's a full or partial update of the buffer.
    bool const isUniformAvailable = getBinding() == VulkanBufferBinding::UNIFORM && isAvailable;
    
    // In the case the content is marked as memory mapped or static, is guaranteed to be safe to do
    // a memcpy if its available.
    bool isStaticOrShared = any(mUsage & (BufferUsage::STATIC | BufferUsage::SHARED_WRITE_BIT));
    bool const useMemcpy = (isUniformAvailable || isStaticOrShared) && isMemcopyable && mStagingBufferBypassEnabled;
    if (useMemcpy) {
        char* dest = static_cast<char*>(mBuffer->getGpuBuffer()->allocationInfo.pMappedData) +
                     byteOffset;
        memcpy(dest, cpuData, numBytes);
        vmaFlushAllocation(mAllocator, mBuffer->getGpuBuffer()->vmaAllocation, byteOffset,
                numBytes);
        return;

        // TODO: to properly bypass staging buffer, we'd need to be able to swap out a VulkanBuffer,
        // which represents a VkBuffer. This means that the corresponding descriptor sets also have
        // to be updated.
    }

    // Note: this should be stored within the command buffer before going out of
    // scope, so that the command buffer can manage its lifecycle.
    fvkmemory::resource_ptr<VulkanStage::Segment> stage = mStagePool.acquireStage(numBytes);
    assert_invariant(stage->memory());
    commands.acquire(stage);
    memcpy(stage->mapping(), cpuData, numBytes);
    vmaFlushAllocation(mAllocator, stage->memory(), stage->offset(), numBytes);

    // If there was a previous read, then we need to make sure the following write is properly
    // synced with the previous read.
    if (!isAvailable) {
        VkAccessFlags srcAccess = 0;
        VkPipelineStageFlags srcStage = 0;
        if (getBinding() == VulkanBufferBinding::UNIFORM) {
            srcAccess = VK_ACCESS_SHADER_READ_BIT;
            srcStage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else if (getBinding() == VulkanBufferBinding::VERTEX) {
            srcAccess = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
            srcStage = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
        } else if (getBinding() == VulkanBufferBinding::INDEX) {
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
        vkCmdPipelineBarrier(commands.buffer(), srcStage, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
                nullptr, 1, &barrier, 0, nullptr);
    }

    VkBufferCopy region = {
        .srcOffset = stage->offset(),
        .dstOffset = byteOffset,
        .size = numBytes,
    };
    vkCmdCopyBuffer(commands.buffer(), stage->buffer(), getVkBuffer(), 1, &region);

    // Firstly, ensure that the copy finishes before the next draw call.
    // Secondly, in case the user decides to upload another chunk (without ever using the first one)
    // we need to ensure that this upload completes first (hence
    // dstStageMask=VK_PIPELINE_STAGE_TRANSFER_BIT).
    VkAccessFlags dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;

    if (getBinding() == VulkanBufferBinding::VERTEX) {
        dstAccessMask |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
        dstStageMask |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
    } else if (getBinding() == VulkanBufferBinding::INDEX) {
        dstAccessMask |= VK_ACCESS_INDEX_READ_BIT;
        dstStageMask |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
    } else if (getBinding() == VulkanBufferBinding::UNIFORM) {
        dstAccessMask |= VK_ACCESS_SHADER_READ_BIT;
        dstStageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    } else if (getBinding() == VulkanBufferBinding::SHADER_STORAGE) {
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

    vkCmdPipelineBarrier(commands.buffer(), VK_PIPELINE_STAGE_TRANSFER_BIT, dstStageMask, 0, 0,
            nullptr, 1, &barrier, 0, nullptr);
}

VkBuffer VulkanBufferProxy::getVkBuffer() const noexcept {
    return mBuffer->getGpuBuffer()->vkbuffer;
}

VulkanBufferBinding VulkanBufferProxy::getBinding() const noexcept {
    return mBuffer->getGpuBuffer()->binding;
}

void VulkanBufferProxy::referencedBy(VulkanCommandBuffer& commands) {
    commands.acquire(mBuffer);
    mLastReadAge = commands.age();
}

} // namespace filament::backend
