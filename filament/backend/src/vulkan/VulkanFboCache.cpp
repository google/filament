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
    if (k1.flags.value != k2.flags.value) return false;
    if (k1.depthLayout != k2.depthLayout) return false;
    if (k1.depthFormat != k2.depthFormat) return false;
    for (int i = 0; i < MRT::TARGET_COUNT; i++) {
        if (k1.colorLayout[i] != k2.colorLayout[i]) return false;
        if (k1.colorFormat[i] != k2.colorFormat[i]) return false;
    }
    return true;
}

bool VulkanFboCache::FboKeyEqualFn::operator()(const FboKey& k1, const FboKey& k2) const {
    if (k1.renderPass != k2.renderPass) return false;
    if (k1.depth != k2.depth) return false;
    for (int i = 0; i < MRT::TARGET_COUNT; i++) {
        if (k1.color[i] != k2.color[i]) return false;
    }
    return true;
}

VulkanFboCache::VulkanFboCache(VulkanContext& context) : mContext(context) {}

VulkanFboCache::~VulkanFboCache() {
    ASSERT_POSTCONDITION(mFramebufferCache.empty() && mRenderPassCache.empty(),
            "Please explicitly call reset() while the VkDevice is still alive.");
}

VkFramebuffer VulkanFboCache::getFramebuffer(FboKey config, uint32_t width,
        uint32_t height, uint32_t layers) noexcept {
    auto iter = mFramebufferCache.find(config);
    if (UTILS_LIKELY(iter != mFramebufferCache.end() && iter->second.handle != VK_NULL_HANDLE)) {
        iter.value().timestamp = mCurrentTime;
        return iter->second.handle;
    }
    VkImageView attachments[MRT::TARGET_COUNT + 1];
    uint32_t nAttachments = 0;
    for (VkImageView attachment : config.color) {
        if (attachment) {
            attachments[nAttachments++] = attachment;
        }
    }
    if (config.depth) {
        attachments[nAttachments++] = config.depth;
    }
    VkFramebufferCreateInfo info {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = config.renderPass,
        .attachmentCount = nAttachments,
        .pAttachments = attachments,
        .width = width,
        .height = height,
        .layers = layers,
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
    const bool isSwapChain = config.colorLayout[0] == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // Set up some const aliases for terseness.
    const VkAttachmentLoadOp kClear = VK_ATTACHMENT_LOAD_OP_CLEAR;
    const VkAttachmentLoadOp kDontCare = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    const VkAttachmentLoadOp kKeep = VK_ATTACHMENT_LOAD_OP_LOAD;

    // In Vulkan, the subpass desc specifies the layout to transition to at the start of the render
    // pass, and the attachment description specifies the layout to transition to at the end.
    // However we use render passes to cause layout transitions only when drawing directly into the
    // swap chain. We keep our offscreen images in GENERAL layout, which is simple and prevents
    // thrashing the layout. Note that pipeline barriers are more powerful than render passes for
    // performing layout transitions, because they allow for per-miplevel transitions.
    const bool discard = any(config.flags.discardStart & TargetBufferFlags::COLOR);
    struct { VkImageLayout subpass, initial, final; } colorLayouts[MRT::TARGET_COUNT];
    if (isSwapChain) {
        colorLayouts[0].subpass = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorLayouts[0].initial = discard ? VK_IMAGE_LAYOUT_UNDEFINED : colorLayouts[0].subpass;
        colorLayouts[0].final = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    } else {
        for (int i = 0; i < MRT::TARGET_COUNT; i++) {
            colorLayouts[i].subpass = config.colorLayout[i];
            colorLayouts[i].initial = config.colorLayout[i];
            colorLayouts[i].final = config.colorLayout[i];
        }
    }

    VkAttachmentReference colorAttachmentRef[MRT::TARGET_COUNT] = {};
    VkAttachmentReference depthAttachmentRef = {};

    const bool hasDepth = config.depthFormat != VK_FORMAT_UNDEFINED;

    VkSubpassDescription subpass {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 0u,
        .pColorAttachments = colorAttachmentRef,
        .pDepthStencilAttachment = hasDepth ? &depthAttachmentRef : nullptr
    };

    VkAttachmentDescription attachments[MRT::TARGET_COUNT + 1] = {};

    VkRenderPassCreateInfo renderPassInfo {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 0u,
        .pAttachments = attachments,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 0u
    };

    int numAttachments = 0;
    for (int i = 0; i < MRT::TARGET_COUNT; i++) {
        if (config.colorFormat[i] == VK_FORMAT_UNDEFINED) {
            continue;
        }
        TargetBufferFlags flag = TargetBufferFlags(int(TargetBufferFlags::COLOR0) << i);
        bool clear = any(config.flags.clear & flag);
        bool discard = any(config.flags.discardStart & flag);
        colorAttachmentRef[numAttachments].layout = colorLayouts[i].subpass;
        colorAttachmentRef[numAttachments].attachment = numAttachments;
        attachments[numAttachments] = {
            .format = config.colorFormat[i],
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = clear ? kClear : (discard ? kDontCare : kKeep),
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = colorLayouts[i].initial,
            .finalLayout = colorLayouts[i].final
        };
        ++numAttachments;
    }
    subpass.colorAttachmentCount = numAttachments;

    if (hasDepth) {
        bool clear = any(config.flags.clear & TargetBufferFlags::DEPTH);
        bool discard = any(config.flags.discardStart & TargetBufferFlags::DEPTH);
        depthAttachmentRef.layout = config.depthLayout;
        depthAttachmentRef.attachment = numAttachments;
        attachments[numAttachments] = {
            .format = config.depthFormat,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = clear ? kClear : (discard ? kDontCare : kKeep),
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = config.depthLayout,
            .finalLayout = config.depthLayout
        };
        ++numAttachments;
    }
    renderPassInfo.attachmentCount = numAttachments;

    // Finally, create the VkRenderPass.
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
