// Copyright 2022 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_CACHERESULT_H_
#define SRC_DAWN_NATIVE_CACHERESULT_H_

#include <memory>
#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/native/CacheKey.h"

namespace dawn::native {

template <typename T>
class CacheResult {
  public:
    static CacheResult CacheHit(CacheKey key, T value) {
        return CacheResult(std::move(key), std::move(value), true);
    }

    static CacheResult CacheMiss(CacheKey key, T value) {
        return CacheResult(std::move(key), std::move(value), false);
    }

    CacheResult() : mKey(), mValue(), mIsCached(false), mIsValid(false) {}

    bool IsCached() const {
        DAWN_ASSERT(mIsValid);
        return mIsCached;
    }
    const CacheKey& GetCacheKey() const {
        DAWN_ASSERT(mIsValid);
        return mKey;
    }

    // Note: Getting mValue is always const, since mutating it would invalidate consistency with
    // mKey.
    const T* operator->() const {
        DAWN_ASSERT(mIsValid);
        return &mValue;
    }
    const T& operator*() const {
        DAWN_ASSERT(mIsValid);
        return mValue;
    }

    T Acquire() {
        DAWN_ASSERT(mIsValid);
        mIsValid = false;
        return std::move(mValue);
    }

  private:
    CacheResult(CacheKey key, T value, bool isCached)
        : mKey(std::move(key)), mValue(std::move(value)), mIsCached(isCached), mIsValid(true) {}

    CacheKey mKey;
    T mValue;
    bool mIsCached;
    bool mIsValid;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_CACHERESULT_H_
