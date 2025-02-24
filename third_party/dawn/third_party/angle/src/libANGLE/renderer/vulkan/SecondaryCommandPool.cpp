//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SecondaryCommandPool:
//    A class for allocating Command Buffers for VulkanSecondaryCommandBuffer.
//

#include "libANGLE/renderer/vulkan/SecondaryCommandPool.h"

#include "common/debug.h"
#include "libANGLE/renderer/vulkan/vk_renderer.h"
#include "libANGLE/renderer/vulkan/vk_utils.h"

namespace rx
{
namespace vk
{

SecondaryCommandPool::SecondaryCommandPool() : mCollectedBuffers(kFixedQueueLimit) {}

SecondaryCommandPool::~SecondaryCommandPool()
{
    ASSERT(mCollectedBuffers.empty());
    ASSERT(mCollectedBuffersOverflow.empty());
}

angle::Result SecondaryCommandPool::init(ErrorContext *context,
                                         uint32_t queueFamilyIndex,
                                         ProtectionType protectionType)
{
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags                   = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolInfo.queueFamilyIndex        = queueFamilyIndex;
    if (context->getFeatures().useResetCommandBufferBitForSecondaryPools.enabled)
    {
        poolInfo.flags |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    }
    ASSERT(protectionType == ProtectionType::Unprotected ||
           protectionType == ProtectionType::Protected);
    if (protectionType == ProtectionType::Protected)
    {
        poolInfo.flags |= VK_COMMAND_POOL_CREATE_PROTECTED_BIT;
    }
    ANGLE_VK_TRY(context, mCommandPool.init(context->getDevice(), poolInfo));
    return angle::Result::Continue;
}

void SecondaryCommandPool::destroy(VkDevice device)
{
    // Command buffers will be destroyed with the Pool. Avoid possible slowdown during cleanup.
    mCollectedBuffers.clear();
    mCollectedBuffersOverflow.clear();
    mCommandPool.destroy(device);
}

angle::Result SecondaryCommandPool::allocate(ErrorContext *context,
                                             VulkanSecondaryCommandBuffer *buffer)
{
    ASSERT(valid());
    ASSERT(!buffer->valid());

    VkDevice device = context->getDevice();

    freeCollectedBuffers(device);

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level                       = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    allocInfo.commandBufferCount          = 1;
    allocInfo.commandPool                 = mCommandPool.getHandle();

    ANGLE_VK_TRY(context, buffer->init(device, allocInfo));

    return angle::Result::Continue;
}

void SecondaryCommandPool::collect(VulkanSecondaryCommandBuffer *buffer)
{
    ASSERT(valid());
    ASSERT(buffer->valid());

    VkCommandBuffer bufferHandle = buffer->releaseHandle();

    if (!mCollectedBuffers.full())
    {
        mCollectedBuffers.push(bufferHandle);
    }
    else
    {
        std::lock_guard<angle::SimpleMutex> lock(mOverflowMutex);
        mCollectedBuffersOverflow.emplace_back(bufferHandle);
        mHasOverflow.store(true, std::memory_order_relaxed);
    }
}

void SecondaryCommandPool::freeCollectedBuffers(VkDevice device)
{
    // Free Command Buffer for now. May later add recycling or reset/free pool at once.
    ANGLE_TRACE_EVENT0("gpu.angle", "SecondaryCommandPool::freeCollectedBuffers");

    while (!mCollectedBuffers.empty())
    {
        VkCommandBuffer bufferHandle = mCollectedBuffers.front();
        mCommandPool.freeCommandBuffers(device, 1, &bufferHandle);
        mCollectedBuffers.pop();
    }

    if (ANGLE_UNLIKELY(mHasOverflow.load(std::memory_order_relaxed)))
    {
        std::vector<VkCommandBuffer> buffers;
        {
            std::lock_guard<angle::SimpleMutex> lock(mOverflowMutex);
            buffers = std::move(mCollectedBuffersOverflow);
            mHasOverflow.store(false, std::memory_order_relaxed);
        }
        for (VkCommandBuffer bufferHandle : buffers)
        {
            mCommandPool.freeCommandBuffers(device, 1, &bufferHandle);
        }
    }
}

}  // namespace vk
}  // namespace rx
