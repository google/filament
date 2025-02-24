//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FloatLex_test.cpp:
// Tests for parsing floats in GLSL source.
//

#include <sstream>
#include <string>

#include "common/debug.h"
#include "common/mathutil.h"
#include "compiler/translator/util.h"
#include "gtest/gtest.h"

namespace
{

class StrtofClampParser
{
  public:
    static float Parse(std::string str)
    {
        float value;
        sh::strtof_clamp(str, &value);
        return value;
    }
};

class NumericLexFloatParser
{
  public:
    static float Parse(std::string str) { return sh::NumericLexFloat32OutOfRangeToInfinity(str); }
};

}  // anonymous namespace

template <typename T>
class FloatLexTest : public ::testing::Test
{
  public:
    FloatLexTest() {}

  protected:
    void SetUp() override {}

    void TearDown() override {}

    static bool ParsedMatches(std::string str, float expected)
    {
        return (T::Parse(str) == expected);
    }

    static bool IsInfinity(std::string str)
    {
        float f = T::Parse(str);
        return gl::isInf(f);
    }

    static std::string Zeros(size_t count) { return std::string(count, '0'); }
};

typedef ::testing::Types<StrtofClampParser, NumericLexFloatParser> FloatParserTypes;
TYPED_TEST_SUITE(FloatLexTest, FloatParserTypes);

TYPED_TEST(FloatLexTest, One)
{
    ASSERT_TRUE(TestFixture::ParsedMatches("1.0", 1.0f));
}

TYPED_TEST(FloatLexTest, Ten)
{
    ASSERT_TRUE(TestFixture::ParsedMatches("10.0", 10.0f));
}

TYPED_TEST(FloatLexTest, TenScientific)
{
    ASSERT_TRUE(TestFixture::ParsedMatches("1.0e1", 10.0f));
}

TYPED_TEST(FloatLexTest, ScientificWithSmallMantissa)
{
    std::stringstream ss;
    ss << "0." << TestFixture::Zeros(100) << "125e102";
    ASSERT_TRUE(TestFixture::ParsedMatches(ss.str(), 12.5f));
}

TYPED_TEST(FloatLexTest, ScientificWithLargeMantissa)
{
    std::stringstream ss;
    ss << "9" << TestFixture::Zeros(100) << ".0e-100";
    ASSERT_TRUE(TestFixture::ParsedMatches(ss.str(), 9.0f));
}

TYPED_TEST(FloatLexTest, ScientificWithVerySmallMantissa)
{
    std::stringstream ss;
    ss << "0." << TestFixture::Zeros(5000) << "125e5002";
    ASSERT_TRUE(TestFixture::ParsedMatches(ss.str(), 12.5f));
}

TYPED_TEST(FloatLexTest, ScientificWithVeryLargeMantissa)
{
    std::stringstream ss;
    ss << "9" << TestFixture::Zeros(5000) << ".0e-5000";
    ASSERT_TRUE(TestFixture::ParsedMatches(ss.str(), 9.0f));
}

TYPED_TEST(FloatLexTest, StartWithDecimalDot)
{
    ASSERT_TRUE(TestFixture::ParsedMatches(".125", 0.125f));
}

TYPED_TEST(FloatLexTest, EndWithDecimalDot)
{
    ASSERT_TRUE(TestFixture::ParsedMatches("123.", 123.0f));
}

TYPED_TEST(FloatLexTest, NoDecimalDot)
{
    ASSERT_TRUE(TestFixture::ParsedMatches("125e-2", 1.25f));
}

TYPED_TEST(FloatLexTest, EndStartWithDecimalDotScientific)
{
    ASSERT_TRUE(TestFixture::ParsedMatches(".625e-1", 0.0625f));
}

TYPED_TEST(FloatLexTest, EndWithDecimalDotScientific)
{
    ASSERT_TRUE(TestFixture::ParsedMatches("102400.e-2", 1024.0f));
}

TYPED_TEST(FloatLexTest, UppercaseE)
{
    ASSERT_TRUE(TestFixture::ParsedMatches("125E-2", 1.25f));
}

TYPED_TEST(FloatLexTest, PlusInExponent)
{
    ASSERT_TRUE(TestFixture::ParsedMatches("1E+2", 100.0f));
}

TYPED_TEST(FloatLexTest, SlightlyAboveMaxFloat)
{
    ASSERT_TRUE(TestFixture::IsInfinity("3.4029e38"));
}

TYPED_TEST(FloatLexTest, SlightlyBelowMaxFloat)
{
    ASSERT_FALSE(TestFixture::IsInfinity("3.4028e38"));
    ASSERT_TRUE(TestFixture::ParsedMatches("3.4028e38", 3.4028e38f));
}

TYPED_TEST(FloatLexTest, SlightlyAboveMaxFloatLargerMantissa)
{
    ASSERT_TRUE(TestFixture::IsInfinity("34.029e37"));
}

TYPED_TEST(FloatLexTest, SlightlyBelowMaxFloatLargerMantissa)
{
    ASSERT_FALSE(TestFixture::IsInfinity("34.028e37"));
    ASSERT_TRUE(TestFixture::ParsedMatches("34.028e37", 3.4028e38f));
}

TYPED_TEST(FloatLexTest, SlightlyAboveMaxFloatSmallerMantissa)
{
    ASSERT_TRUE(TestFixture::IsInfinity("0.34029e39"));
}

TYPED_TEST(FloatLexTest, SlightlyBelowMaxFloatSmallerMantissa)
{
    ASSERT_FALSE(TestFixture::IsInfinity("0.34028e39"));
    ASSERT_TRUE(TestFixture::ParsedMatches("0.34028e39", 3.4028e38f));
}

TYPED_TEST(FloatLexTest, SlightlyBelowMinSubnormalFloat)
{
    ASSERT_TRUE(TestFixture::ParsedMatches("1.0e-48", 0.0f));
}

TYPED_TEST(FloatLexTest, SlightlyAboveMinNormalFloat)
{
    ASSERT_FALSE(TestFixture::ParsedMatches("1.1754943E-38", 0.0f));
}

TYPED_TEST(FloatLexTest, ManySignificantDigits)
{
    ASSERT_TRUE(TestFixture::ParsedMatches("1.23456789", 1.23456789f));
}

TYPED_TEST(FloatLexTest, MantissaBitAboveMaxUint)
{
    ASSERT_TRUE(TestFixture::ParsedMatches("4294967299.", 4294967299.0f));
}

TYPED_TEST(FloatLexTest, ExponentBitAboveMaxInt)
{
    ASSERT_TRUE(TestFixture::IsInfinity("1.0e2147483649"));
}

TYPED_TEST(FloatLexTest, ExponentBitBelowMaxIntAndLargeMantissa)
{
    std::stringstream ss;
    ss << "1" << TestFixture::Zeros(32) << ".0e2147483640";
    ASSERT_TRUE(TestFixture::IsInfinity(ss.str()));
}

TYPED_TEST(FloatLexTest, ExponentBitAboveMinIntAndSmallMantissa)
{
    std::stringstream ss;
    ss << "0." << TestFixture::Zeros(32) << "1e-2147483640";
    ASSERT_TRUE(TestFixture::ParsedMatches(ss.str(), 0.0f));
}
