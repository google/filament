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

#include "dawn/native/d3d11/QuerySetD3D11.h"

#include <utility>

#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d11/BufferD3D11.h"
#include "dawn/native/d3d11/CommandRecordingContextD3D11.h"
#include "dawn/native/d3d11/DeviceD3D11.h"
#include "dawn/native/d3d11/UtilsD3D11.h"

namespace dawn::native::d3d11 {

ResultOrError<Ref<QuerySet>> QuerySet::Create(Device* device,
                                              const QuerySetDescriptor* descriptor) {
    Ref<QuerySet> querySet = AcquireRef(new QuerySet(device, descriptor));
    DAWN_TRY(querySet->Initialize());
    return querySet;
}

MaybeError QuerySet::Initialize() {
    DAWN_ASSERT(GetQueryType() == wgpu::QueryType::Occlusion);
    D3D11_QUERY_DESC queryDesc = {};
    queryDesc.Query = D3D11_QUERY_OCCLUSION_PREDICATE;

    for (uint32_t i = 0; i < GetQueryCount(); ++i) {
        ComPtr<ID3D11Predicate> d3d11Predicate;
        DAWN_TRY(CheckHRESULT(
            ToBackend(GetDevice())->GetD3D11Device()->CreatePredicate(&queryDesc, &d3d11Predicate),
            "ID3D11Device::CreateQuery"));
        mPredicates.push_back(std::move(d3d11Predicate));
    }
    SetLabelImpl();
    return {};
}

void QuerySet::DestroyImpl() {
    // TODO(crbug.com/dawn/831): DestroyImpl is called from two places.
    // - It may be called if the query set is explicitly destroyed with APIDestroy.
    //   This case is NOT thread-safe and needs proper synchronization with other
    //   simultaneous uses of the query set.
    // - It may be called when the last ref to the query set is dropped and it
    //   is implicitly destroyed. This case is thread-safe because there are no
    //   other threads using the query set since there are no other live refs.
    QuerySetBase::DestroyImpl();
    mPredicates.clear();
}

void QuerySet::SetLabelImpl() {
    for (const auto& predicate : mPredicates) {
        SetDebugName(ToBackend(GetDevice()), predicate.Get(), "Dawn_QuerySet", GetLabel());
    }
}

void QuerySet::BeginQuery(ID3D11DeviceContext* d3d11DeviceContext, uint32_t query) {
    d3d11DeviceContext->Begin(mPredicates[query].Get());
}

void QuerySet::EndQuery(ID3D11DeviceContext* d3d11DeviceContext, uint32_t query) {
    d3d11DeviceContext->End(mPredicates[query].Get());
}

MaybeError QuerySet::Resolve(const ScopedSwapStateCommandRecordingContext* commandContext,
                             uint32_t firstQuery,
                             uint32_t queryCount,
                             Buffer* destination,
                             uint64_t offset) {
    DAWN_TRY(destination->Clear(commandContext, 0, offset, queryCount * sizeof(uint64_t)));
    const auto& queryAvailability = GetQueryAvailability();
    for (uint32_t i = 0; i < queryCount; ++i) {
        uint32_t queryIndex = i + firstQuery;
        if (queryAvailability[queryIndex]) {
            auto& predicate = mPredicates[queryIndex];
            DAWN_TRY(destination->PredicatedClear(commandContext, predicate.Get(), 1,
                                                  offset + i * sizeof(uint64_t), sizeof(uint64_t)));
        }
    }
    return {};
}

}  // namespace dawn::native::d3d11
