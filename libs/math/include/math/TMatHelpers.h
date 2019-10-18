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

#ifndef MATH_TMATHELPERS_H_
#define MATH_TMATHELPERS_H_

#include <math/compiler.h>
#include <math/quat.h>
#include <math/TVecHelpers.h>

#include <algorithm>        // for std::swap
#include <cmath>            // for std:: namespace
#include <iostream>         // for operator<<
#include <iomanip>          // for std::setw

#include <math.h>
#include <stdint.h>
#include <sys/types.h>

namespace filament {
namespace math {
namespace details {
// -------------------------------------------------------------------------------------

/*
 * No user serviceable parts here.
 *
 * Don't use this file directly, instead include math/mat*.h
 */


/*
 * Matrix utilities
 */

namespace matrix {

/*
 * Matrix inversion
 */
template<typename MATRIX>
constexpr MATRIX MATH_PURE gaussJordanInverse(MATRIX src) {
    typedef typename MATRIX::value_type T;
    constexpr unsigned int N = MATRIX::NUM_ROWS;
    MATRIX inverted;

    for (size_t i = 0; i < N; ++i) {
        // look for largest element in i'th column
        size_t swap = i;
        T t = src[i][i] < 0 ? -src[i][i] : src[i][i];
        for (size_t j = i + 1; j < N; ++j) {
            const T t2 = src[j][i] < 0 ? -src[j][i] : src[j][i];
            if (t2 > t) {
                swap = j;
                t = t2;
            }
        }

        if (swap != i) {
            // swap columns.
            std::swap(src[i], src[swap]);
            std::swap(inverted[i], inverted[swap]);
        }

        const T denom(src[i][i]);
        for (size_t k = 0; k < N; ++k) {
            src[i][k] /= denom;
            inverted[i][k] /= denom;
        }

        // Factor out the lower triangle
        for (size_t j = 0; j < N; ++j) {
            if (j != i) {
                const T t = src[j][i];
                for (size_t k = 0; k < N; ++k) {
                    src[j][k] -= src[i][k] * t;
                    inverted[j][k] -= inverted[i][k] * t;
                }
            }
        }
    }

    return inverted;
}

//------------------------------------------------------------------------------
// 2x2 matrix inverse is easy.
template<typename MATRIX>
constexpr MATRIX MATH_PURE fastInverse2(const MATRIX& x) {
    typedef typename MATRIX::value_type T;

    // Assuming the input matrix is:
    // | a b |
    // | c d |
    //
    // The analytic inverse is
    // | d -b |
    // | -c a | / (a d - b c)
    //
    // Importantly, our matrices are column-major!

    MATRIX inverted{};

    const T a = x[0][0];
    const T c = x[0][1];
    const T b = x[1][0];
    const T d = x[1][1];

    const T det((a * d) - (b * c));
    inverted[0][0] =  d / det;
    inverted[0][1] = -c / det;
    inverted[1][0] = -b / det;
    inverted[1][1] =  a / det;
    return inverted;
}

//------------------------------------------------------------------------------
// From the Wikipedia article on matrix inversion's section on fast 3x3
// matrix inversion:
// http://en.wikipedia.org/wiki/Invertible_matrix#Inversion_of_3.C3.973_matrices
template<typename MATRIX>
constexpr MATRIX MATH_PURE fastInverse3(const MATRIX& x) {
    typedef typename MATRIX::value_type T;

    // Assuming the input matrix is:
    // | a b c |
    // | d e f |
    // | g h i |
    //
    // The analytic inverse is
    // | A B C |^T
    // | D E F |
    // | G H I | / determinant
    //
    // Which is
    // | A D G |
    // | B E H |
    // | C F I | / determinant
    //
    // Where:
    // A = (ei - fh), B = (fg - di), C = (dh - eg)
    // D = (ch - bi), E = (ai - cg), F = (bg - ah)
    // G = (bf - ce), H = (cd - af), I = (ae - bd)
    //
    // and the determinant is a*A + b*B + c*C (The rule of Sarrus)
    //
    // Importantly, our matrices are column-major!

    MATRIX inverted{};

    const T a = x[0][0];
    const T b = x[1][0];
    const T c = x[2][0];
    const T d = x[0][1];
    const T e = x[1][1];
    const T f = x[2][1];
    const T g = x[0][2];
    const T h = x[1][2];
    const T i = x[2][2];

    // Do the full analytic inverse
    const T A = e * i - f * h;
    const T B = f * g - d * i;
    const T C = d * h - e * g;
    inverted[0][0] = A;                 // A
    inverted[0][1] = B;                 // B
    inverted[0][2] = C;                 // C
    inverted[1][0] = c * h - b * i;     // D
    inverted[1][1] = a * i - c * g;     // E
    inverted[1][2] = b * g - a * h;     // F
    inverted[2][0] = b * f - c * e;     // G
    inverted[2][1] = c * d - a * f;     // H
    inverted[2][2] = a * e - b * d;     // I

    const T det(a * A + b * B + c * C);
    for (size_t col = 0; col < 3; ++col) {
        for (size_t row = 0; row < 3; ++row) {
            inverted[col][row] /= det;
        }
    }

    return inverted;
}


//------------------------------------------------------------------------------
// Determinant and cofactor

// this is just a dummy matrix helper
template<typename T, size_t ORDER>
class Matrix {
    T m[ORDER][ORDER];
public:
    constexpr auto operator[](size_t i) const noexcept { return m[i]; }

