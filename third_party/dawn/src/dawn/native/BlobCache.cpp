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

#include "dawn/native/BlobCache.h"

#include <algorithm>

#include "dawn/common/Assert.h"
#include "dawn/common/Version_autogen.h"
#include "dawn/native/CacheKey.h"
#include "dawn/native/Instance.h"
#include "dawn/platform/DawnPlatform.h"

namespace dawn::native {

BlobCache::BlobCache(const dawn::native::DawnCacheDeviceDescriptor& desc)
    : mLoadFunction(desc.loadDataFunction),
      mStoreFunction(desc.storeDataFunction),
      mFunctionUserdata(desc.functionUserdata) {}

Blob BlobCache::Load(const CacheKey& key) {
    std::lock_guard<std::mutex> lock(mMutex);
    return LoadInternal(key);
}

void BlobCache::Store(const CacheKey& key, size_t valueSize, const void* value) {
    std::lock_guard<std::mutex> lock(mMutex);
    StoreInternal(key, valueSize, value);
}

void BlobCache::Store(const CacheKey& key, const Blob& value) {
    Store(key, value.Size(), value.Data());
}

Blob BlobCache::LoadInternal(const CacheKey& key) {
    DAWN_ASSERT(ValidateCacheKey(key));
    if (mLoadFunction == nullptr) {
        return Blob();
    }
    const size_t expectedSize =
        mLoadFunction(key.data(), key.size(), nullptr, 0, mFunctionUserdata);
    if (expectedSize > 0) {
        // Need to put this inside to trigger copy elision.
        Blob result = CreateBlob(expectedSize);
        const size_t actualSize =
            mLoadFunction(key.data(), key.size(), result.Data(), expectedSize, mFunctionUserdata);
        DAWN_ASSERT(expectedSize == actualSize);
        return result;
    }
    return Blob();
}

void BlobCache::StoreInternal(const CacheKey& key, size_t valueSize, const void* value) {
    DAWN_ASSERT(ValidateCacheKey(key));
    DAWN_ASSERT(value != nullptr);
    DAWN_ASSERT(valueSize > 0);
    if (mStoreFunction == nullptr) {
        return;
    }
    mStoreFunction(key.data(), key.size(), value, valueSize, mFunctionUserdata);
}

bool BlobCache::ValidateCacheKey(const CacheKey& key) {
    return std::search(key.begin(), key.end(), kDawnVersion.begin(), kDawnVersion.end()) !=
           key.end();
}

}  // namespace dawn::native
