// Copyright 2017 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_NATIVE_VULKAN_VULKANINFO_H_
#define SRC_DAWN_NATIVE_VULKAN_VULKANINFO_H_

#include <vector>

#include "dawn/common/ityp_array.h"
#include "dawn/common/vulkan_platform.h"
#include "dawn/native/Error.h"
#include "dawn/native/vulkan/VulkanExtensions.h"

namespace dawn::native::vulkan {

class PhysicalDevice;
class Backend;
struct VulkanFunctions;

// Global information - gathered before the instance is created
struct VulkanGlobalKnobs {
    VulkanLayerSet layers;
    ityp::array<VulkanLayer, InstanceExtSet, static_cast<uint32_t>(VulkanLayer::EnumCount)>
        layerExtensions;

    // During information gathering `extensions` only contains the instance's extensions but
    // during the instance creation logic it becomes the OR of the instance's extensions and
    // the selected layers' extensions.
    InstanceExtSet extensions;
    bool HasExt(InstanceExt ext) const;
};

struct VulkanGlobalInfo : VulkanGlobalKnobs {
    uint32_t apiVersion;
};

// Device information - gathered before the device is created.
struct VulkanDeviceKnobs {
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceShaderFloat16Int8FeaturesKHR shaderFloat16Int8Features;
    VkPhysicalDevice16BitStorageFeaturesKHR _16BitStorageFeatures;
    VkPhysicalDeviceSubgroupSizeControlFeaturesEXT subgroupSizeControlFeatures;
    VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeaturesKHR zeroInitializeWorkgroupMemoryFeatures;
    VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT demoteToHelperInvocationFeatures;
    VkPhysicalDeviceShaderIntegerDotProductFeaturesKHR shaderIntegerDotProductFeatures;
    VkPhysicalDeviceDepthClipEnableFeaturesEXT depthClipEnableFeatures;
    VkPhysicalDeviceRobustness2FeaturesEXT robustness2Features;
    VkPhysicalDeviceSamplerYcbcrConversionFeatures samplerYCbCrConversionFeatures;
    VkPhysicalDeviceShaderSubgroupExtendedTypesFeaturesKHR shaderSubgroupExtendedTypes;
    VkPhysicalDeviceVulkanMemoryModelFeatures vulkanMemoryModelFeatures;
    VkPhysicalDeviceCooperativeMatrixFeaturesKHR cooperativeMatrixFeatures;

    bool HasExt(DeviceExt ext) const;
    DeviceExtSet extensions;
};

struct VulkanDeviceInfo : VulkanDeviceKnobs {
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceMaintenance3Properties propertiesMaintenance3;
    VkPhysicalDeviceDriverProperties driverProperties;
    VkPhysicalDeviceSubgroupSizeControlPropertiesEXT subgroupSizeControlProperties;
    VkPhysicalDeviceShaderIntegerDotProductPropertiesKHR shaderIntegerDotProductProperties;
    VkPhysicalDeviceMaintenance4Properties propertiesMaintenance4;
    VkPhysicalDeviceSubgroupProperties subgroupProperties;
    VkPhysicalDeviceExternalMemoryHostPropertiesEXT externalMemoryHostProperties;
    VkPhysicalDeviceCooperativeMatrixPropertiesKHR cooperativeMatrixProperties;

    std::vector<VkQueueFamilyProperties> queueFamilies;
    std::vector<VkCooperativeMatrixPropertiesKHR> cooperativeMatrixConfigs;

    std::vector<VkMemoryType> memoryTypes;
    std::vector<VkMemoryHeap> memoryHeaps;

    std::vector<VkLayerProperties> layers;
    // TODO(cwallez@chromium.org): layer instance extensions
};

struct VulkanSurfaceInfo {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
    std::vector<bool> supportedQueueFamilies;
};

ResultOrError<VulkanGlobalInfo> GatherGlobalInfo(const VulkanFunctions& vkFunctions);
ResultOrError<std::vector<VkPhysicalDevice>> GatherPhysicalDevices(
    VkInstance instance,
    const VulkanFunctions& vkFunctions);
ResultOrError<VulkanDeviceInfo> GatherDeviceInfo(const PhysicalDevice& physicalDevice);
ResultOrError<VulkanSurfaceInfo> GatherSurfaceInfo(const PhysicalDevice& physicalDevice,
                                                   VkSurfaceKHR surface);
}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_VULKANINFO_H_
