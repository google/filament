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
#include "VulkanDisposer.h"

#include <backend/DriverEnums.h>

#include <bluevk/BlueVK.h>

#include <utils/compiler.h>

#include "vk_mem_alloc.h"

#include <utils/Condition.h>
#include <utils/Mutex.h>

#include <memory>
#include <vector>

namespace filament {
namespace backend {

// All vkCreate* functions take an optional allocator. For now we select the default allocator by
// passing in a null pointer, and we highlight the argument by using the VKALLOC constant.
constexpr VkAllocationCallbacks* VKALLOC = nullptr;

struct VulkanSurfaceContext;
struct VulkanTexture;

// This wrapper exists so that we can use shared_ptr to implement shared ownership for low-level
// Vulkan fences.
struct VulkanCmdFence {
    VulkanCmdFence(VkDevice device, bool signaled = false);
    ~VulkanCmdFence();
    const VkDevice device;
    VkFence fence;
    utils::Condition condition;
    utils::Mutex mutex;
    bool submitted = false;
};

 // The submission fence has shared ownership semantics because it is potentially wrapped by a
// DriverApi fence object and should not be destroyed until both the DriverAPI object is freed and
// we're done waiting on the most recent submission of the given command buffer.
struct VulkanCommandBuffer {
    VkCommandBuffer cmdbuffer;
    std::shared_ptr<VulkanCmdFence> fence;
    VulkanDisposer::Set resources;
};

struct VulkanTimestamps {
    VkQueryPool pool;
    utils::bitset32 used;
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
    VulkanBinder::RasterState rasterState;
    VulkanCommandBuffer* currentCommands;
    VulkanSurfaceContext* currentSurface;
    VkRenderPassBeginInfo currentRenderPass;
    VkViewport viewport;
    VkFormat depthFormat;
    VmaAllocator allocator;

    // The work context is used for activities unrelated to the swap chain or draw calls, such as
    // uploads, blits, and transitions.
    VulkanCommandBuffer work;
};

struct VulkanAttachment {
    VkFormat format;
    VkImage image;
    VkImageView view;
    VkDeviceMemory memory;
    VulkanTexture* offscreen = nullptr;
};

// The SwapContext is the set of objects that gets "swapped" at each beginFrame().
// Typically there are only 2 or 3 instances of the SwapContext per SwapChain.
struct SwapContext {
    VulkanAttachment attachment;
    VulkanCommandBuffer commands;
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
    VkSemaphore imageAvailable;
    VkSemaphore renderingFinished;
    VulkanAttachment depth;
    void* nativeWindow;
};

void selectPhysicalDevice(VulkanContext& context);
void createVirtualDevice(VulkanContext& context);
void getPresentationQueue(VulkanContext& context, VulkanSurfaceContext& sc);
void getSurfaceCaps(VulkanContext& context, VulkanSurfaceContext& sc);

void createSwapChain(VulkanContext& context, VulkanSurfaceContext& sc);
void destroySwapChain(VulkanContext& context, VulkanSurfaceContext& sc, VulkanDisposer& disposer);

uint32_t selectMemoryType(VulkanContext& context, uint32_t flags, VkFlags reqs);
SwapContext& getSwapContext(VulkanContext& context);
void waitForIdle(VulkanContext& context);
bool acquireSwapCommandBuffer(VulkanContext& context);
void releaseCommandBuffer(VulkanContext& context);
void flushCommandBuffer(VulkanContext& context);
VkFormat findSupportedFormat(VulkanContext& context, const std::vector<VkFormat>& candidates,
        VkImageTiling tiling, VkFormatFeatureFlags features);
VkCommandBuffer acquireWorkCommandBuffer(VulkanContext& context);
void flushWorkCommandBuffer(VulkanContext& context);
void createDepthBuffer(VulkanContext& context, VulkanSurfaceContext& sc, VkFormat depthFormat);
VkImageLayout getTextureLayout(TextureUsage usage);

} // namespace filament
} // namespace backend

#endif // TNT_FILAMENT_DRIVER_VULKANCONTEXT_H
