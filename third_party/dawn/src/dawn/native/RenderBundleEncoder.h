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

#ifndef SRC_DAWN_NATIVE_RENDERBUNDLEENCODER_H_
#define SRC_DAWN_NATIVE_RENDERBUNDLEENCODER_H_

#include "dawn/native/EncodingContext.h"
#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"
#include "dawn/native/RenderBundle.h"
#include "dawn/native/RenderEncoderBase.h"

namespace dawn::native {

MaybeError ValidateRenderBundleEncoderDescriptor(DeviceBase* device,
                                                 const RenderBundleEncoderDescriptor* descriptor);

class RenderBundleEncoder final : public RenderEncoderBase {
  public:
    static Ref<RenderBundleEncoder> Create(DeviceBase* device,
                                           const RenderBundleEncoderDescriptor* descriptor);
    static Ref<RenderBundleEncoder> MakeError(DeviceBase* device, StringView label);

    ~RenderBundleEncoder() override;

    ObjectType GetType() const override;

    RenderBundleBase* APIFinish(const RenderBundleDescriptor* descriptor);

    CommandIterator AcquireCommands();

  private:
    RenderBundleEncoder(DeviceBase* device, const RenderBundleEncoderDescriptor* descriptor);
    RenderBundleEncoder(DeviceBase* device, ErrorTag errorTag, StringView label);

    void DestroyImpl() override;

    ResultOrError<Ref<RenderBundleBase>> Finish(const RenderBundleDescriptor* descriptor);
    MaybeError ValidateFinish(const RenderPassResourceUsage& usages) const;

    EncodingContext mBundleEncodingContext;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_RENDERBUNDLEENCODER_H_
