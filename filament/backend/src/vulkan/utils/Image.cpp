/*
 * Copyright (C) 2023 The Android Open Source Project
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

#include "vulkan/utils/Image.h"

#include "vulkan/VulkanTexture.h"

#include <utils/Panic.h>
#include <utils/algorithm.h>
#include <utils/debug.h>

#include <tuple>

using namespace bluevk;

namespace filament::backend::fvkutils {

namespace {

inline std::tuple<VkAccessFlags, VkAccessFlags, VkPipelineStageFlags, VkPipelineStageFlags,
        VkImageLayout, VkImageLayout>
getVkTransition(const VulkanLayoutTransition& transition) {
    VkAccessFlags srcAccessMask, dstAccessMask;
    VkPipelineStageFlags srcStage, dstStage;

    switch (transition.oldLayout) {
        case VulkanLayout::UNDEFINED:
            srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            srcStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
            break;
        case VulkanLayout::COLOR_ATTACHMENT:
            srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT |
                            VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            srcStage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                       VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            break;
        case VulkanLayout::READ_WRITE:
            srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
            srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;
        case VulkanLayout::READ_ONLY:
            srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;
        case VulkanLayout::TRANSFER_SRC:
            srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        case VulkanLayout::TRANSFER_DST:
            srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        case VulkanLayout::DEPTH_ATTACHMENT:
            srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            srcStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            break;
        case VulkanLayout::DEPTH_SAMPLER:
            srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;
        case VulkanLayout::COLOR_ATTACHMENT_RESOLVE:
            srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        case VulkanLayout::PRESENT:
            srcAccessMask = VK_ACCESS_NONE;
            srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
    }

    switch (transition.newLayout) {
        case VulkanLayout::COLOR_ATTACHMENT:
            dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT
                            | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
                            | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dstStage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
                       | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            break;
        case VulkanLayout::READ_WRITE:
            dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
            dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;
        case VulkanLayout::READ_ONLY:
            dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
            break;
        case VulkanLayout::TRANSFER_SRC:
            dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        case VulkanLayout::TRANSFER_DST:
            dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        case VulkanLayout::DEPTH_ATTACHMENT:
            dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT
                            | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            break;
        case VulkanLayout::DEPTH_SAMPLER:
            dstAccessMask =
                    VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                       VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            break;
        case VulkanLayout::PRESENT:
        case VulkanLayout::COLOR_ATTACHMENT_RESOLVE:
        case VulkanLayout::UNDEFINED:
            dstAccessMask = 0;
            dstStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            break;
    }

    return std::make_tuple(srcAccessMask, dstAccessMask, srcStage, dstStage,
            getVkLayout(transition.oldLayout), getVkLayout(transition.newLayout));
}

}// anonymous namespace

bool transitionLayout(VkCommandBuffer cmdbuffer,
        VulkanLayoutTransition transition) {
    if (transition.oldLayout == transition.newLayout) {
        return false;
    }
    auto [srcAccessMask, dstAccessMask, srcStage, dstStage, oldLayout, newLayout]
            = getVkTransition(transition);

    if (oldLayout == newLayout) {
        return false;
    }

    assert_invariant(transition.image != VK_NULL_HANDLE && "No image for transition");
    VkImageMemoryBarrier barrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = srcAccessMask,
            .dstAccessMask = dstAccessMask,
            .oldLayout = oldLayout,
            .newLayout = newLayout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = transition.image,
            .subresourceRange = transition.subresources,
    };
    vkCmdPipelineBarrier(cmdbuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    return true;
}

VkImageAspectFlags getImageAspect(VkFormat format) {
    switch (format) {
        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_X8_D24_UNORM_PACK32:
        case VK_FORMAT_D32_SFLOAT:
            return VK_IMAGE_ASPECT_DEPTH_BIT;
        case VK_FORMAT_S8_UINT:
            return VK_IMAGE_ASPECT_STENCIL_BIT;
        default:
            return VK_IMAGE_ASPECT_COLOR_BIT;
    }
}

bool isVkDepthFormat(VkFormat format) {
    return (getImageAspect(format) & VK_IMAGE_ASPECT_DEPTH_BIT) != 0;
}

bool isVkStencilFormat(VkFormat format) {
    return (getImageAspect(format) & VK_IMAGE_ASPECT_STENCIL_BIT) != 0;
}

static uint32_t mostSignificantBit(uint32_t x) { return 1ul << (31ul - utils::clz(x)); }

uint8_t reduceSampleCount(uint8_t sampleCount, VkSampleCountFlags mask) {
    if (sampleCount & mask) {
        return sampleCount;
    }
    return mostSignificantBit((sampleCount - 1) & mask);
}

} // namespace filament::backend::fvkutils

bool operator<(const VkImageSubresourceRange& a, const VkImageSubresourceRange& b) {
    if (a.aspectMask < b.aspectMask) return true;
    if (a.aspectMask > b.aspectMask) return false;
    if (a.baseMipLevel < b.baseMipLevel) return true;
    if (a.baseMipLevel > b.baseMipLevel) return false;
    if (a.levelCount < b.levelCount) return true;
    if (a.levelCount > b.levelCount) return false;
    if (a.baseArrayLayer < b.baseArrayLayer) return true;
    if (a.baseArrayLayer > b.baseArrayLayer) return false;
    if (a.layerCount < b.layerCount) return true;
    if (a.layerCount > b.layerCount) return false;
    return false;
}

#if FVK_ENABLED(FVK_DEBUG_LAYOUT_TRANSITION) || FVK_ENABLED(FVK_DEBUG_TEXTURE)
#define CASE(VALUE)                                                                                \
    case filament::backend::VulkanLayout::VALUE: {                                                 \
        out << #VALUE;                                                                             \
        out << " ["                                                                                \
            << filament::backend::imgutil::getVkLayout(                                            \
                       filament::backend::VulkanLayout::VALUE)                                     \
            << "]";                                                                                \
        break;                                                                                     \
    }

utils::io::ostream& operator<<(utils::io::ostream& out,
        const filament::backend::VulkanLayout& layout) {
    switch (layout) {
        CASE(UNDEFINED)
        CASE(READ_ONLY)
        CASE(READ_WRITE)
        CASE(TRANSFER_SRC)
        CASE(TRANSFER_DST)
        CASE(DEPTH_ATTACHMENT)
        CASE(DEPTH_SAMPLER)
        CASE(PRESENT)
        CASE(COLOR_ATTACHMENT)
        CASE(COLOR_ATTACHMENT_RESOLVE)
    }
    return out;
}
#undef CASE
#endif
