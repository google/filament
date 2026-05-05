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

#include "dawn/native/d3d11/UtilsD3D11.h"

#include <sstream>

#include "dawn/common/Assert.h"
#include "dawn/native/Format.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d11/DeviceD3D11.h"

namespace dawn::native::d3d11 {

std::string SRVCreationStr(ID3D11Resource* resource,
                           const D3D11_SHADER_RESOURCE_VIEW_DESC1& srvDesc) {
    std::ostringstream ss;
    ss << "CreateShaderResourceView1, " << resource;
    ss << ", Format: " << srvDesc.Format;
    ss << ", ViewDimension: " << srvDesc.ViewDimension;

    switch (srvDesc.ViewDimension) {
        case D3D11_SRV_DIMENSION_BUFFER:
            ss << ", Buffer.FirstElement: " << srvDesc.Buffer.FirstElement;
            ss << ", Buffer.NumElements: " << srvDesc.Buffer.NumElements;
            break;
        case D3D11_SRV_DIMENSION_TEXTURE1D:
            ss << ", Texture1D.MipLevels: " << srvDesc.Texture1D.MipLevels;
            ss << ", Texture1D.MostDetailedMip: " << srvDesc.Texture1D.MostDetailedMip;
            break;
        case D3D11_SRV_DIMENSION_TEXTURE1DARRAY:
            ss << ", Texture1DArray.ArraySize: " << srvDesc.Texture1DArray.ArraySize;
            ss << ", Texture1DArray.FirstArraySlice: " << srvDesc.Texture1DArray.FirstArraySlice;
            ss << ", Texture1DArray.MipLevels: " << srvDesc.Texture1DArray.MipLevels;
            ss << ", Texture1DArray.MostDetailedMip: " << srvDesc.Texture1DArray.MostDetailedMip;
            break;
        case D3D11_SRV_DIMENSION_TEXTURE2D:
            ss << ", Texture2D.MipLevels: " << srvDesc.Texture2D.MipLevels;
            ss << ", Texture2D.MostDetailedMip: " << srvDesc.Texture2D.MostDetailedMip;
            ss << ", Texture2D.PlaneSlice: " << srvDesc.Texture2D.PlaneSlice;
            break;
        case D3D11_SRV_DIMENSION_TEXTURE2DARRAY:
            ss << ", Texture2DArray.ArraySize: " << srvDesc.Texture2DArray.ArraySize;
            ss << ", Texture2DArray.FirstArraySlice: " << srvDesc.Texture2DArray.FirstArraySlice;
            ss << ", Texture2DArray.MipLevels: " << srvDesc.Texture2DArray.MipLevels;
            ss << ", Texture2DArray.MostDetailedMip: " << srvDesc.Texture2DArray.MostDetailedMip;
            ss << ", Texture2DArray.PlaneSlice: " << srvDesc.Texture2DArray.PlaneSlice;
            break;
        case D3D11_SRV_DIMENSION_TEXTURE2DMS:
            // This structure is empty.
            break;
        case D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY:
            ss << ", Texture2DMSArray.ArraySize: " << srvDesc.Texture2DMSArray.ArraySize;
            ss << ", Texture2DMSArray.FirstArraySlice: "
               << srvDesc.Texture2DMSArray.FirstArraySlice;
            break;
        case D3D11_SRV_DIMENSION_TEXTURE3D:
            ss << ", Texture3D.MipLevels: " << srvDesc.Texture3D.MipLevels;
            ss << ", Texture3D.MostDetailedMip: " << srvDesc.Texture3D.MostDetailedMip;
            break;
        case D3D11_SRV_DIMENSION_TEXTURECUBE:
            ss << ", TextureCube.MipLevels: " << srvDesc.TextureCube.MipLevels;
            ss << ", TextureCube.MostDetailedMip: " << srvDesc.TextureCube.MostDetailedMip;
            break;
        case D3D11_SRV_DIMENSION_TEXTURECUBEARRAY:
            ss << ", TextureCubeArray.First2DArrayFace: "
               << srvDesc.TextureCubeArray.First2DArrayFace;
            ss << ", TextureCubeArray.NumCubes: " << srvDesc.TextureCubeArray.NumCubes;
            ss << ", TextureCubeArray.MipLevels: " << srvDesc.TextureCubeArray.MipLevels;
            ss << ", TextureCubeArray.MostDetailedMip: "
               << srvDesc.TextureCubeArray.MostDetailedMip;
            break;
        case D3D11_SRV_DIMENSION_BUFFEREX:
            ss << ", BufferEx.FirstElement: " << srvDesc.BufferEx.FirstElement;
            ss << ", BufferEx.NumElements: " << srvDesc.BufferEx.NumElements;
            ss << ", BufferEx.Flags: " << srvDesc.BufferEx.Flags;
            break;
        default:
            break;
    }
    return ss.str();
}

D3D11_COMPARISON_FUNC ToD3D11ComparisonFunc(wgpu::CompareFunction func) {
    switch (func) {
        case wgpu::CompareFunction::Never:
            return D3D11_COMPARISON_NEVER;
        case wgpu::CompareFunction::Less:
            return D3D11_COMPARISON_LESS;
        case wgpu::CompareFunction::LessEqual:
            return D3D11_COMPARISON_LESS_EQUAL;
        case wgpu::CompareFunction::Greater:
            return D3D11_COMPARISON_GREATER;
        case wgpu::CompareFunction::GreaterEqual:
            return D3D11_COMPARISON_GREATER_EQUAL;
        case wgpu::CompareFunction::Equal:
            return D3D11_COMPARISON_EQUAL;
        case wgpu::CompareFunction::NotEqual:
            return D3D11_COMPARISON_NOT_EQUAL;
        case wgpu::CompareFunction::Always:
            return D3D11_COMPARISON_ALWAYS;
        case wgpu::CompareFunction::Undefined:
            DAWN_UNREACHABLE();
    }
}

void SetDebugName(Device* device,
                  ID3D11DeviceChild* object,
                  const char* prefix,
                  std::string label) {
    if (!device->IsToggleEnabled(Toggle::UseUserDefinedLabelsInBackend)) {
        return;
    }

    if (!object) {
        return;
    }

    if (label.empty()) {
        object->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(prefix), prefix);
        return;
    }

    std::string objectName = prefix;
    objectName += "_";
    objectName += label;
    object->SetPrivateData(WKPDID_D3DDebugObjectName, objectName.length(), objectName.c_str());
}

bool IsDebugLayerEnabled(const ComPtr<ID3D11Device>& d3d11Device) {
    ComPtr<ID3D11Debug> d3d11Debug;
    return SUCCEEDED(d3d11Device.As(&d3d11Debug));
}

}  // namespace dawn::native::d3d11
