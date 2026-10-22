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

#include "src/dawn/native/d3d12/PlatformFunctionsD3D12.h"

#include <array>
#include <sstream>
#include <string>
#include <utility>

#include "src/dawn/common/SystemUtils.h"
#include "src/utils/log.h"

namespace dawn::native::d3d12 {

PlatformFunctions::PlatformFunctions() = default;
PlatformFunctions::~PlatformFunctions() = default;

MaybeError PlatformFunctions::Initialize(std::span<const std::string> searchPaths) {
    DAWN_TRY(Base::Initialize());

    DAWN_TRY(LoadD3D12());
    DAWN_TRY(LoadD3D11());
    LoadPIXRuntime(searchPaths);
#ifdef DAWN_USE_AGILITY_SDK
    EnsureAgilitySDKDeviceFactory();
#endif
    return {};
}

MaybeError PlatformFunctions::LoadD3D12() {
#if DAWN_PLATFORM_IS(WINUWP)
    d3d12CreateDevice = &D3D12CreateDevice;
    d3d12GetDebugInterface = &D3D12GetDebugInterface;
    d3d12SerializeRootSignature = &D3D12SerializeRootSignature;
    d3d12CreateRootSignatureDeserializer = &D3D12CreateRootSignatureDeserializer;
    d3d12SerializeVersionedRootSignature = &D3D12SerializeVersionedRootSignature;
    d3d12CreateVersionedRootSignatureDeserializer = &D3D12CreateVersionedRootSignatureDeserializer;
#else
    std::string error;
    if (!mD3D12Lib.OpenSystemLibrary(L"d3d12.dll", &error) ||
        !mD3D12Lib.GetProc(&d3d12CreateDevice, "D3D12CreateDevice", &error) ||
        !mD3D12Lib.GetProc(&d3d12GetDebugInterface, "D3D12GetDebugInterface", &error) ||
        !mD3D12Lib.GetProc(&d3d12SerializeRootSignature, "D3D12SerializeRootSignature", &error) ||
        !mD3D12Lib.GetProc(&d3d12CreateRootSignatureDeserializer,
                           "D3D12CreateRootSignatureDeserializer", &error) ||
        !mD3D12Lib.GetProc(&d3d12SerializeVersionedRootSignature,
                           "D3D12SerializeVersionedRootSignature", &error) ||
        !mD3D12Lib.GetProc(&d3d12CreateVersionedRootSignatureDeserializer,
                           "D3D12CreateVersionedRootSignatureDeserializer", &error)) {
        return DAWN_INTERNAL_ERROR(error.c_str());
    }
    // Optional: only present in Agility SDK / newer d3d12.dll. Absence is not an error.
    mD3D12Lib.GetProc(&d3d12GetInterface, "D3D12GetInterface", &error);
#endif

    return {};
}

MaybeError PlatformFunctions::LoadD3D11() {
#if DAWN_PLATFORM_IS(WINUWP)
    d3d11on12CreateDevice = &D3D11On12CreateDevice;
#else
    std::string error;
    if (!mD3D11Lib.OpenSystemLibrary(L"d3d11.dll", &error) ||
        !mD3D11Lib.GetProc(&d3d11on12CreateDevice, "D3D11On12CreateDevice", &error)) {
        return DAWN_INTERNAL_ERROR(error.c_str());
    }
#endif

    return {};
}

bool PlatformFunctions::IsPIXEventRuntimeLoaded() const {
    return mPIXEventRuntimeLib.Valid();
}

HRESULT PlatformFunctions::CreateDevice(IUnknown* adapter,
                                        D3D_FEATURE_LEVEL featureLevel,
                                        REFIID riid,
                                        void** ppDevice) const {
#ifdef DAWN_USE_AGILITY_SDK
    if (mDeviceFactory) {
        return mDeviceFactory->CreateDevice(adapter, featureLevel, riid, ppDevice);
    }
#endif
    return d3d12CreateDevice(adapter, featureLevel, riid, ppDevice);
}

HRESULT PlatformFunctions::SerializeVersionedRootSignature(
    const D3D12_VERSIONED_ROOT_SIGNATURE_DESC* pDesc,
    ID3DBlob** ppResult,
    ID3DBlob** ppError) const {
#ifdef DAWN_USE_AGILITY_SDK
    if (mDeviceConfiguration) {
        return mDeviceConfiguration->SerializeVersionedRootSignature(pDesc, ppResult, ppError);
    }
#endif
    return d3d12SerializeVersionedRootSignature(pDesc, ppResult, ppError);
}

HRESULT PlatformFunctions::CreateVersionedRootSignatureDeserializer(const void* pBlob,
                                                                    SIZE_T size,
                                                                    REFIID riid,
                                                                    void** ppDeserializer) const {
#ifdef DAWN_USE_AGILITY_SDK
    if (mDeviceConfiguration) {
        return mDeviceConfiguration->CreateVersionedRootSignatureDeserializer(pBlob, size, riid,
                                                                              ppDeserializer);
    }
#endif
    return d3d12CreateVersionedRootSignatureDeserializer(pBlob, size, riid, ppDeserializer);
}

#ifdef DAWN_USE_AGILITY_SDK
void PlatformFunctions::EnsureAgilitySDKDeviceFactory() {
    // This helper is invoked exactly once by Initialize().
    DAWN_CHECK(!mDeviceFactory);

    if (!IsWindowsDeveloperModeEnabled()) {
        dawn::InfoLog() << "[AgilitySDK] Developer Mode is not enabled; skipping Agility SDK "
                           "device factory creation.";
        return;
    }

    // d3d12GetInterface must be available when building with Agility SDK.
    // TODO(crbug.com/517940507): Gracefully handle when D3D12GetInterface is not available.
    DAWN_CHECK(d3d12GetInterface != nullptr);

    ComPtr<ID3D12SDKConfiguration1> sdkConfig1;
    DAWN_CHECK(
        SUCCEEDED(d3d12GetInterface(CLSID_D3D12SDKConfiguration, IID_PPV_ARGS(&sdkConfig1))));

    DAWN_CHECK(SUCCEEDED(sdkConfig1->CreateDeviceFactory(D3D12_PREVIEW_SDK_VERSION, ".\\D3D12\\",
                                                         IID_PPV_ARGS(&mDeviceFactory))));

    // Allow the factory to return an existing compatible device rather than
    // always creating a new one. Without this flag, Dawn and the Chromium media
    // engine would each load a separate UMD instance, wasting memory.
    DAWN_CHECK(SUCCEEDED(
        mDeviceFactory->SetFlags(D3D12_DEVICE_FACTORY_FLAG_ALLOW_RETURNING_EXISTING_DEVICE)));

    // Obtain the device configuration interface for root signature operations.
    DAWN_CHECK(SUCCEEDED(mDeviceFactory.As(&mDeviceConfiguration)));

    // Must enable experimental shader models for SM 6.10 to use D3D12 LinAlg while it's still in
    // Preview. This must be done before the device is created via the factory.
    {
        UUID features[] = {D3D12ExperimentalShaderModels};
        HRESULT hr = mDeviceFactory->EnableExperimentalFeatures(_countof(features), features,
                                                                nullptr, nullptr);
        if (FAILED(hr)) {
            dawn::InfoLog() << "[AgilitySDK] D3D12ExperimentalShaderModels, needed for LinAlg, is "
                               "not supported, either because it's unrecognized, or Developer Mode "
                               "is not enabled, or some other reason.";
            return;
        }
    }

    dawn::InfoLog() << "[AgilitySDK] active: SDK version = " << D3D12_PREVIEW_SDK_VERSION;
}
#endif  // DAWN_USE_AGILITY_SDK

void PlatformFunctions::LoadPIXRuntime(std::span<const std::string> searchPaths) {
    // TODO(dawn:766):
    // In UWP PIX should be statically linked WinPixEventRuntime_UAP.lib
    // So maybe we should put WinPixEventRuntime as a third party package
    // Currently PIX is not going to be loaded in UWP since the following
    // mPIXEventRuntimeLib.Open will fail.

    if (!mPIXEventRuntimeLib.Open("WinPixEventRuntime.dll", searchPaths) ||
        !mPIXEventRuntimeLib.GetProc(&pixBeginEventOnCommandList, "PIXBeginEventOnCommandList") ||
        !mPIXEventRuntimeLib.GetProc(&pixEndEventOnCommandList, "PIXEndEventOnCommandList") ||
        !mPIXEventRuntimeLib.GetProc(&pixSetMarkerOnCommandList, "PIXSetMarkerOnCommandList")) {
        mPIXEventRuntimeLib.Close();
    }
}

#if defined(DAWN_USE_BUILT_DXC)
MaybeError PlatformFunctions::EnsureDXCLibraries(std::span<const std::string> searchPaths) {
    // TODO(dawn:766)
    // Statically linked with dxcompiler.lib in UWP
    // currently linked with dxcompiler.lib making CoreApp unable to activate
    // LoadDXIL and LoadDXCompiler will fail in UWP, but Initialize() can still be
    // successfully executed.

    if (mDXILLib.Valid()) {
        // The libraries are already loaded, no need to load them again.
        DAWN_CHECK(mDXCompilerLib.Valid());
        return {};
    }

    DynamicLib dxilLib;
    std::string error;
    // DXIL must be loaded before DXC, otherwise shader signing is unavailable
    if (!dxilLib.Open("dxil.dll", searchPaths, &error)) {
        return DAWN_INTERNAL_ERROR(std::move(error));
    }

    DynamicLib dxCompilerLib;
    if (!dxCompilerLib.Open("dxcompiler.dll", searchPaths, &error)) {
        return DAWN_INTERNAL_ERROR(std::move(error));
    }

    if (!dxCompilerLib.GetProc(&dxcCreateInstance, "DxcCreateInstance", &error)) {
        return DAWN_INTERNAL_ERROR(std::move(error));
    }

    mDXCompilerLib = std::move(dxCompilerLib);
    mDXILLib = std::move(dxilLib);
    return {};
}
#endif  // DAWN_USE_BUILT_DXC
}  // namespace dawn::native::d3d12
