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

#include "dawn/native/metal/BindGroupLayoutMTL.h"

#include <vector>

#include "dawn/common/MatchVariant.h"
#include "dawn/native/Device.h"
#include "dawn/native/metal/BindGroupMTL.h"
#include "dawn/native/metal/DeviceMTL.h"

namespace dawn::native::metal {

// static
Ref<BindGroupLayout> BindGroupLayout::Create(DeviceBase* device,
                                             const BindGroupLayoutDescriptor* descriptor) {
    return AcquireRef(new BindGroupLayout(device, descriptor));
}

BindGroupLayout::BindGroupLayout(DeviceBase* device, const BindGroupLayoutDescriptor* descriptor)
    : BindGroupLayoutInternalBase(device, descriptor),
      mBindGroupAllocator(MakeFrontendBindGroupAllocator<BindGroup>(4096)) {
    if (!device->IsToggleEnabled(Toggle::MetalUseArgumentBuffers)) {
        return;
    }

    std::vector<MTLArgumentDescriptor*> descriptors;
    for (BindingIndex i{0}; i < GetBindingCount(); ++i) {
        auto& bindingInfo = GetBindingInfo(i);

        MTLArgumentDescriptor* desc = [MTLArgumentDescriptor argumentDescriptor];
        desc.index = uint32_t(bindingInfo.binding);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated"
        desc.access = MTLArgumentAccessReadOnly;
#pragma clang diagnostic pop

        // TODO(crbug.com/363031535): Handle more then uniform/storage.  (e.g. samplers, textures,
        // etc.)
        MatchVariant(
            bindingInfo.bindingLayout,
            [&](const BufferBindingInfo& layout) { desc.dataType = MTLDataTypePointer; },
            [&](const SamplerBindingInfo&) { desc.dataType = MTLDataTypeSampler; },
            [&](const StaticSamplerBindingInfo&) {
                // Static samplers are handled in the frontend.
                // TODO(crbug.com/dawn/2482): Implement static samplers in the
                // Metal backend.
                DAWN_CHECK(false);
            },
            [&](const TextureBindingInfo&) { desc.dataType = MTLDataTypeTexture; },
            [&](const StorageTextureBindingInfo&) { DAWN_CHECK(false); },
            [](const InputAttachmentBindingInfo&) { DAWN_CHECK(false); });

        descriptors.push_back(desc);
    }

    if (!descriptors.empty()) {
        NSRef<NSArray> ary = AcquireNSRef([NSArray arrayWithObjects:descriptors.data()
                                                              count:descriptors.size()]);
        mArgumentEncoder = [ToBackend(device)->GetMTLDevice() newArgumentEncoderWithArguments:*ary];
    }
}

BindGroupLayout::~BindGroupLayout() = default;

Ref<BindGroup> BindGroupLayout::AllocateBindGroup(Device* device,
                                                  const BindGroupDescriptor* descriptor) {
    return AcquireRef(mBindGroupAllocator->Allocate(device, descriptor));
}

void BindGroupLayout::DeallocateBindGroup(BindGroup* bindGroup) {
    mBindGroupAllocator->Deallocate(bindGroup);
}

void BindGroupLayout::ReduceMemoryUsage() {
    mBindGroupAllocator->DeleteEmptySlabs();
}

NSPRef<id<MTLArgumentEncoder>> BindGroupLayout::GetArgumentEncoder() const {
    return mArgumentEncoder;
}

}  // namespace dawn::native::metal
