//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ConstantFoldingOverflow_test.cpp:
//   Tests for constant folding that results in floating point overflow.
//   In IEEE floating point, the overflow result depends on which of the various rounding modes is
//   chosen - it's either the maximum representable value or infinity.
//   ESSL 3.00.6 section 4.5.1 says that the rounding mode cannot be set and is undefined, so the
//   result in this case is not defined by the spec.
//   We decide to overflow to infinity and issue a warning.
//

#include "tests/test_utils/ConstantFoldingTest.h"

using namespace sh;

namespace
{

class ConstantFoldingOverflowExpressionTest : public ConstantFoldingExpressionTest
{
  public:
    ConstantFoldingOverflowExpressionTest() {}

    void evaluateFloatOverflow(const std::string &floatString, bool positive)
    {
        evaluateFloat(floatString);
        float expected = positive ? std::numeric_limits<float>::infinity()
                                  : -std::numeric_limits<float>::infinity();
        ASSERT_TRUE(constantFoundInAST(expected));
        ASSERT_TRUE(hasWarning());
    }
};

}  // anonymous namespace

// Test that addition that overflows is evaluated correctly.
TEST_F(ConstantFoldingOverflowExpressionTest, Add)
{
    const std::string &floatString = "2.0e38 + 2.0e38";
    evaluateFloatOverflow(floatString, true);
}

// Test that subtraction that overflows is evaluated correctly.
TEST_F(ConstantFoldingOverflowExpressionTest, Subtract)
{
    const std::string &floatString = "2.0e38 - (-2.0e38)";
    evaluateFloatOverflow(floatString, true);
}

// Test that multiplication that overflows is evaluated correctly.
TEST_F(ConstantFoldingOverflowExpressionTest, Multiply)
{
    const std::string &floatString = "1.0e30 * 1.0e10";
    evaluateFloatOverflow(floatString, true);
}

// Test that division that overflows is evaluated correctly.
TEST_F(ConstantFoldingOverflowExpressionTest, Divide)
{
    const std::string &floatString = "1.0e30 / 1.0e-10";
    evaluateFloatOverflow(floatString, true);
}
