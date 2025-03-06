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

#include "src/tint/utils/strconv/float_to_string.h"

#include <math.h>
#include <cstring>
#include <limits>

#include "gtest/gtest.h"
#include "src/tint/utils/memory/bitcast.h"

namespace tint::strconv {
namespace {

////////////////////////////////////////////////////////////////////////////////
// FloatToString                                                              //
////////////////////////////////////////////////////////////////////////////////

TEST(FloatToStringTest, Zero) {
    EXPECT_EQ(FloatToString(0.0f), "0.0");
}

TEST(FloatToStringTest, One) {
    EXPECT_EQ(FloatToString(1.0f), "1.0");
}

TEST(FloatToStringTest, MinusOne) {
    EXPECT_EQ(FloatToString(-1.0f), "-1.0");
}

TEST(FloatToStringTest, Billion) {
    EXPECT_EQ(FloatToString(1e9f), "1000000000.0");
}

TEST(FloatToStringTest, Small) {
    EXPECT_NE(FloatToString(std::numeric_limits<float>::epsilon()), "0.0");
}

TEST(FloatToStringTest, Highest) {
    const auto highest = std::numeric_limits<float>::max();
    const auto expected_highest = 340282346638528859811704183484516925440.0f;
    if (highest < expected_highest || highest > expected_highest) {
        GTEST_SKIP() << "std::numeric_limits<float>::max() is not as expected for "
                        "this target";
    }
    EXPECT_EQ(FloatToString(std::numeric_limits<float>::max()),
              "340282346638528859811704183484516925440.0");
}

TEST(FloatToStringTest, Lowest) {
    // Some compilers complain if you test floating point numbers for equality.
    // So say it via two inequalities.
    const auto lowest = std::numeric_limits<float>::lowest();
    const auto expected_lowest = -340282346638528859811704183484516925440.0f;
    if (lowest < expected_lowest || lowest > expected_lowest) {
        GTEST_SKIP() << "std::numeric_limits<float>::lowest() is not as expected for "
                        "this target";
    }
    EXPECT_EQ(FloatToString(std::numeric_limits<float>::lowest()),
              "-340282346638528859811704183484516925440.0");
}

TEST(FloatToStringTest, Precision) {
    EXPECT_EQ(FloatToString(1e-8f), "0.00000000999999993923");
    EXPECT_EQ(FloatToString(1e-9f), "0.00000000099999997172");
    EXPECT_EQ(FloatToString(1e-10f), "0.00000000010000000134");
    EXPECT_EQ(FloatToString(1e-20f), "0.00000000000000000001");
}

////////////////////////////////////////////////////////////////////////////////
// FloatToBitPreservingString                                                 //
////////////////////////////////////////////////////////////////////////////////

TEST(FloatToBitPreservingStringTest, Zero) {
    EXPECT_EQ(FloatToBitPreservingString(0.0f), "0.0");
}

TEST(FloatToBitPreservingStringTest, NegativeZero) {
    EXPECT_EQ(FloatToBitPreservingString(-0.0f), "-0.0");
}

TEST(FloatToBitPreservingStringTest, One) {
    EXPECT_EQ(FloatToBitPreservingString(1.0f), "1.0");
}

TEST(FloatToBitPreservingStringTest, MinusOne) {
    EXPECT_EQ(FloatToBitPreservingString(-1.0f), "-1.0");
}

TEST(FloatToBitPreservingStringTest, Billion) {
    EXPECT_EQ(FloatToBitPreservingString(1e9f), "1000000000.0");
}

TEST(FloatToBitPreservingStringTest, Small) {
    EXPECT_NE(FloatToBitPreservingString(std::numeric_limits<float>::epsilon()), "0.0");
}

TEST(FloatToBitPreservingStringTest, Highest) {
    const auto highest = std::numeric_limits<float>::max();
    const auto expected_highest = 340282346638528859811704183484516925440.0f;
    if (highest < expected_highest || highest > expected_highest) {
        GTEST_SKIP() << "std::numeric_limits<float>::max() is not as expected for "
                        "this target";
    }
    EXPECT_EQ(FloatToBitPreservingString(std::numeric_limits<float>::max()),
              "340282346638528859811704183484516925440.0");
}

TEST(FloatToBitPreservingStringTest, Lowest) {
    // Some compilers complain if you test floating point numbers for equality.
    // So say it via two inequalities.
    const auto lowest = std::numeric_limits<float>::lowest();
    const auto expected_lowest = -340282346638528859811704183484516925440.0f;
    if (lowest < expected_lowest || lowest > expected_lowest) {
        GTEST_SKIP() << "std::numeric_limits<float>::lowest() is not as expected for "
                        "this target";
    }
    EXPECT_EQ(FloatToBitPreservingString(std::numeric_limits<float>::lowest()),
              "-340282346638528859811704183484516925440.0");
}

TEST(FloatToBitPreservingStringTest, SmallestDenormal) {
    EXPECT_EQ(FloatToBitPreservingString(0x1p-149f), "0x1p-149");
    EXPECT_EQ(FloatToBitPreservingString(-0x1p-149f), "-0x1p-149");
}

TEST(FloatToBitPreservingStringTest, BiggerDenormal) {
    EXPECT_EQ(FloatToBitPreservingString(0x1p-148f), "0x1p-148");
    EXPECT_EQ(FloatToBitPreservingString(-0x1p-148f), "-0x1p-148");
}

TEST(FloatToBitPreservingStringTest, LargestDenormal) {
    static_assert(0x0.fffffep-126f == 0x1.fffffcp-127f);
    EXPECT_EQ(FloatToBitPreservingString(0x0.fffffep-126f), "0x1.fffffcp-127");
}

TEST(FloatToBitPreservingStringTest, Subnormal_cafebe) {
    EXPECT_EQ(FloatToBitPreservingString(0x1.2bfaf8p-127f), "0x1.2bfaf8p-127");
    EXPECT_EQ(FloatToBitPreservingString(-0x1.2bfaf8p-127f), "-0x1.2bfaf8p-127");
}

TEST(FloatToBitPreservingStringTest, Subnormal_aaaaa) {
    EXPECT_EQ(FloatToBitPreservingString(0x1.55554p-130f), "0x1.55554p-130");
    EXPECT_EQ(FloatToBitPreservingString(-0x1.55554p-130f), "-0x1.55554p-130");
}

TEST(FloatToBitPreservingStringTest, Infinity) {
    EXPECT_EQ(FloatToBitPreservingString(INFINITY), "0x1p+128");
    EXPECT_EQ(FloatToBitPreservingString(-INFINITY), "-0x1p+128");
}

TEST(FloatToBitPreservingStringTest, NaN) {
    // TODO(crbug.com/tint/1714): On x86, this bitcast will set bit 22 (the highest mantissa bit) to
    // 1, regardless of the bit value in the integer. This is likely due to IEEE 754's
    // recommendation that that the highest mantissa bit differentiates quiet NaNs from signalling
    // NaNs. On x86, float return values usually go via the FPU which can transform the signalling
    // NaN bit (0) to quiet NaN (1). As NaN floating point numbers can be silently modified by the
    // architecture, and the signalling bit is architecture defined, this test may fail on other
    // architectures.
    auto nan = tint::Bitcast<float>(0x7fc0beef);
    EXPECT_EQ(FloatToBitPreservingString(nan), "0x1.817ddep+128");
    EXPECT_EQ(FloatToBitPreservingString(-nan), "-0x1.817ddep+128");
}

////////////////////////////////////////////////////////////////////////////////
// DoubleToString                                                              //
////////////////////////////////////////////////////////////////////////////////

TEST(DoubleToStringTest, Zero) {
    EXPECT_EQ(DoubleToString(0.000000000), "0.0");
}

TEST(DoubleToStringTest, One) {
    EXPECT_EQ(DoubleToString(1.000000000), "1.0");
}

TEST(DoubleToStringTest, MinusOne) {
    EXPECT_EQ(DoubleToString(-1.000000000), "-1.0");
}

TEST(DoubleToStringTest, Billion) {
    EXPECT_EQ(DoubleToString(1e9), "1000000000.0");
}

TEST(DoubleToStringTest, Small) {
    EXPECT_NE(DoubleToString(std::numeric_limits<double>::epsilon()), "0.0");
}

TEST(DoubleToStringTest, Highest) {
    const auto highest = std::numeric_limits<double>::max();
    const auto expected_highest = 1.797693134862315708e+308;
    if (highest < expected_highest || highest > expected_highest) {
        GTEST_SKIP() << "std::numeric_limits<double>::max() is not as expected for "
                        "this target";
    }
    EXPECT_EQ(DoubleToString(std::numeric_limits<double>::max()),
              "179769313486231570814527423731704356798070567525844996598917476803157260780028538760"
              "589558632766878171540458953514382464234321326889464182768467546703537516986049910576"
              "551282076245490090389328944075868508455133942304583236903222948165808559332123348274"
              "797826204144723168738177180919299881250404026184124858368.0");
}

TEST(DoubleToStringTest, Lowest) {
    // Some compilers complain if you test floating point numbers for equality.
    // So say it via two inequalities.
    const auto lowest = std::numeric_limits<double>::lowest();
    const auto expected_lowest = -1.797693134862315708e+308;
    if (lowest < expected_lowest || lowest > expected_lowest) {
        GTEST_SKIP() << "std::numeric_limits<double>::lowest() is not as expected for "
                        "this target";
    }
    EXPECT_EQ(DoubleToString(std::numeric_limits<double>::lowest()),
              "-17976931348623157081452742373170435679807056752584499659891747680315726078002853876"
              "058955863276687817154045895351438246423432132688946418276846754670353751698604991057"
              "655128207624549009038932894407586850845513394230458323690322294816580855933212334827"
              "4797826204144723168738177180919299881250404026184124858368.0");
}

TEST(DoubleToStringTest, Precision) {
    EXPECT_EQ(DoubleToString(1e-8), "0.00000001");
    EXPECT_EQ(DoubleToString(1e-9), "0.000000001");
    EXPECT_EQ(DoubleToString(1e-10), "0.0000000001");
    EXPECT_EQ(DoubleToString(1e-15), "0.000000000000001");
}

////////////////////////////////////////////////////////////////////////////////
// DoubleToBitPreservingString                                                 //
////////////////////////////////////////////////////////////////////////////////

TEST(DoubleToBitPreservingStringTest, Zero) {
    EXPECT_EQ(DoubleToBitPreservingString(0.0), "0.0");
}

TEST(DoubleToBitPreservingStringTest, NegativeZero) {
    EXPECT_EQ(DoubleToBitPreservingString(-0.0), "-0.0");
}

TEST(DoubleToBitPreservingStringTest, One) {
    EXPECT_EQ(DoubleToBitPreservingString(1.0), "1.0");
}

TEST(DoubleToBitPreservingStringTest, MinusOne) {
    EXPECT_EQ(DoubleToBitPreservingString(-1.0), "-1.0");
}

TEST(DoubleToBitPreservingStringTest, Billion) {
    EXPECT_EQ(DoubleToBitPreservingString(1e9), "1000000000.0");
}

TEST(DoubleToBitPreservingStringTest, Small) {
    EXPECT_NE(DoubleToBitPreservingString(std::numeric_limits<double>::epsilon()), "0.0");
}

TEST(DoubleToBitPreservingStringTest, Highest) {
    const auto highest = std::numeric_limits<double>::max();
    const auto expected_highest = 1.797693134862315708e+308;
    if (highest < expected_highest || highest > expected_highest) {
        GTEST_SKIP() << "std::numeric_limits<float>::max() is not as expected for "
                        "this target";
    }
    EXPECT_EQ(DoubleToBitPreservingString(std::numeric_limits<double>::max()),
              "179769313486231570814527423731704356798070567525844996598917476803157260780028538760"
              "589558632766878171540458953514382464234321326889464182768467546703537516986049910576"
              "551282076245490090389328944075868508455133942304583236903222948165808559332123348274"
              "797826204144723168738177180919299881250404026184124858368.0");
}

TEST(DoubleToBitPreservingStringTest, Lowest) {
    // Some compilers complain if you test floating point numbers for equality.
    // So say it via two inequalities.
    const auto lowest = std::numeric_limits<double>::lowest();
    const auto expected_lowest = -1.797693134862315708e+308;
    if (lowest < expected_lowest || lowest > expected_lowest) {
        GTEST_SKIP() << "std::numeric_limits<float>::lowest() is not as expected for "
                        "this target";
    }
    EXPECT_EQ(DoubleToBitPreservingString(std::numeric_limits<double>::lowest()),
              "-17976931348623157081452742373170435679807056752584499659891747680315726078002853876"
              "058955863276687817154045895351438246423432132688946418276846754670353751698604991057"
              "655128207624549009038932894407586850845513394230458323690322294816580855933212334827"
              "4797826204144723168738177180919299881250404026184124858368.0");
}

TEST(DoubleToBitPreservingStringTest, SmallestDenormal) {
    EXPECT_EQ(DoubleToBitPreservingString(0x1p-1074), "0x1p-1074");
    EXPECT_EQ(DoubleToBitPreservingString(-0x1p-1074), "-0x1p-1074");
}

TEST(DoubleToBitPreservingStringTest, BiggerDenormal) {
    EXPECT_EQ(DoubleToBitPreservingString(0x1p-1073), "0x1p-1073");
    EXPECT_EQ(DoubleToBitPreservingString(-0x1p-1073), "-0x1p-1073");
}

TEST(DoubleToBitPreservingStringTest, LargestDenormal) {
    static_assert(0x0.fffffffffffffp-1022 == 0x1.ffffffffffffep-1023);
    EXPECT_EQ(DoubleToBitPreservingString(0x0.fffffffffffffp-1022), "0x1.ffffffffffffep-1023");
    EXPECT_EQ(DoubleToBitPreservingString(-0x0.fffffffffffffp-1022), "-0x1.ffffffffffffep-1023");
}

TEST(DoubleToBitPreservingStringTest, Subnormal_cafef00dbeef) {
    EXPECT_EQ(DoubleToBitPreservingString(0x1.cafef00dbeefp-1023), "0x1.cafef00dbeefp-1023");
    EXPECT_EQ(DoubleToBitPreservingString(-0x1.cafef00dbeefp-1023), "-0x1.cafef00dbeefp-1023");
}

TEST(DoubleToBitPreservingStringTest, Subnormal_aaaaaaaaaaaaap) {
    static_assert(0x0.aaaaaaaaaaaaap-1023 == 0x1.5555555555554p-1024);
    EXPECT_EQ(DoubleToBitPreservingString(0x0.aaaaaaaaaaaaap-1023), "0x1.5555555555554p-1024");
    EXPECT_EQ(DoubleToBitPreservingString(-0x0.aaaaaaaaaaaaap-1023), "-0x1.5555555555554p-1024");
}

TEST(DoubleToBitPreservingStringTest, Infinity) {
    EXPECT_EQ(DoubleToBitPreservingString(static_cast<double>(INFINITY)), "0x1p+1024");
    EXPECT_EQ(DoubleToBitPreservingString(static_cast<double>(-INFINITY)), "-0x1p+1024");
}

TEST(DoubleToBitPreservingStringTest, NaN) {
    auto nan = tint::Bitcast<double>(0x7ff8cafef00dbeefull);
    EXPECT_EQ(DoubleToBitPreservingString(static_cast<double>(nan)), "0x1.8cafef00dbeefp+1024");
    EXPECT_EQ(DoubleToBitPreservingString(static_cast<double>(-nan)), "-0x1.8cafef00dbeefp+1024");
}

}  // namespace
}  // namespace tint::strconv
