/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include <utils/RefCountedMap.h>

#include <gtest/gtest.h>

using namespace utils;

using KeyType = size_t;
using ValueType = size_t;
using PlainPointerType = size_t*;
using SmartPointerType = std::unique_ptr<size_t>;

/* Value types */

TEST(RefCountedMapTest, ValueType_AcquireAndRelease) {
    RefCountedMap<KeyType, ValueType> map;

    ValueType* a1 = map.acquire(1, []() { return 1; });
    EXPECT_NE(a1, nullptr);
    EXPECT_EQ(*a1, 1);
    EXPECT_FALSE(map.empty());

    map.release(1);
    EXPECT_TRUE(map.empty());
}

TEST(RefCountedMapTest, ValueType_AcquireAndReleaseMany) {
    RefCountedMap<KeyType, ValueType> map;

    ValueType* a1 = map.acquire(1, []() { return 1; });
    EXPECT_NE(a1, nullptr);
    EXPECT_EQ(*a1, 1);
    ValueType* a2 = map.acquire(1, []() {
        ADD_FAILURE();
        return 1;
    });
    EXPECT_EQ(a1, a2);
    ValueType* a3 = map.acquire(1, []() {
        ADD_FAILURE();
        return 1;
    });
    EXPECT_EQ(a1, a3);
    EXPECT_FALSE(map.empty());

    map.release(1, [](ValueType& it) { ADD_FAILURE(); });
    EXPECT_FALSE(map.empty());
    map.release(1, [](ValueType& it) { ADD_FAILURE(); });
    EXPECT_FALSE(map.empty());
    map.release(1);
    EXPECT_TRUE(map.empty());
}

TEST(RefCountedMapTest, ValueType_GetsValue) {
    RefCountedMap<KeyType, ValueType> map;
    RefCountedMap<KeyType, ValueType> const& constMap = map;

    map.acquire(1, []() { return 1; });
    ValueType& v1 = map.get(1);
    EXPECT_EQ(v1, 1);

    ValueType const& v1const = constMap.get(1);
    EXPECT_EQ(v1const, 1);
}

TEST(RefCountedMapTest, ValueType_CanReleaseNullValue) {
    RefCountedMap<KeyType, ValueType> map;
    map.acquire(1);
    map.release(1, [](ValueType& it) { ADD_FAILURE(); });
}

#ifdef GTEST_HAS_DEATH_TEST
TEST(RefCountedMapTest, ValueType_PanicsIfReleaseMissing) {
    RefCountedMap<KeyType, ValueType> map;
    ASSERT_DEATH(map.release(1), "");
}

TEST(RefCountedMapTest, ValueType_PanicsIfGetsMissing) {
    RefCountedMap<KeyType, ValueType> map;
    ASSERT_DEATH(map.get(1), "");
}

TEST(RefCountedMapTest, ValueType_PanicsIfGetsNullValue) {
    RefCountedMap<KeyType, ValueType> map;
    map.acquire(1);
    ASSERT_DEATH(map.get(1), "");
}
#endif // GTEST_HAS_DEATH_TEST

/* Plain pointer types */

TEST(RefCountedMapTest, PlainPointerType_AcquireAndRelease) {
    RefCountedMap<KeyType, PlainPointerType> map;

    ValueType* a1 = map.acquire(1, []() { return new size_t(1); });
    EXPECT_NE(a1, nullptr);
    EXPECT_EQ(*a1, 1);
    EXPECT_FALSE(map.empty());

    map.release(1, [](ValueType& it) { delete &it; });
    EXPECT_TRUE(map.empty());
}

TEST(RefCountedMapTest, PlainPointerType_AcquireAndReleaseMany) {
    RefCountedMap<KeyType, PlainPointerType> map;

    ValueType* a1 = map.acquire(1, []() { return new size_t(1); });
    EXPECT_NE(a1, nullptr);
    EXPECT_EQ(*a1, 1);
    ValueType* a2 = map.acquire(1, []() {
        ADD_FAILURE();
        return new size_t(1);
    });
    EXPECT_EQ(a1, a2);
    ValueType* a3 = map.acquire(1, []() {
        ADD_FAILURE();
        return new size_t(1);
    });
    EXPECT_EQ(a1, a3);
    EXPECT_FALSE(map.empty());

    map.release(1, [](ValueType& it) {
        ADD_FAILURE();
        delete &it;
    });
    EXPECT_FALSE(map.empty());
    map.release(1, [](ValueType& it) {
        ADD_FAILURE();
        delete &it;
    });
    EXPECT_FALSE(map.empty());
    map.release(1, [](ValueType& it) { delete &it; });
    EXPECT_TRUE(map.empty());
}

