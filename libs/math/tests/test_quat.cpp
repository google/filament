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
#include <random>
#include <functional>
#include <type_traits>

#include <gtest/gtest.h>

#include <math/quat.h>
#include <math/mat4.h>
#include <math/vec4.h>
#include <math/vec3.h>
#include <math/scalar.h>

using namespace filament::math;

class QuatTest : public testing::Test {
protected:
};

TEST_F(QuatTest, Basics) {
    quat q;
    double4& v(q.xyzw);

    EXPECT_EQ(sizeof(quat), sizeof(double)*4);
    EXPECT_EQ(reinterpret_cast<void*>(&q), reinterpret_cast<void*>(&v));
}

TEST_F(QuatTest, Constructors) {
    quat q0;
    EXPECT_EQ(q0.x, 0);
    EXPECT_EQ(q0.y, 0);
    EXPECT_EQ(q0.z, 0);
    EXPECT_EQ(q0.w, 0);

    quat q1(1);
    EXPECT_EQ(q1.x, 0);
    EXPECT_EQ(q1.y, 0);
    EXPECT_EQ(q1.z, 0);
    EXPECT_EQ(q1.w, 1);

    quat q2(1, 2, 3, 4);
    EXPECT_EQ(q2.x, 2);
    EXPECT_EQ(q2.y, 3);
    EXPECT_EQ(q2.z, 4);
    EXPECT_EQ(q2.w, 1);

    quat q3(q2);
    EXPECT_EQ(q3.x, 2);
    EXPECT_EQ(q3.y, 3);
    EXPECT_EQ(q3.z, 4);
    EXPECT_EQ(q3.w, 1);

    quat q4(q3.xyz, 42);
    EXPECT_EQ(q4.x, 2);
    EXPECT_EQ(q4.y, 3);
    EXPECT_EQ(q4.z, 4);
    EXPECT_EQ(q4.w, 42);

    quat q5(double3(q2.xy, 42), 24);
    EXPECT_EQ(q5.x, 2);
    EXPECT_EQ(q5.y, 3);
    EXPECT_EQ(q5.z, 42);
    EXPECT_EQ(q5.w, 24);

    quat q6;
    q6 = 12;
    EXPECT_EQ(q6.x, 0);
    EXPECT_EQ(q6.y, 0);
    EXPECT_EQ(q6.z, 0);
    EXPECT_EQ(q6.w, 12);

    quat q7 = 1 + 2_i + 3_j + 4_k;
    EXPECT_EQ(q7.x, 2);
    EXPECT_EQ(q7.y, 3);
    EXPECT_EQ(q7.z, 4);
    EXPECT_EQ(q7.w, 1);

    quatf qf(2);
    EXPECT_EQ(qf.x, 0);
    EXPECT_EQ(qf.y, 0);
    EXPECT_EQ(qf.z, 0);
    EXPECT_EQ(qf.w, 2);
}

TEST_F(QuatTest, Access) {
    quat q0(1, 2, 3, 4);
    q0.x = 10;
    q0.y = 20;
    q0.z = 30;
    q0.w = 40;
    EXPECT_EQ(q0.x, 10);
    EXPECT_EQ(q0.y, 20);
    EXPECT_EQ(q0.z, 30);
    EXPECT_EQ(q0.w, 40);

    q0[0] = 100;
    q0[1] = 200;
    q0[2] = 300;
    q0[3] = 400;
    EXPECT_EQ(q0.x, 100);
    EXPECT_EQ(q0.y, 200);
    EXPECT_EQ(q0.z, 300);
    EXPECT_EQ(q0.w, 400);

    q0.xyz = double3(1, 2, 3);
    EXPECT_EQ(q0.x, 1);
    EXPECT_EQ(q0.y, 2);
    EXPECT_EQ(q0.z, 3);
    EXPECT_EQ(q0.w, 400);
}

