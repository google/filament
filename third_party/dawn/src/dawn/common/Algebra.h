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

#ifndef SRC_DAWN_COMMON_ALGEBRA_H_
#define SRC_DAWN_COMMON_ALGEBRA_H_

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "dawn/common/Assert.h"

// Use a nested namespace so that we can fill it with shorthand functions like `Mul` and shorthand
// type aliases.
namespace dawn::math {

// This namespace contains linear algebra (vectors / matrices) needed for some computations inside
// Dawn. It is not meant to be comprehensive and instead should grow only as needed.

template <typename Scalar>
constexpr size_t WGSLVectorAlignment(size_t size) {
    static_assert(sizeof(Scalar) == 4, "The only supported scalar types are u/i/f32 for now.");

    // vec2<f/i/u32> are all aligned to 8.
    if (size <= 2) {
        return 2 * sizeof(Scalar);
    }

    // vec3/4<f/i/u32> are all aligned to 16.
    return 4 * sizeof(Scalar);
}

// A vector class with template arguments for the type of scalar, number of dimensions. Note that
// the alignment matches WGSL. Use the type aliases defined below (such as Vec3f) instead when
// possible.
template <size_t Size, typename Scalar>
class alignas(WGSLVectorAlignment<Scalar>(Size)) Vector {
  private:
    using Self = Vector<Size, Scalar>;
    std::array<Scalar, Size> data = {};

  public:
    // The default constructor zero-initializes.
    constexpr Vector() { data.fill(Scalar()); }

    // Constructors to initialize from Scalars, either together as an array or as separate arguments
    // depending on the vector size.
    constexpr explicit Vector(const std::array<Scalar, Size>& data) : data(data) {}
    constexpr Vector(const Scalar& s1, const Scalar& s2)
        requires(Size == 2)
        : data{s1, s2} {}
    constexpr Vector(const Scalar& s1, const Scalar& s2, const Scalar& s3)
        requires(Size == 3)
        : data{s1, s2, s3} {}
    constexpr Vector(const Scalar& s1, const Scalar& s2, const Scalar& s3, const Scalar& s4)
        requires(Size == 4)
        : data{s1, s2, s3, s4} {}

    // Constructor to explicitly cast between Vector with different Scalar types.
    template <typename OtherScalar>
    constexpr explicit Vector(const Vector<Size, OtherScalar>& other) {
        for (size_t i = 0; i < Size; i++) {
            data[i] = Scalar(other[i]);
        }
    }

    // Operator overloads. Note that arithmetic operators are only by-component. More complex
    // arithmetic operators are implemented as freestanding functions (like Mul(Matrix, Vector)).
    constexpr Scalar& operator[](size_t i) { return data[i]; }
    constexpr const Scalar& operator[](size_t i) const { return data[i]; }

    constexpr bool operator==(const Self& other) const = default;

    constexpr Self operator+(const Self& other) const {
        Self result;
        for (size_t i = 0; i < Size; i++) {
            result[i] = data[i] + other[i];
        }
        return result;
    }
    constexpr Self& operator+=(const Self& other) {
        *this = *this + other;
        return *this;
    }

    constexpr Self operator-(const Self& other) const {
        Self result;
        for (size_t i = 0; i < Size; i++) {
            result[i] = data[i] - other[i];
        }
        return result;
    }
    constexpr Self& operator-=(const Self& other) {
        *this = *this - other;
        return *this;
    }

    constexpr Self operator*(const Self& other) const {
        Self result;
        for (size_t i = 0; i < Size; i++) {
            result[i] = data[i] * other[i];
        }
        return result;
    }
    constexpr Self& operator*=(const Self& other) {
        *this = *this * other;
        return *this;
    }

    constexpr Self operator/(const Self& other) const {
        Self result;
        for (size_t i = 0; i < Size; i++) {
            DAWN_ASSERT(other[i] != Scalar(0));
            result[i] = data[i] / other[i];
        }
        return result;
    }
    constexpr Self& operator/=(const Self& other) {
        *this = *this / other;
        return *this;
    }

    constexpr Self operator*(const Scalar& s) const {
        Self result;
        for (size_t i = 0; i < Size; i++) {
            result[i] = data[i] * s;
        }
        return result;
    }
    constexpr Self& operator*=(const Scalar& s) {
        *this = *this * s;
        return *this;
    }

