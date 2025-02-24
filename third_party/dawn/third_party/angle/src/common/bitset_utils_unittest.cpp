//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// bitset_utils_unittest:
//   Tests bitset helpers and custom classes.
//

#include <array>

#include <gtest/gtest.h>

#include "common/bitset_utils.h"

using namespace angle;

namespace
{
template <typename T>
class BitSetTest : public testing::Test
{
  protected:
    T mBits;
    typedef T BitSet;
};

using BitSetTypes = ::testing::Types<BitSet<12>, BitSet32<12>, BitSet64<12>>;
TYPED_TEST_SUITE(BitSetTest, BitSetTypes);

// Basic test of various bitset functionalities
TYPED_TEST(BitSetTest, Basic)
{
    EXPECT_EQ(TypeParam::Zero().bits(), 0u);

    TypeParam mBits = this->mBits;
    EXPECT_FALSE(mBits.all());
    EXPECT_FALSE(mBits.any());
    EXPECT_TRUE(mBits.none());
    EXPECT_EQ(mBits.count(), 0u);

    // Set every bit to 1.
    for (size_t i = 0; i < mBits.size(); ++i)
    {
        mBits.set(i);

        EXPECT_EQ(mBits.all(), i + 1 == mBits.size());
        EXPECT_TRUE(mBits.any());
        EXPECT_FALSE(mBits.none());
        EXPECT_EQ(mBits.count(), i + 1);
    }

    // Reset every other bit to 0.
    for (size_t i = 0; i < mBits.size(); i += 2)
    {
        mBits.reset(i);

        EXPECT_FALSE(mBits.all());
        EXPECT_TRUE(mBits.any());
        EXPECT_FALSE(mBits.none());
        EXPECT_EQ(mBits.count(), mBits.size() - i / 2 - 1);
    }

    // Flip all bits.
    for (size_t i = 0; i < mBits.size(); ++i)
    {
        mBits.flip(i);

        EXPECT_FALSE(mBits.all());
        EXPECT_TRUE(mBits.any());
        EXPECT_FALSE(mBits.none());
        EXPECT_EQ(mBits.count(), mBits.size() / 2 + (i % 2 == 0));
    }

    // Make sure the bit pattern is what we expect at this point.
    for (size_t i = 0; i < mBits.size(); ++i)
    {
        EXPECT_EQ(mBits.test(i), i % 2 == 0);
        EXPECT_EQ(static_cast<bool>(mBits[i]), i % 2 == 0);
    }

    // Test that flip, set and reset all bits at once work.
    mBits.flip();
    EXPECT_FALSE(mBits.all());
    EXPECT_TRUE(mBits.any());
    EXPECT_FALSE(mBits.none());
    EXPECT_EQ(mBits.count(), mBits.size() / 2);
    EXPECT_EQ(mBits.first(), 1u);

    mBits.set();
    EXPECT_TRUE(mBits.all());
    EXPECT_TRUE(mBits.any());
    EXPECT_FALSE(mBits.none());
    EXPECT_EQ(mBits.count(), mBits.size());
    EXPECT_EQ(mBits.first(), 0u);

    mBits.reset();
    EXPECT_FALSE(mBits.all());
    EXPECT_FALSE(mBits.any());
    EXPECT_TRUE(mBits.none());
    EXPECT_EQ(mBits.count(), 0u);
}

// Test of gaps detection
TYPED_TEST(BitSetTest, Gaps)
{
    // Test and cache all bitsets with no gaps
    std::vector<TypeParam> gaplessValues;
    for (size_t i = 0; i <= TypeParam::size(); ++i)
    {
        EXPECT_FALSE(TypeParam::Mask(i).hasGaps());
        gaplessValues.push_back(TypeParam::Mask(i));
    }

    // Explicitly check all 8-bit masks
    for (size_t i = 0; i < 256; ++i)
    {
        const TypeParam bits = TypeParam(i);
        const bool notFound =
            std::find(gaplessValues.begin(), gaplessValues.end(), bits) == std::end(gaplessValues);
        EXPECT_EQ(bits.hasGaps(), notFound);
    }
}

// Test that BitSetT's initializer list constructor works correctly.
TYPED_TEST(BitSetTest, InitializerList)
{
    TypeParam bits = TypeParam{
        2, 5, 6, 9, 10,
    };

    for (size_t i = 0; i < bits.size(); ++i)
    {
        if (i == 2 || i == 5 || i == 6 || i == 9 || i == 10)
        {
            EXPECT_TRUE(bits[i]) << i;
        }
        else
        {
            EXPECT_FALSE(bits[i]) << i;
        }
    }
}

// Test bitwise operations
TYPED_TEST(BitSetTest, BitwiseOperators)
{
    TypeParam mBits = this->mBits;
    // Use a value that has a 1 in the 12th bit, to make sure masking to exactly 12 bits does not
    // have an off-by-one error.
    constexpr uint32_t kSelfValue  = 0x9E4;
    constexpr uint32_t kOtherValue = 0xC6A;

    constexpr uint32_t kMask             = (1 << 12) - 1;
    constexpr uint32_t kSelfMaskedValue  = kSelfValue & kMask;
    constexpr uint32_t kOtherMaskedValue = kOtherValue & kMask;

    constexpr uint32_t kShift            = 3;
    constexpr uint32_t kSelfShiftedLeft  = kSelfMaskedValue << kShift & kMask;
    constexpr uint32_t kSelfShiftedRight = kSelfMaskedValue >> kShift & kMask;

    mBits |= kSelfValue;
    typename TestFixture::BitSet other(kOtherValue);
    typename TestFixture::BitSet anded(kSelfMaskedValue & kOtherMaskedValue);
    typename TestFixture::BitSet ored(kSelfMaskedValue | kOtherMaskedValue);
    typename TestFixture::BitSet xored(kSelfMaskedValue ^ kOtherMaskedValue);

    EXPECT_EQ(mBits.bits(), kSelfMaskedValue);
    EXPECT_EQ(other.bits(), kOtherMaskedValue);

    EXPECT_EQ(mBits & other, anded);
    EXPECT_EQ(mBits | other, ored);
    EXPECT_EQ(mBits ^ other, xored);

    EXPECT_NE(mBits, other);
    EXPECT_NE(anded, ored);
    EXPECT_NE(anded, xored);
    EXPECT_NE(ored, xored);

    mBits &= other;
    EXPECT_EQ(mBits, anded);

    mBits |= ored;
    EXPECT_EQ(mBits, ored);

    mBits ^= other;
    mBits ^= anded;
    EXPECT_EQ(mBits, typename TestFixture::BitSet(kSelfValue));

    EXPECT_EQ(mBits << kShift, typename TestFixture::BitSet(kSelfShiftedLeft));
    EXPECT_EQ(mBits >> kShift, typename TestFixture::BitSet(kSelfShiftedRight));

    mBits <<= kShift;
    EXPECT_EQ(mBits, typename TestFixture::BitSet(kSelfShiftedLeft));
    EXPECT_EQ(mBits.bits() & ~kMask, 0u);

    mBits = typename TestFixture::BitSet(kSelfValue);
    mBits >>= kShift;
    EXPECT_EQ(mBits, typename TestFixture::BitSet(kSelfShiftedRight));
    EXPECT_EQ(mBits.bits() & ~kMask, 0u);

    mBits |= kSelfMaskedValue;
    EXPECT_EQ(mBits.bits() & ~kMask, 0u);
    mBits ^= kOtherMaskedValue;
    EXPECT_EQ(mBits.bits() & ~kMask, 0u);
}

// Test BitSetT::Mask
TYPED_TEST(BitSetTest, Mask)
{
    // Test constexpr usage
    TypeParam bits = TypeParam::Mask(0);
    EXPECT_EQ(bits.bits(), 0u);

    bits = TypeParam::Mask(1);
    EXPECT_EQ(bits.bits(), 1u);

    bits = TypeParam::Mask(2);
    EXPECT_EQ(bits.bits(), 3u);

    bits = TypeParam::Mask(TypeParam::size());
    EXPECT_EQ(bits.bits(), (1u << TypeParam::size()) - 1);

    // Complete test
    for (size_t i = 0; i < TypeParam::size(); ++i)
    {
        bits = TypeParam::Mask(i);
        EXPECT_EQ(bits.bits(), (1u << i) - 1);
    }
}

// Tests removing bits from the iterator during iteration.
TYPED_TEST(BitSetTest, ResetLaterBits)
{
    TypeParam bits;
    std::set<size_t> expectedValues;
    for (size_t i = 1; i < TypeParam::size(); i += 2)
    {
        expectedValues.insert(i);
    }

    for (size_t i = 1; i < TypeParam::size(); ++i)
        bits.set(i);

    // Remove the even bits
    TypeParam resetBits;
    for (size_t i = 2; i < TypeParam::size(); i += 2)
    {
        resetBits.set(i);
    }

    std::set<size_t> actualValues;

    for (auto iter = bits.begin(), end = bits.end(); iter != end; ++iter)
    {
        if (*iter == 1)
        {
            iter.resetLaterBits(resetBits);
        }

        actualValues.insert(*iter);
    }

    EXPECT_EQ(expectedValues, actualValues);
}

template <typename T>
class BitSetIteratorTest : public testing::Test
{
  protected:
    T mStateBits;
    typedef T BitSet;
};

using BitSetIteratorTypes = ::testing::Types<BitSet<40>, BitSet64<40>>;
TYPED_TEST_SUITE(BitSetIteratorTest, BitSetIteratorTypes);

// Simple iterator test.
TYPED_TEST(BitSetIteratorTest, Iterator)
{
    TypeParam mStateBits = this->mStateBits;
    std::set<size_t> originalValues;
    originalValues.insert(2);
    originalValues.insert(6);
    originalValues.insert(8);
    originalValues.insert(35);

    for (size_t value : originalValues)
    {
        mStateBits.set(value);
    }

    std::set<size_t> readValues;
    for (size_t bit : mStateBits)
    {
        EXPECT_EQ(1u, originalValues.count(bit));
        EXPECT_EQ(0u, readValues.count(bit));
        readValues.insert(bit);
    }

    EXPECT_EQ(originalValues.size(), readValues.size());
}

// Ensure lsb->msb iteration order.
TYPED_TEST(BitSetIteratorTest, IterationOrder)
{
    TypeParam mStateBits                   = this->mStateBits;
    const std::array<size_t, 8> writeOrder = {20, 25, 16, 31, 10, 14, 36, 19};
    const std::array<size_t, 8> fetchOrder = {10, 14, 16, 19, 20, 25, 31, 36};

    for (size_t value : writeOrder)
    {
        mStateBits.set(value);
    }
    EXPECT_EQ(writeOrder.size(), mStateBits.count());

    size_t i = 0;
    for (size_t bit : mStateBits)
    {
        EXPECT_EQ(fetchOrder[i], bit);
        i++;
    }
    EXPECT_EQ(fetchOrder.size(), mStateBits.count());
}

// Test an empty iterator.
TYPED_TEST(BitSetIteratorTest, EmptySet)
{
    TypeParam mStateBits = this->mStateBits;
    // We don't use the FAIL gtest macro here since it returns immediately,
    // causing an unreachable code warning in MSVS
    bool sawBit = false;
    for (size_t bit : mStateBits)
    {
        sawBit = true;
        ANGLE_UNUSED_VARIABLE(bit);
    }
    EXPECT_FALSE(sawBit);
}

// Test iterating a result of combining two bitsets.
TYPED_TEST(BitSetIteratorTest, NonLValueBitset)
{
    TypeParam mStateBits = this->mStateBits;
    typename TestFixture::BitSet otherBits;

    mStateBits.set(1);
    mStateBits.set(2);
    mStateBits.set(3);
    mStateBits.set(4);

    otherBits.set(0);
    otherBits.set(1);
    otherBits.set(3);
    otherBits.set(5);

    std::set<size_t> seenBits;

    typename TestFixture::BitSet maskedBits = (mStateBits & otherBits);
    for (size_t bit : maskedBits)
    {
        EXPECT_EQ(0u, seenBits.count(bit));
        seenBits.insert(bit);
        EXPECT_TRUE(mStateBits[bit]);
        EXPECT_TRUE(otherBits[bit]);
    }

    EXPECT_EQ((mStateBits & otherBits).count(), seenBits.size());
}

// Test bit assignments.
TYPED_TEST(BitSetIteratorTest, BitAssignment)
{
    TypeParam mStateBits = this->mStateBits;
    std::set<size_t> originalValues;
    originalValues.insert(2);
    originalValues.insert(6);
    originalValues.insert(8);
    originalValues.insert(35);

    for (size_t value : originalValues)
    {
        (mStateBits[value] = false) = true;
    }

    for (size_t value : originalValues)
    {
        EXPECT_TRUE(mStateBits.test(value));
    }
}

// Tests adding bits to the iterator during iteration.
TYPED_TEST(BitSetIteratorTest, SetLaterBit)
{
    TypeParam mStateBits            = this->mStateBits;
    std::set<size_t> expectedValues = {0, 1, 3, 5, 6, 7, 9, 35};
    mStateBits.set(0);
    mStateBits.set(1);

    std::set<size_t> actualValues;

    for (auto iter = mStateBits.begin(), end = mStateBits.end(); iter != end; ++iter)
    {
        if (*iter == 1)
        {
            iter.setLaterBit(3);
            iter.setLaterBit(5);
        }
        if (*iter == 5)
        {
            iter.setLaterBit(6);
            iter.setLaterBit(7);
            iter.setLaterBit(9);
            iter.setLaterBit(35);
        }

        actualValues.insert(*iter);
    }

    EXPECT_EQ(expectedValues, actualValues);
}

// Tests removing bit from the iterator during iteration.
TYPED_TEST(BitSetIteratorTest, ResetLaterBit)
{
    TypeParam mStateBits            = this->mStateBits;
    std::set<size_t> expectedValues = {1, 3, 5, 7, 9};

    for (size_t index = 1; index <= 9; ++index)
        mStateBits.set(index);

    std::set<size_t> actualValues;

    for (auto iter = mStateBits.begin(), end = mStateBits.end(); iter != end; ++iter)
    {
        if (*iter == 1)
        {
            iter.resetLaterBit(2);
            iter.resetLaterBit(4);
            iter.resetLaterBit(6);
            iter.resetLaterBit(8);
        }

        actualValues.insert(*iter);
    }

    EXPECT_EQ(expectedValues, actualValues);
}

// Tests adding bitsets to the iterator during iteration.
TYPED_TEST(BitSetIteratorTest, SetLaterBits)
{
    TypeParam mStateBits            = this->mStateBits;
    std::set<size_t> expectedValues = {1, 2, 3, 4, 5, 7, 9};
    mStateBits.set(1);
    mStateBits.set(2);
    mStateBits.set(3);

    TypeParam laterBits;
    laterBits.set(4);
    laterBits.set(5);
    laterBits.set(7);
    laterBits.set(9);

    std::set<size_t> actualValues;

    for (auto iter = mStateBits.begin(), end = mStateBits.end(); iter != end; ++iter)
    {
        if (*iter == 3)
        {
            iter.setLaterBits(laterBits);
        }

        EXPECT_EQ(actualValues.count(*iter), 0u);
        actualValues.insert(*iter);
    }

    EXPECT_EQ(expectedValues, actualValues);
}

template <typename T>
class BitSetArrayTest : public testing::Test
{
  protected:
    T mBitSet;
};

using BitSetArrayTypes =
    ::testing::Types<BitSetArray<65>, BitSetArray<128>, BitSetArray<130>, BitSetArray<511>>;
TYPED_TEST_SUITE(BitSetArrayTest, BitSetArrayTypes);

// Basic test of various BitSetArray functionalities
TYPED_TEST(BitSetArrayTest, BasicTest)
{
    TypeParam &mBits = this->mBitSet;

    EXPECT_FALSE(mBits.all());
    EXPECT_FALSE(mBits.any());
    EXPECT_TRUE(mBits.none());
    EXPECT_EQ(mBits.count(), 0u);
    EXPECT_EQ(mBits.bits(0), 0u);

    // Verify set on a single bit
    mBits.set(45);
    for (auto bit : mBits)
    {
        EXPECT_EQ(bit, 45u);
    }

    EXPECT_EQ(mBits.first(), 45u);
    EXPECT_EQ(mBits.last(), 45u);

    mBits.reset(45);

    // Set every bit to 1.
    for (size_t i = 0; i < mBits.size(); ++i)
    {
        mBits.set(i);

        EXPECT_EQ(mBits.all(), i + 1 == mBits.size());
        EXPECT_TRUE(mBits.any());
        EXPECT_FALSE(mBits.none());
        EXPECT_EQ(mBits.count(), i + 1);
    }

    // Reset odd bits to 0.
    for (size_t i = 1; i < mBits.size(); i += 2)
    {
        mBits.reset(i);

        EXPECT_FALSE(mBits.all());
        EXPECT_TRUE(mBits.any());
        EXPECT_FALSE(mBits.none());
        EXPECT_EQ(mBits.count(), mBits.size() - i / 2 - 1);
    }

    // Make sure the bit pattern is what we expect at this point.
    // All even bits should be set
    for (size_t i = 0; i < mBits.size(); ++i)
    {
        EXPECT_EQ(mBits.test(i), i % 2 == 0);
        EXPECT_EQ(static_cast<bool>(mBits[i]), i % 2 == 0);
    }

    // Reset everything.
    mBits.reset();
    EXPECT_FALSE(mBits.all());
    EXPECT_FALSE(mBits.any());
    EXPECT_TRUE(mBits.none());
    EXPECT_EQ(mBits.count(), 0u);

    // Test intersection logic
    TypeParam testBitSet;
    testBitSet.set(1);
    EXPECT_EQ(testBitSet.bits(0), (1ul << 1ul));
    testBitSet.set(3);
    EXPECT_EQ(testBitSet.bits(0), (1ul << 1ul) | (1ul << 3ul));
    testBitSet.set(5);
    EXPECT_EQ(testBitSet.bits(0), (1ul << 1ul) | (1ul << 3ul) | (1ul << 5ul));
    EXPECT_FALSE(mBits.intersects(testBitSet));
    mBits.set(3);
    EXPECT_TRUE(mBits.intersects(testBitSet));
    mBits.set(4);
    EXPECT_TRUE(mBits.intersects(testBitSet));
    mBits.reset(3);
    EXPECT_FALSE(mBits.intersects(testBitSet));
    testBitSet.set(4);
    EXPECT_TRUE(mBits.intersects(testBitSet));

    // Test that flip works.
    // Reset everything.
    mBits.reset();
    EXPECT_EQ(mBits.count(), 0u);
    mBits.flip();
    EXPECT_EQ(mBits.count(), mBits.size());

    // Test operators

    // Assignment operators - "=", "&=", "|=" and "^="
    mBits.reset();
    mBits = testBitSet;
    for (auto bit : testBitSet)
    {
        EXPECT_TRUE(mBits.test(bit));
    }

    mBits &= testBitSet;
    for (auto bit : testBitSet)
    {
        EXPECT_TRUE(mBits.test(bit));
    }
    EXPECT_EQ(mBits.count(), testBitSet.count());

    mBits.reset();
    mBits |= testBitSet;
    for (auto bit : testBitSet)
    {
        EXPECT_TRUE(mBits.test(bit));
    }

    mBits ^= testBitSet;
    EXPECT_TRUE(mBits.none());

    // Bitwise operators - "&", "|" and "^"
    std::set<std::size_t> bits1         = {0, 45, 60};
    std::set<std::size_t> bits2         = {5, 45, 50, 63};
    std::set<std::size_t> bits1Andbits2 = {45};
    std::set<std::size_t> bits1Orbits2  = {0, 5, 45, 50, 60, 63};
    std::set<std::size_t> bits1Xorbits2 = {0, 5, 50, 60, 63};
    std::set<std::size_t> actualValues;
    TypeParam testBitSet1;
    TypeParam testBitSet2;

    for (std::size_t bit : bits1)
    {
        testBitSet1.set(bit);
    }
    for (std::size_t bit : bits2)
    {
        testBitSet2.set(bit);
    }

    EXPECT_EQ(testBitSet1.first(), 0u);
    EXPECT_EQ(testBitSet1.last(), 60u);

    EXPECT_EQ(testBitSet2.first(), 5u);
    EXPECT_EQ(testBitSet2.last(), 63u);

    actualValues.clear();
    for (auto bit : (testBitSet1 & testBitSet2))
    {
        actualValues.insert(bit);
    }
    EXPECT_EQ(bits1Andbits2, actualValues);

    actualValues.clear();
    for (auto bit : (testBitSet1 | testBitSet2))
    {
        actualValues.insert(bit);
    }
    EXPECT_EQ(bits1Orbits2, actualValues);

    actualValues.clear();
    for (auto bit : (testBitSet1 ^ testBitSet2))
    {
        actualValues.insert(bit);
    }
    EXPECT_EQ(bits1Xorbits2, actualValues);

    // Relational operators - "==" and "!="
    EXPECT_FALSE(testBitSet1 == testBitSet2);
    EXPECT_TRUE(testBitSet1 != testBitSet2);

    // Unary operators - "~" and "[]"
    mBits.reset();
    mBits = ~testBitSet;
    for (auto bit : mBits)
    {
        EXPECT_FALSE(testBitSet.test(bit));
    }
    EXPECT_EQ(mBits.count(), (mBits.size() - testBitSet.count()));

    mBits.reset();
    for (auto bit : testBitSet)
    {
        mBits[bit] = true;
    }
    for (auto bit : mBits)
    {
        EXPECT_TRUE(testBitSet.test(bit));
    }
}

// Test that BitSetArray's initializer list constructor works correctly.
TEST(BitSetArrayTest, InitializerList)
{
    BitSetArray<500> bits = BitSetArray<500>{
        0,   11,  22,  33,  44,  55,  66,  77,  88,  99,  110, 121, 132, 143, 154, 165,
        176, 187, 198, 209, 220, 231, 242, 253, 264, 275, 286, 297, 308, 319, 330, 341,
        352, 363, 374, 385, 396, 407, 418, 429, 440, 451, 462, 473, 484, 495,
    };

    for (size_t i = 0; i < bits.size(); ++i)
    {
        if (i % 11 == 0)
        {
            EXPECT_TRUE(bits[i]) << i;
        }
        else
        {
            EXPECT_FALSE(bits[i]) << i;
        }
    }
}

// Test that BitSetArray's constructor with uint64_t.
TYPED_TEST(BitSetArrayTest, ConstructorWithUInt64)
{
    uint64_t value = 0x5555555555555555;
    TypeParam testBitSet(value);
    for (size_t i = 0; i < testBitSet.size(); ++i)
    {
        if (i < sizeof(uint64_t) * 8 && (value & (0x1ull << i)))
        {
            EXPECT_TRUE(testBitSet.test(i));
        }
        else
        {
            EXPECT_FALSE(testBitSet.test(i));
        }
    }
}

// Test iteration over BitSetArray where there are gaps
TYPED_TEST(BitSetArrayTest, IterationWithGaps)
{
    TypeParam &mBits = this->mBitSet;

    // Test iterator works with gap in bitset.
    std::set<size_t> bitsToBeSet = {0, mBits.size() / 2, mBits.size() - 1};
    for (size_t bit : bitsToBeSet)
    {
        mBits.set(bit);
    }
    std::set<size_t> bitsActuallySet = {};
    for (size_t bit : mBits)
    {
        bitsActuallySet.insert(bit);
    }
    EXPECT_EQ(bitsToBeSet, bitsActuallySet);
    EXPECT_EQ(mBits.count(), bitsToBeSet.size());
    mBits.reset();
}

// Test BitSetArray::Mask
TYPED_TEST(BitSetArrayTest, Mask)
{
    // Test constexpr usage
    TypeParam bits = TypeParam::Mask(0);
    for (size_t i = 0; i < bits.size(); ++i)
    {
        EXPECT_FALSE(bits[i]) << i;
    }

    bits = TypeParam::Mask(1);
    for (size_t i = 0; i < bits.size(); ++i)
    {
        if (i < 1)
        {
            EXPECT_TRUE(bits[i]) << i;
        }
        else
        {
            EXPECT_FALSE(bits[i]) << i;
        }
    }

    bits = TypeParam::Mask(2);
    for (size_t i = 0; i < bits.size(); ++i)
    {
        if (i < 2)
        {
            EXPECT_TRUE(bits[i]) << i;
        }
        else
        {
            EXPECT_FALSE(bits[i]) << i;
        }
    }

    bits = TypeParam::Mask(TypeParam::size());
    for (size_t i = 0; i < bits.size(); ++i)
    {
        EXPECT_TRUE(bits[i]) << i;
    }

    // Complete test
    for (size_t i = 0; i < TypeParam::size(); ++i)
    {
        bits = TypeParam::Mask(i);
        for (size_t j = 0; j < bits.size(); ++j)
        {
            if (j < i)
            {
                EXPECT_TRUE(bits[j]) << j;
            }
            else
            {
                EXPECT_FALSE(bits[j]) << j;
            }
        }
    }
}

template <typename T>
class BitSetArrayIteratorTest : public testing::Test
{
  protected:
    T mStateBits;
    typedef T BitSet;
};

using BitSetArrayIteratorTypes = ::testing::Types<BitSetArray<90>, BitSetArray<200>>;
TYPED_TEST_SUITE(BitSetArrayIteratorTest, BitSetArrayIteratorTypes);

// Simple iterator test.
TYPED_TEST(BitSetArrayIteratorTest, Iterator)
{
    TypeParam mStateBits = this->mStateBits;
    std::set<size_t> originalValues;
    originalValues.insert(2);
    originalValues.insert(6);
    originalValues.insert(8);
    originalValues.insert(35);
    if (TypeParam::size() > 70)
    {
        originalValues.insert(70);
    }
    if (TypeParam::size() > 181)
    {
        originalValues.insert(181);
    }

    for (size_t value : originalValues)
    {
        mStateBits.set(value);
    }

    std::set<size_t> readValues;
    for (size_t bit : mStateBits)
    {
        EXPECT_EQ(1u, originalValues.count(bit));
        EXPECT_EQ(0u, readValues.count(bit));
        readValues.insert(bit);
    }

    EXPECT_EQ(originalValues.size(), readValues.size());
}

// Ensure lsb->msb iteration order.
TYPED_TEST(BitSetArrayIteratorTest, IterationOrder)
{
    TypeParam mStateBits                   = this->mStateBits;
    const std::array<size_t, 8> writeOrder = {20, 25, 16, 31, 10, 14, 36, 19};
    const std::array<size_t, 8> fetchOrder = {10, 14, 16, 19, 20, 25, 31, 36};

    for (size_t value : writeOrder)
    {
        mStateBits.set(value);
    }
    EXPECT_EQ(writeOrder.size(), mStateBits.count());

    size_t i = 0;
    for (size_t bit : mStateBits)
    {
        EXPECT_EQ(fetchOrder[i], bit);
        i++;
    }
    EXPECT_EQ(fetchOrder.size(), mStateBits.count());
}

// Test an empty iterator.
TYPED_TEST(BitSetArrayIteratorTest, EmptySet)
{
    TypeParam mStateBits = this->mStateBits;
    // We don't use the FAIL gtest macro here since it returns immediately,
    // causing an unreachable code warning in MSVS
    bool sawBit = false;
    for (size_t bit : mStateBits)
    {
        sawBit = true;
        ANGLE_UNUSED_VARIABLE(bit);
    }
    EXPECT_FALSE(sawBit);
}

// Test iterating a result of combining two bitsets.
TYPED_TEST(BitSetArrayIteratorTest, NonLValueBitset)
{
    TypeParam mStateBits = this->mStateBits;
    typename TestFixture::BitSet otherBits;

    mStateBits.set(1);
    mStateBits.set(2);
    mStateBits.set(3);
    mStateBits.set(4);

    otherBits.set(0);
    otherBits.set(1);
    otherBits.set(3);
    otherBits.set(5);

    std::set<size_t> seenBits;

    typename TestFixture::BitSet maskedBits = (mStateBits & otherBits);
    for (size_t bit : maskedBits)
    {
        EXPECT_EQ(0u, seenBits.count(bit));
        seenBits.insert(bit);
        EXPECT_TRUE(mStateBits[bit]);
        EXPECT_TRUE(otherBits[bit]);
    }

    EXPECT_EQ((mStateBits & otherBits).count(), seenBits.size());
}

// Test bit assignments.
TYPED_TEST(BitSetArrayIteratorTest, BitAssignment)
{
    TypeParam mStateBits = this->mStateBits;
    std::set<size_t> originalValues;
    originalValues.insert(2);
    originalValues.insert(6);
    originalValues.insert(8);
    originalValues.insert(35);
    if (TypeParam::size() > 70)
    {
        originalValues.insert(70);
    }
    if (TypeParam::size() > 181)
    {
        originalValues.insert(181);
    }

    for (size_t value : originalValues)
    {
        (mStateBits[value] = false) = true;
    }

    for (size_t value : originalValues)
    {
        EXPECT_TRUE(mStateBits.test(value));
    }
}

// Tests adding bits to the iterator during iteration.
TYPED_TEST(BitSetArrayIteratorTest, SetLaterBit)
{
    TypeParam mStateBits            = this->mStateBits;
    std::set<size_t> expectedValues = {0, 1, 3, 5, 6, 7, 9, 35};
    if (TypeParam::size() > 64)
    {
        expectedValues.insert(64);
    }
    if (TypeParam::size() > 199)
    {
        expectedValues.insert(199);
    }

    mStateBits.set(0);
    mStateBits.set(1);

    std::set<size_t> actualValues;

    for (auto iter = mStateBits.begin(), end = mStateBits.end(); iter != end; ++iter)
    {
        if (*iter == 1)
        {
            iter.setLaterBit(3);
            iter.setLaterBit(5);
        }
        if (*iter == 5)
        {
            iter.setLaterBit(6);
            iter.setLaterBit(7);
            iter.setLaterBit(9);
            iter.setLaterBit(35);
        }
        if (*iter == 35 && TypeParam::size() > 64)
        {
            iter.setLaterBit(64);
        }
        if (*iter == 64 && TypeParam::size() > 199)
        {
            iter.setLaterBit(199);
        }

        actualValues.insert(*iter);
    }

    EXPECT_EQ(expectedValues, actualValues);
}

// Tests removing bit from the iterator during iteration.
TYPED_TEST(BitSetArrayIteratorTest, ResetLaterBit)
{
    TypeParam mStateBits = this->mStateBits;
    std::set<size_t> expectedValues;
    for (size_t index = 1; index < TypeParam::size(); index += 2)
    {
        expectedValues.insert(index);
    }

    for (size_t index = 1; index < TypeParam::size(); ++index)
        mStateBits.set(index);

    std::set<size_t> actualValues;

    for (auto iter = mStateBits.begin(), end = mStateBits.end(); iter != end; ++iter)
    {
        if (*iter == 1)
        {
            for (size_t index = 2; index < TypeParam::size(); index += 2)
            {
                iter.resetLaterBit(index);
            }
        }

        actualValues.insert(*iter);
    }

    EXPECT_EQ(expectedValues, actualValues);
}

// Tests adding bitsets to the iterator during iteration.
TYPED_TEST(BitSetArrayIteratorTest, SetLaterBits)
{
    TypeParam mStateBits            = this->mStateBits;
    std::set<size_t> expectedValues = {1, 2, 3, 4, 5, 7, 9};
    mStateBits.set(1);
    mStateBits.set(2);
    mStateBits.set(3);

    TypeParam laterBits;
    laterBits.set(4);
    laterBits.set(5);
    laterBits.set(7);
    laterBits.set(9);

    if (TypeParam::size() > 71)
    {
        expectedValues.insert(71);
        laterBits.set(71);
    }
    if (TypeParam::size() > 155)
    {
        expectedValues.insert(155);
        laterBits.set(155);
    }

    std::set<size_t> actualValues;

    for (auto iter = mStateBits.begin(), end = mStateBits.end(); iter != end; ++iter)
    {
        if (*iter == 3)
        {
            iter.setLaterBits(laterBits);
        }

        EXPECT_EQ(actualValues.count(*iter), 0u);
        actualValues.insert(*iter);
    }

    EXPECT_EQ(expectedValues, actualValues);
}

// Unit test for angle::Bit
TEST(Bit, Test)
{
    EXPECT_EQ(Bit<uint32_t>(0), 1u);
    EXPECT_EQ(Bit<uint32_t>(1), 2u);
    EXPECT_EQ(Bit<uint32_t>(2), 4u);
    EXPECT_EQ(Bit<uint32_t>(3), 8u);
    EXPECT_EQ(Bit<uint32_t>(31), 0x8000'0000u);
    EXPECT_EQ(Bit<uint64_t>(63), static_cast<uint64_t>(0x8000'0000'0000'0000llu));
}

// Unit test for angle::BitMask
TEST(BitMask, Test)
{
    EXPECT_EQ(BitMask<uint32_t>(1), 1u);
    EXPECT_EQ(BitMask<uint32_t>(2), 3u);
    EXPECT_EQ(BitMask<uint32_t>(3), 7u);
    EXPECT_EQ(BitMask<uint32_t>(31), 0x7FFF'FFFFu);
    EXPECT_EQ(BitMask<uint32_t>(32), 0xFFFF'FFFFu);
    EXPECT_EQ(BitMask<uint64_t>(63), static_cast<uint64_t>(0x7FFF'FFFF'FFFF'FFFFllu));
    EXPECT_EQ(BitMask<uint64_t>(64), static_cast<uint64_t>(0xFFFF'FFFF'FFFF'FFFFllu));
}
}  // anonymous namespace
