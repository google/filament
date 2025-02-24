//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// VulkanHelper.cpp : Helper for allocating & managing vulkan external objects.

#include "test_utils/VulkanHelper.h"

#include <vector>

#include "common/bitset_utils.h"
#include "common/debug.h"
#include "common/system_utils.h"
#include "common/vulkan/vulkan_icd.h"
#include "util/util_gl.h"
#include "vulkan/vulkan_core.h"

namespace angle
{

namespace
{

std::vector<VkPhysicalDevice> EnumeratePhysicalDevices(VkInstance instance)
{
    uint32_t physicalDeviceCount;
    VkResult result = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
    ASSERT(result == VK_SUCCESS);
    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    result = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());
    return physicalDevices;
}

std::vector<VkExtensionProperties> EnumerateDeviceExtensionProperties(
    VkPhysicalDevice physicalDevice,
    const char *layerName)
{
    uint32_t deviceExtensionCount;
    VkResult result = vkEnumerateDeviceExtensionProperties(physicalDevice, layerName,
                                                           &deviceExtensionCount, nullptr);
    ASSERT(result == VK_SUCCESS);
    std::vector<VkExtensionProperties> deviceExtensionProperties(deviceExtensionCount);
    result = vkEnumerateDeviceExtensionProperties(physicalDevice, layerName, &deviceExtensionCount,
                                                  deviceExtensionProperties.data());
    ASSERT(result == VK_SUCCESS);
    return deviceExtensionProperties;
}

std::vector<VkQueueFamilyProperties> GetPhysicalDeviceQueueFamilyProperties(
    VkPhysicalDevice physicalDevice)
{
    uint32_t queueFamilyPropertyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, nullptr);
    std::vector<VkQueueFamilyProperties> physicalDeviceQueueFamilyProperties(
        queueFamilyPropertyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount,
                                             physicalDeviceQueueFamilyProperties.data());
    return physicalDeviceQueueFamilyProperties;
}

bool HasExtension(const std::vector<VkExtensionProperties> instanceExtensions,
                  const char *extensionName)
{
    for (const auto &extensionProperties : instanceExtensions)
    {
        if (!strcmp(extensionProperties.extensionName, extensionName))
            return true;
    }

    return false;
}

bool HasExtension(const std::vector<const char *> enabledExtensions, const char *extensionName)
{
    for (const char *enabledExtension : enabledExtensions)
    {
        if (!strcmp(enabledExtension, extensionName))
            return true;
    }

    return false;
}

bool HasExtension(const char *const *enabledExtensions, const char *extensionName)
{
    size_t i = 0;
    while (enabledExtensions[i])
    {
        if (!strcmp(enabledExtensions[i], extensionName))
            return true;
        i++;
    }
    return false;
}

uint32_t FindMemoryType(const VkPhysicalDeviceMemoryProperties &memoryProperties,
                        uint32_t memoryTypeBits,
                        VkMemoryPropertyFlags requiredMemoryPropertyFlags)
{
    for (size_t memoryIndex : angle::BitSet32<32>(memoryTypeBits))
    {
        ASSERT(memoryIndex < memoryProperties.memoryTypeCount);

        if ((memoryProperties.memoryTypes[memoryIndex].propertyFlags &
             requiredMemoryPropertyFlags) == requiredMemoryPropertyFlags)
        {
            return static_cast<uint32_t>(memoryIndex);
        }
    }

    return UINT32_MAX;
}

void ImageMemoryBarrier(VkCommandBuffer commandBuffer,
                        VkImage image,
                        uint32_t srcQueueFamilyIndex,
                        uint32_t dstQueueFamilyIndex,
                        VkImageLayout oldLayout,
                        VkImageLayout newLayout)
{
    const VkImageMemoryBarrier imageMemoryBarriers[] = {
        /* [0] = */ {/* .sType = */ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                     /* .pNext = */ nullptr,
                     /* .srcAccessMask = */ VK_ACCESS_MEMORY_WRITE_BIT,
                     /* .dstAccessMask = */ VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
                     /* .oldLayout = */ oldLayout,
                     /* .newLayout = */ newLayout,
                     /* .srcQueueFamilyIndex = */ srcQueueFamilyIndex,
                     /* .dstQueueFamilyIndex = */ dstQueueFamilyIndex,
                     /* .image = */ image,
                     /* .subresourceRange = */
                     {
                         /* .aspectMask = */ VK_IMAGE_ASPECT_COLOR_BIT,
                         /* .basicMiplevel = */ 0,
                         /* .levelCount = */ 1,
                         /* .baseArrayLayer = */ 0,
                         /* .layerCount = */ 1,
                     }}};
    const uint32_t imageMemoryBarrierCount = std::extent<decltype(imageMemoryBarriers)>();

    constexpr VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    constexpr VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    const VkDependencyFlags dependencyFlags     = 0;

    vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, 0, nullptr, 0,
                         nullptr, imageMemoryBarrierCount, imageMemoryBarriers);
}

}  // namespace

void VulkanQueueMutex::init(EGLDisplay dpy)
{
    display = dpy;
}

void VulkanQueueMutex::lock()
{
    eglLockVulkanQueueANGLE(display);
    ASSERT_EGL_SUCCESS();
}

void VulkanQueueMutex::unlock()
{
    eglUnlockVulkanQueueANGLE(display);
    ASSERT_EGL_SUCCESS();
}

VulkanHelper::VulkanHelper() {}

VulkanHelper::~VulkanHelper()
{
    if (mDevice != VK_NULL_HANDLE)
    {
        vkDeviceWaitIdle(mDevice);
    }

    if (mCommandPool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
    }

    if (!mInitializedFromANGLE)
    {
        if (mDevice != VK_NULL_HANDLE)
        {
            vkDestroyDevice(mDevice, nullptr);

            mDevice        = VK_NULL_HANDLE;
            mGraphicsQueue = VK_NULL_HANDLE;
        }

        if (mInstance != VK_NULL_HANDLE)
        {
            vkDestroyInstance(mInstance, nullptr);

            mInstance = VK_NULL_HANDLE;
        }
    }
}

