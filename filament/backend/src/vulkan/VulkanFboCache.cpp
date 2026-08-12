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

#include "VulkanFboCache.h"

#include "VulkanConstants.h"
#include "VulkanHandles.h"

#include "vulkan/utils/Image.h"

#include <utils/compiler.h>
#include <utils/Panic.h>

// If any VkRenderPass or VkFramebuffer is unused for more than TIME_BEFORE_EVICTION frames, it
// is evicted from the cache.
static constexpr uint32_t TIME_BEFORE_EVICTION = FVK_MAX_COMMAND_BUFFERS;

using namespace bluevk;

namespace filament::backend {

namespace {

// A depth resolve can only be expressed through VK_KHR_create_renderpass2, so the render pass built
// with the Vulkan 1.0 structures is translated rather than duplicated. Everything is copied
// verbatim except multiview and the fragment density map, whose chains are rebuilt below.
VkResult createRenderPass2(VkDevice device, VkRenderPassCreateInfo const& info, uint32_t viewMask,
        VkAttachmentReference const& depthResolveRef,
    VkAttachmentReference const& fragmentDensityMapRef,
    VkResolveModeFlagBits depthResolveMode, VkRenderPass* outRenderPass) {
    constexpr size_t kMaxAttachments =
            MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT + MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT + 3;
    constexpr size_t kMaxSubpasses = 2;

    assert_invariant(info.attachmentCount <= kMaxAttachments);
    assert_invariant(info.subpassCount <= kMaxSubpasses);

    VkAttachmentDescription2 attachments[kMaxAttachments] = {};
    for (uint32_t i = 0; i < info.attachmentCount; i++) {
        VkAttachmentDescription const& src = info.pAttachments[i];
        attachments[i] = {
            .sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
            .flags = src.flags,
            .format = src.format,
            .samples = src.samples,
            .loadOp = src.loadOp,
            .storeOp = src.storeOp,
            .stencilLoadOp = src.stencilLoadOp,
            .stencilStoreOp = src.stencilStoreOp,
            .initialLayout = src.initialLayout,
            .finalLayout = src.finalLayout,
        };
    }

    auto const convertRef = [](VkAttachmentReference const& src, VkImageAspectFlags aspectMask) {
        return VkAttachmentReference2{
            .sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
            .attachment = src.attachment,
            .layout = src.layout,
            .aspectMask = aspectMask,
        };
    };

    VkAttachmentReference2 colorRefs[kMaxSubpasses][MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT] = {};
    VkAttachmentReference2 inputRefs[kMaxSubpasses][MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT] = {};
    VkAttachmentReference2 resolveRefs[kMaxSubpasses][MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT] = {};
    VkAttachmentReference2 depthRefs[kMaxSubpasses] = {};
    VkAttachmentReference2 depthResolveRefs[kMaxSubpasses] = {};
    VkSubpassDescriptionDepthStencilResolve depthResolves[kMaxSubpasses] = {};
    VkSubpassDescription2 subpasses[kMaxSubpasses] = {};

    for (uint32_t s = 0; s < info.subpassCount; s++) {
        VkSubpassDescription const& src = info.pSubpasses[s];
        for (uint32_t i = 0; i < src.colorAttachmentCount; i++) {
            colorRefs[s][i] = convertRef(src.pColorAttachments[i], 0);
            if (src.pResolveAttachments) {
                resolveRefs[s][i] = convertRef(src.pResolveAttachments[i], 0);
            }
        }
        for (uint32_t i = 0; i < src.inputAttachmentCount; i++) {
            inputRefs[s][i] = convertRef(src.pInputAttachments[i], VK_IMAGE_ASPECT_COLOR_BIT);
        }
        if (src.pDepthStencilAttachment) {
            depthRefs[s] = convertRef(*src.pDepthStencilAttachment, 0);
        }

        subpasses[s] = {
            .sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2,
            .flags = src.flags,
            .pipelineBindPoint = src.pipelineBindPoint,
            .viewMask = viewMask,
            .inputAttachmentCount = src.inputAttachmentCount,
            .pInputAttachments = src.inputAttachmentCount ? inputRefs[s] : nullptr,
            .colorAttachmentCount = src.colorAttachmentCount,
            .pColorAttachments = src.colorAttachmentCount ? colorRefs[s] : nullptr,
            .pResolveAttachments =
                    (src.colorAttachmentCount && src.pResolveAttachments) ? resolveRefs[s] : nullptr,
            .pDepthStencilAttachment = src.pDepthStencilAttachment ? &depthRefs[s] : nullptr,
        };

        // Resolve when depth is used for the last time. A multisampled input-only color subpass
        // deliberately omits depth, so its resolve belongs to the preceding scene subpass.
        bool const isLastDepthUse =
                src.pDepthStencilAttachment &&
                (s + 1 == info.subpassCount || !info.pSubpasses[s + 1].pDepthStencilAttachment);
        if (isLastDepthUse) {
            depthResolveRefs[s] = convertRef(depthResolveRef, 0);
            depthResolves[s] = {
                .sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE,
                .depthResolveMode =
                        fvkutils::isVkDepthFormat(attachments[depthResolveRef.attachment].format)
                        ? depthResolveMode
                                : VK_RESOLVE_MODE_NONE,
                .stencilResolveMode =
                        fvkutils::isVkStencilFormat(attachments[depthResolveRef.attachment].format)
                                ? VK_RESOLVE_MODE_SAMPLE_ZERO_BIT
                                : VK_RESOLVE_MODE_NONE,
                .pDepthStencilResolveAttachment = &depthResolveRefs[s],
            };
            subpasses[s].pNext = &depthResolves[s];
        }
    }

    VkSubpassDependency2 dependencies[1] = {};
    for (uint32_t i = 0; i < info.dependencyCount && i < 1; i++) {
        VkSubpassDependency const& src = info.pDependencies[i];
        dependencies[i] = {
            .sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
            .srcSubpass = src.srcSubpass,
            .dstSubpass = src.dstSubpass,
            .srcStageMask = src.srcStageMask,
            .dstStageMask = src.dstStageMask,
            .srcAccessMask = src.srcAccessMask,
            .dstAccessMask = src.dstAccessMask,
            .dependencyFlags = src.dependencyFlags,
        };
    }

    VkRenderPassFragmentDensityMapCreateInfoEXT const fragmentDensityMapInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_FRAGMENT_DENSITY_MAP_CREATE_INFO_EXT,
        .fragmentDensityMapAttachment = fragmentDensityMapRef,
    };
    VkRenderPassCreateInfo2 const info2 = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2,
        .pNext = fragmentDensityMapRef.attachment != VK_ATTACHMENT_UNUSED
                         ? &fragmentDensityMapInfo
                         : nullptr,
        .attachmentCount = info.attachmentCount,
        .pAttachments = attachments,
        .subpassCount = info.subpassCount,
        .pSubpasses = subpasses,
        .dependencyCount = info.dependencyCount,
        .pDependencies = info.dependencyCount ? dependencies : nullptr,
        .correlatedViewMaskCount = viewMask ? 1u : 0u,
        .pCorrelatedViewMasks = viewMask ? &viewMask : nullptr,
    };
    assert_invariant(vkCreateRenderPass2KHR || vkCreateRenderPass2);
    return vkCreateRenderPass2 ? vkCreateRenderPass2(device, &info2, VKALLOC, outRenderPass)
                               : vkCreateRenderPass2KHR(device, &info2, VKALLOC, outRenderPass);
}

} // anonymous namespace

