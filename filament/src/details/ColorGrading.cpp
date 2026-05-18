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

#include "details/ColorGrading.h"
#include "details/Engine.h"
#include "details/Texture.h"

#include "FilamentAPI-impl.h"

#include "ColorSpaceUtils.h"

#include <private/utils/Tracing.h>

#include <filament/ColorGrading.h>
#include <filament/ColorSpace.h>
#include <filament/ToneMapper.h>

#include <backend/DriverApiForward.h>
#include <backend/DriverEnums.h>

#include <utils/FixedCapacityVector.h>
#include <utils/JobSystem.h>
#include <utils/Mutex.h>
#include <utils/Panic.h>
#include <utils/compiler.h>
#include <utils/debug.h>

#include <math/half.h>
#include <math/scalar.h>
#include <math/mat3.h>
#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <algorithm>
#include <cstdint>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <tuple>
#include <utility>

#if defined(__ARM_NEON)
#include "ColorGradingNeon.h"
#include <arm_neon.h>
#endif

namespace filament {

using namespace utils;
using namespace math;
using namespace color;
using namespace backend;

//------------------------------------------------------------------------------
// Builder
//------------------------------------------------------------------------------

struct ColorGrading::BuilderDetails {
    const ToneMapper* toneMapper = nullptr;

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
    ToneMapping toneMapping = ToneMapping::ACES_LEGACY;
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

    bool hasAdjustments = false;
    bool customToneMapper = false;

    // Everything below must be part of the == comparison operator
    LutFormat format = LutFormat::INTEGER;
    uint8_t dimension = 32;

    // Out-of-gamut color handling
    bool   luminanceScaling = false;
    bool   gamutMapping     = false;
    // Exposure
    float  exposure         = 0.0f;
    // Night adaptation
    float  nightAdaptation  = 0.0f;
    // White balance
    float2 whiteBalance     = {0.0f, 0.0f};
    // Channel mixer
    float3 outRed           = {1.0f, 0.0f, 0.0f};
    float3 outGreen         = {0.0f, 1.0f, 0.0f};
    float3 outBlue          = {0.0f, 0.0f, 1.0f};
    // Tonal ranges
    float3 shadows          = {1.0f, 1.0f, 1.0f};
    float3 midtones         = {1.0f, 1.0f, 1.0f};
    float3 highlights       = {1.0f, 1.0f, 1.0f};
    float4 tonalRanges      = {0.0f, 0.333f, 0.550f, 1.0f}; // defaults in DaVinci Resolve
    // ASC CDL
    float3 slope            = {1.0f};
    float3 offset           = {0.0f};
    float3 power            = {1.0f};
    // Color adjustments
    float  contrast         = 1.0f;
    float  vibrance         = 1.0f;
    float  saturation       = 1.0f;
    // Curves
    float3 shadowGamma      = {1.0f};
    float3 midPoint         = {1.0f};
    float3 highlightScale   = {1.0f};

    // Output color space
    ColorSpace outputColorSpace = Rec709-sRGB-D65;

    // Custom LUT
    FixedCapacityVector<float3> customLutData;
    uint8_t customLutDimension = 0;

    ExportCallback exportCallback = nullptr;
    void* exportUser = nullptr;
    bool fastMath = true;

    bool operator!=(const BuilderDetails &rhs) const {
        return !(rhs == *this);
    }

