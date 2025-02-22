/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include "VulkanContext.h"

#include "VulkanCommands.h"
#include "VulkanHandles.h"
#include "VulkanMemory.h"
#include "VulkanTexture.h"

#include <backend/PixelBufferDescriptor.h>

#include <utils/Panic.h>

#include <algorithm> // for std::max

using namespace bluevk;

namespace {

} // end anonymous namespace

namespace filament::backend {

VkImage VulkanAttachment::getImage() const {
    return texture ? texture->getVkImage() : VK_NULL_HANDLE;
}

VkFormat VulkanAttachment::getFormat() const {
    return texture ? texture->getVkFormat() : VK_FORMAT_UNDEFINED;
}

VulkanLayout VulkanAttachment::getLayout() const {
    return texture ? texture->getLayout(layer, level) : VulkanLayout::UNDEFINED;
}

VkExtent2D VulkanAttachment::getExtent2D() const {
    assert_invariant(texture);
    return { std::max(1u, texture->width >> level), std::max(1u, texture->height >> level) };
}

VkImageView VulkanAttachment::getImageView() {
    assert_invariant(texture);
    VkImageSubresourceRange range = getSubresourceRange();
    if (range.layerCount > 1) {
        return texture->getMultiviewAttachmentView(range);
    }
    return texture->getAttachmentView(range);
}

bool VulkanAttachment::isDepth() const {
    return texture->getImageAspect() & VK_IMAGE_ASPECT_DEPTH_BIT;
}

VkImageSubresourceRange VulkanAttachment::getSubresourceRange() const {
    assert_invariant(texture);
    return {
            .aspectMask = texture->getImageAspect(),
            .baseMipLevel = uint32_t(level),
            .levelCount = 1,
            .baseArrayLayer = uint32_t(layer),
            .layerCount = layerCount,
    };
}


} // namespace filament::backend