    constexpr auto& operator[](size_t i) noexcept { return m[i]; }

    static constexpr Matrix<T, ORDER - 1> submatrix(Matrix in, size_t row, size_t col) noexcept {
        size_t colCount = 0, rowCount = 0;
        Matrix<T, ORDER - 1> dest{};
        for (size_t i = 0; i < ORDER; i++) {
            if (i != row) {
                colCount = 0;
                for (size_t j = 0; j < ORDER; j++) {
                    if (j != col) {
                        dest[rowCount][colCount] = in[i][j];
                        colCount++;
                    }
                }
                rowCount++;
            }
        }
        return dest;
    }
};

template<typename T, size_t O>
struct Determinant {
    static constexpr T determinant(Matrix<T, O> in) {
        T det = {};
        for (size_t i = 0; i < O; i++) {
            T m = Determinant<T, O - 1>::determinant(Matrix<T, O>::submatrix(in, 0, i));
            T factor = (i % 2 == 1) ? T(-1) : T(1);
            det += factor * in[0][i] * m;
        }
        return det;
    }
};

template<typename T>
struct Determinant<T, 3> {
    static constexpr T determinant(Matrix<T, 3> in) {
        return
            in[0][0] * in[1][1] * in[2][2] +
            in[1][0] * in[2][1] * in[0][2] +
            in[2][0] * in[0][1] * in[1][2] -
            in[2][0] * in[1][1] * in[0][2] -
            in[1][0] * in[0][1] * in[2][2] -
            in[0][0] * in[2][1] * in[1][2];
    }
};

template<typename T>
struct Determinant<T, 2> {
    static constexpr T determinant(Matrix<T, 2> in) {
        return in[0][0] * in[1][1] - in[0][1] * in[1][0];
    }
};

template<typename T>
struct Determinant<T, 1> {
    static constexpr T determinant(Matrix<T, 1> in) { return in[0][0]; }
};

template<typename MATRIX>
constexpr MATRIX MATH_PURE cofactor(const MATRIX& m) {
    typedef typename MATRIX::value_type T;

    MATRIX out;
    constexpr size_t order = MATRIX::NUM_COLS;

    Matrix<T, order> in{};
    for (size_t i = 0; i < order; i++) {
        for (size_t j = 0; j < order; j++) {
            in[i][j] = m[i][j];
        }
    }

    for (size_t i = 0; i < order; i++) {
        for (size_t j = 0; j < order; j++) {
            T factor = ((i + j) % 2 == 1) ? T(-1) : T(1);
            out[i][j] = Determinant<T, order - 1>::determinant(
                    Matrix<T, order>::submatrix(in, i, j)) * factor;
        }
    }
    return out;
}

template<typename MATRIX>
constexpr MATRIX MATH_PURE fastCofactor2(const MATRIX& m) {
    typedef typename MATRIX::value_type T;

    // Assuming the input matrix is:
    // | a b |
    // | c d |
    //
    // The cofactor are
    // | d -c |
    // | -b a |
    //
    // Importantly, our matrices are column-major!

    MATRIX cof{};

    const T a = m[0][0];
    const T c = m[0][1];
    const T b = m[1][0];
    const T d = m[1][1];

    cof[0][0] = d;
    cof[0][1] = -b;
    cof[1][0] = -c;
    cof[1][1] = a;
    return cof;
}

template<typename MATRIX>
constexpr MATRIX MATH_PURE fastCofactor3(const MATRIX& m) {
    typedef typename MATRIX::value_type T;

    // Assuming the input matrix is:
    // | a b c |
    // | d e f |
    // | g h i |
    //
    // The cofactor are
    // | A B C |
    // | D E F |
    // | G H I |

    // Where:
    // A = (ei - fh), B = (fg - di), C = (dh - eg)
    // D = (ch - bi), E = (ai - cg), F = (bg - ah)
    // G = (bf - ce), H = (cd - af), I = (ae - bd)

    // Importantly, our matrices are column-major!

    MATRIX cof{};

    const T a = m[0][0];
    const T b = m[1][0];
    const T c = m[2][0];
    const T d = m[0][1];
    const T e = m[1][1];
    const T f = m[2][1];
    const T g = m[0][2];
    const T h = m[1][2];
    const T i = m[2][2];

    cof[0][0] = e * i - f * h;  // A
    cof[0][1] = c * h - b * i;  // D
    cof[0][2] = b * f - c * e;  // G
    cof[1][0] = f * g - d * i;  // B
    cof[1][1] = a * i - c * g;  // E
    cof[1][2] = c * d - a * f;  // H
    cof[2][0] = d * h - e * g;  // C
    cof[2][1] = b * g - a * h;  // F
    cof[2][2] = a * e - b * d;  // I

    return cof;
}


/**
 * Cofactor function which switches on the matrix size.
 */
template<typename MATRIX,
        typename = std::enable_if_t<MATRIX::NUM_ROWS == MATRIX::NUM_COLS, int>>
inline constexpr MATRIX MATH_PURE cof(const MATRIX& matrix) {
    return (MATRIX::NUM_ROWS == 2) ? fastCofactor2<MATRIX>(matrix) :
           ((MATRIX::NUM_ROWS == 3) ? fastCofactor3<MATRIX>(matrix) :
            cofactor<MATRIX>(matrix));
}

/**
 * Determinant of a matrix
 */
template<typename MATRIX,
        typename = std::enable_if_t<MATRIX::NUM_ROWS == MATRIX::NUM_COLS, int>>
inline constexpr typename MATRIX::value_type MATH_PURE det(const MATRIX& matrix) {
    typedef typename MATRIX::value_type T;
    constexpr unsigned int N = MATRIX::NUM_ROWS;
    Matrix<T, N> in{};
    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < N; j++) {
            in[i][j] = matrix[i][j];
        }
    }
    return Determinant<typename MATRIX::value_type, MATRIX::NUM_COLS>::determinant(in);
}

/**
 * Inversion function which switches on the matrix size.
 * @warning This function assumes the matrix is invertible. The result is
 * undefined if it is not. It is the responsibility of the caller to
 * make sure the matrix is not singular.
 */
template<typename MATRIX,
        typename = std::enable_if_t<MATRIX::NUM_ROWS == MATRIX::NUM_COLS, int>>
inline constexpr MATRIX MATH_PURE inverse(const MATRIX& matrix) {
    return (MATRIX::NUM_ROWS == 2) ? fastInverse2<MATRIX>(matrix) :
           ((MATRIX::NUM_ROWS == 3) ? fastInverse3<MATRIX>(matrix) :
            gaussJordanInverse<MATRIX>(matrix));
}

template<typename MATRIX_R, typename MATRIX_A, typename MATRIX_B,
        typename = std::enable_if_t<
                MATRIX_A::NUM_COLS == MATRIX_B::NUM_ROWS &&
                MATRIX_R::NUM_COLS == MATRIX_B::NUM_COLS &&
                MATRIX_R::NUM_ROWS == MATRIX_A::NUM_ROWS, int>>
constexpr MATRIX_R MATH_PURE multiply(MATRIX_A lhs, MATRIX_B rhs) {
    // pre-requisite:
    //  lhs : D columns, R rows
    //  rhs : C columns, D rows
    //  res : C columns, R rows
    MATRIX_R res{};
    for (size_t col = 0; col < MATRIX_R::NUM_COLS; ++col) {
        res[col] = lhs * rhs[col];
    }
    return res;
}

template<typename MATRIX,
        typename = std::enable_if_t<MATRIX::NUM_ROWS == MATRIX::NUM_COLS, int>>
inline constexpr MATRIX MATH_PURE transpose(MATRIX m) {
    // for now we only handle square matrix transpose
    MATRIX result{};
    for (size_t col = 0; col < MATRIX::NUM_COLS; ++col) {
        for (size_t row = 0; row < MATRIX::NUM_ROWS; ++row) {
            result[col][row] = m[row][col];
        }
    }
    return result;
}

template<typename MATRIX,
        typename = std::enable_if_t<MATRIX::NUM_ROWS == MATRIX::NUM_COLS, int>>
inline constexpr typename MATRIX::value_type MATH_PURE trace(MATRIX m) {
    typename MATRIX::value_type result{};
    for (size_t col = 0; col < MATRIX::NUM_COLS; ++col) {
        result += m[col][col];
    }
    return result;
}

template<typename MATRIX,
        typename = std::enable_if_t<MATRIX::NUM_ROWS == MATRIX::NUM_COLS, int>>
inline constexpr typename MATRIX::col_type MATH_PURE diag(MATRIX m) {
    typename MATRIX::col_type result{};
    for (size_t col = 0; col < MATRIX::NUM_COLS; ++col) {
        result[col] = m[col][col];
    }
    return result;
}

//------------------------------------------------------------------------------
// This is taken from the Imath MatrixAlgo code, and is identical to Eigen.
template<typename MATRIX>
TQuaternion<typename MATRIX::value_type> extractQuat(const MATRIX& mat) {
    typedef typename MATRIX::value_type T;

    TQuaternion<T> quat(TQuaternion<T>::NO_INIT);

    // Compute the trace to see if it is positive or not.
    const T trace = mat[0][0] + mat[1][1] + mat[2][2];

    // check the sign of the trace
    if (MATH_LIKELY(trace > 0)) {
        // trace is positive
        T s = std::sqrt(trace + 1);
        quat.w = T(0.5) * s;
        s = T(0.5) / s;
        quat.x = (mat[1][2] - mat[2][1]) * s;
        quat.y = (mat[2][0] - mat[0][2]) * s;
        quat.z = (mat[0][1] - mat[1][0]) * s;
    } else {
        // trace is negative

        // Find the index of the greatest diagonal
        size_t i = 0;
        if (mat[1][1] > mat[0][0]) { i = 1; }
        if (mat[2][2] > mat[i][i]) { i = 2; }

        // Get the next indices: (n+1)%3
        static constexpr size_t next_ijk[3] = { 1, 2, 0 };
        size_t j = next_ijk[i];
        size_t k = next_ijk[j];
        T s = std::sqrt((mat[i][i] - (mat[j][j] + mat[k][k])) + 1);
        quat[i] = T(0.5) * s;
        if (s != 0) {
            s = T(0.5) / s;
        }
        quat.w  = (mat[j][k] - mat[k][j]) * s;
        quat[j] = (mat[i][j] + mat[j][i]) * s;
        quat[k] = (mat[i][k] + mat[k][i]) * s;
    }
    return quat;
}

}  // namespace matrix

// -------------------------------------------------------------------------------------

/*
 * TMatProductOperators implements basic arithmetic and basic compound assignments
 * operators on a vector of type BASE<T>.
 *
 * BASE only needs to implement operator[] and size().
 * By simply inheriting from TMatProductOperators<BASE, T> BASE will automatically
 * get all the functionality here.
 */

template<template<typename> class BASE, typename T,
        template<typename> class VEC>
class TMatProductOperators {
public:
    //  matrix *= matrix
    template<typename U>
    constexpr BASE<T>& operator*=(const BASE<U>& rhs) {
        BASE<T>& lhs(static_cast< BASE<T>& >(*this));
        lhs = matrix::multiply<BASE<T>>(lhs, rhs);
        return lhs;
    }

    // matrix *= scalar
    template<typename U, typename = enable_if_arithmetic_t<U>>
    constexpr BASE<T>& operator*=(U v) {
        BASE<T>& lhs(static_cast< BASE<T>& >(*this));
        for (size_t col = 0; col < BASE<T>::NUM_COLS; ++col) {
            lhs[col] *= v;
        }
        return lhs;
    }

    // matrix /= scalar
    template<typename U, typename = enable_if_arithmetic_t<U>>
    constexpr BASE<T>& operator/=(U v) {
        BASE<T>& lhs(static_cast< BASE<T>& >(*this));
        for (size_t col = 0; col < BASE<T>::NUM_COLS; ++col) {
            lhs[col] /= v;
        }
        return lhs;
    }

private:
    /*
     * NOTE: the functions below ARE NOT member methods. They are friend functions
     * with they definition inlined with their declaration. This makes these
     * template functions available to the compiler when (and only when) this class
     * is instantiated, at which point they're only templated on the 2nd parameter
     * (the first one, BASE<T> being known).
     */

    //  matrix * matrix
    template<typename U>
    friend inline constexpr BASE<arithmetic_result_t<T, U>> MATH_PURE
    operator*(BASE<T> lhs, BASE<U> rhs) {
        return matrix::multiply<BASE<arithmetic_result_t<T, U>>>(lhs, rhs);
    }

    // matrix * vector
    template<typename U>
    friend inline constexpr typename BASE<arithmetic_result_t<T, U>>::col_type MATH_PURE
    operator*(const BASE<T>& lhs, const VEC<U>& rhs) {
        typename BASE<arithmetic_result_t<T, U>>::col_type result{};
        for (size_t col = 0; col < BASE<T>::NUM_COLS; ++col) {
            result += lhs[col] * rhs[col];
        }
        return result;
    }

    // row-vector * matrix
    template<typename U>
    friend inline constexpr typename BASE<arithmetic_result_t<T, U>>::row_type MATH_PURE
    operator*(const VEC<U>& lhs, const BASE<T>& rhs) {
        typename BASE<arithmetic_result_t<T, U>>::row_type result{};
        for (size_t col = 0; col < BASE<T>::NUM_COLS; ++col) {
            result[col] = dot(lhs, rhs[col]);
        }
        return result;
    }

    // matrix * scalar
    template<typename U, typename = enable_if_arithmetic_t <U>>
    friend inline constexpr BASE<arithmetic_result_t < T, U>> MATH_PURE
    operator*(const BASE<T>& lhs, U rhs) {
        BASE<arithmetic_result_t<T, U>> result{};
        for (size_t col = 0; col < BASE<T>::NUM_COLS; ++col) {
            result[col] = lhs[col] * rhs;
        }
        return result;
    }

    // scalar * matrix
    template<typename U, typename = enable_if_arithmetic_t<U>>
    friend inline constexpr BASE<arithmetic_result_t<T, U>> MATH_PURE
    operator*(U rhs, const BASE<T>& lhs) {
        return lhs * rhs;
    }

    // matrix / scalar
    template<typename U, typename = enable_if_arithmetic_t<U>>
    friend inline constexpr BASE<arithmetic_result_t<T, U>> MATH_PURE
    operator/(const BASE<T>& lhs, U rhs) {
        BASE<arithmetic_result_t<T, U>> result{};
        for (size_t col = 0; col < BASE<T>::NUM_COLS; ++col) {
            result[col] = lhs[col] / rhs;
        }
        return result;
    }
};

/*
 * TMatSquareFunctions implements functions on a matrix of type BASE<T>.
 *
 * BASE only needs to implement:
 *  - operator[]
 *  - col_type
 *  - row_type
 *  - COL_SIZE
 *  - ROW_SIZE
 *
 * By simply inheriting from TMatSquareFunctions<BASE, T> BASE will automatically
 * get all the functionality here.
 */

template<template<typename U> class BASE, typename T>
class TMatSquareFunctions {
private:
    /*
     * NOTE: the functions below ARE NOT member methods. They are friend functions
     * with they definition inlined with their declaration. This makes these
     * template functions available to the compiler when (and only when) this class
     * is instantiated, at which point they're only templated on the 2nd parameter
     * (the first one, BASE<T> being known).
     */
    friend inline constexpr BASE<T> MATH_PURE inverse(const BASE<T>& matrix) {
        return matrix::inverse(matrix);
    }

