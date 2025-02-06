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

#include "backend/platforms/VulkanPlatform.h"

#include <backend/DriverEnums.h>

#include "vulkan/platform/VulkanPlatformSwapChainImpl.h"
#include "vulkan/VulkanConstants.h"
#include "vulkan/VulkanDriver.h"
#include "vulkan/VulkanUtility.h"

#include <bluevk/BlueVK.h>
#include <utils/PrivateImplementation-impl.h>
#include <utils/Panic.h>

#define SWAPCHAIN_RET_FUNC(func, handle, ...)                                                      \
    if (mImpl->mSurfaceSwapChains.find(handle) != mImpl->mSurfaceSwapChains.end()) {               \
        return static_cast<VulkanPlatformSurfaceSwapChain*>(handle)->func(__VA_ARGS__);            \
    } else if (mImpl->mHeadlessSwapChains.find(handle) != mImpl->mHeadlessSwapChains.end()) {      \
        return static_cast<VulkanPlatformHeadlessSwapChain*>(handle)->func(__VA_ARGS__);           \
    } else {                                                                                       \
        PANIC_PRECONDITION("Bad handle for swapchain");                                            \
        return {};                                                                                 \
    }

using namespace utils;
using namespace bluevk;

namespace filament::backend {

namespace {

constexpr uint32_t const INVALID_VK_INDEX = 0xFFFFFFFF;

using ExtensionSet = VulkanPlatform::ExtensionSet;

inline bool setContains(ExtensionSet const& set, utils::CString const& extension) {
    return set.find(extension) != set.end();
};

#if FVK_ENABLED(FVK_DEBUG_VALIDATION)
// These strings need to be allocated outside a function stack
const std::string_view DESIRED_LAYERS[] = {
        "VK_LAYER_KHRONOS_validation",
#if FVK_ENABLED(FVK_DEBUG_DUMP_API)
        "VK_LAYER_LUNARG_api_dump",
#endif
#if defined(ENABLE_RENDERDOC)
        "VK_LAYER_RENDERDOC_Capture",
#endif
};

FixedCapacityVector<const char*> getEnabledLayers() {
    constexpr size_t kMaxEnabledLayersCount = sizeof(DESIRED_LAYERS) / sizeof(DESIRED_LAYERS[0]);

    const FixedCapacityVector<VkLayerProperties> availableLayers
            = filament::backend::enumerate(vkEnumerateInstanceLayerProperties);

    auto enabledLayers = FixedCapacityVector<const char*>::with_capacity(kMaxEnabledLayersCount);
    for (auto const& desired: DESIRED_LAYERS) {
        for (const VkLayerProperties& layer: availableLayers) {
            const std::string_view availableLayer(layer.layerName);
            if (availableLayer == desired) {
                enabledLayers.push_back(desired.data());
                break;
            }
        }
    }
    return enabledLayers;
}
#endif // FVK_EANBLED(FVK_DEBUG_VALIDATION)

template<typename StructA, typename StructB>
StructA* chainStruct(StructA* structA, StructB* structB) {
    structB->pNext = const_cast<void*>(structA->pNext);
    structA->pNext = (void*) structB;
    return structA;
}

void printDeviceInfo(VkInstance instance, VkPhysicalDevice device) {
    // Print some driver or MoltenVK information if it is available.
    if (vkGetPhysicalDeviceProperties2) {
        VkPhysicalDeviceDriverProperties driverProperties = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES,
        };
        VkPhysicalDeviceProperties2 physicalDeviceProperties2 = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
        };
        chainStruct(&physicalDeviceProperties2, &driverProperties);
        vkGetPhysicalDeviceProperties2(device, &physicalDeviceProperties2);
        FVK_LOGI << "Vulkan device driver: " << driverProperties.driverName << " "
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

    const FixedCapacityVector<VkPhysicalDevice> physicalDevices
            = filament::backend::enumerate(vkEnumeratePhysicalDevices, instance);

    FVK_LOGI << "Selected physical device '" << deviceProperties.deviceName << "' from "
                  << physicalDevices.size() << " physical devices. "
                  << "(vendor " << utils::io::hex << vendorID << ", "
                  << "device " << deviceID << ", "
                  << "driver " << driverVersion << ", " << utils::io::dec << "api " << major << "."
                  << minor << ")" << utils::io::endl;
}

#if FVK_ENABLED(FVK_DEBUG_VALIDATION)
void printDepthFormats(VkPhysicalDevice device) {
    // For diagnostic purposes, print useful information about available depth formats.
    // Note that Vulkan is more constrained than OpenGL ES 3.1 in this area.
    const VkFormatFeatureFlags required = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
                                              | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
    FVK_LOGI << "Sampleable depth formats: ";
    for (VkFormat format : ALL_VK_FORMATS) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(device, format, &props);
        if ((props.optimalTilingFeatures & required) == required) {
            FVK_LOGI << format << " ";
        }
    }
    FVK_LOGI << utils::io::endl;
}
#endif

