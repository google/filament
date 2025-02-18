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

#ifndef SRC_DAWN_NATIVE_D3D_PLATFORMFUNCTIONS_H_
#define SRC_DAWN_NATIVE_D3D_PLATFORMFUNCTIONS_H_

#include <d3dcompiler.h>

#include <string>

#include "dawn/common/DynamicLib.h"
#include "dawn/native/Error.h"
#include "dawn/native/d3d/d3d_platform.h"

namespace dawn::native::d3d {

// Loads the functions required from the platform dynamically so that we don't need to rely on
// them being present in the system. For example linking against d3d12.lib would prevent
// dawn_native from loading on Windows 7 system where d3d12.dll doesn't exist.
class PlatformFunctions {
  public:
    PlatformFunctions();
    virtual ~PlatformFunctions();

    MaybeError LoadFunctions();
    uint64_t GetWindowsBuildNumber() const;

    // Functions from dxgi.dll
    using PFN_DXGI_GET_DEBUG_INTERFACE1 = HRESULT(WINAPI*)(UINT Flags,
                                                           REFIID riid,
                                                           _COM_Outptr_ void** pDebug);
    PFN_DXGI_GET_DEBUG_INTERFACE1 dxgiGetDebugInterface1 = nullptr;

    using PFN_CREATE_DXGI_FACTORY2 = HRESULT(WINAPI*)(UINT Flags,
                                                      REFIID riid,
                                                      _COM_Outptr_ void** ppFactory);
    PFN_CREATE_DXGI_FACTORY2 createDxgiFactory2 = nullptr;

    // Functions from D3DCompiler_47.dll
    pD3DCompile d3dCompile = nullptr;
    pD3DDisassemble d3dDisassemble = nullptr;

  private:
    MaybeError LoadDXGI();
    MaybeError LoadFXCompiler();
    void InitWindowsVersion();

    DynamicLib mDXGILib;
    DynamicLib mFXCompilerLib;

    uint64_t mCurrentBuildNumber;
};

}  // namespace dawn::native::d3d

#endif  // SRC_DAWN_NATIVE_D3D_PLATFORMFUNCTIONS_H_
