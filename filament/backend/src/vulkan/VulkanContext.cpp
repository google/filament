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
#pragma clang diagnostic ignored "-Wundef"
#pragma clang diagnostic ignored "-Wunused-variable"
#include "vk_mem_alloc.h"
#pragma clang diagnostic pop

#include "VulkanContext.h"
#include "VulkanHandles.h"
#include "VulkanUtility.h"

#include <utils/Panic.h>

#ifndef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
#define VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME "VK_KHR_portability_subset"
#endif

using namespace bluevk;

namespace filament {
namespace backend {

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

        const int major = VK_VERSION_MAJOR(context.physicalDeviceProperties.apiVersion);
        const int minor = VK_VERSION_MINOR(context.physicalDeviceProperties.apiVersion);

        // Does the device support the required Vulkan level?
        if (major < VK_REQUIRED_VERSION_MAJOR) {
            continue;
        }
        if (major == VK_REQUIRED_VERSION_MAJOR && minor < VK_REQUIRED_VERSION_MINOR) {
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
            if (!strcmp(extensions[k].extensionName, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)) {
                context.portabilitySubsetSupported = true;
            }
            if (!strcmp(extensions[k].extensionName, VK_KHR_MAINTENANCE1_EXTENSION_NAME)) {
                context.maintenanceSupported[0] = true;
            }
            if (!strcmp(extensions[k].extensionName, VK_KHR_MAINTENANCE2_EXTENSION_NAME)) {
                context.maintenanceSupported[1] = true;
            }
            if (!strcmp(extensions[k].extensionName, VK_KHR_MAINTENANCE3_EXTENSION_NAME)) {
                context.maintenanceSupported[2] = true;
            }
        }
        if (!supportsSwapchain) continue;

        // Bingo, we finally found a physical device that supports everything we need.
        context.physicalDevice = physicalDevice;
        vkGetPhysicalDeviceFeatures(physicalDevice, &context.physicalDeviceFeatures);
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &context.memoryProperties);

        // Print some driver or MoltenVK information if it is available.
        if (vkGetPhysicalDeviceProperties2KHR) {
            VkPhysicalDeviceDriverProperties driverProperties = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES,
            };
            VkPhysicalDeviceProperties2 physicalDeviceProperties2 = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
                .pNext = &driverProperties,
            };
            vkGetPhysicalDeviceProperties2KHR(physicalDevice, &physicalDeviceProperties2);
            utils::slog.i << "Vulkan device driver: "
                << driverProperties.driverName << " "
                << driverProperties.driverInfo << utils::io::endl;
        }

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
        const uint32_t driverVersion = context.physicalDeviceProperties.driverVersion;
        const uint32_t vendorID = context.physicalDeviceProperties.vendorID;
        const uint32_t deviceID = context.physicalDeviceProperties.deviceID;
        utils::slog.i << "Selected physical device '"
                << context.physicalDeviceProperties.deviceName
                << "' from " << physicalDeviceCount << " physical devices. "
                << "(vendor " << utils::io::hex << vendorID << ", "
                << "device " << deviceID << ", "
                << "driver " << driverVersion << ", "
                << utils::io::dec << "api " << major << "." << minor << ")"
                << utils::io::endl;
        return;
    }
    PANIC_POSTCONDITION("Unable to find suitable device.");
}

void createLogicalDevice(VulkanContext& context) {
    VkDeviceQueueCreateInfo deviceQueueCreateInfo[1] = {};
    const float queuePriority[] = {1.0f};
    VkDeviceCreateInfo deviceCreateInfo = {};
    std::vector<const char*> deviceExtensionNames = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
    if (context.debugMarkersSupported && !context.debugUtilsSupported) {
        deviceExtensionNames.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
    }
    if (context.portabilitySubsetSupported) {
        deviceExtensionNames.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    }
    if (context.maintenanceSupported[0]) {
        deviceExtensionNames.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
    }
    if (context.maintenanceSupported[1]) {
        deviceExtensionNames.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
    }
    if (context.maintenanceSupported[2]) {
        deviceExtensionNames.push_back(VK_KHR_MAINTENANCE3_EXTENSION_NAME);
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
        .samplerAnisotropy = supportedFeatures.samplerAnisotropy,
        .textureCompressionETC2 = supportedFeatures.textureCompressionETC2,
        .textureCompressionBC = supportedFeatures.textureCompressionBC,
    };

    deviceCreateInfo.pEnabledFeatures = &enabledFeatures;
    deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensionNames.size();
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensionNames.data();

    VkPhysicalDevicePortabilitySubsetFeaturesKHR portability = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_FEATURES_KHR,
        .pNext = nullptr,
        .mutableComparisonSamplers = VK_TRUE,
    };
    if (context.portabilitySubsetSupported) {
        deviceCreateInfo.pNext = &portability;
    }

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

    std::unique_lock<utils::Mutex> timestamps_lock(context.timestamps.mutex);
    tqpCreateInfo.queryCount = context.timestamps.used.size() * 2;
    result = vkCreateQueryPool(context.device, &tqpCreateInfo, VKALLOC, &context.timestamps.pool);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkCreateQueryPool error.");
    context.timestamps.used.reset();
    timestamps_lock.unlock();

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
        .vkCmdCopyBuffer = vkCmdCopyBuffer,
        .vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR,
        .vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2KHR
    };
    const VmaAllocatorCreateInfo allocatorInfo {
        .physicalDevice = context.physicalDevice,
        .device = context.device,
        .pVulkanFunctions = &funcs,
        .pRecordSettings = nullptr,
        .instance = context.instance
    };
    vmaCreateAllocator(&allocatorInfo, &context.allocator);
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

VulkanAttachment& getSwapChainAttachment(VulkanContext& context) {
    VulkanSwapChain& surface = *context.currentSurface;
    return surface.attachments[surface.currentSwapIndex];
}

void waitForIdle(VulkanContext& context) {
    // If there's no valid GPU then we have nothing to do.
    if (!context.device) {
        return;
    }
    context.commands->flush();
    context.commands->wait();
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

void createEmptyTexture(VulkanContext& context, VulkanStagePool& stagePool) {
    context.emptyTexture = new VulkanTexture(context, SamplerType::SAMPLER_2D, 1,
            TextureFormat::RGBA8, 1, 1, 1, 1,
            TextureUsage::DEFAULT | TextureUsage::COLOR_ATTACHMENT |
            TextureUsage::SUBPASS_INPUT, stagePool);
    uint32_t black = 0;
    PixelBufferDescriptor pbd(&black, 4, PixelDataFormat::RGBA, PixelDataType::UBYTE);
    context.emptyTexture->update2DImage(pbd, 1, 1, 0);
}

} // namespace filament
} // namespace backend
