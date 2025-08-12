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

#include "dawn/native/d3d12/PlatformFunctionsD3D12.h"

#include <array>
#include <sstream>
#include <string>
#include <utility>

#include "dawn/common/SystemUtils.h"

namespace dawn::native::d3d12 {

PlatformFunctions::PlatformFunctions() = default;
PlatformFunctions::~PlatformFunctions() = default;

MaybeError PlatformFunctions::Initialize(std::span<const std::string> searchPaths) {
    DAWN_TRY(Base::Initialize());

    DAWN_TRY(LoadD3D12());
    DAWN_TRY(LoadD3D11());
    LoadPIXRuntime(searchPaths);
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

#if DAWN_USE_BUILT_DXC
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
