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

#include <utils/LruCache.h>

#include <gtest/gtest.h>

#include <string>
#include <vector>

using namespace utils;

TEST(LruCacheTest, BasicPutGet) {
    LruCache<int, std::string> cache("LruCacheTest", 3);

    cache.put(1, "one", [](std::string&&) {});
    cache.put(2, "two", [](std::string&&) {});

    auto* v1 = cache.get(1);
    ASSERT_NE(v1, nullptr);
    EXPECT_EQ(*v1, "one");

    auto* v2 = cache.get(2);
    ASSERT_NE(v2, nullptr);
    EXPECT_EQ(*v2, "two");

    EXPECT_EQ(cache.size(), 2);
}

TEST(LruCacheTest, MissingKey) {
    LruCache<int, int> cache("LruCacheTest", 3);
    EXPECT_EQ(cache.get(999), nullptr);
}

TEST(LruCacheTest, Eviction) {
    LruCache<int, int> cache("LruCacheTest", 3);
    std::vector<int> evicted;
    auto releaser = [&](int&& v) { evicted.push_back(v); };

    cache.put(1, 10, releaser);
    cache.put(2, 20, releaser);
    cache.put(3, 30, releaser);

    // Cache: 3(MRU), 2, 1(LRU)
    EXPECT_EQ(cache.size(), 3);
    EXPECT_TRUE(evicted.empty());

    cache.put(4, 40, releaser);
    // Evicted 1. Cache: 4(MRU), 3, 2
    EXPECT_EQ(cache.size(), 3);
    ASSERT_EQ(evicted.size(), 1);
    EXPECT_EQ(evicted[0], 10);

    // Verify contents
    EXPECT_NE(cache.get(2), nullptr);
    EXPECT_NE(cache.get(3), nullptr);
    EXPECT_NE(cache.get(4), nullptr);
    EXPECT_EQ(cache.get(1), nullptr);

    // Accessing 2 makes it MRU. Cache: 2(MRU), 4, 3(LRU)
    cache.get(2);

    cache.put(5, 50, releaser);
    // Should evict 3.
    ASSERT_EQ(evicted.size(), 2);
    EXPECT_EQ(evicted[1], 30);
}

TEST(LruCacheTest, UpdatePromotesToMRU) {
    LruCache<int, int> cache("LruCacheTest", 3);
    auto releaser = [](int&&) {};

    cache.put(1, 10, releaser);
    cache.put(2, 20, releaser);
    cache.put(3, 30, releaser);
    // Order: 3, 2, 1

    cache.put(1, 11, releaser);
    // Order: 1, 3, 2

    cache.put(4, 40, releaser);
    // Order: 4, 1, 3, 2 (evicted)

    // Check if 1 exists
    auto* v1 = cache.get(1);
    ASSERT_NE(v1, nullptr);
    EXPECT_EQ(*v1, 11);
}

TEST(LruCacheTest, GetPromotesToMRU) {
    LruCache<int, int> cache("LruCacheTest", 3);
    std::vector<int> evicted;
    auto releaser = [&](int&& v) { evicted.push_back(v); };

    cache.put(1, 10, releaser);
    cache.put(2, 20, releaser);
    cache.put(3, 30, releaser);
    // 3, 2, 1

    cache.get(1);
    // 1, 3, 2

    cache.put(4, 40, releaser);
    // Evicts 2.
    ASSERT_EQ(evicted.size(), 1);
    EXPECT_EQ(evicted[0], 20);
}

TEST(LruCacheTest, MoveToFrontFromTail) {
    // This specific test targets the potential bug where moving from tail doesn't update tail.
    LruCache<int, int> cache("LruCacheTest", 3);
    std::vector<int> evicted;
    auto releaser = [&](int&& v) { evicted.push_back(v); };

    cache.put(1, 10, releaser);
    cache.put(2, 20, releaser);
    cache.put(3, 30, releaser);
    // State: 3 (MRU) -> 2 -> 1 (LRU, Tail)

    // Move 1 to front.
    cache.get(1);
    // Expected State: 1 (MRU) -> 3 -> 2 (LRU, Tail)

    // Add 4. Should evict 2.
    cache.put(4, 40, releaser);

    ASSERT_EQ(evicted.size(), 1);
    EXPECT_EQ(evicted[0], 20);
}

