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

#ifndef SRC_DAWN_NATIVE_PIPELINECACHE_H_
#define SRC_DAWN_NATIVE_PIPELINECACHE_H_

#include <atomic>

#include "dawn/common/RefCounted.h"
#include "dawn/native/BlobCache.h"
#include "dawn/native/CacheKey.h"
#include "dawn/native/Error.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native {

// Abstraction layer for backend dependent pipeline caching.
class PipelineCacheBase : public RefCounted {
  public:
    // Returns whether or not we got a cache hit when initializing.
    bool CacheHit() const;

    // Serializes and writes the current contents of the backend cache object into the backing
    // blob cache, potentially overwriting what is already there. Useful when we are working
    // with more monolithic-like caches where we expect overwriting sometimes.
    MaybeError Flush();

    // Called after pipeline was compiled. The default implementation serializes and writes the
    // current contents of the backend cache object into the backing blob cache iff the initial read
    // from the backend cache did not result in a hit.
    MaybeError DidCompilePipeline();

    // Trigger storing pipeline cache data in BlobCache if necessary.
    MaybeError StoreOnIdle();

  protected:
    // If `storeOnIdle` is true then pipeline cache will only stored in BlobCache when
    // StoreOnIdle() is called.
    PipelineCacheBase(BlobCache* cache, const CacheKey& key, bool storeOnIdle);

    // Initializes and returns the cached blob given the cache and keys. Used by backend
    // implementations to get the cache and set the cache hit state. Should only be called once.
    Blob Initialize();

  private:
    // Backend implementation of serialization of the cache into a blob.
    // Note: given that no local cached blob should be destructed and copy elision has strict
    // requirement cached blob is passed in as a pointer to be assigned.
    virtual MaybeError SerializeToBlobImpl(Blob* blob) = 0;

    // The blob cache is owned by the Adapter and pipeline caches are owned/created by devices
    // or adapters. Since the device owns a reference to the Instance which owns the Adapter,
    // the blob cache is guaranteed to be valid throughout the lifetime of the object.
    const raw_ptr<BlobCache> mCache;
    const CacheKey mKey;
    const bool mStoreOnIdle;
    bool mInitialized = false;
    bool mCacheHit = false;

    // Multiple threads can be using the pipeline cache concurrently and
    // modifying this variable. Loads and stores are done with relaxed ordering
    // since we don't care so much about strict ordering just avoiding UB from
    // concurrent read/writes.
    std::atomic<bool> mNeedsStore = false;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_PIPELINECACHE_H_