ExtensionSet getInstanceExtensions(ExtensionSet const& externallyRequiredExts = {}) {
    ExtensionSet const TARGET_EXTS = {
        // Request all cross-platform extensions.
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,

    // Request these if available.
#if FVK_ENABLED(FVK_DEBUG_DEBUG_UTILS)
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
        VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,

#if FVK_ENABLED(FVK_DEBUG_VALIDATION)
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
#endif
    };
    ExtensionSet exts;
    FixedCapacityVector<VkExtensionProperties> const availableExts =
            filament::backend::enumerate(vkEnumerateInstanceExtensionProperties,
                    static_cast<char const*>(nullptr) /* pLayerName */);
    for (auto const& extProps: availableExts) {
        utils::CString name { extProps.extensionName };

        // To workaround an Adreno bug where the extension name could be of 0 length.
        if (name.size() == 0) {
            continue;
        }

        if (setContains(TARGET_EXTS, name) || setContains(externallyRequiredExts, name)) {
            exts.insert(name);
        }
    }
    return exts;
}

ExtensionSet getDeviceExtensions(VkPhysicalDevice device) {
    ExtensionSet const TARGET_EXTS = {
#if FVK_ENABLED(FVK_DEBUG_DEBUG_UTILS)
            VK_EXT_DEBUG_MARKER_EXTENSION_NAME,
#endif
    // We only support external image for Android for now, but nothing bars us from
    // supporting other platforms.
#if defined(__ANDROID__)
            VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME,
            VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,
            VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME,
            VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME,
 #endif

            VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME,
            VK_KHR_MAINTENANCE1_EXTENSION_NAME,
            VK_KHR_MAINTENANCE2_EXTENSION_NAME,
            VK_KHR_MAINTENANCE3_EXTENSION_NAME,
            VK_KHR_MULTIVIEW_EXTENSION_NAME,
    };
    ExtensionSet exts;
    // Identify supported physical device extensions
    FixedCapacityVector<VkExtensionProperties> const extensions
            = filament::backend::enumerate(vkEnumerateDeviceExtensionProperties, device,
                    static_cast<const char*>(nullptr) /* pLayerName */);
    for (auto const& extension: extensions) {
        utils::CString name { extension.extensionName };

        // To workaround an Adreno bug where the extension name could be of 0 length.
        if (name.size() == 0) {
            continue;
        }

        if (setContains(TARGET_EXTS, name)) {
            exts.insert(name);
        }
    }
    return exts;
}

VkInstance createInstance(ExtensionSet const& requiredExts) {
    VkInstance instance;
    VkInstanceCreateInfo instanceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    };
    bool validationFeaturesSupported = false;

#if FVK_ENABLED(FVK_DEBUG_VALIDATION)
    auto const enabledLayers = getEnabledLayers();
    if (!enabledLayers.empty()) {
        // If layers are supported, Check if VK_EXT_validation_features is supported.
        FixedCapacityVector<VkExtensionProperties> const availableValidationExts
                = filament::backend::enumerate(vkEnumerateInstanceExtensionProperties,
                        "VK_LAYER_KHRONOS_validation");
        for (auto const& extProps: availableValidationExts) {
            if (!strcmp(extProps.extensionName, VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME)) {
                validationFeaturesSupported = true;
                break;
            }
        }
        instanceCreateInfo.enabledLayerCount = (uint32_t) enabledLayers.size();
        instanceCreateInfo.ppEnabledLayerNames = enabledLayers.data();
    } else {
#if defined(__ANDROID__)
        FVK_LOGD << "Validation layers are not available; did you set jniLibs in your "
                 << "gradle file?" << utils::io::endl;
#else
        FVK_LOGD << "Validation layer not available; did you install the Vulkan SDK?\n"
                 << "Please ensure that VK_LAYER_PATH is set correctly." << utils::io::endl;
#endif // __ANDROID__

    }
#endif // FVK_ENABLED(FVK_DEBUG_VALIDATION)

    // The Platform class can require 1 or 2 instance extensions, plus we'll request at most 5
    // instance extensions here in the common code. So that's a max of 7.
    static constexpr uint32_t MAX_INSTANCE_EXTENSION_COUNT = 8;
    const char* ppEnabledExtensions[MAX_INSTANCE_EXTENSION_COUNT];
    uint32_t enabledExtensionCount = 0;

    if (validationFeaturesSupported) {
        ppEnabledExtensions[enabledExtensionCount++] = VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME;
    }
    // Request platform-specific extensions.
    for (auto const& requiredExt: requiredExts) {
        assert_invariant(enabledExtensionCount < MAX_INSTANCE_EXTENSION_COUNT);
        ppEnabledExtensions[enabledExtensionCount++] = requiredExt.data();
    }

    // Create the Vulkan instance.
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pEngineName = "Filament";
    appInfo.apiVersion
            = VK_MAKE_API_VERSION(0, FVK_REQUIRED_VERSION_MAJOR, FVK_REQUIRED_VERSION_MINOR, 0);
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledExtensionCount = enabledExtensionCount;
    instanceCreateInfo.ppEnabledExtensionNames = ppEnabledExtensions;
    if (setContains(requiredExts, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
        instanceCreateInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    }

    VkValidationFeaturesEXT features = {
        .sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
    };
    VkValidationFeatureEnableEXT enables[] = {
        VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
        VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT,
    };
    if (validationFeaturesSupported) {
        features.enabledValidationFeatureCount = sizeof(enables) / sizeof(enables[0]);
        features.pEnabledValidationFeatures = enables;
        chainStruct(&instanceCreateInfo, &features);
    }

    VkResult result = vkCreateInstance(&instanceCreateInfo, VKALLOC, &instance);
    FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS)
            << "Unable to create Vulkan instance. error=" << static_cast<int32_t>(result);
    return instance;
}

