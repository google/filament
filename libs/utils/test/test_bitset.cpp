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

#include <utils/bitset.h>

#include <gtest/gtest.h>

#include <new>

#include <stdint.h>

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

TEST(BitSetTest, ShiftOperatorsSingleWord) {
    bitset64 b;
    b.set(0);
    b.set(5);
    b.set(63);

    bitset64 l = b << 1;
    EXPECT_EQ(2u, l.count());
    EXPECT_TRUE(l[1]);
    EXPECT_TRUE(l[6]);

    bitset64 r = b >> 1;
    EXPECT_EQ(2u, r.count());
    EXPECT_TRUE(r[4]);
    EXPECT_TRUE(r[62]);

    EXPECT_EQ(b, b << 0);
    EXPECT_EQ(b, b >> 0);
    EXPECT_TRUE((b << 64).none());
    EXPECT_TRUE((b >> 64).none());

    bitset8 c(uint8_t(0x81));
    c >>= 1;
    EXPECT_EQ(0x40, c.getValue());
    c <<= 2;
    EXPECT_EQ(0x00, c.getValue());
}

TEST(BitSetTest, ShiftOperatorsMultiWord) {
    bitset256 b;
    b.set(0);
    b.set(63);
    b.set(130);
    b.set(255);

    bitset256 l = b << 1;
    EXPECT_EQ(3u, l.count());
    EXPECT_TRUE(l[1]);
    EXPECT_TRUE(l[64]);
    EXPECT_TRUE(l[131]);

    bitset256 r = b >> 1;
    EXPECT_EQ(3u, r.count());
    EXPECT_TRUE(r[62]);
    EXPECT_TRUE(r[129]);
    EXPECT_TRUE(r[254]);

    // Shifting by a whole number of words.
    bitset256 lw = b << 128;
    EXPECT_EQ(2u, lw.count());
    EXPECT_TRUE(lw[128]);
    EXPECT_TRUE(lw[191]);

    bitset256 rw = b >> 128;
    EXPECT_EQ(2u, rw.count());
    EXPECT_TRUE(rw[2]);
    EXPECT_TRUE(rw[127]);

    // Shifting by a word count plus an intra-word offset.
    bitset256 lm = b << 70;
    EXPECT_EQ(3u, lm.count());
    EXPECT_TRUE(lm[70]);
    EXPECT_TRUE(lm[133]);
    EXPECT_TRUE(lm[200]);

    bitset256 rm = b >> 70;
    EXPECT_EQ(2u, rm.count());
    EXPECT_TRUE(rm[60]);
    EXPECT_TRUE(rm[185]);

    // In-place shifts on a populated multi-word bitset.
    bitset256 ip = b;
    ip <<= 70;
    EXPECT_EQ(lm, ip);

    ip = b;
    ip >>= 70;
    EXPECT_EQ(rm, ip);

    EXPECT_TRUE((b << 256).none());
    EXPECT_TRUE((b >> 1000).none());
}

