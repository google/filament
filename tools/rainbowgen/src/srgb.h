/*
 * Copyright (C) 2024 The Android Open Source Project
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

#pragma once

#include <math/mat3.h>
#include <math/vec3.h>

#include <cmath>

namespace srgb {

using namespace filament::math;

inline constexpr float3 XYZ_to_sRGB(float3 const v) noexcept {
    constexpr mat3f const XYZ_sRGB{
            3.2404542f, -0.9692660f,  0.0556434f,
            -1.5371385f,  1.8760108f, -0.2040259f,
            -0.4985314f,  0.0415560f,  1.0572252f
    };
    return XYZ_sRGB * v;
}

inline float3 linear_to_sRGB(float3 c) noexcept {
    for (auto i = 0; i < c.size(); i++) {
        c[i] = (c[i] <= 0.0031308f) ?
                c[i] * 12.92f : (std::pow(c[i], 1.0f / 2.4f) * 1.055f) - 0.055f;
    }
    return c;
}

inline float3 sRGB_to_linear(float3 c) noexcept {
    for (auto i = 0; i < c.size(); i++) {
        c[i] = (c[i] <= 0.04045f) ?
                c[i] / 12.92f : std::pow((c[i] + 0.055f) / 1.055f, 2.4f);
    }
    return c;
}

} // namespace rainbow