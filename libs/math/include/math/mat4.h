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

#ifndef MATH_MAT4_H_
#define MATH_MAT4_H_

#include <math/compiler.h>
#include <math/mat3.h>
#include <math/quat.h>
#include <math/scalar.h>
#include <math/TMatHelpers.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <stdint.h>
#include <sys/types.h>
#include <limits>

namespace filament {
namespace math {
// -------------------------------------------------------------------------------------
namespace details {

template<typename T>
class TQuaternion;

/**
 * A 4x4 column-major matrix class.
 *
 * Conceptually a 4x4 matrix is a an array of 4 column double4:
 *
 * mat4 m =
 *      \f$
 *      \left(
 *      \begin{array}{cccc}
 *      m[0] & m[1] & m[2] & m[3] \\
 *      \end{array}
 *      \right)
 *      \f$
 *      =
 *      \f$
 *      \left(
 *      \begin{array}{cccc}
 *      m[0][0] & m[1][0] & m[2][0] & m[3][0] \\
 *      m[0][1] & m[1][1] & m[2][1] & m[3][1] \\
 *      m[0][2] & m[1][2] & m[2][2] & m[3][2] \\
 *      m[0][3] & m[1][3] & m[2][3] & m[3][3] \\
 *      \end{array}
 *      \right)
 *      \f$
 *      =
 *      \f$
 *      \left(
 *      \begin{array}{cccc}
 *      m(0,0) & m(0,1) & m(0,2) & m(0,3) \\
 *      m(1,0) & m(1,1) & m(1,2) & m(1,3) \\
 *      m(2,0) & m(2,1) & m(2,2) & m(2,3) \\
 *      m(3,0) & m(3,1) & m(3,2) & m(3,3) \\
 *      \end{array}
 *      \right)
 *      \f$
 *
 * m[n] is the \f$ n^{th} \f$ column of the matrix and is a double4.
 *
 */
template<typename T>
class MATH_EMPTY_BASES TMat44 :
        public TVecUnaryOperators<TMat44, T>,
        public TVecComparisonOperators<TMat44, T>,
        public TVecAddOperators<TMat44, T>,
        public TMatProductOperators<TMat44, T, TVec4>,
        public TMatSquareFunctions<TMat44, T>,
        public TMatTransform<TMat44, T>,
        public TMatHelpers<TMat44, T>,
        public TMatDebug<TMat44, T> {
public:
    enum no_init {
        NO_INIT
    };
    typedef T value_type;
    typedef T& reference;
    typedef T const& const_reference;
    typedef size_t size_type;
    typedef TVec4<T> col_type;
    typedef TVec4<T> row_type;

    static constexpr size_t COL_SIZE = col_type::SIZE;  // size of a column (i.e.: number of rows)
    static constexpr size_t ROW_SIZE = row_type::SIZE;  // size of a row (i.e.: number of columns)
    static constexpr size_t NUM_ROWS = COL_SIZE;
    static constexpr size_t NUM_COLS = ROW_SIZE;

private:
    /*
     *  <--  N columns  -->
     *
     *  a[0][0] a[1][0] a[2][0] ... a[N][0]    ^
     *  a[0][1] a[1][1] a[2][1] ... a[N][1]    |
     *  a[0][2] a[1][2] a[2][2] ... a[N][2]  M rows
     *  ...                                    |
     *  a[0][M] a[1][M] a[2][M] ... a[N][M]    v
     *
     *  COL_SIZE = M
     *  ROW_SIZE = N
     *  m[0] = [ a[0][0] a[0][1] a[0][2] ... a[0][M] ]
     */

    col_type m_value[NUM_COLS];

public:
    // array access
    inline constexpr col_type const& operator[](size_t column) const noexcept {
        assert(column < NUM_COLS);
        return m_value[column];
    }

    inline constexpr col_type& operator[](size_t column) noexcept {
        assert(column < NUM_COLS);
        return m_value[column];
    }

    /*
     *  constructors
     */

    // leaves object uninitialized. use with caution.
    constexpr explicit TMat44(no_init) noexcept {}

    /** initialize to identity.
     *
     *      \f$
     *      \left(
     *      \begin{array}{cccc}
     *      1 & 0 & 0 & 0 \\
     *      0 & 1 & 0 & 0 \\
     *      0 & 0 & 1 & 0 \\
     *      0 & 0 & 0 & 1 \\
     *      \end{array}
     *      \right)
     *      \f$
     */
    constexpr TMat44() noexcept;

    /** initialize to Identity*scalar.
     *
     *      \f$
     *      \left(
     *      \begin{array}{cccc}
     *      v & 0 & 0 & 0 \\
     *      0 & v & 0 & 0 \\
     *      0 & 0 & v & 0 \\
     *      0 & 0 & 0 & v \\
     *      \end{array}
     *      \right)
     *      \f$
     */
    template<typename U>
    constexpr explicit TMat44(U v) noexcept;

    /** sets the diagonal to a vector.
     *
     *      \f$
     *      \left(
     *      \begin{array}{cccc}
     *      v[0] & 0 & 0 & 0 \\
     *      0 & v[1] & 0 & 0 \\
     *      0 & 0 & v[2] & 0 \\
     *      0 & 0 & 0 & v[3] \\
     *      \end{array}
     *      \right)
     *      \f$
     */
    template<typename U>
    constexpr explicit TMat44(const TVec4<U>& v) noexcept;

    // construct from another matrix of the same size
    template<typename U>
    constexpr explicit TMat44(const TMat44<U>& rhs) noexcept;

    /** construct from 4 column vectors.
     *
     *      \f$
     *      \left(
     *      \begin{array}{cccc}
     *      v0 & v1 & v2 & v3 \\
     *      \end{array}
     *      \right)
     *      \f$
     */
    template<typename A, typename B, typename C, typename D>
    constexpr TMat44(const TVec4<A>& v0, const TVec4<B>& v1, const TVec4<C>& v2,
            const TVec4<D>& v3) noexcept;

    /** construct from 16 elements in column-major form.
     *
     *      \f$
     *      \left(
     *      \begin{array}{cccc}
     *      m[0][0] & m[1][0] & m[2][0] & m[3][0] \\
     *      m[0][1] & m[1][1] & m[2][1] & m[3][1] \\
     *      m[0][2] & m[1][2] & m[2][2] & m[3][2] \\
     *      m[0][3] & m[1][3] & m[2][3] & m[3][3] \\
     *      \end{array}
     *      \right)
     *      \f$
     */
    template<
            typename A, typename B, typename C, typename D,
            typename E, typename F, typename G, typename H,
            typename I, typename J, typename K, typename L,
            typename M, typename N, typename O, typename P>
    constexpr explicit TMat44(A m00, B m01, C m02, D m03,
            E m10, F m11, G m12, H m13,
            I m20, J m21, K m22, L m23,
            M m30, N m31, O m32, P m33) noexcept;


    struct row_major_init {
        template<
                typename A, typename B, typename C, typename D,
                typename E, typename F, typename G, typename H,
                typename I, typename J, typename K, typename L,
                typename M, typename N, typename O, typename P>
        constexpr explicit row_major_init(A m00, B m01, C m02, D m03,
                E m10, F m11, G m12, H m13,
                I m20, J m21, K m22, L m23,
                M m30, N m31, O m32, P m33) noexcept
                : m(m00, m10, m20, m30,
                m01, m11, m21, m31,
                m02, m12, m22, m32,
                m03, m13, m23, m33) {}

    private:
        friend TMat44;
        TMat44 m;
    };

    constexpr explicit TMat44(row_major_init c) noexcept : TMat44(std::move(c.m)) {}

    /**
     * construct from a quaternion
     */
    template<typename U>
    constexpr explicit TMat44(const TQuaternion<U>& q) noexcept;

    /**
     * construct from a 3x3 matrix
     */
    template<typename U>
    constexpr explicit TMat44(const TMat33<U>& matrix) noexcept;

    /**
     * construct from a 3x3 matrix and 3d translation
     */
    template<typename U, typename V>
    constexpr TMat44(const TMat33<U>& matrix, const TVec3<V>& translation) noexcept;

    /**
     * construct from a 3x3 matrix and 4d last column.
     */
    template<typename U, typename V>
    constexpr TMat44(const TMat33<U>& matrix, const TVec4<V>& column3) noexcept;

    /*
     *  helpers
     */

    // returns false if the two matrices are different. May return false if they're the
    // same, with some elements only differing by +0 or -0. Behaviour is undefined with NaNs.
    static constexpr bool fuzzyEqual(TMat44 const& l, TMat44 const& r) noexcept {
        uint64_t const* const li = reinterpret_cast<uint64_t const*>(&l);
        uint64_t const* const ri = reinterpret_cast<uint64_t const*>(&r);
        uint64_t result = 0;
        // For some reason clang is not able to vectorize this loop when the number of iteration
        // is known and constant (!?!?!). Still this is better than operator==.
        for (size_t i = 0; i < sizeof(TMat44) / sizeof(uint64_t); i++) {
            result |= li[i] ^ ri[i];
        }
        return result != 0;
    }

    static constexpr TMat44 ortho(T left, T right, T bottom, T top, T near, T far) noexcept;

    static constexpr TMat44 frustum(T left, T right, T bottom, T top, T near, T far) noexcept;

    enum class Fov {
        HORIZONTAL,
        VERTICAL
    };
    static TMat44 perspective(T fov, T aspect, T near, T far, Fov direction = Fov::VERTICAL) noexcept;

    template<typename A, typename B, typename C>
    static TMat44 lookAt(const TVec3<A>& eye, const TVec3<B>& center, const TVec3<C>& up) noexcept;

    template<typename A>
    static constexpr TVec3<A> project(const TMat44& projectionMatrix, TVec3<A> vertice) noexcept{
        TVec4<A> r = projectionMatrix * TVec4<A>{ vertice, 1 };
        return TVec3<A>{ r[0], r[1], r[2] } * (1 / r[3]);
    }

    template<typename A>
    static constexpr TVec4<A> project(const TMat44& projectionMatrix, TVec4<A> vertice) noexcept{
        vertice = projectionMatrix * vertice;
        return { TVec3<A>{ vertice[0], vertice[1], vertice[2] } * (1 / vertice[3]), 1 };
    }

    /**
     * Constructs a 3x3 matrix from the upper-left corner of this 4x4 matrix
     */
    inline constexpr TMat33<T> upperLeft() const noexcept {
        const TVec3<T> v0 = { m_value[0][0], m_value[0][1], m_value[0][2] };
        const TVec3<T> v1 = { m_value[1][0], m_value[1][1], m_value[1][2] };
        const TVec3<T> v2 = { m_value[2][0], m_value[2][1], m_value[2][2] };
        return TMat33<T>(v0, v1, v2);
    }

    template<typename A>
    static constexpr TMat44 translation(const TVec3<A>& t) noexcept {
        TMat44 r;
        r[3] = TVec4<T>{ t, 1 };
        return r;
    }

    template<typename A>
    static constexpr TMat44 scaling(const TVec3<A>& s) noexcept {
        return TMat44{ TVec4<T>{ s, 1 }};
    }

    template<typename A>
    static constexpr TMat44 scaling(A s) noexcept {
        return TMat44{ TVec4<T>{ s, s, s, 1 }};
    }
};

// ----------------------------------------------------------------------------------------
// Constructors
// ----------------------------------------------------------------------------------------

// Since the matrix code could become pretty big quickly, we don't inline most
// operations.

template<typename T>
constexpr TMat44<T>::TMat44() noexcept
        : m_value{
        col_type(1, 0, 0, 0),
        col_type(0, 1, 0, 0),
        col_type(0, 0, 1, 0),
        col_type(0, 0, 0, 1) } {
}

template<typename T>
template<typename U>
constexpr TMat44<T>::TMat44(U v) noexcept
        : m_value{
        col_type(v, 0, 0, 0),
        col_type(0, v, 0, 0),
        col_type(0, 0, v, 0),
        col_type(0, 0, 0, v) } {
}

template<typename T>
template<typename U>
constexpr TMat44<T>::TMat44(const TVec4<U>& v) noexcept
        : m_value{
        col_type(v[0], 0, 0, 0),
        col_type(0, v[1], 0, 0),
        col_type(0, 0, v[2], 0),
        col_type(0, 0, 0, v[3]) } {
}


// construct from 16 scalars
template<typename T>
template<
        typename A, typename B, typename C, typename D,
        typename E, typename F, typename G, typename H,
        typename I, typename J, typename K, typename L,
        typename M, typename N, typename O, typename P>
constexpr TMat44<T>::TMat44(A m00, B m01, C m02, D m03,
        E m10, F m11, G m12, H m13,
        I m20, J m21, K m22, L m23,
        M m30, N m31, O m32, P m33) noexcept
        : m_value{
        col_type(m00, m01, m02, m03),
        col_type(m10, m11, m12, m13),
        col_type(m20, m21, m22, m23),
        col_type(m30, m31, m32, m33) } {
}

template<typename T>
template<typename U>
constexpr TMat44<T>::TMat44(const TMat44<U>& rhs) noexcept {
    for (size_t col = 0; col < NUM_COLS; ++col) {
        m_value[col] = col_type(rhs[col]);
    }
}

// Construct from 4 column vectors.
template<typename T>
template<typename A, typename B, typename C, typename D>
constexpr TMat44<T>::TMat44(const TVec4<A>& v0, const TVec4<B>& v1,
        const TVec4<C>& v2, const TVec4<D>& v3) noexcept
        : m_value{ v0, v1, v2, v3 } {
}

template<typename T>
template<typename U>
constexpr TMat44<T>::TMat44(const TQuaternion<U>& q) noexcept : m_value{} {
    const U n = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
    const U s = n > 0 ? 2 / n : 0;
    const U x = s * q.x;
    const U y = s * q.y;
    const U z = s * q.z;
    const U xx = x * q.x;
    const U xy = x * q.y;
    const U xz = x * q.z;
    const U xw = x * q.w;
    const U yy = y * q.y;
    const U yz = y * q.z;
    const U yw = y * q.w;
    const U zz = z * q.z;
    const U zw = z * q.w;
    m_value[0] = col_type(1 - yy - zz, xy + zw, xz - yw, 0);
    m_value[1] = col_type(xy - zw, 1 - xx - zz, yz + xw, 0);  // NOLINT
    m_value[2] = col_type(xz + yw, yz - xw, 1 - xx - yy, 0);  // NOLINT
    m_value[3] = col_type(0, 0, 0, 1);  // NOLINT
}

template<typename T>
template<typename U>
constexpr TMat44<T>::TMat44(const TMat33<U>& m) noexcept
        : m_value{
        col_type(m[0][0], m[0][1], m[0][2], 0),
        col_type(m[1][0], m[1][1], m[1][2], 0),
        col_type(m[2][0], m[2][1], m[2][2], 0),
        col_type(0, 0, 0, 1) }  // NOLINT
{
}

template<typename T>
template<typename U, typename V>
constexpr TMat44<T>::TMat44(const TMat33<U>& m, const TVec3<V>& v) noexcept
        : m_value{
        col_type(m[0][0], m[0][1], m[0][2], 0),
        col_type(m[1][0], m[1][1], m[1][2], 0),
        col_type(m[2][0], m[2][1], m[2][2], 0),
        col_type(v[0], v[1], v[2], 1) }  // NOLINT
{
}

template<typename T>
template<typename U, typename V>
constexpr TMat44<T>::TMat44(const TMat33<U>& m, const TVec4<V>& v) noexcept
        : m_value{
        col_type(m[0][0], m[0][1], m[0][2], 0),
        col_type(m[1][0], m[1][1], m[1][2], 0),
        col_type(m[2][0], m[2][1], m[2][2], 0),
        col_type(v[0], v[1], v[2], v[3]) }  // NOLINT
{
}


// ----------------------------------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------------------------------

template<typename T>
constexpr TMat44<T> TMat44<T>::ortho(T left, T right, T bottom, T top, T near, T far) noexcept {
    TMat44<T> m;
    m[0][0] = 2 / (right - left);
    m[1][1] = 2 / (top - bottom);
    m[2][2] = -2 / (far - near);
    m[3][0] = -(right + left) / (right - left);
    m[3][1] = -(top + bottom) / (top - bottom);
    m[3][2] = -(far + near) / (far - near);
    return m;
}

template<typename T>
constexpr TMat44<T> TMat44<T>::frustum(T left, T right, T bottom, T top, T near, T far) noexcept {
    TMat44<T> m;
    m[0][0] = (2 * near) / (right - left);
    m[1][1] = (2 * near) / (top - bottom);
    m[2][0] = (right + left) / (right - left);
    m[2][1] = (top + bottom) / (top - bottom);
    m[2][2] = -(far + near) / (far - near);
    m[2][3] = -1;
    m[3][2] = -(2 * far * near) / (far - near);
    m[3][3] = 0;
    return m;
}

template<typename T>
TMat44<T> TMat44<T>::perspective(T fov, T aspect, T near, T far, TMat44::Fov direction) noexcept {
    T h, w;

    if (direction == TMat44::Fov::VERTICAL) {
        h = std::tan(fov * F_PI / 360.0f) * near;
        w = h * aspect;
    } else {
        w = std::tan(fov * F_PI / 360.0f) * near;
        h = w / aspect;
    }
    return frustum(-w, w, -h, h, near, far);
}

/*
 * Returns a matrix representing the pose of a virtual camera looking towards -Z in its
 * local Y-up coordinate system. "eye" is where the camera is located, "center" is the point it's
 * looking at and "up" defines where the Y axis of the camera's local coordinate system is.
 */
template<typename T>
template<typename A, typename B, typename C>
TMat44<T> TMat44<T>::lookAt(const TVec3<A>& eye, const TVec3<B>& center,
        const TVec3<C>& up) noexcept {
    TVec3<T> z_axis(normalize(center - eye));
    TVec3<T> norm_up(normalize(up));
    if (std::abs(dot(z_axis, norm_up)) > T(0.999)) {
        // Fix up vector if we're degenerate (looking straight up, basically)
        norm_up = { norm_up.z, norm_up.x, norm_up.y };
    }
    TVec3<T> x_axis(normalize(cross(z_axis, norm_up)));
    TVec3<T> y_axis(cross(x_axis, z_axis));
    return TMat44<T>(
            TVec4<T>(x_axis, 0),
            TVec4<T>(y_axis, 0),
            TVec4<T>(-z_axis, 0),
            TVec4<T>(eye, 1));
}

// ----------------------------------------------------------------------------------------
// Arithmetic operators outside of class
// ----------------------------------------------------------------------------------------

// mat44 * vec3, result is vec3( mat44 * {vec3, 1} )
template<typename T, typename U>
constexpr typename TMat44<T>::col_type MATH_PURE operator*(const TMat44<T>& lhs,
        const TVec3<U>& rhs) noexcept {
    return lhs * TVec4<U>{ rhs, 1 };
}

} // namespace details

// ----------------------------------------------------------------------------------------

typedef details::TMat44<double> mat4;
typedef details::TMat44<float> mat4f;

// ----------------------------------------------------------------------------------------
}  // namespace math
}  // namespace filament

