//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ConstantFoldingNaN_test.cpp:
//   Tests for constant folding that results in NaN according to IEEE and should also generate a
//   warning. The ESSL spec does not mandate generating NaNs, but this is reasonable behavior in
//   this case.
//

#include "tests/test_utils/ConstantFoldingTest.h"

using namespace sh;

namespace
{

class ConstantFoldingNaNExpressionTest : public ConstantFoldingExpressionTest
{
  public:
    ConstantFoldingNaNExpressionTest() {}

    void evaluateFloatNaN(const std::string &floatString)
    {
        evaluateFloat(floatString);
        ASSERT_TRUE(constantFoundInAST(std::numeric_limits<float>::quiet_NaN()));
        ASSERT_TRUE(hasWarning());
    }
};

}  // anonymous namespace

// Test that infinity - infinity evaluates to NaN.
TEST_F(ConstantFoldingNaNExpressionTest, FoldInfinityMinusInfinity)
{
    const std::string &floatString = "1.0e2048 - 1.0e2048";
    evaluateFloatNaN(floatString);
}

// Test that infinity + negative infinity evaluates to NaN.
TEST_F(ConstantFoldingNaNExpressionTest, FoldInfinityPlusNegativeInfinity)
{
    const std::string &floatString = "1.0e2048 + (-1.0e2048)";
    evaluateFloatNaN(floatString);
}

// Test that infinity multiplied by zero evaluates to NaN.
TEST_F(ConstantFoldingNaNExpressionTest, FoldInfinityMultipliedByZero)
{
    const std::string &floatString = "1.0e2048 * 0.0";
    evaluateFloatNaN(floatString);
}

// Test that infinity divided by infinity evaluates to NaN.
TEST_F(ConstantFoldingNaNExpressionTest, FoldInfinityDividedByInfinity)
{
    const std::string &floatString = "1.0e2048 / 1.0e2048";
    evaluateFloatNaN(floatString);
}

// Test that zero divided by zero evaluates to NaN.
TEST_F(ConstantFoldingNaNExpressionTest, FoldZeroDividedByZero)
{
    const std::string &floatString = "0.0 / 0.0";
    evaluateFloatNaN(floatString);
}