bool VulkanFboCache::RenderPassEq::operator()(const RenderPassKey& k1,
        const RenderPassKey& k2) const {
    if (k1.initialDepthStencilLayout != k2.initialDepthStencilLayout) return false;
    for (int i = 0; i < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT; i++) {
        if (k1.colorFormat[i] != k2.colorFormat[i]) return false;
        if (k1.colorSamples[i] != k2.colorSamples[i]) return false;
    }
    if (k1.depthStencilFormat != k2.depthStencilFormat) return false;
    if (k1.fragmentDensityMapFormat != k2.fragmentDensityMapFormat) return false;
    if (k1.clear != k2.clear) return false;
    if (k1.discardStart != k2.discardStart) return false;
    if (k1.discardEnd != k2.discardEnd) return false;
    if (k1.samples != k2.samples) return false;
    if (k1.needsResolveMask != k2.needsResolveMask) return false;
    if (k1.usesLazilyAllocatedMemory != k2.usesLazilyAllocatedMemory) return false;
    if (k1.subpassMask != k2.subpassMask) return false;
    if (k1.viewCount != k2.viewCount) return false;
    if (k1.needsDepthResolve != k2.needsDepthResolve) return false;
    if (k1.multisampledSubpassInput != k2.multisampledSubpassInput) return false;
    return true;
}

