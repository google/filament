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

#ifndef TNT_FILAMENT_BACKEND_VULKANCONTEXT_H
#define TNT_FILAMENT_BACKEND_VULKANCONTEXT_H

#include "VulkanPipelineCache.h"
#include "VulkanCommands.h"
#include "VulkanConstants.h"

#include <utils/bitset.h>
#include <utils/Slice.h>
#include <utils/Mutex.h>

VK_DEFINE_HANDLE(VmaAllocator)
VK_DEFINE_HANDLE(VmaPool)

namespace filament::backend {

struct VulkanRenderTarget;
struct VulkanSwapChain;
struct VulkanTexture;
class VulkanStagePool;

struct VulkanAttachment {
    VulkanTexture* texture = nullptr;
    uint8_t level = 0;
    uint16_t layer = 0;
    VkImage getImage() const;
    VkFormat getFormat() const;
    VkImageLayout getLayout() const;
    VkExtent2D getExtent2D() const;
    VkImageView getImageView(VkImageAspectFlags aspect) const;
};

struct VulkanTimestamps {
    VkQueryPool pool;
    utils::bitset32 used;
    utils::Mutex mutex;
};

struct VulkanRenderPass {
    VulkanRenderTarget* renderTarget;
    VkRenderPass renderPass;
    RenderPassParams params;
    int currentSubpass;
};

// For now we only support a single-device, single-instance scenario. Our concept of "context" is a
// bundle of state containing the Device, the Instance, and various globally-useful Vulkan objects.
struct VulkanContext {
public:
    void initialize(const char* const* ppRequiredExtensions, uint32_t requiredExtensionCount);
    void createEmptyTexture(VulkanStagePool& stagePool);
    uint32_t selectMemoryType(uint32_t flags, VkFlags reqs) const;

private:
    void afterSelectPhysicalDevice();
    void afterCreateLogicalDevice();
    void afterCreateInstance();

public:
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
    bool debugMarkersSupported = false;
    bool debugUtilsSupported = false;
    bool portabilitySubsetSupported = false;
    bool portabilityEnumerationSupported = false;
    bool maintenanceSupported[3] = {};
    VulkanPipelineCache::RasterState rasterState;
    VulkanSwapChain* currentSwapChain;
    VulkanRenderTarget* defaultRenderTarget;
    VulkanRenderPass currentRenderPass;
    VkViewport viewport;
    VkFormat finalDepthFormat;
    VmaAllocator allocator;
    VulkanTexture* emptyTexture = nullptr;
    VulkanCommands* commands = nullptr;
    VkDebugReportCallbackEXT debugCallback = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

    std::string currentDebugMarker;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANCONTEXT_H
