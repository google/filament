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

#include <gtest/gtest.h>

#include <limits>
#include <random>
#include <functional>

#include <math/mat2.h>
#include <math/mat4.h>
#include <math/mat3.h>
#include <math/quat.h>
#include <math/scalar.h>

using namespace filament::math;

class MatTest : public testing::Test {
protected:
};

//------------------------------------------------------------------------------
// A macro to help with vector comparisons within floating point range.
#define EXPECT_VEC_EQ(VEC1, VEC2)                               \
do {                                                            \
    const decltype(VEC1) v1 = VEC1;                             \
    const decltype(VEC2) v2 = VEC2;                             \
    if (std::is_same<TypeParam,float>::value) {                 \
        for (int i = 0; i < v1.size(); ++i) {                   \
            EXPECT_FLOAT_EQ(v1[i], v2[i]);                      \
        }                                                       \
    } else if (std::is_same<TypeParam,double>::value) {         \
        for (int i = 0; i < v1.size(); ++i) {                   \
            EXPECT_DOUBLE_EQ(v1[i], v2[i]);                     \
        }                                                       \
    } else {                                                    \
        for (int i = 0; i < v1.size(); ++i) {                   \
            EXPECT_EQ(v1[i], v2[i]);                            \
        }                                                       \
    }                                                           \
} while(0)

//------------------------------------------------------------------------------
// A macro to help with vector comparisons within a range.
#define EXPECT_VEC_NEAR(VEC1, VEC2, eps)                        \
do {                                                            \
    const decltype(VEC1) v1 = VEC1;                             \
    const decltype(VEC2) v2 = VEC2;                             \
    for (int i = 0; i < v1.size(); ++i) {                       \
        EXPECT_NEAR(v1[i], v2[i], eps);                         \
    }                                                           \
} while(0)


//------------------------------------------------------------------------------
// A macro to help with type comparisons within floating point range.
#define ASSERT_TYPE_EQ(T1, T2)                                  \
do {                                                            \
    const decltype(T1) t1 = T1;                                 \
    const decltype(T2) t2 = T2;                                 \
    if (std::is_same<TypeParam,float>::value) {                 \
        ASSERT_FLOAT_EQ(t1, t2);                                \
    } else if (std::is_same<TypeParam,double>::value) {         \
        ASSERT_DOUBLE_EQ(t1, t2);                               \
    } else {                                                    \
        ASSERT_EQ(t1, t2);                                      \
    }                                                           \
} while(0)



TEST_F(MatTest, LargeFloatRotationsWithOrthogonalization) {
     double3 const t = { 2304097.1410110965, -4688442.9915525438, -3639452.5611694567 };
     mat4 const T = mat4::translation(t);
    for (float d = 0; d < 90; d = d + 1.0) {
        mat3f const R = mat3f::rotation(d * f::DEG_TO_RAD, float3{ 0, 1, 0 });
        mat3 RR = orthogonalize(mat3{ R });
        ASSERT_NEAR(dot(RR[0], RR[0]), 1.0, 1e-12);
        ASSERT_NEAR(dot(RR[1], RR[1]), 1.0, 1e-12);
        ASSERT_NEAR(dot(RR[2], RR[2]), 1.0, 1e-12);
        mat4 M = mat4{ RR } * T;
        double3 const t2 = transpose(M.upperLeft()) * M[3].xyz;
        EXPECT_VEC_NEAR(t, t2, 0.0001);  // 0.1mm
     }
}

TEST_F(MatTest, ConstexprMat2) {
    constexpr float a = F_PI;
    constexpr mat2f M;
    constexpr mat2f M0(a);
    constexpr mat2f M1(float2{a, a});
    constexpr mat2f M2(1,2,3,4);
    constexpr mat2f M3(M2);
    constexpr mat2f M4(float2{1,2}, float2{3,4});
    constexpr float2 f0 = M0 * float2{1,2};
    constexpr float2 f1 = float2{1,2} * M1;
    CONSTEXPR_IF_NOT_MSVC mat2f M5 = M2 * 2;
    CONSTEXPR_IF_NOT_MSVC mat2f M7 = 2 * M2;
    constexpr float2 f3 = diag(M0);
    constexpr mat2f M8 = transpose(M0);
    constexpr mat2f M9 = inverse(M0);
    constexpr mat2f M12 = abs(M0);
    constexpr mat2f M11 = details::matrix::cof(M0);
    constexpr mat2f M10 = M8 * M9;
    constexpr float s0 = trace(M0);
    constexpr float f4 = M[0][0];
    constexpr float f5 = M(0, 0);
}

