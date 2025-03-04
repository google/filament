// Copyright 2023 The Dawn & Tint Authors
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

#include "dawn/native/d3d/BackendD3D.h"

#include <utility>

#include "dawn/common/HashUtils.h"
#include "dawn/common/Log.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/D3DBackend.h"
#include "dawn/native/Instance.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d/PhysicalDeviceD3D.h"
#include "dawn/native/d3d/PlatformFunctions.h"
#include "dawn/native/d3d/UtilsD3D.h"

namespace dawn::native::d3d {

namespace {

ResultOrError<ComPtr<IDXGIFactory4>> CreateFactory(const PlatformFunctions* functions,
                                                   BackendValidationLevel validationLevel) {
    ComPtr<IDXGIFactory4> factory;

    uint32_t dxgiFactoryFlags = 0;
    if (validationLevel != BackendValidationLevel::Disabled) {
        // Enable additional debug layers.
        dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    }

    if (FAILED(functions->createDxgiFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)))) {
        return DAWN_INTERNAL_ERROR("Failed to create a DXGI factory");
    }

    DAWN_ASSERT(factory != nullptr);
    return std::move(factory);
}

DXGI_GPU_PREFERENCE ToDXGIPowerPreference(wgpu::PowerPreference powerPreference) {
    switch (powerPreference) {
        case wgpu::PowerPreference::Undefined:
            return DXGI_GPU_PREFERENCE_UNSPECIFIED;
        case wgpu::PowerPreference::LowPower:
            return DXGI_GPU_PREFERENCE_MINIMUM_POWER;
        case wgpu::PowerPreference::HighPerformance:
            return DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE;
    }
}

}  // anonymous namespace

Backend::Backend(InstanceBase* instance, wgpu::BackendType type)
    : BackendConnection(instance, type) {}

MaybeError Backend::Initialize(std::unique_ptr<PlatformFunctions> functions) {
    mFunctions = std::move(functions);

    const auto instance = GetInstance();

    DAWN_TRY_ASSIGN(mFactory,
                    CreateFactory(mFunctions.get(), instance->GetBackendValidationLevel()));

    return {};
}

IDXGIFactory4* Backend::GetFactory() const {
    return mFactory.Get();
}

const PlatformFunctions* Backend::GetFunctions() const {
    return mFunctions.get();
}

ResultOrError<Ref<PhysicalDeviceBase>> Backend::GetOrCreatePhysicalDeviceFromLUID(LUID luid) {
    auto it = mPhysicalDevices.find(luid);
    if (it != mPhysicalDevices.end()) {
        // If we've already discovered this physical device, return it.
        return it->second;
    }

    ComPtr<IDXGIAdapter1> dxgiAdapter;
    DAWN_TRY(CheckHRESULT(GetFactory()->EnumAdapterByLuid(luid, IID_PPV_ARGS(&dxgiAdapter)),
                          "EnumAdapterByLuid"));

    Ref<PhysicalDeviceBase> physicalDevice;
    DAWN_TRY_ASSIGN(physicalDevice, CreatePhysicalDeviceFromIDXGIAdapter(dxgiAdapter));
    mPhysicalDevices.emplace(luid, physicalDevice);
    return physicalDevice;
}

ResultOrError<Ref<PhysicalDeviceBase>> Backend::GetOrCreatePhysicalDeviceFromIDXGIAdapter(
    ComPtr<IDXGIAdapter> dxgiAdapter) {
    DXGI_ADAPTER_DESC desc;
    DAWN_TRY(CheckHRESULT(dxgiAdapter->GetDesc(&desc), "IDXGIAdapter::GetDesc"));

    auto it = mPhysicalDevices.find(desc.AdapterLuid);
    if (it != mPhysicalDevices.end()) {
        // If we've already discovered this physical device, return it.
        return it->second;
    }

    Ref<PhysicalDeviceBase> physicalDevice;
    DAWN_TRY_ASSIGN(physicalDevice, CreatePhysicalDeviceFromIDXGIAdapter(dxgiAdapter));
    mPhysicalDevices.emplace(desc.AdapterLuid, physicalDevice);
    return physicalDevice;
}

std::vector<Ref<PhysicalDeviceBase>> Backend::DiscoverPhysicalDevices(
    const UnpackedPtr<RequestAdapterOptions>& options) {
    if (options->forceFallbackAdapter) {
        return {};
    }

    // Get or create just the physical device matching the dxgi adapter.
    if (auto* luidOptions = options.Get<RequestAdapterOptionsLUID>()) {
        Ref<PhysicalDeviceBase> physicalDevice;
        if (GetInstance()->ConsumedErrorAndWarnOnce(
                GetOrCreatePhysicalDeviceFromLUID(luidOptions->adapterLUID), &physicalDevice) ||
            !physicalDevice->SupportsFeatureLevel(options->featureLevel, GetInstance())) {
            return {};
        }
        return {std::move(physicalDevice)};
    }

    std::function<HRESULT(uint32_t, ComPtr<IDXGIAdapter1>&)> enumAdapters;
    DXGI_GPU_PREFERENCE gpuPreference = ToDXGIPowerPreference(options->powerPreference);

    ComPtr<IDXGIFactory6> factory6;
    HRESULT hr = GetFactory()->QueryInterface(IID_PPV_ARGS(&factory6));
    if (SUCCEEDED(hr)) {
        // IDXGIFactory6 is not available on all versions of Windows 10. If it is available, use it
        // to enumerate the adapters based on the desired power preference.
        enumAdapters = [&](uint32_t adapterIndex, ComPtr<IDXGIAdapter1>& dxgiAdapter) -> HRESULT {
            return factory6->EnumAdapterByGpuPreference(adapterIndex, gpuPreference,
                                                        IID_PPV_ARGS(&dxgiAdapter));
        };
    } else {
        enumAdapters = [&](uint32_t adapterIndex, ComPtr<IDXGIAdapter1>& dxgiAdapter) -> HRESULT {
            return GetFactory()->EnumAdapters1(adapterIndex, &dxgiAdapter);
        };
    }

    // Enumerate and discover all available physicalDevices.
    std::vector<Ref<PhysicalDeviceBase>> physicalDevices;
    for (uint32_t adapterIndex = 0;; ++adapterIndex) {
        ComPtr<IDXGIAdapter1> dxgiAdapter;
        if (enumAdapters(adapterIndex, dxgiAdapter) == DXGI_ERROR_NOT_FOUND) {
            break;  // No more physicalDevices to enumerate.
        }

        Ref<PhysicalDeviceBase> physicalDevice;
        if (GetInstance()->ConsumedErrorAndWarnOnce(
                GetOrCreatePhysicalDeviceFromIDXGIAdapter(std::move(dxgiAdapter)),
                &physicalDevice) ||
            !physicalDevice->SupportsFeatureLevel(options->featureLevel, GetInstance())) {
            continue;
        }
        physicalDevices.push_back(std::move(physicalDevice));
    }
    return physicalDevices;
}

size_t Backend::LUIDHashFunc::operator()(const LUID& luid) const {
    size_t hash = 0;
    dawn::HashCombine(&hash, luid.LowPart, luid.HighPart);
    return hash;
}

bool Backend::LUIDEqualFunc::operator()(const LUID& a, const LUID& b) const {
    return std::tie(a.LowPart, a.HighPart) == std::tie(b.LowPart, b.HighPart);
}

}  // namespace dawn::native::d3d
