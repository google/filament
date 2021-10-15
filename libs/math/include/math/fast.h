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

#ifndef TNT_MATH_FAST_H
#define TNT_MATH_FAST_H

#include <math/compiler.h>
#include <math/scalar.h>

#include <cmath>
#include <type_traits>

#include <stdint.h>

#ifdef __ARM_NEON
#include <arm_neon.h>
#endif

namespace filament {
namespace math {
namespace fast {

// fast cos(x), ~8 cycles (vs. 66 cycles on ARM)
// can be vectorized
// x between -pi and pi
template<typename T, typename = std::enable_if_t<std::is_floating_point<T>::value>>
constexpr T MATH_PURE cos(T x) noexcept {
    x *= T(F_1_PI / 2);
    x -= T(0.25) + std::floor(x + T(0.25));
    x *= T(16.0) * std::abs(x) - T(8.0);
    x += T(0.225) * x * (std::abs(x) - T(1.0));
    return x;
}

// fast sin(x), ~8 cycles (vs. 66 cycles on ARM)
// can be vectorized
// x between -pi and pi
template<typename T, typename = std::enable_if_t<std::is_floating_point<T>::value>>
constexpr T MATH_PURE sin(T x) noexcept {
    return filament::math::fast::cos<T>(x - T(F_PI_2));
}

constexpr inline float MATH_PURE ilog2(float x) noexcept {
    union {
        float val;
        int32_t x;
    } u = { x };
    return float(((u.x >> 23) & 0xff) - 127);
}

constexpr inline float MATH_PURE log2(float x) noexcept {
    union {
        float val;
        int32_t x;
    } u = { x };
    float ilog2 = float(((u.x >> 23) & 0xff) - 128);
    u.x = (u.x & 0x007fffff) | 0x3f800000;
    return ilog2 + (-0.34484843f * u.val + 2.02466578f) * u.val - 0.67487759f;
}

// fast 1/sqrt(), on ARMv8 this is 5 cycles vs. 7 cycles, so maybe not worth it.
// we keep this mostly for reference and benchmarking.
inline float MATH_PURE isqrt(float x) noexcept {
#if defined(__ARM_NEON) && defined(__aarch64__)
    float y = vrsqrtes_f32(x);
    return y * vrsqrtss_f32(x, y * y);
#else
    return 1 / std::sqrt(x);
#endif
}

inline double MATH_PURE isqrt(double x) noexcept {
#if defined(__ARM_NEON) && defined(__aarch64__)
    double y = vrsqrted_f64(x);
    return y * vrsqrtsd_f64(x, y * y);
#else
    return 1 / std::sqrt(x);
#endif
}

inline int signbit(float x) noexcept {
#if __has_builtin(__builtin_signbitf)
    // Note: on Android NDK, signbit() is a function call -- not what we want.
    return __builtin_signbitf(x);
#else
    return std::signbit(x);
#endif
}

/*
 * constexpr exp(), pow(), factorial()
 */

constexpr double pow(double x, unsigned int y) noexcept {
    return y == 0 ? 1.0 : x * pow(x, y - 1);
}

constexpr unsigned int factorial(unsigned int x) noexcept {
    return x == 0 ? 1 : x * factorial(x - 1);
}

constexpr double exp(double x) noexcept {
    return 1.0 + x + pow(x, 2) / factorial(2) + pow(x, 3) / factorial(3)
           + pow(x, 4) / factorial(4) + pow(x, 5) / factorial(5)
           + pow(x, 6) / factorial(6) + pow(x, 7) / factorial(7)
           + pow(x, 8) / factorial(8) + pow(x, 9) / factorial(9);
}

constexpr float exp(float x) noexcept {
    return float(exp(double(x)));
}

/*
 * unsigned saturated arithmetic
 */

#if defined(__ARM_NEON) && defined(__aarch64__)
inline uint8_t  MATH_PURE qadd(uint8_t a,  uint8_t b)  noexcept { return vuqaddb_s8(a, b);  }
inline uint16_t MATH_PURE qadd(uint16_t a, uint16_t b) noexcept { return vuqaddh_s16(a, b); }
inline uint32_t MATH_PURE qadd(uint32_t a, uint32_t b) noexcept { return vuqadds_s32(a, b); }

inline uint8_t  MATH_PURE qsub(uint8_t a,  uint8_t b)  noexcept { return vqsubb_s8(a, b);  }
inline uint16_t MATH_PURE qsub(uint16_t a, uint16_t b) noexcept { return vqsubh_s16(a, b); }
inline uint32_t MATH_PURE qsub(uint32_t a, uint32_t b) noexcept { return vqsubs_s32(a, b); }
#else

template<typename T, typename = std::enable_if_t<
        std::is_same<uint8_t, T>::value ||
        std::is_same<uint16_t, T>::value ||
        std::is_same<uint32_t, T>::value>>
inline T MATH_PURE qadd(T a, T b) noexcept {
    T r = a + b;
    return r | -T(r < a);
}

template<typename T, typename = std::enable_if_t<
        std::is_same<uint8_t, T>::value ||
        std::is_same<uint16_t, T>::value ||
        std::is_same<uint32_t, T>::value>>
inline T MATH_PURE qsub(T a, T b) noexcept {
    T r = a - b;
    return r & -T(r <= a);
}

#endif

template<typename T>
inline T MATH_PURE qinc(T a) noexcept {
    return qadd(a, T(1));
}

template<typename T>
inline T MATH_PURE qdec(T a) noexcept {
    return qsub(a, T(1));
}


} // namespace fast
} // namespace math
} // namespace filament

#endif // TNT_MATH_FAST_H
