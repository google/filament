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

#include "src/dawn/native/webgpu/QuerySetWGPU.h"

#include <webgpu/webgpu.h>

#include "dawn/common/StringViewUtils.h"
#include "src/dawn/native/webgpu/DeviceWGPU.h"
#include "src/dawn/native/webgpu/ToWGPU.h"

namespace dawn::native::webgpu {

// static
Ref<QuerySet> QuerySet::Create(Device* device, const QuerySetDescriptor* descriptor) {
    return AcquireRef(new QuerySet(device, descriptor));
}

QuerySet::QuerySet(Device* device, const QuerySetDescriptor* descriptor)
    : QuerySetBase(device, descriptor),
      RecordableObject(schema::ObjectType::QuerySet),
      ObjectWGPU(device->wgpu->querySetRelease) {
    WGPUQuerySetDescriptor desc = WGPU_QUERY_SET_DESCRIPTOR_INIT;
    desc.label = dawn::ToOutputStringView(descriptor->label);
    desc.type = ToAPI(descriptor->type);
    desc.count = descriptor->count;

    mInnerHandle = device->wgpu->deviceCreateQuerySet(device->GetInnerHandle(), &desc);
}

QuerySet::~QuerySet() = default;

void QuerySet::DestroyImpl(DestroyReason reason) {
    Device* device = ToBackend(GetDevice());
    device->wgpu->querySetDestroy(mInnerHandle);
    mInnerHandle = nullptr;
    QuerySetBase::DestroyImpl(reason);
}

void QuerySet::SetLabelImpl() {
    ToBackend(GetDevice())->CaptureSetLabel(this, GetLabel());
}

MaybeError QuerySet::AddReferenced(CaptureContext& captureContext) {
    // QuerySet do not reference other objects.
    return {};
}

MaybeError QuerySet::CaptureCreationParameters(CaptureContext& captureContext) {
    schema::QuerySet querySet{{
        .type = GetQueryType(),
        .count = GetQueryCount(),
    }};
    Serialize(captureContext, querySet);
    return {};
}

}  // namespace dawn::native::webgpu
