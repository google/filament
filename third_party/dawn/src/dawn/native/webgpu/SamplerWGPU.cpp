// Copyright 2025 The Dawn & Tint Authors
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

#include "dawn/native/webgpu/SamplerWGPU.h"

#include "dawn/common/StringViewUtils.h"
#include "dawn/native/webgpu/DeviceWGPU.h"

namespace dawn::native::webgpu {

namespace {

WGPUSamplerDescriptor ToWGPU(const SamplerDescriptor* desc) {
    return {
        .nextInChain = nullptr,
        .label = ToOutputStringView(desc->label),
        .addressModeU = ToAPI(desc->addressModeU),
        .addressModeV = ToAPI(desc->addressModeV),
        .addressModeW = ToAPI(desc->addressModeW),
        .magFilter = ToAPI(desc->magFilter),
        .minFilter = ToAPI(desc->minFilter),
        .mipmapFilter = ToAPI(desc->mipmapFilter),
        .lodMinClamp = desc->lodMinClamp,
        .lodMaxClamp = desc->lodMaxClamp,
        .compare = ToAPI(desc->compare),
        .maxAnisotropy = desc->maxAnisotropy,
    };
}

schema::Sampler ToSchema(const SamplerDescriptor* desc) {
    return {{
        .addressModeU = desc->addressModeU,
        .addressModeV = desc->addressModeV,
        .addressModeW = desc->addressModeW,
        .magFilter = desc->magFilter,
        .minFilter = desc->minFilter,
        .mipmapFilter = desc->mipmapFilter,
        .lodMinClamp = desc->lodMinClamp,
        .lodMaxClamp = desc->lodMaxClamp,
        .compare = desc->compare,
        .maxAnisotropy = desc->maxAnisotropy,
    }};
}

}  // namespace

// static
ResultOrError<Ref<Sampler>> Sampler::Create(Device* device, const SamplerDescriptor* descriptor) {
    Ref<Sampler> sampler = AcquireRef(new Sampler(device, descriptor));
    DAWN_TRY(sampler->Initialize());
    return sampler;
}

Sampler::Sampler(Device* device, const SamplerDescriptor* descriptor)
    : SamplerBase(device, descriptor),
      RecordableObject(schema::ObjectType::Sampler),
      ObjectWGPU(device->wgpu->samplerRelease),
      mSamplerParams(ToSchema(descriptor)) {
    WGPUSamplerDescriptor desc = ToWGPU(descriptor);
    mInnerHandle = ToBackend(GetDevice())
                       ->wgpu->deviceCreateSampler(ToBackend(GetDevice())->GetInnerHandle(), &desc);
    DAWN_ASSERT(mInnerHandle);
}

MaybeError Sampler::Initialize() {
    return {};
}

void Sampler::SetLabelImpl() {
    ToBackend(GetDevice())->CaptureSetLabel(this, GetLabel());
}

MaybeError Sampler::AddReferenced(CaptureContext& captureContext) {
    // Samplers don't reference anything
    return {};
}

MaybeError Sampler::CaptureCreationParameters(CaptureContext& captureContext) {
    Serialize(captureContext, mSamplerParams);
    return {};
}

}  // namespace dawn::native::webgpu
