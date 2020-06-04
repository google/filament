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

#include "ColorSpace.h"

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
    constexpr float TINY = 1e-5f;
    float mi = min(rgb);
    float ma = max(rgb);
    return (max(ma, TINY) - max(mi, TINY)) / max(ma, 1e-2f);
}

inline float rgb_2_yc(float3 rgb) {
    constexpr float ycRadiusWeight = 1.75f;

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

inline float3 darkSurround_to_dimSurround(float3 linearCV) {
    constexpr float DIM_SURROUND_GAMMA = 0.9811f;

    float3 XYZ = AP1_to_XYZ * linearCV;
    float3 xyY = XYZ_to_xyY(XYZ);

    xyY.z = clamp(xyY.z, 0.0f, (float)std::numeric_limits<math::half>::max());
    xyY.z = std::pow(xyY.z, DIM_SURROUND_GAMMA);

    XYZ = xyY_to_XYZ(xyY);
    return XYZ_to_AP1 * XYZ;
}

UTILS_ALWAYS_INLINE
inline float3 ACES(float3 color, float brightness) {
    // Some bits were removed to adapt to our desired output
    // Input:  ACEScg (AP1)
    // Output: linear sRGB

    // "Glow" module constants
    constexpr float RRT_GLOW_GAIN = 0.05f;
    constexpr float RRT_GLOW_MID = 0.08f;

    // Red modifier constants
    constexpr float RRT_RED_SCALE = 0.82f;
    constexpr float RRT_RED_PIVOT = 0.03f;
    constexpr float RRT_RED_HUE   = 0.0f;
    constexpr float RRT_RED_WIDTH = 135.0f;

    // Desaturation contants
    constexpr float RRT_SAT_FACTOR = 0.96f;
    constexpr float ODT_SAT_FACTOR = 0.93f;

    float3 ap0 = AP1_to_AP0 * color;

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
    float3 ap1 = clamp(AP0_to_AP1 * ap0, 0.0f, (float) std::numeric_limits<math::half>::max());

    // Global desaturation
    ap1 = mix(float3(dot(ap1, LUMA_AP1)), ap1, RRT_SAT_FACTOR);

    // NOTE: This is specific to Filament and added only to match ACES to our legacy tone mapper
    //       which was a fit of ACES in Rec.709 but with a brightness boost.
    ap1 *= brightness;

    // Fitting of RRT + ODT (RGB monitor 100 nits dim) from:
    // https://github.com/colour-science/colour-unity/blob/master/Assets/Colour/Notebooks/CIECAM02_Unity.ipynb
    constexpr float a = 2.785085f;
    constexpr float b = 0.107772f;
    constexpr float c = 2.936045f;
    constexpr float d = 0.887122f;
    constexpr float e = 0.806889f;
    float3 rgbPost = (ap1 * (a * ap1 + b)) / (ap1 * (c * ap1 + d) + e);

    // Apply gamma adjustment to compensate for dim surround
    float3 linearCV = darkSurround_to_dimSurround(rgbPost);

    // Apply desaturation to compensate for luminance difference
    linearCV = mix(float3(dot(linearCV, LUMA_AP1)), linearCV, ODT_SAT_FACTOR);

    // Convert to display primary encoding (Rec.709 primaries, D65 white point)
    return AP1_to_sRGB * linearCV;
}

} // namespace aces

//------------------------------------------------------------------------------
// Tone mapping operators
//------------------------------------------------------------------------------

namespace tonemap {

constexpr float3 Linear(float3 x) noexcept {
    return x;
}

constexpr float3 Reinhard(float3 x) noexcept {
    return x / (1.0f + dot(x, LUMA_REC709));
}

constexpr float3 Filmic(float3 x) noexcept {
    // Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
    constexpr float a = 2.51f;
    constexpr float b = 0.03f;
    constexpr float c = 2.43f;
    constexpr float d = 0.59f;
    constexpr float e = 0.14f;
    return (x * (a * x + b)) / (x * (c * x + d) + e);
}

float3 ACES(float3 x) noexcept {
    return aces::ACES(x, 1.0f);
}

float3 ACES_Legacy(float3 x) noexcept {
    return aces::ACES(x, 1.0f / 0.6f);
}

float3 Uchimura(float3 x) {
    // Uchimura 2017, "HDR theory and practice"
    // Math: https://www.desmos.com/calculator/gslcdxvipg
    // Source: https://www.slideshare.net/nikuque/hdr-theory-and-practicce-jp

    constexpr float P = 1.0;  // max display brightness
    constexpr float a = 1.0;  // contrast
    constexpr float m = 0.22; // linear section start
    constexpr float l = 0.4;  // linear section length
    constexpr float c = 1.33; // black
    constexpr float b = 0.0;  // pedestal

    constexpr float l0 = ((P - m) * l) / a;
    constexpr float S0 = m + l0;
    constexpr float S1 = m + a * l0;
    constexpr float C2 = (a * P) / (P - S1);
    constexpr float CP = -C2 / P;

    float3 w0 = 1.0f - smoothstep(0.0f, m, x);
    float3 w2 = step(m + l0, x);
    float3 w1 = 1.0f - w0 - w2;

    float3 T = m * pow(x / m, c) + b;
    float3 S = P - (P - S1) * exp(CP * (x - S0));
    float3 L = m + a * (x - m);

    return T * w0 + L * w1 + S * w2;
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

    constexpr float3 debugColors[17] = {
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
    float v = log2(dot(x, LUMA_REC709) / 0.18f);
    v = clamp(v + 5.0f, 0.0f, 15.0f);

    size_t index = size_t(v);
    return mix(debugColors[index], debugColors[index + 1], saturate(v - float(index)));
}

} // namespace tonemap

} // namespace filament

#endif //TNT_FILAMENT_TONE_MAPPING_H