VkDevice createLogicalDevice(VkPhysicalDevice physicalDevice,
        VkPhysicalDeviceFeatures2 const& features, uint32_t graphicsQueueFamilyIndex,
        uint32_t protectedGraphicsQueueFamilyIndex, ExtensionSet const& deviceExtensions,
        bool requestImageView2DOn3DImage) {
    VkDevice device;
    float queuePriority[] = {1.0f};
    VkDeviceCreateInfo deviceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    };
    FixedCapacityVector<const char*> requestExtensions;
    requestExtensions.reserve(deviceExtensions.size() + 1);

    // TODO: We don't really need this if we only ever expect headless swapchains.
    requestExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    for (auto const& ext: deviceExtensions) {
        requestExtensions.push_back(ext.data());
    }
    VkDeviceQueueCreateInfo deviceQueueCreateInfo[2] = {};
    deviceQueueCreateInfo[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfo[0].queueFamilyIndex = graphicsQueueFamilyIndex;
    deviceQueueCreateInfo[0].queueCount = 1;
    deviceQueueCreateInfo[0].pQueuePriorities = &queuePriority[0];
    // Protected queue
    deviceQueueCreateInfo[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfo[1].flags = VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT;
    deviceQueueCreateInfo[1].queueFamilyIndex = protectedGraphicsQueueFamilyIndex;
    deviceQueueCreateInfo[1].queueCount = 1;
    deviceQueueCreateInfo[1].pQueuePriorities = &queuePriority[0];

    bool const hasProtectedQueue = protectedGraphicsQueueFamilyIndex != INVALID_VK_INDEX;
    deviceCreateInfo.queueCreateInfoCount = hasProtectedQueue ? 2 : 1;
    deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfo;

    // We could simply enable all supported features, but since that may have performance
    // consequences let's just enable the features we need.
    VkPhysicalDeviceFeatures enabledFeatures{
            .depthClamp = features.features.depthClamp,
            .samplerAnisotropy = features.features.samplerAnisotropy,
            .textureCompressionETC2 = features.features.textureCompressionETC2,
            .textureCompressionBC = features.features.textureCompressionBC,
            .shaderClipDistance = features.features.shaderClipDistance,
    };

    deviceCreateInfo.pEnabledFeatures = &enabledFeatures;
    deviceCreateInfo.enabledExtensionCount = (uint32_t) requestExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = requestExtensions.data();

    VkPhysicalDevicePortabilitySubsetFeaturesKHR portability = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_FEATURES_KHR,
        .imageViewFormatSwizzle = VK_TRUE,
        .mutableComparisonSamplers = VK_TRUE,
        .imageView2DOn3DImage = requestImageView2DOn3DImage ? VK_TRUE : VK_FALSE,
    };
    if (setContains(deviceExtensions, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)) {
        chainStruct(&deviceCreateInfo, &portability);
    }

    VkPhysicalDeviceMultiviewFeaturesKHR multiview = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES_KHR,
        .pNext = nullptr,
        .multiview = VK_TRUE,
        .multiviewGeometryShader = VK_FALSE,
        .multiviewTessellationShader = VK_FALSE,
    };
    if (setContains(deviceExtensions, VK_KHR_MULTIVIEW_EXTENSION_NAME)) {
        chainStruct(&deviceCreateInfo, &multiview);
    }

    VkPhysicalDeviceProtectedMemoryFeatures protectedMemory = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES,
        .protectedMemory = VK_TRUE,
    };
    if (hasProtectedQueue) {
        // Enable protected memory, if requested.
        chainStruct(&deviceCreateInfo, &protectedMemory);
    }

    VkResult result = vkCreateDevice(physicalDevice, &deviceCreateInfo, VKALLOC, &device);
    FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS)
            << "vkCreateDevice error=" << static_cast<int32_t>(result);

    return device;
}

