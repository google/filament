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

// Standard CIE 1931 2° illuminant D65, in xyY space
constexpr float3 ILLUMINANT_D65_xyY{0.31271f, 0.32902f, 1.0f};

// Standard CIE 1931 2° illuminant D65, in LMS space (CIECAT02)
// Result of: XYZ_to_CIECAT02 * xyY_to_XYZ(ILLUMINANT_D65_xyY);
constexpr float3 ILLUMINANT_D65_LMS{0.949237f, 1.03542f, 1.08728f};

constexpr mat3f sRGB_to_LMS = XYZ_to_CIECAT02 * sRGB_to_XYZ;

constexpr mat3f LMS_to_sRGB = XYZ_to_sRGB * CIECAT02_to_XYZ;

inline constexpr XYZ xyY_to_XYZ(xyY v) {
    return XYZ{v.x / v.y, v.z, (1.0f - v.x - v.y) / v.y};
}

// Returns the y chromaticity coordinate in xyY for an illuminant series D,
// given its x chromaticity coordinate.
inline constexpr float illuminantD_y(float x) {
    // See http://en.wikipedia.org/wiki/Standard_illuminant#Illuminant_series_D
    return 2.87f * x - 3.0f * x * x - 0.275f;
}

} // namespace filament

#endif //TNT_FILAMENT_COLOR_SPACE_H
