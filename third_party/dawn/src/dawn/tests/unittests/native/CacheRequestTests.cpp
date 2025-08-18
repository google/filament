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
#include "dawn/webgpu_cpp_print.h"

namespace dawn::native {
namespace {

using ::testing::_;
using ::testing::ByMove;
using ::testing::Invoke;
using ::testing::MockFunction;
using ::testing::Return;
using ::testing::StrictMock;
using ::testing::WithArg;

struct CacheRequestTestParam {
    bool enableHashValidation;
};

class CacheRequestTests : public DawnNativeTest,
                          public ::testing::WithParamInterface<CacheRequestTestParam> {
  public:
    wgpu::DawnTogglesDescriptor DeviceToggles() override {
        wgpu::DawnTogglesDescriptor toggles = {};

        // Explicitly set the toggle for hash validation based on the test parameter.
        if (GetParam().enableHashValidation) {
            toggles.enabledToggles = &mHashValidationToggle;
            toggles.enabledToggleCount = 1;
        } else {
            toggles.disabledToggles = &mHashValidationToggle;
            toggles.disabledToggleCount = 1;
        }

        return toggles;
    }

  protected:
    std::unique_ptr<dawn::platform::Platform> CreateTestPlatform() override {
        return std::make_unique<DawnCachingMockPlatform>(&mMockCache);
    }

    DeviceBase* GetDevice() { return dawn::native::FromAPI(device.Get()); }

    StrictMock<CachingInterfaceMock> mMockCache;
    static constexpr const char* mHashValidationToggle = "blob_cache_hash_validation";
};

struct Foo {
    int value;
};

#define REQUEST_MEMBERS(X)              \
    X(int, a)                           \
    X(float, b)                         \
    X(std::vector<unsigned int>, c)     \
    X(UnsafeUnserializedValue<int*>, d) \
    X(UnsafeUnserializedValue<Foo>, e)

DAWN_MAKE_CACHE_REQUEST(CacheRequestForTesting, REQUEST_MEMBERS);

#undef REQUEST_MEMBERS

// static_assert the expected types for various return types from the cache hit handler and cache
// miss handler.
TEST_P(CacheRequestTests, CacheResultTypes) {
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
TEST_P(CacheRequestTests, MakesCacheKey) {
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

// Test that members that are wrapped in UnsafeUnserializedValue do not impact the key.
TEST_P(CacheRequestTests, CacheKeyIgnoresUnsafeIgnoredValue) {
    // Make two requests with different UnsafeUnserializedValue (UnsafeUnkeyed is declared on the
    // struct definition).
    int v1, v2;
    CacheRequestForTesting req1;
    req1.d = UnsafeUnserializedValue(&v1);
    req1.e = UnsafeUnserializedValue(Foo{42});

    CacheRequestForTesting req2;
    req2.d = UnsafeUnserializedValue(&v2);
    req2.e = UnsafeUnserializedValue(Foo{24});

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
TEST_P(CacheRequestTests, CacheMiss) {
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
TEST_P(CacheRequestTests, CacheHit) {
    // Make a request.
    CacheRequestForTesting req;
    req.a = 1;
    req.b = 0.2;
    req.c = {3, 4, 5};

    static StrictMock<MockFunction<int(Blob)>> cacheHitFn;
    static StrictMock<MockFunction<int(CacheRequestForTesting)>> cacheMissFn;

    static constexpr char kCachedData[] = "hello world!";
    // Bytes actually stored into and loaded from Blob cache might be different from raw given data.
    Blob actualStoredData = GetDevice()->GetBlobCache()->GenerateActualStoredBlobForTesting(
        sizeof(kCachedData), kCachedData);

    // Mock a cache hit, and load the cached data.
    EXPECT_CALL(mMockCache, LoadData(_, _, nullptr, 0)).WillOnce(Return(actualStoredData.Size()));
    EXPECT_CALL(mMockCache, LoadData(_, _, _, actualStoredData.Size()))
        .WillOnce(WithArg<2>(Invoke([&actualStoredData](void* dataOut) {
            memcpy(dataOut, actualStoredData.Data(), actualStoredData.Size());
            return actualStoredData.Size();
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
TEST_P(CacheRequestTests, CacheHitError) {
    // Make a request.
    CacheRequestForTesting req;
    req.a = 1;
    req.b = 0.2;
    req.c = {3, 4, 5};

    unsigned int* cPtr = req.c.data();

    static StrictMock<MockFunction<ResultOrError<int>(Blob)>> cacheHitFn;
    static StrictMock<MockFunction<int(CacheRequestForTesting)>> cacheMissFn;

    static constexpr char kCachedData[] = "hello world!";
    // Bytes actually stored into and loaded from Blob cache might be different from raw given data.
    Blob actualStoredData = GetDevice()->GetBlobCache()->GenerateActualStoredBlobForTesting(
        sizeof(kCachedData), kCachedData);

    // Mock a cache hit, and load the cached data.
    EXPECT_CALL(mMockCache, LoadData(_, _, nullptr, 0)).WillOnce(Return(actualStoredData.Size()));
    EXPECT_CALL(mMockCache, LoadData(_, _, _, actualStoredData.Size()))
        .WillOnce(WithArg<2>(Invoke([&actualStoredData](void* dataOut) {
            memcpy(dataOut, actualStoredData.Data(), actualStoredData.Size());
            return actualStoredData.Size();
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

// Test the expected code path when hash validation is enabled, there is a cache hit but the hash
// validation fails. This should be treated as a cache miss, and the cache miss handler should be
// called.
TEST_P(CacheRequestTests, CacheHitHashValidationFailed) {
    // Only run this test if hash validation is enabled.
    if (!GetParam().enableHashValidation) {
        GTEST_SKIP();
    }

    static constexpr char kCachedData[] = "hello world!";
    static constexpr size_t kCachedDataSize = sizeof(kCachedData);
    // Bytes actually stored into and loaded from Blob cache might be different from raw given data.
    Blob actualStoredData = GetDevice()->GetBlobCache()->GenerateActualStoredBlobForTesting(
        kCachedDataSize, kCachedData);
    const size_t sizeWithHash = actualStoredData.Size();
    // With hash validation enabled, the actual stored data size is larger than kCachedData.
    ASSERT_GT(sizeWithHash, kCachedDataSize);
    const size_t addedSize = sizeWithHash - kCachedDataSize;

    auto DoTest = [&](const void* loadBuffer, size_t loadSize, bool expectHashValidationSuccess) {
        // LoadOrRun requires its cache miss handler being a free function (i.e. without any
        // capture), so the mock function has to be static to get used in it. However, using the
        // same mock function object for different test cases might cause their expectations mixed
        // up, so we use a static unique_ptr to ensure that each test case constructs and destructs
        // its own mock function object.
        static std::unique_ptr<StrictMock<MockFunction<int(Blob)>>> cacheHitFn;
        static std::unique_ptr<StrictMock<MockFunction<int(CacheRequestForTesting)>>> cacheMissFn;
        constexpr int rvCacheHit = 21;
        constexpr int rvCacheMiss = 42;

        // Make a request.
        CacheRequestForTesting req;
        req.a = 1;
        req.b = 0.2;
        req.c = {3, 4, 5};

        unsigned int* cPtr = req.c.data();

        // Mock a cache hit with given data buffer.
        EXPECT_CALL(mMockCache, LoadData(_, _, nullptr, 0)).WillOnce(Return(loadSize));
        EXPECT_CALL(mMockCache, LoadData(_, _, _, loadSize))
            .WillOnce(WithArg<2>(Invoke([&](void* dataOut) {
                memcpy(dataOut, loadBuffer, loadSize);
                return loadSize;
            })));

        // Construct mock functions for current test.
        ASSERT_FALSE(cacheHitFn);
        ASSERT_FALSE(cacheMissFn);
        cacheHitFn = std::make_unique<StrictMock<MockFunction<int(Blob)>>>();
        cacheMissFn = std::make_unique<StrictMock<MockFunction<int(CacheRequestForTesting)>>>();

        if (expectHashValidationSuccess) {
            // Expect the cache hit handler to be called with the loaded blob.
            EXPECT_CALL(*cacheHitFn, Call(_)).WillOnce(WithArg<0>(Invoke([=](Blob blob) {
                // Expect the loaded blob contents to match the cached data.
                EXPECT_EQ(blob.Size(), sizeof(kCachedData));
                EXPECT_EQ(memcmp(blob.Data(), kCachedData, sizeof(kCachedData)), 0);

                return rvCacheHit;
            })));
        } else {
            // Expect the cacheMissFn called and return some value, since hash validation failure
            // are treated as miss.
            EXPECT_CALL(*cacheMissFn, Call(_))
                .WillOnce(WithArg<0>(Invoke([=](CacheRequestForTesting req) {
                    // Expect the request contents to be the same. The data pointer for |c| is also
                    // the same since it was moved.
                    EXPECT_EQ(req.a, 1);
                    EXPECT_FLOAT_EQ(req.b, 0.2);
                    EXPECT_EQ(req.c.data(), cPtr);
                    return rvCacheMiss;
                })));
        }

        // Load the request.
        auto result = LoadOrRun(
                          GetDevice(), std::move(req),
                          [](Blob blob) -> int { return cacheHitFn->Call(std::move(blob)); },
                          [](CacheRequestForTesting req) -> ResultOrError<int> {
                              return cacheMissFn->Call(std::move(req));
                          })
                          .AcquireSuccess();

        if (expectHashValidationSuccess) {
            // Expect the result to hold the return value from cache hit.
            EXPECT_EQ(*result, rvCacheHit);
            EXPECT_TRUE(result.IsCached());
        } else {
            // Expect the result to hold the return value from cache miss.
            EXPECT_EQ(*result, rvCacheMiss);
            EXPECT_FALSE(result.IsCached());
        }

        // Destruct the mock functions to ensure all expected behavior happened.
        cacheHitFn.reset();
        cacheMissFn.reset();
    };

    // Control case: hash validation success.
    {
        DoTest(actualStoredData.Data(), sizeWithHash, true);
    }

    // Hash validation failure case 1: loaded blob size too small.
    {
        static constexpr uint8_t tooSmallBuffer[] = "0";
        static constexpr size_t tooSmallBufferSize = sizeof(tooSmallBuffer);
        ASSERT_LT(tooSmallBufferSize, addedSize);

        DoTest(tooSmallBuffer, tooSmallBufferSize, false);
    }

    // Hash validation failure case 2: loaded blob hash mismatched.
    {
        Blob modifiedStoredData = CreateBlob(sizeWithHash);
        memcpy(modifiedStoredData.Data(), actualStoredData.Data(), sizeWithHash);
        // Modify the last byte to make the hash mismatch.
        modifiedStoredData.Data()[sizeWithHash - 1] = ~modifiedStoredData.Data()[sizeWithHash - 1];

        DoTest(modifiedStoredData.Data(), sizeWithHash, false);
    }
}

INSTANTIATE_TEST_SUITE_P(,
                         CacheRequestTests,
                         testing::Values(CacheRequestTestParam{.enableHashValidation = false},
                                         CacheRequestTestParam{.enableHashValidation = true}));

}  // anonymous namespace
}  // namespace dawn::native
