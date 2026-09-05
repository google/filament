// Copyright 2026 The Dawn & Tint Authors
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

#include <vector>

#include "dawn/dawn_version.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "src/dawn/native/BlobCache.h"
#include "src/dawn/native/CacheKey.h"
#include "src/dawn/native/stream/Stream.h"
#include "src/dawn/tests/MockCallback.h"
#include "src/utils/compiler.h"
#include "webgpu/webgpu_cpp.h"

namespace dawn::native {
namespace {

using testing::_;
using testing::MockCppCallback;

CacheKey CreateValidKey() {
    CacheKey key;
    stream::StreamIn(
        &key, DAWN_UNSAFE_TODO(std::string_view(reinterpret_cast<const char*>(kDawnVersion.data()),
                                                kDawnVersion.size())));
    return key;
}

TEST(BlobCacheTests, StoreAndLoad) {
    MockCppCallback<size_t (*)(std::span<const std::byte>, std::span<std::byte>)> mockLoadCallback;
    MockCppCallback<void (*)(std::span<const std::byte>, std::span<const std::byte>)>
        mockStoreCallback;

    wgpu::DawnCacheDeviceDescriptor desc = {};
    desc.SetDawnLoadCacheDataCallback(mockLoadCallback.TemplatedCallback(), &mockLoadCallback);
    desc.SetDawnStoreCacheDataCallback(mockStoreCallback.TemplatedCallback(), &mockStoreCallback);

    BlobCache cache(*FromCppAPI(&desc), false);

    CacheKey key = CreateValidKey();
    const std::vector<std::byte> value = {std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4}};

    // Expect Store
    EXPECT_CALL(mockStoreCallback, Call(_, _))
        .WillOnce([&](std::span<const std::byte> k, std::span<const std::byte> v) {
            EXPECT_TRUE(std::ranges::equal(k, key));
            EXPECT_TRUE(std::ranges::equal(v, value));
        });
    cache.Store(key, value);

    // Expect Load
    EXPECT_CALL(mockLoadCallback, Call(_, _))
        .WillOnce([&](std::span<const std::byte> k, std::span<std::byte> v) -> size_t {
            EXPECT_TRUE(std::ranges::equal(k, key));
            EXPECT_TRUE(v.empty());
            return value.size();
        })
        .WillOnce([&](std::span<const std::byte> k, std::span<std::byte> v) -> size_t {
            EXPECT_TRUE(std::ranges::equal(k, key));
            EXPECT_EQ(v.size(), value.size());
            std::ranges::copy(value, v.begin());
            return value.size();
        });

    auto result = cache.Load(key);
    EXPECT_TRUE(result.IsSuccess());
    Blob loadedBlob = result.AcquireSuccess();

    EXPECT_EQ(loadedBlob.Size(), value.size());
    EXPECT_TRUE(std::ranges::equal(loadedBlob.Data(), value));
}

}  // namespace
}  // namespace dawn::native