TEST(RefCountedMapTest, PlainPointerType_GetsValue) {
    RefCountedMap<KeyType, PlainPointerType> map;
    RefCountedMap<KeyType, PlainPointerType> const& constMap = map;

    ValueType* a1 = map.acquire(1, []() { return new size_t(1); });
    ValueType& v1 = map.get(1);
    EXPECT_EQ(v1, 1);

    ValueType const& v1const = constMap.get(1);
    EXPECT_EQ(v1const, 1);

    delete a1;
}

TEST(RefCountedMapTest, PlainPointerType_CanReleaseNullValue) {
    RefCountedMap<KeyType, PlainPointerType> map;
    map.acquire(1);
    map.release(1, [](ValueType& it) { ADD_FAILURE(); });
}

#ifdef GTEST_HAS_DEATH_TEST
TEST(RefCountedMapTest, PlainPointerType_PanicsIfReleaseMissing) {
    RefCountedMap<KeyType, PlainPointerType> map;
    ASSERT_DEATH(map.release(1), "");
}

TEST(RefCountedMapTest, PlainPointerType_PanicsIfGetsMissing) {
    RefCountedMap<KeyType, PlainPointerType> map;
    ASSERT_DEATH(map.get(1), "");
}

TEST(RefCountedMapTest, PlainPointerType_PanicsIfGetsNullValue) {
    RefCountedMap<KeyType, PlainPointerType> map;
    map.acquire(1);
    ASSERT_DEATH(map.get(1), "");
}
#endif // GTEST_HAS_DEATH_TEST

/* Smart pointer types */

TEST(RefCountedMapTest, SmartPointerType_AcquireAndRelease) {
    RefCountedMap<KeyType, SmartPointerType> map;

    ValueType* a1 = map.acquire(1, []() { return std::make_unique<size_t>(1); });
    EXPECT_NE(a1, nullptr);
    EXPECT_EQ(*a1, 1);
    EXPECT_FALSE(map.empty());

    map.release(1);
    EXPECT_TRUE(map.empty());
}

TEST(RefCountedMapTest, SmartPointerType_AcquireAndReleaseMany) {
    RefCountedMap<KeyType, SmartPointerType> map;

    ValueType* a1 = map.acquire(1, []() { return std::make_unique<size_t>(1); });
    EXPECT_NE(a1, nullptr);
    EXPECT_EQ(*a1, 1);
    ValueType* a2 = map.acquire(1, []() {
        ADD_FAILURE();
        return std::make_unique<size_t>(1);
    });
    EXPECT_EQ(a1, a2);
    ValueType* a3 = map.acquire(1, []() {
        ADD_FAILURE();
        return std::make_unique<size_t>(1);
    });
    EXPECT_EQ(a1, a3);
    EXPECT_FALSE(map.empty());

    map.release(1);
    EXPECT_FALSE(map.empty());
    map.release(1);
    EXPECT_FALSE(map.empty());
    map.release(1);
    EXPECT_TRUE(map.empty());
}

TEST(RefCountedMapTest, SmartPointerType_GetsValue) {
    RefCountedMap<KeyType, SmartPointerType> map;
    RefCountedMap<KeyType, SmartPointerType> const& constMap = map;

    map.acquire(1, []() { return std::make_unique<size_t>(1); });

    ValueType& v1 = map.get(1);
    EXPECT_EQ(v1, 1);

    ValueType const& v1const = constMap.get(1);
    EXPECT_EQ(v1const, 1);
}

TEST(RefCountedMapTest, SmartPointerType_CanReleaseNullValue) {
    RefCountedMap<KeyType, SmartPointerType> map;
    map.acquire(1);
    map.release(1, [](ValueType& it) { ADD_FAILURE(); });
}

