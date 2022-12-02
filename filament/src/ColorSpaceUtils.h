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

#ifndef TNT_FILAMENT_COLORSPACE_H
#define TNT_FILAMENT_COLORSPACE_H

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

constexpr mat3f Rec2020_to_XYZ{
     0.6369530f,  0.2626983f,  0.0000000f,
     0.1446169f,  0.6780088f,  0.0280731f,
     0.1688558f,  0.0592929f,  1.0608272f
};

constexpr mat3f XYZ_to_Rec2020{
    1.7166634f,  -0.6666738f,  0.0176425f,
    -0.3556733f,  1.6164557f, -0.0427770f,
    -0.2533681f,  0.0157683f,  0.9422433f
};

constexpr mat3f XYZ_to_CIECAT16{
     0.401288f, -0.250268f, -0.002079f,
     0.650173f,  1.204414f,  0.048952f,
    -0.051461f,  0.045854f,  0.953127f
};

constexpr mat3f CIECAT16_to_XYZ{
     1.862068f,  0.387527f, -0.015841f,
    -1.011255f,  0.621447f, -0.034123f,
     0.149187f, -0.008974f,  1.049964f
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

constexpr mat3f AP0_to_sRGB{
     2.52169f, -0.27648f, -0.01538f,
    -1.13413f,  1.37272f, -0.15298f,
    -0.38756f, -0.09624f,  1.16835f
};

constexpr mat3f sRGB_to_AP0{
     0.4397010f, 0.0897923f, 0.0175440f,
     0.3829780f, 0.8134230f, 0.1115440f,
     0.1773350f, 0.0967616f, 0.8707040f
};

constexpr mat3f sRGB_to_OkLab_LMS{
     0.4122214708f, 0.2119034982f, 0.0883024619f,
     0.5363325363f, 0.6806995451f, 0.2817188376f,
     0.0514459929f, 0.1073969566f, 0.6299787005f
};

constexpr mat3f XYZ_to_OkLab_LMS{
     0.8189330101f, 0.3618667424f, -0.1288597137f,
     0.0329845436f, 0.9293118715f,  0.0361456387f,
     0.0482003018f, 0.2643662691f,  0.6338517070f
};

constexpr mat3f OkLab_LMS_to_XYZ{
     1.227014f, -0.557800f,  0.281256f,
    -0.040580f,  1.112257f, -0.071677f,
    -0.076381f, -0.421482f,  1.586163f
};

constexpr mat3f OkLab_LMS_to_OkLab{
     0.2104542553f,  1.9779984951f,  0.0259040371f,
     0.7936177850f, -2.4285922050f,  0.7827717662f,
    -0.0040720468f,  0.4505937099f, -0.8086757660f
};

constexpr mat3f OkLab_to_OkLab_LMS{
     1.0000000000f,  1.0000000000f,  1.0000000000f,
     0.3963377774f, -0.1055613458f, -0.0894841775f,
     0.2158037573f, -0.0638541728f, -1.2914855480f
};

constexpr mat3f OkLab_LMS_to_sRGB{
     4.0767416621f, -1.2684380046f, -0.0041960863f,
    -3.3077115913f,  2.6097574011f, -0.7034186147f,
     0.2309699292f, -0.3413193965f,  1.7076147010f
};

constexpr mat3f sRGB_to_Rec2020 = XYZ_to_Rec2020 * sRGB_to_XYZ;

constexpr mat3f Rec2020_to_sRGB = XYZ_to_sRGB * Rec2020_to_XYZ;

constexpr mat3f sRGB_to_LMS_CAT16 = XYZ_to_CIECAT16 * sRGB_to_XYZ;

constexpr mat3f LMS_CAT16_to_sRGB = XYZ_to_sRGB * CIECAT16_to_XYZ;

constexpr mat3f Rec2020_to_LMS_CAT16 = XYZ_to_CIECAT16 * Rec2020_to_XYZ;

constexpr mat3f LMS_CAT16_to_Rec2020 = XYZ_to_Rec2020 * CIECAT16_to_XYZ;

constexpr mat3f Rec2020_to_AP0 = AP1_to_AP0 * XYZ_to_AP1 * Rec2020_to_XYZ;

constexpr mat3f AP1_to_Rec2020 = XYZ_to_Rec2020 * AP1_to_XYZ;

constexpr mat3f Rec2020_to_OkLab_LMS = XYZ_to_OkLab_LMS * Rec2020_to_XYZ;

constexpr mat3f OkLab_LMS_to_Rec2020 = XYZ_to_Rec2020 * OkLab_LMS_to_XYZ;

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------

// Standard CIE 1931 2° illuminant D65, in xyY space
constexpr float3 ILLUMINANT_D65_xyY{0.31271f, 0.32902f, 1.0f};

// Standard CIE 1931 2° illuminant D65, in LMS space (CIECAT16)
// Result of: XYZ_to_CIECAT16 * xyY_to_XYZ(ILLUMINANT_D65_xyY);
constexpr float3 ILLUMINANT_D65_LMS_CAT16{0.975533f, 1.016483f, 1.084837f};

// RGB to luminance coefficients for Rec.2020, from Rec2020_to_XYZ
constexpr float3 LUMINANCE_Rec2020{0.2627002f, 0.6779981f, 0.0593017f};

// RGB to luminance coefficients for ACEScg (AP1), from AP1_to_XYZ
constexpr float3 LUMINANCE_AP1{0.272229f, 0.674082f, 0.0536895f};

// RGB to luminance coefficients for Rec.709, from sRGB_to_XYZ
constexpr float3 LUMINANCE_Rec709{0.2126730f, 0.7151520f, 0.0721750f};

// RGB to luminance coefficients for Rec.709 with HK-like weighting
constexpr float3 LUMINANCE_HK_Rec709{0.13913043f, 0.73043478f, 0.13043478f};

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
    return {v.xy / max(v.x + v.y + v.z, 1e-5f), v.y};
}

