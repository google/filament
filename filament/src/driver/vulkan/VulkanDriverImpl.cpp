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

#include <bluevk/BlueVK.h> // must be included before vk_mem_alloc

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated"
#pragma clang diagnostic ignored "-Wundef"
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#pragma clang diagnostic ignored "-Wtautological-compare"
#define VMA_IMPLEMENTATION
#include <cstdio>
#include "vk_mem_alloc.h"
#pragma clang diagnostic pop

#include "driver/vulkan/VulkanDriverImpl.h"

#include <details/Texture.h> // for FTexture::getFormatSize

#include <utils/Panic.h>

namespace filament {
namespace driver {

void selectPhysicalDevice(VulkanContext& context) {
    uint32_t physicalDeviceCount = 0;
    VkResult result = vkEnumeratePhysicalDevices(context.instance, &physicalDeviceCount, nullptr);
    ASSERT_POSTCONDITION(result == VK_SUCCESS && physicalDeviceCount > 0,
            "vkEnumeratePhysicalDevices count error.");
    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    result = vkEnumeratePhysicalDevices(context.instance, &physicalDeviceCount,
            physicalDevices.data());
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkEnumeratePhysicalDevices error.");
    context.physicalDevice = nullptr;
    for (uint32_t i = 0; i < physicalDeviceCount; ++i) {
        VkPhysicalDevice physicalDevice = physicalDevices[i];
        vkGetPhysicalDeviceProperties(physicalDevice, &context.physicalDeviceProperties);

        // Does the device support Vulkan?
        if (VK_VERSION_MAJOR(context.physicalDeviceProperties.apiVersion) < 1) {
            continue;
        }

        // Does the device have any command queues that support graphics?
        // In theory we should also ensure that the device supports presentation of our
        // particular VkSurface, but we don't have a VkSurface yet so we'll skip this requirement.
        uint32_t queueFamiliesCount;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamiliesCount, nullptr);
        if (queueFamiliesCount == 0) {
            continue;
        }
        std::vector<VkQueueFamilyProperties> queueFamiliesProperties(queueFamiliesCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamiliesCount,
                queueFamiliesProperties.data());
        context.graphicsQueueFamilyIndex = 0xffff;
        for (uint32_t j = 0; j < queueFamiliesCount; ++j) {
            VkQueueFamilyProperties props = queueFamiliesProperties[j];
            if (props.queueCount == 0) {
                continue;
            }
            if (props.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                context.graphicsQueueFamilyIndex = j;
            }
        }
        if (context.graphicsQueueFamilyIndex == 0xffff) continue;

        // Does the device support the VK_KHR_swapchain extension?
        uint32_t extensionCount;
        result = vkEnumerateDeviceExtensionProperties(physicalDevice, /*pLayerName = */ nullptr,
                &extensionCount, nullptr);
        ASSERT_POSTCONDITION(result == VK_SUCCESS,
                "vkEnumerateDeviceExtensionProperties count error.");
        std::vector<VkExtensionProperties> extensions(extensionCount);
        result = vkEnumerateDeviceExtensionProperties(physicalDevice, /*pLayerName = */ nullptr,
                &extensionCount, extensions.data());
        ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkEnumerateDeviceExtensionProperties error.");
        bool supportsSwapchain = false;
        context.debugMarkersSupported = false;
        for (uint32_t k = 0; k < extensionCount; ++k) {
            if (!strcmp(extensions[k].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME)) {
                supportsSwapchain = true;
            }
            if (!strcmp(extensions[k].extensionName, VK_EXT_DEBUG_MARKER_EXTENSION_NAME)) {
                context.debugMarkersSupported = true;
            }
        }
        if (!supportsSwapchain) continue;

        // Bingo, we finally found a physical device that supports everything we need.
        context.physicalDevice = physicalDevice;
        vkGetPhysicalDeviceFeatures(physicalDevice, &context.physicalDeviceFeatures);
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &context.memoryProperties);

        // Print out some properties of the GPU for diagnostic purposes.
        //
        // Ideally, the vendors register their vendor ID's with Khronos so that apps can make an
        // id => string mapping. However in practice this hasn't happened. At the time of this
        // writing the gpuinfo database has the following ID's:
        //
        //     0x1002 - AMD
        //     0x1010 - ImgTec
        //     0x10DE - NVIDIA
        //     0x13B5 - ARM
        //     0x5143 - Qualcomm
        //     0x8086 - INTEL
        //
        // Since we don't have any vendor-specific workarounds yet, there's no need to make this
        // mapping in code. The "deviceName" string informally reveals the marketing name for the
        // GPU. (e.g., Quadro)
        const uint32_t apiVersion = context.physicalDeviceProperties.apiVersion;
        const uint32_t driverVersion = context.physicalDeviceProperties.driverVersion;
        const uint32_t vendorID = context.physicalDeviceProperties.vendorID;
        const uint32_t deviceID = context.physicalDeviceProperties.deviceID;
        utils::slog.i << "Selected physical device '"
                << context.physicalDeviceProperties.deviceName
                << "' from " << physicalDeviceCount << " physical devices. "
                << "(vendor 0x" << utils::io::hex << vendorID << ", "
                << "device 0x" << deviceID << ", "
                << "api 0x" << apiVersion << ", "
                << "driver 0x" << driverVersion << ")"
                << utils::io::dec << utils::io::endl;
        break;
    }
}