TEST_F(MatTest, ConstexprMat3) {
    constexpr float a = F_PI;
    constexpr mat3f M;
    constexpr mat3f M0(a);
    constexpr mat3f M1(float3{a, a, a});
    constexpr mat3f M2(1,2,3,4,5,6,7,8,9);
    constexpr mat3f M3(M2);
    constexpr mat3f M4(float3{1,2,3}, float3{4,5,6}, float3{7,8,9});
    constexpr float3 f0 = M0 * float3{1,2,3};
    constexpr float3 f1 = float3{1,2,3} * M1;
    CONSTEXPR_IF_NOT_MSVC mat3f M5 = M2 * 2;
    CONSTEXPR_IF_NOT_MSVC mat3f M7 = 2 * M2;
    constexpr float3 f3 = diag(M0);
    constexpr mat3f M8 = transpose(M0);
    constexpr mat3f M9 = inverse(M0);
    constexpr mat3f M12 = details::matrix::cof(M0);
    constexpr mat3f M10 = M8 * M9;
    constexpr float s0 = trace(M0);
    constexpr quatf q;
    constexpr mat3f M11{q};
}

TEST_F(MatTest, ConstexprMat4) {
    constexpr float a = F_PI;
    constexpr mat4f M;
    constexpr mat4f M0(a);
    constexpr mat4f M1(float4{a, a, a, a});
    constexpr mat4f M2(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16);
    constexpr mat4f M3(M2);
    constexpr mat4f M4(float4{1,2,3,4}, float4{5,6,7,8}, float4{9,10,11,12}, float4{13,14,15,16});
    constexpr float4 f0 = M0 * float4{1,2,3,4};
    constexpr float4 f1 = float4{1,2,3,4} * M1;
    CONSTEXPR_IF_NOT_MSVC mat4f M5 = M2 * 2;
    CONSTEXPR_IF_NOT_MSVC mat4f M7 = 2 * M2;
    constexpr float4 f3 = diag(M0);
    constexpr mat4f M8 = transpose(M0);
    constexpr mat4f M9 = inverse(M0);
    constexpr mat4f M16 = details::matrix::cof(M0);
    constexpr mat4f M10 = M8 * M9;
    constexpr float s0 = trace(M0);
    constexpr quatf q;
    constexpr mat4f M11{q};
    constexpr mat4f M13{mat3f{}};
    constexpr mat4f M14{mat3f{}, float3{}};
    constexpr mat4f M15{mat3f{}, float4{}};
    constexpr mat4f O = mat4f::ortho(0, 1, 0, 1, -1, 1);
    constexpr mat4f F = mat4f::frustum(0, 1, 0, 1, -1, 1);
    constexpr float4 f4 = mat4f::project(F, float4{1,2,3,1});
    constexpr float3 f5 = mat4f::project(F, float3{1,2,3});
    constexpr mat3f U = M11.upperLeft();
    constexpr mat4f T = mat4f::translation(f5);
    constexpr mat4f S = mat4f::scaling(f5);
    constexpr mat4f V = mat4f::scaling(s0);
}

TEST_F(MatTest, Basics) {
    EXPECT_EQ(sizeof(mat4), sizeof(double)*16);
}

TEST_F(MatTest, ComparisonOps) {
    mat4 m0;
    mat4 m1(2);

    EXPECT_TRUE(m0 == m0);
    EXPECT_TRUE(m0 != m1);
    EXPECT_FALSE(m0 != m0);
    EXPECT_FALSE(m0 == m1);
}

TEST_F(MatTest, Constructors) {
    mat4 m0;
    ASSERT_EQ(m0[0].x, 1);
    ASSERT_EQ(m0[0].y, 0);
    ASSERT_EQ(m0[0].z, 0);
    ASSERT_EQ(m0[0].w, 0);
    ASSERT_EQ(m0[1].x, 0);
    ASSERT_EQ(m0[1].y, 1);
    ASSERT_EQ(m0[1].z, 0);
    ASSERT_EQ(m0[1].w, 0);
    ASSERT_EQ(m0[2].x, 0);
    ASSERT_EQ(m0[2].y, 0);
    ASSERT_EQ(m0[2].z, 1);
    ASSERT_EQ(m0[2].w, 0);
    ASSERT_EQ(m0[3].x, 0);
    ASSERT_EQ(m0[3].y, 0);
    ASSERT_EQ(m0[3].z, 0);
    ASSERT_EQ(m0[3].w, 1);

    mat4 m1(2);
    mat4 m2(double4(2));
    mat4 m3(m2);

    EXPECT_EQ(m1, m2);
    EXPECT_EQ(m2, m3);
    EXPECT_EQ(m3, m1);
}

