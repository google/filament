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

namespace filament {

using namespace math;

using xyY = float3;
using XYZ = float3;

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

// Standard CIE 1931 2° illuminant D65, in xyY space
constexpr float3 ILLUMINANT_D65_xyY{0.31271f, 0.32902f, 1.0f};

// Standard CIE 1931 2° illuminant D65, in LMS space (CIECAT02)
// Result of: XYZ_to_CIECAT02 * xyY_to_XYZ(ILLUMINANT_D65_xyY);
constexpr float3 ILLUMINANT_D65_LMS{0.949237f, 1.03542f, 1.08728f};

constexpr mat3f sRGB_to_LMS = XYZ_to_CIECAT02 * sRGB_to_XYZ;

constexpr mat3f LMS_to_sRGB = XYZ_to_sRGB * CIECAT02_to_XYZ;

inline constexpr XYZ xyY_to_XYZ(xyY v) {
    const float a = v.z / max(v.y, 1e-5f);
    return XYZ{v.x * a, v.z, (1.0f - v.x - v.y) * a};
}

inline constexpr xyY XYZ_to_xyY(XYZ v) {
    return float3(v.xy / max(v.x + v.y + v.z, 1e-5f), v.y);
}

// Returns the y chromaticity coordinate in xyY for an illuminant series D,
// given its x chromaticity coordinate.
inline constexpr float illuminantD_y(float x) {
    // See http://en.wikipedia.org/wiki/Standard_illuminant#Illuminant_series_D
    return 2.87f * x - 3.0f * x * x - 0.275f;
}

} // namespace filament

#endif //TNT_FILAMENT_COLOR_SPACE_H
