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

#include <bluevk/BlueVK.h>
#include <utils/FixedCapacityVector.h>

using namespace bluevk;

namespace filament::backend {

struct VulkanSwapChain : public HwSwapChain {
    // The *fallbackExtent* parameter is for the case where the extent returned by the physical
    // surface is 0xFFFFFFFF.
    VulkanSwapChain(VkDevice device, VkPhysicalDevice physicalDevice,
	    uint32_t graphicsQueueFamilyIndex, VkQueue graphicsQueue, VmaAllocator allocator,
	    std::shared_ptr<VulkanCommands> commands, VulkanContext const& context,
	    VulkanStagePool& stagePool, VkSurfaceKHR vksurface,
	    VkExtent2D fallbackExtent = {.width = 640, .height = 320});

    // Headless constructor.
    VulkanSwapChain(VkDevice device, VkPhysicalDevice physicalDevice,
	    uint32_t graphicsQueueFamilyIndex, VkQueue graphicsQueue, VmaAllocator allocator,
	    std::shared_ptr<VulkanCommands> commands, VulkanContext const& context,
	    VulkanStagePool& stagePool, uint32_t width, uint32_t height);

    bool acquire();
    void destroy();
    void recreate();
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
    void create();

    VkDevice mDevice;
    VkPhysicalDevice mPhysicalDevice;
    std::shared_ptr<VulkanCommands> mCommands;
    VmaAllocator mAllocator;
    VulkanContext const& mContext;
    VulkanStagePool& mStagePool;
    uint32_t mCurrentSwapIndex = 0u;
    const VkExtent2D mFallbackExtent = {};

    // Color attachments are swapped, but depth is not. Typically there are 2 or 3 color attachments
    // in a swap chain.
    utils::FixedCapacityVector<std::unique_ptr<VulkanTexture>> mColor;
    std::unique_ptr<VulkanTexture> mDepth;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANSWAPCHAIN_H