TEST_F(QuatTest, UnaryOps) {
    quat q0(1, 2, 3, 4);

    q0 += 1;
    EXPECT_EQ(q0.x, 2);
    EXPECT_EQ(q0.y, 3);
    EXPECT_EQ(q0.z, 4);
    EXPECT_EQ(q0.w, 2);

    q0 -= 1;
    EXPECT_EQ(q0.x, 2);
    EXPECT_EQ(q0.y, 3);
    EXPECT_EQ(q0.z, 4);
    EXPECT_EQ(q0.w, 1);

    q0 *= 2;
    EXPECT_EQ(q0.x, 4);
    EXPECT_EQ(q0.y, 6);
    EXPECT_EQ(q0.z, 8);
    EXPECT_EQ(q0.w, 2);

    q0 /= 2;
    EXPECT_EQ(q0.x, 2);
    EXPECT_EQ(q0.y, 3);
    EXPECT_EQ(q0.z, 4);
    EXPECT_EQ(q0.w, 1);

    quat q1(10, 20, 30, 40);

    q0 += q1;
    EXPECT_EQ(q0.x, 22);
    EXPECT_EQ(q0.y, 33);
    EXPECT_EQ(q0.z, 44);
    EXPECT_EQ(q0.w, 11);

    q0 -= q1;
    EXPECT_EQ(q0.x, 2);
    EXPECT_EQ(q0.y, 3);
    EXPECT_EQ(q0.z, 4);
    EXPECT_EQ(q0.w, 1);

    q1 = -q1;
    EXPECT_EQ(q1.x, -20);
    EXPECT_EQ(q1.y, -30);
    EXPECT_EQ(q1.z, -40);
    EXPECT_EQ(q1.w, -10);

    // TODO(mathias): multiplies
}

TEST_F(QuatTest, ComparisonOps) {
    quat q0(1, 2, 3, 4);
    quat q1(10, 20, 30, 40);

    EXPECT_TRUE(q0 == q0);
    EXPECT_TRUE(q0 != q1);
    EXPECT_FALSE(q0 != q0);
    EXPECT_FALSE(q0 == q1);
}

TEST_F(QuatTest, ArithmeticOps) {
    quat q0(1, 2, 3, 4);
    quat q1(10, 20, 30, 40);

    quat q2(q0 + q1);
    EXPECT_EQ(q2.x, 22);
    EXPECT_EQ(q2.y, 33);
    EXPECT_EQ(q2.z, 44);
    EXPECT_EQ(q2.w, 11);

    q0 = q1 * 2;
    EXPECT_EQ(q0.x, 40);
    EXPECT_EQ(q0.y, 60);
    EXPECT_EQ(q0.z, 80);
    EXPECT_EQ(q0.w, 20);

    q0 = 2 * q1;
    EXPECT_EQ(q0.x, 40);
    EXPECT_EQ(q0.y, 60);
    EXPECT_EQ(q0.z, 80);
    EXPECT_EQ(q0.w, 20);

    quatf qf(2);
    q0 = q1 * qf;
    EXPECT_EQ(q0.x, 40);
    EXPECT_EQ(q0.y, 60);
    EXPECT_EQ(q0.z, 80);
    EXPECT_EQ(q0.w, 20);

    EXPECT_EQ(1_i * 1_i, quat(-1));
    EXPECT_EQ(1_j * 1_j, quat(-1));
    EXPECT_EQ(1_k * 1_k, quat(-1));
    EXPECT_EQ(1_i * 1_j * 1_k, quat(-1));
}

