// Copyright 2017 The Dawn & Tint Authors
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

#include "dawn/native/d3d12/SamplerD3D12.h"

#include <algorithm>

#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/UtilsD3D12.h"

namespace dawn::native::d3d12 {

namespace {
D3D12_TEXTURE_ADDRESS_MODE AddressMode(wgpu::AddressMode mode) {
    switch (mode) {
        case wgpu::AddressMode::Repeat:
            return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        case wgpu::AddressMode::MirrorRepeat:
            return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
        case wgpu::AddressMode::ClampToEdge:
            return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        case wgpu::AddressMode::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}
}  // namespace

// static
Ref<Sampler> Sampler::Create(Device* device, const SamplerDescriptor* descriptor) {
    return AcquireRef(new Sampler(device, descriptor));
}

Sampler::Sampler(Device* device, const SamplerDescriptor* descriptor)
    : SamplerBase(device, descriptor) {
    D3D12_FILTER_TYPE minFilter;
    switch (descriptor->minFilter) {
        case wgpu::FilterMode::Nearest:
            minFilter = D3D12_FILTER_TYPE_POINT;
            break;
        case wgpu::FilterMode::Linear:
            minFilter = D3D12_FILTER_TYPE_LINEAR;
            break;
        case wgpu::FilterMode::Undefined:
            DAWN_UNREACHABLE();
    }

    D3D12_FILTER_TYPE magFilter;
    switch (descriptor->magFilter) {
        case wgpu::FilterMode::Nearest:
            magFilter = D3D12_FILTER_TYPE_POINT;
            break;
        case wgpu::FilterMode::Linear:
            magFilter = D3D12_FILTER_TYPE_LINEAR;
            break;
        case wgpu::FilterMode::Undefined:
            DAWN_UNREACHABLE();
    }

    D3D12_FILTER_TYPE mipmapFilter;
    switch (descriptor->mipmapFilter) {
        case wgpu::MipmapFilterMode::Nearest:
            mipmapFilter = D3D12_FILTER_TYPE_POINT;
            break;
        case wgpu::MipmapFilterMode::Linear:
            mipmapFilter = D3D12_FILTER_TYPE_LINEAR;
            break;
        case wgpu::MipmapFilterMode::Undefined:
            DAWN_UNREACHABLE();
    }

    D3D12_FILTER_REDUCTION_TYPE reduction = descriptor->compare == wgpu::CompareFunction::Undefined
                                                ? D3D12_FILTER_REDUCTION_TYPE_STANDARD
                                                : D3D12_FILTER_REDUCTION_TYPE_COMPARISON;

    // https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_sampler_desc
    mSamplerDesc.MaxAnisotropy = std::min<uint16_t>(GetMaxAnisotropy(), 16u);

    if (mSamplerDesc.MaxAnisotropy > 1) {
        mSamplerDesc.Filter = D3D12_ENCODE_ANISOTROPIC_FILTER(reduction);
    } else {
        mSamplerDesc.Filter =
            D3D12_ENCODE_BASIC_FILTER(minFilter, magFilter, mipmapFilter, reduction);
    }

    mSamplerDesc.AddressU = AddressMode(descriptor->addressModeU);
    mSamplerDesc.AddressV = AddressMode(descriptor->addressModeV);
    mSamplerDesc.AddressW = AddressMode(descriptor->addressModeW);
    mSamplerDesc.MipLODBias = 0.f;

    if (descriptor->compare != wgpu::CompareFunction::Undefined) {
        mSamplerDesc.ComparisonFunc = ToD3D12ComparisonFunc(descriptor->compare);
    } else {
        // Still set the function so it's not garbage.
        mSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    }
    mSamplerDesc.MinLOD = descriptor->lodMinClamp;
    mSamplerDesc.MaxLOD = descriptor->lodMaxClamp;
}

const D3D12_SAMPLER_DESC& Sampler::GetSamplerDescriptor() const {
    return mSamplerDesc;
}

}  // namespace dawn::native::d3d12
