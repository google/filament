/*
 * Copyright (C) 2015 The Android Open Source Project
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

#ifndef IBL_UTILITIES_H
#define IBL_UTILITIES_H

#include <math.h>

#include <math/vec2.h>
#include <math/vec3.h>

namespace filament {
namespace ibl {

template<typename T>
static inline constexpr T sq(T x) {
    return x * x;
}

template<typename T>
static inline constexpr T log4(T x) {
    // log2(x)/log2(4)
    // log2(x)/2
    return std::log2(x) * T(0.5);
}

inline bool isPOT(size_t x) {
    return !(x & (x - 1));
}

inline filament::math::double2 hammersley(uint32_t i, float iN) {
    constexpr float tof = 0.5f / 0x80000000U;
    uint32_t bits = i;
    bits = (bits << 16) | (bits >> 16);
    bits = ((bits & 0x55555555) << 1) | ((bits & 0xAAAAAAAA) >> 1);
    bits = ((bits & 0x33333333) << 2) | ((bits & 0xCCCCCCCC) >> 2);
    bits = ((bits & 0x0F0F0F0F) << 4) | ((bits & 0xF0F0F0F0) >> 4);
    bits = ((bits & 0x00FF00FF) << 8) | ((bits & 0xFF00FF00) >> 8);
    return { i * iN, bits * tof };
}

} // namespace ibl
} // namespace filament
#endif /* IBL_UTILITIES_H */
