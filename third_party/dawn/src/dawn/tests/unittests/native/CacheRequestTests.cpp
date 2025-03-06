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

#include <memory>
#include <utility>
#include <vector>

#include "dawn/native/Blob.h"
#include "dawn/native/CacheRequest.h"
#include "dawn/tests/DawnNativeTest.h"
#include "dawn/tests/mocks/platform/CachingInterfaceMock.h"

namespace dawn::native {
namespace {

using ::testing::_;
using ::testing::ByMove;
using ::testing::Invoke;
using ::testing::MockFunction;
using ::testing::Return;
using ::testing::StrictMock;
using ::testing::WithArg;

class CacheRequestTests : public DawnNativeTest {
  protected:
    std::unique_ptr<dawn::platform::Platform> CreateTestPlatform() override {
        return std::make_unique<DawnCachingMockPlatform>(&mMockCache);
    }

    DeviceBase* GetDevice() { return dawn::native::FromAPI(device.Get()); }

    StrictMock<CachingInterfaceMock> mMockCache;
};

struct Foo {
    int value;
};

#define REQUEST_MEMBERS(X)                   \
    X(int, a)                                \
    X(float, b)                              \
    X(std::vector<unsigned int>, c)          \
    X(CacheKey::UnsafeUnkeyedValue<int*>, d) \
    X(CacheKey::UnsafeUnkeyedValue<Foo>, e)

DAWN_MAKE_CACHE_REQUEST(CacheRequestForTesting, REQUEST_MEMBERS);

#undef REQUEST_MEMBERS

// static_assert the expected types for various return types from the cache hit handler and cache
// miss handler.
TEST_F(CacheRequestTests, CacheResultTypes) {
    EXPECT_CALL(mMockCache, LoadData(_, _, nullptr, 0)).WillRepeatedly(Return(0));

    // (int, ResultOrError<int>), should be ResultOrError<CacheResult<int>>.
    auto v1 = LoadOrRun(
        GetDevice(), CacheRequestForTesting{}, [](Blob) -> int { return 0; },
        [](CacheRequestForTesting) -> ResultOrError<int> { return 1; });
    v1.AcquireSuccess();
    static_assert(std::is_same_v<ResultOrError<CacheResult<int>>, decltype(v1)>);

    // (ResultOrError<float>, ResultOrError<float>), should be ResultOrError<CacheResult<float>>.
    auto v2 = LoadOrRun(
        GetDevice(), CacheRequestForTesting{}, [](Blob) -> ResultOrError<float> { return 0.0; },
        [](CacheRequestForTesting) -> ResultOrError<float> { return 1.0; });
    v2.AcquireSuccess();
    static_assert(std::is_same_v<ResultOrError<CacheResult<float>>, decltype(v2)>);
}

// Test that using a CacheRequest builds a key from the device key, the request type enum, and all
// of the request members.
TEST_F(CacheRequestTests, MakesCacheKey) {
    // Make a request.
    CacheRequestForTesting req;
    req.a = 1;
    req.b = 0.2;
    req.c = {3, 4, 5};

    // Make the expected key.
    CacheKey expectedKey;
    StreamIn(&expectedKey, GetDevice()->GetCacheKey(), "CacheRequestForTesting", req.a, req.b,
             req.c);

    // Expect a call to LoadData with the expected key.
    EXPECT_CALL(mMockCache, LoadData(_, expectedKey.size(), nullptr, 0))
        .WillOnce(WithArg<0>(Invoke([&](const void* actualKeyData) {
            EXPECT_EQ(memcmp(actualKeyData, expectedKey.data(), expectedKey.size()), 0);
            return 0;
        })));

    // Load the request.
    auto result = LoadOrRun(
                      GetDevice(), std::move(req), [](Blob) -> int { return 0; },
                      [](CacheRequestForTesting) -> ResultOrError<int> { return 0; })
                      .AcquireSuccess();

    // The created cache key should be saved on the result.
    EXPECT_EQ(result.GetCacheKey().size(), expectedKey.size());
    EXPECT_EQ(memcmp(result.GetCacheKey().data(), expectedKey.data(), expectedKey.size()), 0);
}

// Test that members that are wrapped in UnsafeUnkeyedValue do not impact the key.
TEST_F(CacheRequestTests, CacheKeyIgnoresUnsafeIgnoredValue) {
    // Make two requests with different UnsafeUnkeyedValues (UnsafeUnkeyed is declared on the struct
    // definition).
    int v1, v2;
    CacheRequestForTesting req1;
    req1.d = &v1;
    req1.e = Foo{42};

    CacheRequestForTesting req2;
    req2.d = &v2;
    req2.e = Foo{24};

    EXPECT_CALL(mMockCache, LoadData(_, _, nullptr, 0)).WillOnce(Return(0)).WillOnce(Return(0));

    static StrictMock<MockFunction<int(CacheRequestForTesting)>> cacheMissFn;

    // Load the first request, and check that the unsafe unkeyed values were passed though
    EXPECT_CALL(cacheMissFn, Call(_)).WillOnce(WithArg<0>(Invoke([&](CacheRequestForTesting req) {
        EXPECT_EQ(req.d.UnsafeGetValue(), &v1);
        EXPECT_FLOAT_EQ(req.e.UnsafeGetValue().value, 42);
        return 0;
    })));
    auto r1 = LoadOrRun(
                  GetDevice(), std::move(req1), [](Blob) { return 0; },
                  [](CacheRequestForTesting req) -> ResultOrError<int> {
                      return cacheMissFn.Call(std::move(req));
                  })
                  .AcquireSuccess();

    // Load the second request, and check that the unsafe unkeyed values were passed though
    EXPECT_CALL(cacheMissFn, Call(_)).WillOnce(WithArg<0>(Invoke([&](CacheRequestForTesting req) {
        EXPECT_EQ(req.d.UnsafeGetValue(), &v2);
        EXPECT_FLOAT_EQ(req.e.UnsafeGetValue().value, 24);
        return 0;
    })));
    auto r2 = LoadOrRun(
                  GetDevice(), std::move(req2), [](Blob) { return 0; },
                  [](CacheRequestForTesting req) -> ResultOrError<int> {
                      return cacheMissFn.Call(std::move(req));
                  })
                  .AcquireSuccess();

    // Expect their keys to be the same.
    EXPECT_EQ(r1.GetCacheKey().size(), r2.GetCacheKey().size());
    EXPECT_EQ(memcmp(r1.GetCacheKey().data(), r2.GetCacheKey().data(), r1.GetCacheKey().size()), 0);
}

// Test the expected code path when there is a cache miss.
TEST_F(CacheRequestTests, CacheMiss) {
    // Make a request.
    CacheRequestForTesting req;
    req.a = 1;
    req.b = 0.2;
    req.c = {3, 4, 5};

    unsigned int* cPtr = req.c.data();

    static StrictMock<MockFunction<int(Blob)>> cacheHitFn;
    static StrictMock<MockFunction<int(CacheRequestForTesting)>> cacheMissFn;

    // Mock a cache miss.
    EXPECT_CALL(mMockCache, LoadData(_, _, nullptr, 0)).WillOnce(Return(0));

    // Expect the cache miss, and return some value.
    int rv = 42;
    EXPECT_CALL(cacheMissFn, Call(_)).WillOnce(WithArg<0>(Invoke([=](CacheRequestForTesting req) {
        // Expect the request contents to be the same. The data pointer for |c| is also the same
        // since it was moved.
        EXPECT_EQ(req.a, 1);
        EXPECT_FLOAT_EQ(req.b, 0.2);
        EXPECT_EQ(req.c.data(), cPtr);
        return rv;
    })));

    // Load the request.
    auto result = LoadOrRun(
                      GetDevice(), std::move(req),
                      [](Blob blob) -> int { return cacheHitFn.Call(std::move(blob)); },
                      [](CacheRequestForTesting req) -> ResultOrError<int> {
                          return cacheMissFn.Call(std::move(req));
                      })
                      .AcquireSuccess();

    // Expect the result to store the value.
    EXPECT_EQ(*result, rv);
    EXPECT_FALSE(result.IsCached());
}

// Test the expected code path when there is a cache hit.
TEST_F(CacheRequestTests, CacheHit) {
    // Make a request.
    CacheRequestForTesting req;
    req.a = 1;
    req.b = 0.2;
    req.c = {3, 4, 5};

    static StrictMock<MockFunction<int(Blob)>> cacheHitFn;
    static StrictMock<MockFunction<int(CacheRequestForTesting)>> cacheMissFn;

    static constexpr char kCachedData[] = "hello world!";

    // Mock a cache hit, and load the cached data.
    EXPECT_CALL(mMockCache, LoadData(_, _, nullptr, 0)).WillOnce(Return(sizeof(kCachedData)));
    EXPECT_CALL(mMockCache, LoadData(_, _, _, sizeof(kCachedData)))
        .WillOnce(WithArg<2>(Invoke([](void* dataOut) {
            memcpy(dataOut, kCachedData, sizeof(kCachedData));
            return sizeof(kCachedData);
        })));

    // Expect the cache hit, and return some value.
    int rv = 1337;
    EXPECT_CALL(cacheHitFn, Call(_)).WillOnce(WithArg<0>(Invoke([=](Blob blob) {
        // Expect the cached blob contents to match the cached data.
        EXPECT_EQ(blob.Size(), sizeof(kCachedData));
        EXPECT_EQ(memcmp(blob.Data(), kCachedData, sizeof(kCachedData)), 0);

        return rv;
    })));

    // Load the request.
    auto result = LoadOrRun(
                      GetDevice(), std::move(req),
                      [](Blob blob) -> int { return cacheHitFn.Call(std::move(blob)); },
                      [](CacheRequestForTesting req) -> ResultOrError<int> {
                          return cacheMissFn.Call(std::move(req));
                      })
                      .AcquireSuccess();

    // Expect the result to store the value.
    EXPECT_EQ(*result, rv);
    EXPECT_TRUE(result.IsCached());
}

// Test the expected code path when there is a cache hit but the handler errors.
TEST_F(CacheRequestTests, CacheHitError) {
    // Make a request.
    CacheRequestForTesting req;
    req.a = 1;
    req.b = 0.2;
    req.c = {3, 4, 5};

    unsigned int* cPtr = req.c.data();

    static StrictMock<MockFunction<ResultOrError<int>(Blob)>> cacheHitFn;
    static StrictMock<MockFunction<int(CacheRequestForTesting)>> cacheMissFn;

    static constexpr char kCachedData[] = "hello world!";

    // Mock a cache hit, and load the cached data.
    EXPECT_CALL(mMockCache, LoadData(_, _, nullptr, 0)).WillOnce(Return(sizeof(kCachedData)));
    EXPECT_CALL(mMockCache, LoadData(_, _, _, sizeof(kCachedData)))
        .WillOnce(WithArg<2>(Invoke([](void* dataOut) {
            memcpy(dataOut, kCachedData, sizeof(kCachedData));
            return sizeof(kCachedData);
        })));

    // Expect the cache hit.
    EXPECT_CALL(cacheHitFn, Call(_)).WillOnce(WithArg<0>(Invoke([=](Blob blob) {
        // Expect the cached blob contents to match the cached data.
        EXPECT_EQ(blob.Size(), sizeof(kCachedData));
        EXPECT_EQ(memcmp(blob.Data(), kCachedData, sizeof(kCachedData)), 0);

        // Return an error.
        return DAWN_VALIDATION_ERROR("fake test error");
    })));

    // Expect the cache miss handler since the cache hit errored.
    int rv = 79;
    EXPECT_CALL(cacheMissFn, Call(_)).WillOnce(WithArg<0>(Invoke([=](CacheRequestForTesting req) {
        // Expect the request contents to be the same. The data pointer for |c| is also the same
        // since it was moved.
        EXPECT_EQ(req.a, 1);
        EXPECT_FLOAT_EQ(req.b, 0.2);
        EXPECT_EQ(req.c.data(), cPtr);
        return rv;
    })));

    // Load the request.
    auto result =
        LoadOrRun(
            GetDevice(), std::move(req),
            [](Blob blob) -> ResultOrError<int> { return cacheHitFn.Call(std::move(blob)); },
            [](CacheRequestForTesting req) -> ResultOrError<int> {
                return cacheMissFn.Call(std::move(req));
            })
            .AcquireSuccess();

    // Expect the result to store the value.
    EXPECT_EQ(*result, rv);
    EXPECT_FALSE(result.IsCached());
}

}  // anonymous namespace
}  // namespace dawn::native
