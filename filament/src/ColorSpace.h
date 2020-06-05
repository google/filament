/*
 * Copyright (C) 2020 The Android Open Source Project
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

#ifndef TNT_FILAMENT_COLOR_SPACE_H
#define TNT_FILAMENT_COLOR_SPACE_H

#include <utils/compiler.h>

#include <math/mat3.h>
#include <math/vec3.h>
#include <math/scalar.h>

#include <cmath>

namespace filament {

using namespace math;

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------

using xyY = float3;
using XYZ = float3;

//------------------------------------------------------------------------------
// Color space conversion matrices
//------------------------------------------------------------------------------

constexpr mat3f XYZ_to_sRGB{
     3.2404542f, -0.9692660f,  0.0556434f,
    -1.5371385f,  1.8760108f, -0.2040259f,
    -0.4985314f,  0.0415560f,  1.0572252f
};

constexpr mat3f sRGB_to_XYZ{
     0.4124560f,  0.2126730f,  0.0193339f,
     0.3575760f,  0.7151520f,  0.1191920f,
     0.1804380f,  0.0721750f,  0.9503040f
};

constexpr mat3f XYZ_to_CIECAT02{
     0.7328000f, -0.7036000f,  0.0030000f,
     0.4296000f,  1.6975000f,  0.0136000f,
    -0.1624000f,  0.0061000f,  0.9834000f
};

constexpr mat3f CIECAT02_to_XYZ{
     1.0961200f,  0.4543690f, -0.0096276f,
    -0.2788690f,  0.4735330f, -0.0056980f,
     0.1827450f,  0.0720978f,  1.0153300f
};

constexpr mat3f AP1_to_XYZ{
     0.6624541811f,  0.2722287168f, -0.0055746495f,
     0.1340042065f,  0.6740817658f,  0.0040607335f,
     0.1561876870f,  0.0536895174f,  1.0103391003f
};

constexpr mat3f XYZ_to_AP1{
     1.6410233797f, -0.6636628587f,  0.0117218943f,
    -0.3248032942f,  1.6153315917f, -0.0082844420f,
    -0.2364246952f,  0.0167563477f,  0.9883948585f
};

constexpr mat3f AP1_to_AP0{
     0.6954522414f,  0.0447945634f, -0.0055258826f,
     0.1406786965f,  0.8596711185f,  0.0040252103f,
     0.1638690622f,  0.0955343182f,  1.0015006723f
};

constexpr mat3f AP0_to_AP1{
     1.4514393161f, -0.0765537734f,  0.0083161484f,
    -0.2365107469f,  1.1762296998f, -0.0060324498f,
    -0.2149285693f, -0.0996759264f,  0.9977163014f
};

constexpr mat3f AP1_to_sRGB{
     1.70505f, -0.13026f, -0.02400f,
    -0.62179f,  1.14080f, -0.12897f,
    -0.08326f, -0.01055f,  1.15297f
};

constexpr mat3f sRGB_to_AP1{
     0.61319f,  0.07021f,  0.02062f,
     0.33951f,  0.91634f,  0.10957f,
     0.04737f,  0.01345f,  0.86961f
};

constexpr mat3f sRGB_to_LMS = XYZ_to_CIECAT02 * sRGB_to_XYZ;

constexpr mat3f LMS_to_sRGB = XYZ_to_sRGB * CIECAT02_to_XYZ;

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------

// Standard CIE 1931 2° illuminant D65, in xyY space
constexpr float3 ILLUMINANT_D65_xyY{0.31271f, 0.32902f, 1.0f};

// Standard CIE 1931 2° illuminant D65, in LMS space (CIECAT02)
// Result of: XYZ_to_CIECAT02 * xyY_to_XYZ(ILLUMINANT_D65_xyY);
constexpr float3 ILLUMINANT_D65_LMS{0.949237f, 1.03542f, 1.08728f};

// RGB to luma coefficients for ACEScg (AP1), from AP1_to_XYZ
constexpr float3 LUMA_AP1{0.272229f, 0.674082f, 0.0536895f};

// RGB to luma coefficients for Rec.709, from sRGB_to_XYZ
constexpr float3 LUMA_REC709{0.2126730f, 0.7151520f, 0.0721750f};

constexpr float MIDDLE_GRAY_ACEScg = 0.18f;

constexpr float MIDDLE_GRAY_ACEScct = 0.4135884f;

//------------------------------------------------------------------------------
// Chromaticity helpers
//------------------------------------------------------------------------------

// Returns the y chromaticity coordinate in xyY for an illuminant series D,
// given its x chromaticity coordinate.
inline constexpr float chromaticityCoordinateIlluminantD(float x) noexcept {
    // See http://en.wikipedia.org/wiki/Standard_illuminant#Illuminant_series_D
    return 2.87f * x - 3.0f * x * x - 0.275f;
}

//------------------------------------------------------------------------------
// Color space conversions
//------------------------------------------------------------------------------

inline constexpr XYZ xyY_to_XYZ(xyY v) noexcept {
    const float a = v.z / max(v.y, 1e-5f);
    return XYZ{v.x * a, v.z, (1.0f - v.x - v.y) * a};
}

inline constexpr xyY XYZ_to_xyY(XYZ v) noexcept {
    return float3(v.xy / max(v.x + v.y + v.z, 1e-5f), v.y);
}

//------------------------------------------------------------------------------
// Conversion functions and encoding/decoding
//------------------------------------------------------------------------------

// Decodes a linear value from LogC using the Alexa LogC EI 1000 curve
inline float3 LogC_to_linear(float3 x) noexcept {
    const float ia = 1.0f / 5.555556f;
    const float b  = 0.047996f;
    const float ic = 1.0f / 0.244161f;
    const float d  = 0.386036f;
    return (pow(10.0f, (x - d) * ic) - b) * ia;
}

// Encodes a linear value in LogC using the Alexa LogC EI 1000 curve
inline float3 linear_to_LogC(float3 x) noexcept {
    const float a = 5.555556f;
    const float b = 0.047996f;
    const float c = 0.244161f;
    const float d = 0.386036f;
    return c * log10(a * x + b) + d;
}

inline float3 ACEScct_to_linearAP1(float3 x) noexcept {
    constexpr float l = 1.467996312f; // (log2(65504) + 9.72) / 17.52
    for (size_t i = 0; i < 3; i++) {
        if (x[i] <= 0.155251141552511f) {
            x[i] = (x[i] - 0.0729055341958355f) / 10.5402377416545f;
        } else if (x[i] < l) {
            x[i] = pow(2.0f, x[i] * 17.52f - 9.72f);
        } else {
            x[i] = 65504.0f;
        }
    }
    return x;
}

inline float3 linearAP1_to_ACEScct(float3 x) noexcept {
    for (size_t i = 0; i < 3; i++) {
        x[i] = x[i] < 0.0078125f
                ? 10.5402377416545f * x[i] + 0.0729055341958355f
                : (log2(x[i]) + 9.72f) / 17.52f;
    }
    return x;
}

inline float3 OECF_sRGB(float3 x) noexcept {
    constexpr float a  = 0.055f;
    constexpr float a1 = 1.055f;
    constexpr float b  = 12.92f;
    constexpr float p  = 1 / 2.4f;
    for (size_t i = 0; i < 3; i++) {
        x[i] = x[i] <= 0.0031308f ? x[i] * b : a1 * pow(x[i], p) - a;
    }
    return x;
}

inline float3 EOCF_sRGB(float3 x) noexcept {
    constexpr float a  = 0.055f;
    constexpr float a1 = 1.055f;
    constexpr float b  = 1.0f / 12.92f;
    constexpr float p  = 2.4f;
    for (size_t i = 0; i < 3; i++) {
        x[i] = x[i] <= 0.04045f ? x[i] * b : pow((x[i] + a) / a1, p);
    }
    return x;
}

inline float3 OECF_PQ(float3 x, float maxPqValue) {
    constexpr float PQ_constant_N  = 2610.0f / 4096.0f /   4.0f;
    constexpr float PQ_constant_M  = 2523.0f / 4096.0f * 128.0f;
    constexpr float PQ_constant_C1 = 3424.0f / 4096.0f;
    constexpr float PQ_constant_C2 = 2413.0f / 4096.0f *  32.0f;
    constexpr float PQ_constant_C3 = 2392.0f / 4096.0f *  32.0f;

    x /= maxPqValue;

    float3 g = pow(x, PQ_constant_N);
    float3 numerator = PQ_constant_C1 + PQ_constant_C2 * g;
    float3 denominator = 1.0f + PQ_constant_C3 * g;
    return pow(numerator / denominator, PQ_constant_M);
}

inline float3 EOCF_PQ(float3 x, float maxPqValue) {
    constexpr float PQ_constant_N  = 2610.0f / 4096.0f /   4.0f;
    constexpr float PQ_constant_M  = 2523.0f / 4096.0f * 128.0f;
    constexpr float PQ_constant_C1 = 3424.0f / 4096.0f;
    constexpr float PQ_constant_C2 = 2413.0f / 4096.0f *  32.0f;
    constexpr float PQ_constant_C3 = 2392.0f / 4096.0f *  32.0f;

    float3 g = pow(x, 1.0f / PQ_constant_M);
    float3 numerator = max(g - PQ_constant_C1, 0.0f);
    float3 denominator = PQ_constant_C2 - (PQ_constant_C3 * g);
    float3 linearColor = pow(numerator / denominator, 1.0f / PQ_constant_N);

    return linearColor * maxPqValue;
}

} // namespace filament

#endif //TNT_FILAMENT_COLOR_SPACE_H
