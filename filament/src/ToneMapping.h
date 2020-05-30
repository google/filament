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

#ifndef TNT_FILAMENT_TONE_MAPPING_H
#define TNT_FILAMENT_TONE_MAPPING_H

#include <utils/compiler.h>

#include <math/mat3.h>
#include <math/vec3.h>
#include <math/scalar.h>

namespace filament {

using namespace math;

//------------------------------------------------------------------------------
// ACES operations, from https://github.com/ampas/aces-dev
//------------------------------------------------------------------------------

namespace aces {

inline float rgb_2_saturation(float3 rgb) {
    // Input:  ACES
    // Output: OCES
    const float TINY = 1e-5f;
    float mi = min(rgb);
    float ma = max(rgb);
    return (max(ma, TINY) - max(mi, TINY)) / max(ma, 1e-2f);
}

inline float rgb_2_yc(float3 rgb) {
    const float ycRadiusWeight = 1.75f;

    // Converts RGB to a luminance proxy, here called YC
    // YC is ~ Y + K * Chroma
    // Constant YC is a cone-shaped surface in RGB space, with the tip on the
    // neutral axis, towards white.
    // YC is normalized: RGB 1 1 1 maps to YC = 1
    //
    // ycRadiusWeight defaults to 1.75, although can be overridden in function
    // call to rgb_2_yc
    // ycRadiusWeight = 1 -> YC for pure cyan, magenta, yellow == YC for neutral
    // of same value
    // ycRadiusWeight = 2 -> YC for pure red, green, blue  == YC for  neutral of
    // same value.

    float r = rgb.r;
    float g = rgb.g;
    float b = rgb.b;

    float chroma = std::sqrt(b * (b - g) + g * (g - r) + r * (r - b));

    return (b + g + r + ycRadiusWeight * chroma) / 3.0f;
}

inline float sigmoid_shaper(float x) {
    // Sigmoid function in the range 0 to 1 spanning -2 to +2.
    float t = max(1.0f - std::abs(x / 2.0f), 0.0f);
    float y = 1.0f + sign(x) * (1.0f - t * t);
    return y / 2.0f;
}

inline float glow_fwd(float ycIn, float glowGainIn, float glowMid) {
    float glowGainOut;

    if (ycIn <= 2.0f / 3.0f * glowMid) {
        glowGainOut = glowGainIn;
    } else if ( ycIn >= 2.0f * glowMid) {
        glowGainOut = 0.0f;
    } else {
        glowGainOut = glowGainIn * (glowMid / ycIn - 1.0f / 2.0f);
    }

    return glowGainOut;
}

inline float rgb_2_hue(float3 rgb) {
    // Returns a geometric hue angle in degrees (0-360) based on RGB values.
    // For neutral colors, hue is undefined and the function will return a quiet NaN value.
    float hue = 0.0f;
    // RGB triplets where RGB are equal have an undefined hue
    if (!(rgb.x == rgb.y && rgb.y == rgb.z)) {
        hue = (180.0f / float(F_PI)) * std::atan2(
                std::sqrt(3.0f) * (rgb.y - rgb.z),
                2.0f * rgb.x - rgb.y - rgb.z);
    }
    return (hue < 0.0f) ? hue + 360.0f : hue;
}

inline float center_hue(float hue, float centerH) {
    float hueCentered = hue - centerH;
    if (hueCentered < -180.0f) {
        hueCentered = hueCentered + 360.0f;
    } else if (hueCentered > 180.0f) {
        hueCentered = hueCentered - 360.0f;
    }
    return hueCentered;
}

inline float3 XYZ_2_xyY(float3 XYZ) {
    float divisor = max(XYZ.x + XYZ.y + XYZ.z, 1e-5f);
    return float3(XYZ.xy / divisor, XYZ.y);
}

inline float3 xyY_2_XYZ(float3 xyY) {
    float a = xyY.z / max(xyY.y, 1e-5f);
    float3 XYZ = float3(float2{ xyY.x, xyY.z }, (1.0f - xyY.x - xyY.y));
    XYZ.x *= a;
    XYZ.z *= a;
    return XYZ;
}

inline float3 darkSurround_to_dimSurround(float3 linearCV) {
    const float DIM_SURROUND_GAMMA = 0.9811f;

    const mat3f AP1_2_XYZ{
            0.6624541811f, 0.2722287168f, -0.0055746495f,
            0.1340042065f, 0.6740817658f, 0.0040607335f,
            0.1561876870f, 0.0536895174f, 1.0103391003f
    };

    const mat3f XYZ_2_AP1{
            1.6410233797f, -0.6636628587f, 0.0117218943f,
            -0.3248032942f, 1.6153315917f, -0.0082844420f,
            -0.2364246952f, 0.0167563477f, 0.9883948585f
    };

    float3 XYZ = AP1_2_XYZ * linearCV;
    float3 xyY = XYZ_2_xyY(XYZ);

    xyY.z = clamp(xyY.z, 0.0f, (float)std::numeric_limits<math::half>::max());
    xyY.z = std::pow(xyY.z, DIM_SURROUND_GAMMA);

    XYZ = xyY_2_XYZ(xyY);
    return XYZ_2_AP1 * XYZ;
}

UTILS_ALWAYS_INLINE
inline float3 ACES(float3 color) {
    // Some bits were removed to adapt to our desired output
    // Input:  linear sRGB
    // Output: linear sRGB

    const mat3f sRGB_2_AP0{
            0.439701f, 0.0897923f, 0.017544f,
            0.382978f, 0.8134230f, 0.111544f,
            0.177335f, 0.0967616f, 0.870704f
    };

    const mat3f AP0_2_AP1{
            1.4514393161f, -0.0765537734f, 0.0083161484f,
            -0.2365107469f, 1.1762296998f, -0.0060324498f,
            -0.2149285693f, -0.0996759264f, 0.9977163014f
    };

    const mat3f AP1_2_sRGB{
            1.70505f, -0.13026f, -0.024f,
            -0.62179f, 1.1408f, -0.12897f,
            -0.08326f, -0.01055f, 1.15297f
    };

    // "Glow" module constants
    const float RRT_GLOW_GAIN = 0.05f;
    const float RRT_GLOW_MID = 0.08f;

    // Red modifier constants
    const float RRT_RED_SCALE = 0.82f;
    const float RRT_RED_PIVOT = 0.03f;
    const float RRT_RED_HUE   = 0.0f;
    const float RRT_RED_WIDTH = 135.0f;

    // Desaturation contants
    const float RRT_SAT_FACTOR = 0.96f;
    const float ODT_SAT_FACTOR = 0.93f;

    // This assumes our working color space is sRGB
    float3 ap0 = sRGB_2_AP0 * color;

    // Glow module
    float saturation = rgb_2_saturation(ap0);
    float ycIn = rgb_2_yc(ap0);
    float s = sigmoid_shaper((saturation - 0.4f) / 0.2f);
    float addedGlow = 1.0f + glow_fwd(ycIn, RRT_GLOW_GAIN * s, RRT_GLOW_MID);
    ap0 *= addedGlow;

    // Red modifier
    float hue = rgb_2_hue(ap0);
    float centeredHue = center_hue(hue, RRT_RED_HUE);
    float hueWeight = smoothstep(0.0f, 1.0f, 1.0f - std::abs(2.0f * centeredHue / RRT_RED_WIDTH));
    hueWeight *= hueWeight;

    ap0.r += hueWeight * saturation * (RRT_RED_PIVOT - ap0.r) * (1.0f - RRT_RED_SCALE);

    // ACES to RGB rendering space
    float3 ap1 = clamp(AP0_2_AP1 * ap0, 0.0f, (float)std::numeric_limits<math::half>::max());

    // Global desaturation
    const float3 AP1_RGB2Y{ 0.272229f, 0.674082f, 0.0536895f };

    ap1 = mix(float3(dot(ap1, AP1_RGB2Y)), ap1, RRT_SAT_FACTOR);

#if defined(TONEMAP_ACES_MATCH_BRIGHTNESS)
    ap1 *= 1.0f / 0.6f;
#endif

    // Fitting of RRT + ODT (RGB monitor 100 nits dim) from:
    // https://github.com/colour-science/colour-unity/blob/master/Assets/Colour/Notebooks/CIECAM02_Unity.ipynb
    const float a = 2.785085f;
    const float b = 0.107772f;
    const float c = 2.936045f;
    const float d = 0.887122f;
    const float e = 0.806889f;
    float3 rgbPost = (ap1 * (a * ap1 + b)) / (ap1 * (c * ap1 + d) + e);

    // Apply gamma adjustment to compensate for dim surround
    float3 linearCV = darkSurround_to_dimSurround(rgbPost);

    // Apply desaturation to compensate for luminance difference
    linearCV = mix(float3(dot(linearCV, AP1_RGB2Y)), linearCV, ODT_SAT_FACTOR);

    // Convert to display primary encoding (Rec.709 primaries, D65 white point)
    return AP1_2_sRGB * linearCV;
}

} // namespace aces

//------------------------------------------------------------------------------
// Tone mapping operators
//------------------------------------------------------------------------------

namespace tonemap {

UTILS_ALWAYS_INLINE
float3 Linear(float3 x) noexcept {
    return x;
}

float3 Reinhard(float3 x) noexcept {
    return x / (1.0f + dot(x, float3{0.2126f, 0.7152f, 0.0722f}));
}

float3 Filmic(float3 x) noexcept {
    // Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
    const float a = 2.51f;
    const float b = 0.03f;
    const float c = 2.43f;
    const float d = 0.59f;
    const float e = 0.14f;
    return (x * (a * x + b)) / (x * (c * x + d) + e);
}

float3 ACES(float3 x) noexcept {
    return aces::ACES(x);
}

/**
 * Converts the input HDR RGB color into one of 16 debug colors that represent
 * the pixel's exposure. When the output is cyan, the input color represents
 * middle gray (18% exposure). Every exposure stop above or below middle gray
 * causes a color shift.
 *
 * The relationship between exposures and colors is:
 *
 * -5EV  - black
 * -4EV  - darkest blue
 * -3EV  - darker blue
 * -2EV  - dark blue
 * -1EV  - blue
 *  OEV  - cyan
 * +1EV  - dark green
 * +2EV  - green
 * +3EV  - yellow
 * +4EV  - yellow-orange
 * +5EV  - orange
 * +6EV  - bright red
 * +7EV  - red
 * +8EV  - magenta
 * +9EV  - purple
 * +10EV - white
 */
float3 DisplayRange(float3 x) noexcept {
    // 16 debug colors + 1 duplicated at the end for easy indexing

    const float3 debugColors[17] = {
            {0.0,     0.0,     0.0},         // black
            {0.0,     0.0,     0.1647},      // darkest blue
            {0.0,     0.0,     0.3647},      // darker blue
            {0.0,     0.0,     0.6647},      // dark blue
            {0.0,     0.0,     0.9647},      // blue
            {0.0,     0.9255,  0.9255},      // cyan
            {0.0,     0.5647,  0.0},         // dark green
            {0.0,     0.7843,  0.0},         // green
            {1.0,     1.0,     0.0},         // yellow
            {0.90588, 0.75294, 0.0},         // yellow-orange
            {1.0,     0.5647,  0.0},         // orange
            {1.0,     0.0,     0.0},         // bright red
            {0.8392,  0.0,     0.0},         // red
            {1.0,     0.0,     1.0},         // magenta
            {0.6,     0.3333,  0.7882},      // purple
            {1.0,     1.0,     1.0},         // white
            {1.0,     1.0,     1.0}          // white
    };

    // The 5th color in the array (cyan) represents middle gray (18%)
    // Every stop above or below middle gray causes a color shift
    float v = log2(dot(x, float3{0.2126f, 0.7152f, 0.0722f}) / 0.18f);
    v = clamp(v + 5.0f, 0.0f, 15.0f);
    size_t index = size_t(v);
    return mix(debugColors[index], debugColors[index + 1], saturate(v - float(index)));
}

} // namespace tonemap

} // namespace filament

#endif //TNT_FILAMENT_TONE_MAPPING_H