void VulkanHelper::initialize(bool useSwiftshader, bool enableValidationLayers)
{
    bool enableValidationLayersOverride = enableValidationLayers;
#if !defined(ANGLE_ENABLE_VULKAN_VALIDATION_LAYERS)
    enableValidationLayersOverride = false;
#endif

    vk::ICD icd = useSwiftshader ? vk::ICD::SwiftShader : vk::ICD::Default;

    vk::ScopedVkLoaderEnvironment scopedEnvironment(enableValidationLayersOverride, icd);

    ASSERT(mInstance == VK_NULL_HANDLE);
    VkResult result = VK_SUCCESS;
#if ANGLE_SHARED_LIBVULKAN
    result = volkInitialize();
    ASSERT(result == VK_SUCCESS);
#endif  // ANGLE_SHARED_LIBVULKAN

    VkApplicationInfo applicationInfo = {
        /* .sType = */ VK_STRUCTURE_TYPE_APPLICATION_INFO,
        /* .pNext = */ nullptr,
        /* .pApplicationName = */ "ANGLE Tests",
        /* .applicationVersion = */ 1,
        /* .pEngineName = */ nullptr,
        /* .engineVersion = */ 0,
        /* .apiVersion = */ VK_API_VERSION_1_1,
    };

    std::vector<const char *> enabledLayerNames;
    if (enableValidationLayersOverride)
    {
        enabledLayerNames.push_back("VK_LAYER_KHRONOS_validation");
    }

    VkInstanceCreateInfo instanceCreateInfo = {
        /* .sType = */ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        /* .pNext = */ nullptr,
        /* .flags = */ 0,
        /* .pApplicationInfo = */ &applicationInfo,
        /* .enabledLayerCount = */ static_cast<uint32_t>(enabledLayerNames.size()),
        /* .ppEnabledLayerNames = */ enabledLayerNames.data(),
        /* .enabledExtensionCount = */ 0,
        /* .ppEnabledExtensionName = */ nullptr,
    };

    result = vkCreateInstance(&instanceCreateInfo, nullptr, &mInstance);
    ASSERT(result == VK_SUCCESS);
    ASSERT(mInstance != VK_NULL_HANDLE);
#if ANGLE_SHARED_LIBVULKAN
    volkLoadInstance(mInstance);
#endif  // ANGLE_SHARED_LIBVULKAN

    std::vector<VkPhysicalDevice> physicalDevices = EnumeratePhysicalDevices(mInstance);

    ASSERT(physicalDevices.size() > 0);

    VkPhysicalDeviceProperties2 physicalDeviceProperties2;
    VkPhysicalDeviceIDProperties physicalDeviceIDProperties;
    VkPhysicalDeviceDriverProperties driverProperties;
    ChoosePhysicalDevice(vkGetPhysicalDeviceProperties2, physicalDevices, icd, 0, 0, nullptr,
                         nullptr, static_cast<VkDriverId>(0), &mPhysicalDevice,
                         &physicalDeviceProperties2, &physicalDeviceIDProperties,
                         &driverProperties);

    vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &mMemoryProperties);

    std::vector<VkExtensionProperties> deviceExtensionProperties =
        EnumerateDeviceExtensionProperties(mPhysicalDevice, nullptr);

    std::vector<const char *> requestedDeviceExtensions = {
        VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME, VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME,
        VK_FUCHSIA_EXTERNAL_MEMORY_EXTENSION_NAME,   VK_FUCHSIA_EXTERNAL_SEMAPHORE_EXTENSION_NAME,
        VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME,
    };

    std::vector<const char *> enabledDeviceExtensions;

    for (const char *extensionName : requestedDeviceExtensions)
    {
        if (HasExtension(deviceExtensionProperties, extensionName))
        {
            enabledDeviceExtensions.push_back(extensionName);
        }
    }

    std::vector<VkQueueFamilyProperties> queueFamilyProperties =
        GetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice);

    for (uint32_t i = 0; i < queueFamilyProperties.size(); ++i)
    {
        if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            mGraphicsQueueFamilyIndex = i;
        }
    }
    ASSERT(mGraphicsQueueFamilyIndex != UINT32_MAX);

    constexpr uint32_t kQueueCreateInfoCount           = 1;
    constexpr uint32_t kGraphicsQueueCount             = 1;
    float graphicsQueuePriorities[kGraphicsQueueCount] = {0.f};

    VkDeviceQueueCreateInfo queueCreateInfos[kQueueCreateInfoCount] = {
        /* [0] = */ {
            /* .sType = */ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            /* .pNext = */ nullptr,
            /* .flags = */ 0,
            /* .queueFamilyIndex = */ mGraphicsQueueFamilyIndex,
            /* .queueCount = */ 1,
            /* .pQueuePriorities = */ graphicsQueuePriorities,
        },
    };

    uint32_t enabledDeviceExtensionCount = static_cast<uint32_t>(enabledDeviceExtensions.size());

    VkDeviceCreateInfo deviceCreateInfo = {
        /* .sType = */ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        /* .pNext = */ nullptr,
        /* .flags = */ 0,
        /* .queueCreateInfoCount = */ kQueueCreateInfoCount,
        /* .pQueueCreateInfos = */ queueCreateInfos,
        /* .enabledLayerCount = */ 0,
        /* .ppEnabledLayerNames = */ nullptr,
        /* .enabledExtensionCount = */ enabledDeviceExtensionCount,
        /* .ppEnabledExtensionName = */ enabledDeviceExtensions.data(),
        /* .pEnabledFeatures = */ nullptr,
    };

    result = vkCreateDevice(mPhysicalDevice, &deviceCreateInfo, nullptr, &mDevice);
    ASSERT(result == VK_SUCCESS);
    ASSERT(mDevice != VK_NULL_HANDLE);
#if ANGLE_SHARED_LIBVULKAN
    volkLoadDevice(mDevice);
    vkGetPhysicalDeviceExternalSemaphoreProperties =
        reinterpret_cast<PFN_vkGetPhysicalDeviceExternalSemaphoreProperties>(
            vkGetInstanceProcAddr(mInstance, "vkGetPhysicalDeviceExternalSemaphoreProperties"));
    ASSERT(vkGetPhysicalDeviceExternalSemaphoreProperties);
#endif  // ANGLE_SHARED_LIBVULKAN

    constexpr uint32_t kGraphicsQueueIndex = 0;
    static_assert(kGraphicsQueueIndex < kGraphicsQueueCount, "must be in range");
    vkGetDeviceQueue(mDevice, mGraphicsQueueFamilyIndex, kGraphicsQueueIndex, &mGraphicsQueue);
    ASSERT(mGraphicsQueue != VK_NULL_HANDLE);

    VkCommandPoolCreateInfo commandPoolCreateInfo = {
        /* .sType = */ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        /* .pNext = */ nullptr,
        /* .flags = */ 0,
        /* .queueFamilyIndex = */ mGraphicsQueueFamilyIndex,
    };
    result = vkCreateCommandPool(mDevice, &commandPoolCreateInfo, nullptr, &mCommandPool);
    ASSERT(result == VK_SUCCESS);

    mHasExternalMemoryFd =
        HasExtension(enabledDeviceExtensions, VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
    mHasExternalSemaphoreFd =
        HasExtension(enabledDeviceExtensions, VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME);
    mHasExternalMemoryFuchsia =
        HasExtension(enabledDeviceExtensions, VK_FUCHSIA_EXTERNAL_MEMORY_EXTENSION_NAME);
    mHasExternalSemaphoreFuchsia =
        HasExtension(enabledDeviceExtensions, VK_FUCHSIA_EXTERNAL_SEMAPHORE_EXTENSION_NAME);

    vkGetPhysicalDeviceImageFormatProperties2 =
        reinterpret_cast<PFN_vkGetPhysicalDeviceImageFormatProperties2>(
            vkGetInstanceProcAddr(mInstance, "vkGetPhysicalDeviceImageFormatProperties2"));
    ASSERT(vkGetPhysicalDeviceImageFormatProperties2);
    vkGetMemoryFdKHR = reinterpret_cast<PFN_vkGetMemoryFdKHR>(
        vkGetInstanceProcAddr(mInstance, "vkGetMemoryFdKHR"));
    ASSERT(!mHasExternalMemoryFd || vkGetMemoryFdKHR);
    vkGetSemaphoreFdKHR = reinterpret_cast<PFN_vkGetSemaphoreFdKHR>(
        vkGetInstanceProcAddr(mInstance, "vkGetSemaphoreFdKHR"));
    ASSERT(!mHasExternalSemaphoreFd || vkGetSemaphoreFdKHR);
    vkGetMemoryZirconHandleFUCHSIA = reinterpret_cast<PFN_vkGetMemoryZirconHandleFUCHSIA>(
        vkGetInstanceProcAddr(mInstance, "vkGetMemoryZirconHandleFUCHSIA"));
    ASSERT(!mHasExternalMemoryFuchsia || vkGetMemoryZirconHandleFUCHSIA);
    vkGetSemaphoreZirconHandleFUCHSIA = reinterpret_cast<PFN_vkGetSemaphoreZirconHandleFUCHSIA>(
        vkGetInstanceProcAddr(mInstance, "vkGetSemaphoreZirconHandleFUCHSIA"));
    ASSERT(!mHasExternalSemaphoreFuchsia || vkGetSemaphoreZirconHandleFUCHSIA);
}

