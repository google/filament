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

#ifndef SRC_DAWN_NATIVE_ADAPTER_H_
#define SRC_DAWN_NATIVE_ADAPTER_H_

#include <string>
#include <utility>
#include <vector>

#include "dawn/common/Ref.h"
#include "dawn/common/RefCounted.h"
#include "dawn/common/WeakRefSupport.h"
#include "dawn/native/DawnNative.h"
#include "dawn/native/Device.h"
#include "dawn/native/PhysicalDevice.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native {

class DeviceBase;
class TogglesState;
struct SupportedLimits;

class AdapterBase : public RefCounted, public WeakRefSupport<AdapterBase> {
  public:
    AdapterBase(InstanceBase* instance,
                Ref<PhysicalDeviceBase> physicalDevice,
                wgpu::FeatureLevel featureLevel,
                const TogglesState& requiredAdapterToggles,
                wgpu::PowerPreference powerPreference);
    ~AdapterBase() override;

    // Gets the instance without adding a ref.
    InstanceBase* GetInstance() const;

    // WebGPU API
    InstanceBase* APIGetInstance() const;
    wgpu::Status APIGetLimits(Limits* limits) const;
    wgpu::Status APIGetInfo(AdapterInfo* info) const;
    bool APIHasFeature(wgpu::FeatureName feature) const;
    void APIGetFeatures(SupportedFeatures* features) const;
    void APIGetFeatures(wgpu::SupportedFeatures* features) const;
    Future APIRequestDevice(const DeviceDescriptor* descriptor,
                            const WGPURequestDeviceCallbackInfo& callbackInfo);
    DeviceBase* APICreateDevice(const DeviceDescriptor* descriptor = nullptr);
    wgpu::Status APIGetFormatCapabilities(wgpu::TextureFormat format,
                                          DawnFormatCapabilities* capabilities);

    // Return the limits for the adapter.
    const CombinedLimits& GetLimits() const;
    // Set if the adapter uses tiered limits, may result in a change in the adapter's limits.
    void SetUseTieredLimits(bool useTieredLimits);

    // Return the underlying PhysicalDevice.
    PhysicalDeviceBase* GetPhysicalDevice();
    const PhysicalDeviceBase* GetPhysicalDevice() const;

    // Get the actual toggles state of the adapter.
    const TogglesState& GetTogglesState() const;

    // Get the defaulting feature level of the adapter.
    wgpu::FeatureLevel GetFeatureLevel() const;

    // Get a human readable label for the adapter (in practice, the physical device name)
    const std::string& GetName() const;

  private:
    std::pair<Ref<DeviceBase::DeviceLostEvent>, ResultOrError<Ref<DeviceBase>>> CreateDevice(
        const DeviceDescriptor* rawDescriptor);
    ResultOrError<Ref<DeviceBase>> CreateDeviceInternal(const DeviceDescriptor* rawDescriptor,
                                                        Ref<DeviceBase::DeviceLostEvent> lostEvent);

    // Generate the adapter's limits based on current adapter status. Should be called during
    // AdapterBase creation and when the adapter's limits-related status changes, e.g.
    // SetUseTieredLimits.
    void UpdateLimits();

    Ref<InstanceBase> mInstance;
    Ref<PhysicalDeviceBase> mPhysicalDevice;
    wgpu::FeatureLevel mFeatureLevel;
    bool mUseTieredLimits = false;
    // Limits for the adapter, tiered if mUseTieredLimits is set.
    CombinedLimits mLimits;

    // Supported features under adapter toggles.
    FeaturesSet mSupportedFeatures;

    // Adapter toggles state.
    TogglesState mTogglesState;

    wgpu::PowerPreference mPowerPreference;

    // The adapter becomes "consumed" once it has successfully been used to
    // create a device.
    bool mAdapterIsConsumed = false;
};

std::vector<Ref<AdapterBase>> SortAdapters(std::vector<Ref<AdapterBase>> adapters,
                                           const UnpackedPtr<RequestAdapterOptions>& options);

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_ADAPTER_H_
