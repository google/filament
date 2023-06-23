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

#ifndef TNT_FILAMENT_BACKEND_VULKANBLITTER_H
#define TNT_FILAMENT_BACKEND_VULKANBLITTER_H

#include "VulkanContext.h"

#include <utils/compiler.h>

namespace filament::backend {

class VulkanBuffer;
class VulkanFboCache;
class VulkanPipelineCache;
class VulkanSamplerCache;

struct VulkanProgram;

class VulkanBlitter {
public:
    VulkanBlitter(VulkanStagePool& stagePool, VulkanPipelineCache& pipelineCache,
            VulkanFboCache& fboCache, VulkanSamplerCache& samplerCache) noexcept;

    void initialize(VkPhysicalDevice physicalDevice, VkDevice device, VmaAllocator allocator,
            VulkanCommands* commands, VulkanTexture* emptyTexture) noexcept;

    struct BlitArgs {
        const VulkanRenderTarget* dstTarget;
        const VkOffset3D* dstRectPair;
        const VulkanRenderTarget* srcTarget;
        const VkOffset3D* srcRectPair;
        VkFilter filter = VK_FILTER_NEAREST;
        int targetIndex = 0;
    };

    void blitColor(BlitArgs args);
    void blitDepth(BlitArgs args);

    void terminate() noexcept;

private:
    void lazyInit() noexcept;

    void blitSlowDepth(VkFilter filter, const VkExtent2D srcExtent, VulkanAttachment src,
            VulkanAttachment dst, const VkOffset3D srcRect[2], const VkOffset3D dstRect[2]);

    VulkanBuffer* mTriangleBuffer = nullptr;
    VulkanBuffer* mParamsBuffer = nullptr;
    VulkanProgram* mDepthResolveProgram = nullptr;

    UTILS_UNUSED VkPhysicalDevice mPhysicalDevice;
    VkDevice mDevice;
    VmaAllocator mAllocator;
    VulkanCommands* mCommands;
    VulkanTexture* mEmptyTexture;

    VulkanStagePool& mStagePool;
    VulkanPipelineCache& mPipelineCache;
    VulkanFboCache& mFramebufferCache;
    VulkanSamplerCache& mSamplerCache;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANBLITTER_H