    bool operator==(const BuilderDetails &rhs) const {
        // Note: Do NOT compare hasAdjustments and toneMapper
        return format == rhs.format &&
               dimension == rhs.dimension &&
               luminanceScaling == rhs.luminanceScaling &&
               gamutMapping == rhs.gamutMapping &&
               exposure == rhs.exposure &&
               nightAdaptation == rhs.nightAdaptation &&
               whiteBalance == rhs.whiteBalance &&
               outRed == rhs.outRed &&
               outGreen == rhs.outGreen &&
               outBlue == rhs.outBlue &&
               shadows == rhs.shadows &&
               midtones == rhs.midtones &&
               highlights == rhs.highlights &&
               tonalRanges == rhs.tonalRanges &&
               slope == rhs.slope &&
               offset == rhs.offset &&
               power == rhs.power &&
               contrast == rhs.contrast &&
               vibrance == rhs.vibrance &&
               saturation == rhs.saturation &&
               shadowGamma == rhs.shadowGamma &&
               midPoint == rhs.midPoint &&
               highlightScale == rhs.highlightScale &&
               outputColorSpace == rhs.outputColorSpace &&
               customLutData == rhs.customLutData &&
               customLutDimension == rhs.customLutDimension;
    }
};

using BuilderType = ColorGrading;
BuilderType::Builder::Builder() noexcept = default;
BuilderType::Builder::~Builder() noexcept = default;
BuilderType::Builder::Builder(Builder const& rhs) noexcept = default;
BuilderType::Builder::Builder(Builder&& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(Builder const& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(Builder&& rhs) noexcept = default;

ColorGrading::Builder& ColorGrading::Builder::quality(QualityLevel const qualityLevel) noexcept {
    switch (qualityLevel) {
        case QualityLevel::LOW:
            mImpl->format = LutFormat::INTEGER;
            mImpl->dimension = 16;
            break;
        case QualityLevel::MEDIUM:
            mImpl->format = LutFormat::INTEGER;
            mImpl->dimension = 32;
            break;
        case QualityLevel::HIGH:
            mImpl->format = LutFormat::FLOAT;
            mImpl->dimension = 32;
            break;
        case QualityLevel::ULTRA:
            mImpl->format = LutFormat::FLOAT;
            mImpl->dimension = 64;
            break;
    }
    return *this;
}

ColorGrading::Builder& ColorGrading::Builder::format(LutFormat const format) noexcept {
    mImpl->format = format;
    return *this;
}

ColorGrading::Builder& ColorGrading::Builder::dimensions(uint8_t const dim) noexcept {
    mImpl->dimension = clamp(+dim, 16, 64);
    return *this;
}

ColorGrading::Builder& ColorGrading::Builder::toneMapper(const ToneMapper* toneMapper) noexcept {
    mImpl->toneMapper = toneMapper;
    mImpl->customToneMapper = toneMapper != nullptr;
    return *this;
}

ColorGrading::Builder& ColorGrading::Builder::toneMapping(ToneMapping const toneMapping) noexcept {
    mImpl->toneMapping = toneMapping;
    return *this;
}

ColorGrading::Builder& ColorGrading::Builder::luminanceScaling(bool const luminanceScaling) noexcept {
    mImpl->luminanceScaling = luminanceScaling;
    return *this;
}

ColorGrading::Builder& ColorGrading::Builder::gamutMapping(bool const gamutMapping) noexcept {
    mImpl->gamutMapping = gamutMapping;
    return *this;
}

ColorGrading::Builder& ColorGrading::Builder::exposure(float const exposure) noexcept {
    mImpl->exposure = exposure;
    return *this;
}

ColorGrading::Builder& ColorGrading::Builder::nightAdaptation(float const adaptation) noexcept {
    mImpl->nightAdaptation = saturate(adaptation);
    return *this;
}

ColorGrading::Builder& ColorGrading::Builder::whiteBalance(float const temperature, float const tint) noexcept {
    mImpl->whiteBalance = float2{
        clamp(temperature, -1.0f, 1.0f),
        clamp(tint, -1.0f, 1.0f)
    };
    return *this;
}

ColorGrading::Builder& ColorGrading::Builder::channelMixer(
        float3 outRed, float3 outGreen, float3 outBlue) noexcept {
    mImpl->outRed   = clamp(outRed,   -2.0f, 2.0f);
    mImpl->outGreen = clamp(outGreen, -2.0f, 2.0f);
    mImpl->outBlue  = clamp(outBlue,  -2.0f, 2.0f);
    return *this;
}

ColorGrading::Builder& ColorGrading::Builder::shadowsMidtonesHighlights(
        float4 shadows, float4 midtones, float4 highlights, float4 ranges) noexcept {
    mImpl->shadows = max(shadows.rgb + shadows.w, 0.0f);
    mImpl->midtones = max(midtones.rgb + midtones.w, 0.0f);
    mImpl->highlights = max(highlights.rgb + highlights.w, 0.0f);

    ranges.x = saturate(ranges.x); // shadows
    ranges.w = saturate(ranges.w); // highlights
    ranges.y = clamp(ranges.y, ranges.x + 1e-5f, ranges.w - 1e-5f); // darks
    ranges.z = clamp(ranges.z, ranges.x + 1e-5f, ranges.w - 1e-5f); // lights
    mImpl->tonalRanges = ranges;

    return *this;
}

ColorGrading::Builder& ColorGrading::Builder::slopeOffsetPower(
        float3 slope, float3 offset, float3 power) noexcept {
    mImpl->slope = max(1e-5f, slope);
    mImpl->offset = offset;
    mImpl->power = max(1e-5f, power);
    return *this;
}

ColorGrading::Builder& ColorGrading::Builder::contrast(float const contrast) noexcept {
    mImpl->contrast = clamp(contrast, 0.0f, 2.0f);
    return *this;
}

ColorGrading::Builder& ColorGrading::Builder::vibrance(float const vibrance) noexcept {
    mImpl->vibrance = clamp(vibrance, 0.0f, 2.0f);
    return *this;
}

ColorGrading::Builder& ColorGrading::Builder::saturation(float const saturation) noexcept {
    mImpl->saturation = clamp(saturation, 0.0f, 2.0f);
    return *this;
}

ColorGrading::Builder& ColorGrading::Builder::curves(
        float3 shadowGamma, float3 midPoint, float3 highlightScale) noexcept {
    mImpl->shadowGamma = max(1e-5f, shadowGamma);
    mImpl->midPoint = max(1e-5f, midPoint);
    mImpl->highlightScale = highlightScale;
    return *this;
}

ColorGrading::Builder& ColorGrading::Builder::outputColorSpace(
        const ColorSpace& colorSpace) noexcept {
    mImpl->outputColorSpace = colorSpace;
    return *this;
}

ColorGrading::Builder& ColorGrading::Builder::customLut(
        FixedCapacityVector<float3> data, uint8_t const dimension) noexcept {
    mImpl->customLutData = std::move(data);
    mImpl->customLutDimension = dimension;
    return *this;
}

ColorGrading::Builder& ColorGrading::Builder::exportLut(
        ExportCallback UTILS_NULLABLE const callback, void* UTILS_NULLABLE user) noexcept {
    mImpl->exportCallback = callback;
    mImpl->exportUser = user;
    return *this;
}

ColorGrading::Builder& ColorGrading::Builder::fastMath(bool const fastMath) noexcept {
    mImpl->fastMath = fastMath;
    return *this;
}

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
ColorGrading* ColorGrading::Builder::build(Engine& engine) {
    if (mImpl->customLutDimension == 0 || mImpl->customLutData.empty()) {
        mImpl->customLutData.clear();
        mImpl->customLutDimension = 0;
    } else {
        FILAMENT_CHECK_PRECONDITION(mImpl->customLutData.size() == 
                size_t(mImpl->customLutDimension) * mImpl->customLutDimension * mImpl->customLutDimension)
                << "Custom LUT data size does not match dimension^3";
    }

    // We want to see if any of the default adjustment values have been modified
    // We skip the tonemapping operator on purpose since we always want to apply it
    BuilderDetails const defaults;
    bool const hasAdjustments = defaults != *mImpl;
    mImpl->hasAdjustments = hasAdjustments;

    // Fallback for clients that still use the deprecated ToneMapping API
    bool const needToneMapper = mImpl->toneMapper == nullptr;
    if (needToneMapper) {
        switch (mImpl->toneMapping) {
            case ToneMapping::LINEAR:
                mImpl->toneMapper = new LinearToneMapper();
                break;
            case ToneMapping::ACES_LEGACY:
                mImpl->toneMapper = new ACESLegacyToneMapper();
                break;
            case ToneMapping::ACES:
                mImpl->toneMapper = new ACESToneMapper();
                break;
            case ToneMapping::FILMIC:
                mImpl->toneMapper = new FilmicToneMapper();
                break;
            case ToneMapping::DISPLAY_RANGE:
                mImpl->toneMapper = new DisplayRangeToneMapper();
                break;
        }
    }

    FColorGrading* colorGrading = downcast(engine).createColorGrading(*this);

    if (needToneMapper) {
        delete mImpl->toneMapper;
        mImpl->toneMapper = nullptr;
    }

    return colorGrading;
}

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

//------------------------------------------------------------------------------
// Purkinje shift/scotopic vision
//------------------------------------------------------------------------------

// In low-light conditions, peak luminance sensitivity of the eye shifts toward
// the blue end of the visible spectrum. This effect called the Purkinje effect
// occurs during the transition from photopic (cone-based) vision to scotopic
// (rod-based) vision. Because the rods and cones use the same neural pathways,
// a color shift is introduced as the rods take over to improve low-light
// perception.
//
// This function aims to (somewhat) replicate this color shift and peak luminance
// sensitivity increase to more faithfully reproduce scenes in low-light conditions
// as they would be perceived by a human observer (as opposed to an artificial
// observer such as a camera sensor).
//
// The implementation below is based on two papers:
// "Rod Contributions to Color Perception: Linear with Rod Contrast", Cao et al., 2008
//     https://www.ncbi.nlm.nih.gov/pmc/articles/PMC2630540/pdf/nihms80286.pdf
// "Perceptually Based Tone Mapping for Low-Light Conditions", Kirk & O'Brien, 2011
//     http://graphics.berkeley.edu/papers/Kirk-PBT-2011-08/Kirk-PBT-2011-08.pdf
//
// Many thanks to Jasmin Patry for his explanations in "Real-Time Samurai Cinema",
// SIGGRAPH 2021, and the idea of using log-luminance based on "Maximum Entropy
// Spectral Modeling Approach to Mesopic Tone Mapping", Rezagholizadeh & Clark, 2013
static float3 scotopicAdaptation(float3 v, float nightAdaptation) noexcept {
    // The 4 vectors below are generated by the command line tool rgb-to-lmsr.
    // Together they form a 4x3 matrix that can be used to convert a Rec.709
    // input color to the LMSR (long/medium/short cone + rod receptors) space.
    // That matrix is computed using this formula:
    //     Mij = \Integral Ei(lambda) I(lambda) Rj(lambda) d(lambda)
    // Where:
    //     i in {L, M, S, R}
    //     j in {R, G, B}
    //     lambda: wavelength
    //     Ei(lambda): response curve of the corresponding receptor
    //     I(lambda): relative spectral power of the CIE illuminant D65
    //     Rj(lambda): spectral power of the corresponding Rec.709 color
    constexpr float3 L{7.696847f, 18.424824f,  2.068096f};
    constexpr float3 M{2.431137f, 18.697937f,  3.012463f};
    constexpr float3 S{0.289117f,  1.401833f, 13.792292f};
    constexpr float3 R{0.466386f, 15.564362f, 10.059963f};

    constexpr mat3f LMS_to_RGB = inverse(transpose(mat3f{L, M, S}));

    // Maximal LMS cone sensitivity, Cao et al. Table 1
    constexpr float3 m{0.63721f, 0.39242f, 1.6064f};
    // Strength of rod input, free parameters in Cao et al., manually tuned for our needs
    // We follow Kirk & O'Brien who recommend constant values as opposed to Cao et al.
    // who propose to adapt those values based on retinal illuminance. We instead offer
    // artistic control at the end of the process
    // The vector below is {k1, k1, k2} in Kirk & O'Brien, but {k5, k5, k6} in Cao et al.
    constexpr float3 k{0.2f, 0.2f, 0.3f};

    // Transform from opponent space back to LMS
    constexpr mat3f opponent_to_LMS{
        -0.5f, 0.5f, 0.0f,
         0.0f, 0.0f, 1.0f,
         0.5f, 0.5f, 1.0f
    };

    // The constants below follow Cao et al, using the KC pathway
    // Scaling constant
    constexpr float K_ = 45.0f;
    // Static saturation
    constexpr float S_ = 10.0f;
    // Surround strength of opponent signal
    constexpr float k3 = 0.6f;
    // Radio of responses for white light
    constexpr float rw = 0.139f;
    // Relative weight of L cones
    constexpr float p  = 0.6189f;

    // Weighted cone response as described in Cao et al., section 3.3
    // The approximately linear relation defined in the paper is represented here
    // in matrix form to simplify the code
    constexpr mat3f weightedRodResponse = (K_ / S_) * (mat3f{
       -(k3 + rw),       p * k3,          p * S_,
        1.0f + k3 * rw, (1.0f - p) * k3, (1.0f - p) * S_,
        0.0f,            1.0f,            0.0f
    } * mat3f{k} * inverse(mat3f{m}));

    // Move to log-luminance, or the EV values as measured by a Minolta Spotmeter F.
    // The relationship is EV = log2(L * 100 / 14), or 2^EV = L / 0.14. We can therefore
    // multiply our input by 0.14 to obtain our log-luminance values.
    // We then follow Patry's recommendation to shift the log-luminance by ~ +11.4EV to
    // match luminance values to mesopic measurements as described in Rezagholizadeh &
    // Clark 2013,
    // The result is 0.14 * exp2(11.40) ~= 380.0 (we use +11.406 EV to get a round number)
    constexpr float logExposure = 380.0f;

    // Move to scaled log-luminance
    v *= logExposure;

    // Convert the scene color from Rec.709 to LMSR response
    float4 const q{dot(v, L), dot(v, M), dot(v, S), dot(v, R)};
    // Regulated signal through the selected pathway (KC in Cao et al.)
    float3 const g = inversesqrt(1.0f + max(float3{0.0f}, (0.33f / m) * (q.rgb + k * q.w)));

    // Compute the incremental effect that rods have in opponent space
    float3 const deltaOpponent = weightedRodResponse * g * q.w * nightAdaptation;
    // Photopic response in LMS space
    float3 const qHat = q.rgb + opponent_to_LMS * deltaOpponent;

    // And finally, back to RGB
    return (LMS_to_RGB * qHat) / logExposure;
}

//------------------------------------------------------------------------------
// White balance
//------------------------------------------------------------------------------

// Return the chromatic adaptation coefficients in LMS space for the given
// temperature/tint offsets. The chromatic adaption is perfomed following
// the von Kries method, using the CIECAT16 transform.
// See https://en.wikipedia.org/wiki/Chromatic_adaptation
// See https://en.wikipedia.org/wiki/CIECAM02#Chromatic_adaptation
static constexpr mat3f adaptationTransform(float2 const whiteBalance) noexcept {
    // See Mathematica notebook in docs/math/White Balance.nb
    float const k = whiteBalance.x; // temperature
    float const t = whiteBalance.y; // tint

    float const x = ILLUMINANT_D65_xyY[0] - k * (k < 0.0f ? 0.0214f : 0.066f);
    float const y = chromaticityCoordinateIlluminantD(x) + t * 0.066f;

    float3 const lms = XYZ_to_CIECAT16 * xyY_to_XYZ({x, y, 1.0f});
    return LMS_CAT16_to_Rec2020 * mat3f{ILLUMINANT_D65_LMS_CAT16 / lms} * Rec2020_to_LMS_CAT16;
}

//------------------------------------------------------------------------------
// General color grading
//------------------------------------------------------------------------------

using ColorTransform = float3(*)(float3);

UTILS_ALWAYS_INLINE
static constexpr float3 channelMixer(float3 v, float3 r, float3 g, float3 b) noexcept {
    return {dot(v, r), dot(v, g), dot(v, b)};
}

UTILS_ALWAYS_INLINE
static constexpr float3 tonalRanges(
        float3 v, float3 luminance,
        float3 shadows, float3 midtones, float3 highlights,
        float4 ranges
) noexcept {
    // See the Mathematica notebook at docs/math/Shadows Midtones Highlight.nb for
    // details on how the curves were designed. The default curve values are based
    // on the defaults from the "Log" color wheels in DaVinci Resolve.
    float const y = dot(v, luminance);

    // Shadows curve
    float const s = 1.0f - smoothstep(ranges.x, ranges.y, y);
    // Highlights curve
    float const h = smoothstep(ranges.z, ranges.w, y);
    // Mid-tones curves
    float const m = 1.0f - s - h;

    return v * s * shadows + v * m * midtones + v * h * highlights;
}

UTILS_ALWAYS_INLINE
static float3 colorDecisionList(float3 v, float3 slope, float3 offset, float3 power) noexcept {
    // Apply the ASC CSL in log space, as defined in S-2016-001
    v = v * slope + offset;
    float3 const pv = pow(v, power);
    return float3{
            v.r <= 0.0f ? v.r : pv.r,
            v.g <= 0.0f ? v.g : pv.g,
            v.b <= 0.0f ? v.b : pv.b
    };
}

UTILS_ALWAYS_INLINE
static constexpr float3 contrast(float3 const v, float const contrast, float const offset) noexcept {
    // Matches contrast as applied in DaVinci Resolve
    return v * contrast + offset;
}

UTILS_ALWAYS_INLINE
static constexpr float3 saturation(float3 v, float3 luminance, float saturation) noexcept {
    const float3 y = dot(v, luminance);
    return y + saturation * (v - y);
}

UTILS_ALWAYS_INLINE
static float3 vibrance(float3 v, float3 luminance, float vibrance) noexcept {
    float const  r = v.r - max(v.g, v.b);
    float const  s = (vibrance - 1.0f) / (1.0f + std::exp(-r * 3.0f)) + 1.0f;
    float3 const  l{(1.0f - s) * luminance};
    return float3{
        dot(v, l + float3{s, 0.0f, 0.0f}),
        dot(v, l + float3{0.0f, s, 0.0f}),
        dot(v, l + float3{0.0f, 0.0f, s}),
    };
}

UTILS_ALWAYS_INLINE
static float3 curves(float3 v, float3 shadowGamma, float3 midPoint, float3 highlightScale, float3 d) noexcept {
    // "Practical HDR and Wide Color Techniques in Gran Turismo SPORT", Uchimura 2018
    float3 const  dark = pow(v, shadowGamma) * d;
    float3 const  light = highlightScale * (v - midPoint) + midPoint;
    return float3{
        v.r <= midPoint.r ? dark.r : light.r,
        v.g <= midPoint.g ? dark.g : light.g,
        v.b <= midPoint.b ? dark.b : light.b,
    };
}

static float3 applyCustomLut(float3 v, const float3* lut, uint8_t dim) noexcept {
    float3 const pos = v * float(dim - 1);
    float3 const pos_floor = floor(pos);
    float3 const pos_ceil = min(pos_floor + 1.0f, float(dim - 1));
    float3 const d = pos - pos_floor;

    int3 const i0 = int3(pos_floor);
    int3 const i1 = int3(pos_ceil);

    auto fetch = [&](int const r, int const g, int const b) {
        return lut[r + g * dim + b * dim * dim];
    };

    float3 const c000 = fetch(i0.x, i0.y, i0.z);
    float3 const c100 = fetch(i1.x, i0.y, i0.z);
    float3 const c010 = fetch(i0.x, i1.y, i0.z);
    float3 const c110 = fetch(i1.x, i1.y, i0.z);
    float3 const c001 = fetch(i0.x, i0.y, i1.z);
    float3 const c101 = fetch(i1.x, i0.y, i1.z);
    float3 const c011 = fetch(i0.x, i1.y, i1.z);
    float3 const c111 = fetch(i1.x, i1.y, i1.z);

    float3 const c00 = c000 * (1.0f - d.x) + c100 * d.x;
    float3 const c10 = c010 * (1.0f - d.x) + c110 * d.x;
    float3 const c01 = c001 * (1.0f - d.x) + c101 * d.x;
    float3 const c11 = c011 * (1.0f - d.x) + c111 * d.x;

    float3 const c0 = c00 * (1.0f - d.y) + c10 * d.y;
    float3 const c1 = c01 * (1.0f - d.y) + c11 * d.y;

    return c0 * (1.0f - d.z) + c1 * d.z;
}

//------------------------------------------------------------------------------
// Luminance scaling
//------------------------------------------------------------------------------

static float3 luminanceScaling(float3 x,
        const ToneMapper& toneMapper, float3 luminanceWeights) noexcept {

    // Troy Sobotka, 2021, "EVILS - Exposure Value Invariant Luminance Scaling"
    // https://colab.research.google.com/drive/1iPJzNNKR7PynFmsqSnQm3bCZmQ3CvAJ-#scrollTo=psU43hb-BLzB

    float const luminanceIn = dot(x, luminanceWeights);

    // TODO: We could optimize for the case of single-channel luminance
    float const luminanceOut = toneMapper(luminanceIn).y;

    float const peak = max(x);
    float3 const chromaRatio = max(x / peak, 0.0f);

    float const chromaRatioLuminance = dot(chromaRatio, luminanceWeights);

    float3 const maxReserves = 1.0f - chromaRatio;
    float const maxReservesLuminance = dot(maxReserves, luminanceWeights);

    float const luminanceDifference = std::max(luminanceOut - chromaRatioLuminance, 0.0f);
    float const scaledLuminanceDifference =
            luminanceDifference / std::max(maxReservesLuminance, std::numeric_limits<float>::min());

    float const chromaScale = (luminanceOut - luminanceDifference) /
            std::max(chromaRatioLuminance, std::numeric_limits<float>::min());

    return chromaScale * chromaRatio + scaledLuminanceDifference * maxReserves;
}

#if defined(__ARM_NEON)
static inline void luminanceScalingNeon(float32x4_t& vr, float32x4_t& vg, float32x4_t& vb,
        const ToneMapper& toneMapper, float3 const luminanceWeights) noexcept {
    float32x4_t const w_r = vdupq_n_f32(luminanceWeights.r);
    float32x4_t const w_g = vdupq_n_f32(luminanceWeights.g);
    float32x4_t const w_b = vdupq_n_f32(luminanceWeights.b);
    float32x4_t const zero = vdupq_n_f32(0.0f);
    float32x4_t const one = vdupq_n_f32(1.0f);
    float32x4_t const flt_min = vdupq_n_f32(std::numeric_limits<float>::min());

    // luminanceIn = dot(x, luminanceWeights)
    float32x4_t const luminanceIn = vmlaq_f32(vmlaq_f32(vmulq_f32(vr, w_r), vg, w_g), vb, w_b);

    // luminanceOut = toneMapper(luminanceIn).y
    float32x4_t tm_r = luminanceIn;
    float32x4_t tm_g = luminanceIn;
    float32x4_t tm_b = luminanceIn;
    toneMapper(tm_r, tm_g, tm_b);
    float32x4_t const luminanceOut = tm_g;

    // peak = max(x)
    float32x4_t const peak = vmaxq_f32(vr, vmaxq_f32(vg, vb));

    // chromaRatio = max(x / peak, 0.0f)
    float32x4_t const safe_peak = vmaxq_f32(peak, flt_min);
    float32x4_t const cr_r = vmaxq_f32(vdivq_f32(vr, safe_peak), zero);
    float32x4_t const cr_g = vmaxq_f32(vdivq_f32(vg, safe_peak), zero);
    float32x4_t const cr_b = vmaxq_f32(vdivq_f32(vb, safe_peak), zero);

    // chromaRatioLuminance = dot(chromaRatio, luminanceWeights)
    float32x4_t const cr_lum = vmlaq_f32(vmlaq_f32(vmulq_f32(cr_r, w_r), cr_g, w_g), cr_b, w_b);

    // maxReserves = 1.0f - chromaRatio
    float32x4_t const mr_r = vsubq_f32(one, cr_r);
    float32x4_t const mr_g = vsubq_f32(one, cr_g);
    float32x4_t const mr_b = vsubq_f32(one, cr_b);

    // maxReservesLuminance = dot(maxReserves, luminanceWeights)
    float32x4_t const mr_lum = vmlaq_f32(vmlaq_f32(vmulq_f32(mr_r, w_r), mr_g, w_g), mr_b, w_b);

    // luminanceDifference = max(luminanceOut - chromaRatioLuminance, 0.0f)
    float32x4_t const lum_diff = vmaxq_f32(vsubq_f32(luminanceOut, cr_lum), zero);

    // scaledLuminanceDifference = luminanceDifference / max(maxReservesLuminance, flt_min)
    float32x4_t const scaled_lum_diff = vdivq_f32(lum_diff, vmaxq_f32(mr_lum, flt_min));

    // chromaScale = (luminanceOut - luminanceDifference) / max(chromaRatioLuminance, flt_min)
    float32x4_t const chromaScale = vdivq_f32(vsubq_f32(luminanceOut, lum_diff), vmaxq_f32(cr_lum, flt_min));

    // result = chromaScale * chromaRatio + scaledLuminanceDifference * maxReserves
    vr = vmlaq_f32(vmulq_f32(chromaScale, cr_r), scaled_lum_diff, mr_r);
    vg = vmlaq_f32(vmulq_f32(chromaScale, cr_g), scaled_lum_diff, mr_g);
    vb = vmlaq_f32(vmulq_f32(chromaScale, cr_b), scaled_lum_diff, mr_b);
}
#endif


//------------------------------------------------------------------------------
// Quality
//------------------------------------------------------------------------------

static std::tuple<TextureFormat, PixelDataFormat, PixelDataType> selectLutTextureParams(
        ColorGrading::LutFormat const lutFormat, const bool isOneDimensional) noexcept {
    if (isOneDimensional) {
        return { TextureFormat::R16F, PixelDataFormat::R, PixelDataType::HALF };
    }
    // We use RGBA16F for high quality modes instead of RGB16F because RGB16F
    // is not supported everywhere
    switch (lutFormat) {
        case ColorGrading::LutFormat::INTEGER:
            return { TextureFormat::RGB10_A2, PixelDataFormat::RGBA, PixelDataType::UINT_2_10_10_10_REV };
        case ColorGrading::LutFormat::FLOAT:
            return { TextureFormat::RGBA16F, PixelDataFormat::RGBA, PixelDataType::HALF };
    }
    return {};
}

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
// The following functions exist to preserve backward compatibility with the
// `FILMIC` set via the deprecated `ToneMapping` API. Selecting `ToneMapping::FILMIC`
// forces post-processing to be performed in sRGB to guarantee that the inverse tone
// mapping function in the shaders will match the forward tone mapping step exactly.

static mat3f selectColorGradingTransformIn(ColorGrading::ToneMapping const toneMapping) noexcept {
    if (toneMapping == ColorGrading::ToneMapping::FILMIC) {
        return mat3f{};
    }
    return sRGB_to_Rec2020;
}

static mat3f selectColorGradingTransformOut(ColorGrading::ToneMapping const toneMapping) noexcept {
    if (toneMapping == ColorGrading::ToneMapping::FILMIC) {
        return mat3f{};
    }
    return Rec2020_to_sRGB;
}

static float3 selectColorGradingLuminance(ColorGrading::ToneMapping const toneMapping) noexcept {
    if (toneMapping == ColorGrading::ToneMapping::FILMIC) {
        return LUMINANCE_Rec709;
    }
    return LUMINANCE_Rec2020;
}
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

using ColorTransform = float3(*)(float3);

static ColorTransform selectOETF(const ColorSpace& colorSpace) noexcept {
    if (colorSpace.getTransferFunction() == Linear) {
        return OETF_Linear;
    }
    return OETF_sRGB;
}

//------------------------------------------------------------------------------
// Color grading implementation
//------------------------------------------------------------------------------

struct FColorGrading::Config {
    size_t lutDimension{};
    mat3f  adaptationTransform;
    mat3f  colorGradingIn;
    mat3f  colorGradingOut;
    float3 colorGradingLuminance{};
    ColorTransform oetf;

    float precomputedLinear[512]{};
    float3 precomputedInR[512]{};
    float3 precomputedInG[512]{};
    float3 precomputedInB[512]{};
    mat3f combinedInTransform;
    float3 curvesDenominator{1.0f};
    float contrastOffset{0.0f};
};

// Inside the FColorGrading constructor, TSAN sporadically detects a data race on the config struct;
// the Filament thread writes and the Job thread reads. In practice there should be no data race, so
// we force TSAN off to silence the warning.
UTILS_NO_SANITIZE_THREAD
FColorGrading::FColorGrading(FEngine& engine, const Builder& builder) {
    FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_FILAMENT);

    DriverApi& driver = engine.getDriverApi();

    // XXX: The following two conditions also only hold true as long as the input and output color
    // spaces are the same, but we currently don't check that. We must revise these conditions if we
    // ever handle this case.
    mIsOneDimensional = !builder->hasAdjustments && !builder->luminanceScaling
            && builder->customLutData.empty()
            && builder->toneMapper->isOneDimensional()
            && engine.features.engine.color_grading.use_1d_lut;
    mIsLDR = mIsOneDimensional && builder->toneMapper->isLDR();

    Config config = {
        mIsOneDimensional ? 512u : builder->dimension,
        adaptationTransform(builder->whiteBalance),
        selectColorGradingTransformIn(builder->toneMapping),
        selectColorGradingTransformOut(builder->toneMapping),
        selectColorGradingLuminance(builder->toneMapping),
        selectOETF(builder->outputColorSpace),
    };

    float const expScale = builder->hasAdjustments ? std::exp2(builder->exposure) : 1.0f;
    for (size_t i = 0; i < config.lutDimension; i++) {
        float val = float(i) * (1.0f / float(config.lutDimension - 1u));
        val = LogC_to_linear(float3{val}).x;
        val = std::max(val, 0.0f);
        config.precomputedLinear[i] = val * expScale;

        config.precomputedInR[i] = config.colorGradingIn[0] * val;
        config.precomputedInG[i] = config.colorGradingIn[1] * val;
        config.precomputedInB[i] = config.colorGradingIn[2] * val;
    }

    config.combinedInTransform = config.adaptationTransform * config.colorGradingIn;
    config.curvesDenominator = 1.0f / pow(builder->midPoint, builder->shadowGamma - 1.0f);
    config.contrastOffset = MIDDLE_GRAY_ACEScct * (1.0f - builder->contrast);

    mDimension = config.lutDimension;

    uint32_t width;
    uint32_t height;
    uint32_t depth;
    if (mIsOneDimensional) {
        width = config.lutDimension;
        height = 1;
        depth = 1;
    } else {
        width = config.lutDimension;
        height = config.lutDimension;
        depth = config.lutDimension;
    }

    size_t const lutElementCount = width * height * depth;
    size_t elementSize = mIsOneDimensional ? sizeof(half) : sizeof(half4);
    void* data = malloc(lutElementCount * elementSize);

    auto [textureFormat, format, type] =
            selectLutTextureParams(builder->format, mIsOneDimensional);
    assert_invariant(FTexture::isTextureFormatSupported(engine, textureFormat));
    assert_invariant(FTexture::validatePixelFormatAndType(textureFormat, format, type));

#if defined(__ARM_NEON)
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
    bool const isSupportedType = type == PixelDataType::UINT_2_10_10_10_REV;

    bool const isDefaultState = !mIsOneDimensional &&
                                !builder->hasAdjustments &&
                                !builder->customToneMapper &&
                                !builder->luminanceScaling &&
                                !builder->gamutMapping &&
                                builder->customLutData.empty() &&
                                builder->fastMath &&
                                engine.features.engine.color_grading.use_optimized_default_builder &&
                                builder->toneMapping == ToneMapping::ACES_LEGACY && 
                                builder->outputColorSpace == Rec709-sRGB-D65 &&
                                type == PixelDataType::UINT_2_10_10_10_REV &&
                                (config.lutDimension & (config.lutDimension - 1)) == 0 &&
                                (config.lutDimension * config.lutDimension * config.lutDimension) % 4 == 0;

    bool const isMediumState = !isDefaultState &&
                               !mIsOneDimensional &&
                               builder->fastMath &&
                               engine.features.engine.color_grading.use_optimized_default_builder &&
                               builder->outputColorSpace == Rec709-sRGB-D65 &&
                               isSupportedType &&
                               (config.lutDimension & (config.lutDimension - 1)) == 0 &&
                               (config.lutDimension * config.lutDimension * config.lutDimension) % 4 == 0;
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
#endif

    if (mIsOneDimensional) {
        // 1D LUT currently always use fp16
        half* UTILS_RESTRICT p = static_cast<half*>(data);
        if (mIsLDR) {
            auto toneMapper = builder->toneMapper;
            for (uint32_t rgb = 0, c = config.lutDimension; rgb < c; rgb++) {
                float3 v = float3(rgb) * (1.0f / float(config.lutDimension - 1u));

                v = (*toneMapper)(float3(v));

                // We need to clamp for the output transfer function
                v = saturate(v);

                // Apply OETF
                v = config.oetf(v);

                *p++ = half(v.r);
            }
        } else {
            for (uint32_t rgb = 0, c = config.lutDimension; rgb < c; rgb++) {
                *p++ = half(hdrColorAt(builder, config, rgb, rgb, rgb).r);
            }
        }
#if defined(__ARM_NEON)
    } else if (isDefaultState) {
        generateDefaultLUTNeon(engine, data, config, builder);
        elementSize = sizeof(uint32_t);
    } else if (isMediumState) {
        generateMediumLUTNeon(engine, data, config, builder);
        elementSize = sizeof(uint32_t);
#endif
    } else {
        JobSystem& js = engine.getJobSystem();
        auto *slices = js.createJob();
        uint32_t const dim = config.lutDimension;

        for (uint32_t b = 0; b < dim; b++) {
            auto work = [data, b, type, &config, &builder](JobSystem&, JobSystem::Job*) {
                FILAMENT_TRACING_NAME(FILAMENT_TRACING_CATEGORY_FILAMENT, "ColorGrading::job");
                uint32_t const dim = config.lutDimension;
                uint32_t const sliceCount = dim * dim;

                constexpr uint32_t TILE_SIZE = 64;
                float4 tile_buffer[TILE_SIZE];

                for (uint32_t tile = 0; tile < sliceCount; tile += TILE_SIZE) {
                    uint32_t const end = std::min(tile + TILE_SIZE, sliceCount);
                    for (uint32_t i = tile, j = 0; i < end; ++i, ++j) {
                        uint32_t const g = i / dim;
                        uint32_t const r = i % dim;
                        tile_buffer[j] = hdrColorAt(builder, config, r, g, b);
                    }

                    if (type == PixelDataType::UINT_2_10_10_10_REV) {
                        uint32_t* UTILS_RESTRICT const dst = static_cast<uint32_t*>(data) + (b * sliceCount) + tile;
#if defined(__clang__)
#pragma clang loop vectorize_width(8)
#endif
                        for (uint32_t j = 0; j < end - tile; ++j) {
                            float4 const v = tile_buffer[j];
                            uint32_t const pr = uint32_t(std::floor(v.x * 1023.0f + 0.5f));
                            uint32_t const pg = uint32_t(std::floor(v.y * 1023.0f + 0.5f));
                            uint32_t const pb = uint32_t(std::floor(v.z * 1023.0f + 0.5f));
                            dst[j] = (pb << 20u) | (pg << 10u) | pr;
                        }
                    } else {
                        half4* UTILS_RESTRICT const dst = static_cast<half4*>(data) + (b * sliceCount) + tile;
                        for (uint32_t j = 0; j < end - tile; ++j) {
                            dst[j] = half4{tile_buffer[j]};
                        }
                    }
                }
            };

            // It doesn't seem to be worth it to spin up the jobsystem for 32^3 LUTs, we do get a benefit
            // in mobile (e.g. ~6ms to ~4ms), but at the price of spinning all medium cores.
            // For 64^3 however, we're improving from 112ms to 40ms (Pixel 8)
            if (UTILS_UNLIKELY(dim > 32)) {
                auto* job = js.createJob(slices, work);
                js.run(job);
            } else {
                work(js, nullptr);
            }
        }
        js.runAndWait(slices);

        elementSize = type == PixelDataType::UINT_2_10_10_10_REV ? sizeof(uint32_t) : sizeof(half4);
    }

    if (builder->exportCallback) {
        builder->exportCallback(data, lutElementCount * elementSize, format, type, width, height, depth, builder->exportUser);
    }

    // Create texture.
    mLutHandle = driver.createTexture(SamplerType::SAMPLER_3D, 1, textureFormat, 1,
            width, height, depth, TextureUsage::DEFAULT);

    driver.update3DImage(mLutHandle, 0,
            0, 0, 0,
            width, height, depth,
            PixelBufferDescriptor{
                data, lutElementCount * elementSize, format, type,
                [](void* buffer, size_t, void*) { free(buffer); }
            });
}

FColorGrading::~FColorGrading() noexcept = default;

void FColorGrading::terminate(FEngine& engine) {
    DriverApi& driver = engine.getDriverApi();
    driver.destroyTexture(mLutHandle);
}

float4 FColorGrading::hdrColorAt(Builder const& builder, Config const& config,
        size_t r, size_t g, size_t b) noexcept {

    float3 v;
    if (UTILS_UNLIKELY(builder->hasAdjustments)) {
        v = float3{config.precomputedLinear[r], config.precomputedLinear[g], config.precomputedLinear[b]};

        // Purkinje shift ("low-light" vision)
        v = scotopicAdaptation(v, builder->nightAdaptation);

        // Move to color grading color space and white balance (fused)
        v = config.combinedInTransform * v;

        // Kill negative values before the next transforms
        v = max(v, 0.0f);

        // Channel mixer
        v = channelMixer(v, builder->outRed, builder->outGreen, builder->outBlue);

        // Shadows/mid-tones/highlights
        v = tonalRanges(v, config.colorGradingLuminance,
                builder->shadows, builder->midtones, builder->highlights,
                builder->tonalRanges);

        // The adjustments below behave better in log space
        v = linear_to_LogC(v);

        // ASC CDL
        v = colorDecisionList(v, builder->slope, builder->offset, builder->power);

        // Contrast in log space
        v = contrast(v, builder->contrast, config.contrastOffset);

        // Back to linear space
        v = LogC_to_linear(v);

        // Vibrance in linear space
        v = vibrance(v, config.colorGradingLuminance, builder->vibrance);

        // Saturation in linear space
        v = saturation(v, config.colorGradingLuminance, builder->saturation);

        // Kill negative values before curves
        v = max(v, 0.0f);

        // RGB curves
        v = curves(v, builder->shadowGamma, builder->midPoint, builder->highlightScale, config.curvesDenominator);
    } else {
        v = config.precomputedInR[r] + config.precomputedInG[g] + config.precomputedInB[b];
    }

    // Tone mapping
    if (UTILS_UNLIKELY(builder->luminanceScaling)) {
        v = luminanceScaling(v, *builder->toneMapper, config.colorGradingLuminance);
    } else {
        v = (*builder->toneMapper)(v);
    }

    // Go back to display color space
    v = config.colorGradingOut * v;

    // Apply gamut mapping
    if (UTILS_UNLIKELY(builder->gamutMapping)) {
        // TODO: This should depend on the output color space
        v = gamutMapping_sRGB(v);
    }

    // TODO: We should convert to the output color space if we use a working color space that's not sRGB
    // TODO: Allow the user to customize the output color space

    // We need to clamp for the output transfer function
    v = saturate(v);

    // Apply OETF
    v = config.oetf(v);

    // Apply custom LUT if provided
    if (UTILS_UNLIKELY(!builder->customLutData.empty())) {
        v = applyCustomLut(v, builder->customLutData.data(), builder->customLutDimension);
    }

    return {v, 0.0f};
}

#if defined(__ARM_NEON)

UTILS_NOINLINE
void FColorGrading::generateDefaultLUTNeon(FEngine const& engine, void* data,
        Config const& config, Builder const& builder) noexcept {
    FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_FILAMENT);

    uint32_t const dim = config.lutDimension;
    assert_invariant((dim & (dim - 1)) == 0); // dim is power of 2

    JobSystem& js = engine.getJobSystem();
    auto *slices = js.createJob();

    auto const toneMapper = static_cast<const ACESLegacyToneMapper*>(builder->toneMapper);
    for (uint32_t b = 0; b < dim; b++) {
        auto work = [data, b, &config, toneMapper](JobSystem&, JobSystem::Job*) {
            FILAMENT_TRACING_NAME(FILAMENT_TRACING_CATEGORY_FILAMENT, "ColorGrading::jobDefaultNeon");
            uint32_t const dim = config.lutDimension;
            uint32_t const mask = dim - 1;
            uint32_t const shift = __builtin_ctz(dim);
            uint32_t const sliceCount = dim * dim;
            uint32_t const baseIndex = b << (shift * 2);
            uint32_t* UTILS_RESTRICT const p = static_cast<uint32_t*>(data) + (b * sliceCount);

            for (uint32_t i = 0; i < sliceCount; i += 4) {
                uint32_t const i0 = baseIndex + i;
                uint32_t const i1 = i0 + 1;
                uint32_t const i2 = i0 + 2;
                uint32_t const i3 = i0 + 3;

                uint32_t const r0 = i0 & mask;
                uint32_t const g0 = (i0 >> shift) & mask;
                uint32_t const b0 = b;
                uint32_t const r1 = i1 & mask;
                uint32_t const g1 = (i1 >> shift) & mask;
                uint32_t const b1 = b;
                uint32_t const r2 = i2 & mask;
                uint32_t const g2 = (i2 >> shift) & mask;
                uint32_t const b2 = b;
                uint32_t const r3 = i3 & mask;
                uint32_t const g3 = (i3 >> shift) & mask;
                uint32_t const b3 = b;

                float3 const v0 = config.precomputedInR[r0] + config.precomputedInG[g0] + config.precomputedInB[b0];
                float3 const v1 = config.precomputedInR[r1] + config.precomputedInG[g1] + config.precomputedInB[b1];
                float3 const v2 = config.precomputedInR[r2] + config.precomputedInG[g2] + config.precomputedInB[b2];
                float3 const v3 = config.precomputedInR[r3] + config.precomputedInG[g3] + config.precomputedInB[b3];

                float32x4_t vr = {v0.r, v1.r, v2.r, v3.r};
                float32x4_t vg = {v0.g, v1.g, v2.g, v3.g};
                float32x4_t vb = {v0.b, v1.b, v2.b, v3.b};

                toneMapper->ACESLegacyToneMapper::operator()(vr, vg, vb);

                // config.colorGradingOut
                float32x4_t cg_r = vmlaq_n_f32(vmlaq_n_f32(vmulq_n_f32(vr, config.colorGradingOut[0][0]), vg, config.colorGradingOut[1][0]), vb, config.colorGradingOut[2][0]);
                float32x4_t cg_g = vmlaq_n_f32(vmlaq_n_f32(vmulq_n_f32(vr, config.colorGradingOut[0][1]), vg, config.colorGradingOut[1][1]), vb, config.colorGradingOut[2][1]);
                float32x4_t cg_b = vmlaq_n_f32(vmlaq_n_f32(vmulq_n_f32(vr, config.colorGradingOut[0][2]), vg, config.colorGradingOut[1][2]), vb, config.colorGradingOut[2][2]);

                // saturate
                cg_r = vmaxq_f32(vminq_f32(cg_r, vdupq_n_f32(1.0f)), vdupq_n_f32(0.0f));
                cg_g = vmaxq_f32(vminq_f32(cg_g, vdupq_n_f32(1.0f)), vdupq_n_f32(0.0f));
                cg_b = vmaxq_f32(vminq_f32(cg_b, vdupq_n_f32(1.0f)), vdupq_n_f32(0.0f));

                v_oetf_sRGB(cg_r, cg_g, cg_b);

                uint32x4_t const r_int = vcvtnq_u32_f32(vmulq_n_f32(cg_r, 1023.0f));
                uint32x4_t const g_int = vcvtnq_u32_f32(vmulq_n_f32(cg_g, 1023.0f));
                uint32x4_t const b_int = vcvtnq_u32_f32(vmulq_n_f32(cg_b, 1023.0f));

                uint32x4_t const packed = vorrq_u32(vorrq_u32(vshlq_n_u32(b_int, 20), vshlq_n_u32(g_int, 10)), r_int);
                vst1q_u32(p + i, packed);
            }
        };

        if (UTILS_UNLIKELY(dim > 32)) {
            auto* job = js.createJob(slices, work);
            js.run(job);
        } else {
            work(js, nullptr);
        }
    }
    js.runAndWait(slices);
}

UTILS_NOINLINE
void FColorGrading::generateMediumLUTNeon(FEngine const& engine, void* data, Config const& config, Builder const& builder) noexcept {
    FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_FILAMENT);

