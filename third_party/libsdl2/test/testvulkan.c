/*
  Copyright (C) 1997-2018 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "SDL_test_common.h"

#if defined(__ANDROID__) && defined(__ARM_EABI__) && !defined(__ARM_ARCH_7A__)

int main(int argc, char *argv[])
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "No Vulkan support on this system\n");
    return 1;
}

#else

#define VK_NO_PROTOTYPES
#ifdef HAVE_VULKAN_H
#include <vulkan/vulkan.h>
#else
/* SDL includes a copy for building on systems without the Vulkan SDK */
#include "../src/video/khronos/vulkan/vulkan.h"
#endif
#include "SDL_vulkan.h"

#ifndef UINT64_MAX /* VS2008 */
#define UINT64_MAX 18446744073709551615
#endif

#define VULKAN_FUNCTIONS()                                              \
    VULKAN_DEVICE_FUNCTION(vkAcquireNextImageKHR)                       \
    VULKAN_DEVICE_FUNCTION(vkAllocateCommandBuffers)                    \
    VULKAN_DEVICE_FUNCTION(vkBeginCommandBuffer)                        \
    VULKAN_DEVICE_FUNCTION(vkCmdClearColorImage)                        \
    VULKAN_DEVICE_FUNCTION(vkCmdPipelineBarrier)                        \
    VULKAN_DEVICE_FUNCTION(vkCreateCommandPool)                         \
    VULKAN_DEVICE_FUNCTION(vkCreateFence)                               \
    VULKAN_DEVICE_FUNCTION(vkCreateImageView)                           \
    VULKAN_DEVICE_FUNCTION(vkCreateSemaphore)                           \
    VULKAN_DEVICE_FUNCTION(vkCreateSwapchainKHR)                        \
    VULKAN_DEVICE_FUNCTION(vkDestroyCommandPool)                        \
    VULKAN_DEVICE_FUNCTION(vkDestroyDevice)                             \
    VULKAN_DEVICE_FUNCTION(vkDestroyFence)                              \
    VULKAN_DEVICE_FUNCTION(vkDestroyImageView)                          \
    VULKAN_DEVICE_FUNCTION(vkDestroySemaphore)                          \
    VULKAN_DEVICE_FUNCTION(vkDestroySwapchainKHR)                       \
    VULKAN_DEVICE_FUNCTION(vkDeviceWaitIdle)                            \
    VULKAN_DEVICE_FUNCTION(vkEndCommandBuffer)                          \
    VULKAN_DEVICE_FUNCTION(vkFreeCommandBuffers)                        \
    VULKAN_DEVICE_FUNCTION(vkGetDeviceQueue)                            \
    VULKAN_DEVICE_FUNCTION(vkGetFenceStatus)                            \
    VULKAN_DEVICE_FUNCTION(vkGetSwapchainImagesKHR)                     \
    VULKAN_DEVICE_FUNCTION(vkQueuePresentKHR)                           \
    VULKAN_DEVICE_FUNCTION(vkQueueSubmit)                               \
    VULKAN_DEVICE_FUNCTION(vkResetCommandBuffer)                        \
    VULKAN_DEVICE_FUNCTION(vkResetFences)                               \
    VULKAN_DEVICE_FUNCTION(vkWaitForFences)                             \
    VULKAN_GLOBAL_FUNCTION(vkCreateInstance)                            \
    VULKAN_GLOBAL_FUNCTION(vkEnumerateInstanceExtensionProperties)      \
    VULKAN_GLOBAL_FUNCTION(vkEnumerateInstanceLayerProperties)          \
    VULKAN_INSTANCE_FUNCTION(vkCreateDevice)                            \
    VULKAN_INSTANCE_FUNCTION(vkDestroyInstance)                         \
    VULKAN_INSTANCE_FUNCTION(vkDestroySurfaceKHR)                       \
    VULKAN_INSTANCE_FUNCTION(vkEnumerateDeviceExtensionProperties)      \
    VULKAN_INSTANCE_FUNCTION(vkEnumeratePhysicalDevices)                \
    VULKAN_INSTANCE_FUNCTION(vkGetDeviceProcAddr)                       \
    VULKAN_INSTANCE_FUNCTION(vkGetPhysicalDeviceFeatures)               \
    VULKAN_INSTANCE_FUNCTION(vkGetPhysicalDeviceProperties)             \
    VULKAN_INSTANCE_FUNCTION(vkGetPhysicalDeviceQueueFamilyProperties)  \
    VULKAN_INSTANCE_FUNCTION(vkGetPhysicalDeviceSurfaceCapabilitiesKHR) \
    VULKAN_INSTANCE_FUNCTION(vkGetPhysicalDeviceSurfaceFormatsKHR)      \
    VULKAN_INSTANCE_FUNCTION(vkGetPhysicalDeviceSurfacePresentModesKHR) \
    VULKAN_INSTANCE_FUNCTION(vkGetPhysicalDeviceSurfaceSupportKHR)

#define VULKAN_DEVICE_FUNCTION(name) static PFN_##name name = NULL;
#define VULKAN_GLOBAL_FUNCTION(name) static PFN_##name name = NULL;
#define VULKAN_INSTANCE_FUNCTION(name) static PFN_##name name = NULL;
VULKAN_FUNCTIONS()
#undef VULKAN_DEVICE_FUNCTION
#undef VULKAN_GLOBAL_FUNCTION
#undef VULKAN_INSTANCE_FUNCTION
static PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = NULL;

