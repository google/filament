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

#include <utils/PagedArenaBitsetPool.h>

#include <gtest/gtest.h>

#include <cstdint>
#include <utility>
#include <vector>

using namespace utils;

TEST(PagedArenaBitsetPoolTest, BasicCheckoutAndReturn) {
    PagedArenaBitsetPool pool;

    PagedArenaBitset* address = nullptr;

    {
        PagedArenaBitsetPool::ScopedBitset bs = pool.get();
        EXPECT_NE(bs.get(), nullptr);
        address = bs.get();

        // Modify the bitset
        bs->add(10);
        bs->add(20);
        EXPECT_EQ(bs->size(), 2);
        EXPECT_TRUE((*bs)[10]);
        EXPECT_TRUE((*bs)[20]);
    } // bs goes out of scope, should clear and return to pool

    {
        PagedArenaBitsetPool::ScopedBitset bs2 = pool.get();
        // Verify we got the exact same physical bitset back
        EXPECT_EQ(bs2.get(), address);

        // Verify it was sterile/cleared automatically
        EXPECT_TRUE(bs2->empty());
        EXPECT_EQ(bs2->size(), 0);
        EXPECT_FALSE((*bs2)[10]);
        EXPECT_FALSE((*bs2)[20]);
    }
}

TEST(PagedArenaBitsetPoolTest, MoveSemantics) {
    PagedArenaBitsetPool pool;

    PagedArenaBitsetPool::ScopedBitset bs1 = pool.get();
    PagedArenaBitset* address = bs1.get();
    bs1->add(42);

    // Move constructor
    PagedArenaBitsetPool::ScopedBitset bs2 = std::move(bs1);
    EXPECT_EQ(bs1.get(), nullptr);
    EXPECT_EQ(bs2.get(), address);
    EXPECT_TRUE((*bs2)[42]);

    // Move assignment
    PagedArenaBitsetPool::ScopedBitset bs3 = pool.get();
    PagedArenaBitset* address3 = bs3.get();
    
    bs3 = std::move(bs2);
    // bs3's original bitset (address3) should be returned to the pool
    // bs2 should be null, bs3 should own bs1's bitset
    EXPECT_EQ(bs2.get(), nullptr);
    EXPECT_EQ(bs3.get(), address);

    // Let's verify address3 was returned to the pool and is sterile
    PagedArenaBitsetPool::ScopedBitset bs4 = pool.get();
    EXPECT_EQ(bs4.get(), address3);
    EXPECT_TRUE(bs4->empty());
}

TEST(PagedArenaBitsetPoolTest, ConversionOperator) {
    PagedArenaBitsetPool pool;
    PagedArenaBitsetPool::ScopedBitset scoped = pool.get();
    scoped->add(100);

    // Transparent conversion operator to PagedArenaBitset&
    PagedArenaBitset& ref = scoped;
    EXPECT_TRUE(ref[100]);
    EXPECT_EQ(ref.size(), 1);
}

TEST(PagedArenaBitsetPoolTest, HighWaterMarkScaling) {
    PagedArenaBitsetPool pool;

    PagedArenaBitset* addr1 = nullptr;
    PagedArenaBitset* addr2 = nullptr;
    PagedArenaBitset* addr3 = nullptr;

    {
        PagedArenaBitsetPool::ScopedBitset bs1 = pool.get();
        PagedArenaBitsetPool::ScopedBitset bs2 = pool.get();
        PagedArenaBitsetPool::ScopedBitset bs3 = pool.get();

        addr1 = bs1.get();
        addr2 = bs2.get();
        addr3 = bs3.get();

        EXPECT_NE(addr1, addr2);
        EXPECT_NE(addr2, addr3);
        EXPECT_NE(addr1, addr3);
    } // All returned

    // Now check them out again, they should be returned in LIFO order (LIFO or FIFO depending on vector pop_back/push_back)
    // push_back/pop_back is LIFO
    {
        PagedArenaBitsetPool::ScopedBitset bsA = pool.get();
        PagedArenaBitsetPool::ScopedBitset bsB = pool.get();
        PagedArenaBitsetPool::ScopedBitset bsC = pool.get();

        EXPECT_EQ(bsA.get(), addr1);
        EXPECT_EQ(bsB.get(), addr2);
        EXPECT_EQ(bsC.get(), addr3);
    }
}

TEST(PagedArenaBitsetPoolTest, ClearPool) {
    PagedArenaBitsetPool pool;

    PagedArenaBitset* address1 = nullptr;
    PagedArenaBitset* address2 = nullptr;

    {
        PagedArenaBitsetPool::ScopedBitset bs1 = pool.get();
        PagedArenaBitsetPool::ScopedBitset bs2 = pool.get();
        address1 = bs1.get();
        address2 = bs2.get();
    } // Both returned to pool's free-list

    // Clear the pool to free all resources
    pool.clear();

    // Checkout one bitset. Since the pool was cleared, this will allocate a new one.
    PagedArenaBitsetPool::ScopedBitset bs3 = pool.get();
    // Checkout a second one. This will also allocate a new one.
    PagedArenaBitsetPool::ScopedBitset bs4 = pool.get();

    // Verify they are distinct from each other
    EXPECT_NE(bs3.get(), bs4.get());
}

TEST(PagedArenaBitsetPoolTest, PoolSize) {
    PagedArenaBitsetPool pool;
    EXPECT_EQ(pool.size(), 0);

    {
        PagedArenaBitsetPool::ScopedBitset bs1 = pool.get();
        EXPECT_EQ(pool.size(), 0); // Active, not in free-list

        {
            PagedArenaBitsetPool::ScopedBitset bs2 = pool.get();
            EXPECT_EQ(pool.size(), 0);
        } // bs2 returned

        EXPECT_EQ(pool.size(), 1);
    } // bs1 returned

    EXPECT_EQ(pool.size(), 2);

    pool.clear();
    EXPECT_EQ(pool.size(), 0);
}

TEST(PagedArenaBitsetPoolTest, DefaultConstructor) {
    PagedArenaBitsetPool pool;
    
    // Default constructed empty wrapper
    PagedArenaBitsetPool::ScopedBitset empty_wrapper;
    EXPECT_EQ(empty_wrapper.get(), nullptr);
    EXPECT_FALSE(static_cast<bool>(empty_wrapper));

    // Assign a valid checked-out wrapper to it
    empty_wrapper = pool.get();
    EXPECT_NE(empty_wrapper.get(), nullptr);
    EXPECT_TRUE(static_cast<bool>(empty_wrapper));
}
