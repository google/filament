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

#include "dawn/native/webgpu/BindGroupWGPU.h"

#include <vector>

#include "absl/container/inlined_vector.h"
#include "dawn/common/MatchVariant.h"
#include "dawn/common/StringViewUtils.h"
#include "dawn/native/webgpu/BindGroupLayoutWGPU.h"
#include "dawn/native/webgpu/BufferWGPU.h"
#include "dawn/native/webgpu/CaptureContext.h"
#include "dawn/native/webgpu/ComputePipelineWGPU.h"
#include "dawn/native/webgpu/DeviceWGPU.h"
#include "dawn/native/webgpu/ExternalTextureWGPU.h"
#include "dawn/native/webgpu/RenderPipelineWGPU.h"
#include "dawn/native/webgpu/SamplerWGPU.h"
#include "dawn/native/webgpu/TextureWGPU.h"
#include "dawn/native/webgpu/ToWGPU.h"

namespace dawn::native::webgpu {

namespace {

WGPUBindGroupEntry ToWGPU(const BindGroupEntry* entry) {
    return {
        .nextInChain = nullptr,
        .binding = entry->binding,
        .buffer = entry->buffer == nullptr ? nullptr : ToBackend(entry->buffer)->GetInnerHandle(),
        .offset = entry->offset,
        .size = entry->size,
        .sampler =
            entry->sampler == nullptr ? nullptr : ToBackend(entry->sampler)->GetInnerHandle(),
        .textureView = entry->textureView == nullptr
                           ? nullptr
                           : ToBackend(entry->textureView)->GetInnerHandle(),
    };
}

WGPUExternalTextureBindingEntry ToWGPU(const ExternalTextureBindingEntry* entry) {
    return {
        .chain =
            {
                .next = nullptr,
                .sType = WGPUSType_ExternalTextureBindingEntry,
            },
        .externalTexture = ToBackend(entry->externalTexture)->GetInnerHandle(),
    };
}

class ComboBindGroupDescriptor {
  public:
    explicit ComboBindGroupDescriptor(const UnpackedPtr<BindGroupDescriptor>& desc,
                                      uint32_t externalTextureCount) {
        // Use the pre-calculate the number of external textures to reserve upfront to prevent
        // InlinedVector reallocation.
        mExternalTextureEntries.reserve(externalTextureCount);

        mDesc.nextInChain = nullptr;
        mDesc.label = ToOutputStringView(desc->label);
        mDesc.layout = ToBackend(desc->layout->GetInternalBindGroupLayout())->GetInnerHandle();
        mDesc.entryCount = desc->entryCount;
        for (uint32_t i = 0; i < desc->entryCount; ++i) {
            UnpackedPtr<BindGroupEntry> entry = Unpack(&desc->entries[i]);
            mEntries.push_back(ToWGPU(*entry));

            if (auto* externalTextureEntry = entry.Get<ExternalTextureBindingEntry>()) {
                mExternalTextureEntries.push_back(ToWGPU(externalTextureEntry));
                mEntries.back().nextInChain = &mExternalTextureEntries.back().chain;
            }
        }
        mDesc.entries = mEntries.data();
    }

    const WGPUBindGroupDescriptor* Get() const { return &mDesc; }

