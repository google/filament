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

#include "driver/vulkan/VulkanBuffer.h"

#include <utils/Panic.h>

namespace filament {
namespace driver {

VulkanBuffer::VulkanBuffer(VulkanContext& context, VulkanStagePool& stagePool,
        VkBufferUsageFlags usage, uint32_t numBytes) : mContext(context), mStagePool(stagePool) {
    // Create the VkBuffer.
    VkBufferCreateInfo bufferInfo {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = numBytes,
        .usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT
    };
    VmaAllocationCreateInfo allocInfo {
        .usage = VMA_MEMORY_USAGE_GPU_ONLY
    };
    vmaCreateBuffer(context.allocator, &bufferInfo, &allocInfo, &mGpuBuffer, &mGpuMemory, nullptr);
}

VulkanBuffer::~VulkanBuffer() {
    assert(!hasPendingWork(mContext) && "Buffer destroyed while work is pending.");
    vmaDestroyBuffer(mContext.allocator, mGpuBuffer, mGpuMemory);
}

void VulkanBuffer::loadFromCpu(const void* cpuData, uint32_t byteOffset, uint32_t numBytes) {
    assert(byteOffset == 0);
    VkDevice device = mContext.device;
    VulkanStage const* stage = mStagePool.acquireStage(numBytes);
    void* mapped;
    vmaMapMemory(mContext.allocator, stage->memory, &mapped);
    memcpy(mapped, cpuData, numBytes);
    vmaUnmapMemory(mContext.allocator, stage->memory);
    vmaFlushAllocation(mContext.allocator, stage->memory, byteOffset, numBytes);

    // Create and submit a one-off command buffer to allow uploading outside a frame.
    VkCommandBuffer cmdbuffer;
    VkFence fence;
    VkCommandBufferBeginInfo beginInfo { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    VkCommandBufferAllocateInfo allocateInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = mContext.commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };
    VkFenceCreateInfo fenceCreateInfo { .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    VkBufferCopy region { .size = numBytes };
    vkAllocateCommandBuffers(device, &allocateInfo, &cmdbuffer);
    vkCreateFence(device, &fenceCreateInfo, VKALLOC, &fence);
    vkBeginCommandBuffer(cmdbuffer, &beginInfo);
    vkCmdCopyBuffer(cmdbuffer, stage->buffer, mGpuBuffer, 1, &region);

    // Ensure that the copy finishes before the next draw call.
    VkBufferMemoryBarrier barrier {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_INDEX_READ_BIT,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = mGpuBuffer,
        .size = VK_WHOLE_SIZE
    };
    vkCmdPipelineBarrier(cmdbuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0, nullptr, 1, &barrier, 0, nullptr);
    vkEndCommandBuffer(cmdbuffer);
    VkSubmitInfo submitInfo {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmdbuffer,
    };
    vkQueueSubmit(mContext.graphicsQueue, 1, &submitInfo, fence);

    // Enqueue some work to reclaim the staging area and free the command buffer. The pipeline
    // barrier we already placed is a GPU-to-GPU sync point, but reclamation of the staging area
    // needs GPU-CPU synchronization. That's what the fence is for.
    mContext.pendingWork.emplace_back([this, fence, device, cmdbuffer, stage] (VkCommandBuffer)  {
        vkWaitForFences(device, 1, &fence, VK_FALSE, UINT64_MAX);
        vkFreeCommandBuffers(device, mContext.commandPool, 1, &cmdbuffer);
        vkDestroyFence(device, fence, VKALLOC);
        mStagePool.releaseStage(stage);
    });
}

} // namespace filament
} // namespace driver
