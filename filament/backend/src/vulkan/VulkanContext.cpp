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
#include "VulkanTexture.h"
#include "VulkanUtility.h"

#include <backend/PixelBufferDescriptor.h>

#include <utils/Panic.h>
#include <utils/FixedCapacityVector.h>

#include <algorithm> // for std::max

using namespace bluevk;

using utils::FixedCapacityVector;


namespace {

#if VK_ENABLE_VALIDATION
VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(VkDebugReportFlagsEXT flags,
        VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location,
        int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData) {
    if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
        utils::slog.e << "VULKAN ERROR: (" << pLayerPrefix << ") " << pMessage << utils::io::endl;
    } else {
        utils::slog.w << "VULKAN WARNING: (" << pLayerPrefix << ") "
                << pMessage << utils::io::endl;
    }
    utils::slog.e << utils::io::endl;
    return VK_FALSE;
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT types, const VkDebugUtilsMessengerCallbackDataEXT* cbdata,
        void* pUserData) {
    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        utils::slog.e << "VULKAN ERROR: (" << cbdata->pMessageIdName << ") "
                << cbdata->pMessage << utils::io::endl;
    } else {
        // TODO: emit best practices warnings about aggressive pipeline barriers.
        if (strstr(cbdata->pMessage, "ALL_GRAPHICS_BIT") || strstr(cbdata->pMessage, "ALL_COMMANDS_BIT")) {
           return VK_FALSE;
        }
        utils::slog.w << "VULKAN WARNING: (" << cbdata->pMessageIdName << ") "
                << cbdata->pMessage << utils::io::endl;
    }
    utils::slog.e << utils::io::endl;
    return VK_FALSE;
}

// These strings need to be allocated outside a function stack
const std::string_view DESIRED_LAYERS[] = {
  "VK_LAYER_KHRONOS_validation",
#if FILAMENT_VULKAN_DUMP_API
  "VK_LAYER_LUNARG_api_dump",
#endif
#if defined(ENABLE_RENDERDOC)
  "VK_LAYER_RENDERDOC_Capture",
#endif
};

FixedCapacityVector<const char*> getEnabledLayers() {
    constexpr size_t kMaxEnabledLayersCount = sizeof(DESIRED_LAYERS) / sizeof(DESIRED_LAYERS[0]);

    const FixedCapacityVector<VkLayerProperties> availableLayers = filament::backend::enumerate(
            vkEnumerateInstanceLayerProperties);

    auto enabledLayers = FixedCapacityVector<const char*>::with_capacity(kMaxEnabledLayersCount);
    for (const auto& desired : DESIRED_LAYERS) {
        for (const VkLayerProperties& layer : availableLayers) {
            const std::string_view availableLayer(layer.layerName);
            if (availableLayer == desired) {
                enabledLayers.push_back(desired.data());
                break;
            }
        }
    }
    return enabledLayers;
}

#endif // VK_ENABLE_VALIDATION

void printDeviceInfo(VkInstance instance, VkPhysicalDevice device) {
    // Print some driver or MoltenVK information if it is available.
    if (vkGetPhysicalDeviceProperties2KHR) {
        VkPhysicalDeviceDriverProperties driverProperties = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES,
        };
        VkPhysicalDeviceProperties2 physicalDeviceProperties2 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
            .pNext = &driverProperties,
        };
        vkGetPhysicalDeviceProperties2KHR(device, &physicalDeviceProperties2);
        utils::slog.i << "Vulkan device driver: "
                      << driverProperties.driverName << " "
                      << driverProperties.driverInfo << utils::io::endl;
    }

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    // Print out some properties of the GPU for diagnostic purposes.
    //
    // Ideally, the vendors register their vendor ID's with Khronos so that apps can make an
    // id => string mapping. However, in practice this hasn't happened. At the time of this
    // writing the gpuinfo database has the following ID's:
    //
    //     0x1002 - AMD
    //     0x1010 - ImgTec
    //     0x10DE - NVIDIA
    //     0x13B5 - ARM
    //     0x106B - APPLE
    //     0x5143 - Qualcomm
    //     0x8086 - INTEL
    //
    // Since we don't have any vendor-specific workarounds yet, there's no need to make this
    // mapping in code. The "deviceName" string informally reveals the marketing name for the
    // GPU. (e.g., Quadro)
    const uint32_t driverVersion = deviceProperties.driverVersion;
    const uint32_t vendorID = deviceProperties.vendorID;
    const uint32_t deviceID = deviceProperties.deviceID;
    const int major = VK_VERSION_MAJOR(deviceProperties.apiVersion);
    const int minor = VK_VERSION_MINOR(deviceProperties.apiVersion);

    const FixedCapacityVector<VkPhysicalDevice> physicalDevices = filament::backend::enumerate(
            vkEnumeratePhysicalDevices, instance);

    utils::slog.i << "Selected physical device '"
                  << deviceProperties.deviceName
                  << "' from " << physicalDevices.size() << " physical devices. "
                  << "(vendor " << utils::io::hex << vendorID << ", "
                  << "device " << deviceID << ", "
                  << "driver " << driverVersion << ", "
                  << utils::io::dec << "api " << major << "." << minor << ")"
                  << utils::io::endl;
}

