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

#include "dawn/native/vulkan/VulkanExtensions.h"

#include <array>
#include <limits>

#include "dawn/common/Assert.h"
#include "dawn/common/vulkan_platform.h"

namespace dawn::native::vulkan {

static constexpr uint32_t VulkanVersion_1_1 = VK_API_VERSION_1_1;
static constexpr uint32_t VulkanVersion_1_2 = VK_API_VERSION_1_2;
static constexpr uint32_t VulkanVersion_1_3 = VK_API_VERSION_1_3;
static constexpr uint32_t NeverPromoted = std::numeric_limits<uint32_t>::max();

// A static array for InstanceExtInfo that can be indexed with InstanceExts.
// GetInstanceExtInfo checks that "index" matches the index used to access this array so an
// assert will fire if it isn't in the correct order.
static constexpr size_t kInstanceExtCount = static_cast<size_t>(InstanceExt::EnumCount);
static constexpr std::array<InstanceExtInfo, kInstanceExtCount> sInstanceExtInfos{{
    //
    {InstanceExt::GetPhysicalDeviceProperties2, "VK_KHR_get_physical_device_properties2",
     VulkanVersion_1_1},
    {InstanceExt::ExternalMemoryCapabilities, "VK_KHR_external_memory_capabilities",
     VulkanVersion_1_1},
    {InstanceExt::ExternalSemaphoreCapabilities, "VK_KHR_external_semaphore_capabilities",
     VulkanVersion_1_1},

    {InstanceExt::Surface, "VK_KHR_surface", NeverPromoted},
    {InstanceExt::FuchsiaImagePipeSurface, "VK_FUCHSIA_imagepipe_surface", NeverPromoted},
    {InstanceExt::MetalSurface, "VK_EXT_metal_surface", NeverPromoted},
    {InstanceExt::WaylandSurface, "VK_KHR_wayland_surface", NeverPromoted},
    {InstanceExt::Win32Surface, "VK_KHR_win32_surface", NeverPromoted},
    {InstanceExt::XcbSurface, "VK_KHR_xcb_surface", NeverPromoted},
    {InstanceExt::XlibSurface, "VK_KHR_xlib_surface", NeverPromoted},
    {InstanceExt::AndroidSurface, "VK_KHR_android_surface", NeverPromoted},

    {InstanceExt::DebugUtils, "VK_EXT_debug_utils", NeverPromoted},
    {InstanceExt::ValidationFeatures, "VK_EXT_validation_features", NeverPromoted},
    //
}};

const InstanceExtInfo& GetInstanceExtInfo(InstanceExt ext) {
    uint32_t index = static_cast<uint32_t>(ext);
    DAWN_ASSERT(index < sInstanceExtInfos.size());
    DAWN_ASSERT(sInstanceExtInfos[index].index == ext);
    return sInstanceExtInfos[index];
}

absl::flat_hash_map<std::string, InstanceExt> CreateInstanceExtNameMap() {
    absl::flat_hash_map<std::string, InstanceExt> result;
    for (const InstanceExtInfo& info : sInstanceExtInfos) {
        result.emplace(info.name, info.index);
    }
    return result;
}

InstanceExtSet EnsureDependencies(const InstanceExtSet& advertisedExts) {
    // We need to check that all transitive dependencies of extensions are advertised.
    // To do that in a single pass and no data structures, the extensions are topologically
    // sorted in the definition of InstanceExt.
    // To ensure the order is correct, we mark visited extensions in `visitedSet` and each
    // dependency check will first assert all its dependents have been visited.
    InstanceExtSet visitedSet;
    InstanceExtSet trimmedSet;

    auto HasDep = [&](InstanceExt ext) -> bool {
        DAWN_ASSERT(visitedSet[ext]);
        return trimmedSet[ext];
    };

    for (uint32_t i = 0; i < sInstanceExtInfos.size(); i++) {
        InstanceExt ext = static_cast<InstanceExt>(i);

        bool hasDependencies = false;
        switch (ext) {
            case InstanceExt::GetPhysicalDeviceProperties2:
            case InstanceExt::Surface:
            case InstanceExt::DebugUtils:
            case InstanceExt::ValidationFeatures:
                hasDependencies = true;
                break;

            case InstanceExt::ExternalMemoryCapabilities:
            case InstanceExt::ExternalSemaphoreCapabilities:
                hasDependencies = HasDep(InstanceExt::GetPhysicalDeviceProperties2);
                break;

            case InstanceExt::AndroidSurface:
            case InstanceExt::FuchsiaImagePipeSurface:
            case InstanceExt::MetalSurface:
            case InstanceExt::WaylandSurface:
            case InstanceExt::Win32Surface:
            case InstanceExt::XcbSurface:
            case InstanceExt::XlibSurface:
                hasDependencies = HasDep(InstanceExt::Surface);
                break;

            case InstanceExt::EnumCount:
                DAWN_UNREACHABLE();
        }

        trimmedSet.set(ext, hasDependencies && advertisedExts[ext]);
        visitedSet.set(ext, true);
    }

    return trimmedSet;
}

void MarkPromotedExtensions(InstanceExtSet* extensions, uint32_t version) {
    for (const InstanceExtInfo& info : sInstanceExtInfos) {
        if (info.versionPromoted <= version) {
            extensions->set(info.index, true);
        }
    }
}

static constexpr size_t kDeviceExtCount = static_cast<size_t>(DeviceExt::EnumCount);
static constexpr std::array<DeviceExtInfo, kDeviceExtCount> sDeviceExtInfos{{
    //
    {DeviceExt::BindMemory2, "VK_KHR_bind_memory2", VulkanVersion_1_1},
    {DeviceExt::Maintenance1, "VK_KHR_maintenance1", VulkanVersion_1_1},
    {DeviceExt::Maintenance2, "VK_KHR_maintenance2", VulkanVersion_1_1},
    {DeviceExt::Maintenance3, "VK_KHR_maintenance3", VulkanVersion_1_1},
    {DeviceExt::StorageBufferStorageClass, "VK_KHR_storage_buffer_storage_class",
     VulkanVersion_1_1},
    {DeviceExt::GetPhysicalDeviceProperties2, "VK_KHR_get_physical_device_properties2",
     VulkanVersion_1_1},
    {DeviceExt::GetMemoryRequirements2, "VK_KHR_get_memory_requirements2", VulkanVersion_1_1},
    {DeviceExt::DedicatedAllocation, "VK_KHR_dedicated_allocation", VulkanVersion_1_1},
    {DeviceExt::ExternalMemoryCapabilities, "VK_KHR_external_memory_capabilities",
     VulkanVersion_1_1},
    {DeviceExt::ExternalSemaphoreCapabilities, "VK_KHR_external_semaphore_capabilities",
     VulkanVersion_1_1},
    {DeviceExt::ExternalMemory, "VK_KHR_external_memory", VulkanVersion_1_1},
    {DeviceExt::ExternalSemaphore, "VK_KHR_external_semaphore", VulkanVersion_1_1},
    {DeviceExt::_16BitStorage, "VK_KHR_16bit_storage", VulkanVersion_1_1},
    {DeviceExt::SamplerYCbCrConversion, "VK_KHR_sampler_ycbcr_conversion", VulkanVersion_1_1},

    {DeviceExt::DriverProperties, "VK_KHR_driver_properties", VulkanVersion_1_2},
    {DeviceExt::ImageFormatList, "VK_KHR_image_format_list", VulkanVersion_1_2},
    {DeviceExt::ShaderFloat16Int8, "VK_KHR_shader_float16_int8", VulkanVersion_1_2},
    {DeviceExt::ShaderSubgroupExtendedTypes, "VK_KHR_shader_subgroup_extended_types",
     VulkanVersion_1_2},
    {DeviceExt::DrawIndirectCount, "VK_KHR_draw_indirect_count", NeverPromoted},
    {DeviceExt::VulkanMemoryModel, "VK_KHR_vulkan_memory_model", VulkanVersion_1_2},
    {DeviceExt::ShaderFloatControls, "VK_KHR_shader_float_controls", VulkanVersion_1_2},
    {DeviceExt::Spirv14, "VK_KHR_spirv_1_4", VulkanVersion_1_2},

    {DeviceExt::ShaderIntegerDotProduct, "VK_KHR_shader_integer_dot_product", VulkanVersion_1_3},
    {DeviceExt::ZeroInitializeWorkgroupMemory, "VK_KHR_zero_initialize_workgroup_memory",
     VulkanVersion_1_3},
    {DeviceExt::DemoteToHelperInvocation, "VK_EXT_shader_demote_to_helper_invocation",
     VulkanVersion_1_3},
    {DeviceExt::Maintenance4, "VK_KHR_maintenance4", VulkanVersion_1_3},
    {DeviceExt::SubgroupSizeControl, "VK_EXT_subgroup_size_control", VulkanVersion_1_3},

    {DeviceExt::DepthClipEnable, "VK_EXT_depth_clip_enable", NeverPromoted},
    {DeviceExt::ImageDrmFormatModifier, "VK_EXT_image_drm_format_modifier", NeverPromoted},
    {DeviceExt::Swapchain, "VK_KHR_swapchain", NeverPromoted},
    {DeviceExt::QueueFamilyForeign, "VK_EXT_queue_family_foreign", NeverPromoted},
    {DeviceExt::Robustness2, "VK_EXT_robustness2", NeverPromoted},
    {DeviceExt::DisplayTiming, "VK_GOOGLE_display_timing", NeverPromoted},
    {DeviceExt::CooperativeMatrix, "VK_KHR_cooperative_matrix", NeverPromoted},

    {DeviceExt::ExternalMemoryAndroidHardwareBuffer,
     "VK_ANDROID_external_memory_android_hardware_buffer", NeverPromoted},
    {DeviceExt::ExternalMemoryFD, "VK_KHR_external_memory_fd", NeverPromoted},
    {DeviceExt::ExternalMemoryDmaBuf, "VK_EXT_external_memory_dma_buf", NeverPromoted},
    {DeviceExt::ExternalMemoryZirconHandle, "VK_FUCHSIA_external_memory", NeverPromoted},
    {DeviceExt::ExternalMemoryHost, "VK_EXT_external_memory_host", NeverPromoted},
    {DeviceExt::ExternalSemaphoreFD, "VK_KHR_external_semaphore_fd", NeverPromoted},
    {DeviceExt::ExternalSemaphoreZirconHandle, "VK_FUCHSIA_external_semaphore", NeverPromoted},
    //
}};

const DeviceExtInfo& GetDeviceExtInfo(DeviceExt ext) {
    uint32_t index = static_cast<uint32_t>(ext);
    DAWN_ASSERT(index < sDeviceExtInfos.size());
    DAWN_ASSERT(sDeviceExtInfos[index].index == ext);
    return sDeviceExtInfos[index];
}

absl::flat_hash_map<std::string, DeviceExt> CreateDeviceExtNameMap() {
    absl::flat_hash_map<std::string, DeviceExt> result;
    for (const DeviceExtInfo& info : sDeviceExtInfos) {
        result.emplace(info.name, info.index);
    }
    return result;
}

DeviceExtSet EnsureDependencies(const DeviceExtSet& advertisedExts,
                                const InstanceExtSet& instanceExts,
                                uint32_t version) {
    // This is very similar to EnsureDependencies for instanceExtSet. See comment there for
    // an explanation of what happens.
    DeviceExtSet visitedSet;
    DeviceExtSet trimmedSet;

    auto HasDep = [&](DeviceExt ext) -> bool {
        DAWN_ASSERT(visitedSet[ext]);
        return trimmedSet[ext];
    };

    for (uint32_t i = 0; i < sDeviceExtInfos.size(); i++) {
        DeviceExt ext = static_cast<DeviceExt>(i);

        bool hasDependencies = false;
        switch (ext) {
            // Happy extensions don't need anybody else!
            case DeviceExt::BindMemory2:
            case DeviceExt::GetMemoryRequirements2:
            case DeviceExt::Maintenance1:
            case DeviceExt::Maintenance2:
            case DeviceExt::ImageFormatList:
            case DeviceExt::StorageBufferStorageClass:
            case DeviceExt::DrawIndirectCount:
                hasDependencies = true;
                break;

            case DeviceExt::DedicatedAllocation:
                hasDependencies = HasDep(DeviceExt::GetMemoryRequirements2);
                break;

            // Physical device extensions technically don't require the instance to support
            // them but VulkanFunctions only loads the function pointers if the instance
            // advertises the extension. So if we didn't have this check, we'd risk a calling
            // a nullptr.
            case DeviceExt::GetPhysicalDeviceProperties2:
            case DeviceExt::Maintenance3:
                hasDependencies = instanceExts[InstanceExt::GetPhysicalDeviceProperties2];
                break;
            case DeviceExt::ExternalMemoryCapabilities:
                hasDependencies = instanceExts[InstanceExt::ExternalMemoryCapabilities] &&
                                  HasDep(DeviceExt::GetPhysicalDeviceProperties2);
                break;
            case DeviceExt::ExternalSemaphoreCapabilities:
                hasDependencies = instanceExts[InstanceExt::ExternalSemaphoreCapabilities] &&
                                  HasDep(DeviceExt::GetPhysicalDeviceProperties2);
                break;

            case DeviceExt::ImageDrmFormatModifier:
                hasDependencies = HasDep(DeviceExt::BindMemory2) &&
                                  HasDep(DeviceExt::GetPhysicalDeviceProperties2) &&
                                  HasDep(DeviceExt::ImageFormatList) &&
                                  HasDep(DeviceExt::SamplerYCbCrConversion);
                break;

            case DeviceExt::Swapchain:
                hasDependencies = instanceExts[InstanceExt::Surface];
                break;

            case DeviceExt::SamplerYCbCrConversion:
                hasDependencies = HasDep(DeviceExt::Maintenance1) &&
                                  HasDep(DeviceExt::BindMemory2) &&
                                  HasDep(DeviceExt::GetMemoryRequirements2) &&
                                  HasDep(DeviceExt::GetPhysicalDeviceProperties2);
                break;

            case DeviceExt::DriverProperties:
            case DeviceExt::ShaderFloat16Int8:
            case DeviceExt::DepthClipEnable:
            case DeviceExt::ShaderIntegerDotProduct:
            case DeviceExt::ZeroInitializeWorkgroupMemory:
            case DeviceExt::DemoteToHelperInvocation:
            case DeviceExt::Maintenance4:
            case DeviceExt::Robustness2:
            case DeviceExt::SubgroupSizeControl:
            case DeviceExt::ShaderSubgroupExtendedTypes:
            case DeviceExt::VulkanMemoryModel:
            case DeviceExt::CooperativeMatrix:
            case DeviceExt::ShaderFloatControls:
                hasDependencies = HasDep(DeviceExt::GetPhysicalDeviceProperties2);
                break;

            case DeviceExt::ExternalMemory:
                hasDependencies = HasDep(DeviceExt::ExternalMemoryCapabilities);
                break;

            case DeviceExt::ExternalSemaphore:
                hasDependencies = HasDep(DeviceExt::ExternalSemaphoreCapabilities);
                break;

            case DeviceExt::ExternalMemoryAndroidHardwareBuffer:
                hasDependencies = HasDep(DeviceExt::ExternalMemory) &&
                                  HasDep(DeviceExt::SamplerYCbCrConversion) &&
                                  HasDep(DeviceExt::DedicatedAllocation) &&
                                  HasDep(DeviceExt::QueueFamilyForeign);
                break;

            case DeviceExt::ExternalMemoryFD:
            case DeviceExt::ExternalMemoryZirconHandle:
            case DeviceExt::ExternalMemoryHost:
            case DeviceExt::QueueFamilyForeign:
                hasDependencies = HasDep(DeviceExt::ExternalMemory);
                break;

            case DeviceExt::ExternalMemoryDmaBuf:
                hasDependencies = HasDep(DeviceExt::ExternalMemoryFD);
                break;

            case DeviceExt::ExternalSemaphoreFD:
            case DeviceExt::ExternalSemaphoreZirconHandle:
                hasDependencies = HasDep(DeviceExt::ExternalSemaphore);
                break;

            case DeviceExt::_16BitStorage:
                hasDependencies = HasDep(DeviceExt::GetPhysicalDeviceProperties2) &&
                                  HasDep(DeviceExt::StorageBufferStorageClass);
                break;

            case DeviceExt::DisplayTiming:
                hasDependencies = HasDep(DeviceExt::Swapchain);
                break;

            case DeviceExt::Spirv14:
                hasDependencies =
                    version >= VK_VERSION_1_1 && HasDep(DeviceExt::ShaderFloatControls);
                break;

            case DeviceExt::EnumCount:
                DAWN_UNREACHABLE();
        }

        trimmedSet.set(ext, hasDependencies && advertisedExts[ext]);
        visitedSet.set(ext, true);
    }

    return trimmedSet;
}

void MarkPromotedExtensions(DeviceExtSet* extensions, uint32_t version) {
    for (const DeviceExtInfo& info : sDeviceExtInfos) {
        if (info.versionPromoted <= version) {
            extensions->set(info.index, true);
        }
    }
}

// A static array for VulkanLayerInfo that can be indexed with VulkanLayers.
// GetVulkanLayerInfo checks that "index" matches the index used to access this array so an
// assert will fire if it isn't in the correct order.
static constexpr size_t kVulkanLayerCount = static_cast<size_t>(VulkanLayer::EnumCount);
static constexpr std::array<VulkanLayerInfo, kVulkanLayerCount> sVulkanLayerInfos{{
    //
    {VulkanLayer::Validation, "VK_LAYER_KHRONOS_validation"},
    {VulkanLayer::LunargVkTrace, "VK_LAYER_LUNARG_vktrace"},
    {VulkanLayer::RenderDocCapture, "VK_LAYER_RENDERDOC_Capture"},
    {VulkanLayer::FuchsiaImagePipeSwapchain, "VK_LAYER_FUCHSIA_imagepipe_swapchain"},
    //
}};

const VulkanLayerInfo& GetVulkanLayerInfo(VulkanLayer layer) {
    uint32_t index = static_cast<uint32_t>(layer);
    DAWN_ASSERT(index < sVulkanLayerInfos.size());
    DAWN_ASSERT(sVulkanLayerInfos[index].layer == layer);
    return sVulkanLayerInfos[index];
}

absl::flat_hash_map<std::string, VulkanLayer> CreateVulkanLayerNameMap() {
    absl::flat_hash_map<std::string, VulkanLayer> result;
    for (const VulkanLayerInfo& info : sVulkanLayerInfos) {
        result.emplace(info.name, info.layer);
    }
    return result;
}

}  // namespace dawn::native::vulkan