/* Based on the headers found in
 * https://github.com/KhronosGroup/Vulkan-LoaderAndValidationLayers
 */
#if VK_HEADER_VERSION < 22
enum
{
    VK_ERROR_FRAGMENTED_POOL = -12,
};
#endif
#if VK_HEADER_VERSION < 38
enum {
    VK_ERROR_OUT_OF_POOL_MEMORY_KHR = -1000069000
};
#endif

static const char *getVulkanResultString(VkResult result)
{
    switch((int)result)
    {
    case VK_SUCCESS:
        return "VK_SUCCESS";
    case VK_NOT_READY:
        return "VK_NOT_READY";
    case VK_TIMEOUT:
        return "VK_TIMEOUT";
    case VK_EVENT_SET:
        return "VK_EVENT_SET";
    case VK_EVENT_RESET:
        return "VK_EVENT_RESET";
    case VK_INCOMPLETE:
        return "VK_INCOMPLETE";
    case VK_ERROR_OUT_OF_HOST_MEMORY:
        return "VK_ERROR_OUT_OF_HOST_MEMORY";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
    case VK_ERROR_INITIALIZATION_FAILED:
        return "VK_ERROR_INITIALIZATION_FAILED";
    case VK_ERROR_DEVICE_LOST:
        return "VK_ERROR_DEVICE_LOST";
    case VK_ERROR_MEMORY_MAP_FAILED:
        return "VK_ERROR_MEMORY_MAP_FAILED";
    case VK_ERROR_LAYER_NOT_PRESENT:
        return "VK_ERROR_LAYER_NOT_PRESENT";
    case VK_ERROR_EXTENSION_NOT_PRESENT:
        return "VK_ERROR_EXTENSION_NOT_PRESENT";
    case VK_ERROR_FEATURE_NOT_PRESENT:
        return "VK_ERROR_FEATURE_NOT_PRESENT";
    case VK_ERROR_INCOMPATIBLE_DRIVER:
        return "VK_ERROR_INCOMPATIBLE_DRIVER";
    case VK_ERROR_TOO_MANY_OBJECTS:
        return "VK_ERROR_TOO_MANY_OBJECTS";
    case VK_ERROR_FORMAT_NOT_SUPPORTED:
        return "VK_ERROR_FORMAT_NOT_SUPPORTED";
    case VK_ERROR_FRAGMENTED_POOL:
        return "VK_ERROR_FRAGMENTED_POOL";
    case VK_ERROR_SURFACE_LOST_KHR:
        return "VK_ERROR_SURFACE_LOST_KHR";
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
        return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
    case VK_SUBOPTIMAL_KHR:
        return "VK_SUBOPTIMAL_KHR";
    case VK_ERROR_OUT_OF_DATE_KHR:
        return "VK_ERROR_OUT_OF_DATE_KHR";
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
        return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
    case VK_ERROR_VALIDATION_FAILED_EXT:
        return "VK_ERROR_VALIDATION_FAILED_EXT";
    case VK_ERROR_OUT_OF_POOL_MEMORY_KHR:
        return "VK_ERROR_OUT_OF_POOL_MEMORY_KHR";
    case VK_ERROR_INVALID_SHADER_NV:
        return "VK_ERROR_INVALID_SHADER_NV";
    case VK_RESULT_MAX_ENUM:
    case VK_RESULT_RANGE_SIZE:
        break;
    }
    if(result < 0)
        return "VK_ERROR_<Unknown>";
    return "VK_<Unknown>";
}

typedef struct VulkanContext
{
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
} VulkanContext;

static SDLTest_CommonState *state;
static VulkanContext vulkanContext = {0};

static void shutdownVulkan(void);

/* Call this instead of exit(), so we can clean up SDL: atexit() is evil. */
static void quit(int rc)
{
    shutdownVulkan();
    SDLTest_CommonQuit(state);
    exit(rc);
}

static void loadGlobalFunctions(void)
{
    vkGetInstanceProcAddr = SDL_Vulkan_GetVkGetInstanceProcAddr();
    if(!vkGetInstanceProcAddr)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "SDL_Vulkan_GetVkGetInstanceProcAddr(): %s\n",
                     SDL_GetError());
        quit(2);
    }

#define VULKAN_DEVICE_FUNCTION(name)
#define VULKAN_GLOBAL_FUNCTION(name)                                                   \
    name = (PFN_##name)vkGetInstanceProcAddr(VK_NULL_HANDLE, #name);                   \
    if(!name)                                                                          \
    {                                                                                  \
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,                                     \
                     "vkGetInstanceProcAddr(VK_NULL_HANDLE, \"" #name "\") failed\n"); \
        quit(2);                                                                       \
    }
#define VULKAN_INSTANCE_FUNCTION(name)
    VULKAN_FUNCTIONS()
#undef VULKAN_DEVICE_FUNCTION
#undef VULKAN_GLOBAL_FUNCTION
#undef VULKAN_INSTANCE_FUNCTION
}

