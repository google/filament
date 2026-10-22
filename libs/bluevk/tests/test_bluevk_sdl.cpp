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

 /**********************************************************************************************
  * This has been forked from SDL's vulkan_test by Sam Lantinga, and heavily modified.
  **********************************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "SDL.h"
#include "SDL_vulkan.h"

#include <bluevk/BlueVK.h>

#include <utils/Log.h>

#include <math/scalar.h>

using namespace filament::math;
using namespace bluevk;

struct VulkanDriver {
    VkInstance instance;
    VkDevice device;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    VkPhysicalDeviceProperties physicalDeviceProperties;
    VkPhysicalDeviceFeatures physicalDeviceFeatures;
    uint32_t graphicsQueueFamilyIndex;
    uint32_t presentQueueFamilyIndex;
    VkPhysicalDevice physicalDevice;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderingFinishedSemaphore;
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    VkSurfaceFormatKHR *surfaceFormats;
    uint32_t surfaceFormatsAllocatedCount;
    uint32_t surfaceFormatsCount;
    uint32_t swapchainDesiredImageCount;
    VkSurfaceFormatKHR surfaceFormat;
    VkExtent2D swapchainSize;
    VkCommandPool commandPool;
    uint32_t swapchainImageCount;
    VkImage *swapchainImages;
    VkCommandBuffer *commandBuffers;
    VkFence *fences;
};

static SDL_Window* gWindow = nullptr;
static VulkanDriver gVulkanDriver = {0};

static void destroySwapchainThings(SDL_bool doDestroySwapchain);

static void quit(int rc) {
    if (gVulkanDriver.device && vkDeviceWaitIdle) {
        vkDeviceWaitIdle(gVulkanDriver.device);
    }
    destroySwapchainThings(SDL_TRUE);
    if (gVulkanDriver.imageAvailableSemaphore && vkDestroySemaphore) {
        vkDestroySemaphore(gVulkanDriver.device, gVulkanDriver.imageAvailableSemaphore, NULL);
    }
    if (gVulkanDriver.renderingFinishedSemaphore && vkDestroySemaphore) {
        vkDestroySemaphore(gVulkanDriver.device, gVulkanDriver.renderingFinishedSemaphore, NULL);
    }
    if (gVulkanDriver.device && vkDestroyDevice) {
        vkDestroyDevice(gVulkanDriver.device, NULL);
    }
    if (gVulkanDriver.surface && vkDestroySurfaceKHR) {
        vkDestroySurfaceKHR(gVulkanDriver.instance, gVulkanDriver.surface, NULL);
    }
    if (gVulkanDriver.instance && vkDestroyInstance) {
        vkDestroyInstance(gVulkanDriver.instance, NULL);
    }
    SDL_free(gVulkanDriver.surfaceFormats);
    exit(rc);
}

static void createInstance() {
    VkApplicationInfo appInfo = {};
    VkInstanceCreateInfo instanceCreateInfo = {};
    const char **extensions = NULL;
    unsigned extensionCount = 0;
    VkResult result;

    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = VK_API_VERSION_1_0;
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    if (!SDL_Vulkan_GetInstanceExtensions(gWindow, &extensionCount, NULL)) {
        utils::slog.e << "SDL_Vulkan_GetInstanceExtensions(): "
                << SDL_GetError() << utils::io::endl;
        quit(2);
    }
    extensions = (const char**) SDL_malloc(sizeof(const char *) * extensionCount);
    if (!extensions) {
        SDL_OutOfMemory();
        quit(2);
    }
    if (!SDL_Vulkan_GetInstanceExtensions(gWindow, &extensionCount, extensions)) {
        SDL_free((void*)extensions);
        utils::slog.e << "SDL_Vulkan_GetInstanceExtensions(): "
                << SDL_GetError() << utils::io::endl;
        quit(2);
    }
    instanceCreateInfo.enabledExtensionCount = extensionCount;
    instanceCreateInfo.ppEnabledExtensionNames = extensions;
    result = vkCreateInstance(&instanceCreateInfo, NULL, &gVulkanDriver.instance);
    SDL_free((void*)extensions);

    if (result != VK_SUCCESS) {
        gVulkanDriver.instance = VK_NULL_HANDLE;
        utils::slog.e << "vkCreateInstance(): " << result << utils::io::endl;
        quit(2);
    }
}

static void createSurface() {
    if (!SDL_Vulkan_CreateSurface(gWindow, gVulkanDriver.instance, &gVulkanDriver.surface)) {
        gVulkanDriver.surface = VK_NULL_HANDLE;
        utils::slog.e << "SDL_Vulkan_CreateSurface(): "
                << SDL_GetError() << utils::io::endl;
        quit(2);
    }
}

static void findPhysicalDevice() {
    uint32_t physicalDeviceCount = 0;
    VkPhysicalDevice *physicalDevices;
    VkQueueFamilyProperties *queueFamiliesProperties = NULL;
    uint32_t queueFamiliesPropertiesAllocatedSize = 0;
    VkExtensionProperties *deviceExtensions = NULL;
    uint32_t deviceExtensionsAllocatedSize = 0;
    uint32_t physicalDeviceIdx;

    VkResult result =
            vkEnumeratePhysicalDevices(gVulkanDriver.instance, &physicalDeviceCount, NULL);
    if (result != VK_SUCCESS) {
        utils::slog.e << "vkEnumeratePhysicalDevices(): " << result << utils::io::endl;
        quit(2);
    }
    if (physicalDeviceCount == 0) {
        utils::slog.e << "vkEnumeratePhysicalDevices(): no physical devices\n";
        quit(2);
    }
    physicalDevices = (VkPhysicalDevice*) SDL_malloc(sizeof(VkPhysicalDevice) *
            physicalDeviceCount);
    if (!physicalDevices) {
        SDL_OutOfMemory();
        quit(2);
    }
    result =
        vkEnumeratePhysicalDevices(gVulkanDriver.instance, &physicalDeviceCount, physicalDevices);
    if (result != VK_SUCCESS) {
        SDL_free(physicalDevices);
        utils::slog.e << "vkEnumeratePhysicalDevices(): " << result << utils::io::endl;
        quit(2);
    }
    gVulkanDriver.physicalDevice = NULL;
    for (physicalDeviceIdx = 0; physicalDeviceIdx < physicalDeviceCount; physicalDeviceIdx++) {
        uint32_t queueFamiliesCount = 0;
        uint32_t queueFamilyIndex;
        uint32_t deviceExtensionCount = 0;
        SDL_bool hasSwapchainExtension = SDL_FALSE;
        uint32_t i;

        VkPhysicalDevice physicalDevice = physicalDevices[physicalDeviceIdx];
        vkGetPhysicalDeviceProperties(physicalDevice, &gVulkanDriver.physicalDeviceProperties);
        if (VK_VERSION_MAJOR(gVulkanDriver.physicalDeviceProperties.apiVersion) < 1) {
            continue;
        }
        vkGetPhysicalDeviceFeatures(physicalDevice, &gVulkanDriver.physicalDeviceFeatures);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamiliesCount, NULL);
        if (queueFamiliesCount == 0) {
            continue;
        }
        if (queueFamiliesPropertiesAllocatedSize < queueFamiliesCount) {
            SDL_free(queueFamiliesProperties);
            queueFamiliesPropertiesAllocatedSize = queueFamiliesCount;
            queueFamiliesProperties = (VkQueueFamilyProperties*)
                SDL_malloc(sizeof(VkQueueFamilyProperties) * queueFamiliesPropertiesAllocatedSize);
            if (!queueFamiliesProperties) {
                SDL_free(physicalDevices);
                SDL_free(deviceExtensions);
                SDL_OutOfMemory();
                quit(2);
            }
        }
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamiliesCount,
                queueFamiliesProperties);
        gVulkanDriver.graphicsQueueFamilyIndex = queueFamiliesCount;
        gVulkanDriver.presentQueueFamilyIndex = queueFamiliesCount;
        for (queueFamilyIndex = 0; queueFamilyIndex < queueFamiliesCount; queueFamilyIndex++) {
            VkBool32 supported = 0;
            if (queueFamiliesProperties[queueFamilyIndex].queueCount == 0) {
                continue;
            }
            if (queueFamiliesProperties[queueFamilyIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                gVulkanDriver.graphicsQueueFamilyIndex = queueFamilyIndex;
            }
            result = vkGetPhysicalDeviceSurfaceSupportKHR(
                physicalDevice, queueFamilyIndex, gVulkanDriver.surface, &supported);
            if (result != VK_SUCCESS) {
                SDL_free(physicalDevices);
                SDL_free(queueFamiliesProperties);
                SDL_free(deviceExtensions);
                utils::slog.e << "vkGetPhysicalDeviceSurfaceSupportKHR(): "
                        << result << utils::io::endl;
                quit(2);
            }
            if (supported) {
                gVulkanDriver.presentQueueFamilyIndex = queueFamilyIndex;
                if (queueFamiliesProperties[queueFamilyIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    break;
                }
            }
        }
        if (gVulkanDriver.graphicsQueueFamilyIndex == queueFamiliesCount) continue;
        if (gVulkanDriver.presentQueueFamilyIndex == queueFamiliesCount) continue;
        result =
            vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &deviceExtensionCount, NULL);
        if (result != VK_SUCCESS) {
            SDL_free(physicalDevices);
            SDL_free(queueFamiliesProperties);
            SDL_free(deviceExtensions);
            utils::slog.e << "vkEnumerateDeviceExtensionProperties(): " << result
                    << utils::io::endl;
            quit(2);
        }
        if (deviceExtensionCount == 0) continue;
        if (deviceExtensionsAllocatedSize < deviceExtensionCount) {
            SDL_free(deviceExtensions);
            deviceExtensionsAllocatedSize = deviceExtensionCount;
            deviceExtensions = (VkExtensionProperties*)
                SDL_malloc(sizeof(VkExtensionProperties) * deviceExtensionsAllocatedSize);
            if (!deviceExtensions) {
                SDL_free(physicalDevices);
                SDL_free(queueFamiliesProperties);
                SDL_OutOfMemory();
                quit(2);
            }
        }
        result = vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &deviceExtensionCount,
                deviceExtensions);
        if (result != VK_SUCCESS) {
            SDL_free(physicalDevices);
            SDL_free(queueFamiliesProperties);
            SDL_free(deviceExtensions);
            utils::slog.e << "vkEnumerateDeviceExtensionProperties(): " << result
                    << utils::io::endl;
            quit(2);
        }
        for (i = 0; i < deviceExtensionCount; i++) {
            if (!SDL_strcmp(deviceExtensions[i].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME)) {
                hasSwapchainExtension = SDL_TRUE;
                break;
            }
        }
        if (!hasSwapchainExtension) continue;
        gVulkanDriver.physicalDevice = physicalDevice;
        break;
    }
    SDL_free(physicalDevices);
    SDL_free(queueFamiliesProperties);
    SDL_free(deviceExtensions);
    if (!gVulkanDriver.physicalDevice) {
        utils::slog.e << "Vulkan: no viable physical devices found\n";
        quit(2);
    }
}

static void createDevice() {
    VkDeviceQueueCreateInfo deviceQueueCreateInfo[1] = {};
    static const float queuePriority[] = {1.0f};
    VkDeviceCreateInfo deviceCreateInfo = {};
    static const char *const deviceExtensionNames[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
    VkResult result;
    deviceQueueCreateInfo->sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfo->queueFamilyIndex = gVulkanDriver.graphicsQueueFamilyIndex;
    deviceQueueCreateInfo->queueCount = 1;
    deviceQueueCreateInfo->pQueuePriorities = &queuePriority[0];
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfo;
    deviceCreateInfo.pEnabledFeatures = NULL;
    deviceCreateInfo.enabledExtensionCount = SDL_arraysize(deviceExtensionNames);
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensionNames;
    result = vkCreateDevice(
        gVulkanDriver.physicalDevice, &deviceCreateInfo, NULL, &gVulkanDriver.device);
    if (result != VK_SUCCESS) {
        gVulkanDriver.device = VK_NULL_HANDLE;
        utils::slog.e << "vkCreateDevice(): " << result << utils::io::endl;
        quit(2);
    }
}

static void getQueues() {
    vkGetDeviceQueue(gVulkanDriver.device, gVulkanDriver.graphicsQueueFamilyIndex, 0,
            &gVulkanDriver.graphicsQueue);
    if (gVulkanDriver.graphicsQueueFamilyIndex != gVulkanDriver.presentQueueFamilyIndex) {
        vkGetDeviceQueue(gVulkanDriver.device, gVulkanDriver.presentQueueFamilyIndex, 0,
                &gVulkanDriver.presentQueue);
    } else {
        gVulkanDriver.presentQueue = gVulkanDriver.graphicsQueue;
    }
}

static void createSemaphore(VkSemaphore *semaphore) {
    VkResult result;
    VkSemaphoreCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    result = vkCreateSemaphore(gVulkanDriver.device, &createInfo, NULL, semaphore);
    if (result != VK_SUCCESS) {
        *semaphore = VK_NULL_HANDLE;
        utils::slog.e << "vkCreateSemaphore(): " << result << utils::io::endl;
        quit(2);
    }
}

static void createSemaphores() {
    createSemaphore(&gVulkanDriver.imageAvailableSemaphore);
    createSemaphore(&gVulkanDriver.renderingFinishedSemaphore);
}

static void getSurfaceCaps() {
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        gVulkanDriver.physicalDevice, gVulkanDriver.surface, &gVulkanDriver.surfaceCapabilities);
    if (result != VK_SUCCESS) {
        utils::slog.e << "vkGetPhysicalDeviceSurfaceCapabilitiesKHR(): " << result
                << utils::io::endl;
        quit(2);
    }
    if (!(gVulkanDriver.surfaceCapabilities.supportedUsageFlags &
            VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
        utils::slog.e << "Vulkan surface doesn't support VK_IMAGE_USAGE_TRANSFER_DST_BIT\n";
        quit(2);
    }
}

static void getSurfaceFormats() {
    VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(gVulkanDriver.physicalDevice,
            gVulkanDriver.surface, &gVulkanDriver.surfaceFormatsCount, NULL);
    if (result != VK_SUCCESS) {
        gVulkanDriver.surfaceFormatsCount = 0;
        utils::slog.e << "vkGetPhysicalDeviceSurfaceFormatsKHR(): " << result << utils::io::endl;
        quit(2);
    }
    if (gVulkanDriver.surfaceFormatsCount > gVulkanDriver.surfaceFormatsAllocatedCount) {
        gVulkanDriver.surfaceFormatsAllocatedCount = gVulkanDriver.surfaceFormatsCount;
        SDL_free(gVulkanDriver.surfaceFormats);
        gVulkanDriver.surfaceFormats = (VkSurfaceFormatKHR *)
            SDL_malloc(sizeof(VkSurfaceFormatKHR) * gVulkanDriver.surfaceFormatsAllocatedCount);
        if (!gVulkanDriver.surfaceFormats) {
            gVulkanDriver.surfaceFormatsCount = 0;
            SDL_OutOfMemory();
            quit(2);
        }
    }
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(gVulkanDriver.physicalDevice,
            gVulkanDriver.surface, &gVulkanDriver.surfaceFormatsCount,
            gVulkanDriver.surfaceFormats);
    if (result != VK_SUCCESS) {
        gVulkanDriver.surfaceFormatsCount = 0;
        utils::slog.e << "vkGetPhysicalDeviceSurfaceFormatsKHR(): " << result << utils::io::endl;
        quit(2);
    }
}

static void getSwapchainImages() {
    VkResult result;
    SDL_free(gVulkanDriver.swapchainImages);
    gVulkanDriver.swapchainImages = NULL;
    result = vkGetSwapchainImagesKHR(
        gVulkanDriver.device, gVulkanDriver.swapchain, &gVulkanDriver.swapchainImageCount, NULL);
    if (result != VK_SUCCESS) {
        gVulkanDriver.swapchainImageCount = 0;
        utils::slog.e << "vkGetSwapchainImagesKHR(): " << result << utils::io::endl;
        quit(2);
    }
    gVulkanDriver.swapchainImages = (VkImage*) SDL_malloc(sizeof(VkImage) *
            gVulkanDriver.swapchainImageCount);
    if (!gVulkanDriver.swapchainImages) {
        SDL_OutOfMemory();
        quit(2);
    }
    result = vkGetSwapchainImagesKHR(gVulkanDriver.device, gVulkanDriver.swapchain,
        &gVulkanDriver.swapchainImageCount, gVulkanDriver.swapchainImages);
    if (result != VK_SUCCESS) {
        SDL_free(gVulkanDriver.swapchainImages);
        gVulkanDriver.swapchainImages = NULL;
        gVulkanDriver.swapchainImageCount = 0;
        utils::slog.e << "vkGetSwapchainImagesKHR(): " << result << utils::io::endl;
        quit(2);
    }
}

static SDL_bool createSwapchain() {
    uint32_t i;
    int w, h;
    VkSwapchainCreateInfoKHR createInfo = {};
    VkResult result;

    // pick an image count
    gVulkanDriver.swapchainDesiredImageCount = gVulkanDriver.surfaceCapabilities.minImageCount + 1;
    if (gVulkanDriver.swapchainDesiredImageCount > gVulkanDriver.surfaceCapabilities.maxImageCount
            && gVulkanDriver.surfaceCapabilities.maxImageCount > 0) {
        gVulkanDriver.swapchainDesiredImageCount = gVulkanDriver.surfaceCapabilities.maxImageCount;
    }

    // pick a format
    if (gVulkanDriver.surfaceFormatsCount == 1
            && gVulkanDriver.surfaceFormats[0].format == VK_FORMAT_UNDEFINED) {
        // aren't any preferred formats, so we pick
        gVulkanDriver.surfaceFormat.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
        gVulkanDriver.surfaceFormat.format = VK_FORMAT_R8G8B8A8_UNORM;
    } else {
        gVulkanDriver.surfaceFormat = gVulkanDriver.surfaceFormats[0];
        for (i = 0; i < gVulkanDriver.surfaceFormatsCount; i++) {
            if (gVulkanDriver.surfaceFormats[i].format == VK_FORMAT_R8G8B8A8_UNORM) {
                gVulkanDriver.surfaceFormat = gVulkanDriver.surfaceFormats[i];
                break;
            }
        }
    }

    // get size
    SDL_Vulkan_GetDrawableSize(gWindow, &w, &h);
    gVulkanDriver.swapchainSize.width = w;
    gVulkanDriver.swapchainSize.height = h;
    if (w == 0 || h == 0) return SDL_FALSE;

    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = gVulkanDriver.surface;
    createInfo.minImageCount = gVulkanDriver.swapchainDesiredImageCount;
    createInfo.imageFormat = gVulkanDriver.surfaceFormat.format;
    createInfo.imageColorSpace = gVulkanDriver.surfaceFormat.colorSpace;
    createInfo.imageExtent = gVulkanDriver.swapchainSize;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.preTransform = gVulkanDriver.surfaceCapabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = gVulkanDriver.swapchain;
    result =
        vkCreateSwapchainKHR(gVulkanDriver.device, &createInfo, NULL, &gVulkanDriver.swapchain);
    if (createInfo.oldSwapchain)
        vkDestroySwapchainKHR(gVulkanDriver.device, createInfo.oldSwapchain, NULL);
    if (result != VK_SUCCESS) {
        gVulkanDriver.swapchain = VK_NULL_HANDLE;
        utils::slog.e << "vkCreateSwapchainKHR(): " << result << utils::io::endl;
        quit(2);
    }
    getSwapchainImages();
    return SDL_TRUE;
}

static void destroySwapchain() {
    if (gVulkanDriver.swapchain) {
        vkDestroySwapchainKHR(gVulkanDriver.device, gVulkanDriver.swapchain, NULL);
    }
    gVulkanDriver.swapchain = VK_NULL_HANDLE;
    SDL_free(gVulkanDriver.swapchainImages);
    gVulkanDriver.swapchainImages = NULL;
}

static void destroyCommandBuffers() {
    if (gVulkanDriver.commandBuffers) {
        vkFreeCommandBuffers(gVulkanDriver.device,
                             gVulkanDriver.commandPool,
                             gVulkanDriver.swapchainImageCount,
                             gVulkanDriver.commandBuffers);
    }
    SDL_free(gVulkanDriver.commandBuffers);
    gVulkanDriver.commandBuffers = NULL;
}

static void destroyCommandPool() {
    if (gVulkanDriver.commandPool) {
        vkDestroyCommandPool(gVulkanDriver.device, gVulkanDriver.commandPool, NULL);
    }
    gVulkanDriver.commandPool = VK_NULL_HANDLE;
}

static void createCommandPool() {
    VkResult result;

    VkCommandPoolCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.flags =
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    createInfo.queueFamilyIndex = gVulkanDriver.graphicsQueueFamilyIndex;
    result =
        vkCreateCommandPool(gVulkanDriver.device, &createInfo, NULL, &gVulkanDriver.commandPool);
    if (result != VK_SUCCESS) {
        gVulkanDriver.commandPool = VK_NULL_HANDLE;
        utils::slog.e << "vkCreateCommandPool(): " << result << utils::io::endl;
        quit(2);
    }
}

static void createCommandBuffers() {
    VkResult result;
    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = gVulkanDriver.commandPool;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = gVulkanDriver.swapchainImageCount;
    gVulkanDriver.commandBuffers = (VkCommandBuffer*)
        SDL_malloc(sizeof(VkCommandBuffer) * gVulkanDriver.swapchainImageCount);
    result =
        vkAllocateCommandBuffers(gVulkanDriver.device, &allocateInfo, gVulkanDriver.commandBuffers);
    if (result != VK_SUCCESS) {
        SDL_free(gVulkanDriver.commandBuffers);
        gVulkanDriver.commandBuffers = NULL;
        utils::slog.e << "vkAllocateCommandBuffers(): " << result << utils::io::endl;
        quit(2);
    }
}

static void createFences() {
    uint32_t i;
    gVulkanDriver.fences = (VkFence*) SDL_malloc(sizeof(VkFence) *
            gVulkanDriver.swapchainImageCount);
    if (!gVulkanDriver.fences) {
        SDL_OutOfMemory();
        quit(2);
    }
    for (i = 0; i < gVulkanDriver.swapchainImageCount; i++) {
        VkResult result;
        VkFenceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        result = vkCreateFence(gVulkanDriver.device, &createInfo, NULL, &gVulkanDriver.fences[i]);
        if (result != VK_SUCCESS) {
            for (; i > 0; i--) {
                vkDestroyFence(gVulkanDriver.device, gVulkanDriver.fences[i - 1], NULL);
            }
            SDL_free(gVulkanDriver.fences);
            gVulkanDriver.fences = NULL;
            utils::slog.e << "vkCreateFence(): " << result << utils::io::endl;
            quit(2);
        }
    }
}

static void destroyFences() {
    uint32_t i;
    if (!gVulkanDriver.fences) return;
    for (i = 0; i < gVulkanDriver.swapchainImageCount; i++) {
        vkDestroyFence(gVulkanDriver.device, gVulkanDriver.fences[i], NULL);
    }
    SDL_free(gVulkanDriver.fences);
    gVulkanDriver.fences = NULL;
}

static void recordPipelineImageBarrier(VkCommandBuffer commandBuffer,
                                       VkAccessFlags sourceAccessMask,
                                       VkAccessFlags destAccessMask,
                                       VkImageLayout sourceLayout,
                                       VkImageLayout destLayout,
                                       VkImage image) {
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.srcAccessMask = sourceAccessMask;
    barrier.dstAccessMask = destAccessMask;
    barrier.oldLayout = sourceLayout;
    barrier.newLayout = destLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    vkCmdPipelineBarrier(commandBuffer,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         0,
                         0,
                         NULL,
                         0,
                         NULL,
                         1,
                         &barrier);
}

static void rerecordCommandBuffer(uint32_t frameIndex, const VkClearColorValue *clearColor) {
    VkCommandBuffer commandBuffer = gVulkanDriver.commandBuffers[frameIndex];
    VkImage image = gVulkanDriver.swapchainImages[frameIndex];
    VkCommandBufferBeginInfo beginInfo = {};
    VkImageSubresourceRange clearRange = {};

    VkResult result = vkResetCommandBuffer(commandBuffer, 0);
    if (result != VK_SUCCESS) {
        utils::slog.e << "vkResetCommandBuffer(): " << result << utils::io::endl;
        quit(2);
    }
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    if (result != VK_SUCCESS) {
        utils::slog.e << "vkBeginCommandBuffer(): " << result << utils::io::endl;
        quit(2);
    }
    recordPipelineImageBarrier(commandBuffer,
                               0,
                               VK_ACCESS_TRANSFER_WRITE_BIT,
                               VK_IMAGE_LAYOUT_UNDEFINED,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               image);
    clearRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    clearRange.baseMipLevel = 0;
    clearRange.levelCount = 1;
    clearRange.baseArrayLayer = 0;
    clearRange.layerCount = 1;
    vkCmdClearColorImage(
        commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, clearColor, 1, &clearRange);
    recordPipelineImageBarrier(commandBuffer,
                               VK_ACCESS_TRANSFER_WRITE_BIT,
                               VK_ACCESS_MEMORY_READ_BIT,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                               image);
    result = vkEndCommandBuffer(commandBuffer);
    if (result != VK_SUCCESS) {
        utils::slog.e << "vkEndCommandBuffer(): " << result << utils::io::endl;
        quit(2);
    }
}

static void destroySwapchainThings(SDL_bool doDestroySwapchain) {
    destroyFences();
    destroyCommandBuffers();
    destroyCommandPool();
    if (doDestroySwapchain) destroySwapchain();
}

static SDL_bool createNewSwapchainThings() {
    destroySwapchainThings(SDL_FALSE);
    getSurfaceCaps();
    getSurfaceFormats();
    if (!createSwapchain()) {
        return SDL_FALSE;
    }
    createCommandPool();
    createCommandBuffers();
    createFences();
    return SDL_TRUE;
}

static void initVulkan() {
    if (!bluevk::initialize()) {
        utils::slog.e << "BlueVK is unable to load entry points.\n";
        quit(2);
    }
    createInstance();
    bluevk::bindInstance(gVulkanDriver.instance);
    createSurface();
    findPhysicalDevice();
    createDevice();
    getQueues();
    createSemaphores();
    createNewSwapchainThings();
}

static SDL_bool render() {
    const float SPEED = 4;
    uint32_t frameIndex;
    VkResult result;
    double currentTime;
    VkClearColorValue clearColor = {};
    VkPipelineStageFlags waitDestStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    VkSubmitInfo submitInfo = {};
    VkPresentInfoKHR presentInfo = {};
    int w, h;

    if (!gVulkanDriver.swapchain) {
        SDL_bool retval = createNewSwapchainThings();
        if (!retval) SDL_Delay(100);
        return retval;
    }

    result = vkAcquireNextImageKHR(gVulkanDriver.device, gVulkanDriver.swapchain, UINT64_MAX,
            gVulkanDriver.imageAvailableSemaphore, VK_NULL_HANDLE, &frameIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        return createNewSwapchainThings();
    }

    if (result != VK_SUBOPTIMAL_KHR && result != VK_SUCCESS) {
        utils::slog.e << "vkAcquireNextImageKHR(): " << result << utils::io::endl;
        quit(2);
    }

    result = vkWaitForFences(
            gVulkanDriver.device, 1, &gVulkanDriver.fences[frameIndex], VK_FALSE, UINT64_MAX);

    if (result != VK_SUCCESS) {
        utils::slog.e << "vkWaitForFences(): " << result << utils::io::endl;
        quit(2);
    }
    result = vkResetFences(gVulkanDriver.device, 1, &gVulkanDriver.fences[frameIndex]);
    if (result != VK_SUCCESS) {
        utils::slog.e << "vkResetFences(): " << result << utils::io::endl;
        quit(2);
    }
    currentTime = (double) SDL_GetPerformanceCounter() / SDL_GetPerformanceFrequency();
    clearColor.float32[0] = (float)(0.5 + 0.5 * SDL_sin(SPEED * currentTime));
    clearColor.float32[1] = (float)(0.5 + 0.5 * SDL_sin(SPEED * currentTime + F_PI * 2 / 3));
    clearColor.float32[2] = (float)(0.5 + 0.5 * SDL_sin(SPEED * currentTime + F_PI * 4 / 3));
    clearColor.float32[3] = 1;
    rerecordCommandBuffer(frameIndex, &clearColor);
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &gVulkanDriver.imageAvailableSemaphore;
    submitInfo.pWaitDstStageMask = &waitDestStageMask;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &gVulkanDriver.commandBuffers[frameIndex];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &gVulkanDriver.renderingFinishedSemaphore;
    result = vkQueueSubmit(
            gVulkanDriver.graphicsQueue, 1, &submitInfo, gVulkanDriver.fences[frameIndex]);
    if (result != VK_SUCCESS) {
        utils::slog.e << "vkQueueSubmit(): " << result << utils::io::endl;
        quit(2);
    }
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &gVulkanDriver.renderingFinishedSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &gVulkanDriver.swapchain;
    presentInfo.pImageIndices = &frameIndex;
    result = vkQueuePresentKHR(gVulkanDriver.presentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        return createNewSwapchainThings();
    }
    if (result != VK_SUCCESS) {
        utils::slog.e << "vkQueuePresentKHR(): " << result << utils::io::endl;
        quit(2);
    }
    SDL_Vulkan_GetDrawableSize(gWindow, &w, &h);
    if (w != (int)gVulkanDriver.swapchainSize.width ||
            h != (int)gVulkanDriver.swapchainSize.height) {
        return createNewSwapchainThings();
    }
    return SDL_TRUE;
}

int main(int argc, char *argv[]) {
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);
    gWindow = SDL_CreateWindow("Vulkan", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            512, 512, SDL_WINDOW_VULKAN | SDL_INIT_VIDEO);
    SDL_DisplayMode mode;
    SDL_GetCurrentDisplayMode(0, &mode);
    SDL_Log("Screen BPP : %d\n", SDL_BITSPERPIXEL(mode.format));
    initVulkan();
    bool done = false;
    while (!done) {
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    done = true;
                    break;
            }
        }
        if (!done) {
            render();
        }
    }
    quit(0);
    return 0;
}
