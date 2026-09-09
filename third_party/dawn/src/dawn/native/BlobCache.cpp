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

#include "src/dawn/native/BlobCache.h"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <utility>

#include "dawn/dawn_version.h"
#include "dawn/platform/DawnPlatform.h"
#include "src/dawn/common/StringViewUtils.h"
#include "src/dawn/native/CacheKey.h"
#include "src/dawn/native/Instance.h"
#include "src/utils/assert.h"
#include "src/utils/compiler.h"

namespace dawn::native {

namespace detail {
// Hasher for Blob cache validation
using Hasher = Sha3_224;
using Hash = Hasher::Output;
static constexpr const size_t kHashByteSize = sizeof(Hash);

std::vector<std::byte> GenerateHashPrefixedPayload(std::span<const std::byte> value) {
    // Create a buffer for holding hash+payload.
    // TODO(https://crbug.com/512465980): Use HeapArray instead of std::vector.
    const size_t byteSizeWithHash = value.size() + kHashByteSize;
    std::vector<std::byte> result(byteSizeWithHash);

    // Write the hash to the start of the buffer.
    *DAWN_UNSAFE_TODO(reinterpret_cast<Hash*>(result.data())) = Hasher::Hash(value);

    // Write the payload after the hash.
    std::ranges::copy(value, result.begin() + kHashByteSize);
    return result;
}

ResultOrError<Blob> CheckAndUnpackHashPrefixedPayload(Blob&& blobWithHash) {
    // Validate the size of the buffer must be larger than the size of hash result.
    const size_t sizeWithHash = blobWithHash.Size();
    DAWN_INTERNAL_ERROR_IF(!(sizeWithHash > kHashByteSize),
                           "Blob cache hash validation failed. Blob of %zu bytes loaded from cache "
                           "is no larger than size of hash result %zu bytes.",
                           sizeWithHash, kHashByteSize);

    // Read the expected hash before we hide the hash from the blob.
    Hash* expectedHash = reinterpret_cast<Hash*>(blobWithHash.DataPtr());

    // Create a blob that appears without the hash, but still owns the entire piece of memory.
    Blob blob = Blob::Create(std::move(blobWithHash), /*offset=*/kHashByteSize);
    Hash actualHash = Hasher::Hash(blob.Data());

    auto printHash = [](const void* hash) {
        std::stringstream ss;
        const uint8_t* hashBytes = static_cast<const uint8_t*>(hash);
        for (size_t i = 0; i < kHashByteSize; i++) {
            ss << std::uppercase << std::hex << std::setw(2) << std::setfill('0')
               << static_cast<int>(DAWN_UNSAFE_TODO(hashBytes[i]));
        }
        return ss.str();
    };
    // Validate the hash matches the expected hash.
    DAWN_INTERNAL_ERROR_IF(actualHash != *expectedHash,
                           "Blob cache hash validation failed. Loaded blob of size %zu fails the "
                           "hash validation, expected hash: %s, computed hash: %s.",
                           sizeWithHash, printHash(expectedHash), printHash(&actualHash));
    return std::move(blob);
}

}  // namespace detail

BlobCache::BlobCache(const dawn::native::DawnCacheDeviceDescriptor& desc, bool enableHashValidation)
    : mHashValidation(enableHashValidation),
      mLoadCallbackInfo(desc.dawnLoadCacheDataCallbackInfo),
      mStoreCallbackInfo(desc.dawnStoreCacheDataCallbackInfo) {}

ResultOrError<Blob> BlobCache::Load(const CacheKey& key) {
    return LoadInternal(key);
}

void BlobCache::Store(const CacheKey& key, std::span<const std::byte> value) {
    StoreInternal(key, value);
}

void BlobCache::Store(const CacheKey& key, const Blob& value) {
    Store(key, value.Data());
}

Blob BlobCache::GenerateActualStoredBlobForTesting(std::span<const std::byte> value) {
    if (!mHashValidation) {
        Blob blob = Blob::Create(value.size());
        std::ranges::copy(value, blob.Data().begin());
        return blob;
    }
    return Blob::Create(detail::GenerateHashPrefixedPayload(value));
}

void BlobCache::StoreInternal(const CacheKey& cacheKey, std::span<const std::byte> value) {
    DAWN_ASSERT(ValidateCacheKey(cacheKey));
    DAWN_CHECK(value.data() != nullptr);
    DAWN_CHECK(value.size() > 0);

    // Make sure we early out if we are not using storing functionality. Otherwise, computing the
    // hash may add unnecessary overhead.
    if (mStoreCallbackInfo.callback == nullptr) {
        return;
    }
    auto store = [&](std::span<const std::byte> actualValue) {
        mStoreCallbackInfo.callback(
            cacheKey.size(), reinterpret_cast<const uint8_t*>(cacheKey.data()), actualValue.size(),
            reinterpret_cast<const uint8_t*>(actualValue.data()), mStoreCallbackInfo.userdata1,
            mStoreCallbackInfo.userdata2);
    };

    // Call the actual store function for actual stored bytes.
    if (!mHashValidation) {
        store(value);
    } else {
        std::vector<std::byte> actualStoredData = detail::GenerateHashPrefixedPayload(value);
        store(actualStoredData);
    }
}

ResultOrError<Blob> BlobCache::LoadInternal(const CacheKey& cacheKey) {
    DAWN_ASSERT(ValidateCacheKey(cacheKey));

    if (mLoadCallbackInfo.callback == nullptr) {
        return Blob();
    }
    auto load = [&](std::span<std::byte> value) -> size_t {
        return mLoadCallbackInfo.callback(cacheKey.size(),
                                          reinterpret_cast<const uint8_t*>(cacheKey.data()),
                                          value.size(), reinterpret_cast<uint8_t*>(value.data()),
                                          mLoadCallbackInfo.userdata1, mLoadCallbackInfo.userdata2);
    };

    const size_t expectedSize = load({});
    // Non-zero size indicates cache hit
    if (expectedSize > 0) {
        // Load bytes from cache.
        Blob result = Blob::Create(expectedSize);
        const size_t actualSize = load(result.Data());
        // TODO(crbug.com/469351711): If `mLoadCallbackInfo`'s callback returns a different
        // size on the second call (due to external cache eviction, I/O errors, or timeouts), treat
        // it as a cache miss. The blob cache API should be updated to a single `mLoadFunction` call
        // in the future.
        if (expectedSize != actualSize) {
            return Blob();
        }

        if (!mHashValidation) {
            return std::move(result);
        }
        return detail::CheckAndUnpackHashPrefixedPayload(std::move(result));
    }
    return Blob();
}

bool BlobCache::ValidateCacheKey(const CacheKey& key) {
    return !std::ranges::search(std::span(key), std::as_bytes(std::span(kDawnVersion))).empty();
}

}  // namespace dawn::native