void createVirtualDevice(VulkanContext& context) {
    VkDeviceQueueCreateInfo deviceQueueCreateInfo[1] = {};
    static const float queuePriority[] = {1.0f};
    VkDeviceCreateInfo deviceCreateInfo = {};
    std::vector<const char*> deviceExtensionNames = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
    if (context.debugMarkersSupported) {
        deviceExtensionNames.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
    }
    deviceQueueCreateInfo->sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfo->queueFamilyIndex = context.graphicsQueueFamilyIndex;
    deviceQueueCreateInfo->queueCount = 1;
    deviceQueueCreateInfo->pQueuePriorities = &queuePriority[0];
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfo;
    deviceCreateInfo.pEnabledFeatures = nullptr;
    deviceCreateInfo.enabledExtensionCount = deviceExtensionNames.size();
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensionNames.data();
    VkResult result = vkCreateDevice(context.physicalDevice, &deviceCreateInfo, VKALLOC,
            &context.device);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkCreateDevice error.");
    vkGetDeviceQueue(context.device, context.graphicsQueueFamilyIndex, 0,
            &context.graphicsQueue);
    VkCommandPoolCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.flags =
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    createInfo.queueFamilyIndex = context.graphicsQueueFamilyIndex;
    result = vkCreateCommandPool(context.device, &createInfo, VKALLOC, &context.commandPool);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkCreateCommandPool error.");

    const VmaVulkanFunctions funcs {
        .vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties,
        .vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties,
        .vkAllocateMemory = vkAllocateMemory,
        .vkFreeMemory = vkFreeMemory,
        .vkMapMemory = vkMapMemory,
        .vkUnmapMemory = vkUnmapMemory,
        .vkBindBufferMemory = vkBindBufferMemory,
        .vkBindImageMemory = vkBindImageMemory,
        .vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements,
        .vkGetImageMemoryRequirements = vkGetImageMemoryRequirements,
        .vkCreateBuffer = vkCreateBuffer,
        .vkDestroyBuffer = vkDestroyBuffer,
        .vkCreateImage = vkCreateImage,
        .vkDestroyImage = vkDestroyImage,
        .vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR,
        .vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2KHR
    };
    const VmaAllocatorCreateInfo allocatorInfo {
        .physicalDevice = context.physicalDevice,
        .device = context.device,
        .pVulkanFunctions = &funcs
    };
    vmaCreateAllocator(&allocatorInfo, &context.allocator);
}

void createSemaphore(VkDevice device, VkSemaphore *semaphore) {
    VkSemaphoreCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkResult result = vkCreateSemaphore(device, &createInfo, VKALLOC, semaphore);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkCreateSemaphore error.");
}

void getPresentationQueue(VulkanContext& context, VulkanSurfaceContext& sc) {
    uint32_t queueFamiliesCount;
    vkGetPhysicalDeviceQueueFamilyProperties(context.physicalDevice, &queueFamiliesCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamiliesProperties(queueFamiliesCount);
    vkGetPhysicalDeviceQueueFamilyProperties(context.physicalDevice, &queueFamiliesCount,
            queueFamiliesProperties.data());
    uint32_t presentQueueFamilyIndex = 0xffff;
    for (uint32_t j = 0; j < queueFamiliesCount; ++j) {
        VkBool32 supported = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(context.physicalDevice, j, sc.surface, &supported);
        if (supported) {
            presentQueueFamilyIndex = j;
            break;
        }
    }
    ASSERT_POSTCONDITION(presentQueueFamilyIndex != 0xffff,
            "This physical device does not support the presentation queue.");
    if (context.graphicsQueueFamilyIndex != presentQueueFamilyIndex) {
        vkGetDeviceQueue(context.device, presentQueueFamilyIndex, 0, &sc.presentQueue);
    } else {
        sc.presentQueue = context.graphicsQueue;
    }
    ASSERT_POSTCONDITION(sc.presentQueue, "Unable to obtain presentation queue.");
}

void getSurfaceCaps(VulkanContext& context, VulkanSurfaceContext& sc) {
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context.physicalDevice,
            sc.surface, &sc.surfaceCapabilities);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR error.");
    ASSERT_POSTCONDITION(
            sc.surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            "Vulkan surface doesn't support VK_IMAGE_USAGE_TRANSFER_DST_BIT.");
    uint32_t surfaceFormatsCount;
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(context.physicalDevice, sc.surface,
            &surfaceFormatsCount, nullptr);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkGetPhysicalDeviceSurfaceFormatsKHR count error.");
    sc.surfaceFormats.resize(surfaceFormatsCount);
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(context.physicalDevice, sc.surface,
            &surfaceFormatsCount, sc.surfaceFormats.data());
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkGetPhysicalDeviceSurfaceFormatsKHR error.");
}

