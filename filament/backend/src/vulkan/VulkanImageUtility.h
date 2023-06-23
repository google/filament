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

#ifndef TNT_FILAMENT_BACKEND_VULKANIMAGEUTILITY_H
#define TNT_FILAMENT_BACKEND_VULKANIMAGEUTILITY_H

#include <backend/DriverEnums.h>

#include <utils/Log.h>

#include <bluevk/BlueVK.h>

namespace filament::backend {

struct VulkanTexture;

enum class VulkanLayout : uint8_t {
    // The initial layout after the creation of the VkImage. We use this to denote the state before
    // any transition.
    UNDEFINED,
    // Fragment/vertex shader accessible layout for reading and writing.
    READ_WRITE,
    // Fragment/vertex shader accessible layout for reading only.
    READ_ONLY,
    // For the source of a copy operation.
    TRANSFER_SRC,
    // For the destination of a copy operation.
    TRANSFER_DST,
    // For using a depth texture as an attachment.
    DEPTH_ATTACHMENT,
    // For using a depth texture both as an attachment and as a sampler.
    DEPTH_SAMPLER,
    // For swapchain images that will be presented.
    PRESENT,
    // For color attachments, but also used when the image is a sampler.
    // TODO: explore separate layout policies for attachment+sampling and just attachment.
    COLOR_ATTACHMENT,
    // For color attachment MSAA resolves.
    COLOR_ATTACHMENT_RESOLVE,
};

struct VulkanLayoutTransition {
    VkImage image;
    VulkanLayout oldLayout;
    VulkanLayout newLayout;
    VkImageSubresourceRange subresources;
};

class VulkanImageUtility {
public:
    static VkImageViewType getViewType(SamplerType target);

    inline static VulkanLayout getDefaultLayout(TextureUsage usage) {
        if (any(usage & TextureUsage::DEPTH_ATTACHMENT)) {
            if (any(usage & TextureUsage::SAMPLEABLE)) {
                return VulkanLayout::DEPTH_SAMPLER;
            } else {
                return VulkanLayout::DEPTH_ATTACHMENT;
            }
        }

        if (any(usage & TextureUsage::COLOR_ATTACHMENT)) {
            return VulkanLayout::COLOR_ATTACHMENT;
        }
        // Finally, the layout for an immutable texture is optimal read-only.        
        return VulkanLayout::READ_ONLY;
    }

    inline static VulkanLayout getDefaultLayout(VkImageUsageFlags vkusage) {
        TextureUsage usage {};
        if (vkusage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            usage = usage | TextureUsage::DEPTH_ATTACHMENT;
        }
        if (vkusage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
            usage = usage | TextureUsage::COLOR_ATTACHMENT;
        }
        if (vkusage & VK_IMAGE_USAGE_SAMPLED_BIT) {
            usage = usage | TextureUsage::SAMPLEABLE;
        }
        return getDefaultLayout(usage);
    }

    static VkImageLayout getVkLayout(VulkanLayout layout);
    
    static void transitionLayout(VkCommandBuffer cmdbuffer, VulkanLayoutTransition transition);
};

} // namespace filament::backend

bool operator<(const VkImageSubresourceRange& a, const VkImageSubresourceRange& b);

utils::io::ostream& operator<<(utils::io::ostream& out, const filament::backend::VulkanLayout& layout);


#endif // TNT_FILAMENT_BACKEND_VULKANIMAGEUTILITY_H
