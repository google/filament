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

#ifndef SRC_DAWN_NATIVE_BLOBCACHE_H_
#define SRC_DAWN_NATIVE_BLOBCACHE_H_

#include <mutex>

#include "dawn/common/Platform.h"
#include "dawn/native/Blob.h"
#include "dawn/native/CacheResult.h"
#include "partition_alloc/pointers/raw_ptr_exclusion.h"

namespace dawn::platform {
class CachingInterface;
}

namespace dawn::native {

class CacheKey;
class InstanceBase;

// This class should always be thread-safe because it may be called asynchronously.
class BlobCache {
  public:
    explicit BlobCache(const dawn::native::DawnCacheDeviceDescriptor& desc);

    // Returns empty blob if the key is not found in the cache.
    Blob Load(const CacheKey& key);

    // Value to store must be non-empty/non-null.
    void Store(const CacheKey& key, size_t valueSize, const void* value);
    void Store(const CacheKey& key, const Blob& value);

    // Store a CacheResult into the cache if it isn't cached yet.
    // Calls T::ToBlob which should be defined elsewhere.
    template <typename T>
    void EnsureStored(const CacheResult<T>& cacheResult) {
        if (!cacheResult.IsCached()) {
            Store(cacheResult.GetCacheKey(), cacheResult->ToBlob());
        }
    }

  private:
    // Non-thread safe internal implementations of load and store. Exposed callers that use
    // these helpers need to make sure that these are entered with `mMutex` held.
    Blob LoadInternal(const CacheKey& key);
    void StoreInternal(const CacheKey& key, size_t valueSize, const void* value);

    // Validates the cache key for this version of Dawn. At the moment, this is naively checking
    // that the cache key contains the dawn version string in it.
    bool ValidateCacheKey(const CacheKey& key);

    // Protects thread safety of access to mCache.
    std::mutex mMutex;
    // TODO(https://crbug.com/dawn/2365): Convert these members to `raw_ptr`.
    RAW_PTR_EXCLUSION WGPUDawnLoadCacheDataFunction mLoadFunction;
    RAW_PTR_EXCLUSION WGPUDawnStoreCacheDataFunction mStoreFunction;
    RAW_PTR_EXCLUSION void* mFunctionUserdata;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_BLOBCACHE_H_
