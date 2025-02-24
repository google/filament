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

#ifndef SRC_DAWN_NATIVE_BINDGROUPLAYOUT_H_
#define SRC_DAWN_NATIVE_BINDGROUPLAYOUT_H_

#include <string>

#include "dawn/native/BindGroupLayoutInternal.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/ObjectBase.h"

namespace dawn::native {

// Wrapper passthrough frontend object that is essentially just a Ref to a backing
// BindGroupLayoutInternalBase and a pipeline compatibility token.
class BindGroupLayoutBase final : public ApiObjectBase {
  public:
    BindGroupLayoutBase(DeviceBase* device,
                        StringView label,
                        Ref<BindGroupLayoutInternalBase> internal,
                        PipelineCompatibilityToken pipelineCompatibilityToken);

    static Ref<BindGroupLayoutBase> MakeError(DeviceBase* device, StringView label = {});

    ObjectType GetType() const override;

    // Non-proxy functions that are specific to the realized frontend object.
    BindGroupLayoutInternalBase* GetInternalBindGroupLayout() const;
    bool IsLayoutEqual(const BindGroupLayoutBase* other,
                       bool excludePipelineCompatibiltyToken = false) const;
    PipelineCompatibilityToken GetPipelineCompatibilityToken() const {
        return mPipelineCompatibilityToken;
    }

    bool IsEmpty() const;

  protected:
    void DestroyImpl() override;

  private:
    BindGroupLayoutBase(DeviceBase* device, ObjectBase::ErrorTag tag, StringView label);

    const Ref<BindGroupLayoutInternalBase> mInternalLayout;

    // Non-0 if this BindGroupLayout was created as part of a default PipelineLayout.
    const PipelineCompatibilityToken mPipelineCompatibilityToken = PipelineCompatibilityToken(0);
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_BINDGROUPLAYOUT_H_