void createSwapChainAndImages(VulkanContext& context, VulkanSurfaceContext& surfaceContext) {
    // Pick an image count and format.  According to section 30.5 of VK 1.1, maxImageCount of zero
    // apparently means "that there is no limit on the number of images, though there may be limits
    // related to the total amount of memory used by presentable images."
    uint32_t desiredImageCount = 2;
    const uint32_t maxImageCount = surfaceContext.surfaceCapabilities.maxImageCount;
    if (desiredImageCount < surfaceContext.surfaceCapabilities.minImageCount ||
            (maxImageCount != 0 && desiredImageCount > maxImageCount)) {
        utils::slog.e << "Swap chain does not support " << desiredImageCount << " images.\n";
        desiredImageCount = surfaceContext.surfaceCapabilities.minImageCount;
    }
    surfaceContext.surfaceFormat = surfaceContext.surfaceFormats[0];
    for (const VkSurfaceFormatKHR& format : surfaceContext.surfaceFormats) {
        if (format.format == VK_FORMAT_R8G8B8A8_UNORM) {
            surfaceContext.surfaceFormat = format;
            break;
        }
    }
    const auto compositionCaps = surfaceContext.surfaceCapabilities.supportedCompositeAlpha;
    const auto compositeAlpha = (compositionCaps & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) ?
            VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR : VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    // Create the low-level swap chain.
    const auto size = surfaceContext.surfaceCapabilities.currentExtent;
    VkSwapchainCreateInfoKHR createInfo {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surfaceContext.surface,
        .minImageCount = desiredImageCount,
        .imageFormat = surfaceContext.surfaceFormat.format,
        .imageColorSpace = surfaceContext.surfaceFormat.colorSpace,
        .imageExtent = size,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = compositeAlpha,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        .clipped = VK_TRUE
    };
    VkSwapchainKHR swapchain;
    VkResult result = vkCreateSwapchainKHR(context.device, &createInfo, VKALLOC, &swapchain);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkGetPhysicalDeviceSurfaceFormatsKHR error.");
    surfaceContext.swapchain = swapchain;

    // Extract the VkImage handles from the swap chain.
    uint32_t imageCount;
    result = vkGetSwapchainImagesKHR(context.device, swapchain, &imageCount, nullptr);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkGetSwapchainImagesKHR count error.");
    surfaceContext.swapContexts.resize(imageCount);
    std::vector<VkImage> images(imageCount);
    result = vkGetSwapchainImagesKHR(context.device, swapchain, &imageCount,
            images.data());
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkGetSwapchainImagesKHR error.");
    for (size_t i = 0; i < images.size(); ++i) {
        surfaceContext.swapContexts[i].attachment = {
            .image = images[i],
            .format = surfaceContext.surfaceFormat.format
        };
    }
    utils::slog.i
            << "vkCreateSwapchain"
            << ": " << size.width << "x" << size.height
            << ", " << surfaceContext.surfaceFormat.format
            << ", " << surfaceContext.surfaceFormat.colorSpace
            << ", " << imageCount
            << utils::io::endl;

    // Create image views.
    VkImageViewCreateInfo ivCreateInfo = {};
    ivCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ivCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivCreateInfo.format = surfaceContext.surfaceFormat.format;
    ivCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ivCreateInfo.subresourceRange.levelCount = 1;
    ivCreateInfo.subresourceRange.layerCount = 1;
    for (size_t i = 0; i < images.size(); ++i) {
        ivCreateInfo.image = images[i];
        result = vkCreateImageView(context.device, &ivCreateInfo, VKALLOC,
                &surfaceContext.swapContexts[i].attachment.view);
        ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkCreateImageView error.");
    }

    createSemaphore(context.device, &surfaceContext.imageAvailable);
    createSemaphore(context.device, &surfaceContext.renderingFinished);

    surfaceContext.depth = {};
}