static void createInstance(void)
{
    VkApplicationInfo appInfo = {0};
    VkInstanceCreateInfo instanceCreateInfo = {0};
    const char **extensions = NULL;
    unsigned extensionCount = 0;
	VkResult result;


	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = VK_API_VERSION_1_0;
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    if(!SDL_Vulkan_GetInstanceExtensions(state->windows[0], &extensionCount, NULL))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "SDL_Vulkan_GetInstanceExtensions(): %s\n",
                     SDL_GetError());
        quit(2);
    }
    extensions = SDL_malloc(sizeof(const char *) * extensionCount);
    if(!extensions)
    {
        SDL_OutOfMemory();
        quit(2);
    }
    if(!SDL_Vulkan_GetInstanceExtensions(state->windows[0], &extensionCount, extensions))
    {
        SDL_free((void*)extensions);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "SDL_Vulkan_GetInstanceExtensions(): %s\n",
                     SDL_GetError());
        quit(2);
    }
    instanceCreateInfo.enabledExtensionCount = extensionCount;
    instanceCreateInfo.ppEnabledExtensionNames = extensions;
    result = vkCreateInstance(&instanceCreateInfo, NULL, &vulkanContext.instance);
    SDL_free((void*)extensions);
    if(result != VK_SUCCESS)
    {
        vulkanContext.instance = VK_NULL_HANDLE;
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "vkCreateInstance(): %s\n",
                     getVulkanResultString(result));
        quit(2);
    }
}

static void loadInstanceFunctions(void)
{
#define VULKAN_DEVICE_FUNCTION(name)
#define VULKAN_GLOBAL_FUNCTION(name)
#define VULKAN_INSTANCE_FUNCTION(name)                                           \
    name = (PFN_##name)vkGetInstanceProcAddr(vulkanContext.instance, #name);     \
    if(!name)                                                                    \
    {                                                                            \
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,                               \
                     "vkGetInstanceProcAddr(instance, \"" #name "\") failed\n"); \
        quit(2);                                                                 \
    }
    VULKAN_FUNCTIONS()
#undef VULKAN_DEVICE_FUNCTION
#undef VULKAN_GLOBAL_FUNCTION
#undef VULKAN_INSTANCE_FUNCTION
}

static void createSurface(void)
{
    if(!SDL_Vulkan_CreateSurface(state->windows[0],
                                 vulkanContext.instance,
                                 &vulkanContext.surface))
    {
        vulkanContext.surface = VK_NULL_HANDLE;
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION, "SDL_Vulkan_CreateSurface(): %s\n", SDL_GetError());
        quit(2);
    }
}

