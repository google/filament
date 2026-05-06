// Copyright 2026 The Dawn & Tint Authors
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

#include <gtest/gtest.h>

#include "dawn/common/Algebra.h"

namespace dawn::math {
namespace {

// Test that the algebra types have the same size and alignment as in WGSL.
// See https://gpuweb.github.io/gpuweb/wgsl/#alignment-and-size
static_assert(sizeof(Vec2f) == 8);
static_assert(sizeof(Vec3f) == 16);  // 12 in WGSL but in C++ sizeof is rounded to the alignment.
static_assert(sizeof(Vec4f) == 16);
static_assert(sizeof(Vec2u) == 8);
static_assert(sizeof(Vec3u) == 16);  // Same
static_assert(sizeof(Vec4u) == 16);
static_assert(sizeof(Vec2i) == 8);
static_assert(sizeof(Vec3i) == 16);  // Same
static_assert(sizeof(Vec4i) == 16);
static_assert(sizeof(Mat2x2f) == 16);
static_assert(sizeof(Mat3x2f) == 24);
static_assert(sizeof(Mat4x2f) == 32);
static_assert(sizeof(Mat2x3f) == 32);
static_assert(sizeof(Mat3x3f) == 48);
static_assert(sizeof(Mat4x3f) == 64);
static_assert(sizeof(Mat2x4f) == 32);
static_assert(sizeof(Mat3x4f) == 48);
static_assert(sizeof(Mat4x4f) == 64);

static_assert(alignof(Vec2f) == 8);
static_assert(alignof(Vec3f) == 16);
static_assert(alignof(Vec4f) == 16);
static_assert(alignof(Vec2u) == 8);
static_assert(alignof(Vec3u) == 16);
static_assert(alignof(Vec4u) == 16);
static_assert(alignof(Vec2i) == 8);
static_assert(alignof(Vec3i) == 16);
static_assert(alignof(Vec4i) == 16);
static_assert(alignof(Mat2x2f) == 8);
static_assert(alignof(Mat3x2f) == 8);
static_assert(alignof(Mat4x2f) == 8);
static_assert(alignof(Mat2x3f) == 16);
static_assert(alignof(Mat3x3f) == 16);
static_assert(alignof(Mat4x3f) == 16);
static_assert(alignof(Mat2x4f) == 16);
static_assert(alignof(Mat3x4f) == 16);
static_assert(alignof(Mat4x4f) == 16);

// Check that the vector constructor set components as expected (and implicitly test the indexing
// operators)
TEST(Algebra, VectorConstructorAndIndexing) {
    // Check the Vector constructor from an array.
    {
        auto a = Vec2u{{1, 2}};
        auto b = Vec3i{{3, 4, 5}};
        auto c = Vec4f{{6, 7, 8, 9}};

        EXPECT_EQ(a[0], 1u);
        EXPECT_EQ(a[1], 2u);

        EXPECT_EQ(b[0], 3);
        EXPECT_EQ(b[1], 4);
        EXPECT_EQ(b[2], 5);

        EXPECT_EQ(c[0], 6.0f);
        EXPECT_EQ(c[1], 7.0f);
        EXPECT_EQ(c[2], 8.0f);
        EXPECT_EQ(c[3], 9.0f);
    }
    // Check the Vector constructor from an individual scalars.
    {
        auto a = Vec2u{1, 2};
        auto b = Vec3i{3, 4, 5};
        auto c = Vec4f{6, 7, 8, 9};

        EXPECT_EQ(a[0], 1u);
        EXPECT_EQ(a[1], 2u);

        EXPECT_EQ(b[0], 3);
        EXPECT_EQ(b[1], 4);
        EXPECT_EQ(b[2], 5);

        EXPECT_EQ(c[0], 6.0f);
        EXPECT_EQ(c[1], 7.0f);
        EXPECT_EQ(c[2], 8.0f);
        EXPECT_EQ(c[3], 9.0f);

        // Test setting with the indexing operator.
        a[1] = 0;
        EXPECT_EQ(a, Vec2u(1, 0));
    }
}

// Test casting between vectors of different scalar types.
TEST(Algebra, VectorConstructorFromVectorWithDifferentScalarType) {
    EXPECT_EQ(Vec2u(Vec2f(3.0, 4.0)), Vec2u(3, 4));
    EXPECT_EQ(Vec4f(Vec4i(-1, 3, 4, -4)), Vec4f(-1.0, 3.0, 4.0, -4.0));
}

// Test the vector equality operator
TEST(Algebra, VectorEquality) {
    EXPECT_EQ(Vec4u(1, 2, 3, 4), Vec4u(1, 2, 3, 4));

    EXPECT_NE(Vec4u(1, 2, 3, 4), Vec4u(0, 2, 3, 4));
    EXPECT_NE(Vec4u(1, 2, 3, 4), Vec4u(1, 0, 3, 4));
    EXPECT_NE(Vec4u(1, 2, 3, 4), Vec4u(1, 2, 0, 4));
    EXPECT_NE(Vec4u(1, 2, 3, 4), Vec4u(1, 2, 3, 0));
}

// Test the vector-vector addition.
TEST(Algebra, VectorVectorAdd) {
    EXPECT_EQ(Vec3f(1, 2, 3) + Vec3f(10, 11, 12), Vec3f(11, 13, 15));

    Vec3f accumulator = Vec3f(1, 2, 3);
    accumulator += Vec3f(10, 11, 12);
    EXPECT_EQ(accumulator, Vec3f(11, 13, 15));
}

// Test the vector-vector subtraction.
TEST(Algebra, VectorVectorSub) {
    EXPECT_EQ(Vec3f(10, 11, 12) - Vec3f(1, 2, 3), Vec3f(9, 9, 9));

    Vec3f accumulator = Vec3f(1, 2, 3);
    accumulator -= Vec3f(10, 11, 12);
    EXPECT_EQ(accumulator, Vec3f(-9, -9, -9));
}

// Test the vector-vector multiplication.
TEST(Algebra, VectorVectorMul) {
    EXPECT_EQ(Vec3f(1, 2, 3) * Vec3f(-4, 3, 5), Vec3f(-4, 6, 15));

    Vec3f accumulator = Vec3f(1, 2, 3);
    accumulator *= Vec3f(-4, 3, 5);
    EXPECT_EQ(accumulator, Vec3f(-4, 6, 15));
}

// Test the vector-vector division.
TEST(Algebra, VectorVectorDiv) {
    EXPECT_EQ(Vec3f(8, 6, 15) / Vec3f(2, 3, 5), Vec3f(4, 2, 3));

    Vec3f accumulator = Vec3f(8, 6, 15);
    accumulator /= Vec3f(2, 3, 5);
    EXPECT_EQ(accumulator, Vec3f(4, 2, 3));
}

// Test the vector-scalar multiplication.
TEST(Algebra, VectorScalarMul) {
    EXPECT_EQ(Vec3f(1, 2, 3) * 3.0f, Vec3f(3, 6, 9));
    EXPECT_EQ(3.0f * Vec3f(1, 2, 3), Vec3f(3, 6, 9));

    Vec3f a = Vec3f(1, 2, 3);
    a *= 3.0f;
    EXPECT_EQ(a, Vec3f(3, 6, 9));
}

// Test the vector-scalar division.
TEST(Algebra, VectorScalarDiv) {
    EXPECT_EQ(Vec3f(3, 6, 9) / 3.0f, Vec3f(1, 2, 3));

    Vec3f a = Vec3f(3, 6, 9);
    a /= 3.0f;
    EXPECT_EQ(a, Vec3f(1, 2, 3));
}

// Check that the vector constructor set columns as expected (and implicitly test the indexing
// operators)
TEST(Algebra, MatrixConstructorAndIndexing) {
    // Check the Matrix constructor from an array.
    {
        auto a = Mat2x3f{{Vec3f(1, 2, 3), Vec3f(4, 5, 6)}};
        EXPECT_EQ(a[0], Vec3f(1, 2, 3));
        EXPECT_EQ(a[1], Vec3f(4, 5, 6));
    }
    // Check the Matrix constructor from a individual columns.
    {
        auto a = Mat2x3f{Vec3f(1, 2, 3), Vec3f(4, 5, 6)};
        EXPECT_EQ(a[0], Vec3f(1, 2, 3));
        EXPECT_EQ(a[1], Vec3f(4, 5, 6));

        // Test setting with the indexing operator.
        a[1] = Vec3f(0, 0, 0);
        EXPECT_EQ(a, Mat2x3f({{1, 2, 3}, {0, 0, 0}}));
    }
}

// Test the vector equality operator
TEST(Algebra, MatrixEquality) {
    EXPECT_EQ(Mat3x2f({1, 2}, {3, 4}, {5, 6}), Mat3x2f({1, 2}, {3, 4}, {5, 6}));

    EXPECT_NE(Mat3x2f({1, 2}, {3, 4}, {5, 6}), Mat3x2f({0, 2}, {3, 4}, {5, 6}));
    EXPECT_NE(Mat3x2f({1, 2}, {3, 4}, {5, 6}), Mat3x2f({1, 0}, {3, 4}, {5, 6}));
    EXPECT_NE(Mat3x2f({1, 2}, {3, 4}, {5, 6}), Mat3x2f({1, 2}, {0, 4}, {5, 6}));
    EXPECT_NE(Mat3x2f({1, 2}, {3, 4}, {5, 6}), Mat3x2f({1, 2}, {3, 0}, {5, 6}));
    EXPECT_NE(Mat3x2f({1, 2}, {3, 4}, {5, 6}), Mat3x2f({1, 2}, {3, 4}, {0, 6}));
    EXPECT_NE(Mat3x2f({1, 2}, {3, 4}, {5, 6}), Mat3x2f({1, 2}, {3, 4}, {5, 0}));
}

// Test that identity matrices are diagonals filled with 1s
TEST(Algebra, MatrixIdentity) {
    EXPECT_EQ(Mat2x2f::Identity(), Mat2x2f({1, 0}, {0, 1}));
    EXPECT_EQ(Mat3x3f::Identity(), Mat3x3f({1, 0, 0}, {0, 1, 0}, {0, 0, 1}));
    EXPECT_EQ(Mat4x4f::Identity(), Mat4x4f({1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}));
}

// Test the creation of scale matrices
TEST(Algebra, MatrixScale) {
    EXPECT_EQ(Mat2x2f::Scale({1, 2}), Mat2x2f({1, 0}, {0, 2}));
    EXPECT_EQ(Mat3x3f::Scale({3, 4, 5}), Mat3x3f({3, 0, 0}, {0, 4, 0}, {0, 0, 5}));
    EXPECT_EQ(Mat4x4f::Scale({6, 7, 8, 9}),
              Mat4x4f({6, 0, 0, 0}, {0, 7, 0, 0}, {0, 0, 8, 0}, {0, 0, 0, 9}));
}

// Test the creation of scale matrices
TEST(Algebra, MatrixTranslation) {
    EXPECT_EQ(Mat3x3f::Translation({3, 4}), Mat3x3f({1, 0, 0}, {0, 1, 0}, {3, 4, 1}));
    EXPECT_EQ(Mat4x4f::Translation({6, 7, 8}),
              Mat4x4f({1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {6, 7, 8, 1}));
}

// Test the creation of homogeneous space scale matrices
TEST(Algebra, MatrixScaleHomogeneous) {
    EXPECT_EQ(Mat3x3f::ScaleHomogeneous({3, 4}), Mat3x3f({3, 0, 0}, {0, 4, 0}, {0, 0, 1}));
    EXPECT_EQ(Mat4x4f::ScaleHomogeneous({6, 7, 8}),
              Mat4x4f({6, 0, 0, 0}, {0, 7, 0, 0}, {0, 0, 8, 0}, {0, 0, 0, 1}));
}

// Test casting between matrices of different dimension (it crops or fills with zeroes).
TEST(Algebra, MatrixCropOrExpandFrom) {
    EXPECT_EQ(Mat4x2f::CropOrExpandFrom(Mat2x4f({1, 2, 3, 4}, {5, 6, 7, 8})),
              Mat4x2f({1, 2}, {5, 6}, {0, 0}, {0, 0}));
}

// Test computation of the determinant.
TEST(Algebra, MatrixDeterminant) {
    // Simple test vectors.
    EXPECT_EQ(Mat2x2f({2, 3}, {5, 7}).Determinant(), -1.0);
    EXPECT_EQ(Mat3x3f({2, 3, 5}, {7, 11, 13}, {17, 19, 23}).Determinant(), -78.0);

    // Determinant of non-invertible matrices is 0.
    EXPECT_EQ(Mat2x2f({1, 2}, {2, 4}).Determinant(), 0.0);
    EXPECT_EQ(Mat3x3f({1, 2, 3}, {1, 3, 5}, {2, 5, 8}).Determinant(), 0.0);
}

// Test computation of the inverse.
TEST(Algebra, MatrixInverse) {
    auto CheckAlmostIdentity = [](auto m) {
        using Matrix = decltype(m);
        auto identity = Matrix::Identity();
        for (size_t x = 0; x < Matrix::ColumnCount; x++) {
            for (size_t y = 0; y < Matrix::RowCount; y++) {
                EXPECT_NEAR(m[x][y], identity[x][y], 0.0001);
            }
        }
    };

    // 2x2
    {
        // Test with numbers that are not linked with integer relations, just in case.
        auto testMatrix = Mat2x2f({2.1, 3.1}, {5.1, 7.1});
        auto invert = testMatrix.Inverse();
        CheckAlmostIdentity(Mul(testMatrix, invert));
        CheckAlmostIdentity(Mul(invert, testMatrix));
    }
    // 3x3
    {
        // Test with numbers that are not linked with integer relations, just in case.
        auto testMatrix = Mat3x3f({2.1, 3.1, 5.1}, {7.1, 11.1, 13.1}, {17.1, 19.1, 23.1});
        auto invert = testMatrix.Inverse();
        CheckAlmostIdentity(Mul(testMatrix, invert));
        CheckAlmostIdentity(Mul(invert, testMatrix));
    }
}

// Test the vector-scalar division.
TEST(Algebra, MatrixScalarDiv) {
    EXPECT_EQ(Mat2x2f({3, 6}, {9, 12}) / 3.0f, Mat2x2f({1, 2}, {3, 4}));

    Mat2x2f m = Mat2x2f({3, 6}, {9, 12});
    m /= 3.0f;
    EXPECT_EQ(m, Mat2x2f({1, 2}, {3, 4}));
}

// Test matrix / vector multiplication.
TEST(Algebra, MatrixVectorMul) {
    auto m = Mat3x4f({1, 2, 3, 4}, {-1, -2, -3, -4}, {10, 11, 12, 13});

    // The matrix transforms basis vectors to column vectors.
    EXPECT_EQ(Mul(m, Vec3f(1, 0, 0)), m[0]);
    EXPECT_EQ(Mul(m, Vec3f(0, 1, 0)), m[1]);
    EXPECT_EQ(Mul(m, Vec3f(0, 0, 1)), m[2]);

    EXPECT_EQ(Mul(m, Vec3f(1, 2, 3)), m[0] * 1 + m[1] * 2 + m[2] * 3);
}

// Test matrix / matrix multiplication.
TEST(Algebra, MatrixMatrixMul) {
    // Take two matrices that can be multiplied together, containing arbitrary values that are not
    // special wrt to matrix multiplication.
    auto A = Mat4x3f({10, 11, 12}, {4, 3, 2}, {3, 5, 7}, {-2, -5, -8});
    auto B = Mat3x4f({1, 2, 3, 4}, {-1, -2, -3, -4}, {10, 11, 12, 13});

    // The resulting matrix is defined by the property that multiplying vectors with it is the same
    // as multiplying the vector with B and then A. Check each basis vector.
    auto R = Mul(A, B);
    EXPECT_EQ(Mul(R, Vec3f(1, 0, 0)), Mul(A, Mul(B, Vec3f(1, 0, 0))));
    EXPECT_EQ(Mul(R, Vec3f(0, 1, 0)), Mul(A, Mul(B, Vec3f(0, 1, 0))));
    EXPECT_EQ(Mul(R, Vec3f(0, 0, 1)), Mul(A, Mul(B, Vec3f(0, 0, 1))));
}

// Test vector / vector max.
TEST(Algebra, VectorVectorMax) {
    EXPECT_EQ(Max(Vec2f(2, 7), Vec2f(6, 3)), Vec2f(6, 7));
    EXPECT_EQ(Max(Vec4u(1, 2, 3, 4), Vec4u(4, 3, 2, 1)), Vec4u(4, 3, 3, 4));
}

}  // anonymous namespace
}  // namespace dawn::math