void createDepthBuffer(VulkanContext& context, VulkanSurfaceContext& surfaceContext,
        VkFormat depthFormat) {
    assert(context.cmdbuffer);

    // Create an appropriately-sized device-only VkImage.
    const auto size = surfaceContext.surfaceCapabilities.currentExtent;
    VkImage depthImage;
    VkImageCreateInfo imageInfo {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .extent = { size.width, size.height, 1 },
        .format = depthFormat,
        .mipLevels = 1,
        .arrayLayers = 1,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .samples = VK_SAMPLE_COUNT_1_BIT,
    };
    VkResult error = vkCreateImage(context.device, &imageInfo, VKALLOC, &depthImage);
    ASSERT_POSTCONDITION(!error, "Unable to create depth image.");

    // Allocate memory for the VkImage and bind it.
    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(context.device, depthImage, &memReqs);
    VkMemoryAllocateInfo allocInfo {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memReqs.size,
        .memoryTypeIndex = selectMemoryType(context, memReqs.memoryTypeBits,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
    };
    error = vkAllocateMemory(context.device, &allocInfo, nullptr,
            &surfaceContext.depth.memory);
    ASSERT_POSTCONDITION(!error, "Unable to allocate depth memory.");
    error = vkBindImageMemory(context.device, depthImage, surfaceContext.depth.memory, 0);
    ASSERT_POSTCONDITION(!error, "Unable to bind depth memory.");

    // Create a VkImageView so that we can attach depth to the framebuffer.
    VkImageView depthView;
    VkImageViewCreateInfo viewInfo {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = depthImage,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = depthFormat,
        .subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
        .subresourceRange.levelCount = 1,
        .subresourceRange.layerCount = 1,
    };
    error = vkCreateImageView(context.device, &viewInfo, VKALLOC, &depthView);
    ASSERT_POSTCONDITION(!error, "Unable to create depth view.");

    // Transition the depth image into an optimal layout.
    VkImageMemoryBarrier barrier {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = depthImage,
        .subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
        .subresourceRange.levelCount = 1,
        .subresourceRange.layerCount = 1,
        .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
    };
    vkCmdPipelineBarrier(context.cmdbuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    // Go ahead and set the depth attachment fields, which serves as a signal to VulkanDriver that
    // it is now ready.
    surfaceContext.depth.view = depthView;
    surfaceContext.depth.image = depthImage;
    surfaceContext.depth.format = depthFormat;
}

void transitionDepthBuffer(VulkanContext& context, VulkanSurfaceContext& sc, VkFormat depthFormat) {
    // Begin a new command buffer solely for the purpose of transitioning the image layout.
    SwapContext& swap = getSwapContext(context);
    VkResult result = vkWaitForFences(context.device, 1, &swap.fence, VK_FALSE, UINT64_MAX);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkWaitForFences error.");
    result = vkResetFences(context.device, 1, &swap.fence);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkResetFences error.");
    VkCommandBuffer cmdbuffer = swap.cmdbuffer;
    result = vkResetCommandBuffer(cmdbuffer, 0);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkResetCommandBuffer error.");
    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
    };
    result = vkBeginCommandBuffer(cmdbuffer, &beginInfo);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkBeginCommandBuffer error.");
    context.cmdbuffer = cmdbuffer;

    // Create the depth buffer and issue a pipeline barrier command.
    createDepthBuffer(context, sc, depthFormat);

    // Flush the command buffer.
    result = vkEndCommandBuffer(context.cmdbuffer);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkEndCommandBuffer error.");
    context.cmdbuffer = nullptr;
    VkSubmitInfo submitInfo {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &swap.cmdbuffer,
    };
    result = vkQueueSubmit(context.graphicsQueue, 1, &submitInfo, swap.fence);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkQueueSubmit error.");
    swap.submitted = false;
}

void createCommandBuffersAndFences(VulkanContext& context, VulkanSurfaceContext& surfaceContext) {
    // Allocate command buffers.
    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = context.commandPool;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = (uint32_t) surfaceContext.swapContexts.size();
    std::vector<VkCommandBuffer> cmdbufs(allocateInfo.commandBufferCount);
    VkResult result = vkAllocateCommandBuffers(context.device, &allocateInfo, cmdbufs.data());
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkAllocateCommandBuffers error.");
    for (uint32_t i = 0; i < allocateInfo.commandBufferCount; ++i) {
        surfaceContext.swapContexts[i].cmdbuffer = cmdbufs[i];
    }

    // Create fences.
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    for (uint32_t i = 0; i < allocateInfo.commandBufferCount; i++) {
        result = vkCreateFence(context.device, &fenceCreateInfo, VKALLOC,
                &surfaceContext.swapContexts[i].fence);
        ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkCreateFence error.");
    }
}

void destroySurfaceContext(VulkanContext& context, VulkanSurfaceContext& surfaceContext) {
    for (SwapContext& swapContext : surfaceContext.swapContexts) {
        vkFreeCommandBuffers(context.device, context.commandPool, 1, &swapContext.cmdbuffer);
        vkDestroyFence(context.device, swapContext.fence, VKALLOC);
        vkDestroyImageView(context.device, swapContext.attachment.view, VKALLOC);
        swapContext.fence = VK_NULL_HANDLE;
        swapContext.attachment.view = VK_NULL_HANDLE;
    }
    vkDestroySwapchainKHR(context.device, surfaceContext.swapchain, VKALLOC);
    vkDestroySemaphore(context.device, surfaceContext.imageAvailable, VKALLOC);
    vkDestroySemaphore(context.device, surfaceContext.renderingFinished, VKALLOC);
    vkDestroySurfaceKHR(context.instance, surfaceContext.surface, VKALLOC);
    vkDestroyImageView(context.device, surfaceContext.depth.view, VKALLOC);
    vkDestroyImage(context.device, surfaceContext.depth.image, VKALLOC);
    vkFreeMemory(context.device, surfaceContext.depth.memory, VKALLOC);
    if (context.currentSurface == &surfaceContext) {
        context.currentSurface = nullptr;
    }
}

uint32_t selectMemoryType(VulkanContext& context, uint32_t flags, VkFlags reqs) {
    for (uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; i++) {
        if (flags & 1) {
            if ((context.memoryProperties.memoryTypes[i].propertyFlags & reqs) == reqs) {
                return i;
            }
        }
        flags >>= 1;
    }
    ASSERT_POSTCONDITION(false, "Unable to find a memory type that meets requirements.");
    return (uint32_t) ~0ul;
}

VkFormat getVkFormat(ElementType type, bool normalized) {
    using ElementType = ElementType;
    if (normalized) {
        switch (type) {
            // Single Component Types
            case ElementType::BYTE: return VK_FORMAT_R8_SNORM;
            case ElementType::UBYTE: return VK_FORMAT_R8_UNORM;
            case ElementType::SHORT: return VK_FORMAT_R16_SNORM;
            case ElementType::USHORT: return VK_FORMAT_R16_UNORM;
            // Two Component Types
            case ElementType::BYTE2: return VK_FORMAT_R8G8_SNORM;
            case ElementType::UBYTE2: return VK_FORMAT_R8G8_UNORM;
            case ElementType::SHORT2: return VK_FORMAT_R16G16_SNORM;
            case ElementType::USHORT2: return VK_FORMAT_R16G16_UNORM;
            // Three Component Types
            case ElementType::BYTE3: return VK_FORMAT_R8G8B8_SNORM;
            case ElementType::UBYTE3: return VK_FORMAT_R8G8B8_UNORM;
            case ElementType::SHORT3: return VK_FORMAT_R16G16B16_SNORM;
            case ElementType::USHORT3: return VK_FORMAT_R16G16B16_UNORM;
            // Four Component Types
            case ElementType::BYTE4: return VK_FORMAT_R8G8B8A8_SNORM;
            case ElementType::UBYTE4: return VK_FORMAT_R8G8B8A8_UNORM;
            case ElementType::SHORT4: return VK_FORMAT_R16G16B16A16_SNORM;
            case ElementType::USHORT4: return VK_FORMAT_R16G16B16A16_UNORM;
            default:
                ASSERT_POSTCONDITION(false, "Normalized format does not exist.");
                return VK_FORMAT_UNDEFINED;
        }
    }
    switch (type) {
        // Single Component Types
        case ElementType::BYTE: return VK_FORMAT_R8_SINT;
        case ElementType::UBYTE: return VK_FORMAT_R8_UINT;
        case ElementType::SHORT: return VK_FORMAT_R16_SINT;
        case ElementType::USHORT: return VK_FORMAT_R16_UINT;
        case ElementType::HALF: return VK_FORMAT_R16_SFLOAT;
        case ElementType::INT: return VK_FORMAT_R32_SINT;
        case ElementType::UINT: return VK_FORMAT_R32_UINT;
        case ElementType::FLOAT: return VK_FORMAT_R32_SFLOAT;
        // Two Component Types
        case ElementType::BYTE2: return VK_FORMAT_R8G8_SINT;
        case ElementType::UBYTE2: return VK_FORMAT_R8G8_UINT;
        case ElementType::SHORT2: return VK_FORMAT_R16G16_SINT;
        case ElementType::USHORT2: return VK_FORMAT_R16G16_UINT;
        case ElementType::HALF2: return VK_FORMAT_R16G16_SFLOAT;
        case ElementType::FLOAT2: return VK_FORMAT_R32G32_SFLOAT;
        // Three Component Types
        case ElementType::BYTE3: return VK_FORMAT_R8G8B8_SINT;
        case ElementType::UBYTE3: return VK_FORMAT_R8G8B8_UINT;
        case ElementType::SHORT3: return VK_FORMAT_R16G16B16_SINT;
        case ElementType::USHORT3: return VK_FORMAT_R16G16B16_UINT;
        case ElementType::HALF3: return VK_FORMAT_R16G16B16_SFLOAT;
        case ElementType::FLOAT3: return VK_FORMAT_R32G32B32_SFLOAT;
        // Four Component Types
        case ElementType::BYTE4: return VK_FORMAT_R8G8B8A8_SINT;
        case ElementType::UBYTE4: return VK_FORMAT_R8G8B8A8_UINT;
        case ElementType::SHORT4: return VK_FORMAT_R16G16B16A16_SINT;
        case ElementType::USHORT4: return VK_FORMAT_R16G16B16A16_UINT;
        case ElementType::HALF4: return VK_FORMAT_R16G16B16A16_SFLOAT;
        case ElementType::FLOAT4: return VK_FORMAT_R32G32B32A32_SFLOAT;
    }
    return VK_FORMAT_UNDEFINED;
}

VkFormat getVkFormat(TextureFormat format) {
    using TextureFormat = TextureFormat;
    switch (format) {
        // 8 bits per element.
        case TextureFormat::R8:                return VK_FORMAT_R8_UNORM;
        case TextureFormat::R8_SNORM:          return VK_FORMAT_R8_SNORM;
        case TextureFormat::R8UI:              return VK_FORMAT_R8_UINT;
        case TextureFormat::R8I:               return VK_FORMAT_R8_SINT;
        case TextureFormat::STENCIL8:          return VK_FORMAT_S8_UINT;

        // 16 bits per element.
        case TextureFormat::R16F:              return VK_FORMAT_R16_SFLOAT;
        case TextureFormat::R16UI:             return VK_FORMAT_R16_UINT;
        case TextureFormat::R16I:              return VK_FORMAT_R16_SINT;
        case TextureFormat::RG8:               return VK_FORMAT_R8G8_UNORM;
        case TextureFormat::RG8_SNORM:         return VK_FORMAT_R8G8_SNORM;
        case TextureFormat::RG8UI:             return VK_FORMAT_R8G8_UINT;
        case TextureFormat::RG8I:              return VK_FORMAT_R8G8_SINT;
        case TextureFormat::RGB565:            return VK_FORMAT_R5G6B5_UNORM_PACK16;
        case TextureFormat::RGB5_A1:           return VK_FORMAT_R5G5B5A1_UNORM_PACK16;
        case TextureFormat::RGBA4:             return VK_FORMAT_R4G4B4A4_UNORM_PACK16;
        case TextureFormat::DEPTH16:           return VK_FORMAT_D16_UNORM;

        // 24 bits per element. In practice, very few GPU vendors support these. For simplicity
        // we just assume they are not supported, not bothering to query the device capabilities.
        // Note that VK_FORMAT_ enums for 24-bit formats exist, but are meant for vertex attributes.
        case TextureFormat::RGB8:
        case TextureFormat::SRGB8:
        case TextureFormat::RGB8_SNORM:
        case TextureFormat::RGB8UI:
        case TextureFormat::RGB8I:
        case TextureFormat::DEPTH24:
            return VK_FORMAT_UNDEFINED;

        // 32 bits per element.
        case TextureFormat::R32F:              return VK_FORMAT_R32_SFLOAT;
        case TextureFormat::R32UI:             return VK_FORMAT_R32_UINT;
        case TextureFormat::R32I:              return VK_FORMAT_R32_SINT;
        case TextureFormat::RG16F:             return VK_FORMAT_R16G16_SFLOAT;
        case TextureFormat::RG16UI:            return VK_FORMAT_R16G16_UINT;
        case TextureFormat::RG16I:             return VK_FORMAT_R16G16_SINT;
        case TextureFormat::R11F_G11F_B10F:    return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
        case TextureFormat::RGB9_E5:           return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
        case TextureFormat::RGBA8:             return VK_FORMAT_R8G8B8A8_UNORM;
        case TextureFormat::SRGB8_A8:          return VK_FORMAT_R8G8B8A8_SRGB;
        case TextureFormat::RGBA8_SNORM:       return VK_FORMAT_R8G8B8A8_SNORM;
        case TextureFormat::RGBM:              return VK_FORMAT_R8G8B8A8_UNORM;
        case TextureFormat::RGB10_A2:          return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
        case TextureFormat::RGBA8UI:           return VK_FORMAT_R8G8B8A8_UINT;
        case TextureFormat::RGBA8I:            return VK_FORMAT_R8G8B8A8_SINT;
        case TextureFormat::DEPTH32F:          return VK_FORMAT_D32_SFLOAT;
        case TextureFormat::DEPTH24_STENCIL8:  return VK_FORMAT_D24_UNORM_S8_UINT;
        case TextureFormat::DEPTH32F_STENCIL8: return VK_FORMAT_D32_SFLOAT_S8_UINT;

        // 48 bits per element. Note that many GPU vendors do not support these.
        case TextureFormat::RGB16F:            return VK_FORMAT_R16G16B16_SFLOAT;
        case TextureFormat::RGB16UI:           return VK_FORMAT_R16G16B16_UINT;
        case TextureFormat::RGB16I:            return VK_FORMAT_R16G16B16_SINT;

        // 64 bits per element.
        case TextureFormat::RG32F:             return VK_FORMAT_R32G32_SFLOAT;
        case TextureFormat::RG32UI:            return VK_FORMAT_R32G32_UINT;
        case TextureFormat::RG32I:             return VK_FORMAT_R32G32_SINT;
        case TextureFormat::RGBA16F:           return VK_FORMAT_R16G16B16A16_SFLOAT;
        case TextureFormat::RGBA16UI:          return VK_FORMAT_R16G16B16A16_UINT;
        case TextureFormat::RGBA16I:           return VK_FORMAT_R16G16B16A16_SINT;

        // 96-bits per element.
        case TextureFormat::RGB32F:            return VK_FORMAT_R32G32B32_SFLOAT;
        case TextureFormat::RGB32UI:           return VK_FORMAT_R32G32B32_UINT;
        case TextureFormat::RGB32I:            return VK_FORMAT_R32G32B32_SINT;

        // 128-bits per element
        case TextureFormat::RGBA32F:           return VK_FORMAT_R32G32B32A32_SFLOAT;
        case TextureFormat::RGBA32UI:          return VK_FORMAT_R32G32B32A32_UINT;
        case TextureFormat::RGBA32I:           return VK_FORMAT_R32G32B32A32_SINT;

        default:
            return VK_FORMAT_UNDEFINED;
    }
}

uint32_t getBytesPerPixel(TextureFormat format) {
    return details::FTexture::getFormatSize(format);
}

// See also FTexture::computeTextureDataSize, which takes a public-facing Texture format rather
// than a driver-level Texture format, and can account for a specified byte alignment.
uint32_t computeSize(TextureFormat format, uint32_t w, uint32_t h, uint32_t d) {
    const size_t bytesPerTexel = details::FTexture::getFormatSize(format);
    return bytesPerTexel * w * h * d;
}

SwapContext& getSwapContext(VulkanContext& context) {
    VulkanSurfaceContext& surface = *context.currentSurface;
    return surface.swapContexts[surface.currentSwapIndex];
}

bool hasPendingWork(VulkanContext& context) {
    if (context.pendingWork.size() > 0) {
        return true;
    }
    if (context.currentSurface) {
        for (auto& swapContext : context.currentSurface->swapContexts) {
            if (swapContext.pendingWork.size() > 0) {
                return true;
            }
        }
    }
    return false;
}

VkCompareOp getCompareOp(SamplerCompareFunc func) {
    using Compare = driver::SamplerCompareFunc;
    switch (func) {
        case Compare::LE: return VK_COMPARE_OP_LESS_OR_EQUAL;
        case Compare::GE: return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case Compare::L:  return VK_COMPARE_OP_LESS;
        case Compare::G:  return VK_COMPARE_OP_GREATER;
        case Compare::E:  return VK_COMPARE_OP_EQUAL;
        case Compare::NE: return VK_COMPARE_OP_NOT_EQUAL;
        case Compare::A:  return VK_COMPARE_OP_ALWAYS;
        case Compare::N:  return VK_COMPARE_OP_NEVER;
    }
}

VkBlendFactor getBlendFactor(BlendFunction mode) {
    using BlendFunction = filament::driver::BlendFunction;
    switch (mode) {
        case BlendFunction::ZERO:                  return VK_BLEND_FACTOR_ZERO;
        case BlendFunction::ONE:                   return VK_BLEND_FACTOR_ONE;
        case BlendFunction::SRC_COLOR:             return VK_BLEND_FACTOR_SRC_COLOR;
        case BlendFunction::ONE_MINUS_SRC_COLOR:   return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case BlendFunction::DST_COLOR:             return VK_BLEND_FACTOR_DST_COLOR;
        case BlendFunction::ONE_MINUS_DST_COLOR:   return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case BlendFunction::SRC_ALPHA:             return VK_BLEND_FACTOR_SRC_ALPHA;
        case BlendFunction::ONE_MINUS_SRC_ALPHA:   return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case BlendFunction::DST_ALPHA:             return VK_BLEND_FACTOR_DST_ALPHA;
        case BlendFunction::ONE_MINUS_DST_ALPHA:   return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case BlendFunction::SRC_ALPHA_SATURATE:    return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
    }
}

void waitForIdle(VulkanContext& context) {
    // If there's no valid GPU then we have nothing to do.
    if (!context.device) {
        return;
    }

    // If there's no surface, then there's no command buffer.
    if (!context.currentSurface) {
        return;
    }

    // First, wait for submitted command buffer(s) to finish.
    VkFence fences[2];
    uint32_t nfences = 0;
    auto& surfaceContext = *context.currentSurface;
    for (auto& swapContext : surfaceContext.swapContexts) {
        assert(nfences < 2);
        if (swapContext.submitted && swapContext.fence) {
            fences[nfences++] = swapContext.fence;
            swapContext.submitted = false;
        }
    }
    if (nfences > 0) {
        vkWaitForFences(context.device, nfences, fences, VK_FALSE, ~0ull);
    }

    // If we don't have any pending work, we're done.
    if (!hasPendingWork(context)) {
        return;
    }

    // We cannot invoke arbitrary commands inside a render pass.
    assert(context.currentRenderPass.renderPass == VK_NULL_HANDLE);

    // Create a one-off command buffer to avoid the cost of swap chain acquisition and to avoid
    // the possibility of SURFACE_LOST. Note that Vulkan command buffers use the Allocate/Free
    // model instead of Create/Destroy and are therefore okay to create at a high frequency.
    VkCommandBuffer cmdbuffer;
    VkFence fence;
    VkCommandBufferBeginInfo beginInfo { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    VkCommandBufferAllocateInfo allocateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = context.commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };
    VkFenceCreateInfo fenceCreateInfo { .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, };
    vkAllocateCommandBuffers(context.device, &allocateInfo, &cmdbuffer);
    vkCreateFence(context.device, &fenceCreateInfo, VKALLOC, &fence);

    // Keep performing work until there's nothing queued up. This should never iterate more than
    // a couple times because the only work we queue up is for resource transition / reclamation.
    VkPipelineStageFlags waitDestStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    VkSubmitInfo submitInfo {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pWaitDstStageMask = &waitDestStageMask,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmdbuffer,
    };
    int cycles = 0;
    while (hasPendingWork(context)) {
        if (cycles++ > 2) {
            utils::slog.e << "Unexpected daisychaining of pending work." << utils::io::endl;
            break;
        }
        for (auto& swapContext : context.currentSurface->swapContexts) {
            vkBeginCommandBuffer(cmdbuffer, &beginInfo);
            performPendingWork(context, swapContext, cmdbuffer);
            vkEndCommandBuffer(cmdbuffer);
            vkQueueSubmit(context.graphicsQueue, 1, &submitInfo, fence);
            vkWaitForFences(context.device, 1, &fence, VK_FALSE, UINT64_MAX);
            vkResetFences(context.device, 1, &fence);
            vkResetCommandBuffer(cmdbuffer, 0);
        }
    }
    vkFreeCommandBuffers(context.device, context.commandPool, 1, &cmdbuffer);
    vkDestroyFence(context.device, fence, VKALLOC);
}

void acquireCommandBuffer(VulkanContext& context) {
    // Ask Vulkan for the next image in the swap chain and update the currentSwapIndex.
    VulkanSurfaceContext& surface = *context.currentSurface;
    VkResult result = vkAcquireNextImageKHR(context.device, surface.swapchain,
            UINT64_MAX, surface.imageAvailable, VK_NULL_HANDLE, &surface.currentSwapIndex);
    ASSERT_POSTCONDITION(result != VK_ERROR_OUT_OF_DATE_KHR,
            "Stale / resized swap chain not yet supported.");
    ASSERT_POSTCONDITION(result == VK_SUBOPTIMAL_KHR || result == VK_SUCCESS,
            "vkAcquireNextImageKHR error.");
    SwapContext& swap = getSwapContext(context);

    // Ensure that the previous submission of this command buffer has finished.
    result = vkWaitForFences(context.device, 1, &swap.fence, VK_FALSE, UINT64_MAX);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkWaitForFences error.");

    // Restart the command buffer.
    result = vkResetFences(context.device, 1, &swap.fence);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkResetFences error.");
    VkCommandBuffer cmdbuffer = swap.cmdbuffer;
    VkResult error = vkResetCommandBuffer(cmdbuffer, 0);
    ASSERT_POSTCONDITION(not error, "vkResetCommandBuffer error.");
    VkCommandBufferBeginInfo beginInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
    };
    error = vkBeginCommandBuffer(cmdbuffer, &beginInfo);
    ASSERT_POSTCONDITION(not error, "vkBeginCommandBuffer error.");
    context.cmdbuffer = cmdbuffer;
    swap.submitted = false;
}

