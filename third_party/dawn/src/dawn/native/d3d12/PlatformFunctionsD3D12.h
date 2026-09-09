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

#ifndef SRC_DAWN_NATIVE_D3D12_PLATFORMFUNCTIONS_H_
#define SRC_DAWN_NATIVE_D3D12_PLATFORMFUNCTIONS_H_

#include <span>
#include <string>

#include "src/dawn/common/DynamicLib.h"
#include "src/dawn/native/d3d/PlatformFunctions.h"
#include "src/dawn/native/d3d12/d3d12_platform.h"

namespace dawn::native::d3d12 {

class PlatformFunctions final : public d3d::PlatformFunctions {
  public:
    PlatformFunctions();
    ~PlatformFunctions() override;

    MaybeError Initialize(std::span<const std::string> searchPaths);
    MaybeError EnsureDXCLibraries(std::span<const std::string> searchPaths);
    bool IsPIXEventRuntimeLoaded() const;

    // Helper methods that route to the Agility SDK interfaces when DAWN_USE_AGILITY_SDK
    // is defined, or fall back to the loaded d3d12.dll exports otherwise.
    HRESULT CreateDevice(IUnknown* adapter,
                         D3D_FEATURE_LEVEL featureLevel,
                         REFIID riid,
                         void** ppDevice) const;
    HRESULT SerializeVersionedRootSignature(const D3D12_VERSIONED_ROOT_SIGNATURE_DESC* pDesc,
                                            ID3DBlob** ppResult,
                                            ID3DBlob** ppError) const;
    HRESULT CreateVersionedRootSignatureDeserializer(const void* pBlob,
                                                     SIZE_T size,
                                                     REFIID riid,
                                                     void** ppDeserializer) const;

    // Functions from d3d12.dll
    PFN_D3D12_GET_DEBUG_INTERFACE d3d12GetDebugInterface = nullptr;

    PFN_D3D12_SERIALIZE_ROOT_SIGNATURE d3d12SerializeRootSignature = nullptr;
    PFN_D3D12_CREATE_ROOT_SIGNATURE_DESERIALIZER d3d12CreateRootSignatureDeserializer = nullptr;

    // Functions from d3d11.dll
    PFN_D3D11ON12_CREATE_DEVICE d3d11on12CreateDevice = nullptr;

    // Functions from WinPixEventRuntime.dll
    //
    // The only official reference for these function signatures is
    // https://devblogs.microsoft.com/pix/winpixeventruntime/
    // however it is incorrect: it says the third argument is a `formatString` like in PIXBeginEvent
    // (implying it would have varargs, but it doesn't). It appears that PIX in fact treats it as a
    // plain label, not a format string, so we've renamed it here.
    using PFN_PIX_BEGIN_EVENT_ON_COMMAND_LIST =
        HRESULT(WINAPI*)(ID3D12GraphicsCommandList* commandList, UINT64 color, _In_ PCSTR label);
    using PFN_PIX_END_EVENT_ON_COMMAND_LIST =
        HRESULT(WINAPI*)(ID3D12GraphicsCommandList* commandList);
    using PFN_SET_MARKER_ON_COMMAND_LIST = HRESULT(WINAPI*)(ID3D12GraphicsCommandList* commandList,
                                                            UINT64 color,
                                                            _In_ PCSTR label);

    PFN_PIX_BEGIN_EVENT_ON_COMMAND_LIST pixBeginEventOnCommandList = nullptr;
    PFN_PIX_END_EVENT_ON_COMMAND_LIST pixEndEventOnCommandList = nullptr;
    PFN_SET_MARKER_ON_COMMAND_LIST pixSetMarkerOnCommandList = nullptr;

    // Functions from dxcompiler.dll
    using PFN_DXC_CREATE_INSTANCE = HRESULT(WINAPI*)(REFCLSID rclsid,
                                                     REFIID riid,
                                                     _COM_Outptr_ void** ppCompiler);
    PFN_DXC_CREATE_INSTANCE dxcCreateInstance = nullptr;

  private:
    using Base = d3d::PlatformFunctions;

    MaybeError LoadD3D12();
    MaybeError LoadD3D11();
    void LoadPIXRuntime(std::span<const std::string> searchPaths);

    // Raw DLL exports — use the public helper methods instead.
    PFN_D3D12_CREATE_DEVICE d3d12CreateDevice = nullptr;
    PFN_D3D12_SERIALIZE_VERSIONED_ROOT_SIGNATURE d3d12SerializeVersionedRootSignature = nullptr;
    PFN_D3D12_CREATE_VERSIONED_ROOT_SIGNATURE_DESERIALIZER
    d3d12CreateVersionedRootSignatureDeserializer = nullptr;
    // Optional; nullptr on older systems without Agility SDK.
    PFN_D3D12_GET_INTERFACE d3d12GetInterface = nullptr;

    DynamicLib mD3D12Lib;
    DynamicLib mD3D11Lib;
    DynamicLib mPIXEventRuntimeLib;
    DynamicLib mDXILLib;
    DynamicLib mDXCompilerLib;

#ifdef DAWN_USE_AGILITY_SDK
    // Called once by Initialize().
    void EnsureAgilitySDKDeviceFactory();

    // These interfaces are implemented by D3D12Core.dll. They must be declared after the
    // DynamicLib members so reverse-order member destruction releases them before unloading their
    // implementation.
    // Non-null after a successful EnsureAgilitySDKDeviceFactory() call.
    ComPtr<ID3D12DeviceFactory> mDeviceFactory;
    ComPtr<ID3D12DeviceConfiguration> mDeviceConfiguration;
#endif  // DAWN_USE_AGILITY_SDK
};

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_PLATFORMFUNCTIONS_H_
