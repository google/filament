// Copyright 2020 The Dawn & Tint Authors
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

#include "dawn/native/metal/BindGroupMTL.h"

#include "dawn/common/MatchVariant.h"
#include "dawn/common/Range.h"
#include "dawn/native/metal/BindGroupLayoutMTL.h"
#include "dawn/native/metal/BufferMTL.h"
#include "dawn/native/metal/DeviceMTL.h"
#include "dawn/native/metal/SamplerMTL.h"
#include "dawn/native/metal/TextureMTL.h"

namespace dawn::native::metal {

// static
ResultOrError<Ref<BindGroup>> BindGroup::Create(Device* device,
                                                const BindGroupDescriptor* descriptor) {
    Ref<BindGroup> bindGroup = ToBackend(descriptor->layout->GetInternalBindGroupLayout())
                                   ->AllocateBindGroup(device, descriptor);
    DAWN_TRY(bindGroup->Initialize(descriptor));
    return bindGroup;
}

BindGroup::BindGroup(Device* device, const BindGroupDescriptor* descriptor)
    : BindGroupBase(this, device, descriptor) {}

BindGroup::~BindGroup() = default;

MaybeError BindGroup::InitializeImpl() {
    auto* device = ToBackend(GetDevice());
    if (!device->IsToggleEnabled(Toggle::MetalUseArgumentBuffers)) {
        return {};
    }

    // TODO(crbug.com/363031535): The argument buffers should probably work in some kind of pool
    // instead of being allocated here

    auto layout = ToBackend(GetLayout());

    auto encoder = layout->GetArgumentEncoder();
    NSUInteger argumentBufferLength = [*encoder encodedLength];

    mArgumentBuffer = AcquireNSPRef([device->GetMTLDevice() newBufferWithLength:argumentBufferLength
                                                                        options:0]);
    [*encoder setArgumentBuffer:*mArgumentBuffer offset:0];

    for (BindingIndex bindingIndex : Range(layout->GetBindingCount())) {
        const BindingInfo& bindingInfo = layout->GetBindingInfo(bindingIndex);
        uint32_t dstBinding = uint32_t(bindingIndex - bindingInfo.indexInArray);

        auto HandleTextureBinding = [&]() {
            auto textureView = ToBackend(GetBindingAsTextureView(bindingIndex));
            id<MTLTexture> texture = textureView->GetMTLTexture();
            [*encoder setTexture:texture atIndex:dstBinding];
        };

        // TODO(crbug.com/363031535): The buffers, samplers and textures in the MatchVariant need to
        // have resource usage tracking added.
        MatchVariant(
            bindingInfo.bindingLayout,
            [&](const BufferBindingInfo& layout) {
                const BufferBinding& binding = GetBindingAsBufferBinding(bindingIndex);

                const id<MTLBuffer> buffer = ToBackend(binding.buffer)->GetMTLBuffer();
                [*encoder setBuffer:buffer offset:binding.offset atIndex:dstBinding];
            },
            [&](const SamplerBindingInfo&) {
                auto sampler = ToBackend(GetBindingAsSampler(bindingIndex));
                id<MTLSamplerState> samplerState = sampler->GetMTLSamplerState();
                [*encoder setSamplerState:samplerState atIndex:dstBinding];
            },
            [&](const StaticSamplerBindingInfo&) {
                // Static samplers are handled in the frontend.
                // TODO(crbug.com/dawn/2482): Implement static samplers in the
                // Metal backend.
                DAWN_CHECK(false);
            },
            [&](const TextureBindingInfo&) { HandleTextureBinding(); },
            [&](const StorageTextureBindingInfo&) { HandleTextureBinding(); },
            [](const InputAttachmentBindingInfo&) { DAWN_CHECK(false); });
    }

    return {};
}

void BindGroup::DeleteThis() {
    // This function must first run the destructor and then deallocate memory. Take a reference to
    // the BindGroupLayout+SlabAllocator before running the destructor so this function can access
    // it afterwards and it's not destroyed prematurely.
    Ref<BindGroupLayout> layout = ToBackend(GetLayout());
    BindGroupBase::DeleteThis();
    layout->DeallocateBindGroup(this);
}

NSPRef<id<MTLBuffer>> BindGroup::GetArgumentBuffer() const {
    return mArgumentBuffer;
}

}  // namespace dawn::native::metal
