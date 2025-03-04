// Copyright 2017 The Dawn & Tint Authors
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

#include "dawn/common/BitSetIterator.h"
#include "dawn/common/ityp_bitset.h"
#include "gtest/gtest.h"

namespace dawn {
namespace {

// This is ANGLE's BitSetIterator_unittests.cpp file.

class BitSetIteratorTest : public testing::Test {
  protected:
    std::bitset<40> mStateBits;
};

// Simple iterator test.
TEST_F(BitSetIteratorTest, Iterator) {
    std::set<uint32_t> originalValues;
    originalValues.insert(2);
    originalValues.insert(6);
    originalValues.insert(8);
    originalValues.insert(35);

    for (uint32_t value : originalValues) {
        mStateBits.set(value);
    }

    std::set<uint32_t> readValues;
    for (uint32_t bit : IterateBitSet(mStateBits)) {
        EXPECT_EQ(1u, originalValues.count(bit));
        EXPECT_EQ(0u, readValues.count(bit));
        readValues.insert(bit);
    }

    EXPECT_EQ(originalValues.size(), readValues.size());
}

// Test an empty iterator.
TEST_F(BitSetIteratorTest, EmptySet) {
    // We don't use the FAIL gtest macro here since it returns immediately,
    // causing an unreachable code warning in MSVC
    bool sawBit = false;
    for ([[maybe_unused]] uint32_t bit : IterateBitSet(mStateBits)) {
        sawBit = true;
    }
    EXPECT_FALSE(sawBit);
}

// Test iterating a result of combining two bitsets.
TEST_F(BitSetIteratorTest, NonLValueBitset) {
    std::bitset<40> otherBits;

    mStateBits.set(1);
    mStateBits.set(2);
    mStateBits.set(3);
    mStateBits.set(4);

    otherBits.set(0);
    otherBits.set(1);
    otherBits.set(3);
    otherBits.set(5);

    std::set<uint32_t> seenBits;

    for (uint32_t bit : IterateBitSet(mStateBits & otherBits)) {
        EXPECT_EQ(0u, seenBits.count(bit));
        seenBits.insert(bit);
        EXPECT_TRUE(mStateBits[bit]);
        EXPECT_TRUE(otherBits[bit]);
    }

    EXPECT_EQ((mStateBits & otherBits).count(), seenBits.size());
}

class EnumBitSetIteratorTest : public testing::Test {
  protected:
    enum class TestEnum { A, B, C, D, E, F, G, H, I, J, EnumCount };

    static constexpr size_t kEnumCount = static_cast<size_t>(TestEnum::EnumCount);
    ityp::bitset<TestEnum, kEnumCount> mStateBits;
};

// Simple iterator test.
TEST_F(EnumBitSetIteratorTest, Iterator) {
    std::set<TestEnum> originalValues;
    originalValues.insert(TestEnum::B);
    originalValues.insert(TestEnum::F);
    originalValues.insert(TestEnum::C);
    originalValues.insert(TestEnum::I);

    for (TestEnum value : originalValues) {
        mStateBits.set(value);
    }

    std::set<TestEnum> readValues;
    for (TestEnum bit : IterateBitSet(mStateBits)) {
        EXPECT_EQ(1u, originalValues.count(bit));
        EXPECT_EQ(0u, readValues.count(bit));
        readValues.insert(bit);
    }

    EXPECT_EQ(originalValues.size(), readValues.size());
}

// Test an empty iterator.
TEST_F(EnumBitSetIteratorTest, EmptySet) {
    // We don't use the FAIL gtest macro here since it returns immediately,
    // causing an unreachable code warning in MSVC
    bool sawBit = false;
    for ([[maybe_unused]] TestEnum bit : IterateBitSet(mStateBits)) {
        sawBit = true;
    }
    EXPECT_FALSE(sawBit);
}

// Test iterating a result of combining two bitsets.
TEST_F(EnumBitSetIteratorTest, NonLValueBitset) {
    ityp::bitset<TestEnum, kEnumCount> otherBits;

    mStateBits.set(TestEnum::B);
    mStateBits.set(TestEnum::C);
    mStateBits.set(TestEnum::D);
    mStateBits.set(TestEnum::E);

    otherBits.set(TestEnum::A);
    otherBits.set(TestEnum::B);
    otherBits.set(TestEnum::D);
    otherBits.set(TestEnum::F);

    std::set<TestEnum> seenBits;

    for (TestEnum bit : IterateBitSet(mStateBits & otherBits)) {
        EXPECT_EQ(0u, seenBits.count(bit));
        seenBits.insert(bit);
        EXPECT_TRUE(mStateBits[bit]);
        EXPECT_TRUE(otherBits[bit]);
    }

    EXPECT_EQ((mStateBits & otherBits).count(), seenBits.size());
}

class ITypBitsetIteratorTest : public testing::Test {
  protected:
    using IntegerT = TypedInteger<struct Foo, uint32_t>;
    ityp::bitset<IntegerT, 40> mStateBits;
};

// Simple iterator test.
TEST_F(ITypBitsetIteratorTest, Iterator) {
    std::set<IntegerT> originalValues;
    originalValues.insert(IntegerT(2));
    originalValues.insert(IntegerT(6));
    originalValues.insert(IntegerT(8));
    originalValues.insert(IntegerT(35));

    for (IntegerT value : originalValues) {
        mStateBits.set(value);
    }

    std::set<IntegerT> readValues;
    for (IntegerT bit : IterateBitSet(mStateBits)) {
        EXPECT_EQ(1u, originalValues.count(bit));
        EXPECT_EQ(0u, readValues.count(bit));
        readValues.insert(bit);
    }

    EXPECT_EQ(originalValues.size(), readValues.size());
}

// Test an empty iterator.
TEST_F(ITypBitsetIteratorTest, EmptySet) {
    // We don't use the FAIL gtest macro here since it returns immediately,
    // causing an unreachable code warning in MSVC
    bool sawBit = false;
    for ([[maybe_unused]] IntegerT bit : IterateBitSet(mStateBits)) {
        sawBit = true;
    }
    EXPECT_FALSE(sawBit);
}

// Test iterating a result of combining two bitsets.
TEST_F(ITypBitsetIteratorTest, NonLValueBitset) {
    ityp::bitset<IntegerT, 40> otherBits;

    mStateBits.set(IntegerT(1));
    mStateBits.set(IntegerT(2));
    mStateBits.set(IntegerT(3));
    mStateBits.set(IntegerT(4));

    otherBits.set(IntegerT(0));
    otherBits.set(IntegerT(1));
    otherBits.set(IntegerT(3));
    otherBits.set(IntegerT(5));

    std::set<IntegerT> seenBits;

    for (IntegerT bit : IterateBitSet(mStateBits & otherBits)) {
        EXPECT_EQ(0u, seenBits.count(bit));
        seenBits.insert(bit);
        EXPECT_TRUE(mStateBits[bit]);
        EXPECT_TRUE(otherBits[bit]);
    }

    EXPECT_EQ((mStateBits & otherBits).count(), seenBits.size());
}

}  // anonymous namespace
}  // namespace dawn
