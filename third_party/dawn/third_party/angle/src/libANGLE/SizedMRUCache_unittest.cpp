//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SizedMRUCache_unittest.h: Unit tests for the sized MRU cached.

#include <gtest/gtest.h>

#include "libANGLE/SizedMRUCache.h"

namespace angle
{

using Blob = std::vector<uint8_t>;

Blob MakeBlob(size_t size)
{
    Blob blob;
    for (uint8_t value = 0; value < size; ++value)
    {
        blob.push_back(value);
    }
    return blob;
}

// Test a cache with a value that takes up maximum size.
TEST(SizedMRUCacheTest, MaxSizedValue)
{
    constexpr size_t kSize = 32;
    SizedMRUCache<std::string, Blob> sizedCache(kSize);

    EXPECT_TRUE(sizedCache.put("test", MakeBlob(kSize), kSize));
    EXPECT_EQ(32u, sizedCache.size());
    EXPECT_FALSE(sizedCache.empty());

    EXPECT_TRUE(sizedCache.put("test2", MakeBlob(kSize), kSize));
    EXPECT_EQ(32u, sizedCache.size());
    EXPECT_FALSE(sizedCache.empty());

    const Blob *blob = nullptr;
    EXPECT_FALSE(sizedCache.get("test", &blob));

    sizedCache.clear();
    EXPECT_TRUE(sizedCache.empty());
}

// Test a cache with many small values, that it can handle unlimited inserts.
TEST(SizedMRUCacheTest, ManySmallValues)
{
    constexpr size_t kSize = 32;
    SizedMRUCache<size_t, size_t> sizedCache(kSize);

    for (size_t value = 0; value < kSize; ++value)
    {
        size_t valueCopy = value;
        EXPECT_TRUE(sizedCache.put(value, std::move(valueCopy), 1));

        const size_t *qvalue = nullptr;
        EXPECT_TRUE(sizedCache.get(value, &qvalue));
        if (qvalue)
        {
            EXPECT_EQ(value, *qvalue);
        }
    }

    EXPECT_EQ(32u, sizedCache.size());
    EXPECT_FALSE(sizedCache.empty());

    // Putting one element evicts the first element.
    EXPECT_TRUE(sizedCache.put(kSize, std::move(static_cast<int>(kSize)), 1));

    const size_t *qvalue = nullptr;
    EXPECT_FALSE(sizedCache.get(0, &qvalue));

    // Putting one large element cleans out the whole stack.
    EXPECT_TRUE(sizedCache.put(kSize + 1, kSize + 1, kSize));
    EXPECT_EQ(32u, sizedCache.size());
    EXPECT_FALSE(sizedCache.empty());

    for (size_t value = 0; value <= kSize; ++value)
    {
        EXPECT_FALSE(sizedCache.get(value, &qvalue));
    }
    EXPECT_TRUE(sizedCache.get(kSize + 1, &qvalue));
    if (qvalue)
    {
        EXPECT_EQ(kSize + 1, *qvalue);
    }

    // Put a bunch of items in the cache sequentially.
    for (size_t value = 0; value < kSize * 10; ++value)
    {
        size_t valueCopy = value;
        EXPECT_TRUE(sizedCache.put(value, std::move(valueCopy), 1));
    }

    EXPECT_EQ(32u, sizedCache.size());
}

// Tests putting an oversize element.
TEST(SizedMRUCacheTest, OversizeValue)
{
    constexpr size_t kSize = 32;
    SizedMRUCache<size_t, size_t> sizedCache(kSize);

    EXPECT_FALSE(sizedCache.put(5, 5, 100));
}

}  // namespace angle
