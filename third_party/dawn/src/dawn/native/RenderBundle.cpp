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

#include "dawn/native/RenderBundle.h"

#include <utility>

#include "absl/strings/str_format.h"
#include "dawn/common/BitSetIterator.h"
#include "dawn/native/Commands.h"
#include "dawn/native/Device.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/RenderBundleEncoder.h"

namespace dawn::native {

RenderBundleBase::RenderBundleBase(RenderBundleEncoder* encoder,
                                   const RenderBundleDescriptor* descriptor,
                                   Ref<AttachmentState> attachmentState,
                                   bool depthReadOnly,
                                   bool stencilReadOnly,
                                   RenderPassResourceUsage resourceUsage,
                                   IndirectDrawMetadata indirectDrawMetadata)
    : ApiObjectBase(encoder->GetDevice(), kLabelNotImplemented),
      mCommands(encoder->AcquireCommands()),
      mIndirectDrawMetadata(std::move(indirectDrawMetadata)),
      mAttachmentState(std::move(attachmentState)),
      mDepthReadOnly(depthReadOnly),
      mStencilReadOnly(stencilReadOnly),
      mDrawCount(encoder->GetDrawCount()),
      mResourceUsage(std::move(resourceUsage)),
      mEncoderLabel(encoder->GetLabel()) {
    GetObjectTrackingList()->Track(this);
}

void RenderBundleBase::DestroyImpl() {
    mIndirectDrawMetadata.ClearIndexedIndirectBufferValidationInfo();
    FreeCommands(&mCommands);

    // Remove reference to the attachment state so that we don't have lingering references to
    // it preventing it from being uncached in the device.
    mAttachmentState = nullptr;
}

// static
Ref<RenderBundleBase> RenderBundleBase::MakeError(DeviceBase* device, StringView label) {
    return AcquireRef(new RenderBundleBase(device, ObjectBase::kError, label));
}

RenderBundleBase::RenderBundleBase(DeviceBase* device, ErrorTag errorTag, StringView label)
    : ApiObjectBase(device, errorTag, label), mIndirectDrawMetadata(device->GetLimits()) {}

ObjectType RenderBundleBase::GetType() const {
    return ObjectType::RenderBundle;
}

void RenderBundleBase::FormatLabel(absl::FormatSink* s) const {
    s->Append(ObjectTypeAsString(GetType()));

    const std::string& label = GetLabel();
    if (!label.empty()) {
        s->Append(absl::StrFormat(" \"%s\"", label));
    }

    if (!mEncoderLabel.empty()) {
        s->Append(absl::StrFormat(
            " from %s \"%s\"", ObjectTypeAsString(ObjectType::RenderBundleEncoder), mEncoderLabel));
    }
}

const std::string& RenderBundleBase::GetEncoderLabel() const {
    return mEncoderLabel;
}

void RenderBundleBase::SetEncoderLabel(std::string encoderLabel) {
    mEncoderLabel = encoderLabel;
}

CommandIterator* RenderBundleBase::GetCommands() {
    return &mCommands;
}

const AttachmentState* RenderBundleBase::GetAttachmentState() const {
    DAWN_ASSERT(!IsError());
    return mAttachmentState.Get();
}

bool RenderBundleBase::IsDepthReadOnly() const {
    DAWN_ASSERT(!IsError());
    return mDepthReadOnly;
}

bool RenderBundleBase::IsStencilReadOnly() const {
    DAWN_ASSERT(!IsError());
    return mStencilReadOnly;
}

uint64_t RenderBundleBase::GetDrawCount() const {
    DAWN_ASSERT(!IsError());
    return mDrawCount;
}

const RenderPassResourceUsage& RenderBundleBase::GetResourceUsage() const {
    DAWN_ASSERT(!IsError());
    return mResourceUsage;
}

const IndirectDrawMetadata& RenderBundleBase::GetIndirectDrawMetadata() {
    return mIndirectDrawMetadata;
}

}  // namespace dawn::native
