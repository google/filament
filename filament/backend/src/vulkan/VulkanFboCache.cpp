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

#define FILAMENT_VULKAN_VERBOSE 0

namespace filament {
namespace backend {

bool VulkanFboCache::RenderPassEq::operator()(const RenderPassKey& k1,
        const RenderPassKey& k2) const {
    if (k1.clear != k2.clear) return false;
    if (k1.discardStart != k2.discardStart) return false;
    if (k1.discardEnd != k2.discardEnd) return false;
    if (k1.samples != k2.samples) return false;
    if (k1.needsResolveMask != k2.needsResolveMask) return false;
    if (k1.subpassMask != k2.subpassMask) return false;
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
    if (k1.width != k2.width) return false;
    if (k1.height != k2.height) return false;
    if (k1.layers != k2.layers) return false;
    if (k1.samples != k2.samples) return false;
    if (k1.depth != k2.depth) return false;
    for (int i = 0; i < MRT::TARGET_COUNT; i++) {
        if (k1.color[i] != k2.color[i]) return false;
        if (k1.resolve[i] != k2.resolve[i]) return false;
    }
    return true;
}

VulkanFboCache::VulkanFboCache(VulkanContext& context) : mContext(context) {}

VulkanFboCache::~VulkanFboCache() {
    ASSERT_POSTCONDITION(mFramebufferCache.empty() && mRenderPassCache.empty(),
            "Please explicitly call reset() while the VkDevice is still alive.");
}

VkFramebuffer VulkanFboCache::getFramebuffer(FboKey config) noexcept {
    auto iter = mFramebufferCache.find(config);
    if (UTILS_LIKELY(iter != mFramebufferCache.end() && iter->second.handle != VK_NULL_HANDLE)) {
        iter.value().timestamp = mCurrentTime;
        return iter->second.handle;
    }

    // The attachment list contains: Color Attachments, Resolve Attachments, and Depth Attachment.
    // For simplicity, create an array that can hold the maximum possible number of attachments.
    // Note that this needs to have the same ordering as the corollary array in getRenderPass.
    VkImageView attachments[MRT::TARGET_COUNT + MRT::TARGET_COUNT + 1];
    uint32_t attachmentCount = 0;
    for (VkImageView attachment : config.color) {
        if (attachment) {
            attachments[attachmentCount++] = attachment;
        }
    }
    for (VkImageView attachment : config.resolve) {
        if (attachment) {
            attachments[attachmentCount++] = attachment;
        }
    }
    if (config.depth) {
        attachments[attachmentCount++] = config.depth;
    }

    #if FILAMENT_VULKAN_VERBOSE
    utils::slog.d << "Creating framebuffer " << config.width << "x" << config.height << " "
        << "for render pass " << config.renderPass << ", "
        << "samples = " << int(config.samples) << ", "
        << "depth = " << (config.depth ? 1 : 0) << ", "
        << "attachmentCount = " << attachmentCount
        << utils::io::endl;
    #endif

    VkFramebufferCreateInfo info {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = config.renderPass,
        .attachmentCount = attachmentCount,
        .pAttachments = attachments,
        .width = config.width,
        .height = config.height,
        .layers = config.layers,
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
    const VkAttachmentStoreOp kDisableStore = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    const VkAttachmentStoreOp kEnableStore = VK_ATTACHMENT_STORE_OP_STORE;

    // In Vulkan, the subpass desc specifies the layout to transition to at the start of the render
    // pass, and the attachment description specifies the layout to transition to at the end.
    // However we use render passes to cause layout transitions only when drawing directly into the
    // swap chain. We keep our offscreen images in GENERAL layout, which is simple and prevents
    // thrashing the layout. Note that pipeline barriers are more powerful than render passes for
    // performing layout transitions, because they allow for per-miplevel transitions.
    const bool discard = any(config.discardStart & TargetBufferFlags::COLOR);
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

    VkAttachmentReference inputAttachmentRef[MRT::TARGET_COUNT] = {};
    VkAttachmentReference colorAttachmentRef[MRT::TARGET_COUNT] = {};
    VkAttachmentReference resolveAttachmentRef[MRT::TARGET_COUNT] = {};
    VkAttachmentReference depthAttachmentRef = {};

    const bool hasDepth = config.depthFormat != VK_FORMAT_UNDEFINED;

    VkSubpassDescription subpasses[2] = {{
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .pColorAttachments = colorAttachmentRef,
        .pResolveAttachments = resolveAttachmentRef,
        .pDepthStencilAttachment = hasDepth ? &depthAttachmentRef : nullptr
    },
    {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .pInputAttachments = inputAttachmentRef,
        .pColorAttachments = colorAttachmentRef,
        .pResolveAttachments = resolveAttachmentRef,
        .pDepthStencilAttachment = hasDepth ? &depthAttachmentRef : nullptr
    }};

    // The attachment list contains: Color Attachments, Resolve Attachments, and Depth Attachment.
    // For simplicity, create an array that can hold the maximum possible number of attachments.
    // Note that this needs to have the same ordering as the corollary array in getFramebuffer.
    VkAttachmentDescription attachments[MRT::TARGET_COUNT + MRT::TARGET_COUNT + 1] = {};

    // Determine the number of color attachments based on whether the format has been initialized.
    int colorAttachmentCount = 0;
    for (VkFormat format : config.colorFormat) {
        if (format != VK_FORMAT_UNDEFINED) {
            ++colorAttachmentCount;
        }
    }
    subpasses[0].colorAttachmentCount = colorAttachmentCount;
    subpasses[1].colorAttachmentCount = colorAttachmentCount;

    // Nulling out the zero-sized lists is necessary to avoid VK_ERROR_OUT_OF_HOST_MEMORY on Adreno.
    if (colorAttachmentCount == 0) {
        subpasses[0].pColorAttachments = nullptr;
        subpasses[0].pResolveAttachments = nullptr;
        subpasses[1].pColorAttachments = nullptr;
        subpasses[1].pResolveAttachments = nullptr;
    }

    // We support 2 subpasses, which means we need to supply 1 dependency struct.
    VkSubpassDependency dependencies[1] = {{
        .srcSubpass = 0,
        .dstSubpass = 1,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
    }};

    VkRenderPassCreateInfo renderPassInfo {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 0u,
        .pAttachments = attachments,
        .subpassCount = config.subpassMask ? 2u : 1u,
        .pSubpasses = subpasses,
        .dependencyCount = config.subpassMask ? 1u : 0u,
        .pDependencies = dependencies
    };

    int attachmentIndex = 0;

    // Populate the Color Attachments.
    VkAttachmentReference* pColorAttachment = colorAttachmentRef;
    for (int i = 0; i < MRT::TARGET_COUNT; i++) {
        if (config.colorFormat[i] == VK_FORMAT_UNDEFINED) {
            continue;
        }
        TargetBufferFlags flag = TargetBufferFlags(int(TargetBufferFlags::COLOR0) << i);
        bool clear = any(config.clear & flag);
        bool discard = any(config.discardStart & flag);
        if (config.subpassMask & (1 << i)) {
            int subpassInputIndex = subpasses[1].inputAttachmentCount++;
            inputAttachmentRef[subpassInputIndex].layout = colorLayouts[i].subpass;
            inputAttachmentRef[subpassInputIndex].attachment = attachmentIndex;
        }

        pColorAttachment->layout = colorLayouts[i].subpass;
        pColorAttachment->attachment = attachmentIndex;
        ++pColorAttachment;

        attachments[attachmentIndex++] = {
            .format = config.colorFormat[i],
            .samples = (VkSampleCountFlagBits) config.samples,
            .loadOp = clear ? kClear : (discard ? kDontCare : kKeep),
            .storeOp = config.samples == 1 ? kEnableStore : kDisableStore,
            .stencilLoadOp = kDontCare,
            .stencilStoreOp = kDisableStore,
            .initialLayout = colorLayouts[i].initial,
            .finalLayout = colorLayouts[i].final
        };
    }

    // Populate the Resolve Attachments.
    VkAttachmentReference* pResolveAttachment = resolveAttachmentRef;
    for (int i = 0; i < MRT::TARGET_COUNT; i++) {
        if (config.colorFormat[i] == VK_FORMAT_UNDEFINED) {
            continue;
        }

        if (!(config.needsResolveMask & (1 << i))) {
            pResolveAttachment->attachment = VK_ATTACHMENT_UNUSED;
            ++pResolveAttachment;
            continue;
        }

        pResolveAttachment->attachment = attachmentIndex;
        pResolveAttachment->layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        ++pResolveAttachment;

        attachments[attachmentIndex++] = {
            .format = config.colorFormat[i],
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = kDontCare,
            .storeOp = kEnableStore,
            .stencilLoadOp = kDontCare,
            .stencilStoreOp = kDisableStore,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = colorLayouts[i].final
        };
    }

    // Populate the Depth Attachment.
    if (hasDepth) {
        bool clear = any(config.clear & TargetBufferFlags::DEPTH);
        bool discard = any(config.discardStart & TargetBufferFlags::DEPTH);
        depthAttachmentRef.layout = config.depthLayout;
        depthAttachmentRef.attachment = attachmentIndex;
        attachments[attachmentIndex++] = {
            .format = config.depthFormat,
            .samples = (VkSampleCountFlagBits) config.samples,
            .loadOp = clear ? kClear : (discard ? kDontCare : kKeep),
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = config.depthLayout,
            .finalLayout = config.depthLayout
        };
    }
    renderPassInfo.attachmentCount = attachmentIndex;

    // Finally, create the VkRenderPass.
    VkRenderPass renderPass;
    VkResult error = vkCreateRenderPass(mContext.device, &renderPassInfo, VKALLOC, &renderPass);
    ASSERT_POSTCONDITION(!error, "Unable to create render pass.");
    mRenderPassCache[config] = {renderPass, mCurrentTime};

    #if FILAMENT_VULKAN_VERBOSE
    utils::slog.d << "Created render pass " << renderPass << " with "
        << "samples = " << int(config.samples) << ", "
        << "depth = " << (hasDepth ? 1 : 0) << ", "
        << "colorAttachmentCount = " << colorAttachmentCount
        << utils::io::endl;
    #endif

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
    // If this is one of the first few frames, return early to avoid wrapping unsigned integers.
    if (++mCurrentTime <= TIME_BEFORE_EVICTION) {
        return;
    }
    const uint32_t evictTime = mCurrentTime - TIME_BEFORE_EVICTION;

    for (auto iter = mFramebufferCache.begin(); iter != mFramebufferCache.end(); ++iter) {
        const FboVal fbo = iter->second;
        if (fbo.timestamp < evictTime && fbo.handle) {
            mRenderPassRefCount[iter->first.renderPass]--;
            vkDestroyFramebuffer(mContext.device, fbo.handle, VKALLOC);
            iter.value().handle = VK_NULL_HANDLE;
        }
    }
    for (auto iter = mRenderPassCache.begin(); iter != mRenderPassCache.end(); ++iter) {
        const VkRenderPass handle = iter->second.handle;
        if (iter->second.timestamp < evictTime && handle && mRenderPassRefCount[handle] == 0) {
            vkDestroyRenderPass(mContext.device, handle, VKALLOC);
            iter.value().handle = VK_NULL_HANDLE;
        }
    }
}

} // namespace filament
} // namespace backend