void printDepthFormats(VkPhysicalDevice device) {
    // For diagnostic purposes, print useful information about available depth formats.
    // Note that Vulkan is more constrained than OpenGL ES 3.1 in this area.
    if constexpr (VK_ENABLE_VALIDATION && FILAMENT_VULKAN_VERBOSE) {
        const VkFormatFeatureFlags required = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT |
                VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
        utils::slog.i << "Sampleable depth formats: ";
        for (VkFormat format = (VkFormat) 1;;) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(device, format, &props);
            if ((props.optimalTilingFeatures & required) == required) {
                utils::slog.i << format << " ";
            }
            if (format == VK_FORMAT_ASTC_12x12_SRGB_BLOCK) {
                utils::slog.i << utils::io::endl;
                break;
            }
            format = (VkFormat) (1 + (int) format);
        }
    }
}

struct InstanceExtensions {
    bool debugUtilsSupported = false;
    bool portabilityEnumerationSupported = false;
};

struct DeviceExtensions {
    bool debugMarkersSupported = false;
    bool portabilitySubsetSupported = false;
    bool maintenanceSupported[3] = {};
};

InstanceExtensions getInstanceExtensions() {
    InstanceExtensions exts;
    // Determine if the VK_EXT_debug_utils instance extension is available.
    const FixedCapacityVector<VkExtensionProperties> availableExts = filament::backend::enumerate(
            vkEnumerateInstanceExtensionProperties,
            static_cast<const char*>(nullptr) /* pLayerName */);
    for  (const auto& extProps : availableExts) {
        if (!strcmp(extProps.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
            exts.debugUtilsSupported = true;
        } else if (!strcmp(extProps.extensionName, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
            exts.portabilityEnumerationSupported = true;
        }
    }
    return exts;
}

DeviceExtensions getDeviceExtensions(VkPhysicalDevice device) {
    DeviceExtensions exts;

    // Identify supported physical device extensions
    const FixedCapacityVector<VkExtensionProperties> extensions = filament::backend::enumerate(
            vkEnumerateDeviceExtensionProperties, device,
            static_cast<const char*>(nullptr) /* pLayerName */);
    for (const auto& extension : extensions) {
        if (!strcmp(extension.extensionName, VK_EXT_DEBUG_MARKER_EXTENSION_NAME)) {
            exts.debugMarkersSupported = true;
        } else if (!strcmp(extension.extensionName, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)) {
            exts.portabilitySubsetSupported = true;
        } else if (!strcmp(extension.extensionName, VK_KHR_MAINTENANCE1_EXTENSION_NAME)) {
            exts.maintenanceSupported[0] = true;
        } else if (!strcmp(extension.extensionName, VK_KHR_MAINTENANCE2_EXTENSION_NAME)) {
            exts.maintenanceSupported[1] = true;
        } else if (!strcmp(extension.extensionName, VK_KHR_MAINTENANCE3_EXTENSION_NAME)) {
            exts.maintenanceSupported[2] = true;
        }
    }
    return exts;
}

VkInstance createInstance(const char* const* ppRequiredExtensions, uint32_t requiredExtensionCount,
        const InstanceExtensions& instExtensions) {
    VkInstance instance;
    VkInstanceCreateInfo instanceCreateInfo = {};
    bool validationFeaturesSupported = false;

#if VK_ENABLE_VALIDATION
    const auto enabledLayers = getEnabledLayers();
    if (!enabledLayers.empty()) {
        // If layers are supported, Check if VK_EXT_validation_features is supported.
        const FixedCapacityVector<VkExtensionProperties> availableExts =
	        filament::backend::enumerate(vkEnumerateInstanceExtensionProperties,
		        "VK_LAYER_KHRONOS_validation");
        for (const auto& extProps : availableExts) {
            if (!strcmp(extProps.extensionName, VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME)) {
                validationFeaturesSupported = true;
                break;
            }
        }
        instanceCreateInfo.enabledLayerCount = (uint32_t) enabledLayers.size();
        instanceCreateInfo.ppEnabledLayerNames = enabledLayers.data();
    } else {
#if defined(__ANDROID__)
      utils::slog.d << "Validation layers are not available; did you set jniLibs in your "
              << "gradle file?" << utils::io::endl;
#else
      utils::slog.d << "Validation layer not available; did you install the Vulkan SDK?\n"
              << "Please ensure that VK_LAYER_PATH is set correctly." << utils::io::endl;
#endif
    }
#endif // VK_ENABLE_VALIDATION

    // The Platform class can require 1 or 2 instance extensions, plus we'll request at most 5
    // instance extensions here in the common code. So that's a max of 7.
    static constexpr uint32_t MAX_INSTANCE_EXTENSION_COUNT = 7;
    const char* ppEnabledExtensions[MAX_INSTANCE_EXTENSION_COUNT];
    uint32_t enabledExtensionCount = 0;

    // Request all cross-platform extensions.
    ppEnabledExtensions[enabledExtensionCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
    ppEnabledExtensions[enabledExtensionCount++] = VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME;
#if VK_ENABLE_VALIDATION && defined(__ANDROID__)
    ppEnabledExtensions[enabledExtensionCount++] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
#endif
    if (validationFeaturesSupported) {
        ppEnabledExtensions[enabledExtensionCount++] = VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME;
    }
    if (instExtensions.debugUtilsSupported) {
        ppEnabledExtensions[enabledExtensionCount++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    }
    if (instExtensions.portabilityEnumerationSupported) {
        ppEnabledExtensions[enabledExtensionCount++] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
    }

    // Request platform-specific extensions.
    for (uint32_t i = 0; i < requiredExtensionCount; ++i) {
        assert_invariant(enabledExtensionCount < MAX_INSTANCE_EXTENSION_COUNT);
        ppEnabledExtensions[enabledExtensionCount++] = ppRequiredExtensions[i];
    }

    // Create the Vulkan instance.
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = VK_MAKE_API_VERSION(0,
            VK_REQUIRED_VERSION_MAJOR, VK_REQUIRED_VERSION_MINOR, 0);
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledExtensionCount = enabledExtensionCount;
    instanceCreateInfo.ppEnabledExtensionNames = ppEnabledExtensions;
    if (instExtensions.portabilityEnumerationSupported) {
        instanceCreateInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    }

    VkValidationFeaturesEXT features = {};
    VkValidationFeatureEnableEXT enables[] = {
        VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
        // TODO: Enable synchronization validation.
        // VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT,
    };
    if (validationFeaturesSupported) {
        features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
        features.enabledValidationFeatureCount = sizeof(enables) / sizeof(enables[0]);
        features.pEnabledValidationFeatures = enables;
        instanceCreateInfo.pNext = &features;
    }

    VkResult result = vkCreateInstance(&instanceCreateInfo, VKALLOC, &instance);
#ifndef NDEBUG
    if (result != VK_SUCCESS) {
        utils::slog.e << "Unable to create instance: " << result << utils::io::endl;
    }
#endif
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "Unable to create Vulkan instance.");
    return instance;
}

VkDevice createLogicalDevice(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceFeatures& features,
        uint32_t graphicsQueueFamilyIndex, const DeviceExtensions& deviceExtensions) {
    VkDevice device;
    VkDeviceQueueCreateInfo deviceQueueCreateInfo[1] = {};
    const float queuePriority[] = {1.0f};
    VkDeviceCreateInfo deviceCreateInfo = {};
    FixedCapacityVector<const char*> deviceExtensionNames;
    deviceExtensionNames.reserve(6);
    deviceExtensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    if (deviceExtensions.debugMarkersSupported) {
        deviceExtensionNames.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
    }
    if (deviceExtensions.portabilitySubsetSupported) {
        deviceExtensionNames.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    }
    if (deviceExtensions.maintenanceSupported[0]) {
        deviceExtensionNames.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
    }
    if (deviceExtensions.maintenanceSupported[1]) {
        deviceExtensionNames.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
    }
    if (deviceExtensions.maintenanceSupported[2]) {
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
    VkPhysicalDeviceFeatures enabledFeatures {
        .samplerAnisotropy = features.samplerAnisotropy,
        .textureCompressionETC2 = features.textureCompressionETC2,
        .textureCompressionBC = features.textureCompressionBC,
    };

    deviceCreateInfo.pEnabledFeatures = &enabledFeatures;
    deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensionNames.size();
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensionNames.data();

    VkPhysicalDevicePortabilitySubsetFeaturesKHR portability = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_FEATURES_KHR,
        .pNext = nullptr,
        .imageViewFormatSwizzle = VK_TRUE,
        .mutableComparisonSamplers = VK_TRUE,
    };
    if (deviceExtensions.portabilitySubsetSupported) {
        deviceCreateInfo.pNext = &portability;
    }

    VkResult result = vkCreateDevice(physicalDevice, &deviceCreateInfo, VKALLOC, &device);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkCreateDevice error.");

    return device;
}

// This method is used to enable/disable extensions based on external factors (i.e.
// driver/device workarounds).
void pruneExtensions(VkPhysicalDevice device, InstanceExtensions* instExtensions,
        DeviceExtensions* deviceExtensions) {
    if (vkGetPhysicalDeviceProperties2KHR) {
        char* driverInfo = nullptr;
        VkPhysicalDeviceDriverProperties driverProperties = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES,
        };
        VkPhysicalDeviceProperties2 physicalDeviceProperties2 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
            .pNext = &driverProperties,
        };
        vkGetPhysicalDeviceProperties2KHR(device, &physicalDeviceProperties2);
        driverInfo = driverProperties.driverInfo;

        if (instExtensions->debugUtilsSupported) {
            // Workaround for Mesa drivers. See issue #6192
            if (driverInfo && strstr(driverInfo, "Mesa")) {
                instExtensions->debugUtilsSupported = false;
            }
        }
    }

    // debugUtils and debugMarkers extensions are used mutually exclusively.
    if (instExtensions->debugUtilsSupported && deviceExtensions->debugMarkersSupported) {
        deviceExtensions->debugMarkersSupported = false;
    }
}

FixedCapacityVector<VkQueueFamilyProperties> getPhysicalDeviceQueueFamilyPropertiesHelper(
        VkPhysicalDevice device) {
    uint32_t queueFamiliesCount;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamiliesCount, nullptr);
    FixedCapacityVector<VkQueueFamilyProperties> queueFamiliesProperties(queueFamiliesCount);
    if (queueFamiliesCount > 0) {
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamiliesCount,
                queueFamiliesProperties.data());
    }
    return queueFamiliesProperties;
}

