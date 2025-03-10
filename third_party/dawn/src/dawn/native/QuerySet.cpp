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

#include "dawn/native/QuerySet.h"

#include <set>

#include "dawn/native/Device.h"
#include "dawn/native/Features.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/ValidationUtils_autogen.h"

namespace dawn::native {

namespace {

class ErrorQuerySet final : public QuerySetBase {
  public:
    explicit ErrorQuerySet(DeviceBase* device, const QuerySetDescriptor* descriptor)
        : QuerySetBase(device, descriptor, ObjectBase::kError) {}

  private:
    void DestroyImpl() override { DAWN_UNREACHABLE(); }
};

}  // anonymous namespace

MaybeError ValidateQuerySetDescriptor(DeviceBase* device, const QuerySetDescriptor* descriptor) {
    DAWN_INVALID_IF(descriptor->nextInChain != nullptr, "nextInChain must be nullptr");

    DAWN_TRY(ValidateQueryType(descriptor->type));

    DAWN_INVALID_IF(descriptor->count > kMaxQueryCount,
                    "Query count (%u) exceeds the maximum query count (%u).", descriptor->count,
                    kMaxQueryCount);

    switch (descriptor->type) {
        case wgpu::QueryType::Occlusion:
            break;

        case wgpu::QueryType::Timestamp:
            DAWN_INVALID_IF(
                !device->HasFeature(Feature::TimestampQuery) &&
                    !device->HasFeature(Feature::ChromiumExperimentalTimestampQueryInsidePasses),
                "Timestamp query set created without the feature being enabled.");
            break;

        default:
            break;
    }

    return {};
}

QuerySetBase::QuerySetBase(DeviceBase* device, const QuerySetDescriptor* descriptor)
    : ApiObjectBase(device, descriptor->label),
      mQueryType(descriptor->type),
      mQueryCount(descriptor->count),
      mState(QuerySetState::Available) {
    mQueryAvailability.resize(descriptor->count);
    GetObjectTrackingList()->Track(this);
}

QuerySetBase::QuerySetBase(DeviceBase* device,
                           const QuerySetDescriptor* descriptor,
                           ObjectBase::ErrorTag tag)
    : ApiObjectBase(device, tag, descriptor->label),
      mQueryType(descriptor->type),
      mQueryCount(descriptor->count) {}

QuerySetBase::~QuerySetBase() {
    // Uninitialized or already destroyed
    DAWN_ASSERT(mState == QuerySetState::Unavailable || mState == QuerySetState::Destroyed);
}

void QuerySetBase::DestroyImpl() {
    // TODO(crbug.com/dawn/831): DestroyImpl is called from two places.
    // - It may be called if the query set is explicitly destroyed with APIDestroy.
    //   This case is NOT thread-safe and needs proper synchronization with other
    //   simultaneous uses of the query set.
    // - It may be called when the last ref to the query set is dropped and it
    //   is implicitly destroyed. This case is thread-safe because there are no
    //   other threads using the query set since there are no other live refs.
    mState = QuerySetState::Destroyed;
}

// static
Ref<QuerySetBase> QuerySetBase::MakeError(DeviceBase* device,
                                          const QuerySetDescriptor* descriptor) {
    return AcquireRef(new ErrorQuerySet(device, descriptor));
}

ObjectType QuerySetBase::GetType() const {
    return ObjectType::QuerySet;
}

wgpu::QueryType QuerySetBase::GetQueryType() const {
    return mQueryType;
}

uint32_t QuerySetBase::GetQueryCount() const {
    return mQueryCount;
}

const std::vector<bool>& QuerySetBase::GetQueryAvailability() const {
    return mQueryAvailability;
}

void QuerySetBase::SetQueryAvailability(uint32_t index, bool available) {
    mQueryAvailability[index] = available;
}

MaybeError QuerySetBase::ValidateCanUseInSubmitNow() const {
    DAWN_ASSERT(!IsError());
    DAWN_INVALID_IF(mState == QuerySetState::Destroyed, "%s used while destroyed.", this);
    return {};
}

void QuerySetBase::APIDestroy() {
    Destroy();
}

wgpu::QueryType QuerySetBase::APIGetType() const {
    return mQueryType;
}

uint32_t QuerySetBase::APIGetCount() const {
    return mQueryCount;
}

}  // namespace dawn::native
