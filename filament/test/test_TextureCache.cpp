/*
 * Copyright (C) 2026 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "MockDriver.h"
#include "TextureCache.h"

#include <filament/Engine.h>

#include <private/backend/CommandBufferQueue.h>
#include <private/backend/CommandStream.h>
#include <private/backend/DriverApi.h>

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>

namespace {

using namespace filament;
using namespace filament::backend;

class TextureCacheTest : public ::testing::Test {
protected:
    static constexpr size_t MIN_COMMAND_BUFFERS_SIZE = 1 * 1024 * 1024;
    static constexpr size_t COMMAND_BUFFERS_SIZE = 3 * MIN_COMMAND_BUFFERS_SIZE;

    static constexpr std::array<TextureSwizzle, 4> DEFAULT_SWIZZLE = {
        TextureSwizzle::CHANNEL_0,
        TextureSwizzle::CHANNEL_1,
        TextureSwizzle::CHANNEL_2,
        TextureSwizzle::CHANNEL_3,
    };

    TextureCacheTest()
            : mCommandBufferQueue(MIN_COMMAND_BUFFERS_SIZE, COMMAND_BUFFERS_SIZE,
                      /*paused=*/false),
              mCommandStream(mMockDriver, mCommandBufferQueue.getCircularBuffer()),
              mDriverApi(mCommandStream) {}

    TextureHandle allocTexture(TextureCache& cache, uint32_t width, uint32_t height = 64) {
        return cache.createTexture("test", SamplerType::SAMPLER_2D, 1, TextureFormat::RGBA8, 1,
                width, height, 1, DEFAULT_SWIZZLE, TextureUsage::COLOR_ATTACHMENT);
    }

    MockDriver mMockDriver;
    CommandBufferQueue mCommandBufferQueue;
    CommandStream mCommandStream;
    DriverApi& mDriverApi;
};

TEST_F(TextureCacheTest, SkippedFramePreservesLastRenderedFrameAndPurgesOlder) {
    Engine::Config config{};
    config.resourceAllocatorCacheMaxAge = 1;
    TextureCache cache{ config, mDriverApi };

    // Frame 0: allocate and return 3 distinct textures.
    TextureHandle const t100 = allocTexture(cache, 100);
    TextureHandle const t200 = allocTexture(cache, 200);
    TextureHandle const t300 = allocTexture(cache, 300);
    cache.destroyTexture(t100);
    cache.destroyTexture(t200);
    cache.destroyTexture(t300);
    cache.gc(false);

    // Frame 1: reuse only t100; t200 and t300 remain unused from Frame 0.
    TextureHandle const t100_f1 = allocTexture(cache, 100);
    EXPECT_EQ(t100_f1.getId(), t100.getId());
    cache.destroyTexture(t100_f1);
    // At end of Frame 1, t200 and t300 have ageDiff == 1 >= mCacheMaxAge (1).
    // Only MAX_EVICTION_COUNT (1) texture (t200) is evicted; t300 stays in the cache.
    cache.gc(false);

    // Frame 2 & 3 (skipped frames): should purge t300 (from Frame 0, ageDiff == 2 > 1)
    // while keeping t100 (from Frame 1, the last rendered frame, ageDiff == 1).
    cache.gc(true);
    cache.gc(true);

    // Next rendered frame: t100 must be a cache hit, while t200 and t300 must be cache misses.
    TextureHandle const t100_f4 = allocTexture(cache, 100);
    TextureHandle const t200_f4 = allocTexture(cache, 200);
    TextureHandle const t300_f4 = allocTexture(cache, 300);
    EXPECT_EQ(t100_f4.getId(), t100.getId());
    EXPECT_NE(t200_f4.getId(), t200.getId());
    EXPECT_NE(t300_f4.getId(), t300.getId());

    cache.destroyTexture(t100_f4);
    cache.destroyTexture(t200_f4);
    cache.destroyTexture(t300_f4);
    cache.terminate();
}

