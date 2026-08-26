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

#include "src/dawn/native/webgpu/PhysicalDeviceWGPU.h"

#include <string>
#include <utility>

#include "dawn/native/Features_autogen.h"
#include "src/dawn/common/StringViewUtils.h"
#include "src/dawn/native/ChainUtils.h"
#include "src/dawn/native/Instance.h"
#include "src/dawn/native/Surface.h"
#include "src/dawn/native/Toggles.h"
#include "src/dawn/native/webgpu/BackendWGPU.h"
#include "src/dawn/native/webgpu/DeviceWGPU.h"
#include "src/dawn/native/webgpu/SwapChainWGPU.h"
#include "src/utils/compiler.h"

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
    // Create a inner surface to query the surface capabilities.
    WGPUSurface innerSurface;
    DAWN_TRY_ASSIGN(innerSurface, CreateWGPUSurface(mBackend->GetFunctions(),
                                                    mBackend->GetInnerInstance(), surface));
    DAWN_ASSERT(innerSurface);

    WGPUSurfaceCapabilities innerCapabilities = WGPU_SURFACE_CAPABILITIES_INIT;
    WGPUStatus status = mBackend->GetFunctions().surfaceGetCapabilities(innerSurface, mInnerAdapter,
                                                                        &innerCapabilities);

    if (status != WGPUStatus_Success) {
        mBackend->GetFunctions().surfaceRelease(innerSurface);
        return DAWN_VALIDATION_ERROR("Failed to get inner surface capabilities");
    }

    Span<const WGPUTextureFormat> innerSurfaceFormats =
        // SAFETY: The WGPU implementation guarantees that each `things` points to at least
        // `thingCount` valid entries.
        DAWN_UNSAFE_BUFFERS({innerCapabilities.formats, innerCapabilities.formatCount});
    Span<const WGPUPresentMode> innerSurfacePresentModes =
        // SAFETY: The WGPU implementation guarantees that each `things` points to at least
        // `thingCount` valid entries.
        DAWN_UNSAFE_BUFFERS({innerCapabilities.presentModes, innerCapabilities.presentModeCount});
    Span<const WGPUCompositeAlphaMode> innerSurfaceAlphaModes =
        // SAFETY: The WGPU implementation guarantees that each `things` points to at least
        // `thingCount` valid entries.
        DAWN_UNSAFE_BUFFERS({innerCapabilities.alphaModes, innerCapabilities.alphaModeCount});

    PhysicalDeviceSurfaceCapabilities capabilities;
    capabilities.usages = static_cast<wgpu::TextureUsage>(innerCapabilities.usages);
    for (WGPUTextureFormat format : innerSurfaceFormats) {
        capabilities.formats.push_back(FromAPI(format));
    }
    for (WGPUPresentMode mode : innerSurfacePresentModes) {
        capabilities.presentModes.push_back(FromAPI(mode));
    }
    for (WGPUCompositeAlphaMode mode : innerSurfaceAlphaModes) {
        capabilities.alphaModes.push_back(FromAPI(mode));
    }

    mBackend->GetFunctions().surfaceCapabilitiesFreeMembers(innerCapabilities);
    mBackend->GetFunctions().surfaceRelease(innerSurface);

    return capabilities;
}

MaybeError PhysicalDevice::InitializeImpl() {
    return {};
}

void PhysicalDevice::InitializeSupportedFeaturesImpl() {
    DAWN_ASSERT(kEnumCount<Feature> == static_cast<uint32_t>(kFeatureNameAndInfoList.size()));
    for (uint32_t f = 0; f < kEnumCount<Feature>; f++) {
        Feature feature = static_cast<Feature>(f);
        WGPUFeatureName apiFeature = ToAPI(ToCppAPI(feature));
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
}

FeatureValidationResult PhysicalDevice::ValidateFeatureSupportedWithTogglesImpl(
    wgpu::FeatureName feature,
    const TogglesState& toggles) const {
    return {};
}

}  // namespace dawn::native::webgpu