    friend inline constexpr BASE<T> MATH_PURE cof(const BASE<T>& matrix) {
        return matrix::cof(matrix);
    }

    friend inline constexpr BASE<T> MATH_PURE transpose(BASE<T> m) {
        return matrix::transpose(m);
    }

    friend inline constexpr T MATH_PURE trace(BASE<T> m) {
        return matrix::trace(m);
    }

    friend inline constexpr T MATH_PURE det(const BASE<T>& m) {
        return matrix::det(m);
    }

    // unclear why we have to use 'auto' here. 'typename BASE<T>::col_type' produces
    // error: no type named 'col_type' in 'filament::math::details::TMat44<float>'
    friend inline constexpr auto MATH_PURE diag(const BASE<T>& m) {
        return matrix::diag(m);
    }
};

template<template<typename U> class BASE, typename T>
class TMatHelpers {
public:
    constexpr inline size_t getColumnSize() const   { return BASE<T>::COL_SIZE; }
    constexpr inline size_t getRowSize() const      { return BASE<T>::ROW_SIZE; }
    constexpr inline size_t getColumnCount() const  { return BASE<T>::NUM_COLS; }
    constexpr inline size_t getRowCount() const     { return BASE<T>::NUM_ROWS; }
    constexpr inline size_t size()  const           { return BASE<T>::ROW_SIZE; }  // for TVec*<>

    // array access
    constexpr T const* asArray() const {
        return &static_cast<BASE<T> const &>(*this)[0][0];
    }

    // element access
    inline constexpr T const& operator()(size_t row, size_t col) const {
        return static_cast<BASE<T> const &>(*this)[col][row];
    }

    inline T& operator()(size_t row, size_t col) {
        return static_cast<BASE<T>&>(*this)[col][row];
    }

private:
    constexpr friend inline BASE<T> MATH_PURE abs(BASE<T> m) {
        for (size_t col = 0; col < BASE<T>::NUM_COLS; ++col) {
            m[col] = abs(m[col]);
        }
        return m;
    }
};

// functions for 3x3 and 4x4 matrices
template<template<typename U> class BASE, typename T>
class TMatTransform {
public:
    inline constexpr TMatTransform() {
        static_assert(BASE<T>::NUM_ROWS == 3 || BASE<T>::NUM_ROWS == 4, "3x3 or 4x4 matrices only");
    }