    constexpr Self operator/(const Scalar& s) const {
        Self result;
        for (size_t i = 0; i < Size; i++) {
            result[i] = data[i] / s;
        }
        return result;
    }
    constexpr Self& operator/=(const Scalar& other) {
        *this = *this / other;
        return *this;
    }
};

template <size_t Size, typename Scalar>
constexpr Vector<Size, Scalar> operator*(const Scalar& a, const Vector<Size, Scalar>& b) {
    return b * a;
}

// A column-major matrix class with with template arguments for the type of scalar and number of
// dimensions. Note that the alignment matches WGSL. Use the type aliases defined below (such as
// Mat4x4f) instead when possible.
template <size_t Cols, size_t Rows, typename Scalar>
class Matrix {
  public:
    static inline constexpr size_t ColumnCount = Cols;
    static inline constexpr size_t RowCount = Cols;
    using Column = Vector<Rows, Scalar>;

  private:
    using Self = Matrix<Cols, Rows, Scalar>;
    std::array<Column, Cols> data = {};

  public:
    // The default constructor zero-initializes.
    constexpr Matrix() { data.fill(Column()); }

    // Constructors to initialize from Columns, either together as an array or as separate arguments
    // depending on the matrix size.
    constexpr explicit Matrix(const std::array<Column, Cols>& data) : data(data) {}
    constexpr Matrix(const Column& c1, const Column& c2)
        requires(Cols == 2)
        : data{c1, c2} {}
    constexpr Matrix(const Column& c1, const Column& c2, const Column& c3)
        requires(Cols == 3)
        : data{c1, c2, c3} {}
    constexpr Matrix(const Column& c1, const Column& c2, const Column& c3, const Column& c4)
        requires(Cols == 4)
        : data{c1, c2, c3, c4} {}

    constexpr Scalar Determinant() const
        requires std::is_floating_point_v<Scalar> && (Cols == Rows) && (Rows < 4);
    constexpr Self Inverse() const
        requires std::is_floating_point_v<Scalar> && (Cols == Rows) && (Rows < 4);

    // Returns the identity matrix of that dimensionality.
    constexpr static Self Identity()
        requires(Cols == Rows)
    {
        std::array<Scalar, Rows> scales;
        scales.fill(1);
        return Scale(Column{scales});
    }

    // Returns a scale matrix based on the scale vector.
    constexpr static Self Scale(Column scale)
        requires(Cols == Rows)
    {
        Self mat;
        for (size_t i = 0; i < Rows; i++) {
            mat[i][i] = scale[i];
        }
        return mat;
    }

    // Returns a translation matrix when working in a homogeneous space.
    constexpr static Self Translation(Vector<Rows - 1, Scalar> translation)
        requires(Cols == Rows)
    {
        Self result = Identity();
        Column& translationColumn = result[Cols - 1];
        for (size_t row = 0; row < Rows - 1; row++) {
            translationColumn[row] = translation[row];
        }
        return result;
    }

    // Returns a translation matrix when working in a homogeneous space.
    constexpr static Self ScaleHomogeneous(Vector<Rows - 1, Scalar> factorsIn)
        requires(Cols == Rows)
    {
        Column factors;
        for (size_t row = 0; row < Rows - 1; row++) {
            factors[row] = factorsIn[row];
        }
        factors[Rows - 1] = Scalar(1);
        return Scale(factors);
    }

    // Conversion between Matrices with different dimensions. It either crops the data, or expands
    // it with zeroes.
    template <size_t OtherCols, size_t OtherRows>
    constexpr static Self CropOrExpandFrom(const Matrix<OtherCols, OtherRows, Scalar>& other) {
        Self result;
        for (size_t col = 0; col < std::min(Cols, OtherCols); col++) {
            for (size_t row = 0; row < std::min(Rows, OtherRows); row++) {
                result[col][row] = other[col][row];
            }
        }
        return result;
    }

    // Operator overloads. Note that arithmetic operators are only by-component. More complex
    // arithmetic operators are implemented as freestanding functions (like Mul(Matrix, Vector)).
    constexpr Column& operator[](size_t i) { return data[i]; }
    constexpr const Column& operator[](size_t i) const { return data[i]; }

    constexpr bool operator==(const Self& other) const = default;

