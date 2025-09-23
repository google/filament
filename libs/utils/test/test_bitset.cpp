/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include <utils/bitset.h>

using namespace utils;

TEST(BitSetTest, bitset256) {
    bitset256 b;

    EXPECT_EQ(0, b.count());
    EXPECT_TRUE(b == b);

    b.set(0);
    EXPECT_EQ(1, b.count());
    EXPECT_TRUE(b == b);

    b.set(64);
    EXPECT_EQ(2, b.count());
    EXPECT_TRUE(b == b);

    b.set(128);
    EXPECT_EQ(3, b.count());
    EXPECT_TRUE(b == b);

    b.set(192);
    EXPECT_EQ(4, b.count());
    EXPECT_TRUE(b == b);

    b = ~b;
    EXPECT_EQ(252, b.count());
    EXPECT_TRUE(b == b);

    EXPECT_TRUE(b.any());
    EXPECT_TRUE((b & ~b).none());
    EXPECT_TRUE((b | ~b).all());
}

TEST(BitSetTest, LargeBitset) {
    bitset<uint64_t, 2 * 31> b;
    b = ~b;
    EXPECT_EQ(64 * 2 * 31, b.count());
}

TEST(BitSetTest, SetBit) {
    bitset8 b;
    b.set(1, true);
    EXPECT_TRUE(b.test(1));
    b.set(1, false);
    EXPECT_FALSE(b.test(1));
}

TEST(BitSetTest, TestOperator) {
    bitset8 b;
    b.set(1, true);
    EXPECT_TRUE(b[1]);
    b.set(1, false);
    EXPECT_FALSE(b[1]);
}

TEST(BitSetTest, Count) {
    bitset8 b;
    EXPECT_EQ(0, b.count());
    b.set(1, true);
    b.set(3, true);
    b.set(4, true);
    EXPECT_EQ(3, b.count());
}

TEST(BitSetTest, Reset) {
    bitset8 b;
    b.set(1, true);
    b.set(3, true);
    b.set(4, true);
    b.reset();
    EXPECT_TRUE(b.none());
}

TEST(BitSetTest, AnyBit) {
    bitset8 b;
    EXPECT_FALSE(b.any());
    b.set(1, true);
    EXPECT_TRUE(b.any());
}

TEST(BitSetTest, None) {
    bitset8 b;
    EXPECT_TRUE(b.none());
    b.set(1, true);
    b.reset();
    EXPECT_TRUE(b.none());
}

TEST(BitSetTest, All) {
    bitset8 b;
    EXPECT_FALSE(b.all());
    b.set(1, true);
    EXPECT_FALSE(b.all());
    b.set(0, true);
    b.set(2, true);
    b.set(3, true);
    b.set(4, true);
    b.set(5, true);
    b.set(6, true);
    b.set(7, true);
    EXPECT_TRUE(b.all());
}

TEST(BitSetTest, FlipBit) {
    bitset8 b;
    b.flip(1);
    EXPECT_TRUE(b.test(1));
    b.flip(1);
    EXPECT_FALSE(b.test(1));
}

TEST(BitSetTest, SetUnset) {
    bitset8 b;
    b.set(1);
    EXPECT_TRUE(b.test(1));
    b.unset(1);
    EXPECT_FALSE(b.test(1));
}

TEST(BitSetTest, AndOperator) {
    bitset8 b1;
    b1.set(1);

    bitset8 b2;
    b2.set(2);

    bitset8 b3 = b1 & b2;
    EXPECT_FALSE(b3[1]);
    EXPECT_FALSE(b3[2]);
    EXPECT_TRUE(b3.none());

    b2.reset();
    b2.set(1);

    b3 = b1 & b2;
    EXPECT_TRUE(b3[1]);

    b1 &= b2;
    EXPECT_EQ(b3, b1);
}

TEST(BitSetTest, OrOperator) {
    bitset8 b1;
    b1.set(1);

    bitset8 b2;
    b2.set(2);

    bitset8 b3 = b1 | b2;
    EXPECT_TRUE(b3[1]);
    EXPECT_TRUE(b3[2]);

    b1 |= b2;
    EXPECT_EQ(b3, b1);
}

TEST(BitSetTest, XorOperator) {
    bitset8 b1;
    b1.set(1);

    bitset8 b2;
    b2.set(2);

    bitset8 b3 = b1 ^ b2;
    EXPECT_TRUE(b3[1]);
    EXPECT_TRUE(b3[2]);

    b2.reset();
    b2.set(1);

    b3 = b1 ^ b2;
    EXPECT_FALSE(b3[1]);
    EXPECT_FALSE(b3[2]);

    b1 ^= b2;
    EXPECT_EQ(b3, b1);
}

TEST(BitSetTest, NotOperator) {
    bitset8 b1;
    b1.set(1);

    bitset8 b3 = ~b1;
    EXPECT_FALSE(b3[1]);
    EXPECT_TRUE(b3[0]);
    EXPECT_TRUE(b3[2]);
}

TEST(BitSetTest, FirstSetBit) {
    bitset256 b;
    // Without a set bit, we expect an value out-of-bounds.
    EXPECT_GT(b.firstSetBit(), b.size() - 1);

    b.set(64);
    EXPECT_EQ(64, b.firstSetBit());

    b.set(63);
    EXPECT_EQ(63, b.firstSetBit());

    b.set(0);
    EXPECT_EQ(0, b.firstSetBit());

    b.set(128);
    EXPECT_EQ(0, b.firstSetBit());

    b.set(255);
    EXPECT_EQ(0, b.firstSetBit());

    b.unset(0);
    EXPECT_EQ(63, b.firstSetBit());

    b.unset(63);
    EXPECT_EQ(64, b.firstSetBit());

    b.unset(64);
    EXPECT_EQ(128, b.firstSetBit());

    b.unset(128);
    EXPECT_EQ(255, b.firstSetBit());

    b.unset(255);
    // Without a set bit, we expect an value out-of-bounds.
    EXPECT_GT(b.firstSetBit(), b.size() - 1);
}
