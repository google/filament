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

#ifndef TNT_MATH_NORM_H
#define TNT_MATH_NORM_H

#include <math/scalar.h>
#include <math/vec4.h>

#include <cmath>
#include <stdint.h>

namespace filament {
namespace math {

inline uint16_t packUnorm16(float v) noexcept {
    return static_cast<uint16_t>(std::round(clamp(v, 0.0f, 1.0f) * 65535.0f));
}

inline ushort4 packUnorm16(float4 v) noexcept {
    return ushort4{ packUnorm16(v.x), packUnorm16(v.y), packUnorm16(v.z), packUnorm16(v.w) };
}

inline int16_t packSnorm16(float v) noexcept {
    return static_cast<int16_t>(std::round(clamp(v, -1.0f, 1.0f) * 32767.0f));
}

inline short2 packSnorm16(float2 v) noexcept {
    return short2{ packSnorm16(v.x), packSnorm16(v.y) };
}

inline short4 packSnorm16(float4 v) noexcept {
    return short4{ packSnorm16(v.x), packSnorm16(v.y), packSnorm16(v.z), packSnorm16(v.w) };
}

inline float unpackUnorm16(uint16_t v) noexcept {
    return v / 65535.0f;
}

inline float4 unpackUnorm16(ushort4 v) noexcept {
    return float4{ unpackUnorm16(v.x), unpackUnorm16(v.y), unpackUnorm16(v.z), unpackUnorm16(v.w) };
}

inline float unpackSnorm16(int16_t v) noexcept {
    return clamp(v / 32767.0f, -1.0f, 1.0f);
}

inline float4 unpackSnorm16(short4 v) noexcept {
    return float4{ unpackSnorm16(v.x), unpackSnorm16(v.y), unpackSnorm16(v.z), unpackSnorm16(v.w) };
}

inline uint8_t packUnorm8(float v) noexcept {
    return static_cast<uint8_t>(std::round(clamp(v, 0.0f, 1.0f) * 255.0));
}

inline ubyte4 packUnorm8(float4 v) noexcept {
    return ubyte4{ packUnorm8(v.x), packUnorm8(v.y), packUnorm8(v.z), packUnorm8(v.w) };
}

inline int8_t packSnorm8(float v) noexcept {
    return static_cast<int8_t>(std::round(clamp(v, -1.0f, 1.0f) * 127.0));
}

inline byte4 packSnorm8(float4 v) noexcept {
    return byte4{ packSnorm8(v.x), packSnorm8(v.y), packSnorm8(v.z), packSnorm8(v.w) };
}

inline float unpackUnorm8(uint8_t v) noexcept {
    return v / 255.0f;
}

inline float4 unpackUnorm8(ubyte4 v) noexcept {
    return float4{ unpackUnorm8(v.x), unpackUnorm8(v.y), unpackUnorm8(v.z), unpackUnorm8(v.w) };
}

inline float unpackSnorm8(int8_t v) noexcept {
    return clamp(v / 127.0f, -1.0f, 1.0f);
}

inline float4 unpackSnorm8(byte4 v) noexcept {
    return float4{ unpackSnorm8(v.x), unpackSnorm8(v.y), unpackSnorm8(v.z), unpackSnorm8(v.w) };
}

} // namespace math
} // namespace filament

#endif // TNT_MATH_NORM_H