bool VulkanFboCache::FboKeyEqualFn::operator()(const FboKey& k1, const FboKey& k2) const {
    if (k1.renderPass != k2.renderPass) return false;
    if (k1.width != k2.width) return false;
    if (k1.height != k2.height) return false;
    if (k1.layers != k2.layers) return false;
    if (k1.samples != k2.samples) return false;
    if (k1.depthStencil != k2.depthStencil) return false;
    if (k1.depthStencilResolve != k2.depthStencilResolve) return false;
    if (k1.fragmentDensityMap != k2.fragmentDensityMap) return false;
    for (int i = 0; i < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT; i++) {
        if (k1.color[i] != k2.color[i]) return false;
        if (k1.resolve[i] != k2.resolve[i]) return false;
    }
    return true;
}

VulkanFboCache::VulkanFboCache(VkDevice device, uint32_t timeBeforeEvictionFbo,
    VkResolveModeFlagBits depthResolveMode)
        : mDevice(device),
      mDepthResolveMode(depthResolveMode),
          mTimeBeforeEvictionFbo(timeBeforeEvictionFbo) {}

VulkanFboCache::~VulkanFboCache() {
    FILAMENT_CHECK_POSTCONDITION(mFramebufferCache.empty() && mRenderPassCache.empty())
            << "Please explicitly call terminate() while the VkDevice is still alive.";
}

fvkmemory::resource_ptr<VulkanFramebuffer> VulkanFboCache::getFramebuffer(FboKey const& config,
        fvkmemory::ResourceManager* resManager,
        fvkmemory::resource_ptr<VulkanRenderTarget> renderTarget) noexcept {
    FboMap::iterator iter = mFramebufferCache.find(config);
    if (UTILS_LIKELY(iter != mFramebufferCache.end())) {
        iter.value().timestamp = mCurrentTime;
        return iter->second.handle;
    }

    // The attachment list contains: Color Attachments, Resolve Attachments, Depth Attachment, and
    // the resolve target for the Depth Attachment.
    // For simplicity, create an array that can hold the maximum possible number of attachments.
    // Note that this needs to have the same ordering as the corollary array in getRenderPass.
    VkImageView attachments[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT +
            MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT + 3];
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
    if (config.depthStencil) {
        attachments[attachmentCount++] = config.depthStencil;
    }
    if (config.depthStencilResolve) {
        attachments[attachmentCount++] = config.depthStencilResolve;
    }
    if (config.fragmentDensityMap) {
        attachments[attachmentCount++] = config.fragmentDensityMap;
    }

    #if FVK_ENABLED(FVK_DEBUG_FBO_CACHE)
    FVK_LOGD << "Creating framebuffer " << config.width << "x" << config.height << " "
        << "for render pass " << config.renderPass << ", "
        << "samples = " << int(config.samples) << ", "
        << "depth = " << (config.depth ? 1 : 0) << ", "
        << "attachmentCount = " << attachmentCount;
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
    FILAMENT_CHECK_POSTCONDITION(error == VK_SUCCESS) << "Unable to create framebuffer."
                                                     << " error=" << static_cast<int32_t>(error);
    fvkmemory::resource_ptr<VulkanFramebuffer> fbh =
            fvkmemory::resource_ptr<VulkanFramebuffer>::construct(resManager, mDevice, framebuffer,
                    renderTarget);
    mFramebufferCache[config] = { fbh, mCurrentTime };
    return fbh;
}

