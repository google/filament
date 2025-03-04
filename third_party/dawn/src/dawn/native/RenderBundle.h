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

#ifndef SRC_DAWN_NATIVE_RENDERBUNDLE_H_
#define SRC_DAWN_NATIVE_RENDERBUNDLE_H_

#include <bitset>
#include <string>

#include "dawn/common/Constants.h"
#include "dawn/native/AttachmentState.h"
#include "dawn/native/CommandAllocator.h"
#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"
#include "dawn/native/IndirectDrawMetadata.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/PassResourceUsage.h"

#include "dawn/native/dawn_platform.h"

namespace dawn::native {

struct RenderBundleDescriptor;
class RenderBundleEncoder;

class RenderBundleBase final : public ApiObjectBase {
  public:
    RenderBundleBase(RenderBundleEncoder* encoder,
                     const RenderBundleDescriptor* descriptor,
                     Ref<AttachmentState> attachmentState,
                     bool depthReadOnly,
                     bool stencilReadOnly,
                     RenderPassResourceUsage resourceUsage,
                     IndirectDrawMetadata indirectDrawMetadata);

    static Ref<RenderBundleBase> MakeError(DeviceBase* device, StringView label);

    ObjectType GetType() const override;
    void FormatLabel(absl::FormatSink* s) const override;

    const std::string& GetEncoderLabel() const;
    void SetEncoderLabel(std::string encoderLabel);

    CommandIterator* GetCommands();

    const AttachmentState* GetAttachmentState() const;
    bool IsDepthReadOnly() const;
    bool IsStencilReadOnly() const;
    uint64_t GetDrawCount() const;
    const RenderPassResourceUsage& GetResourceUsage() const;
    const IndirectDrawMetadata& GetIndirectDrawMetadata();

  private:
    RenderBundleBase(DeviceBase* device, ErrorTag errorTag, StringView label);

    void DestroyImpl() override;

    CommandIterator mCommands;
    IndirectDrawMetadata mIndirectDrawMetadata;
    Ref<AttachmentState> mAttachmentState;
    bool mDepthReadOnly;
    bool mStencilReadOnly;
    uint64_t mDrawCount;
    RenderPassResourceUsage mResourceUsage;
    std::string mEncoderLabel;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_RENDERBUNDLE_H_
