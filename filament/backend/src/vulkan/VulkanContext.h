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

#ifndef TNT_FILAMENT_DRIVER_VULKANCONTEXT_H
#define TNT_FILAMENT_DRIVER_VULKANCONTEXT_H

#include "VulkanBinder.h"
#include "VulkanCommands.h"
#include "VulkanDisposer.h"

#include <backend/DriverEnums.h>

#include <bluevk/BlueVK.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundef"
#include "vk_mem_alloc.h"
#pragma clang diagnostic pop

#include <utils/Condition.h>
#include <utils/Mutex.h>

#include <atomic>
#include <memory>
#include <vector>

namespace filament {
namespace backend {

// All vkCreate* functions take an optional allocator. For now we select the default allocator by
// passing in a null pointer, and we highlight the argument by using the VKALLOC constant.
constexpr VkAllocationCallbacks* VKALLOC = nullptr;

// At the time of this writing, our copy of MoltenVK supports Vulkan 1.0 only.
constexpr static const int VK_REQUIRED_VERSION_MAJOR = 1;
constexpr static const int VK_REQUIRED_VERSION_MINOR = 0;

struct VulkanRenderTarget;
struct VulkanSurfaceContext;
struct VulkanTexture;
class VulkanStagePool;

struct VulkanTimestamps {
    VkQueryPool pool;
    utils::bitset32 used;
    utils::Mutex mutex;
};

struct VulkanRenderPass {
    VkRenderPass renderPass;
    uint32_t subpassMask;
    int currentSubpass;
};

// For now we only support a single-device, single-instance scenario. Our concept of "context" is a
// bundle of state containing the Device, the Instance, and various globally-useful Vulkan objects.
struct VulkanContext {
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkPhysicalDeviceProperties physicalDeviceProperties;
    VkPhysicalDeviceFeatures physicalDeviceFeatures;
    VkPhysicalDeviceMemoryProperties memoryProperties;
    VkDevice device;
    VkCommandPool commandPool;
    VulkanTimestamps timestamps;
    uint32_t graphicsQueueFamilyIndex;
    VkQueue graphicsQueue;
    bool debugMarkersSupported;
    bool debugUtilsSupported;
    bool portabilitySubsetSupported;
    VulkanBinder::RasterState rasterState;
    VulkanSurfaceContext* currentSurface;
    VulkanRenderPass currentRenderPass;
    VkViewport viewport;
    VkFormat finalDepthFormat;
    VmaAllocator allocator;
    VulkanTexture* emptyTexture = nullptr;
    VulkanCommands* commands = nullptr;
};

struct VulkanAttachment {
    VkFormat format;
    VkImage image;
    VkImageView view;
    VkDeviceMemory memory;
    VulkanTexture* texture = nullptr;
    VkImageLayout layout;
    uint8_t level;
    uint16_t layer;
};

// The SurfaceContext stores various state (including the swap chain) that we tightly associate
// with VkSurfaceKHR, which is basically one-to-one with a platform-specific window.
struct VulkanSurfaceContext {
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    VkSurfaceFormatKHR surfaceFormat;
    VkExtent2D clientSize;
    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    VkQueue presentQueue;
    VkQueue headlessQueue;
    std::vector<VulkanAttachment> attachments;
    uint32_t currentSwapIndex;

    // This is signaled when vkAcquireNextImageKHR succeeds, and is waited on by the first
    // submission.
    VkSemaphore imageAvailable;

    // This is true after the swap chain image has been acquired, but before it has been presented.
    bool acquired;

    VulkanAttachment depth;
    bool suboptimal;
    bool firstRenderPass;
};

void selectPhysicalDevice(VulkanContext& context);
void createLogicalDevice(VulkanContext& context);
void getPresentationQueue(VulkanContext& context, VulkanSurfaceContext& sc);
void getHeadlessQueue(VulkanContext& context, VulkanSurfaceContext& sc);

void createSwapChain(VulkanContext& context, VulkanSurfaceContext& sc);
void destroySwapChain(VulkanContext& context, VulkanSurfaceContext& sc, VulkanDisposer& disposer);
void makeSwapChainPresentable(VulkanContext& context, VulkanSurfaceContext& surface);

uint32_t selectMemoryType(VulkanContext& context, uint32_t flags, VkFlags reqs);
VulkanAttachment& getSwapChainAttachment(VulkanContext& context);
void waitForIdle(VulkanContext& context);
bool acquireSwapChain(VulkanContext& context, VulkanSurfaceContext& surface);
VkFormat findSupportedFormat(VulkanContext& context, const std::vector<VkFormat>& candidates,
        VkImageTiling tiling, VkFormatFeatureFlags features);
void createFinalDepthBuffer(VulkanContext& context, VulkanSurfaceContext& sc, VkFormat depthFormat);
VkImageLayout getTextureLayout(TextureUsage usage);
void createEmptyTexture(VulkanContext& context, VulkanStagePool& stagePool);

} // namespace filament
} // namespace backend

#endif // TNT_FILAMENT_DRIVER_VULKANCONTEXT_H
