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

#ifndef MATH_VEC3_H_
#define MATH_VEC3_H_

#include <math/vec2.h>
#include <math/half.h>
#include <stdint.h>
#include <sys/types.h>


namespace filament {
namespace math {
// -------------------------------------------------------------------------------------

namespace details {

template <typename T>
class MATH_EMPTY_BASES TVec3 :
                public TVecProductOperators<TVec3, T>,
                public TVecAddOperators<TVec3, T>,
                public TVecUnaryOperators<TVec3, T>,
                public TVecComparisonOperators<TVec3, T>,
                public TVecFunctions<TVec3, T>,
                public TVecDebug<TVec3, T> {
public:
    typedef T value_type;
    typedef T& reference;
    typedef T const& const_reference;
    typedef size_t size_type;
    static constexpr size_t SIZE = 3;

    union {
        T v[SIZE];
        TVec2<T> xy;
        TVec2<T> st;
        TVec2<T> rg;
        struct {
            union {
                T x;
                T s;
                T r;
            };
            union {
                struct { T y, z; };
                struct { T t, p; };
                struct { T g, b; };
                TVec2<T> yz;
                TVec2<T> tp;
                TVec2<T> gb;
            };
        };
    };

    inline constexpr size_type size() const { return SIZE; }

    // array access
    inline constexpr T const& operator[](size_t i) const {
        // only possible in C++0x14 with constexpr
        assert(i < SIZE);
        return v[i];
    }

    inline constexpr T& operator[](size_t i) {
        assert(i < SIZE);
        return v[i];
    }

    // constructors

    // default constructor
    constexpr TVec3() = default;

    // handles implicit conversion to a tvec4. must not be explicit.
    template<typename A>
    constexpr TVec3(A v) : v{ T(v), T(v), T(v) } {}

    template<typename A, typename B, typename C>
    constexpr TVec3(A x, B y, C z) : v{ T(x), T(y), T(z) } {}

    template<typename A, typename B>
    constexpr TVec3(const TVec2<A>& v, B z) : v{ T(v[0]), T(v[1]), T(z) } {}

    template<typename A>
    constexpr TVec3(const TVec3<A>& v) : v{ T(v[0]), T(v[1]), T(v[2]) } {}

    // cross product works only on vectors of size 3
    template <typename RT>
    friend inline constexpr TVec3 cross(const TVec3& u, const TVec3<RT>& v) {
        return TVec3(
                u[1] * v[2] - u[2] * v[1],
                u[2] * v[0] - u[0] * v[2],
                u[0] * v[1] - u[1] * v[0]);
    }
};

}  // namespace details

// ----------------------------------------------------------------------------------------

template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value >::type>
using vec3 = details::TVec3<T>;

using double3 = vec3<double>;
using float3 = vec3<float>;
using half3 = vec3<half>;
using int3 = vec3<int32_t>;
using uint3 = vec3<uint32_t>;
using short3 = vec3<int16_t>;
using ushort3 = vec3<uint16_t>;
using byte3 = vec3<int8_t>;
using ubyte3 = vec3<uint8_t>;
using bool3 = vec3<bool>;

// ----------------------------------------------------------------------------------------
}  // namespace math
}  // namespace filament

#endif  // MATH_VEC3_H_
