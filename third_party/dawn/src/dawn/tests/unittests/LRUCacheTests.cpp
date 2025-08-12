// Copyright 2025 The Dawn & Tint Authors
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

#include "dawn/common/HashUtils.h"
#include "dawn/common/LRUCache.h"
#include "dawn/native/Error.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace dawn::native {
namespace {

class CacheKey {
  public:
    explicit CacheKey(uint32_t value, bool isError = false) : mValue(value), mIsError(isError) {}

    const uint32_t mValue;
    const bool mIsError;
};

class CacheValue {
  public:
    explicit CacheValue(uint32_t value) : mValue(value), mId(CacheValue::mNextId++) {}

    uint32_t mValue;
    uint32_t mId;

  private:
    static uint32_t mNextId;
};

uint32_t CacheValue::mNextId = 1;

struct CacheFuncs {
    size_t operator()(const CacheKey& key) const {
        size_t hash = Hash(key.mValue);
        HashCombine(&hash, key.mIsError);
        return hash;
    }
    bool operator()(const CacheKey& a, const CacheKey& b) const {
        return a.mValue == b.mValue && a.mIsError == b.mIsError;
    }
};

class TestCache final : public LRUCache<CacheKey, CacheValue, CacheFuncs> {
    using Base = LRUCache<CacheKey, CacheValue, CacheFuncs>;

  public:
    static const size_t kDefaultCapacity = 4;
    explicit TestCache(size_t capacity = kDefaultCapacity) : Base(capacity) {}
    ~TestCache() override = default;

    MOCK_METHOD(void, EvictedFromCache, (const CacheValue&), (override));

    // Calls GetOrCreate, assuming no error.
    CacheValue GetOrCreateNoError(CacheKey& key) {
        auto result = GetOrCreate(key, &TestCache::CreateFn);
        EXPECT_FALSE(result.IsError());
        return result.AcquireSuccess();
    }

    static Result<CacheValue, ErrorData> CreateFn(const CacheKey& key) {
        DAWN_INTERNAL_ERROR_IF(key.mIsError, "CacheKey was an error key");
        return CacheValue(key.mValue);
    }
};

// A cache size of 0 disables caching, but still creates the value.
TEST(LRUCache, ZeroSizedCache) {
    TestCache cache(0);

    CacheKey key(1);
    uint32_t cacheId;
    {
        // The constructed values should be evicted immediately.
        EXPECT_CALL(cache, EvictedFromCache(testing::_));
        CacheValue value = cache.GetOrCreateNoError(key);
        cacheId = value.mId;
    }

    {
        EXPECT_CALL(cache, EvictedFromCache(testing::_));
        CacheValue value = cache.GetOrCreateNoError(key);

        // A new cached value should be created.
        EXPECT_NE(cacheId, value.mId);
    }
}

// A cache with a non-zero capacity should return the same value for equivalent keys
TEST(LRUCache, Basic) {
    TestCache cache;

    uint32_t cacheId;
    {
        CacheKey key(1);
        CacheValue value = cache.GetOrCreateNoError(key);
        cacheId = value.mId;
    }

    {
        CacheKey key(1);
        CacheValue value = cache.GetOrCreateNoError(key);

        // A new CacheValue should be returned.
        EXPECT_EQ(cacheId, value.mId);
    }
}

// A cache with a non-zero capacity should begin evicting old values after it reached capacity.
TEST(LRUCache, CacheEviction) {
    TestCache cache;

    std::array<uint32_t, TestCache::kDefaultCapacity> keyIds;
    // Fill the cache with values.
    for (uint32_t i = 0; i < TestCache::kDefaultCapacity; ++i) {
        CacheKey key(i);
        auto value = cache.GetOrCreateNoError(key);
        keyIds[i] = value.mId;
    }

    // Calling GetOrCreate with an existing key won't cause a cache eviction
    {
        CacheKey key(1);
        EXPECT_CALL(cache, EvictedFromCache(testing::_)).Times(0);
        cache.GetOrCreateNoError(key);
    }

    // Calling GetOrCreate with a new key should evict an older value.
    {
        CacheKey key(TestCache::kDefaultCapacity);
        EXPECT_CALL(cache, EvictedFromCache(testing::_));
        cache.GetOrCreateNoError(key);
    }

    // The oldest value in the cache (key 0) should have been the one evicted,
    // as evidenced by us receiving a new value when querying it again.
    {
        CacheKey key(0);
        EXPECT_CALL(cache, EvictedFromCache(testing::_));
        auto value = cache.GetOrCreateNoError(key);
        EXPECT_NE(value.mId, keyIds[0]);
    }

    // The value for key 1 should not have been the one evicted after the last
    // call, because it was queried more recently than the other keys.
    {
        CacheKey key(1);
        EXPECT_CALL(cache, EvictedFromCache(testing::_)).Times(0);
        auto value = cache.GetOrCreateNoError(key);
        EXPECT_EQ(value.mId, keyIds[1]);
    }
}

// The Clear method should evict all values from the cache.
TEST(LRUCache, CacheClear) {
    TestCache cache;

    std::array<uint32_t, TestCache::kDefaultCapacity> keyIds;
    // Fill the cache with values.
    for (uint32_t i = 0; i < TestCache::kDefaultCapacity; ++i) {
        CacheKey key(i);
        auto value = cache.GetOrCreateNoError(key);
        keyIds[i] = value.mId;
    }

    // Calling Clear should evict all values from the cache.
    {
        EXPECT_CALL(cache, EvictedFromCache(testing::_)).Times(TestCache::kDefaultCapacity);
        cache.Clear();
    }

    // Calling GetOrCreate with a previously created key should return a new
    // value and cause no evictions
    {
        CacheKey key(0);
        EXPECT_CALL(cache, EvictedFromCache(testing::_)).Times(0);
        auto value = cache.GetOrCreateNoError(key);
        EXPECT_NE(value.mId, keyIds[0]);
    }
}

}  // anonymous namespace
}  // namespace dawn::native