static void findPhysicalDevice(void)
{
    uint32_t physicalDeviceCount = 0;
	VkPhysicalDevice *physicalDevices;
	VkQueueFamilyProperties *queueFamiliesProperties = NULL;
    uint32_t queueFamiliesPropertiesAllocatedSize = 0;
    VkExtensionProperties *deviceExtensions = NULL;
    uint32_t deviceExtensionsAllocatedSize = 0;
	uint32_t physicalDeviceIndex;

    VkResult result =
        vkEnumeratePhysicalDevices(vulkanContext.instance, &physicalDeviceCount, NULL);
    if(result != VK_SUCCESS)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "vkEnumeratePhysicalDevices(): %s\n",
                     getVulkanResultString(result));
        quit(2);
    }
    if(physicalDeviceCount == 0)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "vkEnumeratePhysicalDevices(): no physical devices\n");
        quit(2);
    }
    physicalDevices = SDL_malloc(sizeof(VkPhysicalDevice) * physicalDeviceCount);
    if(!physicalDevices)
    {
        SDL_OutOfMemory();
        quit(2);
    }
    result =
        vkEnumeratePhysicalDevices(vulkanContext.instance, &physicalDeviceCount, physicalDevices);
    if(result != VK_SUCCESS)
    {
        SDL_free(physicalDevices);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "vkEnumeratePhysicalDevices(): %s\n",
                     getVulkanResultString(result));
        quit(2);
    }
    vulkanContext.physicalDevice = NULL;
    for(physicalDeviceIndex = 0; physicalDeviceIndex < physicalDeviceCount;
        physicalDeviceIndex++)
    {
        uint32_t queueFamiliesCount = 0;
		uint32_t queueFamilyIndex;
        uint32_t deviceExtensionCount = 0;
		SDL_bool hasSwapchainExtension = SDL_FALSE;
		uint32_t i;


		VkPhysicalDevice physicalDevice = physicalDevices[physicalDeviceIndex];
        vkGetPhysicalDeviceProperties(physicalDevice, &vulkanContext.physicalDeviceProperties);
        if(VK_VERSION_MAJOR(vulkanContext.physicalDeviceProperties.apiVersion) < 1)
            continue;
        vkGetPhysicalDeviceFeatures(physicalDevice, &vulkanContext.physicalDeviceFeatures);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamiliesCount, NULL);
        if(queueFamiliesCount == 0)
            continue;
        if(queueFamiliesPropertiesAllocatedSize < queueFamiliesCount)
        {
            SDL_free(queueFamiliesProperties);
            queueFamiliesPropertiesAllocatedSize = queueFamiliesCount;
            queueFamiliesProperties =
                SDL_malloc(sizeof(VkQueueFamilyProperties) * queueFamiliesPropertiesAllocatedSize);
            if(!queueFamiliesProperties)
            {
                SDL_free(physicalDevices);
                SDL_free(deviceExtensions);
                SDL_OutOfMemory();
                quit(2);
            }
        }
        vkGetPhysicalDeviceQueueFamilyProperties(
            physicalDevice, &queueFamiliesCount, queueFamiliesProperties);
        vulkanContext.graphicsQueueFamilyIndex = queueFamiliesCount;
        vulkanContext.presentQueueFamilyIndex = queueFamiliesCount;
        for(queueFamilyIndex = 0; queueFamilyIndex < queueFamiliesCount;
            queueFamilyIndex++)
        {
            VkBool32 supported = 0;

			if(queueFamiliesProperties[queueFamilyIndex].queueCount == 0)
                continue;
            if(queueFamiliesProperties[queueFamilyIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                vulkanContext.graphicsQueueFamilyIndex = queueFamilyIndex;
            result = vkGetPhysicalDeviceSurfaceSupportKHR(
                physicalDevice, queueFamilyIndex, vulkanContext.surface, &supported);
            if(result != VK_SUCCESS)
            {
                SDL_free(physicalDevices);
                SDL_free(queueFamiliesProperties);
                SDL_free(deviceExtensions);
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                             "vkGetPhysicalDeviceSurfaceSupportKHR(): %s\n",
                             getVulkanResultString(result));
                quit(2);
            }
            if(supported)
            {
                vulkanContext.presentQueueFamilyIndex = queueFamilyIndex;
                if(queueFamiliesProperties[queueFamilyIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                    break; // use this queue because it can present and do graphics
            }
        }
        if(vulkanContext.graphicsQueueFamilyIndex == queueFamiliesCount) // no good queues found
            continue;
        if(vulkanContext.presentQueueFamilyIndex == queueFamiliesCount) // no good queues found
            continue;
        result =
            vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &deviceExtensionCount, NULL);
        if(result != VK_SUCCESS)
        {
            SDL_free(physicalDevices);
            SDL_free(queueFamiliesProperties);
            SDL_free(deviceExtensions);
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "vkEnumerateDeviceExtensionProperties(): %s\n",
                         getVulkanResultString(result));
            quit(2);
        }
        if(deviceExtensionCount == 0)
            continue;
        if(deviceExtensionsAllocatedSize < deviceExtensionCount)
        {
            SDL_free(deviceExtensions);
            deviceExtensionsAllocatedSize = deviceExtensionCount;
            deviceExtensions =
                SDL_malloc(sizeof(VkExtensionProperties) * deviceExtensionsAllocatedSize);
            if(!deviceExtensions)
            {
                SDL_free(physicalDevices);
                SDL_free(queueFamiliesProperties);
                SDL_OutOfMemory();
                quit(2);
            }
        }
        result = vkEnumerateDeviceExtensionProperties(
            physicalDevice, NULL, &deviceExtensionCount, deviceExtensions);
        if(result != VK_SUCCESS)
        {
            SDL_free(physicalDevices);
            SDL_free(queueFamiliesProperties);
            SDL_free(deviceExtensions);
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "vkEnumerateDeviceExtensionProperties(): %s\n",
                         getVulkanResultString(result));
            quit(2);
        }
        for(i = 0; i < deviceExtensionCount; i++)
        {
            if(0 == SDL_strcmp(deviceExtensions[i].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME))
            {
                hasSwapchainExtension = SDL_TRUE;
                break;
            }
        }
        if(!hasSwapchainExtension)
            continue;
        vulkanContext.physicalDevice = physicalDevice;
        break;
    }
    SDL_free(physicalDevices);
    SDL_free(queueFamiliesProperties);
    SDL_free(deviceExtensions);
    if(!vulkanContext.physicalDevice)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Vulkan: no viable physical devices found");
        quit(2);
    }
}

static void createDevice(void)
{
    VkDeviceQueueCreateInfo deviceQueueCreateInfo[1] = {0};
    static const float queuePriority[] = {1.0f};
    VkDeviceCreateInfo deviceCreateInfo = {0};
    static const char *const deviceExtensionNames[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
	VkResult result;

	deviceQueueCreateInfo->sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfo->queueFamilyIndex = vulkanContext.graphicsQueueFamilyIndex;
    deviceQueueCreateInfo->queueCount = 1;
    deviceQueueCreateInfo->pQueuePriorities = &queuePriority[0];

    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfo;
    deviceCreateInfo.pEnabledFeatures = NULL;
    deviceCreateInfo.enabledExtensionCount = SDL_arraysize(deviceExtensionNames);
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensionNames;
    result = vkCreateDevice(
        vulkanContext.physicalDevice, &deviceCreateInfo, NULL, &vulkanContext.device);
    if(result != VK_SUCCESS)
    {
        vulkanContext.device = VK_NULL_HANDLE;
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION, "vkCreateDevice(): %s\n", getVulkanResultString(result));
        quit(2);
    }
}

