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

#include "VulkanContext.h"
#include "VulkanUtility.h"

#include <utils/Panic.h>

namespace filament {
namespace backend {

VulkanCmdFence::VulkanCmdFence(VkDevice device, bool signaled) : device(device) {
    VkFenceCreateInfo fenceCreateInfo { .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    if (signaled) {
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    }
    vkCreateFence(device, &fenceCreateInfo, VKALLOC, &fence);
}

 VulkanCmdFence::~VulkanCmdFence() {
    vkDestroyFence(device, fence, VKALLOC);
}

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
                assert(props.timestampValidBits > 0);
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
    const float queuePriority[] = {1.0f};
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

    // We could simply enable all supported features, but since that may have performance
    // consequences let's just enable the features we need.
    const auto& supportedFeatures = context.physicalDeviceFeatures;
    VkPhysicalDeviceFeatures enabledFeatures {
        .textureCompressionETC2 = supportedFeatures.textureCompressionETC2,
        .textureCompressionBC = supportedFeatures.textureCompressionBC,
    };

    deviceCreateInfo.pEnabledFeatures = &enabledFeatures;
    deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensionNames.size();
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

    // Create a timestamp pool large enough to hold a pair of queries for each timer.
    VkQueryPoolCreateInfo tqpCreateInfo = {};
    tqpCreateInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    tqpCreateInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
    tqpCreateInfo.queryCount = context.timestamps.used.size() * 2;
    result = vkCreateQueryPool(context.device, &tqpCreateInfo, VKALLOC, &context.timestamps.pool);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkCreateQueryPool error.");
    context.timestamps.used.reset();

    const VmaVulkanFunctions funcs {
        .vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties,
        .vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties,
        .vkAllocateMemory = vkAllocateMemory,
        .vkFreeMemory = vkFreeMemory,
        .vkMapMemory = vkMapMemory,
        .vkUnmapMemory = vkUnmapMemory,
        .vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges,
        .vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges,
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

    // Create the work command buffer and fence for work unrelated to the swap chain.
    const VkCommandBufferAllocateInfo allocateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = context.commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };
    const VkCommandBufferBeginInfo binfo { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    vkAllocateCommandBuffers(context.device, &allocateInfo, &context.work.cmdbuffer);
    vkBeginCommandBuffer(context.work.cmdbuffer, &binfo);
}

void getPresentationQueue(VulkanContext& context, VulkanSurfaceContext& sc) {
    uint32_t queueFamiliesCount;
    vkGetPhysicalDeviceQueueFamilyProperties(context.physicalDevice, &queueFamiliesCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamiliesProperties(queueFamiliesCount);
    vkGetPhysicalDeviceQueueFamilyProperties(context.physicalDevice, &queueFamiliesCount,
            queueFamiliesProperties.data());
    uint32_t presentQueueFamilyIndex = 0xffff;

    // We prefer the graphics and presentation queues to be the same, so first check if that works.
    // On most platforms they must be the same anyway, and this avoids issues with MoltenVK.
    VkBool32 supported = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(context.physicalDevice, context.graphicsQueueFamilyIndex,
            sc.surface, &supported);
    if (supported) {
        presentQueueFamilyIndex = context.graphicsQueueFamilyIndex;
    }

    // Otherwise fall back to separate graphics and presentation queues.
    if (presentQueueFamilyIndex == 0xffff) {
        for (uint32_t j = 0; j < queueFamiliesCount; ++j) {
            vkGetPhysicalDeviceSurfaceSupportKHR(context.physicalDevice, j, sc.surface, &supported);
            if (supported) {
                presentQueueFamilyIndex = j;
                break;
            }
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

void createSwapChain(VulkanContext& context, VulkanSurfaceContext& surfaceContext) {
    getSurfaceCaps(context, surfaceContext);

    // The general advice is to require one more than the minimum swap chain length, since the
    // absolute minimum could easily require waiting for a driver or presentation layer to release
    // the previous frame's buffer. The only situation in which we'd ask for the minimum length is
    // when using a MAILBOX presentation strategy for low-latency situations where tearing is
    // acceptable.
    const uint32_t maxImageCount = surfaceContext.surfaceCapabilities.maxImageCount;
    const uint32_t minImageCount = surfaceContext.surfaceCapabilities.minImageCount;
    uint32_t desiredImageCount = minImageCount + 1;

    // According to section 30.5 of VK 1.1, maxImageCount of zero means "that there is no limit on
    // the number of images, though there may be limits related to the total amount of memory used
    // by presentable images."
    if (maxImageCount != 0 && desiredImageCount > maxImageCount) {
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
            .format = surfaceContext.surfaceFormat.format,
            .image = images[i],
            .view = {},
            .memory = {},
            .offscreen = {}
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

    // Allocate command buffers.
    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = context.commandPool;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = (uint32_t) surfaceContext.swapContexts.size();
    std::vector<VkCommandBuffer> cmdbufs(allocateInfo.commandBufferCount);
    result = vkAllocateCommandBuffers(context.device, &allocateInfo, cmdbufs.data());
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkAllocateCommandBuffers error.");
    for (uint32_t i = 0; i < allocateInfo.commandBufferCount; ++i) {
        surfaceContext.swapContexts[i].commands.cmdbuffer = cmdbufs[i];
    }

    createDepthBuffer(context, surfaceContext, context.depthFormat);
}

void destroySwapChain(VulkanContext& context, VulkanSurfaceContext& surfaceContext,
        VulkanDisposer& disposer) {
    waitForIdle(context);
    for (SwapContext& swapContext : surfaceContext.swapContexts) {
        disposer.release(swapContext.commands.resources);
        vkFreeCommandBuffers(context.device, context.commandPool, 1,
                &swapContext.commands.cmdbuffer);
        swapContext.commands.fence.reset();
        vkDestroyImageView(context.device, swapContext.attachment.view, VKALLOC);
        swapContext.commands.fence = VK_NULL_HANDLE;
        swapContext.attachment.view = VK_NULL_HANDLE;
    }
    vkDestroySwapchainKHR(context.device, surfaceContext.swapchain, VKALLOC);
    vkDestroySemaphore(context.device, surfaceContext.imageAvailable, VKALLOC);
    vkDestroySemaphore(context.device, surfaceContext.renderingFinished, VKALLOC);

    vkDestroyImageView(context.device, surfaceContext.depth.view, VKALLOC);
    vkDestroyImage(context.device, surfaceContext.depth.image, VKALLOC);
    vkFreeMemory(context.device, surfaceContext.depth.memory, VKALLOC);
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

SwapContext& getSwapContext(VulkanContext& context) {
    VulkanSurfaceContext& surface = *context.currentSurface;
    return surface.swapContexts[surface.currentSwapIndex];
}

void waitForIdle(VulkanContext& context) {
    // If there's no valid GPU then we have nothing to do.
    if (!context.device) {
        return;
    }

    // Flush the work command buffer and wait for it to finish.
    flushWorkCommandBuffer(context);
    acquireWorkCommandBuffer(context);

    // Wait for submitted command buffer(s) to finish.
    if (context.currentSurface) {
        VkFence fences[4];
        uint32_t nfences = 0;
        auto& surfaceContext = *context.currentSurface;
        for (auto& swapContext : surfaceContext.swapContexts) {
            assert(nfences < 4);
            if (swapContext.commands.fence && swapContext.commands.fence->submitted) {
                fences[nfences++] = swapContext.commands.fence->fence;
                swapContext.commands.fence->submitted = false;
            }
        }
        if (nfences > 0) {
            vkWaitForFences(context.device, nfences, fences, VK_FALSE, ~0ull);
        }

        // Next flush the active command buffer and wait for it to finish.
        if (context.currentCommands) {
            flushCommandBuffer(context);
        }
    }
}

bool acquireSwapCommandBuffer(VulkanContext& context) {
    // Ask Vulkan for the next image in the swap chain and update the currentSwapIndex.
    VulkanSurfaceContext& surface = *context.currentSurface;
    VkResult result = vkAcquireNextImageKHR(context.device, surface.swapchain,
            UINT64_MAX, surface.imageAvailable, VK_NULL_HANDLE, &surface.currentSwapIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        return false;
    }
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "Unable to acquire image from swap chain.");
    SwapContext& swap = getSwapContext(context);

    // Ensure that the previous submission of this command buffer has finished.
    auto& cmdfence = swap.commands.fence;
    if (cmdfence) {
        result = vkWaitForFences(context.device, 1, &cmdfence->fence, VK_FALSE, UINT64_MAX);
        ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkWaitForFences error.");
    }

     cmdfence.reset(new VulkanCmdFence(context.device));

    // Restart the command buffer.
    VkCommandBuffer cmdbuffer = swap.commands.cmdbuffer;
    VkResult error = vkResetCommandBuffer(cmdbuffer, 0);
    ASSERT_POSTCONDITION(!error, "vkResetCommandBuffer error.");
    VkCommandBufferBeginInfo beginInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
    };
    error = vkBeginCommandBuffer(cmdbuffer, &beginInfo);
    ASSERT_POSTCONDITION(!error, "vkBeginCommandBuffer error.");
    context.currentCommands = &swap.commands;
    return true;
}

// Flushes the current command buffer and waits for it to finish executing.
void flushCommandBuffer(VulkanContext& context) {
    VulkanSurfaceContext& surface = *context.currentSurface;
    const SwapContext& sc = surface.swapContexts[surface.currentSwapIndex];

    // Submit the command buffer.
    VkResult error = vkEndCommandBuffer(context.currentCommands->cmdbuffer);
    ASSERT_POSTCONDITION(!error, "vkEndCommandBuffer error.");
    VkPipelineStageFlags waitDestStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    VkSubmitInfo submitInfo {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pWaitDstStageMask = &waitDestStageMask,
        .commandBufferCount = 1,
        .pCommandBuffers = &context.currentCommands->cmdbuffer,
    };

    auto& cmdfence = sc.commands.fence;
    std::unique_lock<utils::Mutex> lock(cmdfence->mutex);
    error = vkQueueSubmit(context.graphicsQueue, 1, &submitInfo, cmdfence->fence);
    lock.unlock();
    ASSERT_POSTCONDITION(!error, "vkQueueSubmit error.");
    cmdfence->condition.notify_all();

    // Restart the command buffer.
    error = vkWaitForFences(context.device, 1, &cmdfence->fence, VK_FALSE, UINT64_MAX);
    ASSERT_POSTCONDITION(!error, "vkWaitForFences error.");
    error = vkResetFences(context.device, 1, &cmdfence->fence);
    ASSERT_POSTCONDITION(!error, "vkResetFences error.");
    error = vkResetCommandBuffer(context.currentCommands->cmdbuffer, 0);
    ASSERT_POSTCONDITION(!error, "vkResetCommandBuffer error.");
    VkCommandBufferBeginInfo beginInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
    };
    error = vkBeginCommandBuffer(context.currentCommands->cmdbuffer, &beginInfo);
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

VkCommandBuffer acquireWorkCommandBuffer(VulkanContext& context) {
    VulkanCommandBuffer& work = context.work;
    const VkCommandBufferBeginInfo binfo { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    if (work.fence && work.fence->submitted) {
        work.fence->submitted = false;
        vkWaitForFences(context.device, 1, &work.fence->fence, VK_FALSE, UINT64_MAX);
        vkResetCommandBuffer(work.cmdbuffer, 0);
        vkBeginCommandBuffer(work.cmdbuffer, &binfo);
    }
    work.fence.reset(new VulkanCmdFence(context.device));
    return work.cmdbuffer;
}

void flushWorkCommandBuffer(VulkanContext& context) {
    VulkanCommandBuffer& work = context.work;
    ASSERT_PRECONDITION(!work.fence->submitted, "Flushed the work buffer more than once.");
    const VkPipelineStageFlags waitDestStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    VkSubmitInfo submitInfo {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pWaitDstStageMask = &waitDestStageMask,
        .commandBufferCount = 1,
        .pCommandBuffers = &work.cmdbuffer,
    };
    vkEndCommandBuffer(work.cmdbuffer);
    vkQueueSubmit(context.graphicsQueue, 1, &submitInfo, work.fence->fence);
    work.fence->submitted = true;
}

void createDepthBuffer(VulkanContext& context, VulkanSurfaceContext& surfaceContext,
        VkFormat depthFormat) {
    // Create an appropriately-sized device-only VkImage.
    const auto size = surfaceContext.surfaceCapabilities.currentExtent;
    VkImage depthImage;
    VkImageCreateInfo imageInfo {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = depthFormat,
        .extent = { size.width, size.height, 1 },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
    };
    VkResult error = vkCreateImage(context.device, &imageInfo, VKALLOC, &depthImage);
    assert(!error && "Unable to create depth image.");

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
    assert(!error && "Unable to allocate depth memory.");
    error = vkBindImageMemory(context.device, depthImage, surfaceContext.depth.memory, 0);
    assert(!error && "Unable to bind depth memory.");

    // Create a VkImageView so that we can attach depth to the framebuffer.
    VkImageView depthView;
    VkImageViewCreateInfo viewInfo {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = depthImage,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = depthFormat,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .levelCount = 1,
            .layerCount = 1,
        },
    };
    error = vkCreateImageView(context.device, &viewInfo, VKALLOC, &depthView);
    assert(!error && "Unable to create depth view.");

    // Unlike the color attachments (which are double-buffered or triple-buffered), we only need one
    // depth attachment in the entire chain.
    surfaceContext.depth.view = depthView;
    surfaceContext.depth.image = depthImage;
    surfaceContext.depth.format = depthFormat;

    // Begin a new command buffer solely for the purpose of transitioning the image layout.
    VkCommandBuffer cmdbuffer = acquireWorkCommandBuffer(context);

    // Transition the depth image into an optimal layout.
    VkImageMemoryBarrier barrier {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = depthImage,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .levelCount = 1,
            .layerCount = 1,
        },
    };
    vkCmdPipelineBarrier(cmdbuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

VkImageLayout getTextureLayout(TextureUsage usage) {
    // Filament sometimes samples from depth while it is bound to the current render target, (e.g.
    // SSAO does this while depth writes are disabled) so let's keep it simple and use GENERAL for
    // all depth textures.
    if (any(usage & TextureUsage::DEPTH_ATTACHMENT)) {
        return VK_IMAGE_LAYOUT_GENERAL;
    }

    // Filament sometimes samples from one miplevel while writing to another level in the same
    // texture (e.g. bloom does this). Moreover we'd like to avoid lots of expensive layout
    // transitions. So, keep it simple and use GENERAL for all color-attachable textures.
    if (any(usage & TextureUsage::COLOR_ATTACHMENT)) {
        return VK_IMAGE_LAYOUT_GENERAL;
    }

    // Finally, the layout for an immutable texture is optimal read-only.
    return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

} // namespace filament
} // namespace backend
