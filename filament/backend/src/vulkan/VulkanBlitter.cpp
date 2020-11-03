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
#include "VulkanHandles.h"

#include <utils/Panic.h>

#define FILAMENT_VULKAN_CHECK_BLIT_FORMAT 0

using namespace bluevk;

namespace filament {
namespace backend {

void VulkanBlitter::blitColor(VkCommandBuffer cmdBuffer, BlitArgs args) {
    const VulkanAttachment src = args.srcTarget->getColor(args.targetIndex);
    const VulkanAttachment dst = args.dstTarget->getColor(0);
    const VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;

#if FILAMENT_VULKAN_CHECK_BLIT_FORMAT
    const VkPhysicalDevice gpu = mContext.physicalDevice;
    VkFormatProperties info;
    vkGetPhysicalDeviceFormatProperties(gpu, src.format, &info);
    if (!ASSERT_POSTCONDITION_NON_FATAL(info.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT,
            "Source format is not blittable")) {
        return;
    }
    vkGetPhysicalDeviceFormatProperties(gpu, dst.format, &info);
    if (!ASSERT_POSTCONDITION_NON_FATAL(info.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT,
            "Destination format is not blittable")) {
        return;
    }
#endif

    blitFast(aspect, args.filter, args.srcTarget, src, dst, args.srcRectPair, args.dstRectPair,
            cmdBuffer);
}

void VulkanBlitter::blitDepth(VkCommandBuffer cmdBuffer, BlitArgs args) {
    const VulkanAttachment src = args.srcTarget->getDepth();
    const VulkanAttachment dst = args.dstTarget->getDepth();
    const VkImageAspectFlags aspect = VK_IMAGE_ASPECT_DEPTH_BIT;

#if FILAMENT_VULKAN_CHECK_BLIT_FORMAT
    const VkPhysicalDevice gpu = mContext.physicalDevice;
    VkFormatProperties info;
    vkGetPhysicalDeviceFormatProperties(gpu, src.format, &info);
    if (!ASSERT_POSTCONDITION_NON_FATAL(info.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT,
            "Depth format is not blittable")) {
        return;
    }
    vkGetPhysicalDeviceFormatProperties(gpu, dst.format, &info);
    if (!ASSERT_POSTCONDITION_NON_FATAL(info.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT,
            "Depth format is not blittable")) {
        return;
    }
#endif

    blitFast(aspect, args.filter, args.srcTarget, src, dst, args.srcRectPair, args.dstRectPair,
            cmdBuffer);
}

void VulkanBlitter::blitFast(VkImageAspectFlags aspect, VkFilter filter,
    const VulkanRenderTarget* srcTarget, VulkanAttachment src, VulkanAttachment dst,
    const VkOffset3D srcRect[2], const VkOffset3D dstRect[2], VkCommandBuffer cmdbuffer) {
    const VkImageBlit blitRegions[1] = {{
        .srcSubresource = { aspect, src.level, src.layer, 1 },
        .srcOffsets = { srcRect[0], srcRect[1] },
        .dstSubresource = { aspect, dst.level, dst.layer, 1 },
        .dstOffsets = { dstRect[0], dstRect[1] }
    }};

    const VkExtent2D srcExtent = srcTarget->getExtent();

    const VkImageResolve resolveRegions[1] = {{
        .srcSubresource = { aspect, src.level, src.layer, 1 },
        .srcOffset = srcRect[0],
        .dstSubresource = { aspect, dst.level, dst.layer, 1 },
        .dstOffset = dstRect[0],
        .extent = { srcExtent.width, srcExtent.height, 1 }
    }};

    VulkanTexture::transitionImageLayout(cmdbuffer, src.image, VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, src.level, 1, 1, aspect);

    VulkanTexture::transitionImageLayout(cmdbuffer, dst.image, VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, dst.level, 1, 1, aspect);

    if (src.texture && src.texture->samples > 1 && dst.texture && dst.texture->samples == 1) {
        assert(aspect != VK_IMAGE_ASPECT_DEPTH_BIT && "Resolve with depth is not yet supported.");
        vkCmdResolveImage(cmdbuffer, src.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst.image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, resolveRegions);
    } else {
        vkCmdBlitImage(cmdbuffer, src.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst.image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, blitRegions, filter);
    }

    if (src.texture) {
        VulkanTexture::transitionImageLayout(cmdbuffer, src.image, VK_IMAGE_LAYOUT_UNDEFINED,
                getTextureLayout(src.texture->usage), src.level, 1, 1, aspect);
    } else if  (!mContext.currentSurface->headlessQueue) {
        VulkanTexture::transitionImageLayout(cmdbuffer, src.image, VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, src.level, 1, 1, aspect);
    }

    // Determine the desired texture layout for the destination while ensuring that the default
    // render target is supported, which has no associated texture.
    const VkImageLayout desiredLayout = dst.texture ? getTextureLayout(dst.texture->usage) :
            getSwapContext(mContext).attachment.layout;

    VulkanTexture::transitionImageLayout(cmdbuffer, dst.image, VK_IMAGE_LAYOUT_UNDEFINED,
            desiredLayout, dst.level, 1, 1, aspect);
}

void VulkanBlitter::shutdown() noexcept {
    // TODO
}

} // namespace filament
} // namespace backend