VkPhysicalDevice selectPhysicalDevice(VkInstance instance) {
    const FixedCapacityVector<VkPhysicalDevice> physicalDevices = filament::backend::enumerate(
            vkEnumeratePhysicalDevices, instance);
    for (const auto& candidateDevice : physicalDevices) {
        VkPhysicalDeviceProperties targetDeviceProperties;
        vkGetPhysicalDeviceProperties(candidateDevice, &targetDeviceProperties);

        const int major = VK_VERSION_MAJOR(targetDeviceProperties.apiVersion);
        const int minor = VK_VERSION_MINOR(targetDeviceProperties.apiVersion);

        // Does the device support the required Vulkan level?
        if (major < VK_REQUIRED_VERSION_MAJOR) {
            continue;
        }
        if (major == VK_REQUIRED_VERSION_MAJOR && minor < VK_REQUIRED_VERSION_MINOR) {
            continue;
        }

        // Does the device have any command queues that support graphics?
        // In theory, we should also ensure that the device supports presentation of our
        // particular VkSurface, but we don't have a VkSurface yet, so we'll skip this requirement.
        const FixedCapacityVector<VkQueueFamilyProperties> queueFamiliesProperties =
               getPhysicalDeviceQueueFamilyPropertiesHelper(candidateDevice);
        bool foundGraphicsQueue = false;
        for (const auto& props : queueFamiliesProperties) {
            if (props.queueCount != 0 && props.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                foundGraphicsQueue = true;
                break;
            }
        }
        if (!foundGraphicsQueue) {
            continue;
        }

        // Does the device support the VK_KHR_swapchain extension?
        const FixedCapacityVector<VkExtensionProperties> extensions = filament::backend::enumerate(
                vkEnumerateDeviceExtensionProperties, candidateDevice,
                static_cast<const char*>(nullptr) /* pLayerName */);
        bool supportsSwapchain = false;
        for (const auto& extension : extensions) {
            if (!strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME)) {
                supportsSwapchain = true;
                break;
            }
        }
        if (!supportsSwapchain) {
            continue;
        }

        return candidateDevice;
    }

    PANIC_POSTCONDITION("Unable to find suitable device.");
    return VK_NULL_HANDLE;
}

