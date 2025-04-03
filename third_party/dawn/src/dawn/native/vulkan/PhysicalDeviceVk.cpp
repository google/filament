// Copyright 2019 The Dawn & Tint Authors
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

#include "dawn/native/vulkan/PhysicalDeviceVk.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/common/GPUInfo.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/Error.h"
#include "dawn/native/ImmediateConstantsLayout.h"
#include "dawn/native/Instance.h"
#include "dawn/native/Limits.h"
#include "dawn/native/vulkan/BackendVk.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/ResourceMemoryAllocatorVk.h"
#include "dawn/native/vulkan/SwapChainVk.h"
#include "dawn/native/vulkan/TextureVk.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/native/vulkan/VulkanError.h"
#include "dawn/platform/DawnPlatform.h"

#if DAWN_PLATFORM_IS(ANDROID)
#include "dawn/native/AHBFunctions.h"
#endif  // DAWN_PLATFORM_IS(ANDROID)

namespace dawn::native::vulkan {

namespace {

gpu_info::DriverVersion DecodeVulkanDriverVersion(uint32_t vendorID, uint32_t versionRaw) {
    gpu_info::DriverVersion driverVersion;
    switch (vendorID) {
        case gpu_info::kVendorID_Nvidia:
            driverVersion = {static_cast<uint16_t>((versionRaw >> 22) & 0x3FF),
                             static_cast<uint16_t>((versionRaw >> 14) & 0x0FF),
                             static_cast<uint16_t>((versionRaw >> 6) & 0x0FF),
                             static_cast<uint16_t>(versionRaw & 0x003F)};
            break;
        case gpu_info::kVendorID_Intel:
#if DAWN_PLATFORM_IS(WINDOWS)
            // Windows Vulkan driver releases together with D3D driver, so they share the same
            // version. But only CCC.DDDD is encoded in 32-bit driverVersion.
            driverVersion = {static_cast<uint16_t>(versionRaw >> 14),
                             static_cast<uint16_t>(versionRaw & 0x3FFF)};
            break;
#endif
        default:
            // Use Vulkan driver conversions for other vendors
            driverVersion = {static_cast<uint16_t>(versionRaw >> 22),
                             static_cast<uint16_t>((versionRaw >> 12) & 0x3FF),
                             static_cast<uint16_t>(versionRaw & 0xFFF)};
            break;
    }

    return driverVersion;
}

}  // anonymous namespace

PhysicalDevice::PhysicalDevice(VulkanInstance* vulkanInstance, VkPhysicalDevice physicalDevice)
    : PhysicalDeviceBase(wgpu::BackendType::Vulkan),
      mVkPhysicalDevice(physicalDevice),
      mVulkanInstance(vulkanInstance) {}

PhysicalDevice::~PhysicalDevice() = default;

const VulkanDeviceInfo& PhysicalDevice::GetDeviceInfo() const {
    return mDeviceInfo;
}

VkPhysicalDevice PhysicalDevice::GetVkPhysicalDevice() const {
    return mVkPhysicalDevice;
}

VulkanInstance* PhysicalDevice::GetVulkanInstance() const {
    return mVulkanInstance.Get();
}

bool PhysicalDevice::IsDepthStencilFormatSupported(VkFormat format) const {
    DAWN_ASSERT(format == VK_FORMAT_D16_UNORM_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT ||
                format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_S8_UINT);

    VkFormatProperties properties;
    mVulkanInstance->GetFunctions().GetPhysicalDeviceFormatProperties(mVkPhysicalDevice, format,
                                                                      &properties);
    return properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
}

MaybeError PhysicalDevice::InitializeImpl() {
    DAWN_TRY_ASSIGN(mDeviceInfo, GatherDeviceInfo(*this));

    mDriverVersion = DecodeVulkanDriverVersion(mDeviceInfo.properties.vendorID,
                                               mDeviceInfo.properties.driverVersion);
    const std::string driverVersionStr = mDriverVersion.ToString();

#if DAWN_PLATFORM_IS(WINDOWS)
    // Disable Vulkan adapter on Windows Intel driver < 30.0.101.2111 due to flaky
    // issues.
    const gpu_info::DriverVersion kDriverVersion({30, 0, 101, 2111});
    if (gpu_info::IsIntel(mDeviceInfo.properties.vendorID) &&
        gpu_info::CompareWindowsDriverVersion(mDeviceInfo.properties.vendorID, mDriverVersion,
                                              kDriverVersion) == -1) {
        return DAWN_FORMAT_INTERNAL_ERROR(
            "Disable Intel Vulkan adapter on Windows driver version %s. See "
            "https://crbug.com/1338622.",
            driverVersionStr);
    }
#endif

    if (mDeviceInfo.HasExt(DeviceExt::DriverProperties)) {
        mDriverDescription = mDeviceInfo.driverProperties.driverName;
        if (mDeviceInfo.driverProperties.driverInfo[0] != '\0') {
            mDriverDescription += std::string(": ") + mDeviceInfo.driverProperties.driverInfo;
        }
        // There may be no driver version in driverInfo.
        if (mDriverDescription.find(driverVersionStr) == std::string::npos) {
            mDriverDescription += std::string(" ") + driverVersionStr;
        }
    } else {
        mDriverDescription = std::string("Vulkan driver version ") + driverVersionStr;
    }

    mDeviceId = mDeviceInfo.properties.deviceID;
    mVendorId = mDeviceInfo.properties.vendorID;
    mName = mDeviceInfo.properties.deviceName;

    switch (mDeviceInfo.properties.deviceType) {
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            mAdapterType = wgpu::AdapterType::IntegratedGPU;
            break;
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            mAdapterType = wgpu::AdapterType::DiscreteGPU;
            break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            mAdapterType = wgpu::AdapterType::CPU;
            break;
        default:
            mAdapterType = wgpu::AdapterType::Unknown;
            break;
    }

    mSubgroupMinSize = mDeviceInfo.subgroupSizeControlProperties.minSubgroupSize;
    mSubgroupMaxSize = mDeviceInfo.subgroupSizeControlProperties.maxSubgroupSize;

    // Check for essential Vulkan extensions and features
    // Needed for viewport Y-flip.
    if (!mDeviceInfo.HasExt(DeviceExt::Maintenance1)) {
        return DAWN_INTERNAL_ERROR("Vulkan 1.1 or Vulkan 1.0 with KHR_Maintenance1 required.");
    }

    // Needed for separate depth/stencilReadOnly
    if (!mDeviceInfo.HasExt(DeviceExt::Maintenance2)) {
        return DAWN_INTERNAL_ERROR("Vulkan 1.1 or Vulkan 1.0 with KHR_Maintenance2 required.");
    }

    // Needed for security
    if (!mDeviceInfo.features.robustBufferAccess) {
        return DAWN_INTERNAL_ERROR("Vulkan robustBufferAccess feature required.");
    }

    if (!mDeviceInfo.features.textureCompressionBC &&
        !(mDeviceInfo.features.textureCompressionETC2 &&
          mDeviceInfo.features.textureCompressionASTC_LDR)) {
        return DAWN_INTERNAL_ERROR(
            "Vulkan textureCompressionBC feature required or both textureCompressionETC2 and "
            "textureCompressionASTC required.");
    }

    // Needed for the respective WebGPU features.
    if (mSupportsCoreFeatureLevel && !mDeviceInfo.features.depthBiasClamp) {
        SetCoreNotSupported(DAWN_INTERNAL_ERROR("Vulkan depthBiasClamp feature required."));
    }
    if (!mDeviceInfo.features.fragmentStoresAndAtomics) {
        // Technically `fragmentStoresAndAtomics` isn't needed for compat mode. It's essentially
        // always supported on Vulkan 1.1 devices so just leave it as required.
        return DAWN_INTERNAL_ERROR("Vulkan fragmentStoresAndAtomics feature required.");
    }
    if (!mDeviceInfo.features.fullDrawIndexUint32) {
        return DAWN_INTERNAL_ERROR("Vulkan fullDrawIndexUint32 feature required.");
    }
    if (mSupportsCoreFeatureLevel && !mDeviceInfo.features.imageCubeArray) {
        SetCoreNotSupported(DAWN_INTERNAL_ERROR("Vulkan imageCubeArray feature required."));
    }
    if (mSupportsCoreFeatureLevel && !mDeviceInfo.features.independentBlend) {
        SetCoreNotSupported(DAWN_INTERNAL_ERROR("Vulkan independentBlend feature required."));
    }
    if (mSupportsCoreFeatureLevel && !mDeviceInfo.features.sampleRateShading) {
        SetCoreNotSupported(DAWN_INTERNAL_ERROR("Vulkan sampleRateShading feature required."));
    }

    return {};
}

void PhysicalDevice::InitializeSupportedFeaturesImpl() {
    EnableFeature(Feature::AdapterPropertiesMemoryHeaps);
    EnableFeature(Feature::StaticSamplers);
    EnableFeature(Feature::FlexibleTextureViews);

    // Initialize supported extensions
    if (mDeviceInfo.features.textureCompressionBC == VK_TRUE) {
        EnableFeature(Feature::TextureCompressionBC);
    }

    if (mDeviceInfo.features.textureCompressionETC2 == VK_TRUE) {
        EnableFeature(Feature::TextureCompressionETC2);
    }

    if (mDeviceInfo.features.textureCompressionASTC_LDR == VK_TRUE) {
        EnableFeature(Feature::TextureCompressionASTC);
    }

    if (mDeviceInfo.properties.limits.timestampComputeAndGraphics == VK_TRUE) {
        EnableFeature(Feature::TimestampQuery);
        EnableFeature(Feature::ChromiumExperimentalTimestampQueryInsidePasses);
    }

    if (IsDepthStencilFormatSupported(VK_FORMAT_D32_SFLOAT_S8_UINT)) {
        EnableFeature(Feature::Depth32FloatStencil8);
    }

    if (mDeviceInfo.features.drawIndirectFirstInstance == VK_TRUE) {
        EnableFeature(Feature::IndirectFirstInstance);
    }

    if (mDeviceInfo.features.dualSrcBlend == VK_TRUE) {
        EnableFeature(Feature::DualSourceBlending);
    }

    if (mDeviceInfo.features.shaderStorageImageExtendedFormats == VK_TRUE) {
        EnableFeature(Feature::R8UnormStorage);
    }

    if (mDeviceInfo.features.shaderClipDistance == VK_TRUE) {
        EnableFeature(Feature::ClipDistances);
    }

    bool shaderF16Enabled = false;
    if (mDeviceInfo.HasExt(DeviceExt::ShaderFloat16Int8) &&
        mDeviceInfo.HasExt(DeviceExt::_16BitStorage) &&
        mDeviceInfo.shaderFloat16Int8Features.shaderFloat16 == VK_TRUE &&
        mDeviceInfo._16BitStorageFeatures.storageBuffer16BitAccess == VK_TRUE &&
        mDeviceInfo._16BitStorageFeatures.uniformAndStorageBuffer16BitAccess == VK_TRUE) {
        // TODO(crbug.com/tint/2164): Investigate crashes in f16 CTS tests to enable on NVIDIA.
        if (!gpu_info::IsNvidia(GetVendorId())) {
            EnableFeature(Feature::ShaderF16);
            shaderF16Enabled = true;
        }
    }

    if (mDeviceInfo.HasExt(DeviceExt::DrawIndirectCount) &&
        mDeviceInfo.features.multiDrawIndirect == VK_TRUE) {
        EnableFeature(Feature::MultiDrawIndirect);
    }

    // unclippedDepth=true translates to depthClamp=true, which implicitly disables clipping.
    if (mDeviceInfo.features.depthClamp == VK_TRUE) {
        EnableFeature(Feature::DepthClipControl);
    }

    if (mDeviceInfo.HasExt(DeviceExt::SamplerYCbCrConversion) &&
        mDeviceInfo.HasExt(DeviceExt::ExternalMemoryAndroidHardwareBuffer) &&
        mDeviceInfo.samplerYCbCrConversionFeatures.samplerYcbcrConversion == VK_TRUE) {
        EnableFeature(Feature::YCbCrVulkanSamplers);
    }

    VkFormatProperties rg11b10Properties;
    mVulkanInstance->GetFunctions().GetPhysicalDeviceFormatProperties(
        mVkPhysicalDevice, VK_FORMAT_B10G11R11_UFLOAT_PACK32, &rg11b10Properties);

    if (IsSubset(static_cast<VkFormatFeatureFlags>(VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT |
                                                   VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT),
                 rg11b10Properties.optimalTilingFeatures)) {
        EnableFeature(Feature::RG11B10UfloatRenderable);
    }

    VkFormatProperties bgra8unormProperties;
    mVulkanInstance->GetFunctions().GetPhysicalDeviceFormatProperties(
        mVkPhysicalDevice, VK_FORMAT_B8G8R8A8_UNORM, &bgra8unormProperties);
    if (bgra8unormProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) {
        EnableFeature(Feature::BGRA8UnormStorage);
    }

    bool unorm16TextureFormatsSupported = true;
    for (const auto& unorm16Format :
         {VK_FORMAT_R16_UNORM, VK_FORMAT_R16G16_UNORM, VK_FORMAT_R16G16B16A16_UNORM}) {
        VkFormatProperties unorm16Properties;
        mVulkanInstance->GetFunctions().GetPhysicalDeviceFormatProperties(
            mVkPhysicalDevice, unorm16Format, &unorm16Properties);
        unorm16TextureFormatsSupported &= IsSubset(
            static_cast<VkFormatFeatureFlags>(VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT |
                                              VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT |
                                              VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT),
            unorm16Properties.optimalTilingFeatures);
    }
    if (unorm16TextureFormatsSupported) {
        EnableFeature(Feature::Unorm16TextureFormats);
    }

    bool snorm16TextureFormatsSupported = true;
    for (const auto& snorm16Format :
         {VK_FORMAT_R16_SNORM, VK_FORMAT_R16G16_SNORM, VK_FORMAT_R16G16B16A16_SNORM}) {
        VkFormatProperties snorm16Properties;
        mVulkanInstance->GetFunctions().GetPhysicalDeviceFormatProperties(
            mVkPhysicalDevice, snorm16Format, &snorm16Properties);
        snorm16TextureFormatsSupported &= IsSubset(
            static_cast<VkFormatFeatureFlags>(VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT |
                                              VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT |
                                              VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT),
            snorm16Properties.optimalTilingFeatures);
    }
    if (snorm16TextureFormatsSupported) {
        EnableFeature(Feature::Snorm16TextureFormats);
    }

    if (unorm16TextureFormatsSupported && snorm16TextureFormatsSupported) {
        EnableFeature(Feature::Norm16TextureFormats);
    }

    // 32 bit float channel formats.
    VkFormatProperties r32Properties;
    VkFormatProperties rg32Properties;
    VkFormatProperties rgba32Properties;
    mVulkanInstance->GetFunctions().GetPhysicalDeviceFormatProperties(
        mVkPhysicalDevice, VK_FORMAT_R32_SFLOAT, &r32Properties);
    mVulkanInstance->GetFunctions().GetPhysicalDeviceFormatProperties(
        mVkPhysicalDevice, VK_FORMAT_R32G32_SFLOAT, &rg32Properties);
    mVulkanInstance->GetFunctions().GetPhysicalDeviceFormatProperties(
        mVkPhysicalDevice, VK_FORMAT_R32G32B32A32_SFLOAT, &rgba32Properties);
    if ((r32Properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) &&
        (rg32Properties.optimalTilingFeatures &
         VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) &&
        (rgba32Properties.optimalTilingFeatures &
         VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        EnableFeature(Feature::Float32Filterable);
    }
    if ((r32Properties.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT) &&
        (rg32Properties.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT) &&
        (rgba32Properties.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT)) {
        EnableFeature(Feature::Float32Blendable);
    }

    // Multiplanar formats.
    constexpr VkFormat multiplanarFormats[] = {
        VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
    };

    bool allMultiplanarFormatsSupported = true;
    for (const auto multiplanarFormat : multiplanarFormats) {
        VkFormatProperties multiplanarProps;
        mVulkanInstance->GetFunctions().GetPhysicalDeviceFormatProperties(
            mVkPhysicalDevice, multiplanarFormat, &multiplanarProps);

        if (!IsSubset(static_cast<VkFormatFeatureFlagBits>(
                          VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT |
                          VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT |
                          VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT),
                      multiplanarProps.optimalTilingFeatures)) {
            allMultiplanarFormatsSupported = false;
        }
    }

    if (allMultiplanarFormatsSupported) {
        EnableFeature(Feature::DawnMultiPlanarFormats);
        EnableFeature(Feature::MultiPlanarFormatExtendedUsages);
    }

    EnableFeature(Feature::TransientAttachments);
    EnableFeature(Feature::AdapterPropertiesVk);
    EnableFeature(Feature::DawnLoadResolveTexture);

    // The function subgroupBroadcast(f16) fails for some edge cases on intel gen-9 devices.
    // See crbug.com/391680973
    const bool kForceDisableSubgroups = gpu_info::IsIntelGen9(GetVendorId(), GetDeviceId());

    // Enable Subgroups feature if:
    // 1. Vulkan API version is 1.1 or later, and
    // 2. subgroupSupportedStages includes compute and fragment stage bit, and
    // 3. subgroupSupportedOperations includes vote, ballot, shuffle, shuffle relative, arithmetic,
    //    and quad bits, and
    // 4. VK_EXT_subgroup_size_control extension is valid, and both subgroupSizeControl
    //    and computeFullSubgroups is TRUE in VkPhysicalDeviceSubgroupSizeControlFeaturesEXT.
    if (!kForceDisableSubgroups && (mDeviceInfo.properties.apiVersion >= VK_API_VERSION_1_1) &&
        (mDeviceInfo.subgroupProperties.supportedStages & VK_SHADER_STAGE_COMPUTE_BIT) &&
        (mDeviceInfo.subgroupProperties.supportedStages & VK_SHADER_STAGE_FRAGMENT_BIT) &&
        (mDeviceInfo.subgroupProperties.supportedOperations & VK_SUBGROUP_FEATURE_BASIC_BIT) &&
        (mDeviceInfo.subgroupProperties.supportedOperations & VK_SUBGROUP_FEATURE_BALLOT_BIT) &&
        (mDeviceInfo.subgroupProperties.supportedOperations & VK_SUBGROUP_FEATURE_SHUFFLE_BIT) &&
        (mDeviceInfo.subgroupProperties.supportedOperations &
         VK_SUBGROUP_FEATURE_SHUFFLE_RELATIVE_BIT) &&
        (mDeviceInfo.subgroupProperties.supportedOperations & VK_SUBGROUP_FEATURE_ARITHMETIC_BIT) &&
        (mDeviceInfo.subgroupProperties.supportedOperations & VK_SUBGROUP_FEATURE_QUAD_BIT) &&
        (mDeviceInfo.HasExt(DeviceExt::SubgroupSizeControl)) &&
        (mDeviceInfo.subgroupSizeControlFeatures.subgroupSizeControl == VK_TRUE) &&
        (mDeviceInfo.subgroupSizeControlFeatures.computeFullSubgroups == VK_TRUE)) {
        if (shaderF16Enabled) {
            if (mDeviceInfo.shaderSubgroupExtendedTypes.shaderSubgroupExtendedTypes == VK_TRUE) {
                // Enable SubgroupsF16 feature if:
                // 1. Subgroups feature is enabled, and
                // 2. ShaderF16 feature is enabled, and
                // 3. shaderSubgroupExtendedTypes is TRUE in
                //    VkPhysicalDeviceShaderSubgroupExtendedTypesFeaturesKHR.
                // TODO(crbug.com/380244620): Remove when 'subgroups_f16' has been fully deprecated.
                EnableFeature(Feature::SubgroupsF16);
                // If shader f16 is enable we only enable subgroups if we extended subgroup support.
                // This means there is a vary narrow number of devices (~4%) will not get subgroup
                // support due to the fact that they support shader f16 but not actually f16
                // operations in subgroups.
                EnableFeature(Feature::Subgroups);
            }
        } else {
            // Subgroups without extended type support (f16).
            EnableFeature(Feature::Subgroups);
        }
    }

    // Enable subgroup matrix if both Cooperative Matrix and Vulkan Memory Model are supported.
    if (mDeviceInfo.HasExt(DeviceExt::CooperativeMatrix) &&
        mDeviceInfo.cooperativeMatrixFeatures.cooperativeMatrix &&
        mDeviceInfo.HasExt(DeviceExt::VulkanMemoryModel) &&
        mDeviceInfo.vulkanMemoryModelFeatures.vulkanMemoryModel == VK_TRUE &&
        mDeviceInfo.vulkanMemoryModelFeatures.vulkanMemoryModelDeviceScope == VK_TRUE) {
        PopulateSubgroupMatrixConfigs();

        // Enable the feature if at least one valid configuration is supported.
        if (!mSubgroupMatrixConfigs.empty()) {
            EnableFeature(Feature::ChromiumExperimentalSubgroupMatrix);
        }
    }

    if (mDeviceInfo.HasExt(DeviceExt::ExternalMemoryHost) &&
        mDeviceInfo.externalMemoryHostProperties.minImportedHostPointerAlignment <= 4096) {
        // TODO(crbug.com/dawn/2018): properly surface the limit.
        // Linux nearly always exposes 4096.
        // https://vulkan.gpuinfo.org/displayextensionproperty.php?platform=linux&extensionname=VK_EXT_external_memory_host&extensionproperty=minImportedHostPointerAlignment
        EnableFeature(Feature::HostMappedPointer);
    }

    if (mDeviceInfo.HasExt(DeviceExt::ExternalMemoryDmaBuf) &&
        mDeviceInfo.HasExt(DeviceExt::ImageDrmFormatModifier)) {
        EnableFeature(Feature::SharedTextureMemoryDmaBuf);
    }
    if (mDeviceInfo.HasExt(DeviceExt::ExternalMemoryFD)) {
        EnableFeature(Feature::SharedTextureMemoryOpaqueFD);
    }

    if (SupportsBufferMapExtendedUsages(mDeviceInfo)) {
        EnableFeature(Feature::BufferMapExtendedUsages);
    }

#if DAWN_PLATFORM_IS(ANDROID)
    if (mDeviceInfo.HasExt(DeviceExt::ExternalMemoryAndroidHardwareBuffer)) {
        if (GetOrLoadAHBFunctions()->IsValid()) {
            EnableFeature(Feature::SharedTextureMemoryAHardwareBuffer);
        }
    }
#endif  // DAWN_PLATFORM_IS(ANDROID)

    if (CheckSemaphoreSupport(DeviceExt::ExternalSemaphoreZirconHandle,
                              VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_ZIRCON_EVENT_BIT_FUCHSIA)) {
        EnableFeature(Feature::SharedFenceVkSemaphoreZirconHandle);
    }
    if (CheckSemaphoreSupport(DeviceExt::ExternalSemaphoreFD,
                              VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT_KHR)) {
        EnableFeature(Feature::SharedFenceSyncFD);
    }
    if (CheckSemaphoreSupport(DeviceExt::ExternalSemaphoreFD,
                              VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT_KHR)) {
        EnableFeature(Feature::SharedFenceVkSemaphoreOpaqueFD);
    }

    if (mDeviceInfo.HasExt(DeviceExt::ImageDrmFormatModifier)) {
        EnableFeature(Feature::DawnDrmFormatCapabilities);
    }

    EnableFeature(Feature::ChromiumExperimentalImmediateData);
}

MaybeError PhysicalDevice::InitializeSupportedLimitsImpl(CombinedLimits* limits) {
    if (mSupportsCoreFeatureLevel) {
        MaybeError result = InitializeSupportedLimitsInternal(wgpu::FeatureLevel::Core, limits);
        if (result.IsSuccess()) {
            return result;
        }

        // Log a warning why core isn't supported and retry checking lower compat limits.
        SetCoreNotSupported(result.AcquireError());
    }

    return InitializeSupportedLimitsInternal(wgpu::FeatureLevel::Compatibility, limits);
}

MaybeError PhysicalDevice::InitializeSupportedLimitsInternal(wgpu::FeatureLevel featureLevel,
                                                             CombinedLimits* limits) {
    GetDefaultLimits(&limits->v1, featureLevel);
    CombinedLimits baseLimits = *limits;

    const VkPhysicalDeviceLimits& vkLimits = mDeviceInfo.properties.limits;

#define CHECK_AND_SET_V1_LIMIT_IMPL(vulkanName, webgpuName, compareOp, msgSegment)   \
    do {                                                                             \
        if (vkLimits.vulkanName compareOp baseLimits.v1.webgpuName) {                \
            return DAWN_INTERNAL_ERROR("Insufficient Vulkan limits for " #webgpuName \
                                       "."                                           \
                                       " VkPhysicalDeviceLimits::" #vulkanName       \
                                       " must be at " msgSegment " " +               \
                                       std::to_string(baseLimits.v1.webgpuName));    \
        }                                                                            \
        limits->v1.webgpuName = vkLimits.vulkanName;                                 \
    } while (false)

#define CHECK_AND_SET_V1_MAX_LIMIT(vulkanName, webgpuName) \
    CHECK_AND_SET_V1_LIMIT_IMPL(vulkanName, webgpuName, <, "least")
#define CHECK_AND_SET_V1_MIN_LIMIT(vulkanName, webgpuName) \
    CHECK_AND_SET_V1_LIMIT_IMPL(vulkanName, webgpuName, >, "most")

    CHECK_AND_SET_V1_MAX_LIMIT(maxImageDimension1D, maxTextureDimension1D);

    CHECK_AND_SET_V1_MAX_LIMIT(maxImageDimension2D, maxTextureDimension2D);
    CHECK_AND_SET_V1_MAX_LIMIT(maxImageDimensionCube, maxTextureDimension2D);
    CHECK_AND_SET_V1_MAX_LIMIT(maxFramebufferWidth, maxTextureDimension2D);
    CHECK_AND_SET_V1_MAX_LIMIT(maxFramebufferHeight, maxTextureDimension2D);
    CHECK_AND_SET_V1_MAX_LIMIT(maxViewportDimensions[0], maxTextureDimension2D);
    CHECK_AND_SET_V1_MAX_LIMIT(maxViewportDimensions[1], maxTextureDimension2D);
    CHECK_AND_SET_V1_MAX_LIMIT(viewportBoundsRange[1], maxTextureDimension2D);
    limits->v1.maxTextureDimension2D = std::min({
        static_cast<uint32_t>(vkLimits.maxImageDimension2D),
        static_cast<uint32_t>(vkLimits.maxImageDimensionCube),
        static_cast<uint32_t>(vkLimits.maxFramebufferWidth),
        static_cast<uint32_t>(vkLimits.maxFramebufferHeight),
        static_cast<uint32_t>(vkLimits.maxViewportDimensions[0]),
        static_cast<uint32_t>(vkLimits.maxViewportDimensions[1]),
        static_cast<uint32_t>(vkLimits.viewportBoundsRange[1]),
    });

    CHECK_AND_SET_V1_MAX_LIMIT(maxImageDimension3D, maxTextureDimension3D);
    CHECK_AND_SET_V1_MAX_LIMIT(maxImageArrayLayers, maxTextureArrayLayers);
    CHECK_AND_SET_V1_MAX_LIMIT(maxBoundDescriptorSets, maxBindGroups);
    CHECK_AND_SET_V1_MAX_LIMIT(maxDescriptorSetUniformBuffersDynamic,
                               maxDynamicUniformBuffersPerPipelineLayout);
    CHECK_AND_SET_V1_MAX_LIMIT(maxDescriptorSetStorageBuffersDynamic,
                               maxDynamicStorageBuffersPerPipelineLayout);

    CHECK_AND_SET_V1_MAX_LIMIT(maxPerStageDescriptorSampledImages,
                               maxSampledTexturesPerShaderStage);
    CHECK_AND_SET_V1_MAX_LIMIT(maxPerStageDescriptorSamplers, maxSamplersPerShaderStage);
    CHECK_AND_SET_V1_MAX_LIMIT(maxPerStageDescriptorStorageBuffers,
                               maxStorageBuffersPerShaderStage);
    CHECK_AND_SET_V1_MAX_LIMIT(maxPerStageDescriptorStorageImages,
                               maxStorageTexturesPerShaderStage);
    CHECK_AND_SET_V1_MAX_LIMIT(maxPerStageDescriptorUniformBuffers,
                               maxUniformBuffersPerShaderStage);
    CHECK_AND_SET_V1_MAX_LIMIT(maxUniformBufferRange, maxUniformBufferBindingSize);
    CHECK_AND_SET_V1_MAX_LIMIT(maxStorageBufferRange, maxStorageBufferBindingSize);
    CHECK_AND_SET_V1_MAX_LIMIT(maxColorAttachments, maxColorAttachments);

    // Validate against maxFragmentCombinedOutputResources, tightening the limits when necessary.
    const uint32_t minFragmentCombinedOutputResources =
        baseLimits.v1.maxStorageBuffersPerShaderStage +
        baseLimits.v1.maxStorageTexturesPerShaderStage + baseLimits.v1.maxColorAttachments;
    const uint64_t maxFragmentCombinedOutputResources =
        limits->v1.maxStorageBuffersPerShaderStage + limits->v1.maxStorageTexturesPerShaderStage +
        limits->v1.maxColorAttachments;
    // Only re-adjust the limits when the limit makes sense w.r.t to the required WebGPU limits.
    // Otherwise, we ignore the maxFragmentCombinedOutputResources since it is known to yield
    // incorrect values on desktop drivers.
    bool readjustFragmentCombinedOutputResources =
        vkLimits.maxFragmentCombinedOutputResources > minFragmentCombinedOutputResources &&
        uint64_t(vkLimits.maxFragmentCombinedOutputResources) < maxFragmentCombinedOutputResources;
    if (readjustFragmentCombinedOutputResources) {
        // Split extra resources across the three other limits instead of using the default values
        // since it would overflow.
        uint32_t extraResources =
            vkLimits.maxFragmentCombinedOutputResources - minFragmentCombinedOutputResources;
        limits->v1.maxColorAttachments = std::min(
            baseLimits.v1.maxColorAttachments + (extraResources / 3), vkLimits.maxColorAttachments);
        extraResources -= limits->v1.maxColorAttachments - baseLimits.v1.maxColorAttachments;
        limits->v1.maxStorageTexturesPerShaderStage =
            std::min(baseLimits.v1.maxStorageTexturesPerShaderStage + (extraResources / 2),
                     vkLimits.maxPerStageDescriptorStorageImages);
        extraResources -= limits->v1.maxStorageTexturesPerShaderStage -
                          baseLimits.v1.maxStorageTexturesPerShaderStage;
        limits->v1.maxStorageBuffersPerShaderStage =
            std::min(baseLimits.v1.maxStorageBuffersPerShaderStage + extraResources,
                     vkLimits.maxPerStageDescriptorStorageBuffers);
    }
    limits->v1.maxStorageTexturesInFragmentStage = limits->v1.maxStorageTexturesPerShaderStage;
    limits->v1.maxStorageBuffersInFragmentStage = limits->v1.maxStorageBuffersPerShaderStage;
    limits->v1.maxStorageTexturesInVertexStage = limits->v1.maxStorageTexturesPerShaderStage;
    limits->v1.maxStorageBuffersInVertexStage = limits->v1.maxStorageBuffersPerShaderStage;

    CHECK_AND_SET_V1_MIN_LIMIT(minUniformBufferOffsetAlignment, minUniformBufferOffsetAlignment);
    CHECK_AND_SET_V1_MIN_LIMIT(minStorageBufferOffsetAlignment, minStorageBufferOffsetAlignment);

    CHECK_AND_SET_V1_MAX_LIMIT(maxVertexInputBindings, maxVertexBuffers);
    CHECK_AND_SET_V1_MAX_LIMIT(maxVertexInputAttributes, maxVertexAttributes);

    if (vkLimits.maxVertexInputBindingStride < baseLimits.v1.maxVertexBufferArrayStride ||
        vkLimits.maxVertexInputAttributeOffset < baseLimits.v1.maxVertexBufferArrayStride - 1) {
        return DAWN_INTERNAL_ERROR("Insufficient Vulkan limits for maxVertexBufferArrayStride");
    }
    // Note that some drivers have UINT32_MAX as maxVertexInputAttributeOffset so we do that +1 only
    // after the std::min.
    limits->v1.maxVertexBufferArrayStride =
        std::min(vkLimits.maxVertexInputBindingStride - 1, vkLimits.maxVertexInputAttributeOffset) +
        1;

    // Reserve 4 components for the SPIR-V builtin `position`. WebGPU SPEC requires the minimum
    // value of `maxInterStageShaderVariables` be 16. According to Vulkan SPEC, "the Location value
    // specifies an interface slot comprised of a 32-bit four-component vector conveyed between
    // stages". So on any WebGPU Vulkan backend `maxVertexOutputComponents` must be no less than
    // 68 = (16 * 4 + 4).
    if (vkLimits.maxVertexOutputComponents < baseLimits.v1.maxInterStageShaderVariables * 4 + 4 ||
        vkLimits.maxFragmentInputComponents < baseLimits.v1.maxInterStageShaderVariables * 4 + 4) {
        return DAWN_INTERNAL_ERROR("Insufficient Vulkan limits for maxInterStageShaderVariables");
    }
    limits->v1.maxInterStageShaderVariables =
        std::min(vkLimits.maxVertexOutputComponents, vkLimits.maxFragmentInputComponents) / 4 - 1;

    CHECK_AND_SET_V1_MAX_LIMIT(maxComputeSharedMemorySize, maxComputeWorkgroupStorageSize);
    CHECK_AND_SET_V1_MAX_LIMIT(maxComputeWorkGroupInvocations, maxComputeInvocationsPerWorkgroup);
    CHECK_AND_SET_V1_MAX_LIMIT(maxComputeWorkGroupSize[0], maxComputeWorkgroupSizeX);
    CHECK_AND_SET_V1_MAX_LIMIT(maxComputeWorkGroupSize[1], maxComputeWorkgroupSizeY);
    CHECK_AND_SET_V1_MAX_LIMIT(maxComputeWorkGroupSize[2], maxComputeWorkgroupSizeZ);

    CHECK_AND_SET_V1_MAX_LIMIT(maxComputeWorkGroupCount[0], maxComputeWorkgroupsPerDimension);
    CHECK_AND_SET_V1_MAX_LIMIT(maxComputeWorkGroupCount[1], maxComputeWorkgroupsPerDimension);
    CHECK_AND_SET_V1_MAX_LIMIT(maxComputeWorkGroupCount[2], maxComputeWorkgroupsPerDimension);
    limits->v1.maxComputeWorkgroupsPerDimension = std::min({
        vkLimits.maxComputeWorkGroupCount[0],
        vkLimits.maxComputeWorkGroupCount[1],
        vkLimits.maxComputeWorkGroupCount[2],
    });

    if (!IsSubset(VkSampleCountFlags(VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_4_BIT),
                  vkLimits.framebufferColorSampleCounts)) {
        return DAWN_INTERNAL_ERROR("Insufficient Vulkan limits for framebufferColorSampleCounts");
    }
    if (!IsSubset(VkSampleCountFlags(VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_4_BIT),
                  vkLimits.framebufferDepthSampleCounts)) {
        return DAWN_INTERNAL_ERROR("Insufficient Vulkan limits for framebufferDepthSampleCounts");
    }

    limits->v1.maxBufferSize = kAssumedMaxBufferSize;
    if (mDeviceInfo.HasExt(DeviceExt::Maintenance4)) {
        limits->v1.maxBufferSize = mDeviceInfo.propertiesMaintenance4.maxBufferSize;
    } else if (mDeviceInfo.HasExt(DeviceExt::Maintenance3)) {
        limits->v1.maxBufferSize = mDeviceInfo.propertiesMaintenance3.maxMemoryAllocationSize;
    }
    if (limits->v1.maxBufferSize < baseLimits.v1.maxBufferSize) {
        return DAWN_INTERNAL_ERROR("Insufficient Vulkan maxBufferSize limit");
    }

    if (mDeviceInfo.HasExt(DeviceExt::SubgroupSizeControl)) {
        mDefaultComputeSubgroupSize = FindDefaultComputeSubgroupSize();
        if (mDefaultComputeSubgroupSize > 0) {
            // According to VK_EXT_subgroup_size_control, for compute shaders we must ensure
            // computeInvocationsPerWorkgroup <= maxComputeWorkgroupSubgroups x computeSubgroupSize
            limits->v1.maxComputeInvocationsPerWorkgroup =
                std::min(limits->v1.maxComputeInvocationsPerWorkgroup,
                         mDeviceInfo.subgroupSizeControlProperties.maxComputeWorkgroupSubgroups *
                             mDefaultComputeSubgroupSize);
        }
    }

    // vulkan needs to have enough push constant range size for all
    // internal and external immediate data usages.
    constexpr uint32_t kMinVulkanPushConstants = 128;
    DAWN_ASSERT(vkLimits.maxPushConstantsSize >= kMinVulkanPushConstants);
    static_assert(kMinVulkanPushConstants >= sizeof(RenderImmediateConstants));
    static_assert(kMinVulkanPushConstants >= sizeof(ComputeImmediateConstants));

    return {};
}

bool PhysicalDevice::SupportsExternalImages() const {
    // Via dawn::native::vulkan::WrapVulkanImage
    return external_memory::Service::CheckSupport(mDeviceInfo) &&
           external_semaphore::Service::CheckSupport(mDeviceInfo, mVkPhysicalDevice,
                                                     mVulkanInstance->GetFunctions());
}

bool PhysicalDevice::SupportsFeatureLevel(wgpu::FeatureLevel featureLevel,
                                          InstanceBase* instance) const {
    if (featureLevel == wgpu::FeatureLevel::Core && !mSupportsCoreFeatureLevel) {
        if (mCoreError && instance) {
            // Log a warning explaining why this device doesn't support core the first time a core
            // adapter is requested.
            mCoreError->AppendContext(
                absl::StrFormat("checking core feature level support on \"%s\"", GetName()));
            instance->ConsumedErrorAndWarnOnce(std::move(mCoreError));
        }
        return false;
    }
    return true;
}

void PhysicalDevice::SetupBackendAdapterToggles(dawn::platform::Platform* platform,
                                                TogglesState* adapterToggles) const {
    // The environment can only request to use the Vulkan Memory Model when the extension is present
    // and the capabilities are available. Override the decision if they are not supported.
    if (!GetDeviceInfo().HasExt(DeviceExt::VulkanMemoryModel) ||
        GetDeviceInfo().vulkanMemoryModelFeatures.vulkanMemoryModel == VK_FALSE ||
        GetDeviceInfo().vulkanMemoryModelFeatures.vulkanMemoryModelDeviceScope == VK_FALSE) {
        adapterToggles->ForceSet(Toggle::UseVulkanMemoryModel, false);
    }
    adapterToggles->Default(
        Toggle::UseVulkanMemoryModel,
        platform->IsFeatureEnabled(platform::Features::kWebGPUUseVulkanMemoryModel));
}

void PhysicalDevice::SetupBackendDeviceToggles(dawn::platform::Platform* platform,
                                               TogglesState* deviceToggles) const {
    // TODO(crbug.com/dawn/857): tighten this workaround when this issue is fixed in both
    // Vulkan SPEC and drivers.
    deviceToggles->Default(Toggle::UseTemporaryBufferInCompressedTextureToTextureCopy, true);

    if (IsAndroidQualcomm()) {
        // dawn:1564, dawn:1897: Recording a compute pass after a render pass in the same command
        // buffer frequently causes a crash on Qualcomm GPUs. To work around that bug, split the
        // command buffer any time we are about to record a compute pass when a render pass has
        // already been recorded.
        deviceToggles->Default(Toggle::VulkanSplitCommandBufferOnComputePassAfterRenderPass, true);

        // dawn:1569: Qualcomm devices have a bug resolving into a non-zero level of an array
        // texture. Work around it by resolving into a single level texture and then copying into
        // the intended layer.
        deviceToggles->Default(Toggle::AlwaysResolveIntoZeroLevelAndLayer, true);
    }

    if (IsAndroidARM()) {
        // dawn:1550: Resolving multiple color targets in a single pass fails on ARM GPUs. To
        // work around the issue, passes that resolve to multiple color targets will instead be
        // forced to store the multisampled targets and do the resolves as separate passes injected
        // after the original one.
        deviceToggles->Default(Toggle::ResolveMultipleAttachmentInSeparatePasses, true);

        // dawn:379551588: Using the `pack4x8snorm`, `pack4x8unorm`, `unpack4x8snorm` and
        // `unpack4x8unorm` methods can have issues on ARM. To work around the issue we re-write the
        // pack/unpack calls and do the packing manually.
        deviceToggles->Default(Toggle::PolyfillPackUnpack4x8Norm, true);
    }

    if (IsAndroidSamsung() || IsAndroidQualcomm() || IsAndroidHuawei()) {
        deviceToggles->Default(Toggle::IgnoreImportedAHardwareBufferVulkanImageSize, true);
    }

    if (IsIntelMesa() && gpu_info::IsIntelGen12LP(GetVendorId(), GetDeviceId())) {
        // dawn:1688: Intel Mesa driver has a bug about reusing the VkDeviceMemory that was
        // previously bound to a 2D VkImage. To work around that bug we have to disable the resource
        // sub-allocation for 2D textures with CopyDst or RenderAttachment usage.
        const gpu_info::DriverVersion kBuggyDriverVersion = {21, 3, 6, 0};
        if (gpu_info::CompareIntelMesaDriverVersion(GetDriverVersion(), kBuggyDriverVersion) >= 0) {
            deviceToggles->Default(
                Toggle::DisableSubAllocationFor2DTextureWithCopyDstOrRenderAttachment, true);
        }

        // chromium:1361662: Mesa driver has a bug clearing R8 mip-leveled textures on Intel Gen12
        // GPUs. Work around it by clearing the whole texture as soon as they are created.
        const gpu_info::DriverVersion kFixedDriverVersion = {23, 1, 0, 0};
        if (gpu_info::CompareIntelMesaDriverVersion(GetDriverVersion(), kFixedDriverVersion) < 0) {
            deviceToggles->Default(Toggle::VulkanClearGen12TextureWithCCSAmbiguateOnCreation, true);
        }
    }

    if (IsIntelMesa() && (gpu_info::IsIntelGen12LP(GetVendorId(), GetDeviceId()) ||
                          gpu_info::IsIntelGen12HP(GetVendorId(), GetDeviceId()))) {
        // Intel Mesa driver has a bug where vkCmdCopyQueryPoolResults fails to write overlapping
        // queries to a same buffer after the buffer is accessed by a compute shader with correct
        // resource barriers, which may caused by flush and memory coherency issue on Intel Gen12
        // GPUs. Workaround for it to clear the buffer before vkCmdCopyQueryPoolResults on Mesa
        // driver version < 23.1.3.
        const gpu_info::DriverVersion kBuggyDriverVersion = {21, 2, 0, 0};
        const gpu_info::DriverVersion kFixedDriverVersion = {23, 1, 3, 0};
        if (gpu_info::CompareIntelMesaDriverVersion(GetDriverVersion(), kBuggyDriverVersion) >= 0 &&
            gpu_info::CompareIntelMesaDriverVersion(GetDriverVersion(), kFixedDriverVersion) < 0) {
            deviceToggles->Default(Toggle::ClearBufferBeforeResolveQueries, true);
        }
    }

    // The environment can request to various options for depth-stencil formats that could be
    // unavailable. Override the decision if it is not applicable.
    bool supportsD32s8 = IsDepthStencilFormatSupported(VK_FORMAT_D32_SFLOAT_S8_UINT);
    bool supportsD24s8 = IsDepthStencilFormatSupported(VK_FORMAT_D24_UNORM_S8_UINT);
    bool supportsS8 = IsDepthStencilFormatSupported(VK_FORMAT_S8_UINT);

    DAWN_ASSERT(supportsD32s8 || supportsD24s8);

    if (!supportsD24s8) {
        deviceToggles->ForceSet(Toggle::VulkanUseD32S8, true);
    }
    if (!supportsD32s8) {
        deviceToggles->ForceSet(Toggle::VulkanUseD32S8, false);
    }
    // By default try to use D32S8 for Depth24PlusStencil8
    deviceToggles->Default(Toggle::VulkanUseD32S8, true);

    if (!supportsS8) {
        deviceToggles->ForceSet(Toggle::VulkanUseS8, false);
    }
    // By default try to use S8 if available.
    deviceToggles->Default(Toggle::VulkanUseS8, true);

    // The environment can only request to use VK_KHR_zero_initialize_workgroup_memory when the
    // extension is available. Override the decision if it is not applicable or
    // zeroInitializeWorkgroupMemoryFeatures.shaderZeroInitializeWorkgroupMemory == VK_FALSE.
    // Never use the extension on Mali devices due to a known bug (see crbug.com/tint/2101).
    if (!GetDeviceInfo().HasExt(DeviceExt::ZeroInitializeWorkgroupMemory) ||
        GetDeviceInfo().zeroInitializeWorkgroupMemoryFeatures.shaderZeroInitializeWorkgroupMemory ==
            VK_FALSE ||
        IsAndroidARM()) {
        deviceToggles->ForceSet(Toggle::VulkanUseZeroInitializeWorkgroupMemoryExtension, false);
    }
    // By default try to initialize workgroup memory with OpConstantNull according to the Vulkan
    // extension VK_KHR_zero_initialize_workgroup_memory.
    deviceToggles->Default(Toggle::VulkanUseZeroInitializeWorkgroupMemoryExtension, true);

    // Spirv OpKill does not do demote to helper and has also been deprecated. Use
    // OpDemoteToHelperInvocation where the extension is available to get correct platform demote to
    // helper for "discard".
    if (!GetDeviceInfo().HasExt(DeviceExt::DemoteToHelperInvocation) ||
        GetDeviceInfo().demoteToHelperInvocationFeatures.shaderDemoteToHelperInvocation ==
            VK_FALSE) {
        deviceToggles->ForceSet(Toggle::VulkanUseDemoteToHelperInvocationExtension, false);
    }

    // By default we will use the vulkan demote to helper extension if it is available. This gives
    // us correct fragment shader discard semantics.
    deviceToggles->Default(Toggle::VulkanUseDemoteToHelperInvocationExtension, true);

    // The environment can only request to use StorageInputOutput16 when the capability is
    // available.
    if (GetDeviceInfo()._16BitStorageFeatures.storageInputOutput16 == VK_FALSE) {
        deviceToggles->ForceSet(Toggle::VulkanUseStorageInputOutput16, false);
    }
    // By default try to use the StorageInputOutput16 capability.
    deviceToggles->Default(Toggle::VulkanUseStorageInputOutput16, true);

    // Inject fragment shaders in all vertex-only pipelines.
    // TODO(crbug.com/dawn/1698): relax this requirement where the Vulkan spec allows.
    // In particular, enable rasterizer discard if the depth-stencil stage is a no-op, and skip
    // insertion of the placeholder fragment shader.
    deviceToggles->Default(Toggle::UsePlaceholderFragmentInVertexOnlyPipeline, true);

    // The environment can only request to use VK_EXT_robustness2 when the extension is available.
    // Override the decision if it is not applicable or robustImageAccess2 is false.
    if (!GetDeviceInfo().HasExt(DeviceExt::Robustness2) ||
        GetDeviceInfo().robustness2Features.robustImageAccess2 == VK_FALSE) {
        deviceToggles->ForceSet(Toggle::VulkanUseImageRobustAccess2, false);
    }
    // By default try to skip robustness transform on textures according to the Vulkan extension
    // VK_EXT_robustness2.
    deviceToggles->Default(Toggle::VulkanUseImageRobustAccess2, true);
    // The environment can only request to use VK_EXT_robustness2 when the extension is available.
    // Override the decision if it is not applicable or robustBufferAccess2 is false.
    if (!GetDeviceInfo().HasExt(DeviceExt::Robustness2) ||
        GetDeviceInfo().robustness2Features.robustBufferAccess2 == VK_FALSE) {
        deviceToggles->ForceSet(Toggle::VulkanUseBufferRobustAccess2, false);
    }
    // By default try to disable index clamping on the runtime-sized arrays on storage buffers in
    // Tint robustness transform according to the Vulkan extension VK_EXT_robustness2.
    deviceToggles->Default(Toggle::VulkanUseBufferRobustAccess2, true);

    // Enable the polyfill versions of dot4I8Packed() and dot4U8Packed() when the SPIR-V capability
    // `DotProductInput4x8BitPackedKHR` is not supported.
    if (!GetDeviceInfo().HasExt(DeviceExt::ShaderIntegerDotProduct) ||
        GetDeviceInfo().shaderIntegerDotProductFeatures.shaderIntegerDotProduct == VK_FALSE) {
        deviceToggles->ForceSet(Toggle::PolyFillPacked4x8DotProduct, true);
    }
}

ResultOrError<Ref<DeviceBase>> PhysicalDevice::CreateDeviceImpl(
    AdapterBase* adapter,
    const UnpackedPtr<DeviceDescriptor>& descriptor,
    const TogglesState& deviceToggles,
    Ref<DeviceBase::DeviceLostEvent>&& lostEvent) {
    return Device::Create(adapter, descriptor, deviceToggles, std::move(lostEvent));
}

FeatureValidationResult PhysicalDevice::ValidateFeatureSupportedWithTogglesImpl(
    wgpu::FeatureName feature,
    const TogglesState& toggles) const {
    if (feature == wgpu::FeatureName::ChromiumExperimentalSubgroupMatrix &&
        !toggles.IsEnabled(Toggle::UseVulkanMemoryModel)) {
        return FeatureValidationResult(
            absl::StrFormat("Feature %s requires VulkanMemoryModel toggle on Vulkan.", feature));
    }

    return {};
}

// Android devices with Qualcomm GPUs have a myriad of known issues. (dawn:1549)
bool PhysicalDevice::IsAndroidQualcomm() const {
#if DAWN_PLATFORM_IS(ANDROID)
    return gpu_info::IsQualcomm_PCI(GetVendorId());
#else
    return false;
#endif
}

// Android devices with ARM GPUs have known issues. (dawn:1550)
bool PhysicalDevice::IsAndroidARM() const {
#if DAWN_PLATFORM_IS(ANDROID)
    return gpu_info::IsARM(GetVendorId());
#else
    return false;
#endif
}

bool PhysicalDevice::IsAndroidSamsung() const {
#if DAWN_PLATFORM_IS(ANDROID)
    return gpu_info::IsSamsung(GetVendorId());
#else
    return false;
#endif
}

bool PhysicalDevice::IsAndroidHuawei() const {
#if DAWN_PLATFORM_IS(ANDROID)
    return gpu_info::IsHuawei(GetVendorId());
#else
    return false;
#endif
}

bool PhysicalDevice::IsIntelMesa() const {
    if (mDeviceInfo.HasExt(DeviceExt::DriverProperties)) {
        return mDeviceInfo.driverProperties.driverID == VK_DRIVER_ID_INTEL_OPEN_SOURCE_MESA_KHR;
    }
    return false;
}

uint32_t PhysicalDevice::FindDefaultComputeSubgroupSize() const {
    if (!mDeviceInfo.HasExt(DeviceExt::SubgroupSizeControl)) {
        return 0;
    }

    const VkPhysicalDeviceSubgroupSizeControlPropertiesEXT& ext =
        mDeviceInfo.subgroupSizeControlProperties;

    if (ext.minSubgroupSize == ext.maxSubgroupSize) {
        return 0;
    }

    // At the moment, only Intel devices support varying subgroup sizes and 16, which is the
    // next value after the minimum of 8, is the sweet spot according to [1]. Hence the
    // following heuristics, which may need to be adjusted in the future for other
    // architectures, or if a specific API is added to let client code select the size.
    //
    // [1] https://bugs.freedesktop.org/show_bug.cgi?id=108875
    uint32_t subgroupSize = ext.minSubgroupSize * 2;
    if (subgroupSize <= ext.maxSubgroupSize) {
        return subgroupSize;
    } else {
        return ext.minSubgroupSize;
    }
}

bool PhysicalDevice::CheckSemaphoreSupport(DeviceExt deviceExt,
                                           VkExternalSemaphoreHandleTypeFlagBits handleType) const {
    if (!mDeviceInfo.HasExt(deviceExt)) {
        return false;
    }

    constexpr VkFlags kRequiredSemaphoreFlags = VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT_KHR |
                                                VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT_KHR;

    VkPhysicalDeviceExternalSemaphoreInfoKHR semaphoreInfo;
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_INFO_KHR;
    semaphoreInfo.pNext = nullptr;

    VkExternalSemaphorePropertiesKHR semaphoreProperties;
    semaphoreProperties.sType = VK_STRUCTURE_TYPE_EXTERNAL_SEMAPHORE_PROPERTIES_KHR;
    semaphoreProperties.pNext = nullptr;

    semaphoreInfo.handleType = handleType;
    mVulkanInstance->GetFunctions().GetPhysicalDeviceExternalSemaphoreProperties(
        mVkPhysicalDevice, &semaphoreInfo, &semaphoreProperties);

    return IsSubset(kRequiredSemaphoreFlags, semaphoreProperties.externalSemaphoreFeatures);
}

uint32_t PhysicalDevice::GetDefaultComputeSubgroupSize() const {
    return mDefaultComputeSubgroupSize;
}

ResultOrError<PhysicalDeviceSurfaceCapabilities> PhysicalDevice::GetSurfaceCapabilities(
    InstanceBase* instance,
    const Surface* surface) const {
    // Gather the Vulkan surface capabilities.
    VulkanSurfaceInfo vkCaps;
    {
        const VulkanFunctions& fn = GetVulkanInstance()->GetFunctions();
        VkInstance vkInstance = GetVulkanInstance()->GetVkInstance();

        VkSurfaceKHR vkSurface;
        DAWN_TRY_ASSIGN(vkSurface, CreateVulkanSurface(instance, this, surface));
        DAWN_TRY_ASSIGN_WITH_CLEANUP(vkCaps, GatherSurfaceInfo(*this, vkSurface),
                                     { fn.DestroySurfaceKHR(vkInstance, vkSurface, nullptr); });

        fn.DestroySurfaceKHR(vkInstance, vkSurface, nullptr);
    }

    PhysicalDeviceSurfaceCapabilities capabilities;

    // Convert the known swapchain usages.
    capabilities.usages = wgpu::TextureUsage::None;
    if (vkCaps.capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
        capabilities.usages |= wgpu::TextureUsage::CopySrc;
    }
    if (vkCaps.capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
        capabilities.usages |= wgpu::TextureUsage::CopyDst;
    }
    if (vkCaps.capabilities.supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
        capabilities.usages |= wgpu::TextureUsage::RenderAttachment;
    }
    if (vkCaps.capabilities.supportedUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT) {
        capabilities.usages |= wgpu::TextureUsage::TextureBinding;
    }
    if (vkCaps.capabilities.supportedUsageFlags & VK_IMAGE_USAGE_STORAGE_BIT) {
        capabilities.usages |= wgpu::TextureUsage::StorageBinding;
    }

    // Convert known swapchain formats
    auto ToWGPUSwapChainFormat = [](VkFormat format) -> wgpu::TextureFormat {
        switch (format) {
            case VK_FORMAT_R8G8B8A8_UNORM:
                return wgpu::TextureFormat::RGBA8Unorm;
            case VK_FORMAT_R8G8B8A8_SRGB:
                return wgpu::TextureFormat::RGBA8UnormSrgb;
            case VK_FORMAT_B8G8R8A8_UNORM:
                return wgpu::TextureFormat::BGRA8Unorm;
            case VK_FORMAT_B8G8R8A8_SRGB:
                return wgpu::TextureFormat::BGRA8UnormSrgb;
            case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
                return wgpu::TextureFormat::RGB10A2Unorm;
            case VK_FORMAT_R16G16B16A16_SFLOAT:
                return wgpu::TextureFormat::RGBA16Float;
            default:
                return wgpu::TextureFormat::Undefined;
        }
    };
    for (VkSurfaceFormatKHR surfaceFormat : vkCaps.formats) {
        wgpu::TextureFormat format = ToWGPUSwapChainFormat(surfaceFormat.format);
        if (format != wgpu::TextureFormat::Undefined) {
            capabilities.formats.push_back(format);
        }
    }

    // Convert known present modes
    auto ToWGPUPresentMode = [](VkPresentModeKHR mode) -> std::optional<wgpu::PresentMode> {
        switch (mode) {
            case VK_PRESENT_MODE_FIFO_KHR:
                return wgpu::PresentMode::Fifo;
            case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
                return wgpu::PresentMode::FifoRelaxed;
            case VK_PRESENT_MODE_MAILBOX_KHR:
                return wgpu::PresentMode::Mailbox;
            case VK_PRESENT_MODE_IMMEDIATE_KHR:
                return wgpu::PresentMode::Immediate;
            default:
                return {};
        }
    };
    for (VkPresentModeKHR vkMode : vkCaps.presentModes) {
        std::optional<wgpu::PresentMode> wgpuMode = ToWGPUPresentMode(vkMode);
        if (wgpuMode) {
            capabilities.presentModes.push_back(*wgpuMode);
        }
    }

    // Compute supported alpha modes
    struct AlphaModePairs {
        VkCompositeAlphaFlagBitsKHR vkBit;
        wgpu::CompositeAlphaMode webgpuEnum;
    };
    AlphaModePairs alphaModePairs[] = {
        {VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, wgpu::CompositeAlphaMode::Opaque},
        {VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR, wgpu::CompositeAlphaMode::Premultiplied},
        {VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR, wgpu::CompositeAlphaMode::Unpremultiplied},
        {VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR, wgpu::CompositeAlphaMode::Inherit},
    };

    for (auto mode : alphaModePairs) {
        if (vkCaps.capabilities.supportedCompositeAlpha & mode.vkBit) {
            capabilities.alphaModes.push_back(mode.webgpuEnum);
        }
    }

    return capabilities;
}

const AHBFunctions* PhysicalDevice::GetOrLoadAHBFunctions() {
#if DAWN_PLATFORM_IS(ANDROID)
    if (mAHBFunctions == nullptr) {
        mAHBFunctions = std::make_unique<AHBFunctions>();
    }
    return mAHBFunctions.get();
#else
    DAWN_UNREACHABLE();
#endif  // DAWN_PLATFORM_IS(ANDROID)
}

void PhysicalDevice::PopulateBackendProperties(UnpackedPtr<AdapterInfo>& info) const {
    if (auto* subgroupProperties = info.Get<AdapterPropertiesSubgroups>()) {
        // Subgroups are supported only if subgroup size control is supported.
        subgroupProperties->subgroupMinSize =
            mDeviceInfo.subgroupSizeControlProperties.minSubgroupSize;
        subgroupProperties->subgroupMaxSize =
            mDeviceInfo.subgroupSizeControlProperties.maxSubgroupSize;
    }
    if (auto* memoryHeapProperties = info.Get<AdapterPropertiesMemoryHeaps>()) {
        size_t count = mDeviceInfo.memoryHeaps.size();
        auto* heapInfo = new MemoryHeapInfo[count];
        memoryHeapProperties->heapCount = count;
        memoryHeapProperties->heapInfo = heapInfo;

        for (size_t i = 0; i < count; ++i) {
            heapInfo[i].size = mDeviceInfo.memoryHeaps[i].size;
            heapInfo[i].properties = {};
            if (mDeviceInfo.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                heapInfo[i].properties |= wgpu::HeapProperty::DeviceLocal;
            }
        }
        for (const auto& memoryType : mDeviceInfo.memoryTypes) {
            if (memoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
                heapInfo[memoryType.heapIndex].properties |= wgpu::HeapProperty::HostVisible;
            }
            if (memoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
                heapInfo[memoryType.heapIndex].properties |= wgpu::HeapProperty::HostCoherent;
            }
            if (memoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) {
                heapInfo[memoryType.heapIndex].properties |= wgpu::HeapProperty::HostCached;
            } else {
                heapInfo[memoryType.heapIndex].properties |= wgpu::HeapProperty::HostUncached;
            }
        }
    }
    if (auto* vkProperties = info.Get<AdapterPropertiesVk>()) {
        vkProperties->driverVersion = mDeviceInfo.properties.driverVersion;
    }
    if (auto* subgroupMatrixConfigs = info.Get<AdapterPropertiesSubgroupMatrixConfigs>()) {
        size_t count = mSubgroupMatrixConfigs.size();
        SubgroupMatrixConfig* configs = new SubgroupMatrixConfig[count];
        subgroupMatrixConfigs->configs = configs;
        subgroupMatrixConfigs->configCount = count;
        memcpy(configs, mSubgroupMatrixConfigs.data(), count * sizeof(SubgroupMatrixConfig));
    }
}

void PhysicalDevice::PopulateBackendFormatCapabilities(
    wgpu::TextureFormat format,
    UnpackedPtr<DawnFormatCapabilities>& capabilities) const {
    if (auto* drmCapabilities = capabilities.Get<DawnDrmFormatCapabilities>()) {
        auto vk_format = ColorVulkanImageFormat(format);
        if (vk_format == VK_FORMAT_UNDEFINED) {
            drmCapabilities->properties = nullptr;
            drmCapabilities->propertiesCount = 0;
        }
        auto drmFormatModifiers =
            GetFormatModifierProps(mVulkanInstance->GetFunctions(), mVkPhysicalDevice, vk_format);
        if (!drmFormatModifiers.empty()) {
            size_t count = drmFormatModifiers.size();
            auto* properties = new DawnDrmFormatProperties[count];
            drmCapabilities->properties = properties;
            drmCapabilities->propertiesCount = count;

            for (size_t i = 0; i < count; i++) {
                properties[i].modifier = drmFormatModifiers[i].drmFormatModifier;
                properties[i].modifierPlaneCount =
                    drmFormatModifiers[i].drmFormatModifierPlaneCount;
            }
        }
    }
}

void PhysicalDevice::PopulateSubgroupMatrixConfigs() {
    size_t configCount = mDeviceInfo.cooperativeMatrixProperties.size();
    mSubgroupMatrixConfigs.reserve(configCount);
    for (uint32_t i = 0; i < configCount; i++) {
        const VkCooperativeMatrixPropertiesKHR& p = mDeviceInfo.cooperativeMatrixProperties[i];

        // Filter out configurations that WebGPU does not support.
        if (p.AType != p.BType || p.CType != p.ResultType || p.scope != VK_SCOPE_SUBGROUP_KHR ||
            p.saturatingAccumulation) {
            continue;
        }

        SubgroupMatrixConfig config;
        config.M = p.MSize;
        config.N = p.NSize;
        config.K = p.KSize;
        switch (p.AType) {
            case VK_COMPONENT_TYPE_FLOAT32_KHR:
                config.componentType = wgpu::SubgroupMatrixComponentType::F32;
                break;
            case VK_COMPONENT_TYPE_FLOAT16_KHR:
                config.componentType = wgpu::SubgroupMatrixComponentType::F16;
                break;
            case VK_COMPONENT_TYPE_UINT32_KHR:
                config.componentType = wgpu::SubgroupMatrixComponentType::U32;
                break;
            case VK_COMPONENT_TYPE_SINT32_KHR:
                config.componentType = wgpu::SubgroupMatrixComponentType::I32;
                break;
            default:
                continue;
        }
        switch (p.ResultType) {
            case VK_COMPONENT_TYPE_FLOAT32_KHR:
                config.resultComponentType = wgpu::SubgroupMatrixComponentType::F32;
                break;
            case VK_COMPONENT_TYPE_FLOAT16_KHR:
                config.resultComponentType = wgpu::SubgroupMatrixComponentType::F16;
                break;
            case VK_COMPONENT_TYPE_UINT32_KHR:
                config.resultComponentType = wgpu::SubgroupMatrixComponentType::U32;
                break;
            case VK_COMPONENT_TYPE_SINT32_KHR:
                config.resultComponentType = wgpu::SubgroupMatrixComponentType::I32;
                break;
            default:
                continue;
        }

        mSubgroupMatrixConfigs.push_back(config);
    }
}

void PhysicalDevice::SetCoreNotSupported(std::unique_ptr<ErrorData> error) {
    DAWN_ASSERT(mSupportsCoreFeatureLevel);
    mSupportsCoreFeatureLevel = false;
    DAWN_ASSERT(error);
    mCoreError = std::move(error);
}

}  // namespace dawn::native::vulkan