    uint32_t const dim = config.lutDimension;
    assert_invariant((dim & (dim - 1)) == 0); // dim is power of 2

    JobSystem& js = engine.getJobSystem();
    auto *slices = js.createJob();

    for (uint32_t b = 0; b < dim; b++) {
        auto work = [data, b, &config, &builder](JobSystem&, JobSystem::Job*) {
            FILAMENT_TRACING_NAME(FILAMENT_TRACING_CATEGORY_FILAMENT, "ColorGrading::jobNeon");
            uint32_t const dim = config.lutDimension;
            uint32_t const mask = dim - 1;
            uint32_t const shift = __builtin_ctz(dim);
            uint32_t const sliceCount = dim * dim;
            uint32_t const baseIndex = b << (shift * 2);
            bool const hasAdj = builder->hasAdjustments;
            auto const toneMapper = builder->toneMapper;

            constexpr uint32_t TILE_SIZE = 64;
            alignas(16) float tile_r[TILE_SIZE];
            alignas(16) float tile_g[TILE_SIZE];
            alignas(16) float tile_b[TILE_SIZE];

            for (uint32_t tile = 0; tile < sliceCount; tile += TILE_SIZE) {
                uint32_t const end = std::min(tile + TILE_SIZE, sliceCount);
                uint32_t const count = end - tile;

                // Loop 1: Pre-ToneMapping Adjustments (Pure NEON)
                for (uint32_t i = tile, j = 0; j < count; i += 4, j += 4) {
                    uint32_t const i0 = baseIndex + i;
                    uint32_t const i1 = i0 + 1;
                    uint32_t const i2 = i0 + 2;
                    uint32_t const i3 = i0 + 3;

                    uint32_t const r0 = i0 & mask;
                    uint32_t const g0 = (i0 >> shift) & mask;
                    uint32_t const b0 = b;
                    uint32_t const r1 = i1 & mask;
                    uint32_t const g1 = (i1 >> shift) & mask;
                    uint32_t const b1 = b;
                    uint32_t const r2 = i2 & mask;
                    uint32_t const g2 = (i2 >> shift) & mask;
                    uint32_t const b2 = b;
                    uint32_t const r3 = i3 & mask;
                    uint32_t const g3 = (i3 >> shift) & mask;
                    uint32_t const b3 = b;

                    float32x4_t vr, vg, vb;

                    if (hasAdj) {
                        float32x4_t in_r = {config.precomputedLinear[r0], config.precomputedLinear[r1], config.precomputedLinear[r2], config.precomputedLinear[r3]};
                        float32x4_t in_g = {config.precomputedLinear[g0], config.precomputedLinear[g1], config.precomputedLinear[g2], config.precomputedLinear[g3]};
                        float32x4_t in_b = {config.precomputedLinear[b0], config.precomputedLinear[b1], config.precomputedLinear[b2], config.precomputedLinear[b3]};

                        colorGradingAdjustmentsNeon(in_r, in_g, in_b, config, builder);

                        vr = in_r;
                        vg = in_g;
                        vb = in_b;
                    } else {
                        float3 const v0 = config.precomputedInR[r0] + config.precomputedInG[g0] + config.precomputedInB[b0];
                        float3 const v1 = config.precomputedInR[r1] + config.precomputedInG[g1] + config.precomputedInB[b1];
                        float3 const v2 = config.precomputedInR[r2] + config.precomputedInG[g2] + config.precomputedInB[b2];
                        float3 const v3 = config.precomputedInR[r3] + config.precomputedInG[g3] + config.precomputedInB[b3];

                        vr = float32x4_t{v0.r, v1.r, v2.r, v3.r};
                        vg = float32x4_t{v0.g, v1.g, v2.g, v3.g};
                        vb = float32x4_t{v0.b, v1.b, v2.b, v3.b};
                    }

                    vst1q_f32(&tile_r[j], vr);
                    vst1q_f32(&tile_g[j], vg);
                    vst1q_f32(&tile_b[j], vb);
                }

                // Loop 2: NEON Tone Mapping
                bool const lumScaling = builder->luminanceScaling;
                for (uint32_t j = 0; j < count; j += 4) {
                    float32x4_t vr = vld1q_f32(&tile_r[j]);
                    float32x4_t vg = vld1q_f32(&tile_g[j]);
                    float32x4_t vb = vld1q_f32(&tile_b[j]);
                    if (UTILS_UNLIKELY(lumScaling)) {
                        luminanceScalingNeon(vr, vg, vb, *toneMapper, config.colorGradingLuminance);
                    } else {
                        (*toneMapper)(vr, vg, vb);
                    }
                    vst1q_f32(&tile_r[j], vr);
                    vst1q_f32(&tile_g[j], vg);
                    vst1q_f32(&tile_b[j], vb);
                }

                // Loop 3: Post-ToneMapping NEON Math
                for (uint32_t j = 0; j < count; j += 4) {
                    float32x4_t const tm_r = vld1q_f32(&tile_r[j]);
                    float32x4_t const tm_g = vld1q_f32(&tile_g[j]);
                    float32x4_t const tm_b = vld1q_f32(&tile_b[j]);

                    float32x4_t cg_r = vmlaq_n_f32(vmlaq_n_f32(vmulq_n_f32(tm_r, config.colorGradingOut[0][0]), tm_g, config.colorGradingOut[1][0]), tm_b, config.colorGradingOut[2][0]);
                    float32x4_t cg_g = vmlaq_n_f32(vmlaq_n_f32(vmulq_n_f32(tm_r, config.colorGradingOut[0][1]), tm_g, config.colorGradingOut[1][1]), tm_b, config.colorGradingOut[2][1]);
                    float32x4_t cg_b = vmlaq_n_f32(vmlaq_n_f32(vmulq_n_f32(tm_r, config.colorGradingOut[0][2]), tm_g, config.colorGradingOut[1][2]), tm_b, config.colorGradingOut[2][2]);

                    if (UTILS_UNLIKELY(builder->gamutMapping)) {
                        auto process = [](float const r, float const g, float const b) {
                            return gamutMapping_sRGB(float3{r, g, b});
                        };

                        float3 const p0 = process(vgetq_lane_f32(cg_r, 0), vgetq_lane_f32(cg_g, 0), vgetq_lane_f32(cg_b, 0));
                        float3 const p1 = process(vgetq_lane_f32(cg_r, 1), vgetq_lane_f32(cg_g, 1), vgetq_lane_f32(cg_b, 1));
                        float3 const p2 = process(vgetq_lane_f32(cg_r, 2), vgetq_lane_f32(cg_g, 2), vgetq_lane_f32(cg_b, 2));
                        float3 const p3 = process(vgetq_lane_f32(cg_r, 3), vgetq_lane_f32(cg_g, 3), vgetq_lane_f32(cg_b, 3));

                        cg_r = float32x4_t{p0.r, p1.r, p2.r, p3.r};
                        cg_g = float32x4_t{p0.g, p1.g, p2.g, p3.g};
                        cg_b = float32x4_t{p0.b, p1.b, p2.b, p3.b};
                    }

                    cg_r = vmaxq_f32(vminq_f32(cg_r, vdupq_n_f32(1.0f)), vdupq_n_f32(0.0f));
                    cg_g = vmaxq_f32(vminq_f32(cg_g, vdupq_n_f32(1.0f)), vdupq_n_f32(0.0f));
                    cg_b = vmaxq_f32(vminq_f32(cg_b, vdupq_n_f32(1.0f)), vdupq_n_f32(0.0f));

                    v_oetf_sRGB(cg_r, cg_g, cg_b);

                    if (UTILS_UNLIKELY(!builder->customLutData.empty())) {
                        auto const* clData = builder->customLutData.data();
                        uint8_t const clDim = builder->customLutDimension;

                        auto process = [clData, clDim](float const r, float const g, float const b) {
                            return applyCustomLut(float3{r, g, b}, clData, clDim);
                        };

                        float3 const p0 = process(vgetq_lane_f32(cg_r, 0), vgetq_lane_f32(cg_g, 0), vgetq_lane_f32(cg_b, 0));
                        float3 const p1 = process(vgetq_lane_f32(cg_r, 1), vgetq_lane_f32(cg_g, 1), vgetq_lane_f32(cg_b, 1));
                        float3 const p2 = process(vgetq_lane_f32(cg_r, 2), vgetq_lane_f32(cg_g, 2), vgetq_lane_f32(cg_b, 2));
                        float3 const p3 = process(vgetq_lane_f32(cg_r, 3), vgetq_lane_f32(cg_g, 3), vgetq_lane_f32(cg_b, 3));

                        cg_r = float32x4_t{p0.r, p1.r, p2.r, p3.r};
                        cg_g = float32x4_t{p0.g, p1.g, p2.g, p3.g};
                        cg_b = float32x4_t{p0.b, p1.b, p2.b, p3.b};
                    }

                    vst1q_f32(&tile_r[j], cg_r);
                    vst1q_f32(&tile_g[j], cg_g);
                    vst1q_f32(&tile_b[j], cg_b);
                }

                // Loop 4: Fused Packing Epilogue
                uint32_t* UTILS_RESTRICT const p = static_cast<uint32_t*>(data) + (b * sliceCount);
                for (uint32_t j = 0; j < count; j += 4) {
                    float32x4_t const cg_r = vld1q_f32(&tile_r[j]);
                    float32x4_t const cg_g = vld1q_f32(&tile_g[j]);
                    float32x4_t const cg_b = vld1q_f32(&tile_b[j]);

                    uint32x4_t const r_int = vcvtnq_u32_f32(vmulq_n_f32(cg_r, 1023.0f));
                    uint32x4_t const g_int = vcvtnq_u32_f32(vmulq_n_f32(cg_g, 1023.0f));
                    uint32x4_t const b_int = vcvtnq_u32_f32(vmulq_n_f32(cg_b, 1023.0f));

                    uint32x4_t const packed = vorrq_u32(vorrq_u32(vshlq_n_u32(b_int, 20), vshlq_n_u32(g_int, 10)), r_int);
                    vst1q_u32(p + tile + j, packed);
                }
            }
        };

        if (UTILS_UNLIKELY(dim > 32)) {
            auto* job = js.createJob(slices, work);
            js.run(job);
        } else {
            work(js, nullptr);
        }
    }
    js.runAndWait(slices);
}

