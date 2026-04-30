// Copyright 2025 The Dawn & Tint Authors
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

#include "dawn/native/webgpu/PhysicalDeviceWGPU.h"

#include <string>
#include <utility>

#include "dawn/common/StringViewUtils.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/Features_autogen.h"
#include "dawn/native/Instance.h"
#include "dawn/native/Toggles.h"
#include "dawn/native/webgpu/BackendWGPU.h"
#include "dawn/native/webgpu/DeviceWGPU.h"

namespace dawn::native::webgpu {

Ref<PhysicalDevice> PhysicalDevice::Create(Backend* backend, WGPUAdapter innerAdapter) {
    auto physicalDevice = AcquireRef(new PhysicalDevice(backend, innerAdapter));
    MaybeError err = physicalDevice->Initialize();
    DAWN_CHECK(err.IsSuccess());
    return physicalDevice;
}

PhysicalDevice::PhysicalDevice(Backend* backend, WGPUAdapter innerAdapter)
    : PhysicalDeviceBase(wgpu::BackendType::WebGPU),
      mBackend(backend),
      mInnerAdapter(innerAdapter) {
    WGPUAdapterInfo info = {};

    const bool supportsSubgroupSizeControl = GetFunctions().adapterHasFeature(
        mInnerAdapter, WGPUFeatureName_ChromiumExperimentalSubgroupSizeControl);

    WGPUAdapterPropertiesExplicitComputeSubgroupSizeConfigs explicitComputeSubgroupSizeConfigs = {};
    explicitComputeSubgroupSizeConfigs.chain.sType =
        WGPUSType_AdapterPropertiesExplicitComputeSubgroupSizeConfigs;
    if (supportsSubgroupSizeControl) {
        info.nextInChain = &explicitComputeSubgroupSizeConfigs.chain;
    }

    WGPUStatus status = GetFunctions().adapterGetInfo(mInnerAdapter, &info);
    DAWN_ASSERT(status == WGPUStatus_Success);
    DAWN_ASSERT(info.backendType != WGPUBackendType_WebGPU);

    mVendorName = ToString(info.vendor);
    mArchitectureName = ToString(info.architecture);
    mVendorId = info.vendorID;
    mDeviceId = info.deviceID;
    mName = absl::StrFormat("WebGPU backend on %s %s", FromAPI(info.backendType),
                            ToString(info.device));
    mAdapterType = FromAPI(info.adapterType);
    mDriverDescription = ToString(info.description);
    mSubgroupMinSize = info.subgroupMinSize;
    mSubgroupMaxSize = info.subgroupMaxSize;
    mInnerBackendType = info.backendType;

    if (supportsSubgroupSizeControl) {
        mMinExplicitComputeSubgroupSize =
            explicitComputeSubgroupSizeConfigs.minExplicitComputeSubgroupSize;
        mMaxExplicitComputeSubgroupSize =
            explicitComputeSubgroupSizeConfigs.maxExplicitComputeSubgroupSize;
        mMaxComputeWorkgroupSubgroups =
            explicitComputeSubgroupSizeConfigs.maxComputeWorkgroupSubgroups;
    }

    GetFunctions().adapterInfoFreeMembers(info);
}

PhysicalDevice::~PhysicalDevice() {
    if (mInnerAdapter) {
        GetFunctions().adapterRelease(mInnerAdapter);
        mInnerAdapter = nullptr;
    }
}

const DawnProcTable& PhysicalDevice::GetFunctions() const {
    return mBackend->GetFunctions();
}

Backend* PhysicalDevice::GetBackend() const {
    return mBackend;
}

bool PhysicalDevice::SupportsExternalImages() const {
    // TODO(crbug.com/494307326): remove the function.
    return false;
}

bool PhysicalDevice::SupportsFeatureLevel(wgpu::FeatureLevel featureLevel,
                                          InstanceBase* instance) const {
    return true;
}

ResultOrError<PhysicalDeviceSurfaceCapabilities> PhysicalDevice::GetSurfaceCapabilities(
    InstanceBase* instance,
    const Surface* surface) const {
    // TODO(crbug.com/413053623): revisit when implementing Surface.
    PhysicalDeviceSurfaceCapabilities capabilities;
    capabilities.usages = wgpu::TextureUsage::RenderAttachment;
    capabilities.formats = {wgpu::TextureFormat::BGRA8Unorm};
    capabilities.presentModes = {wgpu::PresentMode::Fifo};
    capabilities.alphaModes = {wgpu::CompositeAlphaMode::Opaque};
    return capabilities;
}

MaybeError PhysicalDevice::InitializeImpl() {
    return {};
}

void PhysicalDevice::InitializeSupportedFeaturesImpl() {
    DAWN_ASSERT(kEnumCount<Feature> == static_cast<uint32_t>(kFeatureNameAndInfoList.size()));
    for (uint32_t f = 0; f < kEnumCount<Feature>; f++) {
        Feature feature = static_cast<Feature>(f);
        WGPUFeatureName apiFeature = ToAPI(ToAPI(feature));
        if (GetFunctions().adapterHasFeature(mInnerAdapter, apiFeature)) {
            EnableFeature(feature);
        }
    }
    EnableFeature(Feature::AdapterPropertiesWGPU);
}

MaybeError PhysicalDevice::InitializeSupportedLimitsImpl(CombinedLimits* limits) {
    WGPUStatus status = GetFunctions().adapterGetLimits(mInnerAdapter, ToAPI(&limits->v1));
    if (status != WGPUStatus_Success) {
        return DAWN_INTERNAL_ERROR("Fail to get inner adapter limits");
    }
    return {};
}

void PhysicalDevice::SetupBackendAdapterToggles(dawn::platform::Platform* platform,
                                                TogglesState* adapterToggles) const {}

void PhysicalDevice::SetupBackendDeviceToggles(dawn::platform::Platform* platform,
                                               TogglesState* deviceToggles) const {
    // We should always use this toggle in order to capture the label.
    deviceToggles->ForceSet(Toggle::UseUserDefinedLabelsInBackend, true);
}

ResultOrError<Ref<DeviceBase>> PhysicalDevice::CreateDeviceImpl(
    AdapterBase* adapter,
    const UnpackedPtr<DeviceDescriptor>& descriptor,
    const TogglesState& deviceToggles,
    Ref<DeviceBase::DeviceLostEvent>&& lostEvent) {
    return Device::Create(adapter, mInnerAdapter, descriptor, deviceToggles, std::move(lostEvent));
}

void PhysicalDevice::PopulateBackendProperties(UnpackedPtr<AdapterInfo>& info,
                                               const TogglesState&) const {
    // TODO(crbug.com/413053623): Populate other AdapterInfo Chained extensions when necessary.
    if (auto* wgpuProperties = info.Get<AdapterPropertiesWGPU>()) {
        wgpuProperties->backendType = FromAPI(mInnerBackendType);
    }
    if (auto* explicitSubgroupSizeConfigs =
            info.Get<AdapterPropertiesExplicitComputeSubgroupSizeConfigs>()) {
        explicitSubgroupSizeConfigs->minExplicitComputeSubgroupSize =
            mMinExplicitComputeSubgroupSize;
        explicitSubgroupSizeConfigs->maxExplicitComputeSubgroupSize =
            mMaxExplicitComputeSubgroupSize;
        explicitSubgroupSizeConfigs->maxComputeWorkgroupSubgroups = mMaxComputeWorkgroupSubgroups;
    }
}

FeatureValidationResult PhysicalDevice::ValidateFeatureSupportedWithTogglesImpl(
    wgpu::FeatureName feature,
    const TogglesState& toggles) const {
    return {};
}

}  // namespace dawn::native::webgpu