#ifdef GTEST_HAS_DEATH_TEST
TEST(RefCountedMapTest, SmartPointerType_PanicsIfReleaseMissing) {
    RefCountedMap<KeyType, SmartPointerType> map;
    ASSERT_DEATH(map.release(1), "");
}

TEST(RefCountedMapTest, SmartPointerType_PanicsIfGetsMissing) {
    RefCountedMap<KeyType, SmartPointerType> map;
    ASSERT_DEATH(map.get(1), "");
}

TEST(RefCountedMapTest, SmartPointerType_PanicsIfGetsNullValue) {
    RefCountedMap<KeyType, SmartPointerType> map;
    map.acquire(1);
    ASSERT_DEATH(map.get(1), "");
}
#endif // GTEST_HAS_DEATH_TEST

struct TaggedKey {
    size_t id;
    size_t tag;
    mutable size_t* lastComparedTag = nullptr;
    bool operator==(TaggedKey const& rhs) const {
        if (lastComparedTag) *lastComparedTag = rhs.tag;
        if (rhs.lastComparedTag) *rhs.lastComparedTag = tag;
        return id == rhs.id;
    }
    struct Hash {
        size_t operator()(TaggedKey const& k) const { return std::hash<size_t>{}(k.id); }
    };
};

TEST(RefCountedMapTest, LruRecycling) {
    RefCountedMap<TaggedKey, SmartPointerType, TaggedKey::Hash> map("RefCountedMapTest", 1);
    bool factoryCalled = false;
    auto factory = [&]() {
        factoryCalled = true;
        return std::make_unique<size_t>(100);
    };

    // 1. Acquire K1 with stored key (tag = 100). Ref=1.
    ValueType* v1 = map.acquire(TaggedKey{ 1, 100 }, factory);
    ASSERT_NE(v1, nullptr);
    EXPECT_EQ(*v1, 100);
    EXPECT_TRUE(factoryCalled);

    // 2. Release K1 using a distinct lookup key (tag = 200). Ref=0. Moves entry to LRU.
    map.release(TaggedKey{ 1, 200 });
    EXPECT_TRUE(map.empty());

    // 3. Acquire K1 again using another distinct lookup key (tag = 300).
    // Verifies that step 2 (release) stored the original key (tag = 100) into the LRU cache
    // rather than the temporary lookup key (tag = 200).
    size_t storedInLruTag = 0;
    factoryCalled = false;
    ValueType* v2 = map.acquire(TaggedKey{ 1, 300, &storedInLruTag }, factory);
    ASSERT_NE(v2, nullptr);
    EXPECT_EQ(*v2, 100);
    EXPECT_FALSE(factoryCalled);
    EXPECT_EQ(v1, v2);
    EXPECT_EQ(storedInLruTag, 100u);

    // 4. Release K1 using lookup key (tag = 400).
    // Verifies that step 3 (acquire) re-inserted the stored key (tag = 100) from the LRU cache
    // into mMap rather than the temporary lookup key (tag = 300).
    size_t storedInMapTag = 0;
    map.release(TaggedKey{ 1, 400, &storedInMapTag });
    EXPECT_EQ(storedInMapTag, 100u);
}

TEST(RefCountedMapTest, ClearLruCache) {
    RefCountedMap<KeyType, ValueType> map("RefCountedMapTest", 2);
    int destroyed = 0;
    auto releaser = [&](ValueType& v) { destroyed++; };

    // Acquire and release two items to fill LRU
    map.acquire(1, []{ return 10; });
    map.release(1, releaser);
    map.acquire(2, []{ return 20; });
    map.release(2, releaser);

    // LRU size 2. Destroyed 0.
    EXPECT_EQ(destroyed, 0);

    map.clearLruCache(releaser);

    EXPECT_EQ(destroyed, 2);

    // Check they are gone (revival fails)
    bool factoryCalled = false;
    map.acquire(1, [&]{ factoryCalled = true; return 11; });
    EXPECT_TRUE(factoryCalled);

    map.release(1, releaser);
}