// This method is used to enable/disable extensions based on external factors (i.e.
// driver/device workarounds).
std::tuple<ExtensionSet, ExtensionSet> pruneExtensions(VkPhysicalDevice device,
        ExtensionSet const& instExts, ExtensionSet const& deviceExts) {
    ExtensionSet newInstExts = instExts;
    ExtensionSet newDeviceExts = deviceExts;

#if FVK_ENABLED(FVK_DEBUG_DEBUG_UTILS)
    // debugUtils and debugMarkers extensions are used mutually exclusively.
    if (setContains(newInstExts, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) &&
            setContains(newInstExts, VK_EXT_DEBUG_MARKER_EXTENSION_NAME)) {
        newDeviceExts.erase(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
    }
#endif

#if FVK_ENABLED(FVK_DEBUG_VALIDATION)
    // debugMarker must also request debugReport the instance extension. So check if that's present.
    if (setContains(newInstExts, VK_EXT_DEBUG_MARKER_EXTENSION_NAME) &&
            !setContains(newInstExts, VK_EXT_DEBUG_MARKER_EXTENSION_NAME)) {
        newDeviceExts.erase(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
    }
#endif

    return std::tuple(newInstExts, newDeviceExts);
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

uint32_t identifyGraphicsQueueFamilyIndex(VkPhysicalDevice physicalDevice, VkQueueFlags flags) {
    const FixedCapacityVector<VkQueueFamilyProperties> queueFamiliesProperties
            = getPhysicalDeviceQueueFamilyPropertiesHelper(physicalDevice);
    uint32_t graphicsQueueFamilyIndex = INVALID_VK_INDEX;
    for (uint32_t j = 0; j < queueFamiliesProperties.size(); ++j) {
        VkQueueFamilyProperties props = queueFamiliesProperties[j];
        if (props.queueCount != 0 && props.queueFlags & flags) {
            graphicsQueueFamilyIndex = j;
            break;
        }
    }
    return graphicsQueueFamilyIndex;
}

// Provide a preference ordering of device types.
// Enum based on:
// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPhysicalDeviceType.html
inline int deviceTypeOrder(VkPhysicalDeviceType deviceType) {
    switch (deviceType) {
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            return 5;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            return 4;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            return 3;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            return 2;
        case VK_PHYSICAL_DEVICE_TYPE_OTHER:
            return 1;
        default:
            FVK_LOGW << "deviceTypeOrder: Unexpected deviceType: " << deviceType
                     << utils::io::endl;
            return -1;
    }
}

VkPhysicalDevice selectPhysicalDevice(VkInstance instance,
        VulkanPlatform::Customization::GPUPreference const& gpuPreference) {
    FixedCapacityVector<VkPhysicalDevice> const physicalDevices =
            filament::backend::enumerate(vkEnumeratePhysicalDevices, instance);
    struct DeviceInfo {
        VkPhysicalDevice device = VK_NULL_HANDLE;
        VkPhysicalDeviceType deviceType = VK_PHYSICAL_DEVICE_TYPE_OTHER;
        int8_t index = -1;
        std::string_view name;
    };
    FixedCapacityVector<DeviceInfo> deviceList(physicalDevices.size());

    for (size_t deviceInd = 0; deviceInd < physicalDevices.size(); ++deviceInd) {
        auto const candidateDevice = physicalDevices[deviceInd];
        VkPhysicalDeviceProperties targetDeviceProperties;
        vkGetPhysicalDeviceProperties(candidateDevice, &targetDeviceProperties);

        int const major = VK_VERSION_MAJOR(targetDeviceProperties.apiVersion);
        int const minor = VK_VERSION_MINOR(targetDeviceProperties.apiVersion);

        // Does the device support the required Vulkan level?
        if (major < FVK_REQUIRED_VERSION_MAJOR) {
            continue;
        }
        if (major == FVK_REQUIRED_VERSION_MAJOR && minor < FVK_REQUIRED_VERSION_MINOR) {
            continue;
        }

        // Does the device have any command queues that support graphics?
        // In theory, we should also ensure that the device supports presentation of our
        // particular VkSurface, but we don't have a VkSurface yet, so we'll skip this requirement.
        if (identifyGraphicsQueueFamilyIndex(candidateDevice, VK_QUEUE_GRAPHICS_BIT)
            == INVALID_VK_INDEX) {
            continue;
        }

        // Does the device support the VK_KHR_swapchain extension?
        FixedCapacityVector<VkExtensionProperties> const extensions
                = filament::backend::enumerate(vkEnumerateDeviceExtensionProperties,
                        candidateDevice, static_cast<char const*>(nullptr) /* pLayerName */);
        bool supportsSwapchain = false;
        for (auto const& extension: extensions) {
            if (!strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME)) {
                supportsSwapchain = true;
                break;
            }
        }
        if (!supportsSwapchain) {
            continue;
        }
        deviceList[deviceInd].device = candidateDevice;
        deviceList[deviceInd].deviceType = targetDeviceProperties.deviceType;
        deviceList[deviceInd].index = deviceInd;
        deviceList[deviceInd].name = targetDeviceProperties.deviceName;
    }

    FILAMENT_CHECK_PRECONDITION(gpuPreference.index < static_cast<int32_t>(deviceList.size()))
            << "Provided GPU index=" << gpuPreference.index
            << " >= the number of GPUs=" << static_cast<int32_t>(deviceList.size());

    // Sort the found devices
    std::sort(deviceList.begin(), deviceList.end(),
            [pref = gpuPreference](DeviceInfo const& a, DeviceInfo const& b) {
                if (b.device == VK_NULL_HANDLE) {
                    return false;
                }
                if (a.device == VK_NULL_HANDLE) {
                    return true;
                }
                if (!pref.deviceName.empty()) {
                    if (a.name.find(pref.deviceName.c_str()) != a.name.npos) {
                        return false;
                    }
                    if (b.name.find(pref.deviceName.c_str()) != b.name.npos) {
                        return true;
                    }
                }
                if (pref.index == a.index) {
                    return false;
                }
                if (pref.index == b.index) {
                    return true;
                }
                return deviceTypeOrder(a.deviceType) < deviceTypeOrder(b.deviceType);
            });
    auto device = deviceList.back().device;
    FILAMENT_CHECK_POSTCONDITION(device != VK_NULL_HANDLE) << "Unable to find suitable device.";
    return device;
}

VkFormatList findAttachmentDepthStencilFormats(VkPhysicalDevice device) {
    VkFormatFeatureFlags const features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

    // The ordering here indicates the preference of choosing depth+stencil format.
    VkFormat const formats[] = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_X8_D24_UNORM_PACK32,

        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT,
    };
    std::vector<VkFormat> selectedFormats;
    for (VkFormat format: formats) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(device, format, &props);
        if ((props.optimalTilingFeatures & features) == features) {
            selectedFormats.push_back(format);
        }
    }
    VkFormatList ret(selectedFormats.size());
    std::copy(selectedFormats.begin(), selectedFormats.end(), ret.begin());
    return ret;
}

