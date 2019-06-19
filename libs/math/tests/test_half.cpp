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

#include <math.h>

#include <gtest/gtest.h>

#include <math/half.h>
#include <math/vec4.h>

using namespace filament::math;

class HalfTest : public testing::Test {
protected:
};

TEST_F(HalfTest, Basics) {

    EXPECT_EQ(2, sizeof(half));

    // test +/- zero
    EXPECT_EQ(0x0000, getBits(half( 0.0f)));
    EXPECT_EQ(0x8000, getBits(half(-0.0f)));

    // test nan
    EXPECT_EQ(0x7e00, getBits(half(NAN)));

    // test +/- infinity
    EXPECT_EQ(0x7C00, getBits(half( std::numeric_limits<float>::infinity())));
    EXPECT_EQ(0xFC00, getBits(half(-std::numeric_limits<float>::infinity())));

    // test a few known values
    EXPECT_EQ(0x3C01, getBits(half(1.0009765625)));
    EXPECT_EQ(0xC000, getBits(half(-2)));
    EXPECT_EQ(0x0400, getBits(half(6.10352e-5)));
    EXPECT_EQ(0x7BFF, getBits(half(65504)));
    EXPECT_EQ(0x3555, getBits(half(1.0f/3)));

    // numeric limits
    EXPECT_EQ(0x7C00, getBits(std::numeric_limits<half>::infinity()));
    EXPECT_EQ(0x0400, getBits(std::numeric_limits<half>::min()));
    EXPECT_EQ(0x7BFF, getBits(std::numeric_limits<half>::max()));
    EXPECT_EQ(0xFBFF, getBits(std::numeric_limits<half>::lowest()));

    // denormals
    EXPECT_EQ(0x03FF, getBits(half( 6.09756e-5)));      // if handled, should be: 0x03FF
    EXPECT_EQ(0x0001, getBits(half( 5.96046e-8)));      // if handled, should be: 0x0001
    EXPECT_EQ(0x83FF, getBits(half(-6.09756e-5)));      // if handled, should be: 0x83FF
    EXPECT_EQ(0x8001, getBits(half(-5.96046e-8)));      // if handled, should be: 0x8001

    // test all exactly representable integers
    #pragma nounroll
    for (int i = -2048; i <= 2048; ++i) {
        half h = float(i);
        EXPECT_EQ(i, float(h));
    }
}

TEST_F(HalfTest, Literals) {
    half one = 1.0_h;
    half pi = 3.1415926_h;
    half minusTwo = -2.0_h;

    EXPECT_EQ(half(1.0f), one);
    EXPECT_EQ(half(3.1415926), pi);
    EXPECT_EQ(half(-2.0f), minusTwo);
}


TEST_F(HalfTest, Vec) {
    float4 f4(1,2,3,4);
    half4 h4(f4);
    half3 h3(f4.xyz);
    half2 h2(f4.xy);

    EXPECT_EQ(f4, h4);
    EXPECT_EQ(f4.xyz, h3);
    EXPECT_EQ(f4.xy, h2);
}


using fp10 = fp<0, 5, 5>;
using fp11 = fp<0, 5, 6>;

TEST_F(HalfTest, fp10) {
    // test all exactly representable integers
    #pragma nounroll
    for (int i = 0; i <= 64; ++i) {
        fp10 h = fp10::fromf(float(i));
        EXPECT_EQ(i, fp10::tof(h));
    }
}

TEST_F(HalfTest, fp11) {
    // test all exactly representable integers
    #pragma nounroll
    for (int i = 0; i <= 128; ++i) {
        fp11 h = fp11::fromf(float(i));
        EXPECT_EQ(i, fp11::tof(h));
    }
}