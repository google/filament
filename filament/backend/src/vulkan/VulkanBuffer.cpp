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

namespace filament {
namespace backend {

VulkanBuffer::VulkanBuffer(VulkanContext& context, VulkanStagePool& stagePool,
        VkBufferUsageFlags usage, uint32_t numBytes) {
    // Create the VkBuffer.
    VkBufferCreateInfo bufferInfo {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = numBytes,
        .usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT
    };

    VmaAllocationCreateInfo allocInfo { .pool = context.vmaPoolGPU };
    vmaCreateBuffer(context.allocator, &bufferInfo, &allocInfo, &mGpuBuffer, &mGpuMemory, nullptr);
}

VulkanBuffer::~VulkanBuffer() {
    assert_invariant(mGpuMemory == VK_NULL_HANDLE);
    assert_invariant(mGpuBuffer == VK_NULL_HANDLE);
}

void VulkanBuffer::terminate(VulkanContext& context) {
    vmaDestroyBuffer(context.allocator, mGpuBuffer, mGpuMemory);
    mGpuMemory = VK_NULL_HANDLE;
    mGpuBuffer = VK_NULL_HANDLE;
}

void VulkanBuffer::loadFromCpu(VulkanContext& context, VulkanStagePool& stagePool,
        const void* cpuData, uint32_t byteOffset, uint32_t numBytes) const {
    assert_invariant(byteOffset == 0);
    VulkanStage const* stage = stagePool.acquireStage(numBytes);
    void* mapped;
    vmaMapMemory(context.allocator, stage->memory, &mapped);
    memcpy(mapped, cpuData, numBytes);
    vmaUnmapMemory(context.allocator, stage->memory);
    vmaFlushAllocation(context.allocator, stage->memory, byteOffset, numBytes);

    const VkCommandBuffer cmdbuffer = context.commands->get().cmdbuffer;

    VkBufferCopy region { .size = numBytes };
    vkCmdCopyBuffer(cmdbuffer, stage->buffer, mGpuBuffer, 1, &region);

    // Firstly, ensure that the copy finishes before the next draw call.
    // Secondly, in case the user decides to upload another chunk (without ever using the first one)
    // we need to ensure that this upload completes first.
    VkBufferMemoryBarrier barrier {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_INDEX_READ_BIT |
                         VK_ACCESS_TRANSFER_WRITE_BIT,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = mGpuBuffer,
        .size = VK_WHOLE_SIZE
    };

    vkCmdPipelineBarrier(cmdbuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT,
            0, 0, nullptr, 1, &barrier, 0, nullptr);
}

} // namespace filament
} // namespace backend
