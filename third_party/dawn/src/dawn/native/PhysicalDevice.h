// Copyright 2018 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_PHYSICALDEVICE_H_
#define SRC_DAWN_NATIVE_PHYSICALDEVICE_H_

#include <string>
#include <vector>

#include "dawn/native/DawnNative.h"

#include "dawn/common/GPUInfo.h"
#include "dawn/common/Ref.h"
#include "dawn/common/RefCounted.h"
#include "dawn/common/ityp_span.h"
#include "dawn/native/Device.h"
#include "dawn/native/Error.h"
#include "dawn/native/Features.h"
#include "dawn/native/Forward.h"
#include "dawn/native/Limits.h"
#include "dawn/native/Toggles.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native {

class DeviceBase;

// Structure that holds surface capabilities for a (Surface, PhysicalDevice) pair.
struct PhysicalDeviceSurfaceCapabilities {
    wgpu::TextureUsage usages;
    std::vector<wgpu::TextureFormat> formats;
    std::vector<wgpu::PresentMode> presentModes;
    std::vector<wgpu::CompositeAlphaMode> alphaModes;
};

struct FeatureValidationResult {
    // Constructor of successful result
    FeatureValidationResult();
    // Constructor of failed result
    explicit FeatureValidationResult(std::string errorMsg);

    bool success;
    std::string errorMessage;
};

class PhysicalDeviceBase : public RefCounted {
  public:
    explicit PhysicalDeviceBase(wgpu::BackendType backend);
    ~PhysicalDeviceBase() override;

    MaybeError Initialize();

    ResultOrError<Ref<DeviceBase>> CreateDevice(AdapterBase* adapter,
                                                const UnpackedPtr<DeviceDescriptor>& descriptor,
                                                const TogglesState& deviceToggles,
                                                Ref<DeviceBase::DeviceLostEvent>&& lostEvent);

    uint32_t GetVendorId() const;
    uint32_t GetDeviceId() const;
    const std::string& GetVendorName() const;
    const std::string& GetArchitectureName() const;
    const std::string& GetName() const;
    const gpu_info::DriverVersion& GetDriverVersion() const;
    const std::string& GetDriverDescription() const;
    wgpu::AdapterType GetAdapterType() const;
    wgpu::BackendType GetBackendType() const;
    uint32_t GetSubgroupMinSize() const;
    uint32_t GetSubgroupMaxSize() const;

    MaybeError ResetInternalDeviceForTesting();

    // Get all features supported by the physical device and suitable with given toggles.
    FeaturesSet GetSupportedFeatures(const TogglesState& toggles) const;
    // Check if all given features are supported by the physical device and suitable with given
    // toggles.
    bool SupportsAllRequiredFeatures(const ityp::span<size_t, const wgpu::FeatureName>& features,
                                     const TogglesState& toggles) const;

    const CombinedLimits& GetLimits() const;

    virtual bool SupportsExternalImages() const = 0;

    // `instance` is an optional parameter used to log warnings but may be null.
    virtual bool SupportsFeatureLevel(wgpu::FeatureLevel featureLevel,
                                      InstanceBase* instance) const = 0;

    // Backend-specific force-setting and defaulting device toggles
    virtual void SetupBackendAdapterToggles(dawn::platform::Platform* platform,
                                            TogglesState* adapterToggles) const = 0;
    // Backend-specific force-setting and defaulting device toggles
    virtual void SetupBackendDeviceToggles(dawn::platform::Platform* platform,
                                           TogglesState* deviceToggles) const = 0;

    // Check if a feature os supported by this adapter AND suitable with given toggles.
    FeatureValidationResult ValidateFeatureSupportedWithToggles(wgpu::FeatureName feature,
                                                                const TogglesState& toggles) const;

    // Populate backend properties. Ownership of allocations written are owned by the caller.
    virtual void PopulateBackendProperties(UnpackedPtr<AdapterInfo>& info) const = 0;

    // Populate backend format capabilities. Ownership of allocations written are owned by the
    // caller.
    virtual void PopulateBackendFormatCapabilities(
        wgpu::TextureFormat format,
        UnpackedPtr<DawnFormatCapabilities>& capabilities) const;

    virtual ResultOrError<PhysicalDeviceSurfaceCapabilities> GetSurfaceCapabilities(
        InstanceBase* instance,
        const Surface* surface) const = 0;

  protected:
    uint32_t mVendorId = 0xFFFFFFFF;
    std::string mVendorName;
    std::string mArchitectureName;
    uint32_t mDeviceId = 0xFFFFFFFF;
    std::string mName;
    wgpu::AdapterType mAdapterType = wgpu::AdapterType::Unknown;
    gpu_info::DriverVersion mDriverVersion;
    std::string mDriverDescription;
    // Subgroup min/max size set to 0 by default, which means the subgroup feature is not supported.
    // The actual value will be set by the backend implementation if the feature is supported.
    uint32_t mSubgroupMinSize = 0;
    uint32_t mSubgroupMaxSize = 0;

    // Juat a wrapper of ValidateFeatureSupportedWithToggles, return true if a feature is supported
    // by this adapter AND suitable with given toggles.
    bool IsFeatureSupportedWithToggles(wgpu::FeatureName feature,
                                       const TogglesState& toggles) const;
    // Mark a feature as enabled in mSupportedFeatures.
    void EnableFeature(Feature feature);
    // Used for the tests that intend to use an adapter without all features enabled.
    void SetSupportedFeaturesForTesting(const std::vector<wgpu::FeatureName>& requiredFeatures);

    void GetDefaultLimitsForSupportedFeatureLevel(Limits* limits) const;

  private:
    virtual ResultOrError<Ref<DeviceBase>> CreateDeviceImpl(
        AdapterBase* adapter,
        const UnpackedPtr<DeviceDescriptor>& descriptor,
        const TogglesState& deviceToggles,
        Ref<DeviceBase::DeviceLostEvent>&& lostEvent) = 0;

    virtual MaybeError InitializeImpl() = 0;

    // Check base WebGPU features and discover supported features.
    virtual void InitializeSupportedFeaturesImpl() = 0;

    // Check base WebGPU limits and populate supported limits.
    virtual MaybeError InitializeSupportedLimitsImpl(CombinedLimits* limits) = 0;

    virtual void InitializeVendorArchitectureImpl();

    virtual FeatureValidationResult ValidateFeatureSupportedWithTogglesImpl(
        wgpu::FeatureName feature,
        const TogglesState& toggles) const = 0;

    virtual MaybeError ResetInternalDeviceForTestingImpl();
    wgpu::BackendType mBackend;

    // Features set that CAN be supported by devices of this adapter. Some features in this set may
    // be guarded by toggles, and creating a device with these features required may result in a
    // validation error if proper toggles are not enabled/disabled.
    FeaturesSet mSupportedFeatures;

    CombinedLimits mLimits;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_PHYSICALDEVICE_H_
