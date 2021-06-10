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

#include "VulkanPipelineCache.h"
#include "VulkanCommands.h"
#include "VulkanConstants.h"
#include "VulkanDisposer.h"

#include <backend/DriverEnums.h>

#include <bluevk/BlueVK.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundef"
#include "vk_mem_alloc.h"
#pragma clang diagnostic pop

#include <utils/Condition.h>
#include <utils/Mutex.h>

#include <memory>
#include <vector>

namespace filament {
namespace backend {

struct VulkanRenderTarget;
struct VulkanSwapChain;
struct VulkanTexture;
class VulkanStagePool;

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

struct VulkanTimestamps {
    VkQueryPool pool;
    utils::bitset32 used;
    utils::Mutex mutex;
};

struct VulkanRenderPass {
    VkRenderPass renderPass;
    uint32_t subpassMask;
    int currentSubpass;
    VulkanTexture* depthFeedback;
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
    bool maintenanceSupported[3];
    VulkanPipelineCache::RasterState rasterState;
    VulkanSwapChain* currentSurface;
    VulkanRenderPass currentRenderPass;
    VkViewport viewport;
    VkFormat finalDepthFormat;
    VmaAllocator allocator;
    VulkanTexture* emptyTexture = nullptr;
    VulkanCommands* commands = nullptr;
    std::string currentDebugMarker;
};

void selectPhysicalDevice(VulkanContext& context);
void createLogicalDevice(VulkanContext& context);

uint32_t selectMemoryType(VulkanContext& context, uint32_t flags, VkFlags reqs);
VulkanAttachment& getSwapChainAttachment(VulkanContext& context);
void waitForIdle(VulkanContext& context);
VkFormat findSupportedFormat(VulkanContext& context, const std::vector<VkFormat>& candidates,
        VkImageTiling tiling, VkFormatFeatureFlags features);
VkImageLayout getTextureLayout(TextureUsage usage);
void createEmptyTexture(VulkanContext& context, VulkanStagePool& stagePool);

} // namespace filament
} // namespace backend

#endif // TNT_FILAMENT_DRIVER_VULKANCONTEXT_H
