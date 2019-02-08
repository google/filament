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

#ifndef MATH_VEC4_H_
#define MATH_VEC4_H_

#include <math/vec3.h>
#include <math/half.h>
#include <stdint.h>
#include <sys/types.h>


namespace filament {
namespace math {
// -------------------------------------------------------------------------------------

namespace details {

template <typename T>
class MATH_EMPTY_BASES TVec4 :  public TVecProductOperators<TVec4, T>,
                public TVecAddOperators<TVec4, T>,
                public TVecUnaryOperators<TVec4, T>,
                public TVecComparisonOperators<TVec4, T>,
                public TVecFunctions<TVec4, T>,
                public TVecDebug<TVec4, T> {
public:
    typedef T value_type;
    typedef T& reference;
    typedef T const& const_reference;
    typedef size_t size_type;
    static constexpr size_t SIZE = 4;

    union {
        T v[SIZE];
        TVec2<T> xy, st, rg;
        TVec3<T> xyz, stp, rgb;
        struct {
            union { T x, s, r; };
            union {
                TVec2<T> yz, tp, gb;
                TVec3<T> yzw, tpq, gba;
                struct {
                    union { T y, t, g; };
                    union {
                        TVec2<T> zw, pq, ba;
                        struct { T z, w; };
                        struct { T p, q; };
                        struct { T b, a; };
                    };
                };
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
    constexpr TVec4() = default;

    // handles implicit conversion to a tvec4. must not be explicit.
    template<typename A, typename = typename std::enable_if<std::is_arithmetic<A>::value >::type>
    constexpr TVec4(A v) : x(v), y(v), z(v), w(v) { }

    template<typename A, typename B, typename C, typename D>
    constexpr TVec4(A x, B y, C z, D w) : x(x), y(y), z(z), w(w) { }

    template<typename A, typename B, typename C>
    constexpr TVec4(const TVec2<A>& v, B z, C w) : x(v.x), y(v.y), z(z), w(w) { }

    template<typename A, typename B>
    constexpr TVec4(const TVec3<A>& v, B w) : x(v.x), y(v.y), z(v.z), w(w) { }

    template<typename A>
    explicit
    constexpr TVec4(const TVec4<A>& v) : x(v.x), y(v.y), z(v.z), w(v.w) { }
};

}  // namespace details

// ----------------------------------------------------------------------------------------

typedef details::TVec4<double> double4;
typedef details::TVec4<float> float4;
typedef details::TVec4<half> half4;
typedef details::TVec4<int32_t> int4;
typedef details::TVec4<uint32_t> uint4;
typedef details::TVec4<int16_t> short4;
typedef details::TVec4<uint16_t> ushort4;
typedef details::TVec4<int8_t> byte4;
typedef details::TVec4<uint8_t> ubyte4;
typedef details::TVec4<bool> bool4;


// ----------------------------------------------------------------------------------------
}  // namespace math
}  // namespace filament

#endif  // MATH_VEC4_H_
