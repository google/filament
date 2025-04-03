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

#include "dawn/native/d3d11/BackendD3D11.h"

#include <memory>
#include <utility>

#include "dawn/common/Log.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/D3D11Backend.h"
#include "dawn/native/Instance.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d11/PhysicalDeviceD3D11.h"
#include "dawn/native/d3d11/PlatformFunctionsD3D11.h"

namespace dawn::native::d3d11 {
namespace {

MaybeError ValidateRequestOptions(const UnpackedPtr<RequestAdapterOptions>& options,
                                  ComPtr<IDXGIAdapter>* dxgiAdapter,
                                  ComPtr<ID3D11Device>* d3d11Device) {
    auto* d3d11DeviceOption = options.Get<RequestAdapterOptionsD3D11Device>();
    if (!d3d11DeviceOption) {
        return {};
    }

    DAWN_INVALID_IF(!d3d11DeviceOption->device,
                    "RequestAdapterOptionsD3D11Device::device is null.");

    ComPtr<ID3D11Multithread> d3d11Multithread;
    DAWN_TRY(CheckHRESULT(d3d11DeviceOption->device.As(&d3d11Multithread),
                          "D3D11: Get ID3D11Multithread"));

    DAWN_INVALID_IF(!d3d11Multithread->GetMultithreadProtected(),
                    "Multithread protection is not enabled.");

    ComPtr<IDXGIDevice> dxgiDevice;
    DAWN_TRY(CheckHRESULT(d3d11DeviceOption->device.As(&dxgiDevice), "D3D11: Get IDXGIDevice"));

    ComPtr<IDXGIAdapter> adapter;
    DAWN_TRY(CheckHRESULT(dxgiDevice->GetAdapter(&adapter), "D3D11: Get IDXGIAdapter"));

    DXGI_ADAPTER_DESC adapterDesc;
    DAWN_TRY(CheckHRESULT(adapter->GetDesc(&adapterDesc), "D3D11: IDXGIAdapter::GetDesc()"));

    if (auto* luidOptions = options.Get<d3d::RequestAdapterOptionsLUID>()) {
        DAWN_INVALID_IF(
            memcmp(&adapterDesc.AdapterLuid, &luidOptions->adapterLUID, sizeof(LUID)) != 0,
            "RequestAdapterOptionsLUID and RequestAdapterOptionsD3D11Device don't match.");
    }

    *dxgiAdapter = std::move(adapter);
    *d3d11Device = d3d11DeviceOption->device;

    return {};
}

}  // namespace

Backend::Backend(InstanceBase* instance) : Base(instance, wgpu::BackendType::D3D11) {}

MaybeError Backend::Initialize() {
    auto functions = std::make_unique<PlatformFunctions>();
    DAWN_TRY(functions->LoadFunctions());

    DAWN_TRY(Base::Initialize(std::move(functions)));

    return {};
}

const PlatformFunctions* Backend::GetFunctions() const {
    return static_cast<const PlatformFunctions*>(Base::GetFunctions());
}

std::vector<Ref<PhysicalDeviceBase>> Backend::DiscoverPhysicalDevices(
    const UnpackedPtr<RequestAdapterOptions>& options) {
    if (options->forceFallbackAdapter) {
        return {};
    }

    ComPtr<IDXGIAdapter> dxgiAdapter;
    ComPtr<ID3D11Device> d3d11Device;
    if (GetInstance()->ConsumedError(ValidateRequestOptions(options, &dxgiAdapter, &d3d11Device))) {
        return {};
    }

    if (d3d11Device) {
        Ref<PhysicalDeviceBase> physicalDevice;
        if (GetInstance()->ConsumedErrorAndWarnOnce(
                CreatePhysicalDevice(std::move(dxgiAdapter), std::move(d3d11Device)),
                &physicalDevice) ||
            !physicalDevice->SupportsFeatureLevel(options->featureLevel, GetInstance())) {
            return {};
        }
        return {std::move(physicalDevice)};
    }

    return Base::DiscoverPhysicalDevices(options);
}

ResultOrError<Ref<PhysicalDeviceBase>> Backend::CreatePhysicalDeviceFromIDXGIAdapter(
    ComPtr<IDXGIAdapter> dxgiAdapter) {
    return CreatePhysicalDevice(std::move(dxgiAdapter), /*d3d11Device=*/{});
}

ResultOrError<Ref<PhysicalDeviceBase>> Backend::CreatePhysicalDevice(
    ComPtr<IDXGIAdapter> dxgiAdapter,
    ComPtr<ID3D11Device> d3d11Device) {
    ComPtr<IDXGIAdapter3> dxgiAdapter3;
    DAWN_TRY(CheckHRESULT(dxgiAdapter.As(&dxgiAdapter3), "DXGIAdapter retrieval"));

    Ref<PhysicalDevice> physicalDevice =
        AcquireRef(new PhysicalDevice(this, std::move(dxgiAdapter3), std::move(d3d11Device)));
    DAWN_TRY(physicalDevice->Initialize());

    return {std::move(physicalDevice)};
}

BackendConnection* Connect(InstanceBase* instance) {
    Backend* backend = new Backend(instance);

    if (instance->ConsumedError(backend->Initialize())) {
        delete backend;
        return nullptr;
    }

    return backend;
}

}  // namespace dawn::native::d3d11
