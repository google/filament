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

#include "VulkanConstants.h"
#include "VulkanUtility.h"

// If any VkRenderPass or VkFramebuffer is unused for more than TIME_BEFORE_EVICTION frames, it
// is evicted from the cache.
static constexpr uint32_t TIME_BEFORE_EVICTION = FVK_MAX_COMMAND_BUFFERS;

using namespace bluevk;

namespace filament::backend {

bool VulkanFboCache::RenderPassEq::operator()(const RenderPassKey& k1,
        const RenderPassKey& k2) const {
    if (k1.initialColorLayoutMask != k2.initialColorLayoutMask) return false;
    if (k1.initialDepthLayout != k2.initialDepthLayout) return false;
    for (int i = 0; i < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT; i++) {
        if (k1.colorFormat[i] != k2.colorFormat[i]) return false;
    }
    if (k1.depthFormat != k2.depthFormat) return false;
    if (k1.clear != k2.clear) return false;
    if (k1.discardStart != k2.discardStart) return false;
    if (k1.discardEnd != k2.discardEnd) return false;
    if (k1.samples != k2.samples) return false;
    if (k1.needsResolveMask != k2.needsResolveMask) return false;
    if (k1.subpassMask != k2.subpassMask) return false;
    if (k1.viewCount != k2.viewCount) return false;
    return true;
}

bool VulkanFboCache::FboKeyEqualFn::operator()(const FboKey& k1, const FboKey& k2) const {
    if (k1.renderPass != k2.renderPass) return false;
    if (k1.width != k2.width) return false;
    if (k1.height != k2.height) return false;
    if (k1.layers != k2.layers) return false;
    if (k1.samples != k2.samples) return false;
    if (k1.depth != k2.depth) return false;
    for (int i = 0; i < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT; i++) {
        if (k1.color[i] != k2.color[i]) return false;
        if (k1.resolve[i] != k2.resolve[i]) return false;
    }
    return true;
}

VulkanFboCache::VulkanFboCache(VkDevice device)
    : mDevice(device) {}

VulkanFboCache::~VulkanFboCache() {
    FILAMENT_CHECK_POSTCONDITION(mFramebufferCache.empty() && mRenderPassCache.empty())
            << "Please explicitly call terminate() while the VkDevice is still alive.";
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
    VkImageView attachments[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT + MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT + 1];
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

    #if FVK_ENABLED(FVK_DEBUG_FBO_CACHE)
    FVK_LOGD << "Creating framebuffer " << config.width << "x" << config.height << " "
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
    VkResult error = vkCreateFramebuffer(mDevice, &info, VKALLOC, &framebuffer);
    FILAMENT_CHECK_POSTCONDITION(!error) << "Unable to create framebuffer.";
    mFramebufferCache[config] = {framebuffer, mCurrentTime};
    return framebuffer;
}

VkRenderPass VulkanFboCache::getRenderPass(RenderPassKey config) noexcept {
    auto iter = mRenderPassCache.find(config);
    if (UTILS_LIKELY(iter != mRenderPassCache.end() && iter->second.handle != VK_NULL_HANDLE)) {
        iter.value().timestamp = mCurrentTime;
        return iter->second.handle;
    }
    const bool hasSubpasses = config.subpassMask != 0;

    // Set up some const aliases for terseness.
    const VkAttachmentLoadOp kClear = VK_ATTACHMENT_LOAD_OP_CLEAR;
    const VkAttachmentLoadOp kDontCare = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    const VkAttachmentLoadOp kKeep = VK_ATTACHMENT_LOAD_OP_LOAD;
    const VkAttachmentStoreOp kDisableStore = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    const VkAttachmentStoreOp kEnableStore = VK_ATTACHMENT_STORE_OP_STORE;

    // In Vulkan, the subpass desc specifies the layout to transition to at the start of the render
    // pass, and the attachment description specifies the layout to transition to at the end.

    VkAttachmentReference inputAttachmentRef[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT] = {};
    VkAttachmentReference colorAttachmentRefs[2][MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT] = {};
    VkAttachmentReference resolveAttachmentRef[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT] = {};
    VkAttachmentReference depthAttachmentRef = {};

    const bool hasDepth = config.depthFormat != VK_FORMAT_UNDEFINED;

    VkSubpassDescription subpasses[2] = {{
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .pInputAttachments = nullptr,
        .pColorAttachments = colorAttachmentRefs[0],
        .pResolveAttachments = resolveAttachmentRef,
        .pDepthStencilAttachment = hasDepth ? &depthAttachmentRef : nullptr
    },
    {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .pInputAttachments = inputAttachmentRef,
        .pColorAttachments = colorAttachmentRefs[1],
        .pResolveAttachments = resolveAttachmentRef,
        .pDepthStencilAttachment = hasDepth ? &depthAttachmentRef : nullptr
    }};

    // The attachment list contains: Color Attachments, Resolve Attachments, and Depth Attachment.
    // For simplicity, create an array that can hold the maximum possible number of attachments.
    // Note that this needs to have the same ordering as the corollary array in getFramebuffer.
    VkAttachmentDescription attachments[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT + MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT + 1] = {};

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
        .subpassCount = hasSubpasses ? 2u : 1u,
        .pSubpasses = subpasses,
        .dependencyCount = hasSubpasses ? 1u : 0u,
        .pDependencies = dependencies,
    };