  private:
    WGPUBindGroupDescriptor mDesc;
    absl::InlinedVector<WGPUBindGroupEntry, 8> mEntries;
    // Use an inline size of 1 since external textures are rare, and reserve the required capacity
    // in constructor to preserve reallocations.
    absl::InlinedVector<WGPUExternalTextureBindingEntry, 1> mExternalTextureEntries;
};

}  // namespace

// static
ResultOrError<Ref<BindGroup>> BindGroup::Create(
    Device* device,
    const UnpackedPtr<BindGroupDescriptor>& descriptor) {
    Ref<BindGroup> bindGroup =
        ToBackend(descriptor->layout->GetInternalBindGroupLayout())->AllocateBindGroup(descriptor);
    DAWN_TRY(bindGroup->Initialize(descriptor));
    return bindGroup;
}

BindGroup::BindGroup(Device* device, const UnpackedPtr<BindGroupDescriptor>& descriptor)
    : BindGroupBase(this, device, descriptor),
      RecordableObject(schema::ObjectType::BindGroup),
      ObjectWGPU(device->wgpu->bindGroupRelease) {
    ComboBindGroupDescriptor desc(descriptor, GetLayout()->GetExternalTextureCount());
    mInnerHandle =
        ToBackend(GetDevice())
            ->wgpu->deviceCreateBindGroup(ToBackend(GetDevice())->GetInnerHandle(), desc.Get());
    DAWN_ASSERT(mInnerHandle);
}

MaybeError BindGroup::InitializeImpl() {
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

void BindGroup::SetLabelImpl() {
    ToBackend(GetDevice())->CaptureSetLabel(this, GetLabel());
}

MaybeError BindGroup::AddReferenced(CaptureContext& captureContext) {
    // We have to include any referenced bound textures views as the front end does
    // not track texture views.
    BindGroupLayoutInternalBase* layout = GetLayout();
    DAWN_TRY(captureContext.AddResource(ToBackend(layout)));

    {
        const auto& bindingMap = layout->GetBindingMap();
        for (const auto& [bindingNumber, apiBindingIndex] : bindingMap) {
            const auto& bindingInfo = layout->GetAPIBindingInfo(apiBindingIndex);

            DAWN_TRY(MatchVariant(
                bindingInfo.bindingLayout,
                [&](const SamplerBindingInfo& info) -> MaybeError {
                    BindingIndex bindingIndex = layout->AsBindingIndex(apiBindingIndex);
                    return captureContext.AddResource(ToBackend(GetBindingAsSampler(bindingIndex)));
                },
                [&](const StorageTextureBindingInfo& info) -> MaybeError {
                    BindingIndex bindingIndex = layout->AsBindingIndex(apiBindingIndex);
                    return captureContext.AddResource(
                        ToBackend(GetBindingAsTextureView(bindingIndex)));
                },
                [&](const TextureBindingInfo& info) -> MaybeError {
                    BindingIndex bindingIndex = layout->AsBindingIndex(apiBindingIndex);
                    return captureContext.AddResource(
                        ToBackend(GetBindingAsTextureView(bindingIndex)));
                },
                [&](const ExternalTextureBindingInfo& info) -> MaybeError {
                    Ref<ExternalTextureBase> externalTexture =
                        GetBoundExternalTexture(apiBindingIndex);
                    if (externalTexture) {
                        DAWN_TRY(captureContext.AddResource(ToBackend(externalTexture.Get())));

                        TextureViewBase* plane1 = GetBindingAsTextureView(info.plane1);
                        if (plane1 != nullptr) {
                            DAWN_TRY(captureContext.AddResource(ToBackend(plane1)));
                        }

                        // No need to add reference of the internal params buffer (info.metadata) as
                        // it is not used in WebGPU backend. That of the replayed backend will still
                        // be created internally.
                    }

                    // If not found, it must be a texture view used as an external texture.
                    // plane0 is that texture view.
                    TextureViewBase* plane0 = GetBindingAsTextureView(info.plane0);
                    if (plane0 != nullptr) {
                        DAWN_TRY(captureContext.AddResource(ToBackend(plane0)));
                    }

                    return {};
                },
                [&](const auto& info) -> MaybeError { return {}; }));
        }
    }

    return {};
}

MaybeError BindGroup::CaptureCreationParameters(CaptureContext& captureContext) {
    BindGroupLayoutInternalBase* layout = GetLayout();
    const auto& bindingMap = layout->GetBindingMap();

    schema::BindGroup bg{{
        .layoutId = captureContext.GetId(layout),
        .numEntries = uint32_t(bindingMap.size()),
    }};
    Serialize(captureContext, bg);

    for (const auto& [bindingNumber, apiBindingIndex] : bindingMap) {
        const auto& bindingInfo = layout->GetAPIBindingInfo(apiBindingIndex);
        uint32_t binding = uint32_t(bindingNumber);

        MatchVariant(
            bindingInfo.bindingLayout,
            [&](const BufferBindingInfo& info) {
                BindingIndex bindingIndex = layout->AsBindingIndex(apiBindingIndex);
                const auto& entry = GetBindingAsBufferBinding(bindingIndex);
                schema::BindGroupEntryTypeBufferBinding data{{
                    .binding = binding,
                    .data{{
                        .bufferId = captureContext.GetId(entry.buffer.get()),
                        .offset = entry.offset,
                        .size = entry.size,
                    }},
                }};
                Serialize(captureContext, data);
            },
            [&](const SamplerBindingInfo& info) {
                BindingIndex bindingIndex = layout->AsBindingIndex(apiBindingIndex);
                const auto& entry = GetBindingAsSampler(bindingIndex);
                schema::BindGroupEntryTypeSamplerBinding data{{
                    .binding = binding,
                    .data{{
                        .samplerId = captureContext.GetId(entry),
                    }},
                }};
                Serialize(captureContext, data);
            },
            [&](const StorageTextureBindingInfo& info) {
                BindingIndex bindingIndex = layout->AsBindingIndex(apiBindingIndex);
                const auto& entry = GetBindingAsTextureView(bindingIndex);
                schema::BindGroupEntryTypeTextureBinding data{{
                    .binding = binding,
                    .data{{
                        .textureViewId = captureContext.GetId(entry),
                    }},
                }};
                Serialize(captureContext, data);
            },
            [&](const TextureBindingInfo& info) {
                BindingIndex bindingIndex = layout->AsBindingIndex(apiBindingIndex);
                const auto& entry = GetBindingAsTextureView(bindingIndex);
                schema::BindGroupEntryTypeTextureBinding data{{
                    .binding = binding,
                    .data{{
                        .textureViewId = captureContext.GetId(entry),
                    }},
                }};
                Serialize(captureContext, data);
            },
            [&](const ExternalTextureBindingInfo& info) {
                Ref<ExternalTextureBase> externalTexture = GetBoundExternalTexture(apiBindingIndex);

                schema::BindGroupEntryTypeExternalTextureBinding data{{
                    .binding = binding,
                    .data{{
                        .externalTextureId = captureContext.GetId(ToBackend(externalTexture)),
                        // The binding could bind a regular texture view if not a externalTexture
                        .textureViewId = externalTexture
                                             ? 0
                                             : captureContext.GetId(
                                                   ToBackend(GetBindingAsTextureView(info.plane0))),
                    }},
                }};
                Serialize(captureContext, data);
            },
            [&](const auto& info) { DAWN_CHECK(false); });
    }

    return {};
}

}  // namespace dawn::native::webgpu
