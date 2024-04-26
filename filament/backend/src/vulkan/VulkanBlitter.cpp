/*
 * Copyright (C) 2021 The Android Open Source Project
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

#include "VulkanBlitter.h"
#include "VulkanContext.h"
#include "VulkanFboCache.h"
#include "VulkanHandles.h"
#include "VulkanSamplerCache.h"
#include "VulkanTexture.h"

#include <utils/FixedCapacityVector.h>
#include <utils/Panic.h>

#include <smolv.h>

using namespace bluevk;
using namespace utils;

namespace filament::backend {

namespace {

inline void blitFast(const VkCommandBuffer cmdbuffer, VkImageAspectFlags aspect, VkFilter filter,
        VulkanAttachment src, VulkanAttachment dst,
        const VkOffset3D srcRect[2], const VkOffset3D dstRect[2]) {
    if constexpr (FVK_ENABLED(FVK_DEBUG_BLITTER)) {
        utils::slog.d << "Fast blit from=" << src.texture->getVkImage() << ",level=" << (int) src.level
                      << " layout=" << src.getLayout()
                      << " to=" << dst.texture->getVkImage() << ",level=" << (int) dst.level
                      << " layout=" << dst.getLayout() << utils::io::endl;
    }

    VkImageSubresourceRange const srcRange = src.getSubresourceRange();
    VkImageSubresourceRange const dstRange = dst.getSubresourceRange();

    VulkanLayout oldSrcLayout = src.getLayout();
    VulkanLayout oldDstLayout = dst.getLayout();

    src.texture->transitionLayout(cmdbuffer, srcRange, VulkanLayout::TRANSFER_SRC);
    dst.texture->transitionLayout(cmdbuffer, dstRange, VulkanLayout::TRANSFER_DST);

    const VkImageBlit blitRegions[1] = {{
            .srcSubresource = { aspect, src.level, src.layer, 1 },
            .srcOffsets = { srcRect[0], srcRect[1] },
            .dstSubresource = { aspect, dst.level, dst.layer, 1 },
            .dstOffsets = { dstRect[0], dstRect[1] },
    }};
    vkCmdBlitImage(cmdbuffer,
            src.getImage(), imgutil::getVkLayout(VulkanLayout::TRANSFER_SRC),
            dst.getImage(), imgutil::getVkLayout(VulkanLayout::TRANSFER_DST),
            1, blitRegions, filter);

    if (oldSrcLayout == VulkanLayout::UNDEFINED) {
        oldSrcLayout = imgutil::getDefaultLayout(src.texture->usage);
    }
    if (oldDstLayout == VulkanLayout::UNDEFINED) {
        oldDstLayout = imgutil::getDefaultLayout(dst.texture->usage);
    }
    src.texture->transitionLayout(cmdbuffer, srcRange, oldSrcLayout);
    dst.texture->transitionLayout(cmdbuffer, dstRange, oldDstLayout);
}

inline void resolveFast(const VkCommandBuffer cmdbuffer, VkImageAspectFlags aspect,
        VulkanAttachment src, VulkanAttachment dst) {
    if constexpr (FVK_ENABLED(FVK_DEBUG_BLITTER)) {
        utils::slog.d << "Fast blit from=" << src.texture->getVkImage() << ",level=" << (int) src.level
                      << " layout=" << src.getLayout()
                      << " to=" << dst.texture->getVkImage() << ",level=" << (int) dst.level
                      << " layout=" << dst.getLayout() << utils::io::endl;
    }

    VkImageSubresourceRange const srcRange = src.getSubresourceRange();
    VkImageSubresourceRange const dstRange = dst.getSubresourceRange();

    VulkanLayout oldSrcLayout = src.getLayout();
    VulkanLayout oldDstLayout = dst.getLayout();

    src.texture->transitionLayout(cmdbuffer, srcRange, VulkanLayout::TRANSFER_SRC);
    dst.texture->transitionLayout(cmdbuffer, dstRange, VulkanLayout::TRANSFER_DST);

    assert_invariant(
            aspect != VK_IMAGE_ASPECT_DEPTH_BIT && "Resolve with depth is not yet supported.");
    const VkImageResolve resolveRegions[1] = {{
            .srcSubresource = { aspect, src.level, src.layer, 1 },
            .srcOffset = { 0, 0 },
            .dstSubresource = { aspect, dst.level, dst.layer, 1 },
            .dstOffset = { 0, 0 },
            .extent = { src.getExtent2D().width, src.getExtent2D().height, 1 },
    }};
    vkCmdResolveImage(cmdbuffer,
            src.getImage(), imgutil::getVkLayout(VulkanLayout::TRANSFER_SRC),
            dst.getImage(), imgutil::getVkLayout(VulkanLayout::TRANSFER_DST),
            1, resolveRegions);

    if (oldSrcLayout == VulkanLayout::UNDEFINED) {
        oldSrcLayout = imgutil::getDefaultLayout(src.texture->usage);
    }
    if (oldDstLayout == VulkanLayout::UNDEFINED) {
        oldDstLayout = imgutil::getDefaultLayout(dst.texture->usage);
    }
    src.texture->transitionLayout(cmdbuffer, srcRange, oldSrcLayout);
    dst.texture->transitionLayout(cmdbuffer, dstRange, oldDstLayout);
}

struct BlitterUniforms {
    int sampleCount;
    float inverseSampleCount;
};

}// anonymous namespace

VulkanBlitter::VulkanBlitter(VkPhysicalDevice physicalDevice, VulkanCommands* commands) noexcept
    : mPhysicalDevice(physicalDevice),
      mCommands(commands) {}

void VulkanBlitter::resolve(VulkanAttachment dst, VulkanAttachment src) {

    // src and dst should have the same aspect here
    VkImageAspectFlags const aspect = src.texture->getImageAspect();

    assert_invariant(!(aspect & VK_IMAGE_ASPECT_DEPTH_BIT));

#if FVK_ENABLED(FVK_DEBUG_BLIT_FORMAT)
    VkPhysicalDevice const gpu = mPhysicalDevice;
    VkFormatProperties info;
    vkGetPhysicalDeviceFormatProperties(gpu, src.getFormat(), &info);
    if (!ASSERT_POSTCONDITION_NON_FATAL(info.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT,
                "Depth src format is not blittable %d", src.getFormat())) {
        return;
    }
    vkGetPhysicalDeviceFormatProperties(gpu, dst.getFormat(), &info);
    if (!ASSERT_POSTCONDITION_NON_FATAL(info.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT,
                "Depth dst format is not blittable %d", dst.getFormat())) {
        return;
    }
#endif

    VulkanCommandBuffer& commands = mCommands->get();
    VkCommandBuffer const cmdbuffer = commands.buffer();
    commands.acquire(src.texture);
    commands.acquire(dst.texture);
    resolveFast(cmdbuffer, aspect, src, dst);
}

void VulkanBlitter::blit(VkFilter filter,
        VulkanAttachment dst, const VkOffset3D* dstRectPair,
        VulkanAttachment src, const VkOffset3D* srcRectPair) {
#if FVK_ENABLED(FVK_DEBUG_BLIT_FORMAT)
    VkPhysicalDevice const gpu = mPhysicalDevice;
    VkFormatProperties info;
    vkGetPhysicalDeviceFormatProperties(gpu, src.getFormat(), &info);
    if (!ASSERT_POSTCONDITION_NON_FATAL(info.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT,
                "Depth src format is not blittable %d", src.getFormat())) {
        return;
    }
    vkGetPhysicalDeviceFormatProperties(gpu, dst.getFormat(), &info);
    if (!ASSERT_POSTCONDITION_NON_FATAL(info.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT,
                "Depth dst format is not blittable %d", dst.getFormat())) {
        return;
    }
#endif
    // src and dst should have the same aspect here
    VkImageAspectFlags const aspect = src.texture->getImageAspect();
    VulkanCommandBuffer& commands = mCommands->get();
    VkCommandBuffer const cmdbuffer = commands.buffer();
    commands.acquire(src.texture);
    commands.acquire(dst.texture);
    blitFast(cmdbuffer, aspect, filter, src, dst, srcRectPair, dstRectPair);
}

void VulkanBlitter::terminate() noexcept {
}

} // namespace filament::backend
