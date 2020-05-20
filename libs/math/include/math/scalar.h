/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef TNT_MATH_SCALAR_H
#define TNT_MATH_SCALAR_H

#include <math/compiler.h>

namespace filament {
namespace math {

constexpr const double F_E        = 2.71828182845904523536028747135266250;
constexpr const double F_LOG2E    = 1.44269504088896340735992468100189214;
constexpr const double F_LOG10E   = 0.434294481903251827651128918916605082;
constexpr const double F_LN2      = 0.693147180559945309417232121458176568;
constexpr const double F_LN10     = 2.30258509299404568401799145468436421;
constexpr const double F_PI       = 3.14159265358979323846264338327950288;
constexpr const double F_PI_2     = 1.57079632679489661923132169163975144;
constexpr const double F_PI_4     = 0.785398163397448309615660845819875721;
constexpr const double F_1_PI     = 0.318309886183790671537767526745028724;
constexpr const double F_2_PI     = 0.636619772367581343075535053490057448;
constexpr const double F_2_SQRTPI = 1.12837916709551257389615890312154517;
constexpr const double F_SQRT2    = 1.41421356237309504880168872420969808;
constexpr const double F_SQRT1_2  = 0.707106781186547524400844362104849039;

template<typename T>
inline constexpr T MATH_PURE min(T a, T b) noexcept {
    return a < b ? a : b;
}

template<typename T>
inline constexpr T MATH_PURE max(T a, T b) noexcept {
    return a > b ? a : b;
}

template<typename T>
inline constexpr T MATH_PURE clamp(T v, T min, T max) noexcept {
    return T(math::min(max, math::max(min, v)));
}

template<typename T>
inline constexpr T MATH_PURE saturate(T v) noexcept {
    return clamp(v, T(0), T(1));
}

template<typename T>
inline constexpr T MATH_PURE mix(T x, T y, T a) noexcept {
    return x * (T(1) - a) + y * a;
}

template<typename T>
inline constexpr T MATH_PURE lerp(T x, T y, T a) noexcept {
    return mix(x, y, a);
}

template<typename T>
inline constexpr T MATH_PURE smoothstep(T e0, T e1, T x) noexcept {
    T t = clamp((x - e0) / (e1 - e0), T(0), T(1));
    return t * t * (T(3) - T(2) * t);
}

template<typename T>
inline constexpr T sign(T x) noexcept {
    return x < T(0) ? T(-1) : T(1);
}

} // namespace math
} // namespace filament

#endif // TNT_MATH_SCALAR_H