TEST_F(MatTest, ArithmeticOps) {
    mat4 m0;
    mat4 m1(2);
    mat4 m2(double4(2));

    m1 += m2;
    EXPECT_EQ(mat4(4), m1);

    m2 -= m1;
    EXPECT_EQ(mat4(-2), m2);

    m1 *= 2;
    EXPECT_EQ(mat4(8), m1);

    m1 /= 2;
    EXPECT_EQ(mat4(4), m1);

    m0 = -m0;
    EXPECT_EQ(mat4(-1), m0);
}

TEST_F(MatTest, UnaryOps) {
    const mat4 identity;
    mat4 m0;

    m0 = -m0;
    EXPECT_EQ(mat4(double4(-1, 0,  0,  0),
                   double4(0, -1,  0,  0),
                   double4(0,  0, -1,  0),
                   double4(0,  0,  0, -1)), m0);

    m0 = -m0;
    EXPECT_EQ(identity, m0);
}

TEST_F(MatTest, MiscOps) {
    const mat4 identity;
    mat4 m0;
    EXPECT_EQ(4, trace(m0));

    mat4 m1(double4(1, 2, 3, 4), double4(5, 6, 7, 8), double4(9, 10, 11, 12), double4(13, 14, 15, 16));
    mat4 m2(double4(1, 5, 9, 13), double4(2, 6, 10, 14), double4(3, 7, 11, 15), double4(4, 8, 12, 16));
    EXPECT_EQ(m1, transpose(m2));
    EXPECT_EQ(m2, transpose(m1));
    EXPECT_EQ(double4(1, 6, 11, 16), diag(m1));

    EXPECT_EQ(identity, inverse(identity));

    mat4 m3(double4(4, 3, 0, 0), double4(3, 2, 0, 0), double4(0, 0, 1, 0), double4(0, 0, 0, 1));
    mat4 m3i(inverse(m3));
    EXPECT_FLOAT_EQ(-2, m3i[0][0]);
    EXPECT_FLOAT_EQ(3,  m3i[0][1]);
    EXPECT_FLOAT_EQ(3,  m3i[1][0]);
    EXPECT_FLOAT_EQ(-4, m3i[1][1]);

    mat4 m3ii(inverse(m3i));
    EXPECT_FLOAT_EQ(m3[0][0], m3ii[0][0]);
    EXPECT_FLOAT_EQ(m3[0][1], m3ii[0][1]);
    EXPECT_FLOAT_EQ(m3[1][0], m3ii[1][0]);
    EXPECT_FLOAT_EQ(m3[1][1], m3ii[1][1]);

    EXPECT_EQ(m1, m1*identity);


    for (size_t c=0 ; c<4 ; c++) {
        for (size_t r=0 ; r<4 ; r++) {
            EXPECT_FLOAT_EQ(m1[c][r], m1(r, c));
        }
    }
}

TEST_F(MatTest, ElementAccess) {
    mat4 m(double4(1, 2, 3, 4), double4(5, 6, 7, 8), double4(9, 10, 11, 12), double4(13, 14, 15, 16));
    for (size_t c=0 ; c<4 ; c++) {
        for (size_t r=0 ; r<4 ; r++) {
            EXPECT_FLOAT_EQ(m[c][r], m(r, c));
        }
    }

    m(3,2) = 100;
    EXPECT_FLOAT_EQ(m[2][3], 100);
    EXPECT_FLOAT_EQ(m(3, 2), 100);
}

//------------------------------------------------------------------------------
// MAT 3
//------------------------------------------------------------------------------

class Mat3Test : public testing::Test {
protected:
};

TEST_F(Mat3Test, Basics) {
    EXPECT_EQ(sizeof(mat3), sizeof(double)*9);
}

TEST_F(Mat3Test, ComparisonOps) {
    mat3 m0;
    mat3 m1(2);

    EXPECT_TRUE(m0 == m0);
    EXPECT_TRUE(m0 != m1);
    EXPECT_FALSE(m0 != m0);
    EXPECT_FALSE(m0 == m1);
}