namespace std {
template<typename T>
constexpr void swap(filament::math::details::TMat44<T>& lhs,
        filament::math::details::TMat44<T>& rhs) noexcept {
    // This generates much better code than the default implementation
    // It's unclear why, I believe this is due to an optimization bug in the clang.
    //
    //     filament::math::details::TMat44<T> t(lhs);
    //    lhs = rhs;
    //    rhs = t;
    //
    // clang always copy lhs on the stack, even if it's never using it (it's using the
    // copy it has in registers).

    const T t00 = lhs[0][0];
    const T t01 = lhs[0][1];
    const T t02 = lhs[0][2];
    const T t03 = lhs[0][3];
    const T t10 = lhs[1][0];
    const T t11 = lhs[1][1];
    const T t12 = lhs[1][2];
    const T t13 = lhs[1][3];
    const T t20 = lhs[2][0];
    const T t21 = lhs[2][1];
    const T t22 = lhs[2][2];
    const T t23 = lhs[2][3];
    const T t30 = lhs[3][0];
    const T t31 = lhs[3][1];
    const T t32 = lhs[3][2];
    const T t33 = lhs[3][3];

    lhs[0][0] = rhs[0][0];
    lhs[0][1] = rhs[0][1];
    lhs[0][2] = rhs[0][2];
    lhs[0][3] = rhs[0][3];
    lhs[1][0] = rhs[1][0];
    lhs[1][1] = rhs[1][1];
    lhs[1][2] = rhs[1][2];
    lhs[1][3] = rhs[1][3];
    lhs[2][0] = rhs[2][0];
    lhs[2][1] = rhs[2][1];
    lhs[2][2] = rhs[2][2];
    lhs[2][3] = rhs[2][3];
    lhs[3][0] = rhs[3][0];
    lhs[3][1] = rhs[3][1];
    lhs[3][2] = rhs[3][2];
    lhs[3][3] = rhs[3][3];

    rhs[0][0] = t00;
    rhs[0][1] = t01;
    rhs[0][2] = t02;
    rhs[0][3] = t03;
    rhs[1][0] = t10;
    rhs[1][1] = t11;
    rhs[1][2] = t12;
    rhs[1][3] = t13;
    rhs[2][0] = t20;
    rhs[2][1] = t21;
    rhs[2][2] = t22;
    rhs[2][3] = t23;
    rhs[3][0] = t30;
    rhs[3][1] = t31;
    rhs[3][2] = t32;
    rhs[3][3] = t33;
}
}

#endif  // MATH_MAT4_H_
