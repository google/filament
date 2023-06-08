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

VulkanBuffer::VulkanBuffer(VmaAllocator allocator, VulkanCommands* commands,
        VulkanStagePool& stagePool, VkBufferUsageFlags usage, uint32_t numBytes)
    : mAllocator(allocator), mCommands(commands), mStagePool(stagePool), mUsage(usage) {

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
    assert_invariant(mGpuMemory == VK_NULL_HANDLE);
    assert_invariant(mGpuBuffer == VK_NULL_HANDLE);
}

void VulkanBuffer::terminate() {
    vmaDestroyBuffer(mAllocator, mGpuBuffer, mGpuMemory);
    mGpuMemory = VK_NULL_HANDLE;
    mGpuBuffer = VK_NULL_HANDLE;
}

void VulkanBuffer::loadFromCpu(const void* cpuData, uint32_t byteOffset, uint32_t numBytes) const {
    assert_invariant(byteOffset == 0);
    VulkanStage const* stage = mStagePool.acquireStage(numBytes);
    void* mapped;
    vmaMapMemory(mAllocator, stage->memory, &mapped);
    memcpy(mapped, cpuData, numBytes);
    vmaUnmapMemory(mAllocator, stage->memory);
    vmaFlushAllocation(mAllocator, stage->memory, byteOffset, numBytes);

    VkCommandBuffer const cmdbuffer = mCommands->get(true).cmdbuffer;

    VkBufferCopy region{ .size = numBytes };
    vkCmdCopyBuffer(cmdbuffer, stage->buffer, mGpuBuffer, 1, &region);

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
        dstAccessMask |= VK_ACCESS_UNIFORM_READ_BIT;
        // NOTE: ideally dstStageMask would include VERTEX_SHADER_BIT | FRAGMENT_SHADER_BIT, but
        // this seems to be insufficient on Mali devices. To work around this we are using a more
        // aggressive ALL_GRAPHICS_BIT barrier.
        dstStageMask |= VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    } else if (mUsage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) {
        // TODO: implement me
    }

    VkBufferMemoryBarrier barrier{
	    .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
	    .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
	    .dstAccessMask = dstAccessMask,
	    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
	    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
	    .buffer = mGpuBuffer,
	    .size = VK_WHOLE_SIZE,
    };

    vkCmdPipelineBarrier(cmdbuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, dstStageMask, 0, 0, nullptr, 1,
	    &barrier, 0, nullptr);
}

} // namespace filament::backend