TEST_F(Mat3Test, Constructors) {
    mat3 m0;
    ASSERT_EQ(m0[0].x, 1);
    ASSERT_EQ(m0[0].y, 0);
    ASSERT_EQ(m0[0].z, 0);
    ASSERT_EQ(m0[1].x, 0);
    ASSERT_EQ(m0[1].y, 1);
    ASSERT_EQ(m0[1].z, 0);
    ASSERT_EQ(m0[2].x, 0);
    ASSERT_EQ(m0[2].y, 0);
    ASSERT_EQ(m0[2].z, 1);

    mat3 m1(2);
    mat3 m2(double3(2));
    mat3 m3(m2);

    EXPECT_EQ(m1, m2);
    EXPECT_EQ(m2, m3);
    EXPECT_EQ(m3, m1);
}

TEST_F(Mat3Test, ArithmeticOps) {
    mat3 m0;
    mat3 m1(2);
    mat3 m2(double3(2));

    m1 += m2;
    EXPECT_EQ(mat3(4), m1);

    m2 -= m1;
    EXPECT_EQ(mat3(-2), m2);

    m1 *= 2;
    EXPECT_EQ(mat3(8), m1);

    m1 /= 2;
    EXPECT_EQ(mat3(4), m1);

    m0 = -m0;
    EXPECT_EQ(mat3(-1), m0);
}

TEST_F(Mat3Test, UnaryOps) {
    const mat3 identity;
    mat3 m0;

    m0 = -m0;
    EXPECT_EQ(mat3(double3(-1, 0,  0),
                   double3(0, -1,  0),
                   double3(0,  0, -1)), m0);

    m0 = -m0;
    EXPECT_EQ(identity, m0);
}

TEST_F(Mat3Test, MiscOps) {
    const mat3 identity;
    mat3 m0;
    EXPECT_EQ(3, trace(m0));

    mat3 m1(double3(1, 2, 3), double3(4, 5, 6), double3(7, 8, 9));
    mat3 m2(double3(1, 4, 7), double3(2, 5, 8), double3(3, 6, 9));
    EXPECT_EQ(m1, transpose(m2));
    EXPECT_EQ(m2, transpose(m1));
    EXPECT_EQ(double3(1, 5, 9), diag(m1));

    EXPECT_EQ(identity, inverse(identity));

    mat3 m3(double3(4, 3, 0), double3(3, 2, 0), double3(0, 0, 1));
    mat3 m3i(inverse(m3));
    EXPECT_FLOAT_EQ(-2, m3i[0][0]);
    EXPECT_FLOAT_EQ(3,  m3i[0][1]);
    EXPECT_FLOAT_EQ(3,  m3i[1][0]);
    EXPECT_FLOAT_EQ(-4, m3i[1][1]);

    mat3 m3ii(inverse(m3i));
    EXPECT_FLOAT_EQ(m3[0][0], m3ii[0][0]);
    EXPECT_FLOAT_EQ(m3[0][1], m3ii[0][1]);
    EXPECT_FLOAT_EQ(m3[1][0], m3ii[1][0]);
    EXPECT_FLOAT_EQ(m3[1][1], m3ii[1][1]);

    EXPECT_EQ(m1, m1*identity);
}

//------------------------------------------------------------------------------
// MAT 2
//------------------------------------------------------------------------------

class Mat2Test : public testing::Test {
protected:
};

TEST_F(Mat2Test, Basics) {
    EXPECT_EQ(sizeof(mat2), sizeof(double)*4);
}

TEST_F(Mat2Test, ComparisonOps) {
    mat2 m0;
    mat2 m1(2);

    EXPECT_TRUE(m0 == m0);
    EXPECT_TRUE(m0 != m1);
    EXPECT_FALSE(m0 != m0);
    EXPECT_FALSE(m0 == m1);
}

TEST_F(Mat2Test, Constructors) {
    mat2 m0;
    ASSERT_EQ(m0[0].x, 1);
    ASSERT_EQ(m0[0].y, 0);
    ASSERT_EQ(m0[1].x, 0);
    ASSERT_EQ(m0[1].y, 1);

    mat2 m1(2);
    mat2 m2(double2(2));
    mat2 m3(m2);

    EXPECT_EQ(m1, m2);
    EXPECT_EQ(m2, m3);
    EXPECT_EQ(m3, m1);
}

TEST_F(Mat2Test, ArithmeticOps) {
    mat2 m0;
    mat2 m1(2);
    mat2 m2(double2(2));

    m1 += m2;
    EXPECT_EQ(mat2(4), m1);

    m2 -= m1;
    EXPECT_EQ(mat2(-2), m2);

    m1 *= 2;
    EXPECT_EQ(mat2(8), m1);

    m1 /= 2;
    EXPECT_EQ(mat2(4), m1);

    m0 = -m0;
    EXPECT_EQ(mat2(-1), m0);
}

