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

#include "dawn/common/WeakRefSupport.h"

#include <utility>

namespace dawn::detail {

WeakRefData::WeakRefData(RefCounted* value) : mValue(value) {}

Ref<RefCounted> WeakRefData::TryGetRef() {
    std::lock_guard<std::mutex> lock(mMutex);
    if (!mValue || !mValue->mRefCount.TryIncrement()) {
        return nullptr;
    }
    return AcquireRef(mValue.get());
}

RefCounted* WeakRefData::UnsafeGet() const {
    return mValue;
}

void WeakRefData::Invalidate() {
    std::lock_guard<std::mutex> lock(mMutex);
    mValue = nullptr;
}

WeakRefSupportBase::WeakRefSupportBase(Ref<detail::WeakRefData> data) : mData(std::move(data)) {}

WeakRefSupportBase::~WeakRefSupportBase() {
    mData->Invalidate();
}

}  // namespace dawn::detail
