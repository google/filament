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

#include "dawn/native/d3d11/PipelineLayoutD3D11.h"

#include "dawn/common/BitSetIterator.h"
#include "dawn/common/MatchVariant.h"
#include "dawn/native/BindGroupLayoutInternal.h"
#include "dawn/native/d3d11/DeviceD3D11.h"

namespace dawn::native::d3d11 {

// static
ResultOrError<Ref<PipelineLayout>> PipelineLayout::Create(
    Device* device,
    const UnpackedPtr<PipelineLayoutDescriptor>& descriptor) {
    Ref<PipelineLayout> pipelineLayout = AcquireRef(new PipelineLayout(device, descriptor));
    DAWN_TRY(pipelineLayout->Initialize(device));
    return pipelineLayout;
}

MaybeError PipelineLayout::Initialize(Device* device) {
    unsigned int constantBufferIndex = 0;
    unsigned int samplerIndex = 0;
    unsigned int shaderResourceViewIndex = 0;
    // For d3d11 pixel shaders, the render targets and unordered-access views share the same
    // resource slots when being written out. So we assign UAV binding index decreasingly here.
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11devicecontext-omsetrendertargetsandunorderedaccessviews
    // TODO(dawn:1818): Support testing on both FL11_0 and FL11_1.
    mTotalUAVBindingCount = device->GetUAVSlotCount();

    // Reserve last several UAV slots for Pixel Local Storage attachments.
    uint32_t unorderedAccessViewIndex =
        mTotalUAVBindingCount - static_cast<uint32_t>(GetStorageAttachmentSlots().size());
    for (BindGroupIndex group : IterateBitSet(GetBindGroupLayoutsMask())) {
        const BindGroupLayoutInternalBase* bgl = GetBindGroupLayout(group);
        mIndexInfo[group].resize(bgl->GetBindingCount());

        for (BindingIndex bindingIndex{0}; bindingIndex < bgl->GetBindingCount(); ++bindingIndex) {
            const BindingInfo& bindingInfo = bgl->GetBindingInfo(bindingIndex);
            MatchVariant(
                bindingInfo.bindingLayout,
                [&](const BufferBindingInfo& layout) {
                    switch (layout.type) {
                        case wgpu::BufferBindingType::Uniform:
                            mIndexInfo[group][bindingIndex] = constantBufferIndex++;
                            break;
                        case wgpu::BufferBindingType::Storage:
                        case kInternalStorageBufferBinding:
                            mIndexInfo[group][bindingIndex] = --unorderedAccessViewIndex;
                            mUAVBindGroups.set(group);
                            break;
                        case wgpu::BufferBindingType::ReadOnlyStorage:
                        case kInternalReadOnlyStorageBufferBinding:
                            mIndexInfo[group][bindingIndex] = shaderResourceViewIndex++;
                            break;
                        case wgpu::BufferBindingType::BindingNotUsed:
                        case wgpu::BufferBindingType::Undefined:
                            DAWN_UNREACHABLE();
                    }
                },
                [&](const SamplerBindingInfo&) {
                    mIndexInfo[group][bindingIndex] = samplerIndex++;
                },
                [&](const StaticSamplerBindingInfo&) {
                    // Static samplers are implemented in the frontend on
                    // D3D11.
                    DAWN_UNREACHABLE();
                },
                [&](const TextureBindingInfo&) {
                    mIndexInfo[group][bindingIndex] = shaderResourceViewIndex++;
                },
                [&](const StorageTextureBindingInfo& layout) {
                    switch (layout.access) {
                        case wgpu::StorageTextureAccess::ReadWrite:
                        case wgpu::StorageTextureAccess::WriteOnly:
                            mIndexInfo[group][bindingIndex] = --unorderedAccessViewIndex;
                            mUAVBindGroups.set(group);
                            break;
                        case wgpu::StorageTextureAccess::ReadOnly:
                            mIndexInfo[group][bindingIndex] = shaderResourceViewIndex++;
                            break;
                        case wgpu::StorageTextureAccess::BindingNotUsed:
                        case wgpu::StorageTextureAccess::Undefined:
                            DAWN_UNREACHABLE();
                    }
                },
                [](const InputAttachmentBindingInfo&) { DAWN_UNREACHABLE(); });
        }
    }
    mUnusedUAVBindingCount = unorderedAccessViewIndex;
    DAWN_ASSERT(constantBufferIndex <= kReservedConstantBufferSlot);

    return {};
}

const PipelineLayout::BindingIndexInfo& PipelineLayout::GetBindingIndexInfo() const {
    return mIndexInfo;
}

const BindGroupMask& PipelineLayout::GetUAVBindGroupLayoutsMask() const {
    return mUAVBindGroups;
}

}  // namespace dawn::native::d3d11
