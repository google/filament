//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// VulkanSecondaryCommandBuffer:
//    Implementation of VulkanSecondaryCommandBuffer.
//

#include "libANGLE/renderer/vulkan/VulkanSecondaryCommandBuffer.h"

#include "common/debug.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/vk_utils.h"

namespace rx
{
namespace vk
{
angle::Result VulkanSecondaryCommandBuffer::InitializeCommandPool(ErrorContext *context,
                                                                  SecondaryCommandPool *pool,
                                                                  uint32_t queueFamilyIndex,
                                                                  ProtectionType protectionType)
{
    ANGLE_TRY(pool->init(context, queueFamilyIndex, protectionType));
    return angle::Result::Continue;
}

angle::Result VulkanSecondaryCommandBuffer::InitializeRenderPassInheritanceInfo(
    ContextVk *contextVk,
    const Framebuffer &framebuffer,
    const RenderPassDesc &renderPassDesc,
    VkCommandBufferInheritanceInfo *inheritanceInfoOut,
    VkCommandBufferInheritanceRenderingInfo *renderingInfoOut,
    gl::DrawBuffersArray<VkFormat> *colorFormatStorageOut)
{
    *inheritanceInfoOut       = {};
    inheritanceInfoOut->sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;

    if (contextVk->getFeatures().preferDynamicRendering.enabled)
    {
        renderPassDesc.populateRenderingInheritanceInfo(contextVk->getRenderer(), renderingInfoOut,
                                                        colorFormatStorageOut);
        AddToPNextChain(inheritanceInfoOut, renderingInfoOut);
    }
    else
    {
        const RenderPass *compatibleRenderPass = nullptr;
        ANGLE_TRY(contextVk->getCompatibleRenderPass(renderPassDesc, &compatibleRenderPass));
        inheritanceInfoOut->renderPass  = compatibleRenderPass->getHandle();
        inheritanceInfoOut->subpass     = 0;
        inheritanceInfoOut->framebuffer = framebuffer.getHandle();
    }

    return angle::Result::Continue;
}

angle::Result VulkanSecondaryCommandBuffer::initialize(ErrorContext *context,
                                                       SecondaryCommandPool *pool,
                                                       bool isRenderPassCommandBuffer,
                                                       SecondaryCommandMemoryAllocator *allocator)
{
    mCommandPool = pool;
    mCommandTracker.reset();
    mAnyCommand = false;

    ANGLE_TRY(pool->allocate(context, this));

    // Outside-RP command buffers are begun automatically here.  RP command buffers are begun when
    // the render pass itself starts, as they require inheritance info.
    if (!isRenderPassCommandBuffer)
    {
        VkCommandBufferInheritanceInfo inheritanceInfo = {};
        inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
        ANGLE_TRY(begin(context, inheritanceInfo));
    }

    return angle::Result::Continue;
}

void VulkanSecondaryCommandBuffer::destroy()
{
    if (valid())
    {
        ASSERT(mCommandPool != nullptr);
        mCommandPool->collect(this);
    }
}

angle::Result VulkanSecondaryCommandBuffer::begin(
    ErrorContext *context,
    const VkCommandBufferInheritanceInfo &inheritanceInfo)
{
    ASSERT(!mAnyCommand);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo         = &inheritanceInfo;
    // For render pass command buffers, specify that the command buffer is entirely inside a render
    // pass.  This is determined by either the render pass object existing, or the
    // VkCommandBufferInheritanceRenderingInfo specified in the pNext chain.
    if (inheritanceInfo.renderPass != VK_NULL_HANDLE || inheritanceInfo.pNext != nullptr)
    {
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    }

    ANGLE_VK_TRY(context, CommandBuffer::begin(beginInfo));
    return angle::Result::Continue;
}

angle::Result VulkanSecondaryCommandBuffer::end(ErrorContext *context)
{
    ANGLE_VK_TRY(context, CommandBuffer::end());
    return angle::Result::Continue;
}

VkResult VulkanSecondaryCommandBuffer::reset()
{
    mCommandTracker.reset();
    mAnyCommand = false;
    return CommandBuffer::reset();
}
}  // namespace vk
}  // namespace rx