static void loadDeviceFunctions(void)
{
#define VULKAN_DEVICE_FUNCTION(name)                                         \
    name = (PFN_##name)vkGetDeviceProcAddr(vulkanContext.device, #name);     \
    if(!name)                                                                \
    {                                                                        \
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,                           \
                     "vkGetDeviceProcAddr(device, \"" #name "\") failed\n"); \
        quit(2);                                                             \
    }
#define VULKAN_GLOBAL_FUNCTION(name)
#define VULKAN_INSTANCE_FUNCTION(name)
    VULKAN_FUNCTIONS()
#undef VULKAN_DEVICE_FUNCTION
#undef VULKAN_GLOBAL_FUNCTION
#undef VULKAN_INSTANCE_FUNCTION
}

#undef VULKAN_FUNCTIONS

static void getQueues(void)
{
    vkGetDeviceQueue(vulkanContext.device,
                     vulkanContext.graphicsQueueFamilyIndex,
                     0,
                     &vulkanContext.graphicsQueue);
    if(vulkanContext.graphicsQueueFamilyIndex != vulkanContext.presentQueueFamilyIndex)
        vkGetDeviceQueue(vulkanContext.device,
                         vulkanContext.presentQueueFamilyIndex,
                         0,
                         &vulkanContext.presentQueue);
    else
        vulkanContext.presentQueue = vulkanContext.graphicsQueue;
}

static void createSemaphore(VkSemaphore *semaphore)
{
	VkResult result;

    VkSemaphoreCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    result = vkCreateSemaphore(vulkanContext.device, &createInfo, NULL, semaphore);
    if(result != VK_SUCCESS)
    {
        *semaphore = VK_NULL_HANDLE;
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "vkCreateSemaphore(): %s\n",
                     getVulkanResultString(result));
        quit(2);
    }
}

static void createSemaphores(void)
{
    createSemaphore(&vulkanContext.imageAvailableSemaphore);
    createSemaphore(&vulkanContext.renderingFinishedSemaphore);
}

static void getSurfaceCaps(void)
{
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        vulkanContext.physicalDevice, vulkanContext.surface, &vulkanContext.surfaceCapabilities);
    if(result != VK_SUCCESS)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "vkGetPhysicalDeviceSurfaceCapabilitiesKHR(): %s\n",
                     getVulkanResultString(result));
        quit(2);
    }

    // check surface usage
    if(!(vulkanContext.surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Vulkan surface doesn't support VK_IMAGE_USAGE_TRANSFER_DST_BIT\n");
        quit(2);
    }
}

static void getSurfaceFormats(void)
{
    VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(vulkanContext.physicalDevice,
                                                           vulkanContext.surface,
                                                           &vulkanContext.surfaceFormatsCount,
                                                           NULL);
    if(result != VK_SUCCESS)
    {
        vulkanContext.surfaceFormatsCount = 0;
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "vkGetPhysicalDeviceSurfaceFormatsKHR(): %s\n",
                     getVulkanResultString(result));
        quit(2);
    }
    if(vulkanContext.surfaceFormatsCount > vulkanContext.surfaceFormatsAllocatedCount)
    {
        vulkanContext.surfaceFormatsAllocatedCount = vulkanContext.surfaceFormatsCount;
        SDL_free(vulkanContext.surfaceFormats);
        vulkanContext.surfaceFormats =
            SDL_malloc(sizeof(VkSurfaceFormatKHR) * vulkanContext.surfaceFormatsAllocatedCount);
        if(!vulkanContext.surfaceFormats)
        {
            vulkanContext.surfaceFormatsCount = 0;
            SDL_OutOfMemory();
            quit(2);
        }
    }
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(vulkanContext.physicalDevice,
                                                  vulkanContext.surface,
                                                  &vulkanContext.surfaceFormatsCount,
                                                  vulkanContext.surfaceFormats);
    if(result != VK_SUCCESS)
    {
        vulkanContext.surfaceFormatsCount = 0;
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "vkGetPhysicalDeviceSurfaceFormatsKHR(): %s\n",
                     getVulkanResultString(result));
        quit(2);
    }
}

static void getSwapchainImages(void)
{
	VkResult result;

    SDL_free(vulkanContext.swapchainImages);
    vulkanContext.swapchainImages = NULL;
    result = vkGetSwapchainImagesKHR(
        vulkanContext.device, vulkanContext.swapchain, &vulkanContext.swapchainImageCount, NULL);
    if(result != VK_SUCCESS)
    {
        vulkanContext.swapchainImageCount = 0;
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "vkGetSwapchainImagesKHR(): %s\n",
                     getVulkanResultString(result));
        quit(2);
    }
    vulkanContext.swapchainImages = SDL_malloc(sizeof(VkImage) * vulkanContext.swapchainImageCount);
    if(!vulkanContext.swapchainImages)
    {
        SDL_OutOfMemory();
        quit(2);
    }
    result = vkGetSwapchainImagesKHR(vulkanContext.device,
                                     vulkanContext.swapchain,
                                     &vulkanContext.swapchainImageCount,
                                     vulkanContext.swapchainImages);
    if(result != VK_SUCCESS)
    {
        SDL_free(vulkanContext.swapchainImages);
        vulkanContext.swapchainImages = NULL;
        vulkanContext.swapchainImageCount = 0;
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "vkGetSwapchainImagesKHR(): %s\n",
                     getVulkanResultString(result));
        quit(2);
    }
}

