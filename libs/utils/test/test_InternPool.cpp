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

#include <gtest/gtest.h>

#include <utils/FixedCapacityVector.h>
#include <utils/InternPool.h>
#include <utils/Slice.h>

using namespace utils;

TEST(InternPoolTest, AcquireWithCopy) {
    InternPool<int> pool;

    FixedCapacityVector<int> value = { 1, 3, 3, 7 };
    Slice<const int> interned = pool.acquire(value);

    EXPECT_FALSE(pool.empty());
    EXPECT_EQ(value.as_slice(), interned);
}

TEST(InternPoolTest, AcquireWithMove) {
    InternPool<int> pool;

    FixedCapacityVector<int> value = { 1, 3, 3, 7 };
    FixedCapacityVector<int> copy = value;
    const int* data = value.data();
    Slice<const int> interned = pool.acquire(std::move(value));

    EXPECT_FALSE(pool.empty());
    EXPECT_EQ(copy.as_slice(), interned);
    EXPECT_EQ(data, interned.data());
}

TEST(InternPoolTest, InternIsUnique) {
    InternPool<int> pool;

    FixedCapacityVector<int> value = { 1, 3, 3, 7 };
    Slice<const int> interned1 = pool.acquire(value);
    Slice<const int> interned2 = pool.acquire(value);
    Slice<const int> interned3 = pool.acquire(value);

    EXPECT_FALSE(pool.empty());
    EXPECT_EQ(interned1.begin(), interned2.begin());
    EXPECT_EQ(interned1.begin(), interned3.begin());
    EXPECT_EQ(interned1.end(), interned2.end());
    EXPECT_EQ(interned1.end(), interned3.end());
}

TEST(InternPoolTest, ReleaseByValue) {
    InternPool<int> pool;

    FixedCapacityVector<int> value = { 1, 3, 3, 7 };
    Slice<const int> interned = pool.acquire(value);

    EXPECT_FALSE(pool.empty());

    pool.release(value);

    EXPECT_TRUE(pool.empty());
}

TEST(InternPoolTest, ReleaseByInterned) {
    InternPool<int> pool;

    FixedCapacityVector<int> value = { 1, 3, 3, 7 };
    Slice<const int> interned = pool.acquire(value);

    EXPECT_FALSE(pool.empty());

    pool.release(interned);

    EXPECT_TRUE(pool.empty());
}

TEST(InternPoolTest, AcquireAndReleaseEmpty) {
    InternPool<int> pool;

    FixedCapacityVector<int> value = {};
    Slice<const int> interned = pool.acquire(value);

    EXPECT_TRUE(pool.empty());
    EXPECT_EQ(interned.begin(), nullptr);
    EXPECT_EQ(interned.end(), nullptr);

    // Shouldn't crash to release an empty slice even if the pool is empty.
    pool.release(value);
    pool.release(interned);
}

TEST(InternPoolTest, AcquireAndReleaseManyEqual) {
    InternPool<int> pool;

    FixedCapacityVector<int> value = { 1, 3, 3, 7 };
    pool.acquire(value);
    pool.acquire(value);
    pool.acquire(value);

    EXPECT_FALSE(pool.empty());

    pool.release(value);
    EXPECT_FALSE(pool.empty());
    pool.release(value);
    EXPECT_FALSE(pool.empty());
    pool.release(value);
    EXPECT_TRUE(pool.empty());

#ifdef GTEST_HAS_DEATH_TEST
    ASSERT_DEATH(pool.release(value), "");
#endif
}

TEST(InternPoolTest, AcquireAndReleaseManyDifferent) {
    InternPool<int> pool;

    FixedCapacityVector<int> value1 = { 1, 3, 3, 7 };
    FixedCapacityVector<int> value2 = { 4, 2, 0 };
    FixedCapacityVector<int> value3 = { 9999999 };
    pool.acquire(value1);
    pool.acquire(value2);
    pool.acquire(value3);

    EXPECT_FALSE(pool.empty());

    pool.release(value1);
    EXPECT_FALSE(pool.empty());
    pool.release(value2);
    EXPECT_FALSE(pool.empty());
    pool.release(value3);
    EXPECT_TRUE(pool.empty());
}

#ifdef GTEST_HAS_DEATH_TEST
TEST(InternPoolTest, PanicsIfReleaseMissing) {
    InternPool<int> pool;

    FixedCapacityVector<int> value = { 1, 3, 3, 7 };

    ASSERT_DEATH(pool.release(value), "");
}
#endif // GTEST_HAS_DEATH_TEST