TEST_F(TextureCacheTest, AgedOutRateLimitedEviction) {
    Engine::Config config{};
    config.resourceAllocatorCacheMaxAge = 1;
    TextureCache cache{ config, mDriverApi };

    // Frame 0: allocate and return 3 distinct textures.
    TextureHandle const t100 = allocTexture(cache, 100);
    TextureHandle const t200 = allocTexture(cache, 200);
    TextureHandle const t300 = allocTexture(cache, 300);
    cache.destroyTexture(t100);
    cache.destroyTexture(t200);
    cache.destroyTexture(t300);
    cache.gc(false);

    // Frame 1 (no textures used): all 3 have ageDiff == 1, only 1 (t100) is evicted.
    cache.gc(false);

    // Frame 2 (no textures used): all remaining have ageDiff == 2, only 1 (t200) is evicted.
    cache.gc(false);

    // t300 should still be in the cache, while t100 and t200 were evicted.
    TextureHandle const t300_check = allocTexture(cache, 300);
    TextureHandle const t100_check = allocTexture(cache, 100);
    TextureHandle const t200_check = allocTexture(cache, 200);
    EXPECT_EQ(t300_check.getId(), t300.getId());
    EXPECT_NE(t100_check.getId(), t100.getId());
    EXPECT_NE(t200_check.getId(), t200.getId());

    cache.destroyTexture(t300_check);
    cache.destroyTexture(t100_check);
    cache.destroyTexture(t200_check);
    cache.terminate();
}

TEST_F(TextureCacheTest, UniqueAgeLimitRetainsThreeBuckets) {
    Engine::Config config{};
    config.resourceAllocatorCacheMaxAge = 10;
    TextureCache cache{ config, mDriverApi };

    // Populate 3 distinct age buckets across 3 frames:
    // Frame 0: bucket A (two textures)
    TextureHandle const tA1 = allocTexture(cache, 100);
    TextureHandle const tA2 = allocTexture(cache, 101);
    cache.destroyTexture(tA1);
    cache.destroyTexture(tA2);
    cache.gc(false);

    // Frame 1: bucket B
    TextureHandle const tB = allocTexture(cache, 200);
    cache.destroyTexture(tB);
    cache.gc(false);

    // Frame 2: bucket C -> now 3 unique ages {0, 1, 2} are in the cache.
    // Since MAX_UNIQUE_AGE_COUNT is 3, all 3 buckets must survive.
    TextureHandle const tC = allocTexture(cache, 300);
    cache.destroyTexture(tC);
    cache.gc(false);

    // Verify bucket A's first texture is still cached (re-check it out and return it at Frame 3,
    // while tA2 remains at Frame 0's age!).
    // Wait: if we leave tA1 and tA2 untouched and instead add Frame 3 (bucket D):
    // In Frame 3, before adding bucket D, let's verify that after 3 buckets tA1 and tA2 survived:
    // If we add bucket D in Frame 3, ages before unique-age eviction are {0, 1, 2, 3} (4 buckets).
    // That must evict bucket A (ageDiff == 3) while retaining buckets B (ageDiff == 2),
    // C (ageDiff == 1), and D (ageDiff == 0).
    TextureHandle const tD = allocTexture(cache, 400);
    cache.destroyTexture(tD);
    cache.gc(false);

    // Bucket A (ageDiff == 3 at Frame 3 gc) must have been completely purged (both tA1 and tA2),
    // whereas Bucket B (ageDiff == 2), Bucket C (ageDiff == 1), and Bucket D (ageDiff == 0)
    // must all still be in the cache!
    TextureHandle const tB_check = allocTexture(cache, 200);
    TextureHandle const tC_check = allocTexture(cache, 300);
    TextureHandle const tD_check = allocTexture(cache, 400);
    TextureHandle const tA1_check = allocTexture(cache, 100);
    TextureHandle const tA2_check = allocTexture(cache, 101);

    EXPECT_EQ(tB_check.getId(), tB.getId());
    EXPECT_EQ(tC_check.getId(), tC.getId());
    EXPECT_EQ(tD_check.getId(), tD.getId());
    EXPECT_NE(tA1_check.getId(), tA1.getId());
    EXPECT_NE(tA2_check.getId(), tA2.getId());

    cache.destroyTexture(tB_check);
    cache.destroyTexture(tC_check);
    cache.destroyTexture(tD_check);
    cache.destroyTexture(tA1_check);
    cache.destroyTexture(tA2_check);
    cache.terminate();
}

} // anonymous namespace