void VulkanHelper::initializeFromANGLE()
{
    mInitializedFromANGLE = true;
    VkResult vkResult     = VK_SUCCESS;

    EXPECT_TRUE(IsEGLClientExtensionEnabled("EGL_EXT_device_query"));
    EGLDisplay display = eglGetCurrentDisplay();

    EGLAttrib result = 0;
    EXPECT_EGL_TRUE(eglQueryDisplayAttribEXT(display, EGL_DEVICE_EXT, &result));

    EGLDeviceEXT device = reinterpret_cast<EGLDeviceEXT>(result);
    EXPECT_NE(EGL_NO_DEVICE_EXT, device);
    EXPECT_TRUE(IsEGLDeviceExtensionEnabled(device, "EGL_ANGLE_device_vulkan"));

    EXPECT_EGL_TRUE(eglQueryDeviceAttribEXT(device, EGL_VULKAN_GET_INSTANCE_PROC_ADDR, &result));
    PFN_vkGetInstanceProcAddr getInstanceProcAddr =
        reinterpret_cast<PFN_vkGetInstanceProcAddr>(result);
    EXPECT_NE(getInstanceProcAddr, nullptr);
#if ANGLE_SHARED_LIBVULKAN
    volkInitializeCustom(getInstanceProcAddr);
#endif  // ANGLE_SHARED_LIBVULKAN

    EXPECT_EGL_TRUE(eglQueryDeviceAttribEXT(device, EGL_VULKAN_INSTANCE_ANGLE, &result));
    mInstance = reinterpret_cast<VkInstance>(result);
    EXPECT_NE(mInstance, static_cast<VkInstance>(VK_NULL_HANDLE));

#if ANGLE_SHARED_LIBVULKAN
    volkLoadInstance(mInstance);
#endif  // ANGLE_SHARED_LIBVULKAN

    EXPECT_EGL_TRUE(eglQueryDeviceAttribEXT(device, EGL_VULKAN_PHYSICAL_DEVICE_ANGLE, &result));
    mPhysicalDevice = reinterpret_cast<VkPhysicalDevice>(result);
    EXPECT_NE(mPhysicalDevice, static_cast<VkPhysicalDevice>(VK_NULL_HANDLE));

    vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &mMemoryProperties);

    EXPECT_EGL_TRUE(eglQueryDeviceAttribEXT(device, EGL_VULKAN_DEVICE_ANGLE, &result));
    mDevice = reinterpret_cast<VkDevice>(result);
    EXPECT_NE(mDevice, static_cast<VkDevice>(VK_NULL_HANDLE));

    EXPECT_EGL_TRUE(eglQueryDeviceAttribEXT(device, EGL_VULKAN_QUEUE_ANGLE, &result));
    mGraphicsQueue = reinterpret_cast<VkQueue>(result);
    EXPECT_NE(mGraphicsQueue, static_cast<VkQueue>(VK_NULL_HANDLE));

    mGraphicsQueueMutex.init(display);

    EXPECT_EGL_TRUE(eglQueryDeviceAttribEXT(device, EGL_VULKAN_QUEUE_FAMILIY_INDEX_ANGLE, &result));
    mGraphicsQueueFamilyIndex = static_cast<uint32_t>(result);

    EXPECT_EGL_TRUE(eglQueryDeviceAttribEXT(device, EGL_VULKAN_DEVICE_EXTENSIONS_ANGLE, &result));
    const char *const *enabledDeviceExtensions = reinterpret_cast<const char *const *>(result);

    mHasExternalMemoryFd =
        HasExtension(enabledDeviceExtensions, VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
    mHasExternalSemaphoreFd =
        HasExtension(enabledDeviceExtensions, VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME);
    mHasExternalMemoryFuchsia =
        HasExtension(enabledDeviceExtensions, VK_FUCHSIA_EXTERNAL_MEMORY_EXTENSION_NAME);
    mHasExternalSemaphoreFuchsia =
        HasExtension(enabledDeviceExtensions, VK_FUCHSIA_EXTERNAL_SEMAPHORE_EXTENSION_NAME);

#if ANGLE_SHARED_LIBVULKAN
    volkLoadDevice(mDevice);
    vkGetPhysicalDeviceExternalSemaphoreProperties =
        reinterpret_cast<PFN_vkGetPhysicalDeviceExternalSemaphoreProperties>(
            vkGetInstanceProcAddr(mInstance, "vkGetPhysicalDeviceExternalSemaphoreProperties"));
    ASSERT(vkGetPhysicalDeviceExternalSemaphoreProperties);
#endif  // ANGLE_SHARED_LIBVULKAN

    VkCommandPoolCreateInfo commandPoolCreateInfo = {
        /* .sType = */ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        /* .pNext = */ nullptr,
        /* .flags = */ 0,
        /* .queueFamilyIndex = */ mGraphicsQueueFamilyIndex,
    };
    vkResult = vkCreateCommandPool(mDevice, &commandPoolCreateInfo, nullptr, &mCommandPool);
    ASSERT(vkResult == VK_SUCCESS);

    vkGetPhysicalDeviceImageFormatProperties2 =
        reinterpret_cast<PFN_vkGetPhysicalDeviceImageFormatProperties2>(
            vkGetInstanceProcAddr(mInstance, "vkGetPhysicalDeviceImageFormatProperties2"));
    ASSERT(vkGetPhysicalDeviceImageFormatProperties2);
    vkGetMemoryFdKHR = reinterpret_cast<PFN_vkGetMemoryFdKHR>(
        vkGetInstanceProcAddr(mInstance, "vkGetMemoryFdKHR"));
    ASSERT(!mHasExternalMemoryFd || vkGetMemoryFdKHR);
    vkGetSemaphoreFdKHR = reinterpret_cast<PFN_vkGetSemaphoreFdKHR>(
        vkGetInstanceProcAddr(mInstance, "vkGetSemaphoreFdKHR"));
    ASSERT(!mHasExternalSemaphoreFd || vkGetSemaphoreFdKHR);
    vkGetMemoryZirconHandleFUCHSIA = reinterpret_cast<PFN_vkGetMemoryZirconHandleFUCHSIA>(
        vkGetInstanceProcAddr(mInstance, "vkGetMemoryZirconHandleFUCHSIA"));
    ASSERT(!mHasExternalMemoryFuchsia || vkGetMemoryZirconHandleFUCHSIA);
    vkGetSemaphoreZirconHandleFUCHSIA = reinterpret_cast<PFN_vkGetSemaphoreZirconHandleFUCHSIA>(
        vkGetInstanceProcAddr(mInstance, "vkGetSemaphoreZirconHandleFUCHSIA"));
    ASSERT(!mHasExternalSemaphoreFuchsia || vkGetSemaphoreZirconHandleFUCHSIA);
}

