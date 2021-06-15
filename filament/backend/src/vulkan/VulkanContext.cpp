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

#include "VulkanContext.h"
#include "VulkanHandles.h"
#include "VulkanMemory.h"
#include "VulkanUtility.h"

#include <utils/Panic.h>
#include <utils/FixedCapacityVector.h>

using namespace bluevk;

using utils::FixedCapacityVector;

namespace filament {
namespace backend {

void VulkanContext::selectPhysicalDevice() {
    uint32_t physicalDeviceCount = 0;
    VkResult result = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
    ASSERT_POSTCONDITION(result == VK_SUCCESS && physicalDeviceCount > 0,
            "vkEnumeratePhysicalDevices count error.");
    FixedCapacityVector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    result = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount,
            physicalDevices.data());
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkEnumeratePhysicalDevices error.");
    for (uint32_t i = 0; i < physicalDeviceCount; ++i) {
        physicalDevice = physicalDevices[i];
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

        const int major = VK_VERSION_MAJOR(physicalDeviceProperties.apiVersion);
        const int minor = VK_VERSION_MINOR(physicalDeviceProperties.apiVersion);

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
        FixedCapacityVector<VkQueueFamilyProperties> queueFamiliesProperties(queueFamiliesCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamiliesCount,
                queueFamiliesProperties.data());
        graphicsQueueFamilyIndex = 0xffff;
        for (uint32_t j = 0; j < queueFamiliesCount; ++j) {
            VkQueueFamilyProperties props = queueFamiliesProperties[j];
            if (props.queueCount == 0) {
                continue;
            }
            if (props.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                graphicsQueueFamilyIndex = j;
            }
        }
        if (graphicsQueueFamilyIndex == 0xffff) continue;

        // Does the device support the VK_KHR_swapchain extension?
        uint32_t extensionCount;
        result = vkEnumerateDeviceExtensionProperties(physicalDevice, /*pLayerName = */ nullptr,
                &extensionCount, nullptr);
        ASSERT_POSTCONDITION(result == VK_SUCCESS,
                "vkEnumerateDeviceExtensionProperties count error.");
        FixedCapacityVector<VkExtensionProperties> extensions(extensionCount);
        result = vkEnumerateDeviceExtensionProperties(physicalDevice, /*pLayerName = */ nullptr,
                &extensionCount, extensions.data());
        ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkEnumerateDeviceExtensionProperties error.");
        bool supportsSwapchain = false;
        for (uint32_t k = 0; k < extensionCount; ++k) {
            if (!strcmp(extensions[k].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME)) {
                supportsSwapchain = true;
            }
            if (!strcmp(extensions[k].extensionName, VK_EXT_DEBUG_MARKER_EXTENSION_NAME)) {
                debugMarkersSupported = true;
            }
            if (!strcmp(extensions[k].extensionName, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)) {
                portabilitySubsetSupported = true;
            }
            if (!strcmp(extensions[k].extensionName, VK_KHR_MAINTENANCE1_EXTENSION_NAME)) {
                maintenanceSupported[0] = true;
            }
            if (!strcmp(extensions[k].extensionName, VK_KHR_MAINTENANCE2_EXTENSION_NAME)) {
                maintenanceSupported[1] = true;
            }
            if (!strcmp(extensions[k].extensionName, VK_KHR_MAINTENANCE3_EXTENSION_NAME)) {
                maintenanceSupported[2] = true;
            }
        }
        if (!supportsSwapchain) continue;

        // Bingo, we finally found a physical device that supports everything we need.
        vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

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
        const uint32_t driverVersion = physicalDeviceProperties.driverVersion;
        const uint32_t vendorID = physicalDeviceProperties.vendorID;
        const uint32_t deviceID = physicalDeviceProperties.deviceID;
        utils::slog.i << "Selected physical device '"
                << physicalDeviceProperties.deviceName
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

void VulkanContext::createLogicalDevice() {
    VkDeviceQueueCreateInfo deviceQueueCreateInfo[1] = {};
    const float queuePriority[] = {1.0f};
    VkDeviceCreateInfo deviceCreateInfo = {};
    FixedCapacityVector<const char*> deviceExtensionNames;
    deviceExtensionNames.reserve(6);
    deviceExtensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    if (debugMarkersSupported && !debugUtilsSupported) {
        deviceExtensionNames.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
    }
    if (portabilitySubsetSupported) {
        deviceExtensionNames.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    }
    if (maintenanceSupported[0]) {
        deviceExtensionNames.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
    }
    if (maintenanceSupported[1]) {
        deviceExtensionNames.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
    }
    if (maintenanceSupported[2]) {
        deviceExtensionNames.push_back(VK_KHR_MAINTENANCE3_EXTENSION_NAME);
    }
    deviceQueueCreateInfo->sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfo->queueFamilyIndex = graphicsQueueFamilyIndex;
    deviceQueueCreateInfo->queueCount = 1;
    deviceQueueCreateInfo->pQueuePriorities = &queuePriority[0];
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfo;

    // We could simply enable all supported features, but since that may have performance
    // consequences let's just enable the features we need.
    const auto& supportedFeatures = physicalDeviceFeatures;
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
    if (portabilitySubsetSupported) {
        deviceCreateInfo.pNext = &portability;
    }

    VkResult result = vkCreateDevice(physicalDevice, &deviceCreateInfo, VKALLOC,
            &device);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkCreateDevice error.");
    vkGetDeviceQueue(device, graphicsQueueFamilyIndex, 0,
            &graphicsQueue);

    VkCommandPoolCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.flags =
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    createInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
    result = vkCreateCommandPool(device, &createInfo, VKALLOC, &commandPool);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkCreateCommandPool error.");

    // Create a timestamp pool large enough to hold a pair of queries for each timer.
    VkQueryPoolCreateInfo tqpCreateInfo = {};
    tqpCreateInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    tqpCreateInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;

    std::unique_lock<utils::Mutex> timestamps_lock(timestamps.mutex);
    tqpCreateInfo.queryCount = timestamps.used.size() * 2;
    result = vkCreateQueryPool(device, &tqpCreateInfo, VKALLOC, &timestamps.pool);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkCreateQueryPool error.");
    timestamps.used.reset();
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
        .physicalDevice = physicalDevice,
        .device = device,
        .pVulkanFunctions = &funcs,
        .pRecordSettings = nullptr,
        .instance = instance
    };
    vmaCreateAllocator(&allocatorInfo, &allocator);

    const uint32_t memTypeBits = UINT32_MAX;

    // Create a GPU memory pool for VkBuffer objects that ignores the bufferImageGranularity field
    // in VkPhysicalDeviceLimits. We have observed that honoring bufferImageGranularity can cause
    // the allocator to slow to a crawl.
    uint32_t memTypeIndex = UINT32_MAX;
    const VmaAllocationCreateInfo gpuInfo = { .usage = VMA_MEMORY_USAGE_GPU_ONLY };
    UTILS_UNUSED_IN_RELEASE VkResult res = vmaFindMemoryTypeIndex(allocator, memTypeBits, &gpuInfo,
            &memTypeIndex);
    assert_invariant(res == VK_SUCCESS && memTypeIndex != UINT32_MAX);
    const VmaPoolCreateInfo gpuPoolInfo {
        .memoryTypeIndex = memTypeIndex,
        .flags = VMA_POOL_CREATE_IGNORE_BUFFER_IMAGE_GRANULARITY_BIT,
        .blockSize = VMA_BUFFER_POOL_BLOCK_SIZE_IN_MB * 1024 * 1024,
    };
    res = vmaCreatePool(allocator, &gpuPoolInfo, &vmaPoolGPU);
    assert_invariant(res == VK_SUCCESS);

    // Next, create a similar pool but for CPU mappable memory (typically used as a staging area).
    memTypeIndex = UINT32_MAX;
    const VmaAllocationCreateInfo cpuInfo = { .usage = VMA_MEMORY_USAGE_CPU_ONLY };
    res = vmaFindMemoryTypeIndex(allocator, memTypeBits, &cpuInfo, &memTypeIndex);
    assert_invariant(res == VK_SUCCESS && memTypeIndex != UINT32_MAX);
    const VmaPoolCreateInfo cpuPoolInfo {
        .memoryTypeIndex = memTypeIndex,
        .flags = VMA_POOL_CREATE_IGNORE_BUFFER_IMAGE_GRANULARITY_BIT,
        .blockSize = VMA_BUFFER_POOL_BLOCK_SIZE_IN_MB * 1024 * 1024,
    };
    res = vmaCreatePool(allocator, &cpuPoolInfo, &vmaPoolCPU);
    assert_invariant(res == VK_SUCCESS);

    commands = new VulkanCommands(device, graphicsQueueFamilyIndex);
}

uint32_t VulkanContext::selectMemoryType(uint32_t flags, VkFlags reqs) {
    for (uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; i++) {
        if (flags & 1) {
            if ((memoryProperties.memoryTypes[i].propertyFlags & reqs) == reqs) {
                return i;
            }
        }
        flags >>= 1;
    }
    ASSERT_POSTCONDITION(false, "Unable to find a memory type that meets requirements.");
    return (uint32_t) ~0ul;
}

VkFormat VulkanContext::findSupportedFormat(utils::Slice<VkFormat> candidates,
        VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
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

VkImageLayout VulkanContext::getTextureLayout(TextureUsage usage) const {
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

void VulkanContext::createEmptyTexture(VulkanStagePool& stagePool) {
    emptyTexture = new VulkanTexture(*this, SamplerType::SAMPLER_2D, 1,
            TextureFormat::RGBA8, 1, 1, 1, 1,
            TextureUsage::DEFAULT | TextureUsage::COLOR_ATTACHMENT |
            TextureUsage::SUBPASS_INPUT, stagePool);
    uint32_t black = 0;
    PixelBufferDescriptor pbd(&black, 4, PixelDataFormat::RGBA, PixelDataType::UBYTE);
    emptyTexture->update2DImage(pbd, 1, 1, 0);
}

} // namespace filament
} // namespace backend
