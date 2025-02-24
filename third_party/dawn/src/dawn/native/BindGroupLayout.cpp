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

#include "dawn/native/BindGroupLayout.h"

#include "dawn/native/ObjectType_autogen.h"

namespace dawn::native {

BindGroupLayoutBase::BindGroupLayoutBase(DeviceBase* device,
                                         StringView label,
                                         Ref<BindGroupLayoutInternalBase> internal,
                                         PipelineCompatibilityToken pipelineCompatibilityToken)
    : ApiObjectBase(device, label),
      mInternalLayout(internal),
      mPipelineCompatibilityToken(pipelineCompatibilityToken) {
    GetObjectTrackingList()->Track(this);
}

BindGroupLayoutBase::BindGroupLayoutBase(DeviceBase* device,
                                         ObjectBase::ErrorTag tag,
                                         StringView label)
    : ApiObjectBase(device, tag, label) {}

ObjectType BindGroupLayoutBase::GetType() const {
    return ObjectType::BindGroupLayout;
}

bool BindGroupLayoutBase::IsEmpty() const {
    return mInternalLayout == nullptr || mInternalLayout->IsEmpty();
}

// static
Ref<BindGroupLayoutBase> BindGroupLayoutBase::MakeError(DeviceBase* device, StringView label) {
    return AcquireRef(new BindGroupLayoutBase(device, ObjectBase::kError, label));
}

BindGroupLayoutInternalBase* BindGroupLayoutBase::GetInternalBindGroupLayout() const {
    return mInternalLayout.Get();
}

bool BindGroupLayoutBase::IsLayoutEqual(const BindGroupLayoutBase* other,
                                        bool excludePipelineCompatibiltyToken) const {
    if (!excludePipelineCompatibiltyToken &&
        GetPipelineCompatibilityToken() != other->GetPipelineCompatibilityToken()) {
        return false;
    }
    return GetInternalBindGroupLayout() == other->GetInternalBindGroupLayout();
}

void BindGroupLayoutBase::DestroyImpl() {}

}  // namespace dawn::native