TEST_F(Mat2Test, UnaryOps) {
    const mat2 identity;
    mat2 m0;

    m0 = -m0;
    EXPECT_EQ(mat2(double2(-1, 0),
                   double2(0, -1)), m0);

    m0 = -m0;
    EXPECT_EQ(identity, m0);
}

TEST_F(Mat2Test, MiscOps) {
    const mat2 identity;
    mat2 m0;
    EXPECT_EQ(2, trace(m0));

    mat2 m1(double2(1, 2), double2(3, 4));
    mat2 m2(double2(1, 3), double2(2, 4));
    EXPECT_EQ(m1, transpose(m2));
    EXPECT_EQ(m2, transpose(m1));
    EXPECT_EQ(double2(1, 4), diag(m1));

    EXPECT_EQ(identity, inverse(identity));

    EXPECT_EQ(m1, m1*identity);
}

//------------------------------------------------------------------------------
// MORE MATRIX TESTS
//------------------------------------------------------------------------------

template <typename T>
class MatTestT : public ::testing::Test {
public:
};

typedef ::testing::Types<float,double> TestMatrixValueTypes;

TYPED_TEST_SUITE(MatTestT, TestMatrixValueTypes);

#define TEST_MATRIX_INVERSE(MATRIX, EPSILON)                                \
{                                                                           \
    typedef decltype(MATRIX) MatrixType;                                    \
    MatrixType inv1 = inverse(MATRIX);                                      \
    MatrixType ident1 = MATRIX * inv1;                                      \
    MatrixType inv2 = transpose(cof(MATRIX))/det(MATRIX);                   \
    MatrixType ident2 = MATRIX * inv2;                                      \
    static const MatrixType IDENTITY;                                       \
    for (int row = 0; row < MatrixType::ROW_SIZE; ++row) {                  \
        for (int col = 0; col < MatrixType::COL_SIZE; ++col) {              \
            EXPECT_NEAR(ident1[row][col], IDENTITY[row][col], EPSILON);     \
            EXPECT_NEAR(ident2[row][col], IDENTITY[row][col], EPSILON);     \
        }                                                                   \
    }                                                                       \
}

TYPED_TEST(MatTestT, Inverse4) {
    typedef filament::math::details::TMat44<TypeParam> M44T;

    M44T m1(1,  0,  0,  0,
            0,  1,  0,  0,
            0,  0,  1,  0,
            0,  0,  0,  1);

    M44T m2(0,  -1,  0,  0,
            1,  0,  0,  0,
            0,  0,  1,  0,
            0,  0,  0,  1);

    M44T m3(1,  0,  0,  0,
            0,  2,  0,  0,
            0,  0,  0,  1,
            0,  0,  -1,  0);

    M44T m4(
            4.683281e-01, 1.251189e-02, -8.834660e-01, -4.726541e+00,
             -8.749647e-01,  1.456563e-01, -4.617587e-01, 3.044795e+00,
             1.229049e-01,  9.892561e-01, 7.916244e-02, -6.737138e+00,
             0.000000e+00, 0.000000e+00, 0.000000e+00, 1.000000e+00);

    M44T m5(
        4.683281e-01, 1.251189e-02, -8.834660e-01, -4.726541e+00,
        -8.749647e-01,  1.456563e-01, -4.617587e-01, 3.044795e+00,
        1.229049e-01,  9.892561e-01, 7.916244e-02, -6.737138e+00,
        1.000000e+00, 2.000000e+00, 3.000000e+00, 4.000000e+00);

    TEST_MATRIX_INVERSE(m1, 0);
    TEST_MATRIX_INVERSE(m2, 0);
    TEST_MATRIX_INVERSE(m3, 0);
    TEST_MATRIX_INVERSE(m4, 20.0 * std::numeric_limits<TypeParam>::epsilon());
    TEST_MATRIX_INVERSE(m5, 20.0 * std::numeric_limits<TypeParam>::epsilon());
}

