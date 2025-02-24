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

#include "dawn/common/Assert.h"
#include "dawn/native/Format.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d11/DeviceD3D11.h"

namespace dawn::native::d3d11 {

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
