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

#include <utils/FixedCapacityVector.h>
#include <utils/InternPool.h>
#include <utils/Slice.h>

#include <gtest/gtest.h>

using namespace utils;

using Ref = InternPool<int>::Ref;

TEST(InternPoolTest, AcquireWithCopy) {
    InternPool<int> pool;

    FixedCapacityVector<int> value = { 1, 3, 3, 7 };
    Ref interned = pool.acquire(value);

    EXPECT_FALSE(pool.empty());
    EXPECT_EQ(value.as_slice(), interned.get());
}

TEST(InternPoolTest, AcquireWithMove) {
    InternPool<int> pool;

    FixedCapacityVector<int> value = { 1, 3, 3, 7 };
    FixedCapacityVector<int> copy = value;
    const int* data = value.data();
    Ref interned = pool.acquire(std::move(value));

    EXPECT_FALSE(pool.empty());
    EXPECT_EQ(copy.as_slice(), interned.get());
    EXPECT_EQ(data, interned.get().data());
}

TEST(InternPoolTest, InternIsUnique) {
    InternPool<int> pool;

    FixedCapacityVector<int> value = { 1, 3, 3, 7 };
    Ref interned1 = pool.acquire(value);
    Ref interned2 = pool.acquire(value);
    Ref interned3 = pool.acquire(value);

    EXPECT_FALSE(pool.empty());
    EXPECT_EQ(interned1.get().begin(), interned2.get().begin());
    EXPECT_EQ(interned1.get().begin(), interned3.get().begin());
    EXPECT_EQ(interned1.get().end(), interned2.get().end());
    EXPECT_EQ(interned1.get().end(), interned3.get().end());
}

TEST(InternPoolTest, ReleasesWhenTheLastReferenceGoesAway) {
    InternPool<int> pool;

    FixedCapacityVector<int> value = { 1, 3, 3, 7 };
    {
        Ref interned = pool.acquire(value);
        EXPECT_FALSE(pool.empty());
    }

    EXPECT_TRUE(pool.empty());
}

TEST(InternPoolTest, AcquireEmpty) {
    InternPool<int> pool;

    FixedCapacityVector<int> value = {};
    Ref interned = pool.acquire(value);

    EXPECT_TRUE(pool.empty());
    EXPECT_TRUE(interned.empty());
    EXPECT_EQ(interned.get().begin(), nullptr);
    EXPECT_EQ(interned.get().end(), nullptr);

    // Shouldn't crash when the empty reference is destroyed either.
}

TEST(InternPoolTest, AcquireAndReleaseManyEqual) {
    InternPool<int> pool;

    FixedCapacityVector<int> value = { 1, 3, 3, 7 };
    {
        Ref interned1 = pool.acquire(value);
        {
            Ref interned2 = pool.acquire(value);
            {
                Ref interned3 = pool.acquire(value);
                EXPECT_FALSE(pool.empty());
            }
            EXPECT_FALSE(pool.empty());
        }
        EXPECT_FALSE(pool.empty());
    }

    EXPECT_TRUE(pool.empty());
}

TEST(InternPoolTest, AcquireAndReleaseManyDifferent) {
    InternPool<int> pool;

    FixedCapacityVector<int> value1 = { 1, 3, 3, 7 };
    FixedCapacityVector<int> value2 = { 4, 2, 0 };
    FixedCapacityVector<int> value3 = { 9999999 };
    {
        Ref interned1 = pool.acquire(value1);
        {
            Ref interned2 = pool.acquire(value2);
            {
                Ref interned3 = pool.acquire(value3);
                EXPECT_FALSE(pool.empty());
            }
            EXPECT_FALSE(pool.empty());
        }
        EXPECT_FALSE(pool.empty());
    }

    EXPECT_TRUE(pool.empty());
}

TEST(InternPoolTest, MoveTransfersOwnership) {
    InternPool<int> pool;

    FixedCapacityVector<int> value = { 1, 3, 3, 7 };
    {
        Ref moved;
        {
            Ref interned = pool.acquire(value);
            moved = std::move(interned);

            // NOLINTNEXTLINE(bugprone-use-after-move): checking the moved-from state is the point.
            EXPECT_TRUE(interned.empty());
        }

        // The moved-from reference going out of scope must not have released the entry.
        EXPECT_FALSE(pool.empty());
        EXPECT_EQ(value.as_slice(), moved.get());
    }

    EXPECT_TRUE(pool.empty());
}

TEST(InternPoolTest, CloneAddsAReference) {
    InternPool<int> pool;

    FixedCapacityVector<int> value = { 1, 3, 3, 7 };
    {
        Ref clone;
        {
            Ref interned = pool.acquire(value);
            clone = interned.clone();
            EXPECT_EQ(interned.get(), clone.get());
        }

        EXPECT_FALSE(pool.empty());
        EXPECT_EQ(value.as_slice(), clone.get());
    }

    EXPECT_TRUE(pool.empty());
}

TEST(InternPoolTest, CloneOfAnEmptyReferenceIsEmpty) {
    Ref empty;
    Ref clone = empty.clone();

    EXPECT_TRUE(clone.empty());
}