//------------------------------------------------------------------------------
TYPED_TEST(MatTestT, Inverse3) {
    typedef filament::math::details::TMat33<TypeParam> M33T;

    M33T m1(1,  0,  0,
            0,  1,  0,
            0,  0,  1);

    M33T m2(0,  -1,  0,
            1,  0,  0,
            0,  0,  1);

    M33T m3(2,  0,  0,
            0,  0,  1,
            0,  -1,  0);

    M33T m4(
            4.683281e-01, 1.251189e-02, 0.000000e+00,
            -8.749647e-01, 1.456563e-01, 0.000000e+00,
            0.000000e+00, 0.000000e+00, 1.000000e+00);

    M33T m5(
            4.683281e-01, 1.251189e-02, -8.834660e-01,
           -8.749647e-01, 1.456563e-01, -4.617587e-01,
            1.229049e-01, 9.892561e-01, 7.916244e-02);

    TEST_MATRIX_INVERSE(m1, 0);
    TEST_MATRIX_INVERSE(m2, 0);
    TEST_MATRIX_INVERSE(m3, 0);
    TEST_MATRIX_INVERSE(m4, 20.0 * std::numeric_limits<TypeParam>::epsilon());
    TEST_MATRIX_INVERSE(m5, 20.0 * std::numeric_limits<TypeParam>::epsilon());
}

//------------------------------------------------------------------------------
TYPED_TEST(MatTestT, Inverse2) {
    typedef filament::math::details::TMat22<TypeParam> M22T;

    M22T m1(1,  0,
            0,  1);

    M22T m2(0,  -1,
            1,  0);

    M22T m3(
            4.683281e-01, 1.251189e-02,
            -8.749647e-01, 1.456563e-01);

    M22T m4(
            4.683281e-01, 1.251189e-02,
           -8.749647e-01, 1.456563e-01);

    TEST_MATRIX_INVERSE(m1, 0);
    TEST_MATRIX_INVERSE(m2, 0);
    TEST_MATRIX_INVERSE(m3, 20.0 * std::numeric_limits<TypeParam>::epsilon());
    TEST_MATRIX_INVERSE(m4, 20.0 * std::numeric_limits<TypeParam>::epsilon());
}


TYPED_TEST(MatTestT, NormalsNegativeScale) {
    typedef filament::math::details::TMat33<TypeParam> M33T;
    typedef filament::math::details::TVec3<TypeParam> V3T;

    M33T m(-1,  0,  0,
            0,  1,  0,
            0,  0,  1);

    V3T n = V3T(0, 0, 1);
    V3T n_prime = M33T::getTransformForNormals(m) * n;

    // The normal should be flipped for mirroring transformations (ie when the det < 0).
    //
    // This is intuitive using Grassmann algebra, or when visualizing the mirroring of the
    // tangent + bivector pair rather than the normal itself.
    //
    // Another way of thinking about this is in terms of polygon winding: since the winding is
    // flipped, we render its underside and thus need to flip the shading normal.
    //
    // The following shadertoy is illuminating: https://www.shadertoy.com/view/3s33zj.
    // The shadertoy is interesting for several reasons: (1) it uses the adjoint matrix for
    // transforming normals and (2) it negates the normal when scale is negative (look for
    // "isFlipped") and (3) it demonstrates that inverse-transpose computation is slow.

    ASSERT_LT(det(m), 0);
    EXPECT_VEC_EQ(n_prime, -n);
}

//------------------------------------------------------------------------------
// Test some translation stuff.
TYPED_TEST(MatTestT, Translation4) {
    typedef filament::math::details::TMat44<TypeParam> M44T;
    typedef filament::math::details::TVec4<TypeParam> V4T;
    typedef filament::math::details::TVec3<TypeParam> V3T;

    V3T translateBy(-7.3, 1.1, 14.4);
    V3T translation(translateBy[0], translateBy[1], translateBy[2]);
    M44T translation_matrix = M44T::translation(translation);

    V4T p1(9.9, 3.1, 41.1, 1.0);
    V4T p2(-18.0, 0.0, 1.77, 1.0);
    V4T p3(0, 0, 0, 1);
    V4T p4(-1000, -1000, 1000, 1.0);

    EXPECT_VEC_EQ((translation_matrix * p1).xyz, translateBy + p1.xyz);
    EXPECT_VEC_EQ((translation_matrix * p2).xyz, translateBy + p2.xyz);
    EXPECT_VEC_EQ((translation_matrix * p3).xyz, translateBy + p3.xyz);
    EXPECT_VEC_EQ((translation_matrix * p4).xyz, translateBy + p4.xyz);

    translation_matrix = M44T::translation(V3T{2.7});
    EXPECT_VEC_EQ((translation_matrix * p1).xyz, V3T{2.7} + p1.xyz);
}

