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

#include "dawn/native/PipelineCache.h"

#include <atomic>

namespace dawn::native {

PipelineCacheBase::PipelineCacheBase(BlobCache* cache, const CacheKey& key, bool storeOnIdle)
    : mCache(cache), mKey(key), mStoreOnIdle(storeOnIdle) {}

Blob PipelineCacheBase::Initialize() {
    DAWN_ASSERT(!mInitialized);

    auto loadResult = mCache->Load(mKey);
    Blob blob;
    if (loadResult.IsSuccess()) {
        blob = loadResult.AcquireSuccess();
    }
    // Otherwise cache hit but hash validation failed, leaving blob empty to continue as if it was a
    // cache miss.

    mCacheHit = !blob.Empty();
    mInitialized = true;
    return blob;
}

bool PipelineCacheBase::CacheHit() const {
    DAWN_ASSERT(mInitialized);
    return mCacheHit;
}

MaybeError PipelineCacheBase::Flush() {
    // Try to write the data out to the persistent cache.
    Blob blob;
    DAWN_TRY(SerializeToBlobImpl(&blob));
    if (blob.Size() > 0) {
        // Using a simple heuristic to decide whether to write out the blob right now. May need
        // smarter tracking when we are dealing with monolithic caches.
        mCache->Store(mKey, blob);
    }
    return {};
}

MaybeError PipelineCacheBase::DidCompilePipeline() {
    DAWN_ASSERT(mInitialized);
    if (mStoreOnIdle) {
        // Assume pipeline cache was modified by compiling a pipeline. It will be stored in
        // BlobCache at some later point in StoreOnIdle() if necessary.
        mNeedsStore.store(true, std::memory_order_relaxed);
    } else {
        // TODO(dawn:549): Flush is currently synchronously happening on the same thread as pipeline
        // compilation, but it's perhaps deferrable.
        if (!CacheHit()) {
            return Flush();
        }
    }
    return {};
}

MaybeError PipelineCacheBase::StoreOnIdle() {
    DAWN_ASSERT(mStoreOnIdle);
    if (mNeedsStore.exchange(false, std::memory_order_relaxed)) {
        DAWN_TRY(Flush());
    }
    return {};
}

}  // namespace dawn::native