VkResult VulkanHelper::createImage2D(VkFormat format,
                                     VkImageCreateFlags createFlags,
                                     VkImageUsageFlags usageFlags,
                                     VkExtent3D extent,
                                     VkImage *imageOut,
                                     VkDeviceMemory *deviceMemoryOut,
                                     VkDeviceSize *deviceMemorySizeOut,
                                     VkImageCreateInfo *imageCreateInfoOut)
{
    VkImageCreateInfo imageCreateInfo = {
        /* .sType = */ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        /* .pNext = */ nullptr,
        /* .flags = */ createFlags,
        /* .imageType = */ VK_IMAGE_TYPE_2D,
        /* .format = */ format,
        /* .extent = */ extent,
        /* .mipLevels = */ 1,
        /* .arrayLayers = */ 1,
        /* .samples = */ VK_SAMPLE_COUNT_1_BIT,
        /* .tiling = */ VK_IMAGE_TILING_OPTIMAL,
        /* .usage = */ usageFlags,
        /* .sharingMode = */ VK_SHARING_MODE_EXCLUSIVE,
        /* .queueFamilyIndexCount = */ 0,
        /* .pQueueFamilyIndices = */ nullptr,
        /* initialLayout = */ VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VkImage image   = VK_NULL_HANDLE;
    VkResult result = vkCreateImage(mDevice, &imageCreateInfo, nullptr, &image);
    if (result != VK_SUCCESS)
    {
        return result;
    }

    VkMemoryPropertyFlags requestedMemoryPropertyFlags = 0;
    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(mDevice, image, &memoryRequirements);
    uint32_t memoryTypeIndex = FindMemoryType(mMemoryProperties, memoryRequirements.memoryTypeBits,
                                              requestedMemoryPropertyFlags);
    ASSERT(memoryTypeIndex != UINT32_MAX);
    VkDeviceSize deviceMemorySize = memoryRequirements.size;

    VkMemoryAllocateInfo memoryAllocateInfo = {
        /* .sType = */ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        /* .pNext = */ nullptr,
        /* .allocationSize = */ deviceMemorySize,
        /* .memoryTypeIndex = */ memoryTypeIndex,
    };

    VkDeviceMemory deviceMemory = VK_NULL_HANDLE;
    result = vkAllocateMemory(mDevice, &memoryAllocateInfo, nullptr, &deviceMemory);
    if (result != VK_SUCCESS)
    {
        vkDestroyImage(mDevice, image, nullptr);
        return result;
    }

    VkDeviceSize memoryOffset = 0;
    result                    = vkBindImageMemory(mDevice, image, deviceMemory, memoryOffset);
    if (result != VK_SUCCESS)
    {
        vkFreeMemory(mDevice, deviceMemory, nullptr);
        vkDestroyImage(mDevice, image, nullptr);
        return result;
    }

    *imageOut            = image;
    *deviceMemoryOut     = deviceMemory;
    *deviceMemorySizeOut = deviceMemorySize;
    *imageCreateInfoOut  = imageCreateInfo;

    return VK_SUCCESS;
}

bool VulkanHelper::canCreateImageExternal(VkFormat format,
                                          VkImageType type,
                                          VkImageTiling tiling,
                                          VkImageCreateFlags createFlags,
                                          VkImageUsageFlags usageFlags,
                                          VkExternalMemoryHandleTypeFlagBits handleType) const
{
    VkPhysicalDeviceExternalImageFormatInfo externalImageFormatInfo = {
        /* .sType = */ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO,
        /* .pNext = */ nullptr,
        /* .handleType = */ handleType,
    };

    VkPhysicalDeviceImageFormatInfo2 imageFormatInfo = {
        /* .sType = */ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2,
        /* .pNext = */ &externalImageFormatInfo,
        /* .format = */ format,
        /* .type = */ type,
        /* .tiling = */ tiling,
        /* .usage = */ usageFlags,
        /* .flags = */ createFlags,
    };

    VkExternalImageFormatProperties externalImageFormatProperties = {
        /* .sType = */ VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES,
        /* .pNext = */ nullptr,
    };

    VkImageFormatProperties2 imageFormatProperties = {
        /* .sType = */ VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2,
        /* .pNext = */ &externalImageFormatProperties};

    VkResult result = vkGetPhysicalDeviceImageFormatProperties2(mPhysicalDevice, &imageFormatInfo,
                                                                &imageFormatProperties);
    if (result == VK_ERROR_FORMAT_NOT_SUPPORTED)
    {
        return false;
    }

    ASSERT(result == VK_SUCCESS);

    constexpr VkExternalMemoryFeatureFlags kRequiredFeatures =
        VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT | VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT;
    if ((externalImageFormatProperties.externalMemoryProperties.externalMemoryFeatures &
         kRequiredFeatures) != kRequiredFeatures)
    {
        return false;
    }

    return true;
}

VkResult VulkanHelper::createImage2DExternal(VkFormat format,
                                             VkImageCreateFlags createFlags,
                                             VkImageUsageFlags usageFlags,
                                             const void *imageCreateInfoPNext,
                                             VkExtent3D extent,
                                             VkExternalMemoryHandleTypeFlags handleTypes,
                                             VkImage *imageOut,
                                             VkDeviceMemory *deviceMemoryOut,
                                             VkDeviceSize *deviceMemorySizeOut)
{
    VkExternalMemoryImageCreateInfo externalMemoryImageCreateInfo = {
        /* .sType = */ VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO,
        /* .pNext = */ imageCreateInfoPNext,
        /* .handleTypes = */ handleTypes,
    };

    VkImageCreateInfo imageCreateInfo = {
        /* .sType = */ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        /* .pNext = */ &externalMemoryImageCreateInfo,
        /* .flags = */ createFlags,
        /* .imageType = */ VK_IMAGE_TYPE_2D,
        /* .format = */ format,
        /* .extent = */ extent,
        /* .mipLevels = */ 1,
        /* .arrayLayers = */ 1,
        /* .samples = */ VK_SAMPLE_COUNT_1_BIT,
        /* .tiling = */ VK_IMAGE_TILING_OPTIMAL,
        /* .usage = */ usageFlags,
        /* .sharingMode = */ VK_SHARING_MODE_EXCLUSIVE,
        /* .queueFamilyIndexCount = */ 0,
        /* .pQueueFamilyIndices = */ nullptr,
        /* initialLayout = */ VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VkImage image   = VK_NULL_HANDLE;
    VkResult result = vkCreateImage(mDevice, &imageCreateInfo, nullptr, &image);
    if (result != VK_SUCCESS)
    {
        return result;
    }

    VkMemoryPropertyFlags requestedMemoryPropertyFlags = 0;
    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(mDevice, image, &memoryRequirements);
    uint32_t memoryTypeIndex = FindMemoryType(mMemoryProperties, memoryRequirements.memoryTypeBits,
                                              requestedMemoryPropertyFlags);
    ASSERT(memoryTypeIndex != UINT32_MAX);
    VkDeviceSize deviceMemorySize = memoryRequirements.size;

    VkExportMemoryAllocateInfo exportMemoryAllocateInfo = {
        /* .sType = */ VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO,
        /* .pNext = */ nullptr,
        /* .handleTypes = */ handleTypes,
    };
    VkMemoryDedicatedAllocateInfo memoryDedicatedAllocateInfo = {
        /* .sType = */ VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO,
        /* .pNext = */ &exportMemoryAllocateInfo,
        /* .image = */ image,
    };
    VkMemoryAllocateInfo memoryAllocateInfo = {
        /* .sType = */ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        /* .pNext = */ &memoryDedicatedAllocateInfo,
        /* .allocationSize = */ deviceMemorySize,
        /* .memoryTypeIndex = */ memoryTypeIndex,
    };

    VkDeviceMemory deviceMemory = VK_NULL_HANDLE;
    result = vkAllocateMemory(mDevice, &memoryAllocateInfo, nullptr, &deviceMemory);
    if (result != VK_SUCCESS)
    {
        vkDestroyImage(mDevice, image, nullptr);
        return result;
    }

    VkDeviceSize memoryOffset = 0;
    result                    = vkBindImageMemory(mDevice, image, deviceMemory, memoryOffset);
    if (result != VK_SUCCESS)
    {
        vkFreeMemory(mDevice, deviceMemory, nullptr);
        vkDestroyImage(mDevice, image, nullptr);
        return result;
    }

    *imageOut            = image;
    *deviceMemoryOut     = deviceMemory;
    *deviceMemorySizeOut = deviceMemorySize;

    return VK_SUCCESS;
}

bool VulkanHelper::canCreateImageOpaqueFd(VkFormat format,
                                          VkImageType type,
                                          VkImageTiling tiling,
                                          VkImageCreateFlags createFlags,
                                          VkImageUsageFlags usageFlags) const
{
    if (!mHasExternalMemoryFd)
    {
        return false;
    }

    return canCreateImageExternal(format, type, tiling, createFlags, usageFlags,
                                  VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT);
}

VkResult VulkanHelper::createImage2DOpaqueFd(VkFormat format,
                                             VkImageCreateFlags createFlags,
                                             VkImageUsageFlags usageFlags,
                                             const void *imageCreateInfoPNext,
                                             VkExtent3D extent,
                                             VkImage *imageOut,
                                             VkDeviceMemory *deviceMemoryOut,
                                             VkDeviceSize *deviceMemorySizeOut)
{
    return createImage2DExternal(format, createFlags, usageFlags, imageCreateInfoPNext, extent,
                                 VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT, imageOut,
                                 deviceMemoryOut, deviceMemorySizeOut);
}

VkResult VulkanHelper::exportMemoryOpaqueFd(VkDeviceMemory deviceMemory, int *fd)
{
    VkMemoryGetFdInfoKHR memoryGetFdInfo = {
        /* .sType = */ VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR,
        /* .pNext = */ nullptr,
        /* .memory = */ deviceMemory,
        /* .handleType = */ VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT,
    };

    return vkGetMemoryFdKHR(mDevice, &memoryGetFdInfo, fd);
}

bool VulkanHelper::canCreateImageZirconVmo(VkFormat format,
                                           VkImageType type,
                                           VkImageTiling tiling,
                                           VkImageCreateFlags createFlags,
                                           VkImageUsageFlags usageFlags) const
{
    if (!mHasExternalMemoryFuchsia)
    {
        return false;
    }

    return canCreateImageExternal(format, type, tiling, createFlags, usageFlags,
                                  VK_EXTERNAL_MEMORY_HANDLE_TYPE_ZIRCON_VMO_BIT_FUCHSIA);
}

VkResult VulkanHelper::createImage2DZirconVmo(VkFormat format,
                                              VkImageCreateFlags createFlags,
                                              VkImageUsageFlags usageFlags,
                                              const void *imageCreateInfoPNext,
                                              VkExtent3D extent,
                                              VkImage *imageOut,
                                              VkDeviceMemory *deviceMemoryOut,
                                              VkDeviceSize *deviceMemorySizeOut)
{
    return createImage2DExternal(format, createFlags, usageFlags, imageCreateInfoPNext, extent,
                                 VK_EXTERNAL_MEMORY_HANDLE_TYPE_ZIRCON_VMO_BIT_FUCHSIA, imageOut,
                                 deviceMemoryOut, deviceMemorySizeOut);
}

VkResult VulkanHelper::exportMemoryZirconVmo(VkDeviceMemory deviceMemory, zx_handle_t *vmo)
{
    VkMemoryGetZirconHandleInfoFUCHSIA memoryGetZirconHandleInfo = {
        /* .sType = */ VK_STRUCTURE_TYPE_MEMORY_GET_ZIRCON_HANDLE_INFO_FUCHSIA,
        /* .pNext = */ nullptr,
        /* .memory = */ deviceMemory,
        /* .handleType = */ VK_EXTERNAL_MEMORY_HANDLE_TYPE_ZIRCON_VMO_BIT_FUCHSIA,
    };

    return vkGetMemoryZirconHandleFUCHSIA(mDevice, &memoryGetZirconHandleInfo, vmo);
}

bool VulkanHelper::canCreateSemaphoreOpaqueFd() const
{
    if (!mHasExternalSemaphoreFd)
    {
        return false;
    }

    VkPhysicalDeviceExternalSemaphoreInfo externalSemaphoreInfo = {
        /* .sType = */ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_INFO,
        /* .pNext = */ nullptr,
        /* .handleType = */ VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT,
    };

    VkExternalSemaphoreProperties externalSemaphoreProperties = {
        /* .sType = */ VK_STRUCTURE_TYPE_EXTERNAL_SEMAPHORE_PROPERTIES,
    };
    vkGetPhysicalDeviceExternalSemaphoreProperties(mPhysicalDevice, &externalSemaphoreInfo,
                                                   &externalSemaphoreProperties);

    constexpr VkExternalSemaphoreFeatureFlags kRequiredFeatures =
        VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT | VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT;

    if ((externalSemaphoreProperties.externalSemaphoreFeatures & kRequiredFeatures) !=
        kRequiredFeatures)
    {
        return false;
    }

    return true;
}

VkResult VulkanHelper::createSemaphoreOpaqueFd(VkSemaphore *semaphore)
{
    VkExportSemaphoreCreateInfo exportSemaphoreCreateInfo = {
        /* .sType = */ VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO,
        /* .pNext = */ nullptr,
        /* .handleTypes = */ VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT,
    };

    VkSemaphoreCreateInfo semaphoreCreateInfo = {
        /* .sType = */ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        /* .pNext = */ &exportSemaphoreCreateInfo,
        /* .flags = */ 0,
    };

    return vkCreateSemaphore(mDevice, &semaphoreCreateInfo, nullptr, semaphore);
}

VkResult VulkanHelper::exportSemaphoreOpaqueFd(VkSemaphore semaphore, int *fd)
{
    VkSemaphoreGetFdInfoKHR semaphoreGetFdInfo = {
        /* .sType = */ VK_STRUCTURE_TYPE_SEMAPHORE_GET_FD_INFO_KHR,
        /* .pNext = */ nullptr,
        /* .semaphore = */ semaphore,
        /* .handleType = */ VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT,
    };

    return vkGetSemaphoreFdKHR(mDevice, &semaphoreGetFdInfo, fd);
}

bool VulkanHelper::canCreateSemaphoreZirconEvent() const
{
    if (!mHasExternalSemaphoreFuchsia)
    {
        return false;
    }

    VkPhysicalDeviceExternalSemaphoreInfo externalSemaphoreInfo = {
        /* .sType = */ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_INFO,
        /* .pNext = */ nullptr,
        /* .handleType = */ VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_ZIRCON_EVENT_BIT_FUCHSIA,
    };

    VkExternalSemaphoreProperties externalSemaphoreProperties = {
        /* .sType = */ VK_STRUCTURE_TYPE_EXTERNAL_SEMAPHORE_PROPERTIES,
    };
    vkGetPhysicalDeviceExternalSemaphoreProperties(mPhysicalDevice, &externalSemaphoreInfo,
                                                   &externalSemaphoreProperties);

    constexpr VkExternalSemaphoreFeatureFlags kRequiredFeatures =
        VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT | VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT;

    if ((externalSemaphoreProperties.externalSemaphoreFeatures & kRequiredFeatures) !=
        kRequiredFeatures)
    {
        return false;
    }

    return true;
}

VkResult VulkanHelper::createSemaphoreZirconEvent(VkSemaphore *semaphore)
{
    VkExportSemaphoreCreateInfo exportSemaphoreCreateInfo = {
        /* .sType = */ VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO,
        /* .pNext = */ nullptr,
        /* .handleTypes = */ VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_ZIRCON_EVENT_BIT_FUCHSIA,
    };

    VkSemaphoreCreateInfo semaphoreCreateInfo = {
        /* .sType = */ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        /* .pNext = */ &exportSemaphoreCreateInfo,
        /* .flags = */ 0,
    };

    return vkCreateSemaphore(mDevice, &semaphoreCreateInfo, nullptr, semaphore);
}

VkResult VulkanHelper::exportSemaphoreZirconEvent(VkSemaphore semaphore, zx_handle_t *event)
{
    VkSemaphoreGetZirconHandleInfoFUCHSIA semaphoreGetZirconHandleInfo = {
        /* .sType = */ VK_STRUCTURE_TYPE_SEMAPHORE_GET_ZIRCON_HANDLE_INFO_FUCHSIA,
        /* .pNext = */ nullptr,
        /* .semaphore = */ semaphore,
        /* .handleType = */ VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_ZIRCON_EVENT_BIT_FUCHSIA,
    };

    return vkGetSemaphoreZirconHandleFUCHSIA(mDevice, &semaphoreGetZirconHandleInfo, event);
}

void VulkanHelper::releaseImageAndSignalSemaphore(VkImage image,
                                                  VkImageLayout oldLayout,
                                                  VkImageLayout newLayout,
                                                  VkSemaphore semaphore)
{
    VkResult result;

    VkCommandBuffer commandBuffers[]                      = {VK_NULL_HANDLE};
    constexpr uint32_t commandBufferCount                 = std::extent<decltype(commandBuffers)>();
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
        /* .sType = */ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        /* .pNext = */ nullptr,
        /* .commandPool = */ mCommandPool,
        /* .level = */ VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        /* .commandBufferCount = */ commandBufferCount,
    };

    result = vkAllocateCommandBuffers(mDevice, &commandBufferAllocateInfo, commandBuffers);
    ASSERT(result == VK_SUCCESS);

    VkCommandBufferBeginInfo commandBufferBeginInfo = {
        /* .sType = */ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        /* .pNext = */ nullptr,
        /* .flags = */ VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        /* .pInheritanceInfo = */ nullptr,
    };
    result = vkBeginCommandBuffer(commandBuffers[0], &commandBufferBeginInfo);
    ASSERT(result == VK_SUCCESS);

    ImageMemoryBarrier(commandBuffers[0], image, mGraphicsQueueFamilyIndex,
                       VK_QUEUE_FAMILY_EXTERNAL, oldLayout, newLayout);

    result = vkEndCommandBuffer(commandBuffers[0]);
    ASSERT(result == VK_SUCCESS);

    const VkSemaphore signalSemaphores[] = {
        semaphore,
    };
    constexpr uint32_t signalSemaphoreCount = std::extent<decltype(signalSemaphores)>();

    const VkSubmitInfo submits[] = {
        /* [0] = */ {
            /* .sType */ VK_STRUCTURE_TYPE_SUBMIT_INFO,
            /* .pNext = */ nullptr,
            /* .waitSemaphoreCount = */ 0,
            /* .pWaitSemaphores = */ nullptr,
            /* .pWaitDstStageMask = */ nullptr,
            /* .commandBufferCount = */ commandBufferCount,
            /* .pCommandBuffers = */ commandBuffers,
            /* .signalSemaphoreCount = */ signalSemaphoreCount,
            /* .pSignalSemaphores = */ signalSemaphores,
        },
    };
    constexpr uint32_t submitCount = std::extent<decltype(submits)>();

    std::unique_lock<VulkanQueueMutex> queueLock = getGraphicsQueueLock();
    const VkFence fence = VK_NULL_HANDLE;
    result              = vkQueueSubmit(mGraphicsQueue, submitCount, submits, fence);
    ASSERT(result == VK_SUCCESS);
}

void VulkanHelper::signalSemaphore(VkSemaphore semaphore)
{
    VkResult result;

    const VkSemaphore signalSemaphores[] = {
        semaphore,
    };
    constexpr uint32_t signalSemaphoreCount = std::extent<decltype(signalSemaphores)>();

    const VkSubmitInfo submits[] = {
        /* [0] = */ {
            /* .sType */ VK_STRUCTURE_TYPE_SUBMIT_INFO,
            /* .pNext = */ nullptr,
            /* .waitSemaphoreCount = */ 0,
            /* .pWaitSemaphores = */ nullptr,
            /* .pWaitDstStageMask = */ nullptr,
            /* .commandBufferCount = */ 0,
            /* .pCommandBuffers = */ nullptr,
            /* .signalSemaphoreCount = */ signalSemaphoreCount,
            /* .pSignalSemaphores = */ signalSemaphores,
        },
    };
    constexpr uint32_t submitCount = std::extent<decltype(submits)>();

    std::unique_lock<VulkanQueueMutex> queueLock = getGraphicsQueueLock();
    const VkFence fence = VK_NULL_HANDLE;
    result              = vkQueueSubmit(mGraphicsQueue, submitCount, submits, fence);
    ASSERT(result == VK_SUCCESS);
}

void VulkanHelper::waitSemaphoreAndAcquireImage(VkImage image,
                                                VkImageLayout oldLayout,
                                                VkImageLayout newLayout,
                                                VkSemaphore semaphore)
{
    VkResult result;

    VkCommandBuffer commandBuffers[]                      = {VK_NULL_HANDLE};
    constexpr uint32_t commandBufferCount                 = std::extent<decltype(commandBuffers)>();
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
        /* .sType = */ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        /* .pNext = */ nullptr,
        /* .commandPool = */ mCommandPool,
        /* .level = */ VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        /* .commandBufferCount = */ commandBufferCount,
    };

    result = vkAllocateCommandBuffers(mDevice, &commandBufferAllocateInfo, commandBuffers);
    ASSERT(result == VK_SUCCESS);

    VkCommandBufferBeginInfo commandBufferBeginInfo = {
        /* .sType = */ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        /* .pNext = */ nullptr,
        /* .flags = */ VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        /* .pInheritanceInfo = */ nullptr,
    };
    result = vkBeginCommandBuffer(commandBuffers[0], &commandBufferBeginInfo);
    ASSERT(result == VK_SUCCESS);

    ImageMemoryBarrier(commandBuffers[0], image, VK_QUEUE_FAMILY_EXTERNAL,
                       mGraphicsQueueFamilyIndex, oldLayout, newLayout);

    result = vkEndCommandBuffer(commandBuffers[0]);
    ASSERT(result == VK_SUCCESS);

    const VkSemaphore waitSemaphores[] = {
        semaphore,
    };
    const VkPipelineStageFlags waitDstStageMasks[] = {
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
    };
    constexpr uint32_t waitSemaphoreCount    = std::extent<decltype(waitSemaphores)>();
    constexpr uint32_t waitDstStageMaskCount = std::extent<decltype(waitDstStageMasks)>();
    static_assert(waitSemaphoreCount == waitDstStageMaskCount,
                  "waitSemaphores and waitDstStageMasks must be the same length");

    const VkSubmitInfo submits[] = {
        /* [0] = */ {
            /* .sType */ VK_STRUCTURE_TYPE_SUBMIT_INFO,
            /* .pNext = */ nullptr,
            /* .waitSemaphoreCount = */ waitSemaphoreCount,
            /* .pWaitSemaphores = */ waitSemaphores,
            /* .pWaitDstStageMask = */ waitDstStageMasks,
            /* .commandBufferCount = */ commandBufferCount,
            /* .pCommandBuffers = */ commandBuffers,
            /* .signalSemaphoreCount = */ 0,
            /* .pSignalSemaphores = */ nullptr,
        },
    };
    constexpr uint32_t submitCount = std::extent<decltype(submits)>();

    std::unique_lock<VulkanQueueMutex> queueLock = getGraphicsQueueLock();
    const VkFence fence = VK_NULL_HANDLE;
    result              = vkQueueSubmit(mGraphicsQueue, submitCount, submits, fence);
    ASSERT(result == VK_SUCCESS);
}