TEST(LruCacheTest, MoveToFrontFromMiddle) {
    LruCache<int, int> cache("LruCacheTest", 3);
    std::vector<int> evicted;
    auto releaser = [&](int&& v) { evicted.push_back(v); };

    cache.put(1, 10, releaser);
    cache.put(2, 20, releaser);
    cache.put(3, 30, releaser);
    // 3, 2, 1

    cache.get(2);
    // 2, 3, 1

    cache.put(4, 40, releaser);
    // Evicts 1.
    ASSERT_EQ(evicted.size(), 1);
    EXPECT_EQ(evicted[0], 10);
}

TEST(LruCacheTest, CapacityOne) {
    LruCache<int, int> cache("LruCacheTest", 1);
    std::vector<int> evicted;
    auto releaser = [&](int&& v) { evicted.push_back(v); };

    cache.put(1, 10, releaser);
    EXPECT_NE(cache.get(1), nullptr);
    EXPECT_EQ(*cache.get(1), 10);

    cache.put(2, 20, releaser);
    ASSERT_EQ(evicted.size(), 1);
    EXPECT_EQ(evicted[0], 10);
    EXPECT_NE(cache.get(2), nullptr);
    EXPECT_EQ(*cache.get(2), 20);
}

#ifdef GTEST_HAS_DEATH_TEST
TEST(LruCacheTest, CapacityZero) {
    LruCache<int, int> cache("LruCacheTest", 0);
    EXPECT_EQ(cache.capacity(), 0);
    EXPECT_EQ(cache.size(), 0);
    EXPECT_DEATH(cache.put(1, 1, [](int&&){}), "");
}
#endif

TEST(LruCacheTest, Clear) {
    LruCache<int, int> cache("LruCacheTest", 3);
    std::vector<int> evicted;
    auto releaser = [&](int&& v) { evicted.push_back(v); };

    cache.put(1, 10, releaser);
    cache.put(2, 20, releaser);

    EXPECT_EQ(cache.size(), 2);

    cache.clear(releaser);

    EXPECT_EQ(cache.size(), 0);
    ASSERT_EQ(evicted.size(), 2);
    // Implementation traverses mHead -> mTail (MRU to LRU).
    EXPECT_EQ(evicted[0], 20); // MRU
    EXPECT_EQ(evicted[1], 10); // LRU

    // We should now be able to refill the cache.
    cache.put(3, 30, releaser);
    cache.put(4, 40, releaser);

    EXPECT_EQ(cache.size(), 2);
    EXPECT_NE(cache.get(3), nullptr);
    EXPECT_NE(cache.get(4), nullptr);
}

TEST(LruCacheTest, Pop) {
    LruCache<int, int> cache("LruCacheTest", 3);
    auto releaser = [](int&&) {};

    cache.put(1, 10, releaser);
    cache.put(2, 20, releaser);

    std::optional<int> val1 = cache.pop(1);
    EXPECT_TRUE(val1.has_value());
    EXPECT_EQ(*val1, 10);
    EXPECT_EQ(cache.size(), 1);
    EXPECT_EQ(cache.get(1), nullptr);
    EXPECT_NE(cache.get(2), nullptr);

    std::optional<int> val2 = cache.pop(2);
    EXPECT_TRUE(val2.has_value());
    EXPECT_EQ(*val2, 20);
    EXPECT_EQ(cache.size(), 0);
}

/* Large key tests. */

struct LargeKey {
    long long a, b;
    bool operator==(LargeKey const& other) const {
        return a == other.a && b == other.b;
    }
};

struct LargeKeyHash {
    size_t operator()(LargeKey const& k) const {
        return std::hash<long long>{}(k.a) ^ std::hash<long long>{}(k.b);
    }
};

TEST(LruCacheTest, LargeKey) {
    static_assert(sizeof(LargeKey) > sizeof(void*), "LargeKey must be larger than pointer");
    LruCache<LargeKey, int, LargeKeyHash> cache("LruCacheTest", 2);

    LargeKey k1{1, 1};
    LargeKey k2{2, 2};
    LargeKey k3{3, 3};

    cache.put(k1, 100, [](int&&){});
    cache.put(k2, 200, [](int&&){});

    EXPECT_NE(cache.get(k1), nullptr);
    EXPECT_EQ(*cache.get(k1), 100);

    // Evict k2 (LRU because k1 was accessed)
    cache.put(k3, 300, [](int&&){});

    EXPECT_NE(cache.get(k3), nullptr); // k3 present
}