static SDL_bool createSwapchain(void)
{
	uint32_t i;
	int w, h;
	VkSwapchainCreateInfoKHR createInfo = {0};
	VkResult result;

    // pick an image count
    vulkanContext.swapchainDesiredImageCount = vulkanContext.surfaceCapabilities.minImageCount + 1;
    if(vulkanContext.swapchainDesiredImageCount > vulkanContext.surfaceCapabilities.maxImageCount
       && vulkanContext.surfaceCapabilities.maxImageCount > 0)
        vulkanContext.swapchainDesiredImageCount = vulkanContext.surfaceCapabilities.maxImageCount;

    // pick a format
    if(vulkanContext.surfaceFormatsCount == 1
       && vulkanContext.surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
    {
        // aren't any preferred formats, so we pick
        vulkanContext.surfaceFormat.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
        vulkanContext.surfaceFormat.format = VK_FORMAT_R8G8B8A8_UNORM;
    }
    else
    {
        vulkanContext.surfaceFormat = vulkanContext.surfaceFormats[0];
        for(i = 0; i < vulkanContext.surfaceFormatsCount; i++)
        {
            if(vulkanContext.surfaceFormats[i].format == VK_FORMAT_R8G8B8A8_UNORM)
            {
                vulkanContext.surfaceFormat = vulkanContext.surfaceFormats[i];
                break;
            }
        }
    }

    // get size
    SDL_Vulkan_GetDrawableSize(state->windows[0], &w, &h);
    vulkanContext.swapchainSize.width = w;
    vulkanContext.swapchainSize.height = h;
    if(w == 0 || h == 0)
        return SDL_FALSE;

    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = vulkanContext.surface;
    createInfo.minImageCount = vulkanContext.swapchainDesiredImageCount;
    createInfo.imageFormat = vulkanContext.surfaceFormat.format;
    createInfo.imageColorSpace = vulkanContext.surfaceFormat.colorSpace;
    createInfo.imageExtent = vulkanContext.swapchainSize;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.preTransform = vulkanContext.surfaceCapabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = vulkanContext.swapchain;
    result =
        vkCreateSwapchainKHR(vulkanContext.device, &createInfo, NULL, &vulkanContext.swapchain);
    if(createInfo.oldSwapchain)
        vkDestroySwapchainKHR(vulkanContext.device, createInfo.oldSwapchain, NULL);
    if(result != VK_SUCCESS)
    {
        vulkanContext.swapchain = VK_NULL_HANDLE;
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "vkCreateSwapchainKHR(): %s\n",
                     getVulkanResultString(result));
        quit(2);
    }
    getSwapchainImages();
    return SDL_TRUE;
}

static void destroySwapchain(void)
{
    if(vulkanContext.swapchain)
        vkDestroySwapchainKHR(vulkanContext.device, vulkanContext.swapchain, NULL);
    vulkanContext.swapchain = VK_NULL_HANDLE;
    SDL_free(vulkanContext.swapchainImages);
    vulkanContext.swapchainImages = NULL;
}

static void destroyCommandBuffers(void)
{
    if(vulkanContext.commandBuffers)
        vkFreeCommandBuffers(vulkanContext.device,
                             vulkanContext.commandPool,
                             vulkanContext.swapchainImageCount,
                             vulkanContext.commandBuffers);
    SDL_free(vulkanContext.commandBuffers);
    vulkanContext.commandBuffers = NULL;
}

static void destroyCommandPool(void)
{
    if(vulkanContext.commandPool)
        vkDestroyCommandPool(vulkanContext.device, vulkanContext.commandPool, NULL);
    vulkanContext.commandPool = VK_NULL_HANDLE;
}

static void createCommandPool(void)
{
	VkResult result;

    VkCommandPoolCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.flags =
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    createInfo.queueFamilyIndex = vulkanContext.graphicsQueueFamilyIndex;
    result =
        vkCreateCommandPool(vulkanContext.device, &createInfo, NULL, &vulkanContext.commandPool);
    if(result != VK_SUCCESS)
    {
        vulkanContext.commandPool = VK_NULL_HANDLE;
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "vkCreateCommandPool(): %s\n",
                     getVulkanResultString(result));
        quit(2);
    }
}

static void createCommandBuffers(void)
{
	VkResult result;

    VkCommandBufferAllocateInfo allocateInfo = {0};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = vulkanContext.commandPool;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = vulkanContext.swapchainImageCount;
    vulkanContext.commandBuffers =
        SDL_malloc(sizeof(VkCommandBuffer) * vulkanContext.swapchainImageCount);
    result =
        vkAllocateCommandBuffers(vulkanContext.device, &allocateInfo, vulkanContext.commandBuffers);
    if(result != VK_SUCCESS)
    {
        SDL_free(vulkanContext.commandBuffers);
        vulkanContext.commandBuffers = NULL;
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "vkAllocateCommandBuffers(): %s\n",
                     getVulkanResultString(result));
        quit(2);
    }
}