void VulkanHelper::readPixels(VkImage srcImage,
                              VkImageLayout srcImageLayout,
                              VkFormat srcImageFormat,
                              VkOffset3D imageOffset,
                              VkExtent3D imageExtent,
                              void *pixels,
                              size_t pixelsSize)
{
    ASSERT(srcImageFormat == VK_FORMAT_B8G8R8A8_UNORM ||
           srcImageFormat == VK_FORMAT_R8G8B8A8_UNORM);
    ASSERT(imageExtent.depth == 1);
    ASSERT(pixelsSize == 4 * imageExtent.width * imageExtent.height);

    VkBufferCreateInfo bufferCreateInfo = {
        /* .sType = */ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        /* .pNext = */ nullptr,
        /* .flags = */ 0,
        /* .size = */ pixelsSize,
        /* .usage = */ VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        /* .sharingMode = */ VK_SHARING_MODE_EXCLUSIVE,
        /* .queueFamilyIndexCount = */ 0,
        /* .pQueueFamilyIndices = */ nullptr,
    };
    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VkResult result        = vkCreateBuffer(mDevice, &bufferCreateInfo, nullptr, &stagingBuffer);
    ASSERT(result == VK_SUCCESS);

    VkMemoryPropertyFlags requestedMemoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(mDevice, stagingBuffer, &memoryRequirements);
    uint32_t memoryTypeIndex = FindMemoryType(mMemoryProperties, memoryRequirements.memoryTypeBits,
                                              requestedMemoryPropertyFlags);
    ASSERT(memoryTypeIndex != UINT32_MAX);
    VkDeviceSize deviceMemorySize = memoryRequirements.size;

    VkMemoryDedicatedAllocateInfo memoryDedicatedAllocateInfo = {
        /* .sType = */ VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO,
        /* .pNext = */ nullptr,
        /* .image = */ VK_NULL_HANDLE,
        /* .buffer = */ stagingBuffer,
    };
    VkMemoryAllocateInfo memoryAllocateInfo = {
        /* .sType = */ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        /* .pNext = */ &memoryDedicatedAllocateInfo,
        /* .allocationSize = */ deviceMemorySize,
        /* .memoryTypeIndex = */ memoryTypeIndex,
    };

    VkDeviceMemory deviceMemory = VK_NULL_HANDLE;
    result = vkAllocateMemory(mDevice, &memoryAllocateInfo, nullptr, &deviceMemory);
    ASSERT(result == VK_SUCCESS);

    result = vkBindBufferMemory(mDevice, stagingBuffer, deviceMemory, 0 /* memoryOffset */);
    ASSERT(result == VK_SUCCESS);

    VkCommandBuffer commandBuffers[]                      = {VK_NULL_HANDLE};
    constexpr uint32_t commandBufferCount                 = std::extent<decltype(commandBuffers)>();
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
        /* .sType = */ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        /* .pNext = */ nullptr,
        /* .commandPool = */ mCommandPool,
        /* .level = */ VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        /* .commandBufferCount = */ commandBufferCount,
    };

    result = vkAllocateCommandBuffers(mDevice, &commandBufferAllocateInfo, commandBuffers);
    ASSERT(result == VK_SUCCESS);

    VkCommandBufferBeginInfo commandBufferBeginInfo = {
        /* .sType = */ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        /* .pNext = */ nullptr,
        /* .flags = */ VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        /* .pInheritanceInfo = */ nullptr,
    };
    result = vkBeginCommandBuffer(commandBuffers[0], &commandBufferBeginInfo);
    ASSERT(result == VK_SUCCESS);

    VkBufferImageCopy bufferImageCopies[] = {
        /* [0] = */ {
            /* .bufferOffset = */ 0,
            /* .bufferRowLength = */ 0,
            /* .bufferImageHeight = */ 0,
            /* .imageSubresources = */
            {
                /* .aspectMask = */ VK_IMAGE_ASPECT_COLOR_BIT,
                /* .mipLevel = */ 0,
                /* .baseArrayLayer = */ 0,
                /* .layerCount = */ 1,
            },
            /* .imageOffset = */ imageOffset,
            /* .imageExtent = */ imageExtent,
        },
    };
    constexpr uint32_t bufferImageCopyCount = std::extent<decltype(bufferImageCopies)>();

    if (srcImageLayout != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
    {
        VkImageMemoryBarrier imageMemoryBarriers = {
            /* .sType = */ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            /* .pNext = */ nullptr,
            /* .srcAccessMask = */ VK_ACCESS_TRANSFER_WRITE_BIT,
            /* .dstAccessMask = */ VK_ACCESS_TRANSFER_READ_BIT,
            /* .oldLayout = */ srcImageLayout,
            /* .newLayout = */ VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            /* .srcQueueFamilyIndex = */ mGraphicsQueueFamilyIndex,
            /* .dstQueueFamilyIndex = */ mGraphicsQueueFamilyIndex,
            /* .image = */ srcImage,
            /* .subresourceRange = */
            {
                /* .aspectMask = */ VK_IMAGE_ASPECT_COLOR_BIT,
                /* .baseMipLevel = */ 0,
                /* .levelCount = */ 1,
                /* .baseArrayLayer = */ 0,
                /* .layerCount = */ 1,
            },

        };
        vkCmdPipelineBarrier(commandBuffers[0], VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1,
                             &imageMemoryBarriers);
        srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    }

    vkCmdCopyImageToBuffer(commandBuffers[0], srcImage, srcImageLayout, stagingBuffer,
                           bufferImageCopyCount, bufferImageCopies);

    VkMemoryBarrier memoryBarriers[] = {
        /* [0] = */ {/* .sType = */ VK_STRUCTURE_TYPE_MEMORY_BARRIER,
                     /* .pNext = */ nullptr,
                     /* .srcAccessMask = */ VK_ACCESS_MEMORY_WRITE_BIT,
                     /* .dstAccessMask = */ VK_ACCESS_HOST_READ_BIT},
    };
    constexpr uint32_t memoryBarrierCount = std::extent<decltype(memoryBarriers)>();
    vkCmdPipelineBarrier(commandBuffers[0], VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_HOST_BIT, 0 /* dependencyFlags */, memoryBarrierCount,
                         memoryBarriers, 0, nullptr, 0, nullptr);

    result = vkEndCommandBuffer(commandBuffers[0]);
    ASSERT(result == VK_SUCCESS);

    const VkSubmitInfo submits[] = {
        /* [0] = */ {
            /* .sType */ VK_STRUCTURE_TYPE_SUBMIT_INFO,
            /* .pNext = */ nullptr,
            /* .waitSemaphoreCount = */ 0,
            /* .pWaitSemaphores = */ nullptr,
            /* .pWaitDstStageMask = */ nullptr,
            /* .commandBufferCount = */ commandBufferCount,
            /* .pCommandBuffers = */ commandBuffers,
            /* .signalSemaphoreCount = */ 0,
            /* .pSignalSemaphores = */ nullptr,
        },
    };
    constexpr uint32_t submitCount = std::extent<decltype(submits)>();

    {
        std::unique_lock<VulkanQueueMutex> queueLock = getGraphicsQueueLock();
        const VkFence fence                          = VK_NULL_HANDLE;
        result = vkQueueSubmit(mGraphicsQueue, submitCount, submits, fence);
        ASSERT(result == VK_SUCCESS);

        result = vkQueueWaitIdle(mGraphicsQueue);
        ASSERT(result == VK_SUCCESS);
    }

    vkFreeCommandBuffers(mDevice, mCommandPool, commandBufferCount, commandBuffers);

    void *stagingMemory = nullptr;
    result = vkMapMemory(mDevice, deviceMemory, 0 /* offset */, deviceMemorySize, 0 /* flags */,
                         &stagingMemory);
    ASSERT(result == VK_SUCCESS);

    VkMappedMemoryRange memoryRanges[] = {
        /* [0] = */ {
            /* .sType = */ VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
            /* .pNext = */ nullptr,
            /* .memory = */ deviceMemory,
            /* .offset = */ 0,
            /* .size = */ deviceMemorySize,
        },
    };
    constexpr uint32_t memoryRangeCount = std::extent<decltype(memoryRanges)>();

    result = vkInvalidateMappedMemoryRanges(mDevice, memoryRangeCount, memoryRanges);
    ASSERT(result == VK_SUCCESS);

    memcpy(pixels, stagingMemory, pixelsSize);

    vkDestroyBuffer(mDevice, stagingBuffer, nullptr);

    vkUnmapMemory(mDevice, deviceMemory);
    vkFreeMemory(mDevice, deviceMemory, nullptr);
}

