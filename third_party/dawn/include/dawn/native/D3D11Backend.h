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

#ifndef INCLUDE_DAWN_NATIVE_D3D11BACKEND_H_
#define INCLUDE_DAWN_NATIVE_D3D11BACKEND_H_

#include <d3d11_1.h>
#include <wrl/client.h>

#include <memory>

#include "dawn/native/D3DBackend.h"

namespace dawn::native::d3d11 {

DAWN_NATIVE_EXPORT Microsoft::WRL::ComPtr<ID3D11Device> GetD3D11Device(WGPUDevice device);

// May be chained on RequestAdapterOptions
struct DAWN_NATIVE_EXPORT RequestAdapterOptionsD3D11Device : wgpu::ChainedStruct {
    RequestAdapterOptionsD3D11Device() {
        sType = static_cast<wgpu::SType>(WGPUSType_RequestAdapterOptionsD3D11Device);
    }

    Microsoft::WRL::ComPtr<ID3D11Device> device;
};

// May be chained on SharedTextureMemoryDescriptor
struct DAWN_NATIVE_EXPORT SharedTextureMemoryD3D11Texture2DDescriptor : wgpu::ChainedStruct {
    SharedTextureMemoryD3D11Texture2DDescriptor() {
        sType = static_cast<wgpu::SType>(WGPUSType_SharedTextureMemoryD3D11Texture2DDescriptor);
    }

    // This ID3D11Texture2D object must be created from the same ID3D11Device used in the
    // WGPUDevice.
    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
};

}  // namespace dawn::native::d3d11

#endif  // INCLUDE_DAWN_NATIVE_D3D11BACKEND_H_