VkFormatList findBlittableDepthStencilFormats(VkPhysicalDevice device) {
    std::vector<VkFormat> selectedFormats;
    VkFormatFeatureFlags const required = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT |
            VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT;
    for (VkFormat format : ALL_VK_FORMATS) {
        if (isVkDepthFormat(format)) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(device, format, &props);
            if ((props.optimalTilingFeatures & required) == required) {
                selectedFormats.push_back(format);
            }
        }
    }
    VkFormatList ret(selectedFormats.size());
    std::copy(selectedFormats.begin(), selectedFormats.end(), ret.begin());
    return ret;
}

}// anonymous namespace

using SwapChainPtr = VulkanPlatform::SwapChainPtr;

struct VulkanPlatformPrivate {
    VkInstance mInstance = VK_NULL_HANDLE;
    VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
    VkDevice mDevice = VK_NULL_HANDLE;
    uint32_t mGraphicsQueueFamilyIndex = INVALID_VK_INDEX;
    uint32_t mGraphicsQueueIndex = INVALID_VK_INDEX;
    VkQueue mGraphicsQueue = VK_NULL_HANDLE;
    uint32_t mProtectedGraphicsQueueFamilyIndex = INVALID_VK_INDEX;
    uint32_t mProtectedGraphicsQueueIndex = INVALID_VK_INDEX;
    VkQueue mProtectedGraphicsQueue = VK_NULL_HANDLE;
    VulkanContext mContext = {};

    // We use a map to both map a handle (i.e. SwapChainPtr) to the concrete type and also to
    // store the actual swapchain struct, which is either backed-by-surface or headless.
    std::unordered_set<SwapChainPtr> mSurfaceSwapChains;
    std::unordered_set<SwapChainPtr> mHeadlessSwapChains;

    bool mSharedContext = false;
    bool mForceXCBSwapchain = false;
};

void VulkanPlatform::terminate() {
    for (auto swapchain: mImpl->mHeadlessSwapChains) {
        delete static_cast<VulkanPlatformHeadlessSwapChain*>(swapchain);
    }
    mImpl->mHeadlessSwapChains.clear();

    for (auto swapchain: mImpl->mSurfaceSwapChains) {
        delete static_cast<VulkanPlatformSurfaceSwapChain*>(swapchain);
    }
    mImpl->mSurfaceSwapChains.clear();

    if (!mImpl->mSharedContext) {
        vkDestroyDevice(mImpl->mDevice, VKALLOC);
        vkDestroyInstance(mImpl->mInstance, VKALLOC);
    }
}

