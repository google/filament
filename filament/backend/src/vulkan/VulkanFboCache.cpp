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

#include "vulkan/VulkanFboCache.h"

#include <utils/Panic.h>

namespace filament {
namespace backend {

bool VulkanFboCache::RenderPassEq::operator()(const RenderPassKey& k1,
        const RenderPassKey& k2) const {
    return
            k1.colorLayout == k2.colorLayout &&
            k1.depthLayout == k2.depthLayout &&
            k1.colorFormat == k2.colorFormat &&
            k1.depthFormat == k2.depthFormat &&
            k1.flags.value == k2.flags.value;
}

bool VulkanFboCache::FboKeyEqualFn::operator()(const FboKey& k1, const FboKey& k2) const {
    static_assert(sizeof(FboKey::attachments) == 3 * sizeof(VkImageView), "Unexpected count.");
    return
            k1.renderPass == k2.renderPass &&
            k1.attachments[0] == k2.attachments[0] &&
            k1.attachments[1] == k2.attachments[1] &&
            k1.attachments[2] == k2.attachments[2];
}

VulkanFboCache::VulkanFboCache(VulkanContext& context) : mContext(context) {}

VulkanFboCache::~VulkanFboCache() {
    ASSERT_POSTCONDITION(mFramebufferCache.empty() && mRenderPassCache.empty(),
            "Please explicitly call reset() while the VkDevice is still alive.");
}

VkFramebuffer VulkanFboCache::getFramebuffer(FboKey config, uint32_t w, uint32_t h) noexcept {
    auto iter = mFramebufferCache.find(config);
    if (UTILS_LIKELY(iter != mFramebufferCache.end() && iter->second.handle != VK_NULL_HANDLE)) {
        iter.value().timestamp = mCurrentTime;
        return iter->second.handle;
    }
    uint32_t nAttachments = 0;
    for (auto attachment : config.attachments) {
        if (attachment) {
            nAttachments++;
        }
    }
    VkFramebufferCreateInfo info {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = config.renderPass,
        .attachmentCount = nAttachments,
        .pAttachments = config.attachments,
        .width = w,
        .height = h,
        .layers = 1
    };
    mRenderPassRefCount[info.renderPass]++;
    VkFramebuffer framebuffer;
    VkResult error = vkCreateFramebuffer(mContext.device, &info, VKALLOC, &framebuffer);
    ASSERT_POSTCONDITION(!error, "Unable to create framebuffer.");
    mFramebufferCache[config] = {framebuffer, mCurrentTime};
    return framebuffer;
}

VkRenderPass VulkanFboCache::getRenderPass(RenderPassKey config) noexcept {
    auto iter = mRenderPassCache.find(config);
    if (UTILS_LIKELY(iter != mRenderPassCache.end() && iter->second.handle != VK_NULL_HANDLE)) {
        iter.value().timestamp = mCurrentTime;
        return iter->second.handle;
    }
    const bool hasColor = config.colorFormat != VK_FORMAT_UNDEFINED;
    const bool hasDepth = config.depthFormat != VK_FORMAT_UNDEFINED;
    const bool isSwapChain = config.colorLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // In Vulkan, the subpass desc specifies the layout to transition to at the start of the render
    // pass, and the attachment description specifies the layout to transition to at the end.
    // However we use render passes to cause layout transitions only when drawing directly into the
    // swap chain. We keep our offscreen images in GENERAL layout, which is simple and prevents
    // thrashing the layout. Note that pipeline barriers are more powerful than render passes for
    // performing layout transitions, because they allow for per-miplevel transitions.

    uint32_t numAttachments = 0;
    VkAttachmentReference colorAttachmentRef = {};
    if (hasColor) {
        colorAttachmentRef.layout = isSwapChain ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : config.colorLayout;
        colorAttachmentRef.attachment = numAttachments++;
    }
    VkAttachmentReference depthAttachmentRef = {};
    if (hasDepth) {
        depthAttachmentRef.layout = config.depthLayout;
        depthAttachmentRef.attachment = numAttachments++;
    }
    VkSubpassDescription subpass {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = hasColor ? 1u : 0u,
        .pColorAttachments = hasColor ? &colorAttachmentRef : nullptr,
        .pDepthStencilAttachment = hasDepth ? &depthAttachmentRef : nullptr
    };

    // Set up some const aliases for terseness.
    const VkAttachmentLoadOp clear = VK_ATTACHMENT_LOAD_OP_CLEAR;
    const VkAttachmentLoadOp dontCare = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

    // TODO: For now we do not support LOAD_OP_LOAD for the swap chain due to the following
    // complaint from validation:
    //
    //   Render pass has an attachment with loadOp == VK_ATTACHMENT_LOAD_OP_LOAD and
    //   initialLayout == VK_IMAGE_LAYOUT_UNDEFINED.  This is probably not what you intended.
    //
    const VkAttachmentLoadOp keep = isSwapChain ? VK_ATTACHMENT_LOAD_OP_DONT_CARE :
            VK_ATTACHMENT_LOAD_OP_LOAD;

    const bool clearColor = any(config.flags.clear & TargetBufferFlags::COLOR);
    const bool discardColor = any(config.flags.discardStart & TargetBufferFlags::COLOR);
    const VkAttachmentLoadOp colorLoadOp = clearColor ? clear : (discardColor ? dontCare : keep);

    const bool clearDepth = any(config.flags.clear & TargetBufferFlags::DEPTH);
    const bool discardDepth = any(config.flags.discardStart & TargetBufferFlags::DEPTH);
    const VkAttachmentLoadOp depthLoadOp = clearDepth ? clear : (discardDepth ? dontCare : keep);

    VkAttachmentDescription colorAttachment {
        .format = config.colorFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = colorLoadOp,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = isSwapChain ? VK_IMAGE_LAYOUT_UNDEFINED : config.colorLayout,
        .finalLayout = isSwapChain ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : config.colorLayout
    };
    VkAttachmentDescription depthAttachment {
        .format = config.depthFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = depthLoadOp,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = config.depthLayout,
        .finalLayout = config.depthLayout
    };

    // Finally, create the VkRenderPass.
    VkAttachmentDescription attachments[2];
    VkRenderPassCreateInfo renderPassInfo {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 0u,
        .pAttachments = attachments,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 0u
    };
    if (hasColor) {
        attachments[renderPassInfo.attachmentCount++] = colorAttachment;
    }
    if (hasDepth) {
        attachments[renderPassInfo.attachmentCount++] = depthAttachment;
    }
    VkRenderPass renderPass;
    VkResult error = vkCreateRenderPass(mContext.device, &renderPassInfo, VKALLOC, &renderPass);
    ASSERT_POSTCONDITION(!error, "Unable to create render pass.");
    mRenderPassCache[config] = {renderPass, mCurrentTime};
    return renderPass;
}

void VulkanFboCache::reset() noexcept {
    for (auto pair : mFramebufferCache) {
        mRenderPassRefCount[pair.first.renderPass]--;
        vkDestroyFramebuffer(mContext.device, pair.second.handle, VKALLOC);
    }
    mFramebufferCache.clear();
    for (auto pair : mRenderPassCache) {
        vkDestroyRenderPass(mContext.device, pair.second.handle, VKALLOC);
    }
    mRenderPassCache.clear();
}

// Frees up old framebuffers and render passes, then nulls out their key.  Doesn't bother removing
// the actual map entry since it is fairly small.
void VulkanFboCache::gc() noexcept {
    mCurrentTime++;
    const uint32_t evictTime = mCurrentTime - TIME_BEFORE_EVICTION;
    for (auto iter = mFramebufferCache.begin(); iter != mFramebufferCache.end(); ++iter) {
        if (iter->second.timestamp < evictTime) {
            vkDestroyFramebuffer(mContext.device, iter->second.handle, VKALLOC);
            iter.value().handle = VK_NULL_HANDLE;
        }
    }
    for (auto iter = mRenderPassCache.begin(); iter != mRenderPassCache.end(); ++iter) {
        VkRenderPass handle = iter->second.handle;
        if (iter->second.timestamp < evictTime && mRenderPassRefCount[handle] == 0) {
            vkDestroyRenderPass(mContext.device, handle, VKALLOC);
            iter.value().handle = VK_NULL_HANDLE;
        }
    }
}

} // namespace filament
} // namespace backend
