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

#include <vector>

#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"
#include "dawn/native/ObjectBase.h"

#include "dawn/native/dawn_platform.h"

namespace dawn::native {

MaybeError ValidateQuerySetDescriptor(DeviceBase* device, const QuerySetDescriptor* descriptor);

class QuerySetBase : public ApiObjectBase {
  public:
    static Ref<QuerySetBase> MakeError(DeviceBase* device, const QuerySetDescriptor* descriptor);

    ObjectType GetType() const override;

    wgpu::QueryType GetQueryType() const;
    uint32_t GetQueryCount() const;

    const std::vector<bool>& GetQueryAvailability() const;
    void SetQueryAvailability(uint32_t index, bool available);

    MaybeError ValidateCanUseInSubmitNow() const;

    void APIDestroy();
    wgpu::QueryType APIGetType() const;
    uint32_t APIGetCount() const;

  protected:
    QuerySetBase(DeviceBase* device, const QuerySetDescriptor* descriptor);
    QuerySetBase(DeviceBase* device,
                 const QuerySetDescriptor* descriptor,
                 ObjectBase::ErrorTag tag);

    void DestroyImpl() override;

    ~QuerySetBase() override;

  private:
    wgpu::QueryType mQueryType;
    uint32_t mQueryCount;

    enum class QuerySetState { Unavailable, Available, Destroyed };
    QuerySetState mState = QuerySetState::Unavailable;

    // Indicates the available queries on the query set for resolving
    std::vector<bool> mQueryAvailability;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_QUERYSET_H_