    constexpr Self operator/(const Scalar& s) const {
        Self result;
        for (size_t i = 0; i < Cols; i++) {
            result[i] = data[i] / s;
        }
        return result;
    }
    constexpr Self& operator/=(const Scalar& other) {
        *this = *this / other;
        return *this;
    }
};

// Returns A * V with A a matrix and V a compatible vector.
template <size_t M, size_t N, typename Scalar>
constexpr Vector<N, Scalar> Mul(const Matrix<M, N, Scalar>& A, const Vector<M, Scalar>& V) {
    Vector<N, Scalar> result;
    for (size_t i = 0; i < M; i++) {
        result += A[i] * V[i];
    }
    return result;
}

// Returns A * B with A and B matrices compatible to multiply in that order.
template <size_t M, size_t N, size_t K, typename Scalar>
constexpr Matrix<M, N, Scalar> Mul(const Matrix<K, N, Scalar>& A, const Matrix<M, K, Scalar>& B) {
    Matrix<M, N, Scalar> result;
    for (size_t i = 0; i < M; i++) {
        result[i] = Mul(A, B[i]);
    }
    return result;
}

// Returns the element-wise maximum of two vectors.
template <size_t Size, typename Scalar>
constexpr Vector<Size, Scalar> Max(const Vector<Size, Scalar>& v1, const Vector<Size, Scalar>& v2) {
    Vector<Size, Scalar> result;
    for (size_t i = 0; i < Size; i++) {
        result[i] = std::max(v1[i], v2[i]);
    }
    return result;
}

template <size_t Cols, size_t Rows, typename Scalar>
constexpr Scalar Matrix<Cols, Rows, Scalar>::Determinant() const
    requires std::is_floating_point_v<Scalar> && (Cols == Rows) && (Rows < 4)
{
    if constexpr (Cols == 2) {
        return data[0][0] * data[1][1] - data[0][1] * data[1][0];
    } else if constexpr (Cols == 3) {
        return data[0][0] * (data[2][2] * data[1][1] - data[1][2] * data[2][1]) +  //
               data[0][1] * (data[1][2] * data[2][0] - data[2][2] * data[1][0]) +  //
               data[0][2] * (data[2][1] * data[1][0] - data[1][1] * data[2][0]);
    }
}

template <size_t Cols, size_t Rows, typename Scalar>
constexpr Matrix<Cols, Rows, Scalar> Matrix<Cols, Rows, Scalar>::Inverse() const
    requires std::is_floating_point_v<Scalar> && (Cols == Rows) && (Rows < 4)
{
    Scalar det = Determinant();

    if constexpr (Cols == 2) {
        return Self{{data[1][1], -data[0][1]}, {-data[1][0], data[0][0]}} / det;
    } else if constexpr (Cols == 3) {
        return Self{{
                        data[2][2] * data[1][1] - data[1][2] * data[2][1],
                        -data[2][2] * data[0][1] + data[0][2] * data[2][1],
                        data[1][2] * data[0][1] - data[0][2] * data[1][1],
                    },
                    {
                        -data[2][2] * data[1][0] + data[1][2] * data[2][0],
                        data[2][2] * data[0][0] - data[0][2] * data[2][0],
                        -data[1][2] * data[0][0] + data[0][2] * data[1][0],
                    },
                    {
                        data[2][1] * data[1][0] - data[1][1] * data[2][0],
                        -data[2][1] * data[0][0] + data[0][1] * data[2][0],
                        data[1][1] * data[0][0] - data[0][1] * data[1][0],
                    }} /
               det;
    }
}

// Shorthand type aliases that match WGSL types (in name, layout and alignment).
using Vec2f = Vector<2, float>;
using Vec3f = Vector<3, float>;
using Vec4f = Vector<4, float>;
using Vec2u = Vector<2, uint32_t>;
using Vec3u = Vector<3, uint32_t>;
using Vec4u = Vector<4, uint32_t>;
using Vec2i = Vector<2, int32_t>;
using Vec3i = Vector<3, int32_t>;
using Vec4i = Vector<4, int32_t>;
using Mat2x2f = Matrix<2, 2, float>;
using Mat3x2f = Matrix<3, 2, float>;
using Mat4x2f = Matrix<4, 2, float>;
using Mat2x3f = Matrix<2, 3, float>;
using Mat3x3f = Matrix<3, 3, float>;
using Mat4x3f = Matrix<4, 3, float>;
using Mat2x4f = Matrix<2, 4, float>;
using Mat3x4f = Matrix<3, 4, float>;
using Mat4x4f = Matrix<4, 4, float>;

}  // namespace dawn::math

#endif  // SRC_DAWN_COMMON_ALGEBRA_H_
