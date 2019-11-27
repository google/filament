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

#ifndef MATH_VEC2_H_
#define MATH_VEC2_H_

#include <math/TVecHelpers.h>
#include <math/half.h>
#include <assert.h>
#include <stdint.h>
#include <sys/types.h>
#include <type_traits>


namespace filament {
namespace math {
// -------------------------------------------------------------------------------------

namespace details {

template<typename T>
class MATH_EMPTY_BASES TVec2 :
        public TVecProductOperators<TVec2, T>,
        public TVecAddOperators<TVec2, T>,
        public TVecUnaryOperators<TVec2, T>,
        public TVecComparisonOperators<TVec2, T>,
        public TVecFunctions<TVec2, T>,
        public TVecDebug<TVec2, T> {
public:
    typedef T value_type;
    typedef T& reference;
    typedef T const& const_reference;
    typedef size_t size_type;
    static constexpr size_t SIZE = 2;

    union {
        T v[SIZE] MATH_CONSTEXPR_INIT;
        struct { T x, y; };
        struct { T s, t; };
        struct { T r, g; };
    };

    inline constexpr size_type size() const { return SIZE; }

    // array access
    inline constexpr T const& operator[](size_t i) const noexcept {
        assert(i < SIZE);
        return v[i];
    }

    inline constexpr T& operator[](size_t i) noexcept {
        assert(i < SIZE);
        return v[i];
    }

    // constructors

    // default constructor
    MATH_DEFAULT_CTOR_CONSTEXPR TVec2() MATH_DEFAULT_CTOR

    // handles implicit conversion to a tvec4. must not be explicit.
    template<typename A, typename = enable_if_arithmetic_t<A>>
    constexpr TVec2(A v) noexcept : v{ T(v), T(v) } {}

    template<typename A, typename B, typename = enable_if_arithmetic_t<A, B>>
    constexpr TVec2(A x, B y) noexcept : v{ T(x), T(y) } {}

    template<typename A, typename = enable_if_arithmetic_t<A>>
    constexpr TVec2(const TVec2<A>& v) noexcept : v{ T(v[0]), T(v[1]) } {}

    // cross product works only on vectors of size 2 or 3
    template<typename U>
    friend inline constexpr
    arithmetic_result_t<T, U> cross(const TVec2& u, const TVec2<U>& v) noexcept {
        return u[0] * v[1] - u[1] * v[0];
    }
};

}  // namespace details

// ----------------------------------------------------------------------------------------

template<typename T, typename = details::enable_if_arithmetic_t<T>>
using vec2 = details::TVec2<T>;

using double2 = vec2<double>;
using float2 = vec2<float>;
using half2 = vec2<half>;
using int2 = vec2<int32_t>;
using uint2 = vec2<uint32_t>;
using short2 = vec2<int16_t>;
using ushort2 = vec2<uint16_t>;
using byte2 = vec2<int8_t>;
using ubyte2 = vec2<uint8_t>;
using bool2 = vec2<bool>;

// ----------------------------------------------------------------------------------------
}  // namespace math
}  // namespace filament

#endif  // MATH_VEC2_H_
