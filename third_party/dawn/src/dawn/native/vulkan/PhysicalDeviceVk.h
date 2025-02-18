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

#ifndef SRC_DAWN_NATIVE_VULKAN_ADAPTERVK_H_
#define SRC_DAWN_NATIVE_VULKAN_ADAPTERVK_H_

#include <memory>

#include "dawn/common/Ref.h"
#include "dawn/common/vulkan_platform.h"
#include "dawn/native/PhysicalDevice.h"
#include "dawn/native/vulkan/VulkanInfo.h"

namespace dawn::native {
class AHBFunctions;
}  // namespace dawn::native

namespace dawn::native::vulkan {

class VulkanInstance;

class PhysicalDevice : public PhysicalDeviceBase {
  public:
    PhysicalDevice(VulkanInstance* vulkanInstance, VkPhysicalDevice physicalDevice);
    ~PhysicalDevice() override;

    // PhysicalDeviceBase Implementation
    bool SupportsExternalImages() const override;
    bool SupportsFeatureLevel(wgpu::FeatureLevel featureLevel,
                              InstanceBase* instance) const override;

    const VulkanDeviceInfo& GetDeviceInfo() const;
    VkPhysicalDevice GetVkPhysicalDevice() const;
    VulkanInstance* GetVulkanInstance() const;

    bool IsDepthStencilFormatSupported(VkFormat format) const;

    bool IsAndroidQualcomm() const;
    bool IsAndroidARM() const;
    bool IsAndroidSamsung() const;
    bool IsIntelMesa() const;
    bool IsAndroidHuawei() const;

    uint32_t GetDefaultComputeSubgroupSize() const;

    ResultOrError<PhysicalDeviceSurfaceCapabilities> GetSurfaceCapabilities(
        InstanceBase* instance,
        const Surface* surface) const override;
    const AHBFunctions* GetOrLoadAHBFunctions();

  private:
    MaybeError InitializeImpl() override;
    void InitializeSupportedFeaturesImpl() override;
    MaybeError InitializeSupportedLimitsImpl(CombinedLimits* limits) override;

    MaybeError InitializeSupportedLimitsInternal(wgpu::FeatureLevel featureLevel,
                                                 CombinedLimits* limits);

    FeatureValidationResult ValidateFeatureSupportedWithTogglesImpl(
        wgpu::FeatureName feature,
        const TogglesState& toggles) const override;

    void SetupBackendAdapterToggles(dawn::platform::Platform* platform,
                                    TogglesState* adapterToggles) const override;
    void SetupBackendDeviceToggles(dawn::platform::Platform* platform,
                                   TogglesState* deviceToggles) const override;
    ResultOrError<Ref<DeviceBase>> CreateDeviceImpl(
        AdapterBase* adapter,
        const UnpackedPtr<DeviceDescriptor>& descriptor,
        const TogglesState& deviceToggles,
        Ref<DeviceBase::DeviceLostEvent>&& lostEvent) override;

    uint32_t FindDefaultComputeSubgroupSize() const;
    bool CheckSemaphoreSupport(DeviceExt deviceExt,
                               VkExternalSemaphoreHandleTypeFlagBits handleType) const;

    void PopulateBackendProperties(UnpackedPtr<AdapterInfo>& info) const override;
    void PopulateBackendFormatCapabilities(
        wgpu::TextureFormat format,
        UnpackedPtr<DawnFormatCapabilities>& capabilities) const override;

    // Sets core feature level as not being supported and stores `error` with
    // reason why core isn't supported.
    void SetCoreNotSupported(std::unique_ptr<ErrorData> error);

    VkPhysicalDevice mVkPhysicalDevice;
    Ref<VulkanInstance> mVulkanInstance;
    VulkanDeviceInfo mDeviceInfo = {};

    uint32_t mDefaultComputeSubgroupSize = 0;
    bool mSupportsCoreFeatureLevel = true;
    mutable std::unique_ptr<ErrorData> mCoreError;

#if DAWN_PLATFORM_IS(ANDROID)
    std::unique_ptr<AHBFunctions> mAHBFunctions;
#endif  // DAWN_PLATFORM_IS(ANDROID)
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_ADAPTERVK_H_