void releaseCommandBuffer(VulkanContext& context) {
    // Finalize the command buffer and set the cmdbuffer pointer to null.
    VkResult result = vkEndCommandBuffer(context.cmdbuffer);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkEndCommandBuffer error.");
    context.cmdbuffer = nullptr;

    // Submit the command buffer.
    VkPipelineStageFlags waitDestStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    VulkanSurfaceContext& surfaceContext = *context.currentSurface;
    SwapContext& swapContext = getSwapContext(context);
    VkSubmitInfo submitInfo {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1u,
        .pWaitSemaphores = &surfaceContext.imageAvailable,
        .pWaitDstStageMask = &waitDestStageMask,
        .commandBufferCount = 1,
        .pCommandBuffers = &swapContext.cmdbuffer,
        .signalSemaphoreCount = 1u,
        .pSignalSemaphores = &surfaceContext.renderingFinished,
    };
    result = vkQueueSubmit(context.graphicsQueue, 1, &submitInfo, swapContext.fence);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkQueueSubmit error.");
    swapContext.submitted = true;
}

void performPendingWork(VulkanContext& context, SwapContext& swapContext, VkCommandBuffer cmdbuf) {
    // First, execute pending tasks that are specific to this swap context. Copy the tasks into a
    // local queue first, which allows newly added tasks to be deferred until the next frame.
    decltype(swapContext.pendingWork) tasks;
    tasks.swap(swapContext.pendingWork);
    for (auto& callback : tasks) {
        callback(cmdbuf);
    }
    // Next, execute the global pending work. Again, we copy the work queue into a local queue
    // to allow tasks to re-add themselves.
    tasks.clear();
    tasks.swap(context.pendingWork);
    for (auto& callback : tasks) {
        callback(cmdbuf);
    }
}

