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

#include <math/fast.h>
#include <math/scalar.h>

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>

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

template<typename T>
static void testUnsignedSaturation() {
    constexpr T kMax = std::numeric_limits<T>::max();
    constexpr T kMid = T(kMax / 2 + 1); // first value with the high bit set

    // Inputs are volatile so the compiler cannot constant-fold the (inlined) calls. This
    // ensures the actual saturating instructions (e.g. NEON UQADD/UQSUB on AArch64) are
    // executed and verified at runtime, rather than the compiler's model of them. This
    // mainly matters for optimized builds; debug builds don't fold these calls anyway.
    volatile T zero = 0;
    volatile T one = 1;
    volatile T mid = kMid;
    volatile T max = kMax;

    // qadd
    EXPECT_EQ(T(0),             fast::qadd(T(zero), T(zero)));
    EXPECT_EQ(T(2),             fast::qadd(T(one), T(one)));
    EXPECT_EQ(T(kMid + 1),      fast::qadd(T(mid), T(one)));
    EXPECT_EQ(kMax,             fast::qadd(T(mid), T(mid)));
    EXPECT_EQ(kMax,             fast::qadd(T(max), T(one)));
    EXPECT_EQ(kMax,             fast::qadd(T(max), T(max)));
    EXPECT_EQ(T(kMax - 1),      fast::qadd(T(mid - 1), T(mid - 1)));

    // qsub
    EXPECT_EQ(T(0),             fast::qsub(T(zero), T(zero)));
    EXPECT_EQ(T(0),             fast::qsub(T(zero), T(one)));
    EXPECT_EQ(T(0),             fast::qsub(T(one), T(max)));
    EXPECT_EQ(T(0),             fast::qsub(T(mid), T(max)));
    EXPECT_EQ(T(kMax - 1),      fast::qsub(T(max), T(one)));
    EXPECT_EQ(T(kMax - kMid),   fast::qsub(T(max), T(mid)));
    EXPECT_EQ(T(0),             fast::qsub(T(max), T(max)));

    // qinc / qdec
    EXPECT_EQ(kMax,             fast::qinc(T(max)));
    EXPECT_EQ(T(kMid + 1),      fast::qinc(T(mid)));
    EXPECT_EQ(T(0),             fast::qdec(T(zero)));
    EXPECT_EQ(T(kMid - 1),      fast::qdec(T(mid)));
}

TEST_F(FastTest, SaturatedArithmetic) {
    testUnsignedSaturation<uint8_t>();
    testUnsignedSaturation<uint16_t>();
    testUnsignedSaturation<uint32_t>();

    // values from https://github.com/google/filament/issues/10470
    volatile uint8_t a8 = 200, b8 = 100, c8 = 10, d8 = 20;
    EXPECT_EQ(uint8_t(255), fast::qadd(uint8_t(a8), uint8_t(b8)));
    EXPECT_EQ(uint8_t(200), fast::qadd(uint8_t(b8), uint8_t(b8)));
    EXPECT_EQ(uint8_t(100), fast::qsub(uint8_t(a8), uint8_t(b8)));
    EXPECT_EQ(uint8_t(0),   fast::qsub(uint8_t(c8), uint8_t(d8)));

    volatile uint16_t a16 = 60000, b16 = 10000;
    EXPECT_EQ(uint16_t(65535), fast::qadd(uint16_t(a16), uint16_t(b16)));
    EXPECT_EQ(uint16_t(0),     fast::qsub(uint16_t(b16), uint16_t(a16)));
}
