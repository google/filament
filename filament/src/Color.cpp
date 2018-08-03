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

#include <filament/Color.h>

#include <math/mat3.h>

#include <algorithm>

using namespace math;

namespace filament {

using xyY = math::float3;
using XYZ = math::float3;

float3 Color::sRGBToLinear(float3 color) noexcept {
    float3 linearColor{color};
    #pragma nounroll
    for (size_t i = 0; i < linearColor.size(); i++) {
        linearColor[i] = (linearColor[i] <= 0.04045f) ?
                linearColor[i] / 12.92f : powf((linearColor[i] + 0.055f) / 1.055f, 2.4f);
    }
    return linearColor;
}

float3 Color::linearToSRGB(float3 color) noexcept {
    float3 sRGBColor{color};
    #pragma nounroll
    for (size_t i = 0; i < sRGBColor.size(); i++) {
        sRGBColor[i] = (sRGBColor[i] <= 0.0031308f) ?
                sRGBColor[i] * 12.92f : (powf(sRGBColor[i], 1.0f / 2.4f) * 1.055f) - 0.055f;
    }
    return sRGBColor;
}

static inline constexpr XYZ xyY_to_XYZ(xyY const& v) {
    return XYZ{v.x / v.y, v.z, (1.0f - v.x - v.y) / v.y};
}

static inline LinearColor XYZ_to_sRGB(XYZ const& v) {
    // XYZ to linear sRGB
    const mat3f XYZ_sRGB{
           3.2404542f, -0.9692660f,  0.0556434f,
          -1.5371385f,  1.8760108f, -0.2040259f,
          -0.4985314f,  0.0415560f,  1.0572252f
    };
    return XYZ_sRGB * v;
}

LinearColor Color::cct(float K) {
    // temperature to CIE 1960
    float K2 = K * K;
    float u = (0.860117757f + 1.54118254e-4f * K + 1.28641212e-7f * K2) /
               (1.0f + 8.42420235e-4f * K + 7.08145163e-7f * K2);
    float v = (0.317398726f + 4.22806245e-5f * K + 4.20481691e-8f * K2) /
               (1.0f - 2.89741816e-5f * K + 1.61456053e-7f * K2);

    float d = 1.0f / (2.0f * u - 8.0f * v + 4.0f);
    float3 linear = XYZ_to_sRGB(xyY_to_XYZ({3.0f * u * d, 2.0f * v * d, 1.0f}));
    // normalize and saturate
    return saturate(linear / std::max(1e-5f, max(linear)));
}

LinearColor Color::illuminantD(float K) {
    // temperature to xyY
    const float iK = 1.0f / K;
    float iK2 = iK * iK;
    float x = K <= 7000.0f ?
            0.244063f + 0.09911e3f * iK + 2.9678e6f * iK2 - 4.6070e9f * iK2 * iK :
            0.237040f + 0.24748e3f * iK + 1.9018e6f * iK2 - 2.0064e9f * iK2 * iK;
    float y = -3.0f * x * x + 2.87f * x - 0.275f;

    float3 linear = XYZ_to_sRGB(xyY_to_XYZ({x, y, 1.0f}));
    // normalize and saturate
    return saturate(linear / std::max(1e-5f, max(linear)));
}

} // namespace filament
