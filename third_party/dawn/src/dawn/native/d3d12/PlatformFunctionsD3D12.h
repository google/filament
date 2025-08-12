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

#include "dawn/native/d3d/PlatformFunctions.h"
#include "dawn/native/d3d12/d3d12_platform.h"

#include "dawn/common/DynamicLib.h"

namespace dawn::native::d3d12 {

class PlatformFunctions final : public d3d::PlatformFunctions {
  public:
    PlatformFunctions();
    ~PlatformFunctions() override;

    MaybeError Initialize(std::span<const std::string> searchPaths);
    MaybeError EnsureDXCLibraries(std::span<const std::string> searchPaths);
    bool IsPIXEventRuntimeLoaded() const;

    // Functions from d3d12.dll
    PFN_D3D12_CREATE_DEVICE d3d12CreateDevice = nullptr;
    PFN_D3D12_GET_DEBUG_INTERFACE d3d12GetDebugInterface = nullptr;

    PFN_D3D12_SERIALIZE_ROOT_SIGNATURE d3d12SerializeRootSignature = nullptr;
    PFN_D3D12_CREATE_ROOT_SIGNATURE_DESERIALIZER d3d12CreateRootSignatureDeserializer = nullptr;
    PFN_D3D12_SERIALIZE_VERSIONED_ROOT_SIGNATURE d3d12SerializeVersionedRootSignature = nullptr;
    PFN_D3D12_CREATE_VERSIONED_ROOT_SIGNATURE_DESERIALIZER
    d3d12CreateVersionedRootSignatureDeserializer = nullptr;

    // Functions from d3d11.dll
    PFN_D3D11ON12_CREATE_DEVICE d3d11on12CreateDevice = nullptr;

    // Functions from WinPixEventRuntime.dll
    using PFN_PIX_END_EVENT_ON_COMMAND_LIST =
        HRESULT(WINAPI*)(ID3D12GraphicsCommandList* commandList);

    PFN_PIX_END_EVENT_ON_COMMAND_LIST pixEndEventOnCommandList = nullptr;

    using PFN_PIX_BEGIN_EVENT_ON_COMMAND_LIST = HRESULT(
        WINAPI*)(ID3D12GraphicsCommandList* commandList, UINT64 color, _In_ PCSTR formatString);

    PFN_PIX_BEGIN_EVENT_ON_COMMAND_LIST pixBeginEventOnCommandList = nullptr;

    using PFN_SET_MARKER_ON_COMMAND_LIST = HRESULT(WINAPI*)(ID3D12GraphicsCommandList* commandList,
                                                            UINT64 color,
                                                            _In_ PCSTR formatString);

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

    DynamicLib mD3D12Lib;
    DynamicLib mD3D11Lib;
    DynamicLib mPIXEventRuntimeLib;
    DynamicLib mDXILLib;
    DynamicLib mDXCompilerLib;
};

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_PLATFORMFUNCTIONS_H_
