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

#ifndef SRC_DAWN_NATIVE_D3D_UTILSD3D_H_
#define SRC_DAWN_NATIVE_D3D_UTILSD3D_H_

#include <string>

#include "dawn/native/Error.h"
#include "dawn/native/d3d/d3d_platform.h"

namespace dawn::native::d3d {

ResultOrError<std::wstring> ConvertStringToWstring(std::string_view s);

bool IsTypeless(DXGI_FORMAT format);

bool IsDepthStencil(DXGI_FORMAT format);

DXGI_FORMAT DXGITypelessTextureFormat(const DeviceBase* device, wgpu::TextureFormat format);
DXGI_FORMAT DXGITextureFormat(const DeviceBase* device, wgpu::TextureFormat format);

ResultOrError<wgpu::TextureFormat> FromUncompressedColorDXGITextureFormat(DXGI_FORMAT format);

DXGI_FORMAT DXGIVertexFormat(wgpu::VertexFormat format);

// Returns the DXGI format to use as the SRV format for a texture view. Returns
// DXGI_FORMAT_UNKNOWN if a single aspect is not selected.
DXGI_FORMAT D3DShaderResourceViewFormat(const DeviceBase* device,
                                        const Format& textureFormat,
                                        const Format& viewFormat,
                                        Aspect aspects);

}  // namespace dawn::native::d3d

#endif  // SRC_DAWN_NATIVE_D3D_UTILSD3D_H_
