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

#include "ColorSpaceUtils.h"

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

    xyY.z = clamp(xyY.z, 0.0f, (float) std::numeric_limits<half>::max());
    xyY.z = std::pow(xyY.z, DIM_SURROUND_GAMMA);

    XYZ = xyY_to_XYZ(xyY);
    return XYZ_to_AP1 * XYZ;
}

float3 ACES(float3 color, float brightness) noexcept {
    // Some bits were removed to adapt to our desired output

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

    float3 ap0 = Rec2020_to_AP0 * color;

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
    ap1 = mix(float3(dot(ap1, LUMINANCE_AP1)), ap1, RRT_SAT_FACTOR);

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
    linearCV = mix(float3(dot(linearCV, LUMINANCE_AP1)), linearCV, ODT_SAT_FACTOR);

    return AP1_to_Rec2020 * linearCV;
}

} // namespace aces

//------------------------------------------------------------------------------
// Tone mappers
//------------------------------------------------------------------------------

#define DEFAULT_CONSTRUCTORS(A) \
        A::A() noexcept = default; \
        A::~A() noexcept = default;

DEFAULT_CONSTRUCTORS(ToneMapper)

//------------------------------------------------------------------------------
// Linear tone mapper
//------------------------------------------------------------------------------

DEFAULT_CONSTRUCTORS(LinearToneMapper)

float3 LinearToneMapper::operator()(float3 v) const noexcept {
    return saturate(v);
}

//------------------------------------------------------------------------------
// ACES tone mappers
//------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------
// PBR Neutral tone mapper
//------------------------------------------------------------------------------

DEFAULT_CONSTRUCTORS(PBRNeutralToneMapper)

float3 PBRNeutralToneMapper::operator()(math::float3 color) const noexcept {
    // PBR Tone Mapping, https://modelviewer.dev/examples/tone-mapping.html
    constexpr float startCompression = 0.8f - 0.04f;
    constexpr float desaturation = 0.15f;

    float x = min(color.r, min(color.g, color.b));
    float offset = x < 0.08f ? x - 6.25f * x * x : 0.04f;
    color -= offset;

    float peak = max(color.r, max(color.g, color.b));
    if (peak < startCompression) return color;

    float d = 1.0f - startCompression;
    float newPeak = 1.0f - d * d / (peak + d - startCompression);
    color *= newPeak / peak;

    float g = 1.0f - 1.0f / (desaturation * (peak - newPeak) + 1.0f);
    return mix(color, float3(1.0f), g);
}

//------------------------------------------------------------------------------
// AgX tone mapper
//------------------------------------------------------------------------------

AgxToneMapper::AgxToneMapper(AgxToneMapper::AgxLook look) noexcept : look(look) {}
AgxToneMapper::~AgxToneMapper() noexcept = default;

// These matrices taken from Blender's implementation of AgX, which works with Rec.2020 primaries.
// https://github.com/EaryChow/AgX_LUT_Gen/blob/main/AgXBaseRec2020.py
constexpr mat3f AgXInsetMatrix {
    0.856627153315983, 0.137318972929847, 0.11189821299995,
    0.0951212405381588, 0.761241990602591, 0.0767994186031903,
    0.0482516061458583, 0.101439036467562, 0.811302368396859
};
constexpr mat3f AgXOutsetMatrixInv {
    0.899796955911611, 0.11142098895748, 0.11142098895748,
    0.0871996192028351, 0.875575586156966, 0.0871996192028349,
    0.013003424885555, 0.0130034248855548, 0.801379391839686
};
constexpr mat3f AgXOutsetMatrix { inverse(AgXOutsetMatrixInv) };

// LOG2_MIN      = -10.0
// LOG2_MAX      =  +6.5
// MIDDLE_GRAY   =  0.18
const float AgxMinEv = -12.47393f;      // log2(pow(2, LOG2_MIN) * MIDDLE_GRAY)
const float AgxMaxEv = 4.026069f;       // log2(pow(2, LOG2_MAX) * MIDDLE_GRAY)

// Adapted from https://iolite-engine.com/blog_posts/minimal_agx_implementation
float3 agxDefaultContrastApprox(float3 x) {
    float3 x2 = x * x;
    float3 x4 = x2 * x2;
    float3 x6 = x4 * x2;
    return  - 17.86f    * x6 * x
            + 78.01f    * x6
            - 126.7f    * x4 * x
            + 92.06f    * x4
            - 28.72f    * x2 * x
            + 4.361f    * x2
            - 0.1718f   * x
            + 0.002857f;
}

// Adapted from https://iolite-engine.com/blog_posts/minimal_agx_implementation
float3 agxLook(float3 val, AgxToneMapper::AgxLook look) {
    if (look == AgxToneMapper::AgxLook::NONE) {
        return val;
    }

    const float3 lw = float3(0.2126f, 0.7152f, 0.0722f);
    float luma = dot(val, lw);

    // Default
    float3 offset = float3(0.0f);
    float3 slope = float3(1.0f);
    float3 power = float3(1.0f);
    float sat = 1.0f;

    if (look == AgxToneMapper::AgxLook::GOLDEN) {
        slope = float3(1.0f, 0.9f, 0.5f);
        power = float3(0.8f);
        sat = 1.3;
    }
    if (look == AgxToneMapper::AgxLook::PUNCHY) {
        slope = float3(1.0f);
        power = float3(1.35f, 1.35f, 1.35f);
        sat = 1.4;
    }

    // ASC CDL
    val = pow(val * slope + offset, power);
    return luma + sat * (val - luma);
}