void VulkanHelper::writePixels(VkImage dstImage,
                               VkImageLayout imageLayout,
                               VkFormat imageFormat,
                               VkOffset3D imageOffset,
                               VkExtent3D imageExtent,
                               const void *pixels,
                               size_t pixelsSize)
{
    ASSERT(imageFormat == VK_FORMAT_B8G8R8A8_UNORM || imageFormat == VK_FORMAT_R8G8B8A8_UNORM);
    ASSERT(imageExtent.depth == 1);
    ASSERT(pixelsSize == 4 * imageExtent.width * imageExtent.height);

    VkBufferCreateInfo bufferCreateInfo = {
        /* .sType = */ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        /* .pNext = */ nullptr,
        /* .flags = */ 0,
        /* .size = */ pixelsSize,
        /* .usage = */ VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        /* .sharingMode = */ VK_SHARING_MODE_EXCLUSIVE,
        /* .queueFamilyIndexCount = */ 0,
        /* .pQueueFamilyIndices = */ nullptr,
    };
    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VkResult result        = vkCreateBuffer(mDevice, &bufferCreateInfo, nullptr, &stagingBuffer);
    ASSERT(result == VK_SUCCESS);

    VkMemoryPropertyFlags requestedMemoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(mDevice, stagingBuffer, &memoryRequirements);
    uint32_t memoryTypeIndex = FindMemoryType(mMemoryProperties, memoryRequirements.memoryTypeBits,
                                              requestedMemoryPropertyFlags);
    ASSERT(memoryTypeIndex != UINT32_MAX);
    VkDeviceSize deviceMemorySize = memoryRequirements.size;

    VkMemoryDedicatedAllocateInfo memoryDedicatedAllocateInfo = {
        /* .sType = */ VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO,
        /* .pNext = */ nullptr,
        /* .image = */ VK_NULL_HANDLE,
        /* .buffer = */ stagingBuffer,
    };
    VkMemoryAllocateInfo memoryAllocateInfo = {
        /* .sType = */ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        /* .pNext = */ &memoryDedicatedAllocateInfo,
        /* .allocationSize = */ deviceMemorySize,
        /* .memoryTypeIndex = */ memoryTypeIndex,
    };

    VkDeviceMemory deviceMemory = VK_NULL_HANDLE;
    result = vkAllocateMemory(mDevice, &memoryAllocateInfo, nullptr, &deviceMemory);
    ASSERT(result == VK_SUCCESS);

    result = vkBindBufferMemory(mDevice, stagingBuffer, deviceMemory, 0 /* memoryOffset */);
    ASSERT(result == VK_SUCCESS);

    void *stagingMemory = nullptr;
    result = vkMapMemory(mDevice, deviceMemory, 0 /* offset */, deviceMemorySize, 0 /* flags */,
                         &stagingMemory);
    ASSERT(result == VK_SUCCESS);

    VkMappedMemoryRange memoryRanges[] = {
        /* [0] = */ {
            /* .sType = */ VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
            /* .pNext = */ nullptr,
            /* .memory = */ deviceMemory,
            /* .offset = */ 0,
            /* .size = */ deviceMemorySize,
        },
    };
    constexpr uint32_t memoryRangeCount = std::extent<decltype(memoryRanges)>();

    result = vkInvalidateMappedMemoryRanges(mDevice, memoryRangeCount, memoryRanges);
    ASSERT(result == VK_SUCCESS);

    memcpy(stagingMemory, pixels, pixelsSize);

    vkUnmapMemory(mDevice, deviceMemory);

    VkCommandBuffer commandBuffers[]                      = {VK_NULL_HANDLE};
    constexpr uint32_t commandBufferCount                 = std::extent<decltype(commandBuffers)>();
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
        /* .sType = */ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        /* .pNext = */ nullptr,
        /* .commandPool = */ mCommandPool,
        /* .level = */ VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        /* .commandBufferCount = */ commandBufferCount,
    };

    result = vkAllocateCommandBuffers(mDevice, &commandBufferAllocateInfo, commandBuffers);
    ASSERT(result == VK_SUCCESS);

    VkCommandBufferBeginInfo commandBufferBeginInfo = {
        /* .sType = */ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        /* .pNext = */ nullptr,
        /* .flags = */ VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        /* .pInheritanceInfo = */ nullptr,
    };
    result = vkBeginCommandBuffer(commandBuffers[0], &commandBufferBeginInfo);
    ASSERT(result == VK_SUCCESS);

    // Memory barrier for pipeline from Host-Write to Transfer-Read.
    VkMemoryBarrier memoryBarriers[] = {
        /* [0] = */ {/* .sType = */ VK_STRUCTURE_TYPE_MEMORY_BARRIER,
                     /* .pNext = */ nullptr,
                     /* .srcAccessMask = */ VK_ACCESS_HOST_WRITE_BIT,
                     /* .dstAccessMask = */ VK_ACCESS_TRANSFER_READ_BIT},
    };
    constexpr uint32_t memoryBarrierCount = std::extent<decltype(memoryBarriers)>();
    vkCmdPipelineBarrier(commandBuffers[0], VK_PIPELINE_STAGE_HOST_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, 0 /* dependencyFlags */,
                         memoryBarrierCount, memoryBarriers, 0, nullptr, 0, nullptr);

    // Memory-barrier for image to Transfer-Write.
    if (imageLayout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        VkImageMemoryBarrier imageMemoryBarriers = {
            /* .sType = */ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            /* .pNext = */ nullptr,
            /* .srcAccessMask = */ VK_ACCESS_NONE,
            /* .dstAccessMask = */ VK_ACCESS_TRANSFER_WRITE_BIT,
            /* .oldLayout = */ imageLayout,
            /* .newLayout = */ VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            /* .srcQueueFamilyIndex = */ mGraphicsQueueFamilyIndex,
            /* .dstQueueFamilyIndex = */ mGraphicsQueueFamilyIndex,
            /* .image = */ dstImage,
            /* .subresourceRange = */
            {
                /* .aspectMask = */ VK_IMAGE_ASPECT_COLOR_BIT,
                /* .baseMipLevel = */ 0,
                /* .levelCount = */ 1,
                /* .baseArrayLayer = */ 0,
                /* .layerCount = */ 1,
            },

        };
        vkCmdPipelineBarrier(commandBuffers[0], VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1,
                             &imageMemoryBarriers);
        imageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    }

    // Issue the buffer to image copy.
    VkBufferImageCopy bufferImageCopies[] = {
        /* [0] = */ {
            /* .bufferOffset = */ 0,
            /* .bufferRowLength = */ 0,
            /* .bufferImageHeight = */ 0,
            /* .imageSubresources = */
            {
                /* .aspectMask = */ VK_IMAGE_ASPECT_COLOR_BIT,
                /* .mipLevel = */ 0,
                /* .baseArrayLayer = */ 0,
                /* .layerCount = */ 1,
            },
            /* .imageOffset = */ imageOffset,
            /* .imageExtent = */ imageExtent,
        },
    };

    constexpr uint32_t bufferImageCopyCount = std::extent<decltype(bufferImageCopies)>();

    vkCmdCopyBufferToImage(commandBuffers[0], stagingBuffer, dstImage, imageLayout,
                           bufferImageCopyCount, bufferImageCopies);

    result = vkEndCommandBuffer(commandBuffers[0]);
    ASSERT(result == VK_SUCCESS);

    const VkSubmitInfo submits[] = {
        /* [0] = */ {
            /* .sType */ VK_STRUCTURE_TYPE_SUBMIT_INFO,
            /* .pNext = */ nullptr,
            /* .waitSemaphoreCount = */ 0,
            /* .pWaitSemaphores = */ nullptr,
            /* .pWaitDstStageMask = */ nullptr,
            /* .commandBufferCount = */ commandBufferCount,
            /* .pCommandBuffers = */ commandBuffers,
            /* .signalSemaphoreCount = */ 0,
            /* .pSignalSemaphores = */ nullptr,
        },
    };
    constexpr uint32_t submitCount = std::extent<decltype(submits)>();

    {
        std::unique_lock<VulkanQueueMutex> queueLock = getGraphicsQueueLock();
        const VkFence fence                          = VK_NULL_HANDLE;
        result = vkQueueSubmit(mGraphicsQueue, submitCount, submits, fence);
        ASSERT(result == VK_SUCCESS);

        result = vkQueueWaitIdle(mGraphicsQueue);
        ASSERT(result == VK_SUCCESS);
    }

    vkFreeCommandBuffers(mDevice, mCommandPool, commandBufferCount, commandBuffers);
    vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
    vkFreeMemory(mDevice, deviceMemory, nullptr);
}

}  // namespace angle
