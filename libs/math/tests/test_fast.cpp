/*
 * Copyright (C) 2016 The Android Open Source Project
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

#include <math/fast.h>
#include <math/scalar.h>

using namespace filament::math;

class FastTest : public testing::Test {
protected:
};

TEST_F(FastTest, Trig) {
    constexpr float sqrt1_2f = (float) F_SQRT1_2;
    constexpr double sqrt1_2d = F_SQRT1_2;
    constexpr float abs_error = 0.002f; // 0.2%

    EXPECT_FLOAT_EQ( 0.0f,      fast::sin<float>(-F_PI));
    EXPECT_NEAR    (-sqrt1_2f,  fast::sin<float>(-F_PI_2 - F_PI_4), abs_error);
    EXPECT_FLOAT_EQ(-1.0f,      fast::sin<float>(-F_PI_2));
    EXPECT_NEAR    (-sqrt1_2f,  fast::sin<float>(-F_PI_4), abs_error);
    EXPECT_FLOAT_EQ( 0.0f,      fast::sin<float>(0.0));
    EXPECT_NEAR    ( sqrt1_2f,  fast::sin<float>(F_PI_4), abs_error);
    EXPECT_FLOAT_EQ( 1.0f,      fast::sin<float>(F_PI_2));
    EXPECT_NEAR    ( sqrt1_2f,  fast::sin<float>(F_PI_2 + F_PI_4), abs_error);
    EXPECT_FLOAT_EQ( 0.0f,      fast::sin<float>(F_PI));

    EXPECT_FLOAT_EQ(-1.0f,      fast::cos<float>(-F_PI));
    EXPECT_NEAR    (-sqrt1_2f,  fast::cos<float>(-F_PI_2 - F_PI_4), abs_error);
    EXPECT_FLOAT_EQ( 0.0f,      fast::cos<float>(-F_PI_2));
    EXPECT_NEAR    (sqrt1_2f,   fast::cos<float>(-F_PI_4), abs_error);
    EXPECT_FLOAT_EQ( 1.0f,      fast::cos<float>(0.0));
    EXPECT_NEAR    (sqrt1_2f,   fast::cos<float>(F_PI_4), abs_error);
    EXPECT_FLOAT_EQ( 0.0f,      fast::cos<float>(F_PI_2));
    EXPECT_NEAR    (-sqrt1_2f,  fast::cos<float>(F_PI_2 + F_PI_4), abs_error);
    EXPECT_FLOAT_EQ(-1.0f,      fast::cos<float>(F_PI));

    EXPECT_FLOAT_EQ( 0.0f,      fast::sin<double>(-F_PI));
    EXPECT_NEAR    (-sqrt1_2d,  fast::sin<double>(-F_PI_2 - F_PI_4), abs_error);
    EXPECT_FLOAT_EQ(-1.0f,      fast::sin<double>(-F_PI_2));
    EXPECT_NEAR    (-sqrt1_2d,  fast::sin<double>(-F_PI_4), abs_error);
    EXPECT_FLOAT_EQ( 0.0f,      fast::sin<double>(0.0));
    EXPECT_NEAR    ( sqrt1_2d,  fast::sin<double>(F_PI_4), abs_error);
    EXPECT_FLOAT_EQ( 1.0f,      fast::sin<double>(F_PI_2));
    EXPECT_NEAR    ( sqrt1_2d,  fast::sin<double>(F_PI_2 + F_PI_4), abs_error);
    EXPECT_FLOAT_EQ( 0.0f,      fast::sin<double>(F_PI));

    EXPECT_FLOAT_EQ(-1.0f,      fast::cos<double>(-F_PI));
    EXPECT_NEAR    (-sqrt1_2d,  fast::cos<double>(-F_PI_2 - F_PI_4), abs_error);
    EXPECT_FLOAT_EQ( 0.0f,      fast::cos<double>(-F_PI_2));
    EXPECT_NEAR    (sqrt1_2d,   fast::cos<double>(-F_PI_4), abs_error);
    EXPECT_FLOAT_EQ( 1.0f,      fast::cos<double>(0.0));
    EXPECT_NEAR    (sqrt1_2d,   fast::cos<double>(F_PI_4), abs_error);
    EXPECT_FLOAT_EQ( 0.0f,      fast::cos<double>(F_PI_2));
    EXPECT_NEAR    (-sqrt1_2d,  fast::cos<double>(F_PI_2 + F_PI_4), abs_error);
    EXPECT_FLOAT_EQ(-1.0f,      fast::cos<double>(F_PI));
}
