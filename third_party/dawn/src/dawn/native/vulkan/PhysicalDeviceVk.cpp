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
#include <cstdint>
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

bool VKComponentTypeToWGPUSubgroupMatrixComponentType(
    wgpu::SubgroupMatrixComponentType* wgpuComponentType,
    VkComponentTypeKHR vkComponentType) {
    DAWN_ASSERT(wgpuComponentType != nullptr);

    switch (vkComponentType) {
        case VK_COMPONENT_TYPE_FLOAT32_KHR:
            *wgpuComponentType = wgpu::SubgroupMatrixComponentType::F32;
            return true;
        case VK_COMPONENT_TYPE_FLOAT16_KHR:
            *wgpuComponentType = wgpu::SubgroupMatrixComponentType::F16;
            return true;
        case VK_COMPONENT_TYPE_UINT32_KHR:
            *wgpuComponentType = wgpu::SubgroupMatrixComponentType::U32;
            return true;
        case VK_COMPONENT_TYPE_SINT32_KHR:
            *wgpuComponentType = wgpu::SubgroupMatrixComponentType::I32;
            return true;
        case VK_COMPONENT_TYPE_UINT8_KHR:
            *wgpuComponentType = wgpu::SubgroupMatrixComponentType::U8;
            return true;
        case VK_COMPONENT_TYPE_SINT8_KHR:
            *wgpuComponentType = wgpu::SubgroupMatrixComponentType::I8;
            return true;
        default:
            return false;
    }
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
    return (properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0;
}

bool PhysicalDevice::IsTextureCompressionASTCSliced3DSupported(VkFormat format) const {
    VkImageFormatProperties properties;
    VkResult result =
        VkResult::WrapUnsafe(mVulkanInstance->GetFunctions().GetPhysicalDeviceImageFormatProperties(
            mVkPhysicalDevice, format, VK_IMAGE_TYPE_3D, VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_SAMPLED_BIT, {}, &properties));
    return (result == VK_SUCCESS);
}

MaybeError PhysicalDevice::InitializeImpl() {
    DAWN_TRY_ASSIGN(mDeviceInfo, GatherDeviceInfo(*this));

    mDriverVersion = DecodeVulkanDriverVersion(mDeviceInfo.properties.vendorID,
                                               mDeviceInfo.properties.driverVersion);
    const std::string driverVersionStr = mDriverVersion.ToString();

#if DAWN_PLATFORM_IS(WINDOWS)
    // Disable Vulkan adapter on Windows Intel driver < 30.0.101.2111 due to flaky
    // issues.
    const gpu_info::IntelWindowsDriverVersion kDriverVersion({30, 0, 101, 2111});
    if (gpu_info::IsIntel(mDeviceInfo.properties.vendorID) &&
        gpu_info::IntelWindowsDriverVersion(mDriverVersion) < kDriverVersion) {
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

    mMinExplicitComputeSubgroupSize = mDeviceInfo.subgroupSizeControlProperties.minSubgroupSize;
    mMaxExplicitComputeSubgroupSize = mDeviceInfo.subgroupSizeControlProperties.maxSubgroupSize;
    mMaxComputeWorkgroupSubgroups =
        mDeviceInfo.subgroupSizeControlProperties.maxComputeWorkgroupSubgroups;

    // Check for essential Vulkan extensions and features

    // Dawn requires at least Vulkan 1.1
    DAWN_ASSERT(mDeviceInfo.properties.apiVersion >= VK_API_VERSION_1_1);

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

    // Needed for binding_array support.
    if (!mDeviceInfo.features.shaderUniformBufferArrayDynamicIndexing ||
        !mDeviceInfo.features.shaderStorageBufferArrayDynamicIndexing ||
        !mDeviceInfo.features.shaderSampledImageArrayDynamicIndexing ||
        !mDeviceInfo.features.shaderStorageImageArrayDynamicIndexing) {
        return DAWN_INTERNAL_ERROR("Vulkan shaderUniform*ArrayDynamicIndexing required.");
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
    EnableFeature(Feature::DawnDeviceAllocatorControl);

    // Initialize supported extensions
    if (mDeviceInfo.features.textureCompressionBC == VK_TRUE) {
        EnableFeature(Feature::TextureCompressionBC);
        EnableFeature(Feature::TextureCompressionBCSliced3D);
    }

    if (mDeviceInfo.features.textureCompressionETC2 == VK_TRUE) {
        EnableFeature(Feature::TextureCompressionETC2);
    }

    if (mDeviceInfo.features.textureCompressionASTC_LDR == VK_TRUE) {
        EnableFeature(Feature::TextureCompressionASTC);

        bool textureCompressionASTCSliced3DSupported = true;
        for (const auto& astcFormat :
             {VK_FORMAT_ASTC_4x4_UNORM_BLOCK,   VK_FORMAT_ASTC_4x4_SRGB_BLOCK,
              VK_FORMAT_ASTC_5x4_UNORM_BLOCK,   VK_FORMAT_ASTC_5x4_SRGB_BLOCK,
              VK_FORMAT_ASTC_5x5_UNORM_BLOCK,   VK_FORMAT_ASTC_5x5_SRGB_BLOCK,
              VK_FORMAT_ASTC_6x5_UNORM_BLOCK,   VK_FORMAT_ASTC_6x5_SRGB_BLOCK,
              VK_FORMAT_ASTC_6x6_UNORM_BLOCK,   VK_FORMAT_ASTC_6x6_SRGB_BLOCK,
              VK_FORMAT_ASTC_8x5_UNORM_BLOCK,   VK_FORMAT_ASTC_8x5_SRGB_BLOCK,
              VK_FORMAT_ASTC_8x6_UNORM_BLOCK,   VK_FORMAT_ASTC_8x6_SRGB_BLOCK,
              VK_FORMAT_ASTC_8x8_UNORM_BLOCK,   VK_FORMAT_ASTC_8x8_SRGB_BLOCK,
              VK_FORMAT_ASTC_10x5_UNORM_BLOCK,  VK_FORMAT_ASTC_10x5_SRGB_BLOCK,
              VK_FORMAT_ASTC_10x6_UNORM_BLOCK,  VK_FORMAT_ASTC_10x6_SRGB_BLOCK,
              VK_FORMAT_ASTC_10x8_UNORM_BLOCK,  VK_FORMAT_ASTC_10x8_SRGB_BLOCK,
              VK_FORMAT_ASTC_10x10_UNORM_BLOCK, VK_FORMAT_ASTC_10x10_SRGB_BLOCK,
              VK_FORMAT_ASTC_12x10_UNORM_BLOCK, VK_FORMAT_ASTC_12x10_SRGB_BLOCK,
              VK_FORMAT_ASTC_12x12_UNORM_BLOCK, VK_FORMAT_ASTC_12x12_SRGB_BLOCK}) {
            textureCompressionASTCSliced3DSupported &=
                IsTextureCompressionASTCSliced3DSupported(astcFormat);
        }
        if (textureCompressionASTCSliced3DSupported) {
            EnableFeature(Feature::TextureCompressionASTCSliced3D);
        }
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

    if (mDeviceInfo.features.shaderClipDistance == VK_TRUE) {
        EnableFeature(Feature::ClipDistances);
    }

    // primitive_index is currently exposed if we support geometry shaders.
    // TODO(342172182): We could also potentially use tessellation or mesh shaders.
    if (mDeviceInfo.features.geometryShader == VK_TRUE) {
        EnableFeature(Feature::PrimitiveIndex);
    }

    bool shaderF16Enabled = false;
    if (mDeviceInfo.HasExt(DeviceExt::ShaderFloat16Int8) &&
        mDeviceInfo.shaderFloat16Int8Features.shaderFloat16 == VK_TRUE &&
        mDeviceInfo._16BitStorageFeatures.storageBuffer16BitAccess == VK_TRUE) {
        EnableFeature(Feature::ShaderF16);
        shaderF16Enabled = true;
    }

    if (mDeviceInfo.HasExt(DeviceExt::DrawIndirectCount) &&
        mDeviceInfo.features.multiDrawIndirect == VK_TRUE) {
        EnableFeature(Feature::MultiDrawIndirect);
    }

    // unclippedDepth=true translates to depthClamp=true, which implicitly disables clipping.
    if (mDeviceInfo.features.depthClamp == VK_TRUE) {
        EnableFeature(Feature::DepthClipControl);
    }

    if (mDeviceInfo.HasExt(DeviceExt::MultisampledRenderToSingleSampled) &&
        mDeviceInfo.multisampledRenderToSingleSampledFeatures.multisampledRenderToSingleSampled ==
            VK_TRUE) {
        EnableFeature(Feature::MSAARenderToSingleSampled);
    }

    if (mDeviceInfo.HasExt(DeviceExt::ExternalMemoryAndroidHardwareBuffer) &&
        mDeviceInfo.samplerYCbCrConversionFeatures.samplerYcbcrConversion == VK_TRUE) {
        EnableFeature(Feature::YCbCrVulkanSamplers);
        EnableFeature(Feature::OpaqueYCbCrAndroidForExternalTexture);
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
    bool unorm16FormatsFilterabilitySupported = true;
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
        unorm16FormatsFilterabilitySupported &= IsSubset(
            static_cast<VkFormatFeatureFlags>(VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT |
                                              VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT),
            unorm16Properties.optimalTilingFeatures);
    }
    if (unorm16TextureFormatsSupported) {
        EnableFeature(Feature::Unorm16TextureFormats);
    }
    if (unorm16FormatsFilterabilitySupported) {
        EnableFeature(Feature::Unorm16Filterable);
        EnableFeature(Feature::Unorm16FormatsForExternalTexture);
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
    EnableFeature(Feature::RenderPassRenderArea);

    // Enable Subgroups feature if:
    // 1. Vulkan API version is 1.1 or later, and
    // 2. subgroupSupportedStages includes compute and fragment stage bit, and
    // 3. subgroupSupportedOperations includes vote, ballot, shuffle, shuffle relative, arithmetic,
    //    and quad bits, and
    // 4. VK_EXT_subgroup_size_control extension is valid, and subgroupSizeControl
    //    is TRUE in VkPhysicalDeviceSubgroupSizeControlFeaturesEXT. This is required
    //    to support VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT.
    const bool hasBaseSubgroupSupport =
        (mDeviceInfo.properties.apiVersion >= VK_API_VERSION_1_1) &&
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
        (mDeviceInfo.subgroupSizeControlFeatures.subgroupSizeControl == VK_TRUE);

    // If shader f16 is enabled we only enable subgroups if we extended subgroup support.
    // This means there is a vary narrow number of devices (~4%) will not get subgroup
    // support due to the fact that they support shader f16 but not actually f16
    // operations in subgroups.
    const bool hasRequiredF16Support =
        !shaderF16Enabled ||
        (mDeviceInfo.shaderSubgroupExtendedTypes.shaderSubgroupExtendedTypes == VK_TRUE);

    const bool hasAtomic64Support = mDeviceInfo.HasExt(DeviceExt::ShaderBufferInt64Atomics) &&
                                    mDeviceInfo.shaderAtomicInt64Features.shaderBufferInt64Atomics;

    if (hasAtomic64Support) {
        EnableFeature(Feature::AtomicVec2uMinMax);
    }
    // Some devices (PowerVR GE8320) can apparently report subgroup size of 1.
    const bool allowSubgroupSizeRanges =
        mSubgroupMinSize >= kDefaultSubgroupMinSize && mSubgroupMaxSize <= kDefaultSubgroupMaxSize;

    const bool supportsSubgroupsFeature =
        hasBaseSubgroupSupport && hasRequiredF16Support && allowSubgroupSizeRanges;
    if (supportsSubgroupsFeature) {
        EnableFeature(Feature::Subgroups);
    }

    // Enable subgroup matrix if all of the following are true:
    //   1. Cooperative Matrix is supported in compute shaders.
    //   2. Vulkan Memory Model is supported at device scope.
    //   3. Subgroup Size Control is supported along with the full subgroups feature bit.
    //   4. There is at least one supported matrix configuration.
    const bool hasCooperativeMatrix =
        mDeviceInfo.HasExt(DeviceExt::CooperativeMatrix) &&
        mDeviceInfo.cooperativeMatrixFeatures.cooperativeMatrix == VK_TRUE &&
        (mDeviceInfo.cooperativeMatrixProperties.cooperativeMatrixSupportedStages &
         VK_SHADER_STAGE_COMPUTE_BIT) != 0;
    const bool hasVulkanMemoryModel =
        mDeviceInfo.HasExt(DeviceExt::VulkanMemoryModel) &&
        mDeviceInfo.vulkanMemoryModelFeatures.vulkanMemoryModel == VK_TRUE &&
        mDeviceInfo.vulkanMemoryModelFeatures.vulkanMemoryModelDeviceScope == VK_TRUE;
    const bool hasComputeFullSubgroups =
        mDeviceInfo.HasExt(DeviceExt::SubgroupSizeControl) &&
        (mDeviceInfo.subgroupSizeControlFeatures.subgroupSizeControl == VK_TRUE) &&
        (mDeviceInfo.subgroupSizeControlFeatures.computeFullSubgroups == VK_TRUE);
    if (hasCooperativeMatrix && hasVulkanMemoryModel && hasComputeFullSubgroups) {
        // crbug.com/415828149: Older Mesa drivers have bugs around subgroup matrix initialization,
        // so we blocklist SubgroupMatrix for Mesa drivers older than 25.2.
        const gpu_info::DriverVersion kGoodMesaDriver = {25, 2, 0, 0};
        const bool badDriver = IsIntelMesa() && GetDriverVersion() < kGoodMesaDriver;
        if (!badDriver) {
            EnableFeature(Feature::ChromiumExperimentalSubgroupMatrix);
        }
    }

    if (supportsSubgroupsFeature && hasComputeFullSubgroups) {
        EnableFeature(Feature::ChromiumExperimentalSubgroupSizeControl);
    }

    // HostMappedPointer is currently disabled on AMD due to a driver bug: crbug.com/494566064
    if (!gpu_info::IsAMD(GetVendorId()) && mDeviceInfo.HasExt(DeviceExt::ExternalMemoryHost) &&
        mDeviceInfo.externalMemoryHostProperties.minImportedHostPointerAlignment <=
            kMinimumHostMappedPointerAlignment) {
        EnableFeature(Feature::HostMappedPointer);
    }

    if (mDeviceInfo.HasExt(DeviceExt::ExternalMemoryDmaBuf) &&
        mDeviceInfo.HasExt(DeviceExt::ImageDrmFormatModifier)) {
        EnableFeature(Feature::SharedTextureMemoryDmaBuf);
    }
    if (mDeviceInfo.HasExt(DeviceExt::ExternalMemoryFD)) {
        EnableFeature(Feature::SharedTextureMemoryOpaqueFD);
    }

    if (mDeviceInfo.HasExt(DeviceExt::PhysicalDeviceDrm)) {
        EnableFeature(Feature::AdapterPropertiesDrm);
    }

    // Using mappable buffers on NVIDIA was found to be significantly slower in some tests. The
    // memory heap size is also small (~250MB) on many devices which leads to OOMs when allocating
    // large mappable buffers, see https://crbug.com/432044227 for details.
    if (!gpu_info::IsNvidia(GetVendorId()) && SupportsBufferMapExtendedUsages(mDeviceInfo)) {
        EnableFeature(Feature::BufferMapExtendedUsages);
    }

    if (mDeviceInfo.features.shaderStorageImageExtendedFormats == VK_TRUE) {
        bool supportTier1AndTier2 = true;

        for (const auto& format :
             {VK_FORMAT_R16_UNORM, VK_FORMAT_R16_SNORM, VK_FORMAT_R16G16_UNORM,
              VK_FORMAT_R16G16_SNORM, VK_FORMAT_R16G16B16A16_UNORM, VK_FORMAT_R16G16B16A16_SNORM,
              VK_FORMAT_R8_SNORM, VK_FORMAT_R8G8_SNORM, VK_FORMAT_R8G8B8A8_SNORM,
              VK_FORMAT_B10G11R11_UFLOAT_PACK32}) {
            VkFormatProperties properties;
            mVulkanInstance->GetFunctions().GetPhysicalDeviceFormatProperties(mVkPhysicalDevice,
                                                                              format, &properties);
            if (!IsSubset(
                    static_cast<VkFormatFeatureFlags>(VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT |
                                                      VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT),
                    properties.optimalTilingFeatures)) {
                supportTier1AndTier2 = false;
                break;
            }
        }

        // We cannot enable TextureFormatsTier2 if TextureFormatsTier1 is not enabled.
        // TextureFormatsTier2 only requires shaderStorageImageExtendedFormats from Vulkan
        // so we don't have to check anything else.
        if (supportTier1AndTier2) {
            EnableFeature(Feature::TextureFormatsTier1);
            EnableFeature(Feature::TextureFormatsTier2);
        }
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

    EnableFeature(Feature::TextureComponentSwizzle);

    // Enable support for bindless if WebGPU semantics are supported:
    //  - We can update descriptor sets while pending.
    //  - Descriptor set may have variable size and be partially bound.
    //  - We can declare runtime-sized arrays of resources in SPIRV.
    //  - Limits are large enough for WebGPU.
    // Bindless support for WebGPU requires checking a LOT of things, use runtimeDescriptorArray as
    // good proxy to whether bindless will be available, and do the proper check using separate
    // boolean variables instead of a giant if-statement. runtimeDescriptorArray checks if we can
    // declare runtime-sized arrays of resources in SPIRV.
    if (mDeviceInfo.HasExt(DeviceExt::DescriptorIndexing) &&
        mDeviceInfo.descriptorIndexingFeatures.runtimeDescriptorArray) {
        const auto& vkFeatures = mDeviceInfo.descriptorIndexingFeatures;
        const auto& vkProperties = mDeviceInfo.descriptorIndexingProperties;

        // All shader resource types support update-after-bind, except uniform buffers and input
        // attachments that are not supported in WebGPU"s bindless.
        bool hasUpdateAfterBind = vkFeatures.descriptorBindingSampledImageUpdateAfterBind &&
                                  vkFeatures.descriptorBindingStorageImageUpdateAfterBind &&
                                  vkFeatures.descriptorBindingStorageBufferUpdateAfterBind &&
                                  vkFeatures.descriptorBindingStorageTexelBufferUpdateAfterBind;

        // We can modify descriptor sets after binding them, they can have variable size and be
        // sparse.
        bool hasOtherFeatures = vkFeatures.descriptorBindingUpdateUnusedWhilePending &&
                                vkFeatures.descriptorBindingPartiallyBound &&
                                vkFeatures.descriptorBindingVariableDescriptorCount;

        // We need to check all the limits for numbers of bindings related to bindless.
        constexpr uint32_t kRequiredLimit = kMaxResourceTableSize + kReservedResourceTableSlots;
        bool hasLimit =
            vkProperties.maxPerStageDescriptorUpdateAfterBindSamplers >= kRequiredLimit &&
            vkProperties.maxPerStageDescriptorUpdateAfterBindSampledImages >= kRequiredLimit &&
            vkProperties.maxPerStageUpdateAfterBindResources >= kRequiredLimit &&
            vkProperties.maxDescriptorSetUpdateAfterBindSamplers >= kRequiredLimit &&
            vkProperties.maxDescriptorSetUpdateAfterBindSampledImages >= kRequiredLimit &&
            vkProperties.maxUpdateAfterBindDescriptorsInAllPools >= kRequiredLimit;

        // Drivers may not support robust buffer access with UpdateAfterBind (presumably because
        // they only pass a GPU virtual memory address in the descriptor and not the size of the
        // binding). This is denoted with robustBufferAccessUpdateAfterBind and when this is false
        // Vulkan 1.0 robustness and the later robustness2 feature must not be used at all. While
        // Dawn can emulate most robustness, doing it for vertex inputs would be too onerous.
        //
        // Luckily, the extension VK_EXT_pipeline_robustness (promoted to 1.4, made originally for
        // ANGLE) allows enabling robustness per-pipeline and more importantly per buffer type.
        // Without robustBufferAccessUpdateAfterBind, all the buffer robustness is disallowed,
        // except vertexInputs, the one we actually care about.
        //
        // Note that we could add this requirement only when robustness is enabled, but doing so
        // would make some GPUAdapter potentially advertise bindless support but fail to create a
        // GPUDevice with it (because it cannot be supported with robustness).
        bool canSupportRobustness = vkProperties.robustBufferAccessUpdateAfterBind ||
                                    (mDeviceInfo.HasExt(DeviceExt::PipelineRobustness) &&
                                     mDeviceInfo.pipelineRobustnessFeatures.pipelineRobustness);

        if (hasUpdateAfterBind && hasOtherFeatures && hasLimit && canSupportRobustness) {
            EnableFeature(Feature::ChromiumExperimentalSamplingResourceTable);
        }

        // TODO(https://issues.chromium.org/473444515): add support for heterogeneous bindless by
        // checking both for storage buffers/images limits and some support for heterogeneous
        // bindings via VK_EXT_mutable_descriptor_type, VK_EXT_descriptor_buffer or descriptor
        // heaps.
    }
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
    GetDefaultLimits(limits, featureLevel);
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
    CHECK_AND_SET_V1_MAX_LIMIT(maxStorageBufferRange, maxStorageBufferBindingSize);
    CHECK_AND_SET_V1_MAX_LIMIT(maxColorAttachments, maxColorAttachments);

    // Round max uniform buffer size down to a multiple of 16 bytes since Tint will polyfill them as
    // array<vec4u, ...>.
    uint32_t maxUniformBufferSize = vkLimits.maxUniformBufferRange;
    maxUniformBufferSize = maxUniformBufferSize - (maxUniformBufferSize % 16);
    if (maxUniformBufferSize < baseLimits.v1.maxUniformBufferBindingSize) {
        return DAWN_INTERNAL_ERROR("Insufficient Vulkan limits for maxUniformBufferBindingSize");
    }
    limits->v1.maxUniformBufferBindingSize = maxUniformBufferSize;

    // Vulkan has no such limit; set it to 32 (the byte width of the largest format) multiplied by
    // the maximum number of attachments in order to make it a no-op.
    limits->v1.maxColorAttachmentBytesPerSample = 32 * vkLimits.maxColorAttachments;

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
    limits->compat.maxStorageTexturesInFragmentStage = limits->v1.maxStorageTexturesPerShaderStage;
    limits->compat.maxStorageBuffersInFragmentStage = limits->v1.maxStorageBuffersPerShaderStage;
    limits->compat.maxStorageTexturesInVertexStage = limits->v1.maxStorageTexturesPerShaderStage;
    limits->compat.maxStorageBuffersInVertexStage = limits->v1.maxStorageBuffersPerShaderStage;

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
    // stages". The WebGPU SPEC also requires position at pixel centers even in the case of
    // multisampling. Vulkan does not provide a reliable way of enforcing this so must potentially
    // use another vec4f slot to emulate this behavior. So on any WebGPU Vulkan backend
    // `maxVertexOutputComponents` must be no less than 72 = (16 * 4 + 8).
    if (vkLimits.maxVertexOutputComponents < baseLimits.v1.maxInterStageShaderVariables * 4 + 8 ||
        vkLimits.maxFragmentInputComponents < baseLimits.v1.maxInterStageShaderVariables * 4 + 8) {
        return DAWN_INTERNAL_ERROR("Insufficient Vulkan limits for maxInterStageShaderVariables");
    }
    // Reserve 1 for position and 1 for emulated fragment pixel center.
    auto constexpr kNumReservedVariables = 1 + 1;
    limits->v1.maxInterStageShaderVariables =
        std::min(vkLimits.maxVertexOutputComponents, vkLimits.maxFragmentInputComponents) / 4 -
        kNumReservedVariables;

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
    } else {
        limits->v1.maxBufferSize = mDeviceInfo.propertiesMaintenance3.maxMemoryAllocationSize;
    }

    // OpArrayLength returns 0 for buffers >= 2GB for some NVIDIA devices.
    // There is an open bug with NVIDIA to resolve. Likely some issue with the sign bit.
    // See: crbug.com/435684920
    if (gpu_info::IsNvidia(GetVendorId())) {
        // 2GB limits will actually be capped to 2GB - 4 by Chrome tier limits but we do so
        // explicitly here to avoid failures in Dawn native.
        const uint64_t k2GbMinus4 = 0x80000000 - 4;
        limits->v1.maxStorageBufferBindingSize = std::min(limits->v1.maxBufferSize, k2GbMinus4);
    }

    if (limits->v1.maxBufferSize < baseLimits.v1.maxBufferSize) {
        return DAWN_INTERNAL_ERROR("Insufficient Vulkan maxBufferSize limit");
    }

    if (mDeviceInfo.HasExt(DeviceExt::SubgroupSizeControl)) {
        mDefaultComputeSubgroupSize = FindDefaultComputeSubgroupSize();
        if (mDefaultComputeSubgroupSize.has_value()) {
            // According to VK_EXT_subgroup_size_control, for compute shaders we must ensure
            // computeInvocationsPerWorkgroup <= maxComputeWorkgroupSubgroups x computeSubgroupSize
            limits->v1.maxComputeInvocationsPerWorkgroup =
                std::min(limits->v1.maxComputeInvocationsPerWorkgroup,
                         mDeviceInfo.subgroupSizeControlProperties.maxComputeWorkgroupSubgroups *
                             mDefaultComputeSubgroupSize.value());
        }
    }

    // Vulkan needs to have enough push constant range size for all
    // internal and external immediate data usages.
    constexpr uint32_t kVkGuaranteedMaxPushConstantsSize = 128;  // from Vulkan spec
    constexpr uint32_t kMaxInternalConstants =
        std::max(sizeof(RenderImmediateConstants) - sizeof(UserImmediateConstants),
                 sizeof(ComputeImmediateConstants) - sizeof(UserImmediateConstants));
    static_assert(kVkGuaranteedMaxPushConstantsSize >=
                  kMaxImmediateDataBytes + kMaxInternalConstants);
    DAWN_ASSERT(vkLimits.maxPushConstantsSize >= kVkGuaranteedMaxPushConstantsSize);
    limits->v1.maxImmediateSize = kMaxImmediateDataBytes;

    if (mDeviceInfo.HasExt(DeviceExt::ExternalMemoryHost) &&
        mDeviceInfo.externalMemoryHostProperties.minImportedHostPointerAlignment <=
            kMinimumHostMappedPointerAlignment) {
        limits->hostMappedPointerLimits.hostMappedPointerAlignment =
            kMinimumHostMappedPointerAlignment;
    }

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
    adapterToggles->Default(Toggle::UseVulkanMemoryModel, true);

    adapterToggles->Default(
        Toggle::DecomposeUniformBuffers,
        platform->IsFeatureEnabled(platform::Features::kWebGPUDecomposeUniformBuffers));

    // VulkanUseDynamicRendering and VulkanUseCreateRenderPass2 are treated as Adapter toggles
    // because they affect whether or not the MSAARenderToSingleSampled feature is available.

    // Use dynamic rendering by default if the corresponding extension is available.
    // Also disable on older Intel devices and ARM Mali-G68 devices which have been observed to have
    // driver issues with the dynamic rendering path.
    if (!GetDeviceInfo().HasExt(DeviceExt::DynamicRendering) ||
        GetDeviceInfo().dynamicRenderingFeatures.dynamicRendering == VK_FALSE ||
        (gpu_info::IsIntel(GetVendorId()) &&
         gpu_info::GetIntelGen(GetVendorId(), GetDeviceId()) <= gpu_info::IntelGen::Gen9) ||
        (gpu_info::IsARM(GetVendorId()) && gpu_info::IsMaliG68(GetDeviceId()))) {
        adapterToggles->ForceSet(Toggle::VulkanUseDynamicRendering, false);
    } else {
        adapterToggles->Default(Toggle::VulkanUseDynamicRendering, true);
    }

    // Use CreateRenderPass2KHR by default if the corresponding extension is available.
    if (!GetDeviceInfo().HasExt(DeviceExt::CreateRenderPass2)) {
        adapterToggles->ForceSet(Toggle::VulkanUseCreateRenderPass2, false);
    } else {
        adapterToggles->Default(Toggle::VulkanUseCreateRenderPass2, true);
    }
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

        // chromium:411656647: Qualcomm devices have a bug where an empty render pass that has a
        // resolve target doesn't perform the resolve. To work around it, add a small amount of work
        // to the pass to force it to execute.
        deviceToggles->Default(Toggle::VulkanAddWorkToEmptyResolvePass, true);

        // chromium:407109052: Qualcomm devices have a bug where the spirv extended op NClamp
        // modifies other components of a vector when one of the components is nan.
        deviceToggles->Default(Toggle::ScalarizeMaxMinClamp, true);

        // Qualcomm's shader compiler returns an internal error when binding_array<texture*> is
        // passed by argument to functions.
        deviceToggles->Default(Toggle::VulkanDirectVariableAccessTransformHandle, true);

        // Qualcomm has compiler error only in 32 bit. Modern devices (64 bit, adreno 8xx) do not
        // exhibit this compiler error.
#if DAWN_PLATFORM_IS(32_BIT)
        deviceToggles->Default(Toggle::VulkanSampleCompareDepthCubeArrayWorkaround, true);
#endif
    }

    if (IsIntelMesa()) {
        // chromium:448873316: Non-scalar (vector) saturate from uniform fails.
        deviceToggles->Default(Toggle::SaturateAsMinMaxF16, true);
    }

    if (IsPixel10() || IsAndroidSamsung()) {
        // Pixel 10 has a bug in vkGetPipelineCacheData(), see https://crbug.com/437807243.
        // Samsung Xclipse GPUs appear to have the same problem, see https://crbug.com/487613497.
        // TODO(crbug.com/437807243): If newer driver version without bug is released then we can
        // gate this on driver version.
        deviceToggles->Default(Toggle::VulkanIncompletePipelineCacheWorkaround, true);
    }

    if (gpu_info::IsImgTec(GetVendorId())) {
        // crbug.com/443906252: Polyfill for case switch with large ranges.
        deviceToggles->Default(Toggle::VulkanPolyfillSwitchWithIf, true);
    }

    // AMD Mesa front end optimizer bug for unary negation and abs.
    // Fixed in 25.3 - See crbug.com/448294721
    // See crbug.com/93692702 for variations of this bug.
    if (IsAmdMesa()) {
        const gpu_info::DriverVersion kGoodMesaDriver = {25, 3, 0, 0};
        const bool badDriver = GetDriverVersion() < kGoodMesaDriver;
        if (badDriver) {
            deviceToggles->Default(Toggle::VulkanPolyfillF32Abs, true);
            deviceToggles->Default(Toggle::VulkanPolyfillF32Negation, true);
        }
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

    if (gpu_info::IsARM(GetVendorId())) {
        // chromium:387000529: Arm devices have issues passing texture handles as parameters to
        // functions for accesses without a sampler (TextureLoad).
        deviceToggles->Default(Toggle::VulkanDirectVariableAccessTransformHandle, true);

        // Mali drivers incorrectly treat the stride operand to cooperative matrix load and store
        // instructions as matrix elements instead of a source/dest pointee elements.
        // See crbug.com/460209126
        deviceToggles->Default(Toggle::VulkanCooperativeMatrixStrideIsMatrixElements, true);
    }

    if (IsAndroidSamsung() || IsAndroidQualcomm() || IsAndroidHuawei()) {
        deviceToggles->Default(Toggle::IgnoreImportedAHardwareBufferVulkanImageSize, true);
    }

    if (IsSwiftshader()) {
        // Swiftshader doesn't handle propagating decorations for descriptors through
        // OpCompositeExtract which happens when a binding_array is indexed "by value" instead of
        // through a pointer.
        deviceToggles->Default(Toggle::VulkanDirectVariableAccessTransformHandle, true);

        // Disable use of ExtendedDynamicState on SwitfShader. It doesn't appear to be handling all
        // dynamic states properly, specifically some stencil ops.
        deviceToggles->ForceSet(Toggle::VulkanUseExtendedDynamicState, false);
    }

    if (IsIntelMesa()) {
        // Polyfill a clamp of `id` param in subgroupShuffle to follow spec limitations.
        // See crbug.com/435246627
        deviceToggles->Default(Toggle::SubgroupShuffleClamped, true);
    }

    if (IsIntelMesa() && gpu_info::IsIntelGen12LP(GetVendorId(), GetDeviceId())) {
        // dawn:1688: Intel Mesa driver has a bug about reusing the VkDeviceMemory that was
        // previously bound to a 2D VkImage. To work around that bug we have to disable the resource
        // sub-allocation for 2D textures with CopyDst or RenderAttachment usage.
        const gpu_info::DriverVersion kBuggyDriverVersion = {21, 3, 6, 0};
        if (GetDriverVersion() >= kBuggyDriverVersion) {
            deviceToggles->Default(
                Toggle::DisableSubAllocationFor2DTextureWithCopyDstOrRenderAttachment, true);
        }

        // chromium:1361662: Mesa driver has a bug clearing R8 mip-leveled textures on Intel Gen12
        // GPUs. Work around it by clearing the whole texture as soon as they are created.
        const gpu_info::DriverVersion kFixedDriverVersion = {23, 1, 0, 0};
        if (GetDriverVersion() < kFixedDriverVersion) {
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
        if (GetDriverVersion() >= kBuggyDriverVersion && GetDriverVersion() < kFixedDriverVersion) {
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
    // Pixel 10 workgroup zero init does not always work as expected (see crbug.com/479242793). We
    // have conservatively enabled this for all of imagination.
    if (!GetDeviceInfo().HasExt(DeviceExt::ZeroInitializeWorkgroupMemory) ||
        GetDeviceInfo().zeroInitializeWorkgroupMemoryFeatures.shaderZeroInitializeWorkgroupMemory ==
            VK_FALSE ||
        IsAndroidARM() || gpu_info::IsImgTec(GetVendorId())) {
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

    // By default try to skip injecting robustness checks on textures using VK_EXT_robustness2. But
    // disable that optimization when the feature is not available.
    if (!GetDeviceInfo().HasExt(DeviceExt::Robustness2) ||
        GetDeviceInfo().robustness2Features.robustImageAccess2 == VK_FALSE) {
        deviceToggles->ForceSet(Toggle::VulkanUseImageRobustAccess2, false);
    } else {
        deviceToggles->Default(Toggle::VulkanUseImageRobustAccess2, true);
    }

    // By default try to skip injecting robustness checks on buffers using VK_EXT_robustness2. But
    // disable that optimization when the feature is not available or if it conflicts with bindless
    // support (see comment in the detection of bindless support for more details).
    if (!GetDeviceInfo().HasExt(DeviceExt::Robustness2) ||
        GetDeviceInfo().robustness2Features.robustBufferAccess2 == VK_FALSE) {
        deviceToggles->ForceSet(Toggle::VulkanUseBufferRobustAccess2, false);
    } else if (GetDeviceInfo().HasExt(DeviceExt::DescriptorIndexing) &&
               !GetDeviceInfo().descriptorIndexingProperties.robustBufferAccessUpdateAfterBind) {
        // TODO(https://issues.chromium.org/435317394): Only disable the toggle if the bindless
        // extension is actually requested.
        deviceToggles->ForceSet(Toggle::VulkanUseBufferRobustAccess2, false);
    } else {
        deviceToggles->Default(Toggle::VulkanUseBufferRobustAccess2, true);
    }

    // Enable the polyfill versions of dot4I8Packed() and dot4U8Packed() when the SPIR-V capability
    // `DotProductInput4x8BitPackedKHR` is not supported.
    if (!GetDeviceInfo().HasExt(DeviceExt::ShaderIntegerDotProduct) ||
        GetDeviceInfo().shaderIntegerDotProductFeatures.shaderIntegerDotProduct == VK_FALSE) {
        deviceToggles->ForceSet(Toggle::PolyFillPacked4x8DotProduct, true);
    }

    // Enable the integer range analysis for shader robustness by default if the corresponding
    // platform feature is enabled.
    deviceToggles->Default(
        Toggle::EnableIntegerRangeAnalysisInRobustness,
        platform->IsFeatureEnabled(platform::Features::kWebGPUEnableRangeAnalysisForRobustness));

    if (GetDeviceInfo().HasExt(DeviceExt::Spirv14)) {
        deviceToggles->Default(Toggle::UseSpirv14,
                               platform->IsFeatureEnabled(platform::Features::kWebGPUUseSpirv14));
    } else {
        deviceToggles->ForceSet(Toggle::UseSpirv14, false);
    }

    // Vulkan waiting is already thread safe.
    deviceToggles->Default(Toggle::WaitIsThreadSafe, true);

    // Enable validation of generated SPIR-V by default.
    // Graphite and other native clients may turn this off.
    deviceToggles->Default(Toggle::EnableSpirvValidation, true);

    // Use ExtendedDynamicState by default if the corresponding extension is available.
    if (!GetDeviceInfo().HasExt(DeviceExt::ExtendedDynamicState) ||
        GetDeviceInfo().extendedDynamicStateFeatures.extendedDynamicState == VK_FALSE) {
        deviceToggles->ForceSet(Toggle::VulkanUseExtendedDynamicState, false);
    } else {
        deviceToggles->Default(Toggle::VulkanUseExtendedDynamicState, false);
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
    switch (feature) {
        // Subgroup matrices require the vulkan memory model to be used.
        case wgpu::FeatureName::ChromiumExperimentalSubgroupMatrix:
            if (!toggles.IsEnabled(Toggle::UseVulkanMemoryModel)) {
                return FeatureValidationResult(absl::StrFormat(
                    "Feature %s requires VulkanMemoryModel toggle on Vulkan.", feature));
            }
            break;

        // The function subgroupBroadcast(f16) fails for some edge cases on Intel Gen-9 devices.
        // See crbug.com/391680973. We disable subgroups on this device unless the user has
        // explicitly enabled the 'enable_subgroups_intel_gen9' toggle.
        case wgpu::FeatureName::Subgroups:
            if (gpu_info::IsIntelGen9(GetVendorId(), GetDeviceId()) &&
                !toggles.IsEnabled(Toggle::EnableSubgroupsIntelGen9)) {
                return FeatureValidationResult(
                    absl::StrFormat("Intel Gen-9 devices require `enable_subgroups_intel_gen9`"
                                    " to enable %s.",
                                    feature));
            }
            break;

        case wgpu::FeatureName::ShaderF16:
            if (!toggles.IsEnabled(Toggle::DecomposeUniformBuffers) &&
                mDeviceInfo._16BitStorageFeatures.uniformAndStorageBuffer16BitAccess == VK_FALSE) {
                return FeatureValidationResult(
                    absl::StrFormat("uniformAndStorageBuffer16BitAccess is required to enable %s "
                                    "if `decompose_uniform_buffers` is not used",
                                    feature));
            }
            // TODO(crbug.com/42251215): Investigate f16 CTS test failures to enable on Nvidia.
            if (gpu_info::IsNvidia(mVendorId) &&
                !toggles.IsEnabled(Toggle::VulkanEnableF16OnNvidia)) {
                return FeatureValidationResult(
                    absl::StrFormat("Feature %s is not yet supported on Nvidia GPUs", feature));
            }
            break;

        case wgpu::FeatureName::MSAARenderToSingleSampled:
            // Must be using either Dynamic Rendering or CreateRenderPass2 for this feature to be
            // available.
            if (!toggles.IsEnabled(Toggle::VulkanUseDynamicRendering) &&
                !toggles.IsEnabled(Toggle::VulkanUseCreateRenderPass2)) {
                return FeatureValidationResult(
                    absl::StrFormat("Feature %s requires either the VulkanUseDynamicRendering or "
                                    "VulkanUseCreateRenderPass2 toggle on Vulkan",
                                    feature));
            }
            break;

        // HostMappedPointer is currently disabled on AMD due to a driver bug: crbug.com/494566064
        case wgpu::FeatureName::HostMappedPointer:
            if (gpu_info::IsAMD(GetVendorId())) {
                return FeatureValidationResult(
                    absl::StrFormat("Feature %s is not yet supported on AMD GPUs", feature));
            }
            break;

        default:
            break;
    }

    return {};
}

// Android devices with Qualcomm GPUs have a myriad of known issues. (dawn:1549)
bool PhysicalDevice::IsAndroidQualcomm() const {
#if DAWN_PLATFORM_IS(ANDROID)
    return gpu_info::IsQualcommPCI(GetVendorId());
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

bool PhysicalDevice::IsAndroidImgTec() const {
#if DAWN_PLATFORM_IS(ANDROID)
    return gpu_info::IsImgTec(GetVendorId());
#else
    return false;
#endif
}

bool PhysicalDevice::IsPixel10() const {
    // Pixel 10 is the only device seen with PowerVR D-Series DXT-48-1536 so far.
    return IsAndroidImgTec() && GetDeviceId() == 0x71061212;
}

bool PhysicalDevice::IsIntelMesa() const {
    if (mDeviceInfo.HasExt(DeviceExt::DriverProperties)) {
        return mDeviceInfo.driverProperties.driverID == VK_DRIVER_ID_INTEL_OPEN_SOURCE_MESA_KHR;
    }
    return false;
}

bool PhysicalDevice::IsAmdMesa() const {
    if (mDeviceInfo.HasExt(DeviceExt::DriverProperties)) {
        return mDeviceInfo.driverProperties.driverID == VK_DRIVER_ID_MESA_RADV_KHR;
    }
    return false;
}

bool PhysicalDevice::IsSwiftshader() const {
    return gpu_info::IsGoogleSwiftshader(GetVendorId(), GetDeviceId());
}

std::optional<uint32_t> PhysicalDevice::FindDefaultComputeSubgroupSize() const {
    if (!mDeviceInfo.HasExt(DeviceExt::SubgroupSizeControl)) {
        return std::nullopt;
    }

    const VkPhysicalDeviceSubgroupSizeControlPropertiesEXT& ext =
        mDeviceInfo.subgroupSizeControlProperties;

    if (ext.minSubgroupSize == ext.maxSubgroupSize) {
        return std::nullopt;
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

std::optional<uint32_t> PhysicalDevice::GetDefaultComputeSubgroupSize() const {
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

void PhysicalDevice::PopulateBackendProperties(UnpackedPtr<AdapterInfo>& info,
                                               const TogglesState& toggles) const {
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
    if (auto* drmProperties = info.Get<AdapterPropertiesDrm>()) {
        drmProperties->hasPrimary = mDeviceInfo.drmProperties.hasPrimary;
        drmProperties->hasRender = mDeviceInfo.drmProperties.hasRender;
        drmProperties->primaryMajor = mDeviceInfo.drmProperties.primaryMajor;
        drmProperties->primaryMinor = mDeviceInfo.drmProperties.primaryMinor;
        drmProperties->renderMajor = mDeviceInfo.drmProperties.renderMajor;
        drmProperties->renderMinor = mDeviceInfo.drmProperties.renderMinor;
    }
    if (auto* subgroupMatrixConfigs = info.Get<AdapterPropertiesSubgroupMatrixConfigs>()) {
        std::vector<SubgroupMatrixConfig> supportedConfigs =
            EnumerateSubgroupMatrixConfigs(toggles);
        size_t count = supportedConfigs.size();
        SubgroupMatrixConfig* configs = new SubgroupMatrixConfig[count];
        subgroupMatrixConfigs->configs = configs;
        subgroupMatrixConfigs->configCount = supportedConfigs.size();
        memcpy(configs, supportedConfigs.data(), count * sizeof(SubgroupMatrixConfig));
    }
    if (auto* explicitComputeSubgroupSizeConfigs =
            info.Get<AdapterPropertiesExplicitComputeSubgroupSizeConfigs>()) {
        explicitComputeSubgroupSizeConfigs->minExplicitComputeSubgroupSize =
            GetMinExplicitComputeSubgroupSize();
        explicitComputeSubgroupSizeConfigs->maxExplicitComputeSubgroupSize =
            GetMaxExplicitComputeSubgroupSize();
        explicitComputeSubgroupSizeConfigs->maxComputeWorkgroupSubgroups =
            GetMaxComputeWorkgroupSubgroups();
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

std::vector<SubgroupMatrixConfig> PhysicalDevice::EnumerateSubgroupMatrixConfigs(
    const TogglesState& toggles) const {
    size_t configCount = mDeviceInfo.cooperativeMatrixConfigs.size();
    std::vector<SubgroupMatrixConfig> subgroupMatrixConfigs;
    subgroupMatrixConfigs.reserve(configCount);

    auto SupportComponentType = [&](wgpu::SubgroupMatrixComponentType componentType) {
        if (componentType == wgpu::SubgroupMatrixComponentType::F16) {
            return IsFeatureSupportedWithToggles(wgpu::FeatureName::ShaderF16, toggles);
        }
        return true;
    };

    for (uint32_t i = 0; i < configCount; i++) {
        const VkCooperativeMatrixPropertiesKHR& p = mDeviceInfo.cooperativeMatrixConfigs[i];

        // Filter out configurations that WebGPU does not support.
        if (p.AType != p.BType || p.CType != p.ResultType || p.scope != VK_SCOPE_SUBGROUP_KHR ||
            p.saturatingAccumulation) {
            continue;
        }

        SubgroupMatrixConfig config;
        config.M = p.MSize;
        config.N = p.NSize;
        config.K = p.KSize;

        // Filter out the component types that WebGPU does not support.
        if (!VKComponentTypeToWGPUSubgroupMatrixComponentType(&config.componentType, p.AType)) {
            continue;
        }
        if (!SupportComponentType(config.componentType)) {
            continue;
        }
        if (!VKComponentTypeToWGPUSubgroupMatrixComponentType(&config.resultComponentType,
                                                              p.ResultType)) {
            continue;
        }
        if (!SupportComponentType(config.resultComponentType)) {
            continue;
        }

        subgroupMatrixConfigs.push_back(config);
    }

    return subgroupMatrixConfigs;
}

void PhysicalDevice::SetCoreNotSupported(std::unique_ptr<ErrorData> error) {
    DAWN_ASSERT(mSupportsCoreFeatureLevel);
    mSupportsCoreFeatureLevel = false;
    DAWN_ASSERT(error);
    mCoreError = std::move(error);
}

}  // namespace dawn::native::vulkan
