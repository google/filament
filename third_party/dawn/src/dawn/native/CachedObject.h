// Copyright 2019 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_CACHEDOBJECT_H_
#define SRC_DAWN_NATIVE_CACHEDOBJECT_H_

#include <cstddef>
#include <string>

#include "dawn/native/CacheKey.h"
#include "dawn/native/Forward.h"

namespace dawn::native {

// Some objects are cached so that instead of creating new duplicate objects, we increase the
// refcount of an existing object.
class CachedObject {
  public:
    // Functor necessary for the unordered_set<CachedObject*>-based cache.
    struct HashFunc {
        size_t operator()(const CachedObject* obj) const;
    };

    size_t GetContentHash() const;
    void SetContentHash(size_t contentHash);

    // Returns the cache key for the object only, i.e. without device/adapter information.
    const CacheKey& GetCacheKey() const;

  protected:
    // Cache key member is protected so that derived classes can modify it.
    CacheKey mCacheKey;

  private:
    // Called by ObjectContentHasher upon creation to record the object.
    virtual size_t ComputeContentHash() = 0;

    size_t mContentHash = 0;
    bool mIsContentHashInitialized = false;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_CACHEDOBJECT_H_
