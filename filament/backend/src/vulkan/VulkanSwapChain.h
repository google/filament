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

 #ifndef TNT_FILAMENT_DRIVER_VULKANSWAPCHAIN_H
 #define TNT_FILAMENT_DRIVER_VULKANSWAPCHAIN_H

#include "VulkanContext.h"
#include "VulkanDriver.h"

#include <utils/FixedCapacityVector.h>

namespace filament {
namespace backend {

struct VulkanSwapChain : public HwSwapChain {
    VulkanSwapChain(VulkanContext& context, VkSurfaceKHR vksurface);
    VulkanSwapChain(VulkanContext& context, uint32_t width, uint32_t height);

    bool acquire();
    void create();
    void destroy();
    void makePresentable();
    bool hasResized() const;
    VulkanAttachment& getColor() { return color[currentSwapIndex]; }

    VulkanContext& context;
    VkSurfaceKHR surface = {};
    VkSwapchainKHR swapchain = {};
    VkSurfaceFormatKHR surfaceFormat = {};
    VkExtent2D clientSize = {};
    VkQueue presentQueue = {};
    VkQueue headlessQueue = {};
    uint32_t currentSwapIndex = {};

    // Color attachments are swapped, but depth is not. Typically there are 2 or 3 color attachments
    // in a swap chain.
    utils::FixedCapacityVector<VulkanAttachment> color;
    VulkanAttachment depth = {};

    // This is signaled when vkAcquireNextImageKHR succeeds, and is waited on by the first
    // submission.
    VkSemaphore imageAvailable = {};

    // This is true after the swap chain image has been acquired, but before it has been presented.
    bool acquired = false;

    bool suboptimal = false;
    bool firstRenderPass = false;
};

} // namespace filament
} // namespace backend

#endif // TNT_FILAMENT_DRIVER_VULKANTEXTURE_H
