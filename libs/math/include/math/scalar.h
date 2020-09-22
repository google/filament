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
#include <assert.h>

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
constexpr const double F_TAU      = 2.0 * F_PI;

namespace d {
constexpr const double E                = F_E;
constexpr const double LOG2E            = F_LOG2E;
constexpr const double LOG10E           = F_LOG10E;
constexpr const double LN2              = F_LN2;
constexpr const double LN10             = F_LN10;
constexpr const double PI               = F_PI;
constexpr const double PI_2             = F_PI_2;
constexpr const double PI_4             = F_PI_4;
constexpr const double ONE_OVER_PI      = F_1_PI;
constexpr const double TWO_OVER_PI      = F_2_PI;
constexpr const double TWO_OVER_SQRTPI  = F_2_SQRTPI;
constexpr const double SQRT2            = F_SQRT2;
constexpr const double SQRT1_2          = F_SQRT1_2;
constexpr const double TAU              = F_TAU;
constexpr const double DEG_TO_RAD       = F_PI / 180.0;
constexpr const double RAD_TO_DEG       = 180.0 / F_PI;
} // namespace d

namespace f {
constexpr const float E                = (float)d::E;
constexpr const float LOG2E            = (float)d::LOG2E;
constexpr const float LOG10E           = (float)d::LOG10E;
constexpr const float LN2              = (float)d::LN2;
constexpr const float LN10             = (float)d::LN10;
constexpr const float PI               = (float)d::PI;
constexpr const float PI_2             = (float)d::PI_2;
constexpr const float PI_4             = (float)d::PI_4;
constexpr const float ONE_OVER_PI      = (float)d::ONE_OVER_PI;
constexpr const float TWO_OVER_PI      = (float)d::TWO_OVER_PI;
constexpr const float TWO_OVER_SQRTPI  = (float)d::TWO_OVER_SQRTPI;
constexpr const float SQRT2            = (float)d::SQRT2;
constexpr const float SQRT1_2          = (float)d::SQRT1_2;
constexpr const float TAU              = (float)d::TAU;
constexpr const float DEG_TO_RAD       = (float)d::DEG_TO_RAD;
constexpr const float RAD_TO_DEG       = (float)d::RAD_TO_DEG;
} // namespace f

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
    assert(min <= max);
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
