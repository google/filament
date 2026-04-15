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

// A static array for InstanceExtInfo that can be indexed with InstanceExts.
// GetInstanceExtInfo checks that "index" matches the index used to access this array so an
// assert will fire if it isn't in the correct order.
static constexpr size_t kInstanceExtCount = static_cast<size_t>(InstanceExt::EnumCount);
static constexpr std::array<InstanceExtInfo, kInstanceExtCount> sInstanceExtInfos{{
    // Not promoted to core in any version
    {InstanceExt::Surface, "VK_KHR_surface"},
    {InstanceExt::FuchsiaImagePipeSurface, "VK_FUCHSIA_imagepipe_surface"},
    {InstanceExt::MetalSurface, "VK_EXT_metal_surface"},
    {InstanceExt::WaylandSurface, "VK_KHR_wayland_surface"},
    {InstanceExt::Win32Surface, "VK_KHR_win32_surface"},
    {InstanceExt::XcbSurface, "VK_KHR_xcb_surface"},
    {InstanceExt::XlibSurface, "VK_KHR_xlib_surface"},
    {InstanceExt::AndroidSurface, "VK_KHR_android_surface"},

    {InstanceExt::DebugUtils, "VK_EXT_debug_utils"},
    {InstanceExt::ValidationFeatures, "VK_EXT_validation_features"},
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
            case InstanceExt::Surface:
            case InstanceExt::DebugUtils:
            case InstanceExt::ValidationFeatures:
                hasDependencies = true;
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

static constexpr size_t kDeviceExtCount = static_cast<size_t>(DeviceExt::EnumCount);
static constexpr std::array<DeviceExtInfo, kDeviceExtCount> sDeviceExtInfos{{
    // Promoted in 1.2
    {DeviceExt::DriverProperties, "VK_KHR_driver_properties"},
    {DeviceExt::ImageFormatList, "VK_KHR_image_format_list"},
    {DeviceExt::ShaderFloat16Int8, "VK_KHR_shader_float16_int8"},
    {DeviceExt::ShaderSubgroupExtendedTypes, "VK_KHR_shader_subgroup_extended_types"},
    {DeviceExt::ShaderBufferInt64Atomics, "VK_KHR_shader_atomic_int64"},
    {DeviceExt::DrawIndirectCount, "VK_KHR_draw_indirect_count"},
    {DeviceExt::VulkanMemoryModel, "VK_KHR_vulkan_memory_model"},
    {DeviceExt::ShaderFloatControls, "VK_KHR_shader_float_controls"},
    {DeviceExt::Spirv14, "VK_KHR_spirv_1_4"},
    {DeviceExt::DescriptorIndexing, "VK_EXT_descriptor_indexing"},
    {DeviceExt::CreateRenderPass2, "VK_KHR_create_renderpass2"},
    {DeviceExt::DepthStencilResolve, "VK_KHR_depth_stencil_resolve"},

    // Promoted in 1.3
    {DeviceExt::ShaderIntegerDotProduct, "VK_KHR_shader_integer_dot_product"},
    {DeviceExt::ZeroInitializeWorkgroupMemory, "VK_KHR_zero_initialize_workgroup_memory"},
    {DeviceExt::DemoteToHelperInvocation, "VK_EXT_shader_demote_to_helper_invocation"},
    {DeviceExt::Maintenance4, "VK_KHR_maintenance4"},
    {DeviceExt::SubgroupSizeControl, "VK_EXT_subgroup_size_control"},
    {DeviceExt::DynamicRendering, "VK_KHR_dynamic_rendering"},
    {DeviceExt::ExtendedDynamicState, "VK_EXT_extended_dynamic_state"},

    // Promoted in 1.4
    {DeviceExt::PipelineRobustness, "VK_EXT_pipeline_robustness"},
    {DeviceExt::Maintenance5, "VK_KHR_maintenance5"},

    // Not promoted to core in any version
    {DeviceExt::DepthClipEnable, "VK_EXT_depth_clip_enable"},
    {DeviceExt::ImageDrmFormatModifier, "VK_EXT_image_drm_format_modifier"},
    {DeviceExt::Swapchain, "VK_KHR_swapchain"},
    {DeviceExt::QueueFamilyForeign, "VK_EXT_queue_family_foreign"},
    {DeviceExt::Robustness2, "VK_EXT_robustness2"},
    {DeviceExt::DisplayTiming, "VK_GOOGLE_display_timing"},
    {DeviceExt::CooperativeMatrix, "VK_KHR_cooperative_matrix"},
    {DeviceExt::MultisampledRenderToSingleSampled, "VK_EXT_multisampled_render_to_single_sampled"},
    {DeviceExt::PhysicalDeviceDrm, "VK_EXT_physical_device_drm"},

    {DeviceExt::ExternalMemoryAndroidHardwareBuffer,
     "VK_ANDROID_external_memory_android_hardware_buffer"},
    {DeviceExt::ExternalMemoryFD, "VK_KHR_external_memory_fd"},
    {DeviceExt::ExternalMemoryDmaBuf, "VK_EXT_external_memory_dma_buf"},
    {DeviceExt::ExternalMemoryZirconHandle, "VK_FUCHSIA_external_memory"},
    {DeviceExt::ExternalMemoryHost, "VK_EXT_external_memory_host"},
    {DeviceExt::ExternalSemaphoreFD, "VK_KHR_external_semaphore_fd"},
    {DeviceExt::ExternalSemaphoreZirconHandle, "VK_FUCHSIA_external_semaphore"},
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

    // Dawn requires at least Vulkan 1.1
    DAWN_ASSERT(version >= VK_API_VERSION_1_1);

    auto HasDep = [&](DeviceExt ext) -> bool {
        DAWN_ASSERT(visitedSet[ext]);
        return trimmedSet[ext];
    };

    for (uint32_t i = 0; i < sDeviceExtInfos.size(); i++) {
        DeviceExt ext = static_cast<DeviceExt>(i);

        bool hasDependencies = false;
        switch (ext) {
            // Happy extensions don't need anybody else!
            case DeviceExt::ImageFormatList:
            case DeviceExt::DrawIndirectCount:
            // Extensions which only depend on Vulkan 1.1
            case DeviceExt::DriverProperties:
            case DeviceExt::ShaderFloat16Int8:
            case DeviceExt::DepthClipEnable:
            case DeviceExt::ShaderIntegerDotProduct:
            case DeviceExt::ZeroInitializeWorkgroupMemory:
            case DeviceExt::DemoteToHelperInvocation:
            case DeviceExt::Maintenance4:
            case DeviceExt::PipelineRobustness:
            case DeviceExt::Robustness2:
            case DeviceExt::SubgroupSizeControl:
            case DeviceExt::ShaderSubgroupExtendedTypes:
            case DeviceExt::ShaderBufferInt64Atomics:
            case DeviceExt::VulkanMemoryModel:
            case DeviceExt::CooperativeMatrix:
            case DeviceExt::ShaderFloatControls:
            case DeviceExt::DescriptorIndexing:
            case DeviceExt::CreateRenderPass2:
            case DeviceExt::ExternalMemoryFD:
            case DeviceExt::ExternalMemoryZirconHandle:
            case DeviceExt::ExternalMemoryHost:
            case DeviceExt::ExternalSemaphoreFD:
            case DeviceExt::ExternalSemaphoreZirconHandle:
            case DeviceExt::QueueFamilyForeign:
            case DeviceExt::PhysicalDeviceDrm:
            case DeviceExt::ExtendedDynamicState:
                hasDependencies = true;
                break;

            // Physical device extensions technically don't require the instance to support
            // them but VulkanFunctions only loads the function pointers if the instance
            // advertises the extension. So if we didn't have this check, we'd risk a calling
            // a nullptr.
            case DeviceExt::ImageDrmFormatModifier:
                hasDependencies = HasDep(DeviceExt::ImageFormatList);
                break;

            case DeviceExt::Swapchain:
                hasDependencies = instanceExts[InstanceExt::Surface];
                break;

            case DeviceExt::ExternalMemoryAndroidHardwareBuffer:
                hasDependencies = HasDep(DeviceExt::QueueFamilyForeign);
                break;

            case DeviceExt::ExternalMemoryDmaBuf:
                hasDependencies = HasDep(DeviceExt::ExternalMemoryFD);
                break;

            case DeviceExt::DepthStencilResolve:
                hasDependencies = HasDep(DeviceExt::CreateRenderPass2);
                break;

            case DeviceExt::DisplayTiming:
                hasDependencies = HasDep(DeviceExt::Swapchain);
                break;

            case DeviceExt::MultisampledRenderToSingleSampled:
                hasDependencies =
                    HasDep(DeviceExt::CreateRenderPass2) && HasDep(DeviceExt::DepthStencilResolve);
                break;

            case DeviceExt::Spirv14:
                hasDependencies = HasDep(DeviceExt::ShaderFloatControls);
                break;

            case DeviceExt::DynamicRendering:
                hasDependencies = HasDep(DeviceExt::DepthStencilResolve);
                break;

            case DeviceExt::Maintenance5:
                hasDependencies = HasDep(DeviceExt::DynamicRendering);
                break;

            case DeviceExt::EnumCount:
                DAWN_UNREACHABLE();
        }

        trimmedSet.set(ext, hasDependencies && advertisedExts[ext]);
        visitedSet.set(ext, true);
    }

    return trimmedSet;
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