TEST_F(QuatTest, ArithmeticFunc) {
    quat q(1, 2, 3, 4);
    quat qc(conj(q));
    MATH_UNUSED quat qi(inverse(q));
    quat qn(normalize(q));

    EXPECT_EQ(qc.x, -2);
    EXPECT_EQ(qc.y, -3);
    EXPECT_EQ(qc.z, -4);
    EXPECT_EQ(qc.w,  1);

    EXPECT_EQ(~q, qc);
    EXPECT_EQ(length(q), length(qc));
    EXPECT_EQ(sqrt(30), length(q));
    EXPECT_DOUBLE_EQ(1, length(qn));
    EXPECT_DOUBLE_EQ(1, dot(qn, qn));

    quat qr = quat::fromAxisAngle(double3(0, 0, 1), F_PI / 2);
    EXPECT_EQ(mat4(qr).toQuaternion(), qr);
    EXPECT_EQ(1_i, mat4(1_i).toQuaternion());
    EXPECT_EQ(1_j, mat4(1_j).toQuaternion());
    EXPECT_EQ(1_k, mat4(1_k).toQuaternion());


    EXPECT_EQ(qr, log(exp(qr)));

    quat qq = qr * qr;
    quat q2 = pow(qr, 2);
    EXPECT_NEAR(qq.x, q2.x, 1e-15);
    EXPECT_NEAR(qq.y, q2.y, 1e-15);
    EXPECT_NEAR(qq.z, q2.z, 1e-15);
    EXPECT_NEAR(qq.w, q2.w, 1e-15);

    quat qa = quat::fromAxisAngle(double3(0, 0, 1), 0);
    quat qb = quat::fromAxisAngle(double3(0, 0, 1), F_PI / 2);
    quat qs = slerp(qa, qb, 0.5);
    qr = quat::fromAxisAngle(double3(0, 0, 1), F_PI / 4);
    EXPECT_DOUBLE_EQ(qr.x, qs.x);
    EXPECT_DOUBLE_EQ(qr.y, qs.y);
    EXPECT_DOUBLE_EQ(qr.z, qs.z);
    EXPECT_DOUBLE_EQ(qr.w, qs.w);

    qs = nlerp(qa, qb, 0.5);
    EXPECT_DOUBLE_EQ(qr.x, qs.x);
    EXPECT_DOUBLE_EQ(qr.y, qs.y);
    EXPECT_DOUBLE_EQ(qr.z, qs.z);
    EXPECT_DOUBLE_EQ(qr.w, qs.w);

    // Ensure that we're taking the shortest path.
    qa = {-0.707, 0, 0, 0.707};
    qb = {1, 0, 0, 0};
    qs = slerp(qa, qb, 0.5);
    EXPECT_NEAR(qs[3], -0.92, 0.1);
    EXPECT_NEAR(qs[2], +0.38, 0.1);

    // Create two quats that are near to each other, but with opposite signs.
    qa = { 0.76,   0.39,   0.51,  0.19};
    qb = {-0.759, -0.385, -0.50, -0.19};
    qs = slerp(qa, qb, 0.5);

    // The rotation angle produced by v * slerp(A, B, .5) should be between the rotation angles
    // produced by (v * A) and (v * B).
    double3 v(0, 0, 1);
    double3 va = qa * v;
    double3 vb = qb * v;
    double3 vs = qs * v;
    EXPECT_LT(dot(v, va), dot(v, vs));
    EXPECT_LT(dot(v, vs), dot(v, vb));
}

TEST_F(QuatTest, MultiplicationExhaustive) {
    std::default_random_engine generator(171717);
    std::uniform_real_distribution<double> distribution(-10.0, 10.0);
    auto rand_gen = std::bind(distribution, generator);

    for (size_t i = 0; i < (1024 * 1024); ++i) {
        double3 axis_a = normalize(double3(rand_gen(), rand_gen(), rand_gen()));
        double angle_a = rand_gen();
        quat a = quat::fromAxisAngle(axis_a, angle_a);

        double3 axis_b = normalize(double3(rand_gen(), rand_gen(), rand_gen()));
        double angle_b = rand_gen();
        quat b = quat::fromAxisAngle(axis_b, angle_b);

        quat ab = a * b;
        quat ab_other(a.w * b.xyz + b.w * a.xyz + cross(a.xyz, b.xyz),
            (a.w * b.w) - dot(a.xyz, b.xyz));

        ASSERT_FLOAT_EQ(ab.x, ab_other.x);
        ASSERT_FLOAT_EQ(ab.y, ab_other.y);
        ASSERT_FLOAT_EQ(ab.z, ab_other.z);
        ASSERT_FLOAT_EQ(ab.w, ab_other.w);
    }
}

TEST_F(QuatTest, NaN) {
    quatf qa = {.5, .5, .5, .5};
    quatf qb = {0.49995, 0.49998, 0.49998, 0.49995};
    quatf qs = slerp(qa, qb, 0.034934);

    EXPECT_NEAR(qs[0], 0.5, 0.1);
    EXPECT_NEAR(qs[1], 0.5, 0.1);
    EXPECT_NEAR(qs[2], 0.5, 0.1);
    EXPECT_NEAR(qs[3], 0.5, 0.1);
}

