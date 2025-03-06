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

#ifndef SRC_DAWN_TESTS_MOCKS_PLATFORM_CACHINGINTERFACEMOCK_H_
#define SRC_DAWN_TESTS_MOCKS_PLATFORM_CACHINGINTERFACEMOCK_H_

#include <dawn/platform/DawnPlatform.h>
#include <gmock/gmock.h>

#include <string>
#include <unordered_map>
#include <vector>

#include "dawn/common/TypedInteger.h"
#include "partition_alloc/pointers/raw_ptr.h"

#define EXPECT_CACHE_HIT(cache, N, statement) \
    do {                                      \
        FlushWire();                          \
        size_t before = cache.GetHitCount();  \
        statement;                            \
        FlushWire();                          \
        size_t after = cache.GetHitCount();   \
        EXPECT_EQ(N, after - before);         \
    } while (0)

// Check that |HitN| cache hits occurred, and |AddN| entries were added.
// Usage: EXPECT_CACHE_STATS(myMockCache, Hit(42), Add(3), ...)
// Hit / Add help readability, and enforce the args are passed correctly in the expected order.
#define EXPECT_CACHE_STATS(cache, HitN, AddN, statement)                    \
    do {                                                                    \
        using Hit = dawn::TypedInteger<struct HitT, size_t>;                \
        using Add = dawn::TypedInteger<struct AddT, size_t>;                \
        static_assert(std::is_same_v<decltype(HitN), Hit>);                 \
        static_assert(std::is_same_v<decltype(AddN), Add>);                 \
        FlushWire();                                                        \
        size_t hitBefore = cache.GetHitCount();                             \
        size_t entriesBefore = cache.GetNumEntries();                       \
        statement;                                                          \
        FlushWire();                                                        \
        size_t hitAfter = cache.GetHitCount();                              \
        size_t entriesAfter = cache.GetNumEntries();                        \
        EXPECT_EQ(static_cast<size_t>(HitN), hitAfter - hitBefore);         \
        EXPECT_EQ(static_cast<size_t>(AddN), entriesAfter - entriesBefore); \
    } while (0)

// A mock caching interface class that also supplies an in memory cache for testing.
class CachingInterfaceMock : public dawn::platform::CachingInterface {
  public:
    CachingInterfaceMock();

    // Toggles to disable/enable caching.
    void Disable();
    void Enable();

    // Returns the number of cache hits up to this point.
    size_t GetHitCount() const;

    // Returns the number of entries in the cache.
    size_t GetNumEntries() const;

    MOCK_METHOD(size_t, LoadData, (const void*, size_t, void*, size_t), (override));
    MOCK_METHOD(void, StoreData, (const void*, size_t, const void*, size_t), (override));

  private:
    size_t LoadDataDefault(const void* key, size_t keySize, void* value, size_t valueSize);
    void StoreDataDefault(const void* key, size_t keySize, const void* value, size_t valueSize);

    bool mEnabled = true;
    size_t mHitCount = 0;
    std::unordered_map<std::string, std::vector<uint8_t>> mCache;
};

// Dawn platform used for testing with a mock caching interface.
class DawnCachingMockPlatform : public dawn::platform::Platform {
  public:
    explicit DawnCachingMockPlatform(dawn::platform::CachingInterface* cachingInterface);

    dawn::platform::CachingInterface* GetCachingInterface() override;

  private:
    raw_ptr<dawn::platform::CachingInterface> mCachingInterface = nullptr;
};

#endif  // SRC_DAWN_TESTS_MOCKS_PLATFORM_CACHINGINTERFACEMOCK_H_
