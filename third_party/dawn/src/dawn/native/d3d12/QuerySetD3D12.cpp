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

#include "dawn/native/d3d12/QuerySetD3D12.h"

#include <algorithm>

#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/UtilsD3D12.h"

namespace dawn::native::d3d12 {

namespace {
D3D12_QUERY_HEAP_TYPE D3D12QueryHeapType(wgpu::QueryType type) {
    switch (type) {
        case wgpu::QueryType::Occlusion:
            return D3D12_QUERY_HEAP_TYPE_OCCLUSION;
        case wgpu::QueryType::Timestamp:
            return D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
    }
}
}  // anonymous namespace

// static
ResultOrError<Ref<QuerySet>> QuerySet::Create(Device* device,
                                              const QuerySetDescriptor* descriptor) {
    Ref<QuerySet> querySet = AcquireRef(new QuerySet(device, descriptor));
    DAWN_TRY(querySet->Initialize());
    return querySet;
}

MaybeError QuerySet::Initialize() {
    D3D12_QUERY_HEAP_DESC queryHeapDesc = {};
    queryHeapDesc.Type = D3D12QueryHeapType(GetQueryType());
    queryHeapDesc.Count = std::max(GetQueryCount(), uint32_t(1u));

    ID3D12Device* d3d12Device = ToBackend(GetDevice())->GetD3D12Device();
    DAWN_TRY(CheckOutOfMemoryHRESULT(
        d3d12Device->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(&mQueryHeap)),
        "ID3D12Device::CreateQueryHeap"));

    SetLabelImpl();

    return {};
}

ID3D12QueryHeap* QuerySet::GetQueryHeap() const {
    return mQueryHeap.Get();
}

QuerySet::~QuerySet() = default;

void QuerySet::DestroyImpl() {
    // TODO(crbug.com/dawn/831): DestroyImpl is called from two places.
    // - It may be called if the query set is explicitly destroyed with APIDestroy.
    //   This case is NOT thread-safe and needs proper synchronization with other
    //   simultaneous uses of the query set.
    // - It may be called when the last ref to the query set is dropped and it
    //   is implicitly destroyed. This case is thread-safe because there are no
    //   other threads using the query set since there are no other live refs.
    QuerySetBase::DestroyImpl();
    ToBackend(GetDevice())->ReferenceUntilUnused(mQueryHeap);
    mQueryHeap = nullptr;
}

void QuerySet::SetLabelImpl() {
    SetDebugName(ToBackend(GetDevice()), mQueryHeap.Get(), "Dawn_QuerySet", GetLabel());
}

}  // namespace dawn::native::d3d12
