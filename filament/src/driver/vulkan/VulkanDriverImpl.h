/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef TNT_FILAMENT_DRIVER_VULKANDRIVER_IMPL_H
#define TNT_FILAMENT_DRIVER_VULKANDRIVER_IMPL_H

#include "VulkanBinder.h"

#include <filament/driver/DriverEnums.h>

#include <bluevk/BlueVK.h>

#include <utils/compiler.h>

#include "vk_mem_alloc.h"

#include <vector>
#include <functional>

namespace filament {
namespace driver {

// All vkCreate* functions take an optional allocator. For now we select the default allocator by
// passing in a null pointer, and we highlight the argument by using the VKALLOC constant.
static constexpr VkAllocationCallbacks* VKALLOC = nullptr;

using VulkanTask = std::function<void(VkCommandBuffer)>;
using VulkanTaskQueue = std::vector<VulkanTask>;

struct VulkanSurfaceContext;

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
    uint32_t graphicsQueueFamilyIndex;
    VkQueue graphicsQueue;
    bool debugMarkersSupported;
    VulkanTaskQueue pendingWork;
    VulkanBinder::RasterState rasterState;
    VkCommandBuffer cmdbuffer;
    VulkanSurfaceContext* currentSurface;
    VkRenderPassBeginInfo currentRenderPass;
    VkViewport viewport;
    VkFormat depthFormat;
    VmaAllocator allocator;
};

struct VulkanAttachment {
    VkFormat format;
    VkImage image;
    VkImageView view;
    VkDeviceMemory memory;
};

// The SwapContext is the set of objects that gets "swapped" at each beginFrame().
// Typically there are only 2 or 3 instances of the SwapContext per SwapChain.
struct SwapContext {
    VulkanAttachment attachment;
    VkCommandBuffer cmdbuffer;
    VkFence fence;
    VulkanTaskQueue pendingWork;
    bool submitted;
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
    std::vector<SwapContext> swapContexts;
    uint32_t currentSwapIndex;
    VulkanAttachment depth;
    VkSemaphore imageAvailable;
    VkSemaphore renderingFinished;
};

void selectPhysicalDevice(VulkanContext& context);
void createVirtualDevice(VulkanContext& context);
void createSemaphore(VkDevice device, VkSemaphore* semaphore);
void getPresentationQueue(VulkanContext& context, VulkanSurfaceContext& sc);
void getSurfaceCaps(VulkanContext& context, VulkanSurfaceContext& sc);
void createSwapChainAndImages(VulkanContext& context, VulkanSurfaceContext& sc);
void createDepthBuffer(VulkanContext& context, VulkanSurfaceContext& sc, VkFormat depthFormat);
void transitionDepthBuffer(VulkanContext& context, VulkanSurfaceContext& sc, VkFormat depthFormat);
void createCommandBuffersAndFences(VulkanContext& context, VulkanSurfaceContext& sc);
void destroySurfaceContext(VulkanContext& context, VulkanSurfaceContext& sc);
uint32_t selectMemoryType(VulkanContext& context, uint32_t flags, VkFlags reqs);
VkFormat getVkFormat(ElementType type, bool normalized);
VkFormat getVkFormat(TextureFormat format);
uint32_t getBytesPerPixel(TextureFormat format);
SwapContext& getSwapContext(VulkanContext& context);
bool hasPendingWork(VulkanContext& context);
VkCompareOp getCompareOp(SamplerCompareFunc func);
VkBlendFactor getBlendFactor(BlendFunction mode);
VkCullModeFlags getCullMode(CullingMode mode);
VkFrontFace getFrontFace(bool inverseFrontFaces);
void waitForIdle(VulkanContext& context);
void acquireCommandBuffer(VulkanContext& context);
void releaseCommandBuffer(VulkanContext& context);
void performPendingWork(VulkanContext& context, SwapContext& swapContext, VkCommandBuffer cmdbuf);
void flushCommandBuffer(VulkanContext& context);
VkFormat findSupportedFormat(VulkanContext& context, const std::vector<VkFormat>& candidates,
        VkImageTiling tiling, VkFormatFeatureFlags features);

} // namespace filament
} // namespace driver

#endif // TNT_FILAMENT_DRIVER_VULKANDRIVER_IMPL_H
