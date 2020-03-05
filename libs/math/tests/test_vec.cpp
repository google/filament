/*
 * Copyright 2013 The Android Open Source Project
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

#include <math/vec4.h>
#include <math/scalar.h>

using namespace filament::math;

class VecTest : public testing::Test {
protected:
};

TEST_F(VecTest, Constexpr) {
    constexpr float a = F_PI;
    constexpr float2 Z2{};
    constexpr float2 A2 = a;
    constexpr float2 B2 = { a, a };
    constexpr float2 C2 = A2;
    constexpr float2 E2 = A2 + 0.5f;
    constexpr float2 F2 = A2 + 0.5 - 1.0 + (1 + A2);
    constexpr float3 D2 = cross(A2, C2);

    constexpr float3 Z3{};
    constexpr float3 A3 = a;
    constexpr float3 B3 = { a, a, a };
    constexpr float3 C3 = A3;
    constexpr float3 D3 = { A2, a };
    constexpr float3 E3 = cross(A3, D3);

    constexpr float4 Z4{};
    constexpr float4 A4 = a;
    constexpr float4 B4 = { a, a, a, a };
    constexpr float4 C4 = A4;
    constexpr float4 D4 = { A2, a, a };
    constexpr float4 E4 = { A2, B2 };

    constexpr float4 AN4 = abs(A4);
    constexpr float4 BN4 = abs(B4);
    constexpr float4 CN4 = abs(C4);
    constexpr float4 DN4 = abs(D4);
    constexpr float4 EN4 = abs(E4);

    constexpr float4 AS4 = saturate(A4);
    constexpr float4 BS4 = saturate(B4);
    constexpr float4 CS4 = saturate(C4);
    constexpr float4 DS4 = saturate(D4);
    constexpr float4 ES4 = saturate(E4);

    constexpr float d0 = dot(A4, A4);
    constexpr float d1 = dot(B4, A4);
    constexpr float d2 = dot(C4, A4);
    constexpr float d4 = dot(D4, A4);

    constexpr float4 S0 = A4 + B4;
    constexpr float4 S1 = C4 - D4;
    constexpr float4 S2 = (a * A4) + (A4 * a);
    constexpr float4 S7 = (a / A4) + (A4 / a);
    constexpr float4 S3 = A4 * A4;
    constexpr float4 S4 = A4 / a;
    constexpr float4 S5 = A4 / A4;

    constexpr float4 S6 = -E4;

    constexpr bool b0 = A4 == B4;
    constexpr bool b1 = A4 != B4;

    constexpr bool4 b2 = equal(A4, B4);
    constexpr bool b3 = any(A4);
    constexpr bool b4 = all(A4);
}

TEST_F(VecTest, Basics) {
    double4 v4;
    double3& v3(v4.xyz);

    EXPECT_EQ(sizeof(double4), sizeof(double)*4);
    EXPECT_EQ(sizeof(double3), sizeof(double)*3);
    EXPECT_EQ(sizeof(double2), sizeof(double)*2);
    EXPECT_EQ(reinterpret_cast<void*>(&v3), reinterpret_cast<void*>(&v4));
}

TEST_F(VecTest, Constructors) {
    double4 v1(1);
    EXPECT_EQ(v1.x, 1);
    EXPECT_EQ(v1.y, 1);
    EXPECT_EQ(v1.z, 1);
    EXPECT_EQ(v1.w, 1);

    double4 v2(1, 2, 3, 4);
    EXPECT_EQ(v2.x, 1);
    EXPECT_EQ(v2.y, 2);
    EXPECT_EQ(v2.z, 3);
    EXPECT_EQ(v2.w, 4);

    double4 v3(v2);
    EXPECT_EQ(v3.x, 1);
    EXPECT_EQ(v3.y, 2);
    EXPECT_EQ(v3.z, 3);
    EXPECT_EQ(v3.w, 4);

    double4 v4(v3.xyz, 42);
    EXPECT_EQ(v4.x, 1);
    EXPECT_EQ(v4.y, 2);
    EXPECT_EQ(v4.z, 3);
    EXPECT_EQ(v4.w, 42);

    double4 v5(double3(v2.xy, 42), 24);
    EXPECT_EQ(v5.x, 1);
    EXPECT_EQ(v5.y, 2);
    EXPECT_EQ(v5.z, 42);
    EXPECT_EQ(v5.w, 24);

    float4 vf(2);
    EXPECT_EQ(vf.x, 2);
    EXPECT_EQ(vf.y, 2);
    EXPECT_EQ(vf.z, 2);
    EXPECT_EQ(vf.w, 2);
}

TEST_F(VecTest, Access) {
    double4 v0(1, 2, 3, 4);

#ifdef __EXCEPTIONS
    EXPECT_NO_THROW(v0[0]);
    EXPECT_NO_THROW(v0[1]);
    EXPECT_NO_THROW(v0[2]);
    EXPECT_NO_THROW(v0[3]);
// we removed the bounds check - maybe we should keep it in debug builds?
//    EXPECT_THROW(v0[4], std::out_of_range);
//    EXPECT_THROW(v0[5], std::out_of_range);
#endif

    v0.x = 10;
    v0.y = 20;
    v0.z = 30;
    v0.w = 40;
    EXPECT_EQ(v0.x, 10);
    EXPECT_EQ(v0.y, 20);
    EXPECT_EQ(v0.z, 30);
    EXPECT_EQ(v0.w, 40);

    v0[0] = 100;
    v0[1] = 200;
    v0[2] = 300;
    v0[3] = 400;
    EXPECT_EQ(v0.x, 100);
    EXPECT_EQ(v0.y, 200);
    EXPECT_EQ(v0.z, 300);
    EXPECT_EQ(v0.w, 400);

    v0.xyz = double3(1, 2, 3);
    EXPECT_EQ(v0.x, 1);
    EXPECT_EQ(v0.y, 2);
    EXPECT_EQ(v0.z, 3);
    EXPECT_EQ(v0.w, 400);
}

TEST_F(VecTest, UnaryOps) {
    double4 v0(1, 2, 3, 4);

    v0 += 1;
    EXPECT_EQ(v0.x, 2);
    EXPECT_EQ(v0.y, 3);
    EXPECT_EQ(v0.z, 4);
    EXPECT_EQ(v0.w, 5);

    v0 -= 1;
    EXPECT_EQ(v0.x, 1);
    EXPECT_EQ(v0.y, 2);
    EXPECT_EQ(v0.z, 3);
    EXPECT_EQ(v0.w, 4);

    v0 *= 2;
    EXPECT_EQ(v0.x, 2);
    EXPECT_EQ(v0.y, 4);
    EXPECT_EQ(v0.z, 6);
    EXPECT_EQ(v0.w, 8);

    v0 /= 2;
    EXPECT_EQ(v0.x, 1);
    EXPECT_EQ(v0.y, 2);
    EXPECT_EQ(v0.z, 3);
    EXPECT_EQ(v0.w, 4);

    double4 v1(10, 20, 30, 40);

    v0 += v1;
    EXPECT_EQ(v0.x, 11);
    EXPECT_EQ(v0.y, 22);
    EXPECT_EQ(v0.z, 33);
    EXPECT_EQ(v0.w, 44);

    v0 -= v1;
    EXPECT_EQ(v0.x, 1);
    EXPECT_EQ(v0.y, 2);
    EXPECT_EQ(v0.z, 3);
    EXPECT_EQ(v0.w, 4);

    v0 *= v1;
    EXPECT_EQ(v0.x, 10);
    EXPECT_EQ(v0.y, 40);
    EXPECT_EQ(v0.z, 90);
    EXPECT_EQ(v0.w, 160);

    v0 /= v1;
    EXPECT_EQ(v0.x, 1);
    EXPECT_EQ(v0.y, 2);
    EXPECT_EQ(v0.z, 3);
    EXPECT_EQ(v0.w, 4);

    v1 = -v1;
    EXPECT_EQ(v1.x, -10);
    EXPECT_EQ(v1.y, -20);
    EXPECT_EQ(v1.z, -30);
    EXPECT_EQ(v1.w, -40);

    float4 fv(1, 2, 3, 4);
    v1 += fv;
    EXPECT_EQ(v1.x,  -9);
    EXPECT_EQ(v1.y, -18);
    EXPECT_EQ(v1.z, -27);
    EXPECT_EQ(v1.w, -36);
}

TEST_F(VecTest, ComparisonOps) {
    double4 v0(1, 2, 3, 4);
    double4 v1(10, 20, 30, 40);

    EXPECT_TRUE(v0 == v0);
    EXPECT_TRUE(v0 != v1);
    EXPECT_FALSE(v0 != v0);
    EXPECT_FALSE(v0 == v1);
}

TEST_F(VecTest, ComparisonFunctions) {
    float4 v0(1, 2, 3, 4);
    float4 v1(10, 20, 30, 40);

    EXPECT_TRUE(all(equal(v0, v0)));
    EXPECT_TRUE(all(notEqual(v0, v1)));
    EXPECT_FALSE(any(notEqual(v0, v0)));
    EXPECT_FALSE(any(equal(v0, v1)));

    EXPECT_FALSE(all(lessThan(v0, v0)));
    EXPECT_TRUE(all(lessThanEqual(v0, v0)));
    EXPECT_FALSE(all(greaterThan(v0, v0)));
    EXPECT_TRUE(all(greaterThanEqual(v0, v0)));
    EXPECT_TRUE(all(lessThan(v0, v1)));
    EXPECT_TRUE(all(greaterThan(v1, v0)));
}

TEST_F(VecTest, ArithmeticOps) {
    double4 v0(1, 2, 3, 4);
    double4 v1(10, 20, 30, 40);

    double4 v2(v0 + v1);
    EXPECT_EQ(v2.x, 11);
    EXPECT_EQ(v2.y, 22);
    EXPECT_EQ(v2.z, 33);
    EXPECT_EQ(v2.w, 44);

    v0 = v1 * 2;
    EXPECT_EQ(v0.x, 20);
    EXPECT_EQ(v0.y, 40);
    EXPECT_EQ(v0.z, 60);
    EXPECT_EQ(v0.w, 80);

    v0 = 2 * v1;
    EXPECT_EQ(v0.x, 20);
    EXPECT_EQ(v0.y, 40);
    EXPECT_EQ(v0.z, 60);
    EXPECT_EQ(v0.w, 80);

    float4 vf(2);
    v0 = v1 * vf;
    EXPECT_EQ(v0.x, 20);
    EXPECT_EQ(v0.y, 40);
    EXPECT_EQ(v0.z, 60);
    EXPECT_EQ(v0.w, 80);
}

TEST_F(VecTest, ArithmeticFunc) {
    double3 east(1, 0, 0);
    double3 north(0, 1, 0);
    double3 up(cross(east, north));
    EXPECT_EQ(up, double3(0, 0, 1));
    EXPECT_EQ(dot(east, north), 0);
    EXPECT_EQ(length(east), 1);
    EXPECT_EQ(distance(east, north), sqrt(2));

    double3 v0(1, 2, 3);
    double3 vn(normalize(v0));
    EXPECT_FLOAT_EQ(1, length(vn));
    EXPECT_FLOAT_EQ(length(v0), dot(v0, vn));

    float3 vf(east);
    EXPECT_EQ(length(vf), 1);
}

TEST_F(VecTest, MiscFunc) {
    EXPECT_TRUE(any(float3(0, 0, 1)));
    EXPECT_FALSE(any(float3(0, 0, 0)));

    EXPECT_TRUE(all(float3(1, 1, 1)));
    EXPECT_FALSE(all(float3(0, 0, 1)));

    EXPECT_TRUE(any(bool3(false, false, true)));
    EXPECT_FALSE(any(bool3(false)));

    EXPECT_TRUE(all(bool3(true)));
    EXPECT_FALSE(all(bool3(false, false, true)));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
