// Copyright 2020 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_VULKAN_VULKANEXTENSIONS_H_
#define SRC_DAWN_NATIVE_VULKAN_VULKANEXTENSIONS_H_

#include <string>

#include "absl/container/flat_hash_map.h"
#include "dawn/common/ityp_bitset.h"

namespace dawn::native::vulkan {

// The list of known instance extensions. They must be in dependency order (this is checked
// inside EnsureDependencies)
enum class InstanceExt {
    // Promoted to 1.1
    GetPhysicalDeviceProperties2,
    ExternalMemoryCapabilities,
    ExternalSemaphoreCapabilities,

    // Surface extensions
    Surface,
    FuchsiaImagePipeSurface,
    MetalSurface,
    WaylandSurface,
    Win32Surface,
    XcbSurface,
    XlibSurface,
    AndroidSurface,

    // Others
    DebugUtils,
    ValidationFeatures,

    EnumCount,
};

// A bitset that is indexed with InstanceExt.
using InstanceExtSet = ityp::bitset<InstanceExt, static_cast<uint32_t>(InstanceExt::EnumCount)>;

// Information about a known instance extension.
struct InstanceExtInfo {
    InstanceExt index;
    const char* name;
    // The version in which this extension was promoted as built with VK_API_VERSION_1_x,
    // or NeverPromoted if it was never promoted.
    uint32_t versionPromoted;
};

// Returns the information about a known InstanceExt
const InstanceExtInfo& GetInstanceExtInfo(InstanceExt ext);
// Returns a map that maps a Vulkan extension name to its InstanceExt.
absl::flat_hash_map<std::string, InstanceExt> CreateInstanceExtNameMap();

// Sets entries in `extensions` to true if that entry was promoted in Vulkan version `version`
void MarkPromotedExtensions(InstanceExtSet* extensions, uint32_t version);
// From a set of extensions advertised as supported by the instance (or promoted), remove all
// extensions that don't have all their transitive dependencies in advertisedExts.
InstanceExtSet EnsureDependencies(const InstanceExtSet& advertisedExts);

// The list of known device extensions. They must be in dependency order (this is checked
// inside EnsureDependencies)
enum class DeviceExt {
    // Promoted to 1.1
    BindMemory2,
    Maintenance1,
    Maintenance2,
    Maintenance3,
    StorageBufferStorageClass,
    GetPhysicalDeviceProperties2,
    GetMemoryRequirements2,
    DedicatedAllocation,
    ExternalMemoryCapabilities,
    ExternalSemaphoreCapabilities,
    ExternalMemory,
    ExternalSemaphore,
    _16BitStorage,
    SamplerYCbCrConversion,

    // Promoted to 1.2
    DriverProperties,
    ImageFormatList,
    ShaderFloat16Int8,
    ShaderSubgroupExtendedTypes,
    DrawIndirectCount,
    VulkanMemoryModel,
    ShaderFloatControls,
    Spirv14,

    // Promoted to 1.3
    ShaderIntegerDotProduct,
    ZeroInitializeWorkgroupMemory,
    DemoteToHelperInvocation,
    Maintenance4,
    SubgroupSizeControl,

    // Others
    DepthClipEnable,
    ImageDrmFormatModifier,
    Swapchain,
    QueueFamilyForeign,
    Robustness2,
    DisplayTiming,
    CooperativeMatrix,

    // External* extensions
    ExternalMemoryAndroidHardwareBuffer,
    ExternalMemoryFD,
    ExternalMemoryDmaBuf,
    ExternalMemoryZirconHandle,
    ExternalMemoryHost,
    ExternalSemaphoreFD,
    ExternalSemaphoreZirconHandle,

    EnumCount,
};

// A bitset that is indexed with DeviceExt.
using DeviceExtSet = ityp::bitset<DeviceExt, static_cast<uint32_t>(DeviceExt::EnumCount)>;

// Information about a known device extension.
struct DeviceExtInfo {
    DeviceExt index;
    const char* name;
    // The version in which this extension was promoted as built with VK_API_VERSION_1_x,
    // or NeverPromoted if it was never promoted.
    uint32_t versionPromoted;
};

// Returns the information about a known DeviceExt
const DeviceExtInfo& GetDeviceExtInfo(DeviceExt ext);
// Returns a map that maps a Vulkan extension name to its DeviceExt.
absl::flat_hash_map<std::string, DeviceExt> CreateDeviceExtNameMap();

// Sets entries in `extensions` to true if that entry was promoted in Vulkan version `version`
void MarkPromotedExtensions(DeviceExtSet* extensions, uint32_t version);
// From a set of extensions advertised as supported by the device (or promoted), remove all
// extensions that don't have all their transitive dependencies in advertisedExts or in
// instanceExts.
DeviceExtSet EnsureDependencies(const DeviceExtSet& advertisedExts,
                                const InstanceExtSet& instanceExts,
                                uint32_t version);

// The list of all known Vulkan layers.
enum class VulkanLayer {
    Validation,
    LunargVkTrace,
    RenderDocCapture,

    // Fuchsia implements the swapchain through a layer (VK_LAYER_FUCHSIA_image_pipe_swapchain),
    // which adds an instance extensions (VK_FUCHSIA_image_surface) to all ICDs.
    FuchsiaImagePipeSwapchain,

    EnumCount,
};

// A bitset that is indexed with VulkanLayer.
using VulkanLayerSet = ityp::bitset<VulkanLayer, static_cast<uint32_t>(VulkanLayer::EnumCount)>;

// Information about a known layer
struct VulkanLayerInfo {
    VulkanLayer layer;
    const char* name;
};

// Returns the information about a known VulkanLayer
const VulkanLayerInfo& GetVulkanLayerInfo(VulkanLayer layer);
// Returns a map that maps a Vulkan layer name to its VulkanLayer.
absl::flat_hash_map<std::string, VulkanLayer> CreateVulkanLayerNameMap();

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_VULKANEXTENSIONS_H_