fvkmemory::resource_ptr<VulkanRenderPass> VulkanFboCache::getRenderPass(
        RenderPassKey const& config, fvkmemory::ResourceManager* resManager) noexcept {
    auto iter = mRenderPassCache.find(config);
    if (UTILS_LIKELY(iter != mRenderPassCache.end())) {
        iter.value().timestamp = mCurrentTime;
        return iter->second.handle;
    }
    const bool hasSubpasses = config.subpassMask != 0;

    // The second subpass reads its first color attachment while still writing it, and Vulkan only
    // allows GENERAL for an attachment used both ways at once.
    constexpr VkImageLayout kFeedbackLoopLayout = VK_IMAGE_LAYOUT_GENERAL;

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
    VkAttachmentReference depthStencilAttachmentRef = {};
    VkAttachmentReference depthStencilResolveAttachmentRef = {};

    const bool hasDepth = fvkutils::isVkDepthFormat(config.depthStencilFormat);
    const bool hasStencil = fvkutils::isVkStencilFormat(config.depthStencilFormat);
    const bool hasDepthOrStencil = hasDepth || hasStencil;

    VkSubpassDescription subpasses[2] = {
        { .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .pInputAttachments = nullptr,
            .pColorAttachments = colorAttachmentRefs[0],
            .pResolveAttachments = resolveAttachmentRef,
            .pDepthStencilAttachment = hasDepthOrStencil ? &depthStencilAttachmentRef : nullptr },
        { .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .pInputAttachments = inputAttachmentRef,
            .pColorAttachments = colorAttachmentRefs[1],
            .pResolveAttachments = resolveAttachmentRef,
            .pDepthStencilAttachment = hasDepthOrStencil && !config.multisampledSubpassInput
                                               ? &depthStencilAttachmentRef
                                               : nullptr }
    };

    // The attachment list contains: Color Attachments, Resolve Attachments, Depth/Stencil
    // Attachment, and the resolve target for the Depth/Stencil Attachment.
    // For simplicity, create an array that can hold the maximum possible number of attachments.
    // Note that this needs to have the same ordering as the corollary array in getFramebuffer.
        VkAttachmentDescription attachments[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT +
            MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT + 3] = {};

    // We support 2 subpasses, which means we need to supply 1 dependency struct.
    VkSubpassDependency dependencies[1] = { {
        .srcSubpass = 0,
        .dstSubpass = 1,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
    } };

    VkRenderPassCreateInfo renderPassInfo {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 0u,
        .pAttachments = attachments,
        .subpassCount = hasSubpasses ? 2u : 1u,
        .pSubpasses = subpasses,
        .dependencyCount = hasSubpasses ? 1u : 0u,
        .pDependencies = dependencies
    };

    VkAttachmentReference fragmentDensityMapRef = {
        .attachment = VK_ATTACHMENT_UNUSED,
        .layout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT,
    };
    VkRenderPassFragmentDensityMapCreateInfoEXT fragmentDensityMapInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_FRAGMENT_DENSITY_MAP_CREATE_INFO_EXT,
        .fragmentDensityMapAttachment = fragmentDensityMapRef,
    };

    VkRenderPassMultiviewCreateInfo multiviewCreateInfo = {};
    uint32_t const subpassViewMask = (1 << config.viewCount) - 1;
    // Prepare a view mask array for the maximum number of subpasses. All subpasses have all views
    // activated.
    uint32_t const viewMasks[2] = {subpassViewMask, subpassViewMask};
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
        const VkImageLayout subpassLayout = fvkutils::getVkLayout(VulkanLayout::COLOR_ATTACHMENT);
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
                inputAttachmentRef[index].layout =
                        config.multisampledSubpassInput ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                                                        : kFeedbackLoopLayout;
                inputAttachmentRef[index].attachment = attachmentIndex;
            }

            if (!(config.multisampledSubpassInput && (config.subpassMask & (1 << i)))) {
                index = subpasses[1].colorAttachmentCount++;
                colorAttachmentRefs[1][index].layout =
                        (config.subpassMask & (1 << i)) ? kFeedbackLoopLayout : subpassLayout;
                colorAttachmentRefs[1][index].attachment = attachmentIndex;
            }
        }

        TargetBufferFlags const flag = TargetBufferFlags(int(TargetBufferFlags::COLOR0) << i);
        bool const clear = any(config.clear & flag);
        bool const discardStart = any(config.discardStart & flag);
        bool const discardEnd = any(config.discardEnd & flag);

        attachments[attachmentIndex++] = {
            .format = config.colorFormat[i],
            .samples = (VkSampleCountFlagBits) config.colorSamples[i],
            .loadOp = clear ? kClear : (discardStart ? kDontCare : kKeep),
            .storeOp = (discardEnd || (config.usesLazilyAllocatedMemory & (1 << i))) ? kDisableStore
                                                                                     : kEnableStore,
            .stencilLoadOp = kDontCare,
            .stencilStoreOp = kDisableStore,
            .initialLayout = fvkutils::getVkLayout(VulkanLayout::COLOR_ATTACHMENT),
            .finalLayout = fvkutils::getVkLayout(VulkanLayout::COLOR_ATTACHMENT),
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
                = fvkutils::getVkLayout(VulkanLayout::COLOR_ATTACHMENT_RESOLVE);
        ++pResolveAttachment;

        attachments[attachmentIndex++] = {
            .format = config.colorFormat[i],
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = kDontCare,
            .storeOp = kEnableStore,
            .stencilLoadOp = kDontCare,
            .stencilStoreOp = kDisableStore,
            .initialLayout = fvkutils::getVkLayout(VulkanLayout::COLOR_ATTACHMENT),
            .finalLayout = fvkutils::getVkLayout(VulkanLayout::COLOR_ATTACHMENT),
        };
    }

    // Populate the Depth/Stencil Attachment.
    if (hasDepthOrStencil) {
        const bool clearDepth = any(config.clear & TargetBufferFlags::DEPTH);
        const bool discardStartDepth = any(config.discardStart & TargetBufferFlags::DEPTH);
        const bool discardEndDepth = any(config.discardEnd & TargetBufferFlags::DEPTH);
        const bool clearStencil = any(config.clear & TargetBufferFlags::STENCIL);
        const bool discardStartStencil = any(config.discardStart & TargetBufferFlags::STENCIL);
        const bool discardEndStencil = any(config.discardEnd & TargetBufferFlags::STENCIL);

        depthStencilAttachmentRef.layout = fvkutils::getVkLayout(VulkanLayout::DEPTH_STENCIL_ATTACHMENT);
        depthStencilAttachmentRef.attachment = attachmentIndex;
        attachments[attachmentIndex++] = {
            .format = config.depthStencilFormat,
            .samples = (VkSampleCountFlagBits) config.samples,
            .loadOp = hasDepth ? (clearDepth ? kClear : (discardStartDepth ? kDontCare : kKeep)) : kDontCare,
            .storeOp = hasDepth ? (discardEndDepth ? kDisableStore : kEnableStore) : kDisableStore,
            .stencilLoadOp = hasStencil ? (clearStencil ? kClear : (discardStartStencil ? kDontCare : kKeep)) : kDontCare,
            .stencilStoreOp = hasStencil ? (discardEndStencil ? kDisableStore : kEnableStore) : kDisableStore,
            .initialLayout = fvkutils::getVkLayout(config.initialDepthStencilLayout),
            .finalLayout = fvkutils::getVkLayout(VulkanLayout::DEPTH_STENCIL_ATTACHMENT),
        };

        // The resolve target is single-sampled and is only ever written, so its prior contents
        // never matter.
        if (config.needsDepthResolve) {
            depthStencilResolveAttachmentRef.layout =
                    fvkutils::getVkLayout(VulkanLayout::DEPTH_STENCIL_ATTACHMENT);
            depthStencilResolveAttachmentRef.attachment = attachmentIndex;
            attachments[attachmentIndex++] = {
                .format = config.depthStencilFormat,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .loadOp = kDontCare,
                .storeOp = hasDepth ? kEnableStore : kDisableStore,
                .stencilLoadOp = kDontCare,
                .stencilStoreOp = hasStencil ? kEnableStore : kDisableStore,
                .initialLayout = fvkutils::getVkLayout(config.initialDepthStencilLayout),
                .finalLayout = fvkutils::getVkLayout(VulkanLayout::DEPTH_STENCIL_ATTACHMENT),
            };
        }
    }

    if (config.fragmentDensityMapFormat != VK_FORMAT_UNDEFINED) {
        fragmentDensityMapRef.attachment = attachmentIndex;
        fragmentDensityMapInfo.fragmentDensityMapAttachment = fragmentDensityMapRef;
        fragmentDensityMapInfo.pNext = renderPassInfo.pNext;
        renderPassInfo.pNext = &fragmentDensityMapInfo;
        attachments[attachmentIndex++] = {
            .format = config.fragmentDensityMapFormat,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = kDontCare,
            .storeOp = kDisableStore,
            .stencilLoadOp = kDontCare,
            .stencilStoreOp = kDisableStore,
            .initialLayout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT,
            .finalLayout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT,
        };
    }
    renderPassInfo.attachmentCount = attachmentIndex;

    // Finally, create the VkRenderPass.
    VkRenderPass renderPass;
    VkResult error;
    if (config.needsDepthResolve) {
        // Only the VK_KHR_create_renderpass2 structures can express a depth resolve, so translate
        // what was built above rather than duplicating the logic.
        error = createRenderPass2(mDevice, renderPassInfo,
                config.viewCount > 1 ? subpassViewMask : 0u, depthStencilResolveAttachmentRef,
            fragmentDensityMapRef, mDepthResolveMode, &renderPass);
    } else {
        error = vkCreateRenderPass(mDevice, &renderPassInfo, VKALLOC, &renderPass);
    }
    FILAMENT_CHECK_POSTCONDITION(error == VK_SUCCESS) << "Unable to create render pass."
                                                      << " error=" << error;
    fvkmemory::resource_ptr<VulkanRenderPass> rph =
        fvkmemory::resource_ptr<VulkanRenderPass>::construct(resManager, mDevice, renderPass);
    mRenderPassCache[config] = {rph, mCurrentTime};