    VkRenderPassMultiviewCreateInfo multiviewCreateInfo = {};
    uint32_t subpassViewMask = (1 << config.viewCount) - 1;
    // Prepare a view mask array for the maximum number of subpasses. All subpasses have all views
    // activated.
    uint32_t viewMasks[2] = {subpassViewMask, subpassViewMask};
    if (config.viewCount > 1) {
      // Fill the multiview create info.
      multiviewCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO;
      multiviewCreateInfo.pNext = nullptr;
      multiviewCreateInfo.subpassCount = hasSubpasses ? 2u : 1u;
      multiviewCreateInfo.pViewMasks = viewMasks;
      multiviewCreateInfo.dependencyCount = 0;
      multiviewCreateInfo.pViewOffsets = nullptr;
      multiviewCreateInfo.correlationMaskCount = 1;
      multiviewCreateInfo.pCorrelationMasks = &subpassViewMask;

      renderPassInfo.pNext = &multiviewCreateInfo;
    }

    int attachmentIndex = 0;

    // Populate the Color Attachments.
    for (int i = 0; i < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT; i++) {
        if (config.colorFormat[i] == VK_FORMAT_UNDEFINED) {
            continue;
        }
        const VkImageLayout subpassLayout = imgutil::getVkLayout(VulkanLayout::COLOR_ATTACHMENT);
        uint32_t index;

        if (!hasSubpasses) {
            index = subpasses[0].colorAttachmentCount++;
            colorAttachmentRefs[0][index].layout = subpassLayout;
            colorAttachmentRefs[0][index].attachment = attachmentIndex;
        } else {

            // The Driver API consolidates all color attachments from the first and second subpasses
            // into a single list, and uses a bitmask to mark attachments that belong only to the
            // second subpass and should be available as inputs. All color attachments in the first
            // subpass are automatically made available to the second subpass.

            // If there are subpasses, we require the input attachment to be the first attachment.
            // Breaking this assumption would likely require enhancements to the Driver API in order
            // to supply Vulkan with all the information needed.
            assert_invariant(config.subpassMask == 1);

            if (config.subpassMask & (1 << i)) {
                index = subpasses[0].colorAttachmentCount++;
                colorAttachmentRefs[0][index].layout = subpassLayout;
                colorAttachmentRefs[0][index].attachment = attachmentIndex;

                index = subpasses[1].inputAttachmentCount++;
                inputAttachmentRef[index].layout = subpassLayout;
                inputAttachmentRef[index].attachment = attachmentIndex;
            }

            index = subpasses[1].colorAttachmentCount++;
            colorAttachmentRefs[1][index].layout = subpassLayout;
            colorAttachmentRefs[1][index].attachment = attachmentIndex;
        }

        const TargetBufferFlags flag = TargetBufferFlags(int(TargetBufferFlags::COLOR0) << i);
        const bool clear = any(config.clear & flag);
        const bool discard = any(config.discardStart & flag);

        attachments[attachmentIndex++] = {
            .format = config.colorFormat[i],
            .samples = (VkSampleCountFlagBits) config.samples,
            .loadOp = clear ? kClear : (discard ? kDontCare : kKeep),
            .storeOp = kEnableStore,
            .stencilLoadOp = kDontCare,
            .stencilStoreOp = kDisableStore,
            .initialLayout = ((!discard && config.initialColorLayoutMask & (1 << i)) || clear)
                                     ? imgutil::getVkLayout(VulkanLayout::COLOR_ATTACHMENT)
                                     : imgutil::getVkLayout(VulkanLayout::UNDEFINED),
            .finalLayout = imgutil::getVkLayout(FINAL_COLOR_ATTACHMENT_LAYOUT),
        };
    }

    // Nulling out the zero-sized lists is necessary to avoid VK_ERROR_OUT_OF_HOST_MEMORY on Adreno.
    if (subpasses[0].colorAttachmentCount == 0) {
        subpasses[0].pColorAttachments = nullptr;
        subpasses[0].pResolveAttachments = nullptr;
        subpasses[1].pColorAttachments = nullptr;
        subpasses[1].pResolveAttachments = nullptr;
    }

