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

#ifndef SRC_DAWN_NATIVE_QUERYSET_H_
#define SRC_DAWN_NATIVE_QUERYSET_H_

#include "src/dawn/common/ityp_vector.h"
#include "src/dawn/native/Error.h"
#include "src/dawn/native/Forward.h"
#include "src/dawn/native/IntegerTypes.h"
#include "src/dawn/native/ObjectBase.h"

namespace dawn::native {

MaybeError ValidateQuerySetDescriptor(DeviceBase* device, const QuerySetDescriptor* descriptor);

uint32_t ToQueryStorageSize(QueryIndex count);
inline constexpr uint32_t kSingleQueryStorageSize = 8;  // size of a uint64_t

class QuerySetBase : public ApiObjectBase {
  public:
    static Ref<QuerySetBase> MakeError(DeviceBase* device, const QuerySetDescriptor* descriptor);

    ObjectType GetType() const override;

    wgpu::QueryType GetQueryType() const;
    QueryIndex GetQueryCount() const;

    bool IsQueryAvailable(QueryIndex index) const;
    bool AreAllQueriesAvailable(QueryIndex first, QueryIndex count) const;
    void MarkQueryAvailable(QueryIndex index);

    MaybeError ValidateCanUseInSubmitNow() const;

    // Dawn API
    void APIDestroy();
    wgpu::QueryType APIGetType() const;
    uint32_t APIGetCount() const;

  protected:
    QuerySetBase(DeviceBase* device, const QuerySetDescriptor* descriptor);
    QuerySetBase(DeviceBase* device,
                 const QuerySetDescriptor* descriptor,
                 ObjectBase::ErrorTag tag);

    void DestroyImpl(DestroyReason reason) override;

    ~QuerySetBase() override;

  private:
    wgpu::QueryType mQueryType = static_cast<wgpu::QueryType>(0);
    QueryIndex mQueryCount = QueryIndex(0u);

    enum class QuerySetState { Unavailable, Available, Destroyed };
    QuerySetState mState = QuerySetState::Unavailable;

    // Indicates the available queries on the query set for resolving
    ityp::vector<QueryIndex, bool> mQueryAvailability;
};

// Helper function to help walk ranges of available queries to do bulk operations on therm.
// For each range of available queries in [start, start + count) as denoted by the predicate
// IsQueryAvailable, call DoOnRange(startOfRange, sizeOfRange).
template <typename IsAvailableT, typename DoOnRangeT>
void ForEachAvailableQueryRange(QueryIndex start,
                                QueryIndex count,
                                IsAvailableT IsAvailable,
                                DoOnRangeT DoOnRange) {
    QueryIndex current = start;
    QueryIndex end = start + count;
    while (current < end) {
        if (!IsAvailable(current)) {
            current++;
            continue;
        }

        QueryIndex firstAvailable = current;
        while (current < end && IsAvailable(current)) {
            current++;
        }
        QueryIndex availableCount = current - firstAvailable;

        DoOnRange(firstAvailable, availableCount);
    }
}

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_QUERYSET_H_
