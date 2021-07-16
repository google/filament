/*
 * Copyright (C) 2021 The Android Open Source Project
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

#include <filament/ToneMapper.h>

#include "ColorSpace.h"

#include <math/vec3.h>
#include <math/scalar.h>

namespace filament {

using namespace math;

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
        hue = f::RAD_TO_DEG * std::atan2(
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

    xyY.z = clamp(xyY.z, 0.0f, (float)std::numeric_limits<half>::max());
    xyY.z = std::pow(xyY.z, DIM_SURROUND_GAMMA);

    XYZ = xyY_to_XYZ(xyY);
    return XYZ_to_AP1 * XYZ;
}

float3 ACES(float3 color, float brightness) noexcept {
    // Some bits were removed to adapt to our desired output
    // Input:  ACEScg (AP1)
    // Output: ACEScg (AP1)

    // "Glow" module constants
    constexpr float RRT_GLOW_GAIN = 0.05f;
    constexpr float RRT_GLOW_MID = 0.08f;

    // Red modifier constants
    constexpr float RRT_RED_SCALE = 0.82f;
    constexpr float RRT_RED_PIVOT = 0.03f;
    constexpr float RRT_RED_HUE   = 0.0f;
    constexpr float RRT_RED_WIDTH = 135.0f;

    // Desaturation constants
    constexpr float RRT_SAT_FACTOR = 0.96f;
    constexpr float ODT_SAT_FACTOR = 0.93f;

    float3 ap0 = REC2020_to_AP0 * color;

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
    float3 ap1 = clamp(AP0_to_AP1 * ap0, 0.0f, (float) std::numeric_limits<half>::max());

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

    return AP1_to_REC2020 * linearCV;
}

} // namespace aces

//------------------------------------------------------------------------------
// Tone mappers
//------------------------------------------------------------------------------

#define DEFAULT_CONSTRUCTORS(A) \
        A::A() noexcept = default; \
        A::~A() noexcept = default;

DEFAULT_CONSTRUCTORS(ToneMapper)

DEFAULT_CONSTRUCTORS(LinearToneMapper)

float3 LinearToneMapper::operator()(float3 v) const noexcept {
    return v;
}

DEFAULT_CONSTRUCTORS(ACESToneMapper)

float3 ACESToneMapper::operator()(math::float3 c) const noexcept {
    return aces::ACES(c, 1.0f);
}

DEFAULT_CONSTRUCTORS(ACESLegacyToneMapper)

float3 ACESLegacyToneMapper::operator()(math::float3 c) const noexcept {
    return aces::ACES(c, 1.0f / 0.6f);
}

DEFAULT_CONSTRUCTORS(FilmicToneMapper)

float3 FilmicToneMapper::operator()(math::float3 x) const noexcept {
    // Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
    constexpr float a = 2.51f;
    constexpr float b = 0.03f;
    constexpr float c = 2.43f;
    constexpr float d = 0.59f;
    constexpr float e = 0.14f;
    return (x * (a * x + b)) / (x * (c * x + d) + e);
}

DEFAULT_CONSTRUCTORS(DisplayRangeToneMapper)

float3 DisplayRangeToneMapper::operator()(math::float3 c) const noexcept {
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
    float v = log2(dot(c, LUMA_REC709) / 0.18f);
    v = clamp(v + 5.0f, 0.0f, 15.0f);

    size_t index = size_t(v);
    return mix(debugColors[index], debugColors[index + 1], saturate(v - float(index)));
}

// TODO: These constants were chosen to match our ACES tone mappers as closely as possible
//       in terms of compression. We should expose these parameters to users via an API.
//       We must however carefully validate exposed parameters as it is easy to get the
//       generic tonemapper to produce invalid curves.
// TODO: Expose this as a public tone mapper
float genericTonemap(
        float x,
        float contrast = 1.6f,
        float shoulder = 1.0f,
        float midGreyIn = 0.18f,
        float midGreyOut = 0.227f,
        float hdrMax = 64.0f
) noexcept {
    // Lottes, 2016,"Advanced Techniques and Optimization of VDR Color Pipelines"
    // https://gpuopen.com/wp-content/uploads/2016/03/GdcVdrLottes.pdf
    float mc = std::pow(midGreyIn, contrast);
    float mcs = std::pow(mc, shoulder);

    float hc = std::pow(hdrMax, contrast);
    float hcs = std::pow(hc, shoulder);

    float b1 = -mc + hc * midGreyOut;
    float b2 = (hcs - mcs) * midGreyOut;
    float b = b1 / b2;

    float c1 = hcs * mc - hc * mcs * midGreyOut;
    float c2 = (hcs - mcs) * midGreyOut;
    float c = c1 / c2;

    float xc = std::pow(x, contrast);
    return saturate(xc / (std::pow(xc, shoulder) * b + c));
}

#undef DEFAULT_CONSTRUCTORS

} // namespace filament
