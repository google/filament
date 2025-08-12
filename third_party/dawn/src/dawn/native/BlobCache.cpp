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

namespace details {
// Hasher for Blob cache validation
using Hasher = Sha3_224;
using Hash = Hasher::Output;
static constexpr const size_t mHashByteSize = sizeof(Hash);

Blob GenerateHashPrefixedPayload(size_t valueSize, const void* value) {
    // Create a Blob for holding hash+payload.
    const size_t byteSizeWithHash = valueSize + mHashByteSize;
    Blob result = CreateBlob(byteSizeWithHash);
    Hash* hashHeader = reinterpret_cast<Hash*>(result.Data());
    uint8_t* payload = result.Data() + mHashByteSize;
    // Copy the payload into buffer and compute the hash as prefix.
    memcpy(payload, value, valueSize);
    *hashHeader = Hasher::Hash(payload, valueSize);

    return result;
}

ResultOrError<Blob> CheckAndUnpackHashPrefixedPayload(uint8_t* buffer, size_t sizeWithHash) {
    // Validate the size of the buffer must be larger than the size of hash result.
    DAWN_INTERNAL_ERROR_IF(!(sizeWithHash > mHashByteSize),
                           "Blob cache hash validation failed. Blob of %zu bytes loaded from cache "
                           "is no larger than size of hash result %zu bytes.",
                           sizeWithHash, mHashByteSize);
    // Compute and validate the payload's hash.
    size_t payloadByteSize = sizeWithHash - mHashByteSize;
    Hash* expectedHash = (reinterpret_cast<Hash*>(buffer));
    uint8_t* payload = buffer + mHashByteSize;
    Hash payloadHash = Hasher::Hash(payload, payloadByteSize);
    auto printHash = [](const void* hash) {
        std::stringstream ss;
        const uint8_t* hashBytes = static_cast<const uint8_t*>(hash);
        for (size_t i = 0; i < mHashByteSize; i++) {
            ss << std::uppercase << std::hex << std::setw(2) << std::setfill('0')
               << static_cast<int>(hashBytes[i]);
        }
        return ss.str();
    };
    // Validate the hash matches the expected hash.
    DAWN_INTERNAL_ERROR_IF(payloadHash != *expectedHash,
                           "Blob cache hash validation failed. Loaded blob of size %zu fails the "
                           "hash validation, expected hash: %s, computed hash: %s.",
                           sizeWithHash, printHash(expectedHash), printHash(&payloadHash));

    // Wrap the buffer as Blob without exposing the hash header.
    Blob result =
        Blob::UnsafeCreateWithDeleter(payload, payloadByteSize, [=]() { delete[] buffer; });

    return result;
}

}  // namespace details

BlobCache::BlobCache(const dawn::native::DawnCacheDeviceDescriptor& desc, bool enableHashValidation)
    : mHashValidation(enableHashValidation),
      mLoadFunction(desc.loadDataFunction),
      mStoreFunction(desc.storeDataFunction),
      mFunctionUserdata(desc.functionUserdata) {}

ResultOrError<Blob> BlobCache::Load(const CacheKey& key) {
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

Blob BlobCache::GenerateActualStoredBlobForTesting(size_t valueSize, const void* value) {
    if (!mHashValidation) {
        Blob blob = CreateBlob(valueSize);
        memcpy(blob.Data(), value, valueSize);
        return blob;
    }
    return details::GenerateHashPrefixedPayload(valueSize, value);
}

void BlobCache::StoreInternal(const CacheKey& key, size_t valueSize, const void* value) {
    DAWN_ASSERT(ValidateCacheKey(key));
    DAWN_ASSERT(value != nullptr);
    DAWN_ASSERT(valueSize > 0);
    if (mStoreFunction == nullptr) {
        return;
    }

    // Call the actual store function for actual stored bytes.
    if (!mHashValidation) {
        mStoreFunction(key.data(), key.size(), value, valueSize, mFunctionUserdata);
    } else {
        Blob actualStoredBlob = details::GenerateHashPrefixedPayload(valueSize, value);
        mStoreFunction(key.data(), key.size(), actualStoredBlob.Data(), actualStoredBlob.Size(),
                       mFunctionUserdata);
    }
}

ResultOrError<Blob> BlobCache::LoadInternal(const CacheKey& key) {
    DAWN_ASSERT(ValidateCacheKey(key));
    if (mLoadFunction == nullptr) {
        return Blob();
    }
    const size_t expectedSize =
        mLoadFunction(key.data(), key.size(), nullptr, 0, mFunctionUserdata);
    // Non-zero size indicates cache hit
    if (expectedSize > 0) {
        // Load bytes from cache.
        uint8_t* buffer = new uint8_t[expectedSize];
        const size_t actualSize =
            mLoadFunction(key.data(), key.size(), buffer, expectedSize, mFunctionUserdata);
        DAWN_CHECK(expectedSize == actualSize);

        if (!mHashValidation) {
            return Blob::UnsafeCreateWithDeleter(buffer, actualSize, [=]() { delete[] buffer; });
        }
        return details::CheckAndUnpackHashPrefixedPayload(buffer, expectedSize);
    }
    return Blob();
}

bool BlobCache::ValidateCacheKey(const CacheKey& key) {
    return std::search(key.begin(), key.end(), kDawnVersion.begin(), kDawnVersion.end()) !=
           key.end();
}

}  // namespace dawn::native