    // Populate the Resolve Attachments.
    VkAttachmentReference* pResolveAttachment = resolveAttachmentRef;
    for (int i = 0; i < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT; i++) {
        if (config.colorFormat[i] == VK_FORMAT_UNDEFINED) {
            continue;
        }

        if (!(config.needsResolveMask & (1 << i))) {
            pResolveAttachment->attachment = VK_ATTACHMENT_UNUSED;
            ++pResolveAttachment;
            continue;
        }

        pResolveAttachment->attachment = attachmentIndex;
        pResolveAttachment->layout
                = imgutil::getVkLayout(VulkanLayout::COLOR_ATTACHMENT_RESOLVE);
        ++pResolveAttachment;

        attachments[attachmentIndex++] = {
            .format = config.colorFormat[i],
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = kDontCare,
            .storeOp = kEnableStore,
            .stencilLoadOp = kDontCare,
            .stencilStoreOp = kDisableStore,
            .initialLayout = imgutil::getVkLayout(VulkanLayout::COLOR_ATTACHMENT),
            .finalLayout = imgutil::getVkLayout(FINAL_COLOR_ATTACHMENT_LAYOUT),
        };
    }

    // Populate the Depth Attachment.
    if (hasDepth) {
        const bool clear = any(config.clear & TargetBufferFlags::DEPTH);
        const bool discardStart = any(config.discardStart & TargetBufferFlags::DEPTH);
        const bool discardEnd = any(config.discardEnd & TargetBufferFlags::DEPTH);
        const VkAttachmentLoadOp loadOp = clear ? kClear : (discardStart ? kDontCare : kKeep);
        depthAttachmentRef.layout = imgutil::getVkLayout(VulkanLayout::DEPTH_ATTACHMENT);
        depthAttachmentRef.attachment = attachmentIndex;
        VkImageLayout initialLayout = imgutil::getVkLayout(config.initialDepthLayout);
        
        attachments[attachmentIndex++] = {
            .format = config.depthFormat,
            .samples = (VkSampleCountFlagBits) config.samples,
            .loadOp = loadOp,
            .storeOp = discardEnd ? kDisableStore : kEnableStore,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = initialLayout,
            .finalLayout = imgutil::getVkLayout(FINAL_DEPTH_ATTACHMENT_LAYOUT),
        };
    }
    renderPassInfo.attachmentCount = attachmentIndex;

    // Finally, create the VkRenderPass.
    VkRenderPass renderPass;
    VkResult error = vkCreateRenderPass(mDevice, &renderPassInfo, VKALLOC, &renderPass);
    FILAMENT_CHECK_POSTCONDITION(!error) << "Unable to create render pass.";
    mRenderPassCache[config] = {renderPass, mCurrentTime};

    #if FVK_ENABLED(FVK_DEBUG_FBO_CACHE)
    FVK_LOGD << "Created render pass " << renderPass << " with "
        << "samples = " << int(config.samples) << ", "
        << "depth = " << (hasDepth ? 1 : 0) << ", "
        << "colorAttachmentCount[0] = " << subpasses[0].colorAttachmentCount
        << utils::io::endl;
    #endif

    return renderPass;
}

void VulkanFboCache::reset() noexcept {
    for (auto pair : mFramebufferCache) {
        mRenderPassRefCount[pair.first.renderPass]--;
        vkDestroyFramebuffer(mDevice, pair.second.handle, VKALLOC);
    }
    mFramebufferCache.clear();
    for (auto pair : mRenderPassCache) {
        vkDestroyRenderPass(mDevice, pair.second.handle, VKALLOC);
    }
    mRenderPassCache.clear();
}

// Frees up old framebuffers and render passes, then nulls out their key.  Doesn't bother removing
// the actual map entry since it is fairly small.
void VulkanFboCache::gc() noexcept {
    FVK_SYSTRACE_CONTEXT();
    FVK_SYSTRACE_START("fbocache::gc");

    // If this is one of the first few frames, return early to avoid wrapping unsigned integers.
    if (++mCurrentTime <= TIME_BEFORE_EVICTION) {
        return;
    }
    const uint32_t evictTime = mCurrentTime - TIME_BEFORE_EVICTION;

    for (auto iter = mFramebufferCache.begin(); iter != mFramebufferCache.end(); ++iter) {
        const FboVal fbo = iter->second;
        if (fbo.timestamp < evictTime && fbo.handle) {
            mRenderPassRefCount[iter->first.renderPass]--;
            vkDestroyFramebuffer(mDevice, fbo.handle, VKALLOC);
            iter.value().handle = VK_NULL_HANDLE;
        }
    }
    for (auto iter = mRenderPassCache.begin(); iter != mRenderPassCache.end(); ++iter) {
        const VkRenderPass handle = iter->second.handle;
        if (iter->second.timestamp < evictTime && handle && mRenderPassRefCount[handle] == 0) {
            vkDestroyRenderPass(mDevice, handle, VKALLOC);
            iter.value().handle = VK_NULL_HANDLE;
        }
    }
    FVK_SYSTRACE_END();
}

} // namespace filament::backend