VkFormat findSupportedFormat(VkPhysicalDevice device, utils::Slice<VkFormat> candidates,
        VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(device, format, &props);
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

} // end anonymous namespace

namespace filament::backend {

VkImage VulkanAttachment::getImage() const {
    return texture ? texture->getVkImage() : VK_NULL_HANDLE;
}

VkFormat VulkanAttachment::getFormat() const {
    return texture ? texture->getVkFormat() : VK_FORMAT_UNDEFINED;
}

VkImageLayout VulkanAttachment::getLayout() const {
    return texture ? texture->getVkLayout(layer, level) : VK_IMAGE_LAYOUT_UNDEFINED;
}

VkExtent2D VulkanAttachment::getExtent2D() const {
    assert_invariant(texture);
    return { std::max(1u, texture->width >> level), std::max(1u, texture->height >> level) };
}

VkImageView VulkanAttachment::getImageView(VkImageAspectFlags aspect) const {
    assert_invariant(texture);
    return texture->getAttachmentView(level, layer, aspect);
}

void VulkanContext::initialize(const char* const* ppRequiredExtensions,
        uint32_t requiredExtensionCount) {
    InstanceExtensions instExtensions = getInstanceExtensions();
    instance = createInstance(ppRequiredExtensions, requiredExtensionCount, instExtensions);
    bluevk::bindInstance(instance);

    physicalDevice = selectPhysicalDevice(instance);
    // Initialize the following fields: physicalDeviceProperties, memoryProperties,
    // physicalDeviceFeatures, graphicsQueueFamilyIndex.
    afterSelectPhysicalDevice();
    printDeviceInfo(instance, physicalDevice);

    DeviceExtensions deviceExtensions = getDeviceExtensions(physicalDevice);
    pruneExtensions(physicalDevice, &instExtensions, &deviceExtensions);

    device = createLogicalDevice(physicalDevice, physicalDeviceFeatures, graphicsQueueFamilyIndex,
            deviceExtensions);
    // Initialize graphicsQueue, commandPool, timestamps, allocator,
    // and command buffer manager (commands).
    afterCreateLogicalDevice();

    // Store the extension support in the context
    debugUtilsSupported = instExtensions.debugUtilsSupported;
    portabilityEnumerationSupported = instExtensions.portabilityEnumerationSupported;
    debugMarkersSupported = deviceExtensions.debugMarkersSupported;
    portabilitySubsetSupported = deviceExtensions.portabilitySubsetSupported;
    for (int j = 0; j < 3; ++j) {
        maintenanceSupported[j] = deviceExtensions.maintenanceSupported[j];
    }

    // Initialize finalDepthFormat, debugCallback, and debugMessenger
    afterCreateInstance();

    printDepthFormats(physicalDevice);
}

void VulkanContext::afterSelectPhysicalDevice() {
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

    const FixedCapacityVector<VkQueueFamilyProperties> queueFamiliesProperties =
            getPhysicalDeviceQueueFamilyPropertiesHelper(physicalDevice);
    graphicsQueueFamilyIndex = 0xffff;
    for (uint32_t j = 0; j < queueFamiliesProperties.size(); ++j) {
        VkQueueFamilyProperties props = queueFamiliesProperties[j];
        if (props.queueCount != 0 && props.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsQueueFamilyIndex = j;
        }
    }

    vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
}

void VulkanContext::afterCreateLogicalDevice() {
    vkGetDeviceQueue(device, graphicsQueueFamilyIndex, 0,
            &graphicsQueue);

    VkCommandPoolCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.flags =
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    createInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
    VkResult result = vkCreateCommandPool(device, &createInfo, VKALLOC, &commandPool);
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
#if VMA_DYNAMIC_VULKAN_FUNCTIONS
        .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
#else
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
#endif
    };
    const VmaAllocatorCreateInfo allocatorInfo {
        .physicalDevice = physicalDevice,
        .device = device,
        .pVulkanFunctions = &funcs,
        .instance = instance
    };
    vmaCreateAllocator(&allocatorInfo, &allocator);
    commands = new VulkanCommands(device, graphicsQueueFamilyIndex);
}