// This is the main entry point for context creation.
Driver* VulkanPlatform::createDriver(void* sharedContext,
        const Platform::DriverConfig& driverConfig) noexcept {
    // Load Vulkan entry points.
    FILAMENT_CHECK_POSTCONDITION(bluevk::initialize()) << "BlueVK is unable to load entry points.";

    if (sharedContext) {
        VulkanSharedContext const* scontext = (VulkanSharedContext const*) sharedContext;
        // All fields of VulkanSharedContext should be present.
        FILAMENT_CHECK_PRECONDITION(scontext->instance != VK_NULL_HANDLE)
                << "Client needs to provide VkInstance";
        FILAMENT_CHECK_PRECONDITION(scontext->physicalDevice != VK_NULL_HANDLE)
                << "Client needs to provide VkPhysicalDevice";
        FILAMENT_CHECK_PRECONDITION(scontext->logicalDevice != VK_NULL_HANDLE)
                << "Client needs to provide VkDevice";
        FILAMENT_CHECK_PRECONDITION(scontext->graphicsQueueFamilyIndex != INVALID_VK_INDEX)
                << "Client needs to provide graphics queue family index";
        FILAMENT_CHECK_PRECONDITION(scontext->graphicsQueueIndex != INVALID_VK_INDEX)
                << "Client needs to provide graphics queue index";

        mImpl->mInstance = scontext->instance;
        mImpl->mPhysicalDevice = scontext->physicalDevice;
        mImpl->mDevice = scontext->logicalDevice;
        mImpl->mGraphicsQueueFamilyIndex = scontext->graphicsQueueFamilyIndex;
        mImpl->mGraphicsQueueIndex = scontext->graphicsQueueIndex;

        mImpl->mSharedContext = true;
    }

    VulkanContext context;

    ExtensionSet instExts;
    // If using a shared context, we do not assume any extensions.
    if (!mImpl->mSharedContext) {
        // This constains instance extensions that are required for the platform, which includes
        // swapchain surface extensions.
        auto const& swapchainExts = getSwapchainInstanceExtensions();
        instExts = getInstanceExtensions(swapchainExts);

#if defined(FILAMENT_SUPPORTS_XCB) && defined(FILAMENT_SUPPORTS_XLIB)
        // For the special case where we're on linux and both xcb and xlib are "required", then we
        // check if the set of supported extensions contain both of them.  If only xcb is supported,
        // we force XCB surface creation.  This workaround is needed for the default swiftshader
        // build where only XCB is available.
        if (setContains(swapchainExts, VK_KHR_XCB_SURFACE_EXTENSION_NAME) &&
                setContains(swapchainExts, VK_KHR_XLIB_SURFACE_EXTENSION_NAME)) {
            // Assume only XCB is left, then we force the XCB path in the swapchain creation.
            mImpl->mForceXCBSwapchain = !setContains(instExts, VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
            assert_invariant(!mImpl->mForceXCBSwapchain ||
                             setContains(instExts, VK_KHR_XCB_SURFACE_EXTENSION_NAME));
        }
#endif

        instExts.merge(getRequiredInstanceExtensions());
    }

    mImpl->mInstance
            = mImpl->mInstance == VK_NULL_HANDLE ? createInstance(instExts) : mImpl->mInstance;
    assert_invariant(mImpl->mInstance != VK_NULL_HANDLE);

    bluevk::bindInstance(mImpl->mInstance);

    VulkanPlatform::Customization::GPUPreference const pref = getCustomization().gpu;
    bool const hasGPUPreference = pref.index >= 0 || !pref.deviceName.empty();
    FILAMENT_CHECK_PRECONDITION(!(hasGPUPreference && sharedContext))
            << "Cannot both share context and indicate GPU preference";

    mImpl->mPhysicalDevice = mImpl->mPhysicalDevice == VK_NULL_HANDLE
                                     ? selectPhysicalDevice(mImpl->mInstance, pref)
                                     : mImpl->mPhysicalDevice;
    assert_invariant(mImpl->mPhysicalDevice != VK_NULL_HANDLE);

    printDeviceInfo(mImpl->mInstance, mImpl->mPhysicalDevice);

    VkPhysicalDeviceProtectedMemoryFeatures queryProtectedMemoryFeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES,
    };
    VkPhysicalDeviceProtectedMemoryProperties protectedMemoryProperties = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_PROPERTIES,
    };
    chainStruct(&context.mPhysicalDeviceFeatures, &queryProtectedMemoryFeatures);
    chainStruct(&context.mPhysicalDeviceProperties, &protectedMemoryProperties);

    // Initialize the following fields: physicalDeviceProperties, memoryProperties,
    // physicalDeviceFeatures, graphicsQueueFamilyIndex.
    vkGetPhysicalDeviceProperties2(mImpl->mPhysicalDevice, &context.mPhysicalDeviceProperties);
    vkGetPhysicalDeviceFeatures2(mImpl->mPhysicalDevice, &context.mPhysicalDeviceFeatures);
    vkGetPhysicalDeviceMemoryProperties(mImpl->mPhysicalDevice, &context.mMemoryProperties);

    mImpl->mGraphicsQueueFamilyIndex
            = mImpl->mGraphicsQueueFamilyIndex == INVALID_VK_INDEX
                      ? identifyGraphicsQueueFamilyIndex(mImpl->mPhysicalDevice,
                          VK_QUEUE_GRAPHICS_BIT)
                      : mImpl->mGraphicsQueueFamilyIndex;
    assert_invariant(mImpl->mGraphicsQueueFamilyIndex != INVALID_VK_INDEX);

    // We know we need to allocate the protected version of the VK objects
    context.mProtectedMemorySupported =
        static_cast<bool>(queryProtectedMemoryFeatures.protectedMemory);
    if (context.mProtectedMemorySupported) {
        mImpl->mProtectedGraphicsQueueFamilyIndex
            = identifyGraphicsQueueFamilyIndex(mImpl->mPhysicalDevice,
                (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_PROTECTED_BIT));
        assert_invariant(mImpl->mProtectedGraphicsQueueFamilyIndex != INVALID_VK_INDEX);
    }

    // Only enable shaderClipDistance if we are doing instanced stereoscopic rendering.
    if (context.mPhysicalDeviceFeatures.features.shaderClipDistance == VK_TRUE
            && driverConfig.stereoscopicType != StereoscopicType::INSTANCED) {
        context.mPhysicalDeviceFeatures.features.shaderClipDistance = VK_FALSE;
    }

    // At this point, we should have a family index that points to a family that has > 0 queues for
    // graphics. In which case, we will allocate one queue for all of Filament (and assumes at least
    // one has been allocated by the client if context was shared). If the index of the target queue
    // within the family hasn't been provided by the client, we assume it to be 0.
    mImpl->mGraphicsQueueIndex
            = mImpl->mGraphicsQueueIndex == INVALID_VK_INDEX ? 0 : mImpl->mGraphicsQueueIndex;

    // Applying the same logic to the protected queue index (Not sure about shared context and protection)
    mImpl->mProtectedGraphicsQueueIndex
        = mImpl->mProtectedGraphicsQueueIndex == INVALID_VK_INDEX ? 0 :
        mImpl->mProtectedGraphicsQueueIndex;

    ExtensionSet deviceExts;
    // If using a shared context, we do not assume any extensions.
    if (!mImpl->mSharedContext) {
        deviceExts = getDeviceExtensions(mImpl->mPhysicalDevice);
        auto [prunedInstExts, prunedDeviceExts]
                = pruneExtensions(mImpl->mPhysicalDevice, instExts, deviceExts);
        instExts = prunedInstExts;
        deviceExts = prunedDeviceExts;

        if (driverConfig.stereoscopicType != StereoscopicType::MULTIVIEW) {
            deviceExts.erase(VK_KHR_MULTIVIEW_EXTENSION_NAME);
        }
    }

    bool requestPortabilitySubsetImageView2DOn3DImage = false;
    if (setContains(deviceExts, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)) {
        // We are on a non-conformant vulkan implementation so we need to ascertain if the features
        // we need are available.

        chainStruct(&context.mPhysicalDeviceFeatures, &context.mPortabilitySubsetFeatures);
        vkGetPhysicalDeviceFeatures2(mImpl->mPhysicalDevice, &context.mPhysicalDeviceFeatures);
        requestPortabilitySubsetImageView2DOn3DImage =
                context.mPortabilitySubsetFeatures.imageView2DOn3DImage == VK_TRUE;
    }

    mImpl->mDevice =
            mImpl->mDevice == VK_NULL_HANDLE
                    ? createLogicalDevice(mImpl->mPhysicalDevice, context.mPhysicalDeviceFeatures,
                              mImpl->mGraphicsQueueFamilyIndex,
                              mImpl->mProtectedGraphicsQueueFamilyIndex, deviceExts,
                              requestPortabilitySubsetImageView2DOn3DImage)
                    : mImpl->mDevice;
    assert_invariant(mImpl->mDevice != VK_NULL_HANDLE);

    vkGetDeviceQueue(mImpl->mDevice, mImpl->mGraphicsQueueFamilyIndex, mImpl->mGraphicsQueueIndex,
            &mImpl->mGraphicsQueue);
    assert_invariant(mImpl->mGraphicsQueue != VK_NULL_HANDLE);

    if (context.mProtectedMemorySupported) {
        VkDeviceQueueInfo2 info = {};
        info.queueFamilyIndex = mImpl->mProtectedGraphicsQueueFamilyIndex;
        info.queueIndex = mImpl->mProtectedGraphicsQueueIndex;
        info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2;
        info.flags = VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT;
        vkGetDeviceQueue2(mImpl->mDevice, &info, &mImpl->mProtectedGraphicsQueue);
    }

    // Store the extension support in the context
    context.mDebugUtilsSupported = setContains(instExts, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    context.mDebugMarkersSupported = setContains(deviceExts, VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
    context.mMultiviewEnabled = setContains(deviceExts, VK_KHR_MULTIVIEW_EXTENSION_NAME);

    // Check the availability of lazily allocated memory
    {
        context.mLazilyAllocatedMemorySupported = false;
        const uint32_t typeCount = context.mMemoryProperties.memoryTypeCount;
        for(uint32_t i = 0; i < typeCount; ++i) {
            const VkMemoryType type = context.mMemoryProperties.memoryTypes[i];
            if (type.propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) {
                context.mLazilyAllocatedMemorySupported = true;
                assert_invariant(type.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                break;
            }
        }
    }

#ifdef NDEBUG
    // If we are in release build, we should not have turned on debug extensions
    FILAMENT_CHECK_POSTCONDITION(!context.mDebugUtilsSupported && !context.mDebugMarkersSupported)
            << "Debug utils should not be enabled in release build.";
#endif

    context.mDepthStencilFormats = findAttachmentDepthStencilFormats(mImpl->mPhysicalDevice);
    context.mBlittableDepthStencilFormats =
            findBlittableDepthStencilFormats(mImpl->mPhysicalDevice);

    assert_invariant(context.mDepthStencilFormats.size() > 0);

#if FVK_ENABLED(FVK_DEBUG_VALIDATION)
    printDepthFormats(mImpl->mPhysicalDevice);
#endif

    // Keep a copy of context for swapchains.
    mImpl->mContext = context;

    return VulkanDriver::create(this, context, driverConfig);
}

// This needs to be explictly written for
// utils::PrivateImplementation<VulkanPlatformPrivate>::PrivateImplementation() to be properly
// defined.
VulkanPlatform::VulkanPlatform() = default;

VulkanPlatform::~VulkanPlatform() = default;

VulkanPlatform::SwapChainBundle VulkanPlatform::getSwapChainBundle(SwapChainPtr handle) {
    SWAPCHAIN_RET_FUNC(getSwapChainBundle, handle, )
}

VkResult VulkanPlatform::acquire(SwapChainPtr handle, ImageSyncData* outImageSyncData) {
    SWAPCHAIN_RET_FUNC(acquire, handle, outImageSyncData)
}

VkResult VulkanPlatform::present(SwapChainPtr handle, uint32_t index,
        VkSemaphore finishedDrawing) {
    SWAPCHAIN_RET_FUNC(present, handle, index, finishedDrawing)
}

bool VulkanPlatform::hasResized(SwapChainPtr handle) {
    SWAPCHAIN_RET_FUNC(hasResized, handle, )
}

bool VulkanPlatform::isProtected(SwapChainPtr handle) {
    SWAPCHAIN_RET_FUNC(isProtected, handle, )
}

VkResult VulkanPlatform::recreate(SwapChainPtr handle) {
    SWAPCHAIN_RET_FUNC(recreate, handle, )
}

void VulkanPlatform::destroy(SwapChainPtr handle) {
    if (mImpl->mSurfaceSwapChains.erase(handle)) {
        delete static_cast<VulkanPlatformSurfaceSwapChain*>(handle);
    } else if (mImpl->mHeadlessSwapChains.erase(handle)) {
        delete static_cast<VulkanPlatformHeadlessSwapChain*>(handle);
    } else {
        PANIC_PRECONDITION("Bad handle for swapchain");
    }
}

SwapChainPtr VulkanPlatform::createSwapChain(void* nativeWindow, uint64_t flags,
        VkExtent2D extent) {
    assert_invariant(nativeWindow || (extent.width != 0 && extent.height != 0));
    bool const headless = extent.width != 0 && extent.height != 0;
    if (headless) {
        VulkanPlatformHeadlessSwapChain* swapchain = new VulkanPlatformHeadlessSwapChain(
                mImpl->mContext, mImpl->mDevice, mImpl->mGraphicsQueue, extent, flags);
        mImpl->mHeadlessSwapChains.insert(swapchain);
        return swapchain;
    }

    if (mImpl->mForceXCBSwapchain) {
        flags |= SWAP_CHAIN_CONFIG_ENABLE_XCB;
    }

    if (flags & backend::SWAP_CHAIN_CONFIG_PROTECTED_CONTENT) {
        if (!mImpl->mContext.mProtectedMemorySupported) {
            utils::slog.w << "protected swapchain requested, but VulkanPlatform does not support it"
                << utils::io::endl;
        }
    }

    auto [surface, fallbackExtent] = createVkSurfaceKHR(nativeWindow, mImpl->mInstance, flags);
    // The VulkanPlatformSurfaceSwapChain now `owns` the surface.
    VulkanPlatformSurfaceSwapChain* swapchain = new VulkanPlatformSurfaceSwapChain(mImpl->mContext,
            mImpl->mPhysicalDevice, mImpl->mDevice, mImpl->mGraphicsQueue, mImpl->mInstance,
            surface, fallbackExtent, flags);
    mImpl->mSurfaceSwapChains.insert(swapchain);
    return swapchain;
}

VkInstance VulkanPlatform::getInstance() const noexcept {
    return mImpl->mInstance;
}

VkDevice VulkanPlatform::getDevice() const noexcept {
    return mImpl->mDevice;
}

VkPhysicalDevice VulkanPlatform::getPhysicalDevice() const noexcept {
    return mImpl->mPhysicalDevice;
}

uint32_t VulkanPlatform::getGraphicsQueueFamilyIndex() const noexcept {
    return mImpl->mGraphicsQueueFamilyIndex;
}

uint32_t VulkanPlatform::getGraphicsQueueIndex() const noexcept {
    return mImpl->mGraphicsQueueIndex;
}

VkQueue VulkanPlatform::getGraphicsQueue() const noexcept {
    return mImpl->mGraphicsQueue;
}

uint32_t VulkanPlatform::getProtectedGraphicsQueueFamilyIndex() const noexcept {
    return mImpl->mProtectedGraphicsQueueFamilyIndex;
}

uint32_t VulkanPlatform::getProtectedGraphicsQueueIndex() const noexcept {
    return mImpl->mProtectedGraphicsQueueIndex;
}

VkQueue VulkanPlatform::getProtectedGraphicsQueue() const noexcept {
    return mImpl->mProtectedGraphicsQueue;
}

VulkanPlatform::ExternalImageMetadata VulkanPlatform::getExternalImageMetadata(
        void* externalImage) {
    return getExternalImageMetadataImpl(externalImage, mImpl->mDevice);
}

VulkanPlatform::ImageData VulkanPlatform::createExternalImage(void* externalImage,
            const ExternalImageMetadata& metadata) {
    return createExternalImageImpl(externalImage, mImpl->mDevice, nullptr, metadata);
}

#undef SWAPCHAIN_RET_FUNC

}// namespace filament::backend