// Flushes the command buffer and waits for it to finish executing. Useful for diagnosing
// sychronization issues.
void flushCommandBuffer(VulkanContext& context) {
    VulkanSurfaceContext& surface = *context.currentSurface;
    const SwapContext& sc = surface.swapContexts[surface.currentSwapIndex];

    // Submit the command buffer.
    VkResult error = vkEndCommandBuffer(context.cmdbuffer);
    ASSERT_POSTCONDITION(!error, "vkEndCommandBuffer error.");
    VkPipelineStageFlags waitDestStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    VkSubmitInfo submitInfo {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pWaitDstStageMask = &waitDestStageMask,
        .commandBufferCount = 1,
        .pCommandBuffers = &context.cmdbuffer,
    };
    error = vkQueueSubmit(context.graphicsQueue, 1, &submitInfo, sc.fence);
    ASSERT_POSTCONDITION(!error, "vkQueueSubmit error.");

    // Restart the command buffer.
    error = vkWaitForFences(context.device, 1, &sc.fence, VK_FALSE, UINT64_MAX);
    ASSERT_POSTCONDITION(!error, "vkWaitForFences error.");
    error = vkResetFences(context.device, 1, &sc.fence);
    ASSERT_POSTCONDITION(!error, "vkResetFences error.");
    error = vkResetCommandBuffer(context.cmdbuffer, 0);
    ASSERT_POSTCONDITION(!error, "vkResetCommandBuffer error.");
    VkCommandBufferBeginInfo beginInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
    };
    error = vkBeginCommandBuffer(context.cmdbuffer, &beginInfo);
    ASSERT_POSTCONDITION(!error, "vkBeginCommandBuffer error.");
}

VkFormat findSupportedFormat(VulkanContext& context, const std::vector<VkFormat>& candidates,
        VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(context.physicalDevice, format, &props);
        if (tiling == VK_IMAGE_TILING_LINEAR &&
                (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
                (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }
    return VK_FORMAT_UNDEFINED;
}

} // namespace filament
} // namespace driver
