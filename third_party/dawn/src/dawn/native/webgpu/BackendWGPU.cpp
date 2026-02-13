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

#include "dawn/native/webgpu/BackendWGPU.h"

#include "dawn/dawn_proc_table.h"
#include "dawn/native/Instance.h"
#include "dawn/native/webgpu/PhysicalDeviceWGPU.h"

namespace dawn::native::webgpu {

Backend::Backend(InstanceBase* instance, wgpu::BackendType backendType)
    : BackendConnection(instance, backendType) {
    mDawnProcs = dawn::native::GetProcs();

    // Enable all instance features and limits.
    WGPUInstanceDescriptor instanceDesc = {};

    WGPUSupportedInstanceFeatures features{};
    mDawnProcs.getInstanceFeatures(&features);
    instanceDesc.requiredFeatureCount = features.featureCount;
    instanceDesc.requiredFeatures = features.features;

    WGPUInstanceLimits limits{};
    mDawnProcs.getInstanceLimits(&limits);
    instanceDesc.requiredLimits = &limits;

    mInnerInstance = mDawnProcs.createInstance(&instanceDesc);

    mDawnProcs.supportedInstanceFeaturesFreeMembers(features);
}

Backend::~Backend() {
    if (mInnerInstance) {
        mDawnProcs.instanceRelease(mInnerInstance);
        mInnerInstance = nullptr;
    }
}

std::vector<Ref<PhysicalDeviceBase>> Backend::DiscoverPhysicalDevices(
    const UnpackedPtr<RequestAdapterOptions>& options) {
    // TODO(crbug.com/413053623): Expose Instance::EnumerateAdapters in DawnProcTable
    // So that we can discover multiple WebGPU physical devices.

    // Pass through all of the core options. Note if backendType=WebGPU,
    // RequestAdapter will return null since it's not enabled by default.
    WGPURequestAdapterOptions innerAdapterOption = *ToAPI(*options);
    // Don't pass through any of the extension options.
    // TODO(crbug.com/413053623): revisit to see if chaining any other extensions of
    // RequestAdapterOptions is needed.
    innerAdapterOption.nextInChain = nullptr;

    WGPUAdapter innerAdapter = nullptr;

    WGPURequestAdapterCallbackInfo callbackInfo = {};
    callbackInfo.mode = WGPUCallbackMode_WaitAnyOnly;
    callbackInfo.callback = [](WGPURequestAdapterStatus status, WGPUAdapter adapter,
                               WGPUStringView message, void* userdata1, void* userdata2) {
        if (status == WGPURequestAdapterStatus_Success) {
            *static_cast<WGPUAdapter*>(userdata2) = adapter;
        }
    };
    callbackInfo.userdata1 = nullptr;
    callbackInfo.userdata2 = static_cast<void*>(&innerAdapter);

    WGPUFutureWaitInfo waitInfo = {};
    waitInfo.future =
        mDawnProcs.instanceRequestAdapter(mInnerInstance, &innerAdapterOption, callbackInfo);
    mDawnProcs.instanceWaitAny(mInnerInstance, 1, &waitInfo, UINT64_MAX);

    if (innerAdapter == nullptr) {
        return {};
    }

    return {PhysicalDevice::Create(this, innerAdapter)};
}

const DawnProcTable& Backend::GetFunctions() const {
    return mDawnProcs;
}

WGPUInstance Backend::GetInnerInstance() const {
    return mInnerInstance;
}

BackendConnection* Connect(InstanceBase* instance) {
    return new Backend(instance, wgpu::BackendType::WebGPU);
}

}  // namespace dawn::native::webgpu
