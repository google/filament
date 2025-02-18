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

#include "dawn/native/d3d11/SamplerD3D11.h"

#include <algorithm>

#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d11/DeviceD3D11.h"
#include "dawn/native/d3d11/UtilsD3D11.h"

namespace dawn::native::d3d11 {

namespace {

D3D11_TEXTURE_ADDRESS_MODE D3D11TextureAddressMode(wgpu::AddressMode mode) {
    switch (mode) {
        case wgpu::AddressMode::Repeat:
            return D3D11_TEXTURE_ADDRESS_WRAP;
        case wgpu::AddressMode::MirrorRepeat:
            return D3D11_TEXTURE_ADDRESS_MIRROR;
        case wgpu::AddressMode::ClampToEdge:
            return D3D11_TEXTURE_ADDRESS_CLAMP;
        case wgpu::AddressMode::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

D3D11_FILTER_TYPE D3D11FilterType(wgpu::FilterMode mode) {
    switch (mode) {
        case wgpu::FilterMode::Nearest:
            return D3D11_FILTER_TYPE_POINT;
        case wgpu::FilterMode::Linear:
            return D3D11_FILTER_TYPE_LINEAR;
        case wgpu::FilterMode::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

D3D11_FILTER_TYPE D3D11MipmapFilterType(wgpu::MipmapFilterMode mode) {
    switch (mode) {
        case wgpu::MipmapFilterMode::Nearest:
            return D3D11_FILTER_TYPE_POINT;
        case wgpu::MipmapFilterMode::Linear:
            return D3D11_FILTER_TYPE_LINEAR;
        case wgpu::MipmapFilterMode::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

}  // namespace

// static
ResultOrError<Ref<Sampler>> Sampler::Create(Device* device, const SamplerDescriptor* descriptor) {
    auto sampler = AcquireRef(new Sampler(device, descriptor));
    DAWN_TRY(sampler->Initialize(descriptor));
    return sampler;
}

MaybeError Sampler::Initialize(const SamplerDescriptor* descriptor) {
    D3D11_SAMPLER_DESC samplerDesc = {};
    D3D11_FILTER_TYPE minFilter = D3D11FilterType(descriptor->minFilter);
    D3D11_FILTER_TYPE magFilter = D3D11FilterType(descriptor->magFilter);
    D3D11_FILTER_TYPE mipmapFilter = D3D11MipmapFilterType(descriptor->mipmapFilter);

    D3D11_FILTER_REDUCTION_TYPE reduction = descriptor->compare == wgpu::CompareFunction::Undefined
                                                ? D3D11_FILTER_REDUCTION_TYPE_STANDARD
                                                : D3D11_FILTER_REDUCTION_TYPE_COMPARISON;

    // https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_sampler_desc
    samplerDesc.MaxAnisotropy = std::min<uint16_t>(GetMaxAnisotropy(), 16u);

    if (samplerDesc.MaxAnisotropy > 1) {
        samplerDesc.Filter = D3D11_ENCODE_ANISOTROPIC_FILTER(reduction);
    } else {
        samplerDesc.Filter =
            D3D11_ENCODE_BASIC_FILTER(minFilter, magFilter, mipmapFilter, reduction);
    }

    samplerDesc.AddressU = D3D11TextureAddressMode(descriptor->addressModeU);
    samplerDesc.AddressV = D3D11TextureAddressMode(descriptor->addressModeV);
    samplerDesc.AddressW = D3D11TextureAddressMode(descriptor->addressModeW);
    samplerDesc.MipLODBias = 0.f;

    if (descriptor->compare != wgpu::CompareFunction::Undefined) {
        samplerDesc.ComparisonFunc = ToD3D11ComparisonFunc(descriptor->compare);
    } else {
        // Still set the function so it's not garbage.
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    }
    samplerDesc.MinLOD = descriptor->lodMinClamp;
    samplerDesc.MaxLOD = descriptor->lodMaxClamp;

    DAWN_TRY(CheckHRESULT(ToBackend(GetDevice())
                              ->GetD3D11Device()
                              ->CreateSamplerState(&samplerDesc, &mD3d11SamplerState),
                          "ID3D11Device::CreateSamplerState"));

    SetLabelImpl();
    return {};
}

ID3D11SamplerState* Sampler::GetD3D11SamplerState() const {
    return mD3d11SamplerState.Get();
}

void Sampler::SetLabelImpl() {
    SetDebugName(ToBackend(GetDevice()), mD3d11SamplerState.Get(), "Dawn_Sampler", GetLabel());
}

}  // namespace dawn::native::d3d11