namespace {

template<typename Bitset>
UTILS_NOINLINE Bitset shiftLeftDynamic(Bitset const& b, size_t n) noexcept {
    return b << n;
}

template<typename Bitset>
UTILS_NOINLINE Bitset shiftRightDynamic(Bitset const& b, size_t n) noexcept {
    return b >> n;
}

template<typename Bitset>
UTILS_NOINLINE void shiftLeftAssignDynamic(Bitset& b, size_t n) noexcept {
    b <<= n;
}

template<typename Bitset>
UTILS_NOINLINE void shiftRightAssignDynamic(Bitset& b, size_t n) noexcept {
    b >>= n;
}

template<typename Bitset>
Bitset makePattern() noexcept {
    Bitset b;
    for (size_t i = 0; i < Bitset::BIT_COUNT; ++i) {
        // Set word boundaries and a non-periodic hash pattern across all words.
        bool const boundary = (i % Bitset::BITS_PER_WORD == 0) ||
                              (i % Bitset::BITS_PER_WORD == Bitset::BITS_PER_WORD - 1);
        bool const hashed = (((i * 1315423911u) ^ (i >> 3)) & 3u) == 0u;
        if (boundary || hashed) {
            b.set(i);
        }
    }
    return b;
}

template<typename Bitset>
void verifyShiftAmount(Bitset const& src, size_t const n) {
    Bitset expectedLeft;
    Bitset expectedRight;
    if (n < Bitset::BIT_COUNT) {
        for (size_t i = 0; i + n < Bitset::BIT_COUNT; ++i) {
            if (src.test(i)) {
                expectedLeft.set(i + n);
            }
            if (src.test(i + n)) {
                expectedRight.set(i);
            }
        }
    }

    EXPECT_EQ(expectedLeft, shiftLeftDynamic(src, n)) << "n=" << n;
    EXPECT_EQ(expectedRight, shiftRightDynamic(src, n)) << "n=" << n;

    Bitset inPlaceLeft = src;
    shiftLeftAssignDynamic(inPlaceLeft, n);
    EXPECT_EQ(expectedLeft, inPlaceLeft) << "n=" << n;

    Bitset inPlaceRight = src;
    shiftRightAssignDynamic(inPlaceRight, n);
    EXPECT_EQ(expectedRight, inPlaceRight) << "n=" << n;
}

template<typename Bitset>
void verifyAllShifts() {
    Bitset const pattern = makePattern<Bitset>();
    Bitset allOnes = ~Bitset{};
    for (size_t n = 0; n <= Bitset::BIT_COUNT + 8; ++n) {
        verifyShiftAmount(pattern, n);
        verifyShiftAmount(allOnes, n);
    }
}

} // namespace

TEST(BitSetTest, ShiftOperatorsRuntimeExhaustive) {
    // Single-word instantiations.
    verifyAllShifts<bitset8>();
    verifyAllShifts<bitset<uint16_t>>();
    verifyAllShifts<bitset32>();
    verifyAllShifts<bitset64>();

    // 128-bit instantiations (exercises the 128-bit fast path across different container types).
    verifyAllShifts<bitset128>();
    verifyAllShifts<bitset<uint32_t, 4>>();
    verifyAllShifts<bitset<uint8_t, 16>>();

    // Multi-word instantiations, including narrow unsigned word types subject to integer promotion.
    verifyAllShifts<bitset<uint8_t, 5>>();
    verifyAllShifts<bitset<uint16_t, 5>>();
    verifyAllShifts<bitset<uint32_t, 5>>();
    verifyAllShifts<bitset256>();
}

TEST(BitSetTest, ShiftOperatorsLargeRuntime) {
    using LargeBitset = bitset<uint64_t, 2 * 31>;
    LargeBitset const src = makePattern<LargeBitset>();
    size_t const shifts[] = {
        0, 1, 7, 63, 64, 65, 127, 128, 129, 255, 256, 257,
        1023, 1024, 1025, 3711, 3712, 3713, 3903, 3904, 3905,
        3967, 3968, 3969, 5000
    };
    for (size_t const n : shifts) {
        verifyShiftAmount(src, n);
    }
}

TEST(BitSetTest, EightByteAligned) {
    // Pool allocators only guarantee alignof(std::max_align_t), which can be 8, so every
    // operation must work on a bitset that isn't 16-byte aligned.
    alignas(16) uint64_t buffer[2 + 4 + 4];
    auto* a = new(buffer + 1) bitset256();
    auto* b = new(buffer + 5) bitset256();
    ASSERT_EQ(8, reinterpret_cast<uintptr_t>(a) % 16);
    ASSERT_EQ(8, reinterpret_cast<uintptr_t>(b) % 16);

    a->set(0);
    a->set(255);
    b->set(255);
    EXPECT_EQ(2, a->count());
    EXPECT_TRUE(a->any());
    EXPECT_FALSE(a->all());
    EXPECT_TRUE(*a != *b);
    EXPECT_EQ(1, (*a & *b).count());
    EXPECT_EQ(2, (*a | *b).count());
    EXPECT_EQ(1, (*a ^ *b).count());
    EXPECT_EQ(254, (~*a).count());
    EXPECT_TRUE((*a | ~*a).all());
}
