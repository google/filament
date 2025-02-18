// Copyright 2019 The Dawn & Tint Authors
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

#include "dawn/native/AttachmentState.h"

#include "dawn/common/BitSetIterator.h"
#include "dawn/common/Enumerator.h"
#include "dawn/common/ityp_span.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/Device.h"
#include "dawn/native/ObjectContentHasher.h"
#include "dawn/native/PipelineLayout.h"
#include "dawn/native/Texture.h"

namespace dawn::native {

AttachmentState::AttachmentState(DeviceBase* device,
                                 const RenderBundleEncoderDescriptor* descriptor)
    : ObjectBase(device), mSampleCount(descriptor->sampleCount) {
    DAWN_ASSERT(descriptor->colorFormatCount <= kMaxColorAttachments);
    auto colorFormats = ityp::SpanFromUntyped<ColorAttachmentIndex>(descriptor->colorFormats,
                                                                    descriptor->colorFormatCount);

    for (auto [i, format] : Enumerate(colorFormats)) {
        if (format != wgpu::TextureFormat::Undefined) {
            mColorAttachmentsSet.set(i);
            mColorFormats[i] = format;
        }
    }
    mDepthStencilFormat = descriptor->depthStencilFormat;

    // TODO(dawn:1710): support MSAA render to single sampled in render bundles.
    // TODO(dawn:1710): support LoadOp::ExpandResolveTexture in render bundles.
    // TODO(dawn:1704): support PLS in render bundles.

    SetContentHash(ComputeContentHash());
}

AttachmentState::AttachmentState(DeviceBase* device,
                                 const UnpackedPtr<RenderPipelineDescriptor>& descriptor,
                                 const PipelineLayoutBase* layout)
    : ObjectBase(device), mSampleCount(descriptor->multisample.count) {
    if (descriptor->fragment != nullptr) {
        DAWN_ASSERT(descriptor->fragment->targetCount <= kMaxColorAttachments);
        auto targets = ityp::SpanFromUntyped<ColorAttachmentIndex>(
            descriptor->fragment->targets, descriptor->fragment->targetCount);

        for (auto [i, target] : Enumerate(targets)) {
            wgpu::TextureFormat format = target.format;
            if (format != wgpu::TextureFormat::Undefined) {
                mColorAttachmentsSet.set(i);
                mColorFormats[i] = format;

                UnpackedPtr<ColorTargetState> unpackedTarget = Unpack(&target);
                if (auto* expandResolveState =
                        unpackedTarget.Get<ColorTargetStateExpandResolveTextureDawn>()) {
                    mExpandResolveInfo.attachmentsToExpandResolve.set(i,
                                                                      expandResolveState->enabled);
                    // The presence of ColorTargetStateExpandResolveTextureDawn implies that
                    // this color target has a resolve target. Doesn't matter `enabled` is true or
                    // not.
                    mExpandResolveInfo.resolveTargetsMask.set(i);
                }
            }
        }
    }
    if (!mExpandResolveInfo.attachmentsToExpandResolve.any()) {
        // If render pipeline doesn't have any color target using ExpandResolveTexture load op then
        // ignore resolve targets. This is to relax compatibility requirement for common cases
        // where ExpandResolveTexture is not used.
        mExpandResolveInfo.resolveTargetsMask.reset();
    }

    DAWN_ASSERT(IsSubset(mExpandResolveInfo.attachmentsToExpandResolve,
                         mExpandResolveInfo.resolveTargetsMask));

    if (descriptor->depthStencil != nullptr) {
        mDepthStencilFormat = descriptor->depthStencil->format;
    }

    mHasPLS = layout->HasPixelLocalStorage();
    mStorageAttachmentSlots = layout->GetStorageAttachmentSlots();

    SetContentHash(ComputeContentHash());
}

AttachmentState::AttachmentState(DeviceBase* device,
                                 const UnpackedPtr<RenderPassDescriptor>& descriptor)
    : ObjectBase(device) {
    auto colorAttachments = ityp::SpanFromUntyped<ColorAttachmentIndex>(
        descriptor->colorAttachments, descriptor->colorAttachmentCount);
    for (auto [i, colorAttachment] : Enumerate(colorAttachments)) {
        TextureViewBase* attachment = colorAttachment.view;
        if (attachment == nullptr) {
            continue;
        }
        mColorAttachmentsSet.set(i);
        mColorFormats[i] = attachment->GetFormat().format;

        UnpackedPtr<RenderPassColorAttachment> unpackedColorAttachment = Unpack(&colorAttachment);
        auto* msaaRenderToSingleSampledDesc =
            unpackedColorAttachment.Get<DawnRenderPassColorAttachmentRenderToSingleSampled>();
        uint32_t attachmentSampleCount;
        if (msaaRenderToSingleSampledDesc != nullptr &&
            msaaRenderToSingleSampledDesc->implicitSampleCount > 1) {
            attachmentSampleCount = msaaRenderToSingleSampledDesc->implicitSampleCount;
        } else {
            attachmentSampleCount = attachment->GetTexture()->GetSampleCount();
        }

        if (mSampleCount == 0) {
            mSampleCount = attachmentSampleCount;
        } else {
            DAWN_ASSERT(mSampleCount == attachmentSampleCount);
        }

        if (colorAttachment.loadOp == wgpu::LoadOp::ExpandResolveTexture) {
            mExpandResolveInfo.attachmentsToExpandResolve.set(i);
        }
        mExpandResolveInfo.resolveTargetsMask.set(i, colorAttachment.resolveTarget);
    }

    // Gather the depth-stencil information.
    if (descriptor->depthStencilAttachment != nullptr) {
        TextureViewBase* attachment = descriptor->depthStencilAttachment->view;
        mDepthStencilFormat = attachment->GetFormat().format;
        if (mSampleCount == 0) {
            mSampleCount = attachment->GetTexture()->GetSampleCount();
        } else {
            DAWN_ASSERT(mSampleCount == attachment->GetTexture()->GetSampleCount());
        }
    }

    if (!mExpandResolveInfo.attachmentsToExpandResolve.any()) {
        // If render pass doesn't have any color attachment using ExpandResolveTexture load op then
        // ignore resolve targets. This is to relax compatibility requirement for common cases
        // where ExpandResolveTexture is not used.
        mExpandResolveInfo.resolveTargetsMask.reset();
    }
    DAWN_ASSERT(IsSubset(mExpandResolveInfo.attachmentsToExpandResolve,
                         mExpandResolveInfo.resolveTargetsMask));

    // Gather the PLS information.
    if (auto* pls = descriptor.Get<RenderPassPixelLocalStorage>()) {
        mHasPLS = true;
        mStorageAttachmentSlots = std::vector<wgpu::TextureFormat>(
            pls->totalPixelLocalStorageSize / kPLSSlotByteSize, wgpu::TextureFormat::Undefined);
        for (size_t i = 0; i < pls->storageAttachmentCount; i++) {
            size_t slot = pls->storageAttachments[i].offset / kPLSSlotByteSize;
            const TextureViewBase* attachment = pls->storageAttachments[i].storage;
            mStorageAttachmentSlots[slot] = attachment->GetFormat().format;

            if (mSampleCount == 0) {
                mSampleCount = attachment->GetTexture()->GetSampleCount();
            } else {
                DAWN_ASSERT(mSampleCount == attachment->GetTexture()->GetSampleCount());
            }
        }
    }

    DAWN_ASSERT(mSampleCount > 0);
    SetContentHash(ComputeContentHash());
}

AttachmentState::AttachmentState(const AttachmentState& blueprint)
    : ObjectBase(blueprint.GetDevice()) {
    mColorAttachmentsSet = blueprint.mColorAttachmentsSet;
    mColorFormats = blueprint.mColorFormats;
    mDepthStencilFormat = blueprint.mDepthStencilFormat;
    mSampleCount = blueprint.mSampleCount;
    mExpandResolveInfo = blueprint.mExpandResolveInfo;
    mHasPLS = blueprint.mHasPLS;
    mStorageAttachmentSlots = blueprint.mStorageAttachmentSlots;
    DAWN_ASSERT(IsSubset(mExpandResolveInfo.attachmentsToExpandResolve,
                         mExpandResolveInfo.resolveTargetsMask));
    SetContentHash(blueprint.GetContentHash());
}

void AttachmentState::DeleteThis() {
    Uncache();
    RefCounted::DeleteThis();
}

bool AttachmentState::EqualityFunc::operator()(const AttachmentState* a,
                                               const AttachmentState* b) const {
    // Check set attachments
    if (a->mColorAttachmentsSet != b->mColorAttachmentsSet) {
        return false;
    }

    // Check color formats
    for (auto i : IterateBitSet(a->mColorAttachmentsSet)) {
        if (a->mColorFormats[i] != b->mColorFormats[i]) {
            return false;
        }
    }

    // Check depth stencil format
    if (a->mDepthStencilFormat != b->mDepthStencilFormat) {
        return false;
    }

    // Check sample count
    if (a->mSampleCount != b->mSampleCount) {
        return false;
    }

    // Both attachment state must have the same `ExpandResolveTexture` load ops.
    if (a->mExpandResolveInfo.attachmentsToExpandResolve !=
        b->mExpandResolveInfo.attachmentsToExpandResolve) {
        return false;
    }

    if (a->mExpandResolveInfo.resolveTargetsMask != b->mExpandResolveInfo.resolveTargetsMask) {
        return false;
    }

    // Check PLS
    if (a->mHasPLS != b->mHasPLS) {
        return false;
    }
    if (a->mStorageAttachmentSlots.size() != b->mStorageAttachmentSlots.size()) {
        return false;
    }
    for (size_t i = 0; i < a->mStorageAttachmentSlots.size(); i++) {
        if (a->mStorageAttachmentSlots[i] != b->mStorageAttachmentSlots[i]) {
            return false;
        }
    }

    return true;
}

size_t AttachmentState::ComputeContentHash() {
    size_t hash = 0;

    // Hash color formats
    HashCombine(&hash, mColorAttachmentsSet);
    for (auto i : IterateBitSet(mColorAttachmentsSet)) {
        HashCombine(&hash, mColorFormats[i]);
    }

    // Hash depth stencil attachment
    HashCombine(&hash, mDepthStencilFormat);

    // Hash sample count
    HashCombine(&hash, mSampleCount);

    // Hash expand resolve load op bits
    HashCombine(&hash, mExpandResolveInfo.attachmentsToExpandResolve);
    HashCombine(&hash, mExpandResolveInfo.resolveTargetsMask);

    // Hash the PLS state
    HashCombine(&hash, mHasPLS);
    for (wgpu::TextureFormat slotFormat : mStorageAttachmentSlots) {
        HashCombine(&hash, slotFormat);
    }

    return hash;
}

ColorAttachmentMask AttachmentState::GetColorAttachmentsMask() const {
    return mColorAttachmentsSet;
}

wgpu::TextureFormat AttachmentState::GetColorAttachmentFormat(ColorAttachmentIndex index) const {
    DAWN_ASSERT(mColorAttachmentsSet[index]);
    return mColorFormats[index];
}

bool AttachmentState::HasDepthStencilAttachment() const {
    return mDepthStencilFormat != wgpu::TextureFormat::Undefined;
}

wgpu::TextureFormat AttachmentState::GetDepthStencilFormat() const {
    DAWN_ASSERT(HasDepthStencilAttachment());
    return mDepthStencilFormat;
}

uint32_t AttachmentState::GetSampleCount() const {
    return mSampleCount;
}

const AttachmentState::ExpandResolveInfo& AttachmentState::GetExpandResolveInfo() const {
    return mExpandResolveInfo;
}

bool AttachmentState::HasPixelLocalStorage() const {
    return mHasPLS;
}

const std::vector<wgpu::TextureFormat>& AttachmentState::GetStorageAttachmentSlots() const {
    return mStorageAttachmentSlots;
}

std::vector<ColorAttachmentIndex>
AttachmentState::ComputeStorageAttachmentPackingInColorAttachments() const {
    // TODO(dawn:1704): Consider caching this on AttachmentState creation, but does it become part
    // of the hashing and comparison operators? Fill with garbage data to more easily detect cases
    // where an incorrect slot is accessed.
    std::vector<ColorAttachmentIndex> result(mStorageAttachmentSlots.size(),
                                             ityp::PlusOne(kMaxColorAttachmentsTyped));

    // Iterate over the empty bits of mColorAttachmentsSet to pack storage attachment in them.
    auto availableSlots = ~mColorAttachmentsSet;
    for (size_t i = 0; i < mStorageAttachmentSlots.size(); i++) {
        DAWN_ASSERT(!availableSlots.none());
        auto slot = ColorAttachmentIndex(uint8_t(ScanForward(availableSlots.to_ulong())));
        availableSlots.reset(slot);
        result[i] = slot;
    }

    return result;
}

}  // namespace dawn::native
