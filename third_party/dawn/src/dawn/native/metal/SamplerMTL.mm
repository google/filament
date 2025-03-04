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

#include "dawn/native/metal/SamplerMTL.h"

#include "dawn/native/metal/DeviceMTL.h"
#include "dawn/native/metal/UtilsMetal.h"

namespace dawn::native::metal {

namespace {
MTLSamplerMinMagFilter FilterModeToMinMagFilter(wgpu::FilterMode mode) {
    switch (mode) {
        case wgpu::FilterMode::Nearest:
            return MTLSamplerMinMagFilterNearest;
        case wgpu::FilterMode::Linear:
            return MTLSamplerMinMagFilterLinear;
        case wgpu::FilterMode::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

MTLSamplerMipFilter FilterModeToMipFilter(wgpu::MipmapFilterMode mode) {
    switch (mode) {
        case wgpu::MipmapFilterMode::Nearest:
            return MTLSamplerMipFilterNearest;
        case wgpu::MipmapFilterMode::Linear:
            return MTLSamplerMipFilterLinear;
        case wgpu::MipmapFilterMode::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

MTLSamplerAddressMode AddressMode(wgpu::AddressMode mode) {
    switch (mode) {
        case wgpu::AddressMode::Repeat:
            return MTLSamplerAddressModeRepeat;
        case wgpu::AddressMode::MirrorRepeat:
            return MTLSamplerAddressModeMirrorRepeat;
        case wgpu::AddressMode::ClampToEdge:
            return MTLSamplerAddressModeClampToEdge;
        case wgpu::AddressMode::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}
}  // namespace

// static
ResultOrError<Ref<Sampler>> Sampler::Create(Device* device, const SamplerDescriptor* descriptor) {
    DAWN_INVALID_IF(
        descriptor->compare != wgpu::CompareFunction::Undefined &&
            device->IsToggleEnabled(Toggle::MetalDisableSamplerCompare),
        "Sampler compare function (%s) not supported. Compare functions are disabled with the "
        "Metal backend.",
        descriptor->compare);

    Ref<Sampler> sampler = AcquireRef(new Sampler(device, descriptor));
    DAWN_TRY(sampler->Initialize(descriptor));
    return sampler;
}

Sampler::Sampler(DeviceBase* dev, const SamplerDescriptor* desc) : SamplerBase(dev, desc) {}

Sampler::~Sampler() = default;

MaybeError Sampler::Initialize(const SamplerDescriptor* descriptor) {
    NSRef<MTLSamplerDescriptor> mtlDescRef = AcquireNSRef([MTLSamplerDescriptor new]);
    MTLSamplerDescriptor* mtlDesc = mtlDescRef.Get();

    NSRef<NSString> label = MakeDebugName(GetDevice(), "Dawn_Sampler", GetLabel());
    mtlDesc.label = label.Get();

    mtlDesc.minFilter = FilterModeToMinMagFilter(descriptor->minFilter);
    mtlDesc.magFilter = FilterModeToMinMagFilter(descriptor->magFilter);
    mtlDesc.mipFilter = FilterModeToMipFilter(descriptor->mipmapFilter);

    mtlDesc.sAddressMode = AddressMode(descriptor->addressModeU);
    mtlDesc.tAddressMode = AddressMode(descriptor->addressModeV);
    mtlDesc.rAddressMode = AddressMode(descriptor->addressModeW);

    mtlDesc.lodMinClamp = descriptor->lodMinClamp;
    mtlDesc.lodMaxClamp = descriptor->lodMaxClamp;
    // https://developer.apple.com/documentation/metal/mtlsamplerdescriptor/1516164-maxanisotropy
    mtlDesc.maxAnisotropy = std::min<uint16_t>(GetMaxAnisotropy(), 16u);

    if (descriptor->compare != wgpu::CompareFunction::Undefined) {
        // Sampler compare is unsupported before A9, which we validate in
        // Sampler::Create.
        mtlDesc.compareFunction = ToMetalCompareFunction(descriptor->compare);
        // The value is default-initialized in the else-case, and we don't set it or the
        // Metal debug device errors.
    }

    mMtlSamplerState = AcquireNSPRef(
        [ToBackend(GetDevice())->GetMTLDevice() newSamplerStateWithDescriptor:mtlDesc]);

    if (mMtlSamplerState == nil) {
        return DAWN_OUT_OF_MEMORY_ERROR("Failed to allocate sampler.");
    }
    return {};
}

id<MTLSamplerState> Sampler::GetMTLSamplerState() {
    return mMtlSamplerState.Get();
}

}  // namespace dawn::native::metal