static void createFences(void)
{
	uint32_t i;

    vulkanContext.fences = SDL_malloc(sizeof(VkFence) * vulkanContext.swapchainImageCount);
    if(!vulkanContext.fences)
    {
        SDL_OutOfMemory();
        quit(2);
    }
    for(i = 0; i < vulkanContext.swapchainImageCount; i++)
    {
		VkResult result;

        VkFenceCreateInfo createInfo = {0};
        createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        result =
            vkCreateFence(vulkanContext.device, &createInfo, NULL, &vulkanContext.fences[i]);
        if(result != VK_SUCCESS)
        {
            for(; i > 0; i--)
            {
                vkDestroyFence(vulkanContext.device, vulkanContext.fences[i - 1], NULL);
            }
            SDL_free(vulkanContext.fences);
            vulkanContext.fences = NULL;
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "vkCreateFence(): %s\n",
                         getVulkanResultString(result));
            quit(2);
        }
    }
}

static void destroyFences(void)
{
	uint32_t i;

    if(!vulkanContext.fences)
        return;
    for(i = 0; i < vulkanContext.swapchainImageCount; i++)
    {
        vkDestroyFence(vulkanContext.device, vulkanContext.fences[i], NULL);
    }
    SDL_free(vulkanContext.fences);
    vulkanContext.fences = NULL;
}

static void recordPipelineImageBarrier(VkCommandBuffer commandBuffer,
                                       VkAccessFlags sourceAccessMask,
                                       VkAccessFlags destAccessMask,
                                       VkImageLayout sourceLayout,
                                       VkImageLayout destLayout,
                                       VkImage image)
{
    VkImageMemoryBarrier barrier = {0};
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

static void rerecordCommandBuffer(uint32_t frameIndex, const VkClearColorValue *clearColor)
{
    VkCommandBuffer commandBuffer = vulkanContext.commandBuffers[frameIndex];
    VkImage image = vulkanContext.swapchainImages[frameIndex];
	VkCommandBufferBeginInfo beginInfo = {0};
    VkImageSubresourceRange clearRange = {0};

    VkResult result = vkResetCommandBuffer(commandBuffer, 0);
    if(result != VK_SUCCESS)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "vkResetCommandBuffer(): %s\n",
                     getVulkanResultString(result));
        quit(2);
    }
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    if(result != VK_SUCCESS)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "vkBeginCommandBuffer(): %s\n",
                     getVulkanResultString(result));
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
    if(result != VK_SUCCESS)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "vkEndCommandBuffer(): %s\n",
                     getVulkanResultString(result));
        quit(2);
    }
}

static void destroySwapchainAndSwapchainSpecificStuff(SDL_bool doDestroySwapchain)
{
    destroyFences();
    destroyCommandBuffers();
    destroyCommandPool();
    if(doDestroySwapchain)
        destroySwapchain();
}

static SDL_bool createNewSwapchainAndSwapchainSpecificStuff(void)
{
    destroySwapchainAndSwapchainSpecificStuff(SDL_FALSE);
    getSurfaceCaps();
    getSurfaceFormats();
    if(!createSwapchain())
        return SDL_FALSE;
    createCommandPool();
    createCommandBuffers();
    createFences();
    return SDL_TRUE;
}

static void initVulkan(void)
{
    SDL_Vulkan_LoadLibrary(NULL);
    SDL_memset(&vulkanContext, 0, sizeof(VulkanContext));
    loadGlobalFunctions();
    createInstance();
    loadInstanceFunctions();
    createSurface();
    findPhysicalDevice();
    createDevice();
    loadDeviceFunctions();
    getQueues();
    createSemaphores();
    createNewSwapchainAndSwapchainSpecificStuff();
}

static void shutdownVulkan(void)
{
    if(vulkanContext.device && vkDeviceWaitIdle)
        vkDeviceWaitIdle(vulkanContext.device);
    destroySwapchainAndSwapchainSpecificStuff(SDL_TRUE);
    if(vulkanContext.imageAvailableSemaphore && vkDestroySemaphore)
        vkDestroySemaphore(vulkanContext.device, vulkanContext.imageAvailableSemaphore, NULL);
    if(vulkanContext.renderingFinishedSemaphore && vkDestroySemaphore)
        vkDestroySemaphore(vulkanContext.device, vulkanContext.renderingFinishedSemaphore, NULL);
    if(vulkanContext.device && vkDestroyDevice)
        vkDestroyDevice(vulkanContext.device, NULL);
    if(vulkanContext.surface && vkDestroySurfaceKHR)
        vkDestroySurfaceKHR(vulkanContext.instance, vulkanContext.surface, NULL);
    if(vulkanContext.instance && vkDestroyInstance)
        vkDestroyInstance(vulkanContext.instance, NULL);
    SDL_free(vulkanContext.surfaceFormats);
    SDL_Vulkan_UnloadLibrary();
}

