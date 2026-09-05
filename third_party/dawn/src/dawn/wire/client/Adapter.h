// Copyright 2021 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_WIRE_CLIENT_ADAPTER_H_
#define SRC_DAWN_WIRE_CLIENT_ADAPTER_H_

#include <string>
#include <vector>

#include "dawn/wire/WireClient.h"
#include "dawn/wire/WireCmd_autogen.h"
#include "partition_alloc/pointers/raw_ptr.h"
#include "src/dawn/wire/client/LimitsAndFeatures.h"
#include "src/dawn/wire/client/ObjectBase.h"

namespace dawn::wire::client {

void APIAdapterInfoFreeMembers(WGPUAdapterInfo info);
void APIAdapterPropertiesMemoryHeapsFreeMembers(
    WGPUAdapterPropertiesMemoryHeaps memoryHeapProperties);
void APIDawnDrmFormatCapabilitiesFreeMembers(WGPUDawnDrmFormatCapabilities capabilities);
void APISupportedFeaturesFreeMembers(WGPUSupportedFeatures supportedFeatures);
void APIAdapterPropertiesSubgroupMatrixConfigsFreeMembers(
    WGPUAdapterPropertiesSubgroupMatrixConfigs subgroupMatrixConfigs);

class Adapter final : public ObjectWithEventsBase {
  public:
    using ObjectWithEventsBase::ObjectWithEventsBase;

    ObjectType GetObjectType() const override;

    void SetLimits(const Limits* limits);
    void SetFeatures(Span<const wgpu::FeatureName> features);
    void SetInfo(const AdapterInfo* info);

    Instance* APIGetInstance() const;
    wgpu::Status APIGetLimits(Limits* limits) const;
    bool APIHasFeature(wgpu::FeatureName feature) const;
    wgpu::Status APIGetInfo(AdapterInfo* info) const;
    void APIGetFeatures(SupportedFeatures* features) const;
    Future APIRequestDevice(const DeviceDescriptor* descriptor,
                            const WGPURequestDeviceCallbackInfo& callbackInfo);

    // Unimplementable. Only available in dawn_native.
    Device* APICreateDevice(const DeviceDescriptor*);
    wgpu::Status APIGetFormatCapabilities(wgpu::TextureFormat format,
                                          DawnFormatCapabilities* capabilities);

  private:
    LimitsAndFeatures mLimitsAndFeatures;
    AdapterInfo mInfo;
    std::string mVendor;
    std::string mArchitecture;
    std::string mDeviceName;
    std::string mDescription;
    std::vector<MemoryHeapInfo> mMemoryHeapInfo;

    AdapterPropertiesD3D mD3DProperties;
    AdapterPropertiesVk mVkProperties;
    std::vector<SubgroupMatrixConfig> mSubgroupMatrixConfigs;
    DawnAdapterPropertiesPowerPreference mPowerProperties;
};

}  // namespace dawn::wire::client

#endif  // SRC_DAWN_WIRE_CLIENT_ADAPTER_H_
