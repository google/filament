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

#include "VulkanImageUtility.h"

#include "VulkanTexture.h"

#include <utils/Panic.h>
#include <utils/algorithm.h>
#include <utils/debug.h>

#include <tuple>

using namespace bluevk;

namespace filament::backend {

namespace {

inline VkImageLayout getVkImageLayout(VulkanLayout layout) {
    switch (layout) {
        case VulkanLayout::UNDEFINED:
            return VK_IMAGE_LAYOUT_UNDEFINED;
        case VulkanLayout::READ_WRITE:
            return VK_IMAGE_LAYOUT_GENERAL;
        case VulkanLayout::READ_ONLY:
            return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        case VulkanLayout::TRANSFER_SRC:
            return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        case VulkanLayout::TRANSFER_DST:
            return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        case VulkanLayout::DEPTH_ATTACHMENT:
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        case VulkanLayout::DEPTH_SAMPLER:
            return VK_IMAGE_LAYOUT_GENERAL;
        case VulkanLayout::PRESENT:
            return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        // Filament sometimes samples from one miplevel while writing to another level in the same
        // texture (e.g. bloom does this). Moreover we'd like to avoid lots of expensive layout
        // transitions. So, keep it simple and use GENERAL for all color-attachable textures.
        case VulkanLayout::COLOR_ATTACHMENT:
            return VK_IMAGE_LAYOUT_GENERAL;
        case VulkanLayout::COLOR_ATTACHMENT_RESOLVE:
            return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }
}

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
            srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT
                            | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
                            | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            srcStage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
                       | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
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
            getVkImageLayout(transition.oldLayout), getVkImageLayout(transition.newLayout));
}

}// anonymous namespace

VkImageViewType VulkanImageUtility::getViewType(SamplerType target) {
    switch (target) {
        case SamplerType::SAMPLER_CUBEMAP:
            return VK_IMAGE_VIEW_TYPE_CUBE;
        case SamplerType::SAMPLER_2D_ARRAY:
            return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        case SamplerType::SAMPLER_CUBEMAP_ARRAY:
            return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
        case SamplerType::SAMPLER_3D:
            return VK_IMAGE_VIEW_TYPE_3D;
        default:
            return VK_IMAGE_VIEW_TYPE_2D;
    }
}

void VulkanImageUtility::transitionLayout(VkCommandBuffer cmdbuffer,
        VulkanLayoutTransition transition) {
    if (transition.oldLayout == transition.newLayout) {
        return;
    }
    auto [srcAccessMask, dstAccessMask, srcStage, dstStage, oldLayout, newLayout]
            = getVkTransition(transition);

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
}

VkImageLayout VulkanImageUtility::getVkLayout(VulkanLayout layout) {
    return getVkImageLayout(layout);
}

}// namespace filament::backend

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
            << filament::backend::VulkanImageUtility::getVkLayout(                                 \
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