float3 AgxToneMapper::operator()(float3 v) const noexcept {
    // Ensure no negative values
    v = max(float3(0.0f), v);

    v = AgXInsetMatrix * v;

    // Log2 encoding
    v = max(v, 1E-10f); // avoid 0 or negative numbers for log2
    v = log2(v);
    v = (v - AgxMinEv) / (AgxMaxEv - AgxMinEv);

    v = clamp(v, 0.0f, 1.0f);

    // Apply sigmoid
    v = agxDefaultContrastApprox(v);

    // Apply AgX look
    v = agxLook(v, look);

    v = AgXOutsetMatrix * v;

    // Linearize
    v = pow(max(float3(0.0f), v), 2.2f);

    return v;
}

//------------------------------------------------------------------------------
// Display range tone mapper
//------------------------------------------------------------------------------

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
    // TODO: This should depend on the working color grading color space
    float v = log2(dot(c, LUMINANCE_Rec2020) / 0.18f);
    v = clamp(v + 5.0f, 0.0f, 15.0f);

    size_t index = size_t(v);
    return mix(debugColors[index], debugColors[index + 1], saturate(v - float(index)));
}

//------------------------------------------------------------------------------
// Generic tone mapper
//------------------------------------------------------------------------------

struct GenericToneMapper::Options {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshadow"
    void setParameters(
            float contrast,
            float midGrayIn,
            float midGrayOut,
            float hdrMax
    ) {
        contrast = max(contrast, 1e-5f);
        midGrayIn = clamp(midGrayIn, 1e-5f, 1.0f);
        midGrayOut = clamp(midGrayOut, 1e-5f, 1.0f);
        hdrMax = max(hdrMax, 1.0f);

        this->contrast = contrast;
        this->midGrayIn = midGrayIn;
        this->midGrayOut = midGrayOut;
        this->hdrMax = hdrMax;

        float a = pow(midGrayIn, contrast);
        float b = pow(hdrMax, contrast);
        float c = a - midGrayOut * b;

        inputScale = (a * b * (midGrayOut - 1.0f)) / c;
        outputScale = midGrayOut * (a - b) / c;
    }
#pragma clang diagnostic pop

    float contrast;
    float midGrayIn;
    float midGrayOut;
    float hdrMax;

    // TEMP
    float inputScale;
    float outputScale;
};

GenericToneMapper::GenericToneMapper(
        float contrast,
        float midGrayIn,
        float midGrayOut,
        float hdrMax
) noexcept {
    mOptions = new Options();
    mOptions->setParameters(contrast, midGrayIn, midGrayOut, hdrMax);
}

GenericToneMapper::~GenericToneMapper() noexcept {
    delete mOptions;
}

GenericToneMapper::GenericToneMapper(GenericToneMapper&& rhs)  noexcept : mOptions(rhs.mOptions) {
    rhs.mOptions = nullptr;
}

GenericToneMapper& GenericToneMapper::operator=(GenericToneMapper&& rhs) noexcept {
    mOptions = rhs.mOptions;
    rhs.mOptions = nullptr;
    return *this;
}

float3 GenericToneMapper::operator()(math::float3 x) const noexcept {
    x = pow(x, mOptions->contrast);
    return mOptions->outputScale * x / (x + mOptions->inputScale);
}

float GenericToneMapper::getContrast() const noexcept { return  mOptions->contrast; }
float GenericToneMapper::getMidGrayIn() const noexcept { return  mOptions->midGrayIn; }
float GenericToneMapper::getMidGrayOut() const noexcept { return  mOptions->midGrayOut; }
float GenericToneMapper::getHdrMax() const noexcept { return  mOptions->hdrMax; }

void GenericToneMapper::setContrast(float contrast) noexcept {
    mOptions->setParameters(
            contrast,
            mOptions->midGrayIn,
            mOptions->midGrayOut,
            mOptions->hdrMax
    );
}
void GenericToneMapper::setMidGrayIn(float midGrayIn) noexcept {
    mOptions->setParameters(
            mOptions->contrast,
            midGrayIn,
            mOptions->midGrayOut,
            mOptions->hdrMax
    );
}

void GenericToneMapper::setMidGrayOut(float midGrayOut) noexcept {
    mOptions->setParameters(
            mOptions->contrast,
            mOptions->midGrayIn,
            midGrayOut,
            mOptions->hdrMax
    );
}

void GenericToneMapper::setHdrMax(float hdrMax) noexcept {
    mOptions->setParameters(
            mOptions->contrast,
            mOptions->midGrayIn,
            mOptions->midGrayOut,
            hdrMax
    );
}

#undef DEFAULT_CONSTRUCTORS

} // namespace filament
