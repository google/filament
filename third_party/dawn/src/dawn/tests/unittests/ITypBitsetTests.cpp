// Copyright 2020 The Dawn & Tint Authors
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

#include <set>

#include "gtest/gtest.h"

#include "dawn/common/TypedInteger.h"
#include "dawn/common/ityp_bitset.h"

namespace dawn {
namespace {

class ITypBitsetTest : public testing::Test {
  protected:
    using Key = TypedInteger<struct KeyT, size_t>;
    using Bitset = ityp::bitset<Key, 9>;
    using Bitset40 = ityp::bitset<Key, 40>;

    // Test that the expected bitset methods can be constexpr
    struct ConstexprTest {
        static constexpr Bitset kBitset = {1 << 0 | 1 << 3 | 1 << 7 | 1 << 8};

        static_assert(kBitset[Key(0)] == true);
        static_assert(kBitset[Key(1)] == false);
        static_assert(kBitset[Key(2)] == false);
        static_assert(kBitset[Key(3)] == true);
        static_assert(kBitset[Key(4)] == false);
        static_assert(kBitset[Key(5)] == false);
        static_assert(kBitset[Key(6)] == false);
        static_assert(kBitset[Key(7)] == true);
        static_assert(kBitset[Key(8)] == true);

        static_assert(kBitset.size() == 9);
    };

