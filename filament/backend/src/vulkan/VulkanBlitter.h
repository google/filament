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

#ifndef TNT_FILAMENT_DRIVER_VULKANBLITTER_H
#define TNT_FILAMENT_DRIVER_VULKANBLITTER_H

#include "VulkanContext.h"

namespace filament {
namespace backend {

class VulkanBlitter {
public:
    VulkanBlitter(VulkanContext& context) : mContext(context) {}

    struct BlitArgs {
        const VulkanRenderTarget* dstTarget;
        const VkOffset3D* dstRectPair;
        const VulkanRenderTarget* srcTarget;
        const VkOffset3D* srcRectPair;
        VkFilter filter = VK_FILTER_NEAREST;
        int targetIndex = 0;
    };

    void blitColor(VkCommandBuffer cmdBuffer, BlitArgs args);
    void blitDepth(VkCommandBuffer cmdBuffer, BlitArgs args);

    void shutdown() noexcept;

private:

    void blitFast(VkImageAspectFlags aspect, VkFilter filter, const VulkanRenderTarget* srcTarget,
        VulkanAttachment src, VulkanAttachment dst, const VkOffset3D srcRect[2],
        const VkOffset3D dstRect[2], VkCommandBuffer cmdBuffer);

    VulkanContext& mContext;
};

} // namespace filament
} // namespace backend

#endif // TNT_FILAMENT_DRIVER_VULKANBLITTER_H
