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

 #ifndef TNT_FILAMENT_BACKEND_VULKANSWAPCHAIN_H
 #define TNT_FILAMENT_BACKEND_VULKANSWAPCHAIN_H

#include "VulkanContext.h"
#include "VulkanDriver.h"

#include <memory>

#include <utils/FixedCapacityVector.h>

namespace filament::backend {


struct VulkanSwapChain : public HwSwapChain {
    VulkanSwapChain(VulkanContext& context, VulkanStagePool& stagePool, VkSurfaceKHR vksurface);

    // Headless constructor.
    VulkanSwapChain(VulkanContext& context, VulkanStagePool& stagePool, uint32_t width, uint32_t height);

    bool acquire();
    void create(VulkanStagePool& stagePool);
    void destroy();
    void makePresentable();
    bool hasResized() const;

    VulkanTexture& getColorTexture();
    VulkanTexture& getDepthTexture();
    uint32_t getSwapIndex() const { return mCurrentSwapIndex; }

    VkSurfaceKHR surface = {};
    VkSwapchainKHR swapchain = {};
    VkSurfaceFormatKHR surfaceFormat = {};
    VkExtent2D clientSize = {};
    VkQueue presentQueue = {};
    VkQueue headlessQueue = {};

    // This is signaled when vkAcquireNextImageKHR succeeds, and is waited on by the first
    // submission.
    VkSemaphore imageAvailable = VK_NULL_HANDLE;

    // This is true after the swap chain image has been acquired, but before it has been presented.
    bool acquired = false;

    bool suboptimal = false;
    bool firstRenderPass = false;

private:
    VulkanContext& mContext;
    uint32_t mCurrentSwapIndex = 0u;

    // Color attachments are swapped, but depth is not. Typically there are 2 or 3 color attachments
    // in a swap chain.
    utils::FixedCapacityVector<std::unique_ptr<VulkanTexture>> mColor;
    std::unique_ptr<VulkanTexture> mDepth;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANSWAPCHAIN_H
