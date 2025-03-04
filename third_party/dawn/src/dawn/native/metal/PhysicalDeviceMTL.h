// Copyright 2024 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_METAL_PHYSICALDEVICEMTL_H_
#define SRC_DAWN_NATIVE_METAL_PHYSICALDEVICEMTL_H_

#include "dawn/native/PhysicalDevice.h"

#include "dawn/common/NSRef.h"

#import <Metal/Metal.h>

namespace dawn::native::metal {

class PhysicalDevice : public PhysicalDeviceBase {
  public:
    PhysicalDevice(InstanceBase* instance,
                   NSPRef<id<MTLDevice>> device,
                   bool metalValidationEnabled);

    bool IsMetalValidationEnabled() const;

    // PhysicalDeviceBase Implementation
    bool SupportsExternalImages() const override;
    bool SupportsFeatureLevel(wgpu::FeatureLevel, InstanceBase* instance) const override;
    ResultOrError<PhysicalDeviceSurfaceCapabilities> GetSurfaceCapabilities(
        InstanceBase* instance,
        const Surface*) const override;

  private:
    ResultOrError<Ref<DeviceBase>> CreateDeviceImpl(
        AdapterBase* adapter,
        const UnpackedPtr<DeviceDescriptor>& descriptor,
        const TogglesState& deviceToggles,
        Ref<DeviceBase::DeviceLostEvent>&& lostEvent) override;

    void SetupBackendAdapterToggles(dawn::platform::Platform* platform,
                                    TogglesState* adapterToggles) const override;
    void SetupBackendDeviceToggles(dawn::platform::Platform* platform,
                                   TogglesState* deviceToggles) const override;

    MaybeError InitializeImpl() override;

    void InitializeSupportedFeaturesImpl() override;
    void InitializeVendorArchitectureImpl() override;
    MaybeError InitializeSupportedLimitsImpl(CombinedLimits* limits) override;

    FeatureValidationResult ValidateFeatureSupportedWithTogglesImpl(
        wgpu::FeatureName feature,
        const TogglesState& toggles) const override;
    void PopulateBackendProperties(UnpackedPtr<AdapterInfo>& info) const override;

    NSPRef<id<MTLDevice>> mDevice;
    const bool mMetalValidationEnabled;
};

}  // namespace dawn::native::metal

#endif  // SRC_DAWN_NATIVE_METAL_PHYSICALDEVICEMTL_H_