UTILS_ALWAYS_INLINE
inline void FColorGrading::colorGradingAdjustmentsNeon(float32x4_t& vr, float32x4_t& vg, float32x4_t& vb,
        Config const& config, Builder const& builder) noexcept {
    float32x4_t in_r = vr;
    float32x4_t in_g = vg;
    float32x4_t in_b = vb;

    if (UTILS_UNLIKELY(builder->nightAdaptation != 0.0f)) {
        v_scotopicAdaptation(in_r, in_g, in_b, builder->nightAdaptation);
    }

    // config.combinedInTransform * v
    float32x4_t c_r = vmlaq_n_f32(vmlaq_n_f32(vmulq_n_f32(in_r, config.combinedInTransform[0][0]), in_g, config.combinedInTransform[1][0]), in_b, config.combinedInTransform[2][0]);
    float32x4_t c_g = vmlaq_n_f32(vmlaq_n_f32(vmulq_n_f32(in_r, config.combinedInTransform[0][1]), in_g, config.combinedInTransform[1][1]), in_b, config.combinedInTransform[2][1]);
    float32x4_t c_b = vmlaq_n_f32(vmlaq_n_f32(vmulq_n_f32(in_r, config.combinedInTransform[0][2]), in_g, config.combinedInTransform[1][2]), in_b, config.combinedInTransform[2][2]);

    // max(v, 0.0f)
    c_r = vmaxq_f32(c_r, vdupq_n_f32(0.0f));
    c_g = vmaxq_f32(c_g, vdupq_n_f32(0.0f));
    c_b = vmaxq_f32(c_b, vdupq_n_f32(0.0f));

    // channelMixer
    v_channelMixer(c_r, c_g, c_b, builder->outRed, builder->outGreen, builder->outBlue);

    // tonalRanges
    v_tonalRanges(c_r, c_g, c_b, config.colorGradingLuminance, builder->tonalRanges, builder->shadows, builder->midtones, builder->highlights);

    // linear_to_LogC
    float32x4_t const logc_r = v_toLogC(c_r);
    float32x4_t const logc_g = v_toLogC(c_g);
    float32x4_t const logc_b = v_toLogC(c_b);

    // ASC CDL
    float32x4_t cdl_r = v_cdl(logc_r, builder->slope.r, builder->offset.r, builder->power.r);
    float32x4_t cdl_g = v_cdl(logc_g, builder->slope.g, builder->offset.g, builder->power.g);
    float32x4_t cdl_b = v_cdl(logc_b, builder->slope.b, builder->offset.b, builder->power.b);

    // contrast
    v_contrast(cdl_r, cdl_g, cdl_b, builder->contrast, config.contrastOffset);

    // LogC_to_linear
    float32x4_t lin_r = v_toLinear(cdl_r);
    float32x4_t lin_g = v_toLinear(cdl_g);
    float32x4_t lin_b = v_toLinear(cdl_b);

    // vibrance
    v_vibrance(lin_r, lin_g, lin_b, builder->vibrance, config.colorGradingLuminance);

    // saturation
    v_saturation(lin_r, lin_g, lin_b, builder->saturation, config.colorGradingLuminance);

    // curves
    v_curves(lin_r, lin_g, lin_b, builder->shadowGamma, builder->midPoint, builder->highlightScale, config.curvesDenominator);

    vr = lin_r;
    vg = lin_g;
    vb = lin_b;
}

#endif

} //namespace filament