void VulkanContext::afterCreateInstance() {
#if VK_ENABLE_VALIDATION
    UTILS_UNUSED const PFN_vkCreateDebugReportCallbackEXT createDebugReportCallback =
            vkCreateDebugReportCallbackEXT;
    VkResult result;
    if (debugUtilsSupported) {
        VkDebugUtilsMessengerCreateInfoEXT createInfo = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .pNext = nullptr,
            .flags = 0,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
            .pfnUserCallback = debugUtilsCallback
        };
        result = vkCreateDebugUtilsMessengerEXT(instance, &createInfo, VKALLOC, &debugMessenger);
        ASSERT_POSTCONDITION(result == VK_SUCCESS, "Unable to create Vulkan debug messenger.");
    } else if (createDebugReportCallback) {
        const VkDebugReportCallbackCreateInfoEXT cbinfo = {
            VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
            nullptr,
            VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT,
            debugReportCallback,
            nullptr
        };
        result = createDebugReportCallback(instance, &cbinfo, VKALLOC, &debugCallback);
        ASSERT_POSTCONDITION(result == VK_SUCCESS, "Unable to create Vulkan debug callback.");
    }
#endif

    // Choose a depth format that meets our requirements. Take care not to include stencil formats
    // just yet, since that would require a corollary change to the "aspect" flags for the VkImage.
    const VkFormat formats[] = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_X8_D24_UNORM_PACK32 };
    finalDepthFormat = findSupportedFormat(
        physicalDevice, utils::Slice<VkFormat>(formats, formats + 2), VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
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

void VulkanContext::createEmptyTexture(VulkanStagePool& stagePool) {
    emptyTexture = new VulkanTexture(*this, SamplerType::SAMPLER_2D, 1,
            TextureFormat::RGBA8, 1, 1, 1, 1,
            TextureUsage::DEFAULT | TextureUsage::COLOR_ATTACHMENT |
            TextureUsage::SUBPASS_INPUT, stagePool);
    uint32_t black = 0;
    PixelBufferDescriptor pbd(&black, 4, PixelDataFormat::RGBA, PixelDataType::UBYTE);
    emptyTexture->updateImage(pbd, 1, 1, 1, 0, 0, 0, 0);
}

} // namespace filament::backend
