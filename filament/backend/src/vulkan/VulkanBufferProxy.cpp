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

#include "vulkan/utils/CommandBufferAgeCounter.h"

#include <backend/DriverEnums.h>

#include <utils/compiler.h>
#include <utils/debug.h>

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
          mUsage(usage),
          mBinding(binding),
          mNumBytes(numBytes) {}

void VulkanBufferProxy::loadFromCpu(VulkanCommandBuffer& commands,
        fvkutils::CommandBufferAgeCounter const& ageCounter, const void* cpuData,
        uint32_t byteOffset, uint32_t numBytes) {
    // The frontend is what actually enforces this, via always-on preconditions on setBuffer() and
    // friends. This is only a debug-time invariant check on internal callers, catching a bad range
    // before it reaches the memcpy below or the vkCmdCopyBuffer on the staging path.
    // Note that VulkanBufferCache recycles pooled allocations, so `capacity` is the size of the
    // underlying allocation, which may exceed the logical size the frontend asked for; this is an
    // upper bound and cannot be tightened to an exact-size check.
    // Written as two comparisons so that a large byteOffset cannot wrap around.
    UTILS_UNUSED_IN_RELEASE uint32_t const capacity = mBuffer->getGpuBuffer()->numBytes;
    assert_invariant(numBytes <= capacity && byteOffset <= capacity - numBytes);

    bool isBeingRead = false;
    auto [barrierAge, readingAges] = mBuffer->getAges();
    readingAges.forEachSetBit([barrierAge, &ageCounter, &isBeingRead](size_t index) {
        isBeingRead = isBeingRead || !ageCounter.isDone(index + barrierAge + 1);
    });

    // Check if we can just memcpy directly to the GPU memory.
    bool const isMemcopyable = mBuffer->isMemcopyable();

    // If the buffer is marked as shared, it is guaranteed to be safe to do a memcpy.
    bool const isShared = any(mUsage & BufferUsage::SHARED_WRITE_BIT);

    // This is a whole buffer update. We use `isMemcopyable` as a proxy for UMA support.
    // If the current buffer *is not* being read, then we can update without conflict.
    // If the current buffer *is* being read, just acquire another buffer.  The buffer being read is
    // now orphaned and will be recycled when it's done.
    // This is limited to uniform buffers: descriptor sets detect the swap (see getGeneration()) and
    // are re-pointed to the new VkBuffer, whereas other users cache the VkBuffer handle (e.g.
    // VulkanVertexBuffer) and would be left with a dangling one.
    bool const canOrphan = mStagingBufferBypassEnabled && isMemcopyable && !isShared &&
                           mBinding == VulkanBufferBinding::UNIFORM;
    if (canOrphan && isBeingRead && byteOffset == 0 && numBytes == mNumBytes) {
        mBuffer = mBufferCache.acquire(mBinding, mNumBytes);
        // Descriptor sets that recorded an older generation now reference a stale VkBuffer.
        mGeneration++;
        isBeingRead = false;
    }

    // Writing directly into a buffer that is not being read is gated behind the staging buffer
    // bypass feature flag.
    bool const isSafeToWrite = isShared || (mStagingBufferBypassEnabled && !isBeingRead);

    // Keep track of the VulkanBuffer usage
    commands.acquire(mBuffer);

    // Proceed with memcpy directly into the buffer.
    if (isSafeToWrite && isMemcopyable) {
        char* dest = static_cast<char*>(mBuffer->getGpuBuffer()->allocationInfo.pMappedData) +
                     byteOffset;
        memcpy(dest, cpuData, numBytes);
        vmaFlushAllocation(mAllocator, mBuffer->getGpuBuffer()->vmaAllocation, byteOffset,
                numBytes);
        return;
    }

    // Note: this should be stored within the command buffer before going out of
    // scope, so that the command buffer can manage its lifecycle.
    fvkmemory::resource_ptr<VulkanStage::Segment> stage = mStagePool.acquireStage(numBytes);
    assert_invariant(stage->memory());
    commands.acquire(stage);
    memcpy(stage->mapping(), cpuData, numBytes);
    vmaFlushAllocation(mAllocator, stage->memory(), stage->offset(), numBytes);

    // This means that we're recording a write into a command buffer with a previous read, so it
    // needs to add a barrier (to protect the write from writing over a read).
    bool const issueBarrier = mBuffer->isReadBy(commands.age());

    // If there was a previous read, then we need to make sure the following write is properly
    // synced with the previous read.
    if (issueBarrier) {
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

    // The copy executes when `commands` executes. Until then, a later loadFromCpu() must not
    // memcpy into this buffer, or this pending copy would overwrite the newer data.
    mBuffer->write(ageCounter.getBarrierAge(), commands.age());

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
        dstAccessMask |= VK_ACCESS_UNIFORM_READ_BIT;
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

void VulkanBufferProxy::referencedBy(VulkanCommandBuffer& commands,
        fvkutils::CommandBufferAgeCounter const& ageCounter) {
    commands.acquire(mBuffer);
    mBuffer->read(ageCounter.getBarrierAge(), commands.age());
}

} // namespace filament::backend