TEST_F(QuatTest, Conversions) {
    quat qd;
    quatf qf;
    float3 vf;
    double3 vd;
    double d = 0.0;
    float f = 0.0f;

    static_assert(std::is_same<details::arithmetic_result_t<float, float>, float>::value);
    static_assert(std::is_same<details::arithmetic_result_t<float, double>, double>::value);
    static_assert(std::is_same<details::arithmetic_result_t<double, float>, double>::value);
    static_assert(std::is_same<details::arithmetic_result_t<double, double>, double>::value);

    {
        auto r1 = qd * d;
        auto r2 = qd * f;
        auto r3 = qf * d;
        auto r4 = qf * f;
        static_assert(std::is_same<decltype(r1), quat>::value);
        static_assert(std::is_same<decltype(r2), quat>::value);
        static_assert(std::is_same<decltype(r3), quat>::value);
        static_assert(std::is_same<decltype(r4), quatf>::value);
    }
    {
        auto r1 = qd / d;
        auto r2 = qd / f;
        auto r3 = qf / d;
        auto r4 = qf / f;
        static_assert(std::is_same<decltype(r1), quat>::value);
        static_assert(std::is_same<decltype(r2), quat>::value);
        static_assert(std::is_same<decltype(r3), quat>::value);
        static_assert(std::is_same<decltype(r4), quatf>::value);
    }
    {
        auto r1 = d * qd;
        auto r2 = f * qd;
        auto r3 = d * qf;
        auto r4 = f * qf;
        static_assert(std::is_same<decltype(r1), quat>::value);
        static_assert(std::is_same<decltype(r2), quat>::value);
        static_assert(std::is_same<decltype(r3), quat>::value);
        static_assert(std::is_same<decltype(r4), quatf>::value);
    }
    {
        auto r1 = qd * vd;
        auto r2 = qf * vd;
        auto r3 = qd * vf;
        auto r4 = qf * vf;
        static_assert(std::is_same<decltype(r1), double3>::value);
        static_assert(std::is_same<decltype(r2), double3>::value);
        static_assert(std::is_same<decltype(r3), double3>::value);
        static_assert(std::is_same<decltype(r4), float3>::value);
    }
    {
        auto r1 = qd * qd;
        auto r2 = qf * qd;
        auto r3 = qd * qf;
        auto r4 = qf * qf;
        static_assert(std::is_same<decltype(r1), quat>::value);
        static_assert(std::is_same<decltype(r2), quat>::value);
        static_assert(std::is_same<decltype(r3), quat>::value);
        static_assert(std::is_same<decltype(r4), quatf>::value);
    }
    {
        auto r1 = dot(qd, qd);
        auto r2 = dot(qf, qd);
        auto r3 = dot(qd, qf);
        auto r4 = dot(qf, qf);
        static_assert(std::is_same<decltype(r1), double>::value);
        static_assert(std::is_same<decltype(r2), double>::value);
        static_assert(std::is_same<decltype(r3), double>::value);
        static_assert(std::is_same<decltype(r4), float>::value);
    }
    {
        auto r1 = cross(qd, qd);
        auto r2 = cross(qf, qd);
        auto r3 = cross(qd, qf);
        auto r4 = cross(qf, qf);
        static_assert(std::is_same<decltype(r1), quat>::value);
        static_assert(std::is_same<decltype(r2), quat>::value);
        static_assert(std::is_same<decltype(r3), quat>::value);
        static_assert(std::is_same<decltype(r4), quatf>::value);
    }
}

template <typename L, typename R, typename = void>
struct has_divide_assign : std::false_type {};

template <typename L, typename R>
struct has_divide_assign<L, R,
        decltype(std::declval<L&>() /= std::declval<R>(), void())> : std::true_type {};

// Static assertions to validate the availability of the /= operator for specific type
// combinations. The first static_assert checks that the quat does not have a /= operator with Foo.
// This ensures that quat does not provide an inappropriate overload that could be erroneously
// selected.
struct Foo {};
static_assert(!has_divide_assign<quat, Foo>::value);
static_assert(has_divide_assign<quat, float>::value);