    void ExpectBits(const Bitset& bits, std::set<size_t> indices) {
        size_t mask = 0;

        for (size_t i = 0; i < bits.size(); ++i) {
            if (indices.count(i) == 0) {
                ASSERT_FALSE(bits[Key(i)]) << i;
                ASSERT_FALSE(bits.test(Key(i))) << i;
            } else {
                mask |= (size_t(1) << i);
                ASSERT_TRUE(bits[Key(i)]) << i;
                ASSERT_TRUE(bits.test(Key(i))) << i;
            }
        }

        ASSERT_EQ(bits.to_ullong(), mask);
        ASSERT_EQ(bits.to_ulong(), mask);
        ASSERT_EQ(bits.count(), indices.size());
        ASSERT_EQ(bits.all(), indices.size() == bits.size());
        ASSERT_EQ(bits.any(), indices.size() != 0);
        ASSERT_EQ(bits.none(), indices.size() == 0);
    }
};

// Test that by default no bits are set
TEST_F(ITypBitsetTest, DefaultZero) {
    Bitset bits;
    ExpectBits(bits, {});
}

// Test the bitset can be initialized with a bitmask
TEST_F(ITypBitsetTest, InitializeByBits) {
    Bitset bits = {1 << 1 | 1 << 2 | 1 << 7};
    ExpectBits(bits, {1, 2, 7});
}

// Test that bits can be set at an index and retrieved from the same index.
TEST_F(ITypBitsetTest, Indexing) {
    Bitset bits;
    ExpectBits(bits, {});

    bits[Key(2)] = true;
    bits[Key(4)] = false;
    bits.set(Key(1));
    bits.set(Key(7), true);
    bits.set(Key(8), false);

    ExpectBits(bits, {1, 2, 7});

    bits.reset(Key(2));
    bits.reset(Key(7));
    ExpectBits(bits, {1});
}

// Test that bits can be flipped
TEST_F(ITypBitsetTest, Flip) {
    Bitset bits = {1 << 1 | 1 << 2 | 1 << 7};
    ExpectBits(bits, {1, 2, 7});

    bits.flip(Key(4));
    bits.flip(Key(1));  // false
    bits.flip(Key(6));
    bits.flip(Key(5));
    ExpectBits(bits, {2, 4, 5, 6, 7});

    bits.flip();
    ExpectBits(bits, {0, 1, 3, 8});

    ExpectBits(~bits, {2, 4, 5, 6, 7});
}

// Test that all the bits can be set/reset.
TEST_F(ITypBitsetTest, SetResetAll) {
    Bitset bits;

    bits.set();

    ASSERT_EQ(bits.count(), 9u);
    ASSERT_TRUE(bits.all());
    ASSERT_TRUE(bits.any());
    ASSERT_FALSE(bits.none());

    for (Key i(0); i < Key(9); ++i) {
        ASSERT_TRUE(bits[i]);
    }

    bits.reset();

    ASSERT_EQ(bits.count(), 0u);
    ASSERT_FALSE(bits.all());
    ASSERT_FALSE(bits.any());
    ASSERT_TRUE(bits.none());

    for (Key i(0); i < Key(9); ++i) {
        ASSERT_FALSE(bits[i]);
    }
}

// Test And operations
TEST_F(ITypBitsetTest, And) {
    Bitset bits = {1 << 1 | 1 << 2 | 1 << 7};
    ExpectBits(bits, {1, 2, 7});

    Bitset bits2 = bits & Bitset{1 << 0 | 1 << 3 | 1 << 7};
    ExpectBits(bits2, {7});
    ExpectBits(bits, {1, 2, 7});

    bits &= Bitset{1 << 1 | 1 << 6};
    ExpectBits(bits, {1});
}

// Test Or operations
TEST_F(ITypBitsetTest, Or) {
    Bitset bits = {1 << 1 | 1 << 2 | 1 << 7};
    ExpectBits(bits, {1, 2, 7});

    Bitset bits2 = bits | Bitset{1 << 0 | 1 << 3 | 1 << 7};
    ExpectBits(bits2, {0, 1, 2, 3, 7});
    ExpectBits(bits, {1, 2, 7});

    bits |= Bitset{1 << 1 | 1 << 6};
    ExpectBits(bits, {1, 2, 6, 7});
}

// Test xor operations
TEST_F(ITypBitsetTest, Xor) {
    Bitset bits = {1 << 1 | 1 << 2 | 1 << 7};
    ExpectBits(bits, {1, 2, 7});

    Bitset bits2 = bits ^ Bitset { 1 << 0 | 1 << 3 | 1 << 7 };
    ExpectBits(bits2, {0, 1, 2, 3});
    ExpectBits(bits, {1, 2, 7});

    bits ^= Bitset{1 << 1 | 1 << 6};
    ExpectBits(bits, {2, 6, 7});
}

// Testing the GetHighestBitIndexPlusOne function
TEST_F(ITypBitsetTest, GetHighestBitIndexPlusOne) {
    // <= 32 bit
    EXPECT_EQ(0u, static_cast<size_t>(GetHighestBitIndexPlusOne(Bitset(0b00))));
    EXPECT_EQ(1u, static_cast<size_t>(GetHighestBitIndexPlusOne(Bitset(0b01))));
    EXPECT_EQ(2u, static_cast<size_t>(GetHighestBitIndexPlusOne(Bitset(0b10))));
    EXPECT_EQ(2u, static_cast<size_t>(GetHighestBitIndexPlusOne(Bitset(0b11))));

    EXPECT_EQ(3u, static_cast<size_t>(GetHighestBitIndexPlusOne(Bitset{1 << 2})));
    EXPECT_EQ(9u, static_cast<size_t>(GetHighestBitIndexPlusOne(Bitset{1 << 8})));
    EXPECT_EQ(9u, static_cast<size_t>(GetHighestBitIndexPlusOne(Bitset{1 << 8 | 1 << 2})));

    // > 32 bit
    EXPECT_EQ(0u, static_cast<size_t>(GetHighestBitIndexPlusOne(Bitset40(0b00))));
    EXPECT_EQ(1u, static_cast<size_t>(GetHighestBitIndexPlusOne(Bitset40(0b01))));
    EXPECT_EQ(2u, static_cast<size_t>(GetHighestBitIndexPlusOne(Bitset40(0b10))));
    EXPECT_EQ(2u, static_cast<size_t>(GetHighestBitIndexPlusOne(Bitset40(0b11))));

    EXPECT_EQ(5u, static_cast<size_t>(GetHighestBitIndexPlusOne(Bitset40(0x10))));
    EXPECT_EQ(5u, static_cast<size_t>(GetHighestBitIndexPlusOne(Bitset40(0x1F))));
    EXPECT_EQ(16u, static_cast<size_t>(GetHighestBitIndexPlusOne(Bitset40(0xF000))));
    EXPECT_EQ(16u, static_cast<size_t>(GetHighestBitIndexPlusOne(Bitset40(0xFFFF))));
    EXPECT_EQ(32u, static_cast<size_t>(GetHighestBitIndexPlusOne(Bitset40(0xF0000000))));
    EXPECT_EQ(32u, static_cast<size_t>(GetHighestBitIndexPlusOne(Bitset40(0xFFFFFFFF))));
    EXPECT_EQ(36u, static_cast<size_t>(GetHighestBitIndexPlusOne(Bitset40(0xF00000000))));
    EXPECT_EQ(36u, static_cast<size_t>(GetHighestBitIndexPlusOne(Bitset40(0xFFFFFFFFF))));
    EXPECT_EQ(40u, static_cast<size_t>(GetHighestBitIndexPlusOne(Bitset40(0xF000000000))));
    EXPECT_EQ(40u, static_cast<size_t>(GetHighestBitIndexPlusOne(Bitset40(0xFFFFFFFFFF))));
}

}  // anonymous namespace
}  // namespace dawn