//------------------------------------------------------------------------------
// Test some scale stuff.
TYPED_TEST(MatTestT, Scale4) {
    typedef filament::math::details::TMat44<TypeParam> M44T;
    typedef filament::math::details::TVec4<TypeParam> V4T;
    typedef filament::math::details::TVec3<TypeParam> V3T;

    V3T scaleBy(2.0, 3.0, 4.0);
    V3T scale(scaleBy[0], scaleBy[1], scaleBy[2]);
    M44T scale_matrix = M44T::scaling(scale);

    V4T p1(9.9, 3.1, 41.1, 1.0);
    V4T p2(-18.0, 0.0, 1.77, 1.0);
    V4T p3(0, 0, 0, 1);
    V4T p4(-1000, -1000, 1000, 1.0);

    EXPECT_VEC_EQ((scale_matrix * p1).xyz, scaleBy * p1.xyz);
    EXPECT_VEC_EQ((scale_matrix * p2).xyz, scaleBy * p2.xyz);
    EXPECT_VEC_EQ((scale_matrix * p3).xyz, scaleBy * p3.xyz);
    EXPECT_VEC_EQ((scale_matrix * p4).xyz, scaleBy * p4.xyz);

    scale_matrix = M44T::scaling(3.0);
    EXPECT_VEC_EQ((scale_matrix * p1).xyz, V3T{3.0} * p1.xyz);
}

//------------------------------------------------------------------------------
template <typename MATRIX>
static void verifyOrthonormal(const MATRIX& A) {
    typedef typename MATRIX::value_type T;

    static constexpr T value_eps = T(100) * std::numeric_limits<T>::epsilon();

    const MATRIX prod = A * transpose(A);
    for (int i = 0; i < MATRIX::NUM_COLS; ++i) {
        for (int j = 0; j < MATRIX::NUM_ROWS; ++j) {
            if (i == j) {
                ASSERT_NEAR(prod[i][j], T(1), value_eps);
            } else {
                ASSERT_NEAR(prod[i][j], T(0), value_eps);
            }
        }
    }
}

//------------------------------------------------------------------------------
// Test euler code.
TYPED_TEST(MatTestT, EulerZYX_44) {
    typedef filament::math::details::TMat44<TypeParam> M44T;

    std::default_random_engine generator(82828); // NOLINT
    std::uniform_real_distribution<TypeParam> distribution(-6.0 * 2.0*F_PI, 6.0 * 2.0*F_PI);
    auto rand_gen = std::bind(distribution, generator);

    for (size_t i = 0; i < 100; ++i) {
        M44T m = M44T::eulerZYX(rand_gen(), rand_gen(), rand_gen());
        verifyOrthonormal(m);
    }

    M44T m = M44T::eulerZYX(1, 2, 3);
    verifyOrthonormal(m);
}

//------------------------------------------------------------------------------
// Test euler code.
TYPED_TEST(MatTestT, EulerZYX_33) {

    typedef filament::math::details::TMat33<TypeParam> M33T;

    std::default_random_engine generator(112233); // NOLINT
    std::uniform_real_distribution<TypeParam> distribution(-6.0 * 2.0*F_PI, 6.0 * 2.0*F_PI);
    auto rand_gen = std::bind(distribution, generator);

    for (size_t i = 0; i < 100; ++i) {
        M33T m = M33T::eulerZYX(rand_gen(), rand_gen(), rand_gen());
        verifyOrthonormal(m);
    }

    M33T m = M33T::eulerZYX(1, 2, 3);
    verifyOrthonormal(m);
}

//------------------------------------------------------------------------------
// Test to quaternion with post translation.
TYPED_TEST(MatTestT, ToQuaternionPostTranslation) {

    typedef filament::math::details::TMat44<TypeParam> M44T;
    typedef filament::math::details::TVec3<TypeParam> V3T;
    typedef filament::math::details::TQuaternion<TypeParam> QuatT;

    std::default_random_engine generator(112233); // NOLINT
    std::uniform_real_distribution<TypeParam> distribution(-6.0 * 2.0*F_PI, 6.0 * 2.0*F_PI);
    auto rand_gen = std::bind(distribution, generator);

    for (size_t i = 0; i < 100; ++i) {
        M44T r = M44T::eulerZYX(rand_gen(), rand_gen(), rand_gen());
        M44T t = M44T::translation(V3T(rand_gen(), rand_gen(), rand_gen()));
        QuatT qr = r.toQuaternion();
        M44T tr = t * r;
        QuatT qtr = tr.toQuaternion();

        ASSERT_TYPE_EQ(qr.x, qtr.x);
        ASSERT_TYPE_EQ(qr.y, qtr.y);
        ASSERT_TYPE_EQ(qr.z, qtr.z);
        ASSERT_TYPE_EQ(qr.w, qtr.w);
    }

    M44T r = M44T::eulerZYX(1, 2, 3);
    M44T t = M44T::translation(V3T(20, -15, 2));
    QuatT qr = r.toQuaternion();
    M44T tr = t * r;
    QuatT qtr = tr.toQuaternion();

    ASSERT_TYPE_EQ(qr.x, qtr.x);
    ASSERT_TYPE_EQ(qr.y, qtr.y);
    ASSERT_TYPE_EQ(qr.z, qtr.z);
    ASSERT_TYPE_EQ(qr.w, qtr.w);
}