static SDL_bool render(void)
{
    uint32_t frameIndex;
    VkResult result;
    double currentTime;
    VkClearColorValue clearColor = {0};
    VkPipelineStageFlags waitDestStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    VkSubmitInfo submitInfo = {0};
    VkPresentInfoKHR presentInfo = {0};
    int w, h;

    if(!vulkanContext.swapchain)
    {
        SDL_bool retval = createNewSwapchainAndSwapchainSpecificStuff();
        if(!retval)
            SDL_Delay(100);
        return retval;
    }
    result = vkAcquireNextImageKHR(vulkanContext.device,
                                            vulkanContext.swapchain,
                                            UINT64_MAX,
                                            vulkanContext.imageAvailableSemaphore,
                                            VK_NULL_HANDLE,
                                            &frameIndex);
    if(result == VK_ERROR_OUT_OF_DATE_KHR)
        return createNewSwapchainAndSwapchainSpecificStuff();
    if(result != VK_SUBOPTIMAL_KHR && result != VK_SUCCESS)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "vkAcquireNextImageKHR(): %s\n",
                     getVulkanResultString(result));
        quit(2);
    }
    result = vkWaitForFences(
        vulkanContext.device, 1, &vulkanContext.fences[frameIndex], VK_FALSE, UINT64_MAX);
    if(result != VK_SUCCESS)
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION, "vkWaitForFences(): %s\n", getVulkanResultString(result));
        quit(2);
    }
    result = vkResetFences(vulkanContext.device, 1, &vulkanContext.fences[frameIndex]);
    if(result != VK_SUCCESS)
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION, "vkResetFences(): %s\n", getVulkanResultString(result));
        quit(2);
    }
    currentTime = (double)SDL_GetPerformanceCounter() / SDL_GetPerformanceFrequency();
    clearColor.float32[0] = (float)(0.5 + 0.5 * SDL_sin(currentTime));
    clearColor.float32[1] = (float)(0.5 + 0.5 * SDL_sin(currentTime + M_PI * 2 / 3));
    clearColor.float32[2] = (float)(0.5 + 0.5 * SDL_sin(currentTime + M_PI * 4 / 3));
    clearColor.float32[3] = 1;
    rerecordCommandBuffer(frameIndex, &clearColor);
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &vulkanContext.imageAvailableSemaphore;
    submitInfo.pWaitDstStageMask = &waitDestStageMask;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vulkanContext.commandBuffers[frameIndex];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &vulkanContext.renderingFinishedSemaphore;
    result = vkQueueSubmit(
        vulkanContext.graphicsQueue, 1, &submitInfo, vulkanContext.fences[frameIndex]);
    if(result != VK_SUCCESS)
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION, "vkQueueSubmit(): %s\n", getVulkanResultString(result));
        quit(2);
    }
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &vulkanContext.renderingFinishedSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &vulkanContext.swapchain;
    presentInfo.pImageIndices = &frameIndex;
    result = vkQueuePresentKHR(vulkanContext.presentQueue, &presentInfo);
    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        return createNewSwapchainAndSwapchainSpecificStuff();
    }
    if(result != VK_SUCCESS)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "vkQueuePresentKHR(): %s\n",
                     getVulkanResultString(result));
        quit(2);
    }
    SDL_Vulkan_GetDrawableSize(state->windows[0], &w, &h);
    if(w != (int)vulkanContext.swapchainSize.width || h != (int)vulkanContext.swapchainSize.height)
    {
        return createNewSwapchainAndSwapchainSpecificStuff();
    }
    return SDL_TRUE;
}

int main(int argc, char *argv[])
{
    int fsaa, accel;
    int i, done;
    SDL_DisplayMode mode;
    SDL_Event event;
    Uint32 then, now, frames;
    int dw, dh;

    /* Enable standard application logging */
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    /* Initialize parameters */
    fsaa = 0;
    accel = -1;

    /* Initialize test framework */
    state = SDLTest_CommonCreateState(argv, SDL_INIT_VIDEO);
    if(!state)
    {
        return 1;
    }
    for(i = 1; i < argc;)
    {
        int consumed;

        consumed = SDLTest_CommonArg(state, i);
        if(consumed < 0)
        {
            SDL_Log("Usage: %s %s\n", argv[0], SDLTest_CommonUsage(state));
            quit(1);
        }
        i += consumed;
    }

    /* Set Vulkan parameters */
    state->window_flags |= SDL_WINDOW_VULKAN;
    state->num_windows = 1;
    state->skip_renderer = 1;

    if(!SDLTest_CommonInit(state))
    {
        quit(2);
    }

    SDL_GetCurrentDisplayMode(0, &mode);
    SDL_Log("Screen BPP    : %d\n", SDL_BITSPERPIXEL(mode.format));
    SDL_GetWindowSize(state->windows[0], &dw, &dh);
    SDL_Log("Window Size   : %d,%d\n", dw, dh);
    SDL_Vulkan_GetDrawableSize(state->windows[0], &dw, &dh);
    SDL_Log("Draw Size     : %d,%d\n", dw, dh);
    SDL_Log("\n");

    initVulkan();

    /* Main render loop */
    frames = 0;
    then = SDL_GetTicks();
    done = 0;
    while(!done)
    {
        /* Check for events */
        ++frames;
        while(SDL_PollEvent(&event))
        {
            SDLTest_CommonEvent(state, &event, &done);
        }

        if(!done)
            render();
    }

    /* Print out some timing information */
    now = SDL_GetTicks();
    if(now > then)
    {
        SDL_Log("%2.2f frames per second\n", ((double)frames * 1000) / (now - then));
    }
    quit(0);
    return 0;
}

#endif
