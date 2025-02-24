//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// mathutil_unittest:
//   Unit tests for the utils defined in mathutil.h
//

#include "mathutil.h"

#include <gtest/gtest.h>

using namespace gl;

namespace
{

// Test the correctness of packSnorm2x16 and unpackSnorm2x16 functions.
// For floats f1 and f2, unpackSnorm2x16(packSnorm2x16(f1, f2)) should be same as f1 and f2.
TEST(MathUtilTest, packAndUnpackSnorm2x16)
{
    const float input[8][2] = {
        {0.0f, 0.0f},    {1.0f, 1.0f},          {-1.0f, 1.0f},           {-1.0f, -1.0f},
        {0.875f, 0.75f}, {0.00392f, -0.99215f}, {-0.000675f, 0.004954f}, {-0.6937f, -0.02146f}};
    const float floatFaultTolerance = 0.0001f;
    float outputVal1, outputVal2;

    for (size_t i = 0; i < 8; i++)
    {
        unpackSnorm2x16(packSnorm2x16(input[i][0], input[i][1]), &outputVal1, &outputVal2);
        EXPECT_NEAR(input[i][0], outputVal1, floatFaultTolerance);
        EXPECT_NEAR(input[i][1], outputVal2, floatFaultTolerance);
    }
}

// Test the correctness of packSnorm2x16 and unpackSnorm2x16 functions with infinity values,
// result should be clamped to [-1, 1].
TEST(MathUtilTest, packAndUnpackSnorm2x16Infinity)
{
    const float floatFaultTolerance = 0.0001f;
    float outputVal1, outputVal2;

    unpackSnorm2x16(packSnorm2x16(std::numeric_limits<float>::infinity(),
                                  std::numeric_limits<float>::infinity()),
                    &outputVal1, &outputVal2);
    EXPECT_NEAR(1.0f, outputVal1, floatFaultTolerance);
    EXPECT_NEAR(1.0f, outputVal2, floatFaultTolerance);

    unpackSnorm2x16(packSnorm2x16(std::numeric_limits<float>::infinity(),
                                  -std::numeric_limits<float>::infinity()),
                    &outputVal1, &outputVal2);
    EXPECT_NEAR(1.0f, outputVal1, floatFaultTolerance);
    EXPECT_NEAR(-1.0f, outputVal2, floatFaultTolerance);

    unpackSnorm2x16(packSnorm2x16(-std::numeric_limits<float>::infinity(),
                                  -std::numeric_limits<float>::infinity()),
                    &outputVal1, &outputVal2);
    EXPECT_NEAR(-1.0f, outputVal1, floatFaultTolerance);
    EXPECT_NEAR(-1.0f, outputVal2, floatFaultTolerance);
}

// Test the correctness of packUnorm2x16 and unpackUnorm2x16 functions.
// For floats f1 and f2, unpackUnorm2x16(packUnorm2x16(f1, f2)) should be same as f1 and f2.
TEST(MathUtilTest, packAndUnpackUnorm2x16)
{
    const float input[8][2] = {
        {0.0f, 0.0f},    {1.0f, 1.0f},          {-1.0f, 1.0f},           {-1.0f, -1.0f},
        {0.875f, 0.75f}, {0.00392f, -0.99215f}, {-0.000675f, 0.004954f}, {-0.6937f, -0.02146f}};
    const float floatFaultTolerance = 0.0001f;
    float outputVal1, outputVal2;

    for (size_t i = 0; i < 8; i++)
    {
        unpackUnorm2x16(packUnorm2x16(input[i][0], input[i][1]), &outputVal1, &outputVal2);
        float expected = input[i][0] < 0.0f ? 0.0f : input[i][0];
        EXPECT_NEAR(expected, outputVal1, floatFaultTolerance);
        expected = input[i][1] < 0.0f ? 0.0f : input[i][1];
        EXPECT_NEAR(expected, outputVal2, floatFaultTolerance);
    }
}

// Test the correctness of packUnorm2x16 and unpackUnorm2x16 functions with infinity values,
// result should be clamped to [0, 1].
TEST(MathUtilTest, packAndUnpackUnorm2x16Infinity)
{
    const float floatFaultTolerance = 0.0001f;
    float outputVal1, outputVal2;

    unpackUnorm2x16(packUnorm2x16(std::numeric_limits<float>::infinity(),
                                  std::numeric_limits<float>::infinity()),
                    &outputVal1, &outputVal2);
    EXPECT_NEAR(1.0f, outputVal1, floatFaultTolerance);
    EXPECT_NEAR(1.0f, outputVal2, floatFaultTolerance);

    unpackUnorm2x16(packUnorm2x16(std::numeric_limits<float>::infinity(),
                                  -std::numeric_limits<float>::infinity()),
                    &outputVal1, &outputVal2);
    EXPECT_NEAR(1.0f, outputVal1, floatFaultTolerance);
    EXPECT_NEAR(0.0f, outputVal2, floatFaultTolerance);

    unpackUnorm2x16(packUnorm2x16(-std::numeric_limits<float>::infinity(),
                                  -std::numeric_limits<float>::infinity()),
                    &outputVal1, &outputVal2);
    EXPECT_NEAR(0.0f, outputVal1, floatFaultTolerance);
    EXPECT_NEAR(0.0f, outputVal2, floatFaultTolerance);
}

// Test the correctness of packHalf2x16 and unpackHalf2x16 functions.
// For floats f1 and f2, unpackHalf2x16(packHalf2x16(f1, f2)) should be same as f1 and f2.
TEST(MathUtilTest, packAndUnpackHalf2x16)
{
    const float input[8][2] = {
        {0.0f, 0.0f},    {1.0f, 1.0f},          {-1.0f, 1.0f},           {-1.0f, -1.0f},
        {0.875f, 0.75f}, {0.00392f, -0.99215f}, {-0.000675f, 0.004954f}, {-0.6937f, -0.02146f},
    };
    const float floatFaultTolerance = 0.0005f;
    float outputVal1, outputVal2;

    for (size_t i = 0; i < 8; i++)
    {
        unpackHalf2x16(packHalf2x16(input[i][0], input[i][1]), &outputVal1, &outputVal2);
        EXPECT_NEAR(input[i][0], outputVal1, floatFaultTolerance);
        EXPECT_NEAR(input[i][1], outputVal2, floatFaultTolerance);
    }
}

// Test the correctness of packUnorm4x8 and unpackUnorm4x8 functions.
// For floats f1 to f4, unpackUnorm4x8(packUnorm4x8(f1, f2, f3, f4)) should be same as f1 to f4.
TEST(MathUtilTest, packAndUnpackUnorm4x8)
{
    const float input[5][4] = {{0.0f, 0.0f, 0.0f, 0.0f},
                               {1.0f, 1.0f, 1.0f, 1.0f},
                               {-1.0f, 1.0f, -1.0f, 1.0f},
                               {-1.0f, -1.0f, -1.0f, -1.0f},
                               {64.0f / 255.0f, 128.0f / 255.0f, 32.0f / 255.0f, 16.0f / 255.0f}};

    const float floatFaultTolerance = 0.005f;
    float outputVals[4];

    for (size_t i = 0; i < 5; i++)
    {
        UnpackUnorm4x8(PackUnorm4x8(input[i][0], input[i][1], input[i][2], input[i][3]),
                       outputVals);
        for (size_t j = 0; j < 4; j++)
        {
            float expected = input[i][j] < 0.0f ? 0.0f : input[i][j];
            EXPECT_NEAR(expected, outputVals[j], floatFaultTolerance);
        }
    }
}

// Test the correctness of packSnorm4x8 and unpackSnorm4x8 functions.
// For floats f1 to f4, unpackSnorm4x8(packSnorm4x8(f1, f2, f3, f4)) should be same as f1 to f4.
TEST(MathUtilTest, packAndUnpackSnorm4x8)
{
    const float input[5][4] = {{0.0f, 0.0f, 0.0f, 0.0f},
                               {1.0f, 1.0f, 1.0f, 1.0f},
                               {-1.0f, 1.0f, -1.0f, 1.0f},
                               {-1.0f, -1.0f, -1.0f, -1.0f},
                               {64.0f / 127.0f, -8.0f / 127.0f, 32.0f / 127.0f, 16.0f / 127.0f}};

    const float floatFaultTolerance = 0.01f;
    float outputVals[4];

    for (size_t i = 0; i < 5; i++)
    {
        UnpackSnorm4x8(PackSnorm4x8(input[i][0], input[i][1], input[i][2], input[i][3]),
                       outputVals);
        for (size_t j = 0; j < 4; j++)
        {
            float expected = input[i][j];
            EXPECT_NEAR(expected, outputVals[j], floatFaultTolerance);
        }
    }
}

// Test the correctness of gl::isNaN function.
TEST(MathUtilTest, isNaN)
{
    EXPECT_TRUE(isNaN(bitCast<float>(0xffu << 23 | 1u)));
    EXPECT_TRUE(isNaN(bitCast<float>(1u << 31 | 0xffu << 23 | 1u)));
    EXPECT_TRUE(isNaN(bitCast<float>(1u << 31 | 0xffu << 23 | 0x400000u)));
    EXPECT_TRUE(isNaN(bitCast<float>(1u << 31 | 0xffu << 23 | 0x7fffffu)));
    EXPECT_FALSE(isNaN(0.0f));
    EXPECT_FALSE(isNaN(bitCast<float>(1u << 31 | 0xffu << 23)));
    EXPECT_FALSE(isNaN(bitCast<float>(0xffu << 23)));
}

// Test the correctness of gl::isInf function.
TEST(MathUtilTest, isInf)
{
    EXPECT_TRUE(isInf(bitCast<float>(0xffu << 23)));
    EXPECT_TRUE(isInf(bitCast<float>(1u << 31 | 0xffu << 23)));
    EXPECT_FALSE(isInf(0.0f));
    EXPECT_FALSE(isInf(bitCast<float>(0xffu << 23 | 1u)));
    EXPECT_FALSE(isInf(bitCast<float>(1u << 31 | 0xffu << 23 | 1u)));
    EXPECT_FALSE(isInf(bitCast<float>(1u << 31 | 0xffu << 23 | 0x400000u)));
    EXPECT_FALSE(isInf(bitCast<float>(1u << 31 | 0xffu << 23 | 0x7fffffu)));
    EXPECT_FALSE(isInf(bitCast<float>(0xfeu << 23 | 0x7fffffu)));
    EXPECT_FALSE(isInf(bitCast<float>(1u << 31 | 0xfeu << 23 | 0x7fffffu)));
}

TEST(MathUtilTest, CountLeadingZeros)
{
    for (unsigned int i = 0; i < 32u; ++i)
    {
        uint32_t iLeadingZeros = 1u << (31u - i);
        EXPECT_EQ(i, CountLeadingZeros(iLeadingZeros));
    }
    EXPECT_EQ(32u, CountLeadingZeros(0));
}

// Some basic tests. Pow2 roundUp test and test that rounding up zero produces zero.
TEST(MathUtilTest, Pow2RoundUp)
{
    EXPECT_EQ(0u, rx::roundUpPow2(0u, 4u));
    EXPECT_EQ(4u, rx::roundUpPow2(1u, 4u));
    EXPECT_EQ(4u, rx::roundUpPow2(4u, 4u));
}

// Non-pow2 test.
TEST(MathUtilTest, BasicRoundUp)
{
    EXPECT_EQ(0u, rx::roundUp(0u, 5u));
    EXPECT_EQ(5u, rx::roundUp(1u, 5u));
    EXPECT_EQ(5u, rx::roundUp(4u, 5u));
    EXPECT_EQ(5u, rx::roundUp(5u, 5u));
}

// Test that rounding up zero produces zero for checked ints.
TEST(MathUtilTest, CheckedRoundUpZero)
{
    auto checkedValue = rx::CheckedRoundUp(0u, 4u);
    ASSERT_TRUE(checkedValue.IsValid());
    ASSERT_EQ(0u, checkedValue.ValueOrDie());
}

// Test out-of-bounds with CheckedRoundUp
TEST(MathUtilTest, CheckedRoundUpInvalid)
{
    // The answer to this query is out of bounds.
    auto limit        = std::numeric_limits<unsigned int>::max();
    auto checkedValue = rx::CheckedRoundUp(limit, limit - 1);
    ASSERT_FALSE(checkedValue.IsValid());

    // Our implementation can't handle this query, despite the parameters being in range.
    auto checkedLimit = rx::CheckedRoundUp(limit - 1, limit);
    ASSERT_FALSE(checkedLimit.IsValid());
}

// Test BitfieldReverse which reverses the order of the bits in an integer.
TEST(MathUtilTest, BitfieldReverse)
{
    EXPECT_EQ(0u, gl::BitfieldReverse(0u));
    EXPECT_EQ(0x80000000u, gl::BitfieldReverse(1u));
    EXPECT_EQ(0x1u, gl::BitfieldReverse(0x80000000u));
    uint32_t bits     = (1u << 4u) | (1u << 7u);
    uint32_t reversed = (1u << (31u - 4u)) | (1u << (31u - 7u));
    EXPECT_EQ(reversed, gl::BitfieldReverse(bits));
}

// Test BitCount, which counts 1 bits in an integer.
TEST(MathUtilTest, BitCount)
{
    EXPECT_EQ(0, gl::BitCount(0u));
    EXPECT_EQ(32, gl::BitCount(0xFFFFFFFFu));
    EXPECT_EQ(10, gl::BitCount(0x17103121u));

    EXPECT_EQ(0, gl::BitCount(static_cast<uint64_t>(0ull)));
    EXPECT_EQ(32, gl::BitCount(static_cast<uint64_t>(0xFFFFFFFFull)));
    EXPECT_EQ(10, gl::BitCount(static_cast<uint64_t>(0x17103121ull)));

    EXPECT_EQ(33, gl::BitCount(static_cast<uint64_t>(0xFFFFFFFF80000000ull)));
    EXPECT_EQ(11, gl::BitCount(static_cast<uint64_t>(0x1710312180000000ull)));
}

// Test ScanForward, which scans for the least significant 1 bit from a non-zero integer.
TEST(MathUtilTest, ScanForward)
{
    EXPECT_EQ(0ul, gl::ScanForward(1u));
    EXPECT_EQ(16ul, gl::ScanForward(0x80010000u));
    EXPECT_EQ(31ul, gl::ScanForward(0x80000000u));

    EXPECT_EQ(0ul, gl::ScanForward(static_cast<uint64_t>(1ull)));
    EXPECT_EQ(16ul, gl::ScanForward(static_cast<uint64_t>(0x80010000ull)));
    EXPECT_EQ(31ul, gl::ScanForward(static_cast<uint64_t>(0x80000000ull)));

    EXPECT_EQ(32ul, gl::ScanForward(static_cast<uint64_t>(0x100000000ull)));
    EXPECT_EQ(48ul, gl::ScanForward(static_cast<uint64_t>(0x8001000000000000ull)));
    EXPECT_EQ(63ul, gl::ScanForward(static_cast<uint64_t>(0x8000000000000000ull)));
}

// Test ScanReverse, which scans for the most significant 1 bit from a non-zero integer.
TEST(MathUtilTest, ScanReverse)
{
    EXPECT_EQ(0ul, gl::ScanReverse(1u));
    EXPECT_EQ(16ul, gl::ScanReverse(static_cast<uint64_t>(0x00010030ull)));
    EXPECT_EQ(31ul, gl::ScanReverse(static_cast<uint64_t>(0x80000000ull)));

    EXPECT_EQ(32ul, gl::ScanReverse(static_cast<uint64_t>(0x100000000ull)));
    EXPECT_EQ(48ul, gl::ScanReverse(static_cast<uint64_t>(0x0001080000000000ull)));
    EXPECT_EQ(63ul, gl::ScanReverse(static_cast<uint64_t>(0x8000000000000000ull)));
}

// Test FindLSB, which finds the least significant 1 bit.
TEST(MathUtilTest, FindLSB)
{
    EXPECT_EQ(-1, gl::FindLSB(0u));
    EXPECT_EQ(0, gl::FindLSB(1u));
    EXPECT_EQ(16, gl::FindLSB(0x80010000u));
    EXPECT_EQ(31, gl::FindLSB(0x80000000u));
}

// Test FindMSB, which finds the most significant 1 bit.
TEST(MathUtilTest, FindMSB)
{
    EXPECT_EQ(-1, gl::FindMSB(0u));
    EXPECT_EQ(0, gl::FindMSB(1u));
    EXPECT_EQ(16, gl::FindMSB(0x00010030u));
    EXPECT_EQ(31, gl::FindMSB(0x80000000u));
}

// Test Ldexp, which combines mantissa and exponent into a floating-point number.
TEST(MathUtilTest, Ldexp)
{
    EXPECT_EQ(2.5f, Ldexp(0.625f, 2));
    EXPECT_EQ(-5.0f, Ldexp(-0.625f, 3));
    EXPECT_EQ(std::numeric_limits<float>::infinity(), Ldexp(0.625f, 129));
    EXPECT_EQ(0.0f, Ldexp(1.0f, -129));
}

// Test that Range::extend works as expected.
TEST(MathUtilTest, RangeExtend)
{
    RangeI range(0, 0);

    range.extend(5);
    EXPECT_EQ(0, range.low());
    EXPECT_EQ(6, range.high());
    EXPECT_EQ(6, range.length());

    range.extend(-1);
    EXPECT_EQ(-1, range.low());
    EXPECT_EQ(6, range.high());
    EXPECT_EQ(7, range.length());

    range.extend(10);
    EXPECT_EQ(-1, range.low());
    EXPECT_EQ(11, range.high());
    EXPECT_EQ(12, range.length());
}

// Test that Range::merge works as expected.
TEST(MathUtilTest, RangMerge)
{
    // merge valid range to invalid range
    RangeI range1;
    range1.invalidate();
    RangeI range2(1, 2);
    range1.merge(range2);
    EXPECT_EQ(1, range1.low());
    EXPECT_EQ(2, range1.high());
    EXPECT_EQ(1, range1.length());

    // merge invalid range to valid range
    RangeI range3(1, 2);
    RangeI range4;
    range4.invalidate();
    range3.merge(range4);
    EXPECT_EQ(1, range3.low());
    EXPECT_EQ(2, range3.high());
    EXPECT_EQ(1, range3.length());

    // merge two valid non-overlapping ranges
    RangeI range5(1, 2);
    RangeI range6(3, 4);
    range5.merge(range6);
    EXPECT_EQ(1, range5.low());
    EXPECT_EQ(4, range5.high());
    EXPECT_EQ(3, range5.length());

    // merge two valid non-overlapping ranges
    RangeI range7(2, 3);
    RangeI range8(1, 2);
    range7.merge(range8);
    EXPECT_EQ(1, range7.low());
    EXPECT_EQ(3, range7.high());
    EXPECT_EQ(2, range7.length());

    // merge two valid overlapping ranges
    RangeI range9(2, 4);
    RangeI range10(1, 3);
    range9.merge(range10);
    EXPECT_EQ(1, range9.low());
    EXPECT_EQ(4, range9.high());
    EXPECT_EQ(3, range9.length());

    // merge two valid overlapping ranges
    RangeI range11(1, 3);
    RangeI range12(2, 4);
    range11.merge(range12);
    EXPECT_EQ(1, range11.low());
    EXPECT_EQ(4, range11.high());
    EXPECT_EQ(3, range11.length());
}

// Test that Range::intersectsOrContinuous works as expected.
TEST(MathUtilTest, RangIntersectsOrContinuous)
{
    // Two non-overlapping ranges
    RangeI range1(1, 2);
    RangeI range2(3, 4);
    EXPECT_EQ(false, range1.intersectsOrContinuous(range2));
    EXPECT_EQ(false, range2.intersectsOrContinuous(range1));

    // Two overlapping ranges
    RangeI range3(1, 3);
    RangeI range4(2, 4);
    EXPECT_EQ(true, range3.intersectsOrContinuous(range4));
    EXPECT_EQ(true, range4.intersectsOrContinuous(range3));

    // Two overlapping ranges
    RangeI range5(1, 4);
    RangeI range6(2, 3);
    EXPECT_EQ(true, range5.intersectsOrContinuous(range6));
    EXPECT_EQ(true, range6.intersectsOrContinuous(range5));

    // Two continuous ranges
    RangeI range7(1, 3);
    RangeI range8(3, 4);
    EXPECT_EQ(true, range7.intersectsOrContinuous(range8));
    EXPECT_EQ(true, range8.intersectsOrContinuous(range7));

    // Two identical ranges
    RangeI range9(1, 3);
    RangeI range10(1, 3);
    EXPECT_EQ(true, range9.intersectsOrContinuous(range10));
}

// Test that Range iteration works as expected.
TEST(MathUtilTest, RangeIteration)
{
    RangeI range(0, 10);
    int expected = 0;
    for (int value : range)
    {
        EXPECT_EQ(expected, value);
        expected++;
    }
    EXPECT_EQ(range.length(), expected);
}

// Tests for clampForBitCount
TEST(MathUtilTest, ClampForBitCount)
{
    constexpr uint64_t kUnsignedMax = std::numeric_limits<uint64_t>::max();
    constexpr int64_t kSignedMax    = std::numeric_limits<int64_t>::max();
    constexpr int64_t kSignedMin    = std::numeric_limits<int64_t>::min();
    constexpr int64_t kRandomValue  = 0x4D34A0B1;

    ASSERT_EQ(clampForBitCount<uint64_t>(kUnsignedMax, 64), std::numeric_limits<uint64_t>::max());
    ASSERT_EQ(clampForBitCount<uint64_t>(kUnsignedMax, 32),
              static_cast<uint64_t>(std::numeric_limits<uint32_t>::max()));
    ASSERT_EQ(clampForBitCount<uint64_t>(kUnsignedMax, 16),
              static_cast<uint64_t>(std::numeric_limits<uint16_t>::max()));
    ASSERT_EQ(clampForBitCount<uint64_t>(kUnsignedMax, 8),
              static_cast<uint64_t>(std::numeric_limits<uint8_t>::max()));
    ASSERT_EQ(clampForBitCount<uint64_t>(kUnsignedMax, 4), 15u);
    ASSERT_EQ(clampForBitCount<uint64_t>(kUnsignedMax, 2), 3u);
    ASSERT_EQ(clampForBitCount<uint64_t>(kUnsignedMax, 1), 1u);
    ASSERT_EQ(clampForBitCount<uint64_t>(kUnsignedMax, 0), 0u);

    ASSERT_EQ(clampForBitCount<int64_t>(kSignedMax, 64), std::numeric_limits<int64_t>::max());
    ASSERT_EQ(clampForBitCount<uint64_t>(static_cast<uint64_t>(kSignedMax), 64),
              static_cast<uint64_t>(std::numeric_limits<int64_t>::max()));
    ASSERT_EQ(clampForBitCount<int64_t>(kSignedMax, 32),
              static_cast<int64_t>(std::numeric_limits<int32_t>::max()));
    ASSERT_EQ(clampForBitCount<int64_t>(kSignedMax, 16),
              static_cast<int64_t>(std::numeric_limits<int16_t>::max()));
    ASSERT_EQ(clampForBitCount<int64_t>(kSignedMax, 8),
              static_cast<int64_t>(std::numeric_limits<int8_t>::max()));
    ASSERT_EQ(clampForBitCount<int64_t>(kSignedMax, 4), 7);
    ASSERT_EQ(clampForBitCount<int64_t>(kSignedMax, 2), 1);
    ASSERT_EQ(clampForBitCount<int64_t>(kSignedMax, 0), 0);

    ASSERT_EQ(clampForBitCount<int64_t>(kRandomValue, 64), kRandomValue);
    ASSERT_EQ(clampForBitCount<int64_t>(kRandomValue, 32), kRandomValue);
    ASSERT_EQ(clampForBitCount<int64_t>(kRandomValue, 16),
              static_cast<int64_t>(std::numeric_limits<int16_t>::max()));
    ASSERT_EQ(clampForBitCount<int64_t>(kRandomValue, 8),
              static_cast<int64_t>(std::numeric_limits<int8_t>::max()));
    ASSERT_EQ(clampForBitCount<int64_t>(kRandomValue, 4), 7);
    ASSERT_EQ(clampForBitCount<int64_t>(kRandomValue, 2), 1);
    ASSERT_EQ(clampForBitCount<int64_t>(kRandomValue, 0), 0);

    ASSERT_EQ(clampForBitCount<int64_t>(kSignedMin, 64), std::numeric_limits<int64_t>::min());
    ASSERT_EQ(clampForBitCount<int64_t>(kSignedMin, 32),
              static_cast<int64_t>(std::numeric_limits<int32_t>::min()));
    ASSERT_EQ(clampForBitCount<int64_t>(kSignedMin, 16),
              static_cast<int64_t>(std::numeric_limits<int16_t>::min()));
    ASSERT_EQ(clampForBitCount<int64_t>(kSignedMin, 8),
              static_cast<int64_t>(std::numeric_limits<int8_t>::min()));
    ASSERT_EQ(clampForBitCount<int64_t>(kSignedMin, 4), -8);
    ASSERT_EQ(clampForBitCount<int64_t>(kSignedMin, 2), -2);
    ASSERT_EQ(clampForBitCount<int64_t>(kSignedMin, 0), 0);
}

// Tests for float32 to float16 conversion
TEST(MathUtilTest, Float32ToFloat16)
{
    ASSERT_EQ(float32ToFloat16(0.0f), 0x0000);
    ASSERT_EQ(float32ToFloat16(-0.0f), 0x8000);

    float inf = std::numeric_limits<float>::infinity();

    ASSERT_EQ(float32ToFloat16(inf), 0x7C00);
    ASSERT_EQ(float32ToFloat16(-inf), 0xFC00);

    // Check that NaN is converted to a value in one of the float16 NaN ranges
    float nan      = std::numeric_limits<float>::quiet_NaN();
    uint16_t nan16 = float32ToFloat16(nan);
    ASSERT_TRUE(nan16 > 0xFC00 || (nan16 < 0x8000 && nan16 > 0x7C00));

    ASSERT_EQ(float32ToFloat16(1.0f), 0x3C00);
}

// Tests the RGB float to 999E5 conversion
TEST(MathUtilTest, convertRGBFloatsTo999E5)
{
    const int numTests                  = 18;
    const float input[numTests][3]      = {// The basics
                                      {0.0f, 0.0f, 0.0f},
                                      {0.0f, 0.0f, 1.0f},
                                      {0.0f, 1.0f, 0.0f},
                                      {0.0f, 1.0f, 1.0f},
                                      {1.0f, 0.0f, 0.0f},
                                      {1.0f, 0.0f, 1.0f},
                                      {1.0f, 1.0f, 0.0f},
                                      {1.0f, 1.0f, 1.0f},
                                      // Extended range
                                      {0.0f, 0.0f, 1.5f},
                                      {0.0f, 2.0f, 0.0f},
                                      {0.0f, 2.5f, 3.0f},
                                      {3.5f, 0.0f, 0.0f},
                                      {4.0f, 0.0f, 4.5f},
                                      {5.0f, 5.5f, 0.0f},
                                      {6.0f, 6.5f, 7.0f},
                                      // Random
                                      {0.1f, 9.6f, 3.2f},
                                      {2.0f, 1.7f, 8.6f},
                                      {0.7f, 4.2f, 9.1f}};
    const unsigned int result[numTests] = {// The basics
                                           0x00000000, 0x84000000, 0x80020000, 0x84020000,
                                           0x80000100, 0x84000100, 0x80020100, 0x84020100,
                                           // Extended range
                                           0x86000000, 0x88020000, 0x8E028000, 0x880001C0,
                                           0x94800100, 0x9002C140, 0x97034180,
                                           // Random
                                           0x999A6603, 0x9C4C6C40, 0x9C8D0C16};

    for (int i = 0; i < numTests; i++)
    {
        EXPECT_EQ(convertRGBFloatsTo999E5(input[i][0], input[i][1], input[i][2]), result[i]);
    }
}

// Tests the 999E5 to RGB float conversion
TEST(MathUtilTest, convert999E5toRGBFloats)
{
    const int numTests                 = 18;
    const float result[numTests][3]    = {// The basics
                                       {0.0f, 0.0f, 0.0f},
                                       {0.0f, 0.0f, 1.0f},
                                       {0.0f, 1.0f, 0.0f},
                                       {0.0f, 1.0f, 1.0f},
                                       {1.0f, 0.0f, 0.0f},
                                       {1.0f, 0.0f, 1.0f},
                                       {1.0f, 1.0f, 0.0f},
                                       {1.0f, 1.0f, 1.0f},
                                       // Extended range
                                       {0.0f, 0.0f, 1.5f},
                                       {0.0f, 2.0f, 0.0f},
                                       {0.0f, 2.5f, 3.0f},
                                       {3.5f, 0.0f, 0.0f},
                                       {4.0f, 0.0f, 4.5f},
                                       {5.0f, 5.5f, 0.0f},
                                       {6.0f, 6.5f, 7.0f},
                                       // Random
                                       {0.1f, 9.6f, 3.2f},
                                       {2.0f, 1.7f, 8.6f},
                                       {0.7f, 4.2f, 9.1f}};
    const unsigned int input[numTests] = {// The basics
                                          0x00000000, 0x84000000, 0x80020000, 0x84020000,
                                          0x80000100, 0x84000100, 0x80020100, 0x84020100,
                                          // Extended range
                                          0x86000000, 0x88020000, 0x8E028000, 0x880001C0,
                                          0x94800100, 0x9002C140, 0x97034180,
                                          // Random
                                          0x999A6603, 0x9C4C6C40, 0x9C8D0C16};
    // Note: quite a low tolerance is required
    const float floatFaultTolerance = 0.05f;
    float outR, outG, outB;

    for (int i = 0; i < numTests; i++)
    {
        convert999E5toRGBFloats(input[i], &outR, &outG, &outB);
        EXPECT_NEAR(result[i][0], outR, floatFaultTolerance);
        EXPECT_NEAR(result[i][1], outG, floatFaultTolerance);
        EXPECT_NEAR(result[i][2], outB, floatFaultTolerance);
    }
}

// Test sRGB conversions
TEST(MathUtilTest, sRGB)
{
    // Check simple roundtrip
    for (size_t c = 0; c < 256; c++)
    {
        ASSERT_EQ(c, linearToSRGB(sRGBToLinear(c)));
    }

    // Check average of identical values
    for (size_t c = 0; c < 256; c++)
    {
        ASSERT_EQ(c, linearToSRGB((sRGBToLinear(c) + sRGBToLinear(c)) * 0.5));
    }

    // Check that all average values are in range
    for (size_t a = 0; a < 256; a++)
    {
        for (size_t b = 0; b < 256; b++)
        {
            const float avg = (sRGBToLinear(a) + sRGBToLinear(b)) * 0.5f;
            ASSERT_GE(avg, 0.0f);
            ASSERT_LE(avg, 1.0f);
        }
    }
}

// Test roundToNearest with boundary values
TEST(MathUtilTest, RoundToNearest)
{
    EXPECT_EQ((roundToNearest<float, uint8_t>(0.00000000f)), 0u);
    EXPECT_EQ((roundToNearest<float, uint8_t>(0.49999997f)), 0u);
    EXPECT_EQ((roundToNearest<float, uint8_t>(0.50000000f)), 1u);

    EXPECT_EQ((roundToNearest<float, int8_t>(-0.50000000f)), -1);
    EXPECT_EQ((roundToNearest<float, int8_t>(-0.49999997f)), 0);
    EXPECT_EQ((roundToNearest<float, int8_t>(-0.00000000f)), 0);
    EXPECT_EQ((roundToNearest<float, int8_t>(+0.00000000f)), 0);
    EXPECT_EQ((roundToNearest<float, int8_t>(+0.49999997f)), 0);
    EXPECT_EQ((roundToNearest<float, int8_t>(+0.50000000f)), +1);

    EXPECT_EQ((roundToNearest<double, uint8_t>(0.00000000000000000)), 0u);
    EXPECT_EQ((roundToNearest<double, uint8_t>(0.49999999999999994)), 0u);
    EXPECT_EQ((roundToNearest<double, uint8_t>(0.50000000000000000)), 1u);

    EXPECT_EQ((roundToNearest<double, int8_t>(-0.50000000000000000)), -1);
    EXPECT_EQ((roundToNearest<double, int8_t>(-0.49999999999999994)), 0);
    EXPECT_EQ((roundToNearest<double, int8_t>(-0.00000000000000000)), 0);
    EXPECT_EQ((roundToNearest<double, int8_t>(+0.00000000000000000)), 0);
    EXPECT_EQ((roundToNearest<double, int8_t>(+0.49999999999999994)), 0);
    EXPECT_EQ((roundToNearest<double, int8_t>(+0.50000000000000000)), +1);
}

// Test floatToNormalized conversions with uint8_t
TEST(MathUtilTest, FloatToNormalizedUnorm8)
{
    // Check exact values
    EXPECT_EQ(floatToNormalized<uint8_t>(0.00f), 0u);
    EXPECT_EQ(floatToNormalized<uint8_t>(0.25f), 64u);
    EXPECT_EQ(floatToNormalized<uint8_t>(0.75f), 191u);
    EXPECT_EQ(floatToNormalized<uint8_t>(1.00f), 255u);

    // Check near values
    EXPECT_NEAR(floatToNormalized<uint8_t>(0.50f), 127u, 1);
}

// Test floatToNormalized conversions with int8_t
TEST(MathUtilTest, FloatToNormalizedSnorm8)
{
    // Check exact values
    EXPECT_EQ(floatToNormalized<int8_t>(-1.00f), -127);
    EXPECT_EQ(floatToNormalized<int8_t>(-0.75f), -95);
    EXPECT_EQ(floatToNormalized<int8_t>(-0.25f), -32);
    EXPECT_EQ(floatToNormalized<int8_t>(+0.00f), 0);
    EXPECT_EQ(floatToNormalized<int8_t>(+0.25f), +32);
    EXPECT_EQ(floatToNormalized<int8_t>(+0.75f), +95);
    EXPECT_EQ(floatToNormalized<int8_t>(+1.00f), +127);

    // Check near values
    EXPECT_NEAR(floatToNormalized<int8_t>(-0.50f), -63, 1);
    EXPECT_NEAR(floatToNormalized<int8_t>(+0.50f), +63, 1);
}

// Test floatToNormalized conversions with uint16_t
TEST(MathUtilTest, FloatToNormalizedUnorm16)
{
    // Check exact values
    EXPECT_EQ(floatToNormalized<uint16_t>(0.00f), 0u);
    EXPECT_EQ(floatToNormalized<uint16_t>(0.25f), 16384u);
    EXPECT_EQ(floatToNormalized<uint16_t>(0.75f), 49151u);
    EXPECT_EQ(floatToNormalized<uint16_t>(1.00f), 65535u);

    // Check near values
    EXPECT_NEAR(floatToNormalized<uint16_t>(0.50f), 32767u, 1);
}

// Test floatToNormalized conversions with int16_t
TEST(MathUtilTest, FloatToNormalizedSnorm16)
{
    // Check exact values
    EXPECT_EQ(floatToNormalized<int16_t>(-1.00f), -32767);
    EXPECT_EQ(floatToNormalized<int16_t>(-0.75f), -24575);
    EXPECT_EQ(floatToNormalized<int16_t>(-0.25f), -8192);
    EXPECT_EQ(floatToNormalized<int16_t>(+0.00f), 0);
    EXPECT_EQ(floatToNormalized<int16_t>(+0.25f), +8192);
    EXPECT_EQ(floatToNormalized<int16_t>(+0.75f), +24575);
    EXPECT_EQ(floatToNormalized<int16_t>(+1.00f), +32767);

    // Check near values
    EXPECT_NEAR(floatToNormalized<int16_t>(-0.50f), -16383, 1);
    EXPECT_NEAR(floatToNormalized<int16_t>(+0.50f), +16383, 1);
}

// Test floatToNormalized conversions with uint32_t
TEST(MathUtilTest, FloatToNormalizedUnorm32)
{
    // Check exact values
    EXPECT_EQ(floatToNormalized<uint32_t>(0.00f), 0u);
    EXPECT_EQ(floatToNormalized<uint32_t>(0.25f), 1073741824u);
    EXPECT_EQ(floatToNormalized<uint32_t>(0.75f), 3221225471u);
    EXPECT_EQ(floatToNormalized<uint32_t>(1.00f), 4294967295u);

    // Check near values
    EXPECT_NEAR(floatToNormalized<uint32_t>(0.50f), 2147483647u, 1);
}

// Test floatToNormalized conversions with int32_t
TEST(MathUtilTest, FloatToNormalizedSnorm32)
{
    // Check exact values
    EXPECT_EQ(floatToNormalized<int32_t>(-1.00f), -2147483647);
    EXPECT_EQ(floatToNormalized<int32_t>(-0.75f), -1610612735);
    EXPECT_EQ(floatToNormalized<int32_t>(-0.25f), -536870912);
    EXPECT_EQ(floatToNormalized<int32_t>(+0.00f), 0);
    EXPECT_EQ(floatToNormalized<int32_t>(+0.25f), +536870912);
    EXPECT_EQ(floatToNormalized<int32_t>(+0.75f), +1610612735);
    EXPECT_EQ(floatToNormalized<int32_t>(+1.00f), +2147483647);

    // Check near values
    EXPECT_NEAR(floatToNormalized<int32_t>(-0.50f), -1073741823, 1);
    EXPECT_NEAR(floatToNormalized<int32_t>(+0.50f), +1073741823, 1);
}

// Test floatToNormalized conversions with 2-bit unsigned
TEST(MathUtilTest, FloatToNormalizedUnorm2)
{
    // Check exact values
    EXPECT_EQ((floatToNormalized<2, uint8_t>(0.00f)), 0u);
    EXPECT_EQ((floatToNormalized<2, uint8_t>(0.25f)), 1u);
    EXPECT_EQ((floatToNormalized<2, uint8_t>(0.75f)), 2u);
    EXPECT_EQ((floatToNormalized<2, uint8_t>(1.00f)), 3u);

    // Check near values
    EXPECT_NEAR((floatToNormalized<2, uint8_t>(0.50f)), 1u, 1);
}

// Test floatToNormalized conversions with 2-bit signed
TEST(MathUtilTest, FloatToNormalizedSnorm2)
{
    // Check exact values
    EXPECT_EQ((floatToNormalized<2, int8_t>(-1.00f)), -1);
    EXPECT_EQ((floatToNormalized<2, int8_t>(-0.75f)), -1);
    EXPECT_EQ((floatToNormalized<2, int8_t>(-0.25f)), 0);
    EXPECT_EQ((floatToNormalized<2, int8_t>(+0.00f)), 0);
    EXPECT_EQ((floatToNormalized<2, int8_t>(+0.25f)), 0);
    EXPECT_EQ((floatToNormalized<2, int8_t>(+0.75f)), +1);
    EXPECT_EQ((floatToNormalized<2, int8_t>(+1.00f)), +1);

    // Check near values
    EXPECT_NEAR((floatToNormalized<2, int8_t>(-0.50f)), 0, 1);
    EXPECT_NEAR((floatToNormalized<2, int8_t>(+0.50f)), 0, 1);
}

// Test floatToNormalized conversions with 10-bit unsigned
TEST(MathUtilTest, FloatToNormalizedUnorm10)
{
    // Check exact values
    EXPECT_EQ((floatToNormalized<10, uint16_t>(0.00f)), 0u);
    EXPECT_EQ((floatToNormalized<10, uint16_t>(0.25f)), 256u);
    EXPECT_EQ((floatToNormalized<10, uint16_t>(0.75f)), 767u);
    EXPECT_EQ((floatToNormalized<10, uint16_t>(1.00f)), 1023u);

    // Check near values
    EXPECT_NEAR((floatToNormalized<10, uint16_t>(0.50f)), 511u, 1);
}

// Test floatToNormalized conversions with 10-bit signed
TEST(MathUtilTest, FloatToNormalizedSnorm10)
{
    // Check exact values
    EXPECT_EQ((floatToNormalized<10, int16_t>(-1.00f)), -511);
    EXPECT_EQ((floatToNormalized<10, int16_t>(-0.75f)), -383);
    EXPECT_EQ((floatToNormalized<10, int16_t>(-0.25f)), -128);
    EXPECT_EQ((floatToNormalized<10, int16_t>(+0.00f)), 0);
    EXPECT_EQ((floatToNormalized<10, int16_t>(+0.25f)), +128);
    EXPECT_EQ((floatToNormalized<10, int16_t>(+0.75f)), +383);
    EXPECT_EQ((floatToNormalized<10, int16_t>(+1.00f)), +511);

    // Check near values
    EXPECT_NEAR((floatToNormalized<10, int16_t>(-0.50f)), -255, 1);
    EXPECT_NEAR((floatToNormalized<10, int16_t>(+0.50f)), +255, 1);
}

// Test floatToNormalized conversions with 30-bit unsigned
TEST(MathUtilTest, FloatToNormalizedUnorm30)
{
    // Check exact values
    EXPECT_EQ((floatToNormalized<30, uint32_t>(0.00f)), 0u);
    EXPECT_EQ((floatToNormalized<30, uint32_t>(0.25f)), 268435456u);
    EXPECT_EQ((floatToNormalized<30, uint32_t>(0.75f)), 805306367u);
    EXPECT_EQ((floatToNormalized<30, uint32_t>(1.00f)), 1073741823u);

    // Check near values
    EXPECT_NEAR((floatToNormalized<30, uint32_t>(0.50f)), 536870911u, 1);
}

// Test floatToNormalized conversions with 30-bit signed
TEST(MathUtilTest, FloatToNormalizedSnorm30)
{
    // Check exact values
    EXPECT_EQ((floatToNormalized<30, int32_t>(-1.00f)), -536870911);
    EXPECT_EQ((floatToNormalized<30, int32_t>(-0.75f)), -402653183);
    EXPECT_EQ((floatToNormalized<30, int32_t>(-0.25f)), -134217728);
    EXPECT_EQ((floatToNormalized<30, int32_t>(+0.00f)), 0);
    EXPECT_EQ((floatToNormalized<30, int32_t>(+0.25f)), +134217728);
    EXPECT_EQ((floatToNormalized<30, int32_t>(+0.75f)), +402653183);
    EXPECT_EQ((floatToNormalized<30, int32_t>(+1.00f)), +536870911);

    // Check near values
    EXPECT_NEAR((floatToNormalized<30, int32_t>(-0.50f)), -268435455, 1);
    EXPECT_NEAR((floatToNormalized<30, int32_t>(+0.50f)), +268435455, 1);
}

// Test normalizedToFloat conversions with 8-bit unsigned
TEST(MathUtilTest, NormalizedToFloatUnorm8)
{
    // Check exact values
    EXPECT_EQ((normalizedToFloat<uint8_t>(0)), 0.0f);
    EXPECT_EQ((normalizedToFloat<uint8_t>(255)), 1.0f);

    // Check near values
    EXPECT_NEAR((normalizedToFloat<uint8_t>(127)), 0.5f, 0.004f);
    EXPECT_NEAR((normalizedToFloat<uint8_t>(128)), 0.5f, 0.004f);
}

// Test normalizedToFloat conversions with 8-bit signed
TEST(MathUtilTest, NormalizedToFloatSnorm8)
{
    // Check exact values
    EXPECT_EQ((normalizedToFloat<int8_t>(0)), 0.0f);
    EXPECT_EQ((normalizedToFloat<int8_t>(+127)), +1.0f);
    EXPECT_EQ((normalizedToFloat<int8_t>(-127)), -1.0f);
    EXPECT_EQ((normalizedToFloat<int8_t>(-128)), -1.0f);

    // Check near values
    EXPECT_NEAR((normalizedToFloat<int8_t>(+64)), +0.5f, 0.008f);
    EXPECT_NEAR((normalizedToFloat<int8_t>(+63)), +0.5f, 0.008f);
    EXPECT_NEAR((normalizedToFloat<int8_t>(-63)), -0.5f, 0.008f);
    EXPECT_NEAR((normalizedToFloat<int8_t>(-64)), -0.5f, 0.008f);
}

// Test normalizedToFloat conversions with 16-bit unsigned
TEST(MathUtilTest, NormalizedToFloatUnorm16)
{
    // Check exact values
    EXPECT_EQ((normalizedToFloat<uint16_t>(0)), 0.0f);
    EXPECT_EQ((normalizedToFloat<uint16_t>(65535)), 1.0f);

    // Check near values
    EXPECT_NEAR((normalizedToFloat<uint16_t>(32767)), 0.5f, 0.00002f);
    EXPECT_NEAR((normalizedToFloat<uint16_t>(32768)), 0.5f, 0.00002f);
}

// Test normalizedToFloat conversions with 16-bit signed
TEST(MathUtilTest, NormalizedToFloatSnorm16)
{
    // Check exact values
    EXPECT_EQ((normalizedToFloat<int16_t>(0)), 0.0f);
    EXPECT_EQ((normalizedToFloat<int16_t>(+32767)), +1.0f);
    EXPECT_EQ((normalizedToFloat<int16_t>(-32767)), -1.0f);
    EXPECT_EQ((normalizedToFloat<int16_t>(-32768)), -1.0f);

    // Check near values
    EXPECT_NEAR((normalizedToFloat<int16_t>(+16384)), +0.5f, 0.00004f);
    EXPECT_NEAR((normalizedToFloat<int16_t>(+16383)), +0.5f, 0.00004f);
    EXPECT_NEAR((normalizedToFloat<int16_t>(-16383)), -0.5f, 0.00004f);
    EXPECT_NEAR((normalizedToFloat<int16_t>(-16384)), -0.5f, 0.00004f);
}

// Test normalizedToFloat conversions with 32-bit unsigned
TEST(MathUtilTest, NormalizedToFloatUnorm32)
{
    // Check exact values
    EXPECT_EQ((normalizedToFloat<uint32_t>(0)), 0.0f);
    EXPECT_EQ((normalizedToFloat<uint32_t>(4294967295)), 1.0f);

    // Check near values
    EXPECT_NEAR((normalizedToFloat<uint32_t>(2147483647)), 0.5, 0.0000000003);
    EXPECT_NEAR((normalizedToFloat<uint32_t>(2147483648)), 0.5, 0.0000000003);
}

// Test normalizedToFloat conversions with 32-bit signed
TEST(MathUtilTest, NormalizedToFloatSnorm32)
{
    // Check exact values
    EXPECT_EQ((normalizedToFloat<int32_t>(0)), 0.0f);
    EXPECT_EQ((normalizedToFloat<int32_t>(+2147483647)), +1.0);
    EXPECT_EQ((normalizedToFloat<int32_t>(-2147483647)), -1.0);
    EXPECT_EQ((normalizedToFloat<int32_t>(-2147483648)), -1.0);

    // Check near values
    EXPECT_NEAR((normalizedToFloat<int32_t>(+1073741824)), +0.5, 0.0000000005);
    EXPECT_NEAR((normalizedToFloat<int32_t>(+1073741823)), +0.5, 0.0000000005);
    EXPECT_NEAR((normalizedToFloat<int32_t>(-1073741823)), -0.5, 0.0000000005);
    EXPECT_NEAR((normalizedToFloat<int32_t>(-1073741824)), -0.5, 0.0000000005);
}

// Test normalizedToFloat conversions with 1-bit unsigned
TEST(MathUtilTest, NormalizedToFloatUnorm1)
{
    // Check exact values
    EXPECT_EQ((normalizedToFloat<1>(0u)), 0.0f);
    EXPECT_EQ((normalizedToFloat<1>(1u)), 1.0f);
}

// Test normalizedToFloat conversions with 2-bit unsigned
TEST(MathUtilTest, NormalizedToFloatUnorm2)
{
    // Check exact values
    EXPECT_EQ((normalizedToFloat<2>(0u)), 0.0f);
    EXPECT_EQ((normalizedToFloat<2>(1u)), 0.33333333f);
    EXPECT_EQ((normalizedToFloat<2>(2u)), 0.66666667f);
    EXPECT_EQ((normalizedToFloat<2>(3u)), 1.0f);
}

// Test normalizedToFloat conversions with 2-bit signed
TEST(MathUtilTest, NormalizedToFloatSnorm2)
{
    // Check exact values
    EXPECT_EQ((normalizedToFloat<2>(0)), 0.0f);
    EXPECT_EQ((normalizedToFloat<2>(+1)), +1.0f);
    EXPECT_EQ((normalizedToFloat<2>(-1)), -1.0f);
    EXPECT_EQ((normalizedToFloat<2>(-2)), -1.0f);
}

// Test normalizedToFloat conversions with 4-bit unsigned
TEST(MathUtilTest, NormalizedToFloatUnorm4)
{
    // Check exact values
    EXPECT_EQ((normalizedToFloat<4>(0u)), 0.0f);
    EXPECT_EQ((normalizedToFloat<4>(15u)), 1.0f);

    // Check near values
    EXPECT_NEAR((normalizedToFloat<4>(7u)), 0.5f, 0.07);
    EXPECT_NEAR((normalizedToFloat<4>(8u)), 0.5f, 0.07);
}

// Test normalizedToFloat conversions with 5-bit unsigned
TEST(MathUtilTest, NormalizedToFloatUnorm5)
{
    // Check exact values
    EXPECT_EQ((normalizedToFloat<5>(0u)), 0.0f);
    EXPECT_EQ((normalizedToFloat<5>(31u)), 1.0f);

    // Check near values
    EXPECT_NEAR((normalizedToFloat<5>(15u)), 0.5f, 0.04);
    EXPECT_NEAR((normalizedToFloat<5>(16u)), 0.5f, 0.04);
}

// Test normalizedToFloat conversions with 6-bit unsigned
TEST(MathUtilTest, NormalizedToFloatUnorm6)
{
    // Check exact values
    EXPECT_EQ((normalizedToFloat<6>(0u)), 0.0f);
    EXPECT_EQ((normalizedToFloat<6>(63u)), 1.0f);

    // Check near values
    EXPECT_NEAR((normalizedToFloat<6>(31u)), 0.5f, 0.02);
    EXPECT_NEAR((normalizedToFloat<6>(32u)), 0.5f, 0.02);
}

// Test normalizedToFloat conversions with 10-bit unsigned
TEST(MathUtilTest, NormalizedToFloatUnorm10)
{
    // Check exact values
    EXPECT_EQ((normalizedToFloat<10>(0u)), 0.0f);
    EXPECT_EQ((normalizedToFloat<10>(1023u)), 1.0f);

    // Check near values
    EXPECT_NEAR((normalizedToFloat<10>(511u)), 0.5f, 0.001);
    EXPECT_NEAR((normalizedToFloat<10>(512u)), 0.5f, 0.001);
}

// Test normalizedToFloat conversions with 10-bit unsigned
TEST(MathUtilTest, NormalizedToFloatSnorm10)
{
    // Check exact values
    EXPECT_EQ((normalizedToFloat<10>(0)), 0.0f);
    EXPECT_EQ((normalizedToFloat<10>(+511)), +1.0f);
    EXPECT_EQ((normalizedToFloat<10>(-511)), -1.0f);
    EXPECT_EQ((normalizedToFloat<10>(-512)), -1.0f);

    // Check near values
    EXPECT_NEAR((normalizedToFloat<10>(+256)), +0.5f, 0.002);
    EXPECT_NEAR((normalizedToFloat<10>(+255)), +0.5f, 0.002);
    EXPECT_NEAR((normalizedToFloat<10>(-255)), -0.5f, 0.002);
    EXPECT_NEAR((normalizedToFloat<10>(-256)), -0.5f, 0.002);
}

// Test normalizedToFloat conversions with 24-bit unsigned
TEST(MathUtilTest, NormalizedToFloatUnorm24)
{
    // Check exact values
    EXPECT_EQ((normalizedToFloat<24>(0u)), 0.0f);
    EXPECT_EQ((normalizedToFloat<24>(16777215u)), 1.0f);

    // Check near values
    EXPECT_NEAR((normalizedToFloat<24>(8388607u)), 0.5f, 0.00000006);
    EXPECT_NEAR((normalizedToFloat<24>(8388608u)), 0.5f, 0.00000006);
}

// Test normalizedToFloat conversions with 24-bit signed
TEST(MathUtilTest, NormalizedToFloatSnorm24)
{
    // Check exact values
    EXPECT_EQ((normalizedToFloat<24>(0)), 0.0f);
    EXPECT_EQ((normalizedToFloat<24>(+8388607)), +1.0f);
    EXPECT_EQ((normalizedToFloat<24>(-8388607)), -1.0f);
    EXPECT_EQ((normalizedToFloat<24>(-8388608)), -1.0f);

    // Check near values
    EXPECT_NEAR((normalizedToFloat<24>(+4194304)), +0.5f, 0.00000012);
    EXPECT_NEAR((normalizedToFloat<24>(+4194303)), +0.5f, 0.00000012);
    EXPECT_NEAR((normalizedToFloat<24>(-4194303)), -0.5f, 0.00000012);
    EXPECT_NEAR((normalizedToFloat<24>(-4194304)), -0.5f, 0.00000012);
}

// Test normalizedToFloat conversions with 25-bit signed
TEST(MathUtilTest, NormalizedToFloatSnorm25)
{
    // Check exact values
    EXPECT_EQ((normalizedToFloat<25>(0)), 0.0f);
    EXPECT_EQ((normalizedToFloat<25>(+16777215)), +1.0f);
    EXPECT_EQ((normalizedToFloat<25>(-16777215)), -1.0f);
    EXPECT_EQ((normalizedToFloat<25>(-16777216)), -1.0f);

    // Check near values
    EXPECT_NEAR((normalizedToFloat<25>(+8388608)), +0.5f, 0.00000006);
    EXPECT_NEAR((normalizedToFloat<25>(+8388607)), +0.5f, 0.00000006);
    EXPECT_NEAR((normalizedToFloat<25>(-8388607)), -0.5f, 0.00000006);
    EXPECT_NEAR((normalizedToFloat<25>(-8388608)), -0.5f, 0.00000006);
}

// Test normalizedToFloat conversions with 26-bit signed
TEST(MathUtilTest, NormalizedToFloatSnorm26)
{
    // Check exact values
    EXPECT_EQ((normalizedToFloat<26>(0)), 0.0f);
    EXPECT_EQ((normalizedToFloat<26>(+33554431)), +1.0f);
    EXPECT_EQ((normalizedToFloat<26>(-33554431)), -1.0f);
    EXPECT_EQ((normalizedToFloat<26>(-33554432)), -1.0f);

    // Check near values
    EXPECT_NEAR((normalizedToFloat<26>(+16777216)), +0.5f, 0.00000003);
    EXPECT_NEAR((normalizedToFloat<26>(+16777215)), +0.5f, 0.00000003);
    EXPECT_NEAR((normalizedToFloat<26>(-16777215)), -0.5f, 0.00000003);
    EXPECT_NEAR((normalizedToFloat<26>(-16777216)), -0.5f, 0.00000003);
}

}  // anonymous namespace