    template<typename A, typename VEC, typename = enable_if_arithmetic_t<A>>
    static BASE<T> rotation(A radian, VEC about) {
        BASE<T> r;
        T c = std::cos(radian);
        T s = std::sin(radian);
        if (about[0] == 1 && about[1] == 0 && about[2] == 0) {
            r[1][1] = c;   r[2][2] = c;
            r[1][2] = s;   r[2][1] = -s;
        } else if (about[0] == 0 && about[1] == 1 && about[2] == 0) {
            r[0][0] = c;   r[2][2] = c;
            r[2][0] = s;   r[0][2] = -s;
        } else if (about[0] == 0 && about[1] == 0 && about[2] == 1) {
            r[0][0] = c;   r[1][1] = c;
            r[0][1] = s;   r[1][0] = -s;
        } else {
            VEC nabout = normalize(about);
            typename VEC::value_type x = nabout[0];
            typename VEC::value_type y = nabout[1];
            typename VEC::value_type z = nabout[2];
            T nc = 1 - c;
            T xy = x * y;
            T yz = y * z;
            T zx = z * x;
            T xs = x * s;
            T ys = y * s;
            T zs = z * s;
            r[0][0] = x*x*nc +  c;    r[1][0] =  xy*nc - zs;    r[2][0] =  zx*nc + ys;
            r[0][1] =  xy*nc + zs;    r[1][1] = y*y*nc +  c;    r[2][1] =  yz*nc - xs;
            r[0][2] =  zx*nc - ys;    r[1][2] =  yz*nc + xs;    r[2][2] = z*z*nc +  c;

            // Clamp results to -1, 1.
            for (size_t col = 0; col < 3; ++col) {
                for (size_t row = 0; row < 3; ++row) {
                    r[col][row] = std::min(std::max(r[col][row], T(-1)), T(1));
                }
            }
        }
        return r;
    }

    /**
     * Create a matrix from euler angles using YPR around YXZ respectively
     * @param yaw about Y axis
     * @param pitch about X axis
     * @param roll about Z axis
     */
    template<typename Y, typename P, typename R, typename = enable_if_arithmetic_t<Y, P, R>>
    static BASE<T> eulerYXZ(Y yaw, P pitch, R roll) {
        return eulerZYX(roll, pitch, yaw);
    }

    /**
     * Create a matrix from euler angles using YPR around ZYX respectively
     * @param roll about X axis
     * @param pitch about Y axis
     * @param yaw about Z axis
     *
     * The euler angles are applied in ZYX order. i.e: a vector is first rotated
     * about X (roll) then Y (pitch) and then Z (yaw).
     */
    template<typename Y, typename P, typename R, typename = enable_if_arithmetic_t<Y, P, R>>
    static BASE<T> eulerZYX(Y yaw, P pitch, R roll) {
        BASE<T> r;
        T cy = std::cos(yaw);
        T sy = std::sin(yaw);
        T cp = std::cos(pitch);
        T sp = std::sin(pitch);
        T cr = std::cos(roll);
        T sr = std::sin(roll);
        T cc = cr * cy;
        T cs = cr * sy;
        T sc = sr * cy;
        T ss = sr * sy;
        r[0][0] = cp * cy;
        r[0][1] = cp * sy;
        r[0][2] = -sp;
        r[1][0] = sp * sc - cs;
        r[1][1] = sp * ss + cc;
        r[1][2] = cp * sr;
        r[2][0] = sp * cc + ss;
        r[2][1] = sp * cs - sc;
        r[2][2] = cp * cr;

        // Clamp results to -1, 1.
        for (size_t col = 0; col < 3; ++col) {
            for (size_t row = 0; row < 3; ++row) {
                r[col][row] = std::min(std::max(r[col][row], T(-1)), T(1));
            }
        }
        return r;
    }

    TQuaternion <T> toQuaternion() const {
        return matrix::extractQuat(static_cast<const BASE<T>&>(*this));
    }
};


template<template<typename T> class BASE, typename T>
class TMatDebug {
private:
    friend std::ostream& operator<<(std::ostream& stream, const BASE<T>& m) {
        for (size_t row = 0; row < BASE<T>::NUM_ROWS; ++row) {
            if (row != 0) {
                stream << std::endl;
            }
            if (row == 0) {
                stream << "/ ";
            } else if (row == BASE<T>::NUM_ROWS - 1) {
                stream << "\\ ";
            } else {
                stream << "| ";
            }
            for (size_t col = 0; col < BASE<T>::NUM_COLS; ++col) {
                stream << std::setw(10) << std::to_string(m[col][row]);
            }
            if (row == 0) {
                stream << " \\";
            } else if (row == BASE<T>::NUM_ROWS - 1) {
                stream << " /";
            } else {
                stream << " |";
            }
        }
        return stream;
    }
};

// -------------------------------------------------------------------------------------
}  // namespace details
}  // namespace math
}  // namespace filament

#endif  // MATH_TMATHELPERS_H_