inline constexpr float3 pow3(float3 x) noexcept {
    return x * x * x;
}

inline float3 sRGB_to_OkLab(float3 x) noexcept {
    return OkLab_LMS_to_OkLab * cbrt(sRGB_to_OkLab_LMS * x);
}

inline float3 Rec2020_to_OkLab(float3 x) noexcept {
    return OkLab_LMS_to_OkLab * cbrt(Rec2020_to_OkLab_LMS * x);
}

inline float3 OkLab_to_sRGB(float3 x) noexcept {
    return OkLab_LMS_to_sRGB * pow3(OkLab_to_OkLab_LMS * x);
}

inline float3 OkLab_to_Rec2020(float3 x) noexcept {
    return OkLab_LMS_to_Rec2020 * pow3(OkLab_to_OkLab_LMS * x);
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

inline float3 OETF_Linear(float3 x) noexcept {
    return x;
}

inline float3 OETF_sRGB(float3 x) noexcept {
    constexpr float a  = 0.055f;
    constexpr float a1 = 1.055f;
    constexpr float b  = 12.92f;
    constexpr float p  = 1 / 2.4f;
    for (size_t i = 0; i < 3; i++) {
        x[i] = x[i] <= 0.0031308f ? x[i] * b : a1 * pow(x[i], p) - a;
    }
    return x;
}

inline float3 EOTF_sRGB(float3 x) noexcept {
    constexpr float a  = 0.055f;
    constexpr float a1 = 1.055f;
    constexpr float b  = 1.0f / 12.92f;
    constexpr float p  = 2.4f;
    for (size_t i = 0; i < 3; i++) {
        x[i] = x[i] <= 0.04045f ? x[i] * b : pow((x[i] + a) / a1, p);
    }
    return x;
}

inline float3 OETF_PQ(float3 x, float maxPqValue) {
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

inline float3 EOTF_PQ(float3 x, float maxPqValue) {
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

//------------------------------------------------------------------------------
// Gamut mapping
//------------------------------------------------------------------------------

float3 gamutMapping_sRGB(float3 rgb) noexcept;

} // namespace filament

#endif // TNT_FILAMENT_COLORSPACE_H
