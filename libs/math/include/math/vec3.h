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
    template<typename A, typename = typename std::enable_if<std::is_arithmetic<A>::value >::type>
    constexpr TVec3(A v) : x(v), y(v), z(v) { }

    template<typename A, typename B, typename C>
    constexpr TVec3(A x, B y, C z) : x(x), y(y), z(z) { }

    template<typename A, typename B>
    constexpr TVec3(const TVec2<A>& v, B z) : x(v.x), y(v.y), z(z) { }

    template<typename A>
    explicit
    constexpr TVec3(const TVec3<A>& v) : x(v.x), y(v.y), z(v.z) { }

    // cross product works only on vectors of size 3
    template <typename RT>
    friend inline
    constexpr TVec3 cross(const TVec3& u, const TVec3<RT>& v) {
        return TVec3(
                u.y*v.z - u.z*v.y,
                u.z*v.x - u.x*v.z,
                u.x*v.y - u.y*v.x);
    }
};

}  // namespace details

// ----------------------------------------------------------------------------------------

typedef details::TVec3<double> double3;
typedef details::TVec3<float> float3;
typedef details::TVec3<half> half3;
typedef details::TVec3<int32_t> int3;
typedef details::TVec3<uint32_t> uint3;
typedef details::TVec3<int16_t> short3;
typedef details::TVec3<uint16_t> ushort3;
typedef details::TVec3<int8_t> byte3;
typedef details::TVec3<uint8_t> ubyte3;
typedef details::TVec3<bool> bool3;

// ----------------------------------------------------------------------------------------
}  // namespace math
}  // namespace filament

#endif  // MATH_VEC3_H_
