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

#ifndef SRC_DAWN_COMMON_REFCOUNTEDWITHEXTERNALCOUNT_H_
#define SRC_DAWN_COMMON_REFCOUNTEDWITHEXTERNALCOUNT_H_

#include "dawn/common/RefCounted.h"

namespace dawn {

// RecCountedWithExternalCountBase is a version of RefCounted which tracks a separate
// refcount for calls to APIAddRef/APIRelease (refs added/removed by the application).
// The external refcount starts at 0, and the total refcount starts at 1 - i.e. the first
// ref isn't an external ref.
// When the external refcount drops to zero, WillDropLastExternalRef is called. and it can be called
// more than once.
// The derived class must override the behavior of WillDropLastExternalRef.
template <typename T>
class RefCountedWithExternalCount : public T {
  public:
    static constexpr bool HasExternalRefCount = true;

    using T::T;

    void APIAddRef() {
        IncrementExternalRefCount();
        T::APIAddRef();
    }

    void APIRelease() {
        if (mExternalRefCount.Decrement()) {
            WillDropLastExternalRef();
        }
        T::APIRelease();
    }

    void IncrementExternalRefCount() { mExternalRefCount.Increment(); }

    uint64_t GetExternalRefCountForTesting() const {
        return mExternalRefCount.GetValueForTesting();
    }

  private:
    virtual void WillDropLastExternalRef() = 0;

    RefCount mExternalRefCount{/*initCount=*/0, /*payload=*/0};
};

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_REFCOUNTEDWITHEXTERNALCOUNT_H_