//------------------------------------------------------------------------------
// Test to quaternion with post translation.
TYPED_TEST(MatTestT, ToQuaternionPointTransformation33) {
    static constexpr TypeParam value_eps =
            TypeParam(1000) * std::numeric_limits<TypeParam>::epsilon();

    typedef filament::math::details::TMat33<TypeParam> M33T;
    typedef filament::math::details::TVec3<TypeParam> V3T;
    typedef filament::math::details::TQuaternion<TypeParam> QuatT;

    std::default_random_engine generator(112233); // NOLINT
    std::uniform_real_distribution<TypeParam> distribution(-100.0, 100.0);
    auto rand_gen = std::bind(distribution, generator);

    for (size_t i = 0; i < 100; ++i) {
        M33T r = M33T::eulerZYX(rand_gen(), rand_gen(), rand_gen());
        QuatT qr = r.toQuaternion();
        V3T p(rand_gen(), rand_gen(), rand_gen());

        V3T pr = r * p;
        V3T pq = qr * p;

        ASSERT_NEAR(pr.x, pq.x, value_eps);
        ASSERT_NEAR(pr.y, pq.y, value_eps);
        ASSERT_NEAR(pr.z, pq.z, value_eps);
    }
}

//------------------------------------------------------------------------------
// Test to quaternion with post translation.
TYPED_TEST(MatTestT, ToQuaternionPointTransformation44) {
    static constexpr TypeParam value_eps =
            TypeParam(1000) * std::numeric_limits<TypeParam>::epsilon();

    typedef filament::math::details::TMat44<TypeParam> M44T;
    typedef filament::math::details::TVec4<TypeParam> V4T;
    typedef filament::math::details::TVec3<TypeParam> V3T;
    typedef filament::math::details::TQuaternion<TypeParam> QuatT;

    std::default_random_engine generator(992626); // NOLINT
    std::uniform_real_distribution<TypeParam> distribution(-100.0, 100.0);
    auto rand_gen = std::bind(distribution, generator);

    for (size_t i = 0; i < 100; ++i) {
        M44T r = M44T::eulerZYX(rand_gen(), rand_gen(), rand_gen());
        QuatT qr = r.toQuaternion();
        V3T p(rand_gen(), rand_gen(), rand_gen());

        V4T pr = r * V4T(p.x, p.y, p.z, 1);
        pr.x /= pr.w;
        pr.y /= pr.w;
        pr.z /= pr.w;
        V3T pq = qr * p;

        ASSERT_NEAR(pr.x, pq.x, value_eps);
        ASSERT_NEAR(pr.y, pq.y, value_eps);
        ASSERT_NEAR(pr.z, pq.z, value_eps);
    }
}


TYPED_TEST(MatTestT, cofactor) {
    static constexpr TypeParam value_eps =
            TypeParam(1000) * std::numeric_limits<TypeParam>::epsilon();

    typedef filament::math::details::TMat33<TypeParam> M33T;

    std::default_random_engine generator(992626); // NOLINT
    std::uniform_real_distribution<TypeParam> distribution(-100.0, 100.0);
    auto rand_gen = std::bind(distribution, generator);

    for (size_t i = 0; i < 100; ++i) {
        M33T r = M33T::eulerZYX(rand_gen(), rand_gen(), rand_gen());
        M33T c0 = details::matrix::cofactor(r);
        M33T c1 = details::matrix::fastCofactor3(r);

        EXPECT_VEC_NEAR(c0[0], c1[0], value_eps);
        EXPECT_VEC_NEAR(c0[1], c1[1], value_eps);
        EXPECT_VEC_NEAR(c0[2], c1[2], value_eps);
    }
}



#undef TEST_MATRIX_INVERSE