#if FVK_ENABLED(FVK_DEBUG_FBO_CACHE)
    FVK_LOGD << "Created render pass " << renderPass << " with ";
    for (int i = 0; i < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT; ++i) {
        FVK_LOGD << (int) config.colorFormat[i] << " ";
    }
    FVK_LOGD << ", "
             << "depth = " << config.depthFormat << ", "
             << "initialDepthLayout = " << (int) config.initialDepthLayout << ", "
             << "samples = " << int(config.samples) << ", "
             << "needsResolveMask = " << int(config.needsResolveMask) << ", "
             << "usesLazilyAllocatedMemory = " << int(config.usesLazilyAllocatedMemory) << ", "
             << "viewCount = " << int(config.viewCount) << ", "
             << "colorAttachmentCount[0] = " << subpasses[0].colorAttachmentCount;
#endif

    return rph;
}

void VulkanFboCache::resetFramebuffers() noexcept {
    for (const auto& pair: mFramebufferCache) {
        mRenderPassRefCount[pair.first.renderPass]--;
    }
    mFramebufferCache.clear();
}

void VulkanFboCache::terminate() noexcept {
    resetFramebuffers();

    mRenderPassRefCount.clear();
    mRenderPassCache.clear();
}

// Frees up old framebuffers and render passes, then nulls out their key.  Doesn't bother removing
// the actual map entry since it is fairly small.
void VulkanFboCache::gc() noexcept {
    FVK_SYSTRACE_CONTEXT();
    FVK_SYSTRACE_START("fbocache::gc");

    // If this is one of the first few frames, return early to avoid wrapping unsigned integers.
    ++mCurrentTime;

    if (UTILS_UNLIKELY(mCurrentTime > mTimeBeforeEvictionFbo)) {
        const uint32_t evictTimeFbo = mCurrentTime - mTimeBeforeEvictionFbo;
        for (FboMap::iterator iter = mFramebufferCache.begin(); iter != mFramebufferCache.end();) {
            const FboVal fbo = iter->second;
            if (fbo.timestamp < evictTimeFbo && fbo.handle) {
                mRenderPassRefCount[iter->first.renderPass]--;

                // erase(iterator) returns the iterator to the next element.
                iter = mFramebufferCache.erase(iter);
            } else {
                ++iter;
            }
        }
    }

    if (UTILS_UNLIKELY(mCurrentTime > TIME_BEFORE_EVICTION)) {
        const uint32_t evictTimeRp = mCurrentTime - TIME_BEFORE_EVICTION;
        for (RenderPassMap::iterator iter = mRenderPassCache.begin();
                iter != mRenderPassCache.end();) {
            const VkRenderPass handle = iter->second.handle->getVkRenderPass();
            if (iter->second.timestamp < evictTimeRp && handle &&
                    mRenderPassRefCount[handle] == 0) {
                // erase(iterator) returns the iterator to the next element.
                iter = mRenderPassCache.erase(iter);
                mRenderPassRefCount.erase(handle);
            } else {
                ++iter;
            }
        }
    }

    FVK_SYSTRACE_END();
}

} // namespace filament::backend
