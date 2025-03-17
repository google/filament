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

#include <filament/ColorSpace.h>

#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <utils/JobSystem.h>
#include <utils/Mutex.h>
#include <utils/Systrace.h>

#include <cmath>
#include <cstdlib>
#include <tuple>

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
               outputColorSpace == rhs.outputColorSpace;
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

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
ColorGrading* ColorGrading::Builder::build(Engine& engine) {
    // We want to see if any of the default adjustment values have been modified
    // We skip the tonemapping operator on purpose since we always want to apply it
    BuilderDetails defaults;
    bool hasAdjustments = defaults != *mImpl;
    mImpl->hasAdjustments = hasAdjustments;

    // Fallback for clients that still use the deprecated ToneMapping API
    bool needToneMapper = mImpl->toneMapper == nullptr;
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
// Exposure
//------------------------------------------------------------------------------

UTILS_ALWAYS_INLINE
inline float3 adjustExposure(float3 const v, float const exposure) {
    return v * std::exp2(exposure);
}

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
float3 scotopicAdaptation(float3 v, float nightAdaptation) noexcept {
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
    constexpr mat3f weightedRodResponse = (K_ / S_) * mat3f{
       -(k3 + rw),       p * k3,          p * S_,
        1.0f + k3 * rw, (1.0f - p) * k3, (1.0f - p) * S_,
        0.0f,            1.0f,            0.0f
    } * mat3f{k} * inverse(mat3f{m});

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
    float4 q{dot(v, L), dot(v, M), dot(v, S), dot(v, R)};
    // Regulated signal through the selected pathway (KC in Cao et al.)
    float3 g = inversesqrt(1.0f + max(float3{0.0f}, (0.33f / m) * (q.rgb + k * q.w)));

    // Compute the incremental effect that rods have in opponent space
    float3 deltaOpponent = weightedRodResponse * g * q.w * nightAdaptation;
    // Photopic response in LMS space
    float3 qHat = q.rgb + opponent_to_LMS * deltaOpponent;

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
constexpr mat3f adaptationTransform(float2 const whiteBalance) noexcept {
    // See Mathematica notebook in docs/math/White Balance.nb
    float k = whiteBalance.x; // temperature
    float t = whiteBalance.y; // tint

    float x = ILLUMINANT_D65_xyY[0] - k * (k < 0.0f ? 0.0214f : 0.066f);
    float y = chromaticityCoordinateIlluminantD(x) + t * 0.066f;

    float3 lms = XYZ_to_CIECAT16 * xyY_to_XYZ({x, y, 1.0f});
    return LMS_CAT16_to_Rec2020 * mat3f{ILLUMINANT_D65_LMS_CAT16 / lms} * Rec2020_to_LMS_CAT16;
}

UTILS_ALWAYS_INLINE
inline float3 chromaticAdaptation(float3 const v, mat3f adaptationTransform) {
    return adaptationTransform * v;
}

//------------------------------------------------------------------------------
// General color grading
//------------------------------------------------------------------------------

using ColorTransform = float3(*)(float3);

UTILS_ALWAYS_INLINE
inline constexpr float3 channelMixer(float3 v, float3 r, float3 g, float3 b) {
    return {dot(v, r), dot(v, g), dot(v, b)};
}

UTILS_ALWAYS_INLINE
inline constexpr float3 tonalRanges(
        float3 v, float3 luminance,
        float3 shadows, float3 midtones, float3 highlights,
        float4 ranges
) {
    // See the Mathematica notebook at docs/math/Shadows Midtones Highlight.nb for
    // details on how the curves were designed. The default curve values are based
    // on the defaults from the "Log" color wheels in DaVinci Resolve.
    float y = dot(v, luminance);

    // Shadows curve
    float s = 1.0f - smoothstep(ranges.x, ranges.y, y);
    // Highlights curve
    float h = smoothstep(ranges.z, ranges.w, y);
    // Mid-tones curves
    float m = 1.0f - s - h;

    return v * s * shadows + v * m * midtones + v * h * highlights;
}

UTILS_ALWAYS_INLINE
inline float3 colorDecisionList(float3 v, float3 slope, float3 offset, float3 power) {
    // Apply the ASC CSL in log space, as defined in S-2016-001
    v = v * slope + offset;
    float3 pv = pow(v, power);
    return float3{
            v.r <= 0.0f ? v.r : pv.r,
            v.g <= 0.0f ? v.g : pv.g,
            v.b <= 0.0f ? v.b : pv.b
    };
}

UTILS_ALWAYS_INLINE
inline constexpr float3 contrast(float3 const v, float const contrast) {
    // Matches contrast as applied in DaVinci Resolve
    return MIDDLE_GRAY_ACEScct + contrast * (v - MIDDLE_GRAY_ACEScct);
}

UTILS_ALWAYS_INLINE
inline constexpr float3 saturation(float3 v, float3 luminance, float saturation) {
    const float3 y = dot(v, luminance);
    return y + saturation * (v - y);
}

UTILS_ALWAYS_INLINE
inline float3 vibrance(float3 v, float3 luminance, float vibrance) {
    float r = v.r - max(v.g, v.b);
    float s = (vibrance - 1.0f) / (1.0f + std::exp(-r * 3.0f)) + 1.0f;
    float3 l{(1.0f - s) * luminance};
    return float3{
        dot(v, l + float3{s, 0.0f, 0.0f}),
        dot(v, l + float3{0.0f, s, 0.0f}),
        dot(v, l + float3{0.0f, 0.0f, s}),
    };

}

UTILS_ALWAYS_INLINE
inline float3 curves(float3 v, float3 shadowGamma, float3 midPoint, float3 highlightScale) {
    // "Practical HDR and Wide Color Techniques in Gran Turismo SPORT", Uchimura 2018
    float3 d = 1.0f / (pow(midPoint, shadowGamma - 1.0f));
    float3 dark = pow(v, shadowGamma) * d;
    float3 light = highlightScale * (v - midPoint) + midPoint;
    return float3{
        v.r <= midPoint.r ? dark.r : light.r,
        v.g <= midPoint.g ? dark.g : light.g,
        v.b <= midPoint.b ? dark.b : light.b,
    };
}

//------------------------------------------------------------------------------
// Luminance scaling
//------------------------------------------------------------------------------

static float3 luminanceScaling(float3 x,
        const ToneMapper& toneMapper, float3 luminanceWeights) noexcept {

    // Troy Sobotka, 2021, "EVILS - Exposure Value Invariant Luminance Scaling"
    // https://colab.research.google.com/drive/1iPJzNNKR7PynFmsqSnQm3bCZmQ3CvAJ-#scrollTo=psU43hb-BLzB

    float luminanceIn = dot(x, luminanceWeights);

    // TODO: We could optimize for the case of single-channel luminance
    float luminanceOut = toneMapper(luminanceIn).y;

    float peak = max(x);
    float3 chromaRatio = max(x / peak, 0.0f);

    float chromaRatioLuminance = dot(chromaRatio, luminanceWeights);

    float3 maxReserves = 1.0f - chromaRatio;
    float maxReservesLuminance = dot(maxReserves, luminanceWeights);

    float luminanceDifference = std::max(luminanceOut - chromaRatioLuminance, 0.0f);
    float scaledLuminanceDifference =
            luminanceDifference / std::max(maxReservesLuminance, std::numeric_limits<float>::min());

    float chromaScale = (luminanceOut - luminanceDifference) /
            std::max(chromaRatioLuminance, std::numeric_limits<float>::min());

    return chromaScale * chromaRatio + scaledLuminanceDifference * maxReserves;
}

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

struct Config {
    size_t lutDimension{};
    mat3f  adaptationTransform;
    mat3f  colorGradingIn;
    mat3f  colorGradingOut;
    float3 colorGradingLuminance{};

    ColorTransform oetf;
};

// Inside the FColorGrading constructor, TSAN sporadically detects a data race on the config struct;
// the Filament thread writes and the Job thread reads. In practice there should be no data race, so
// we force TSAN off to silence the warning.
UTILS_NO_SANITIZE_THREAD
FColorGrading::FColorGrading(FEngine& engine, const Builder& builder) {
    SYSTRACE_CALL();

    DriverApi& driver = engine.getDriverApi();

    // XXX: The following two conditions also only hold true as long as the input and output color
    // spaces are the same, but we currently don't check that. We must revise these conditions if we
    // ever handle this case.
    mIsOneDimensional = !builder->hasAdjustments && !builder->luminanceScaling
            && builder->toneMapper->isOneDimensional();
    mIsLDR = mIsOneDimensional && builder->toneMapper->isLDR();

    const Config config = {
            mIsOneDimensional ? 512u : builder->dimension,
            adaptationTransform(builder->whiteBalance),
            selectColorGradingTransformIn(builder->toneMapping),
            selectColorGradingTransformOut(builder->toneMapping),
            selectColorGradingLuminance(builder->toneMapping),
            selectOETF(builder->outputColorSpace),
    };

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

    size_t lutElementCount = width * height * depth;
    size_t elementSize = mIsOneDimensional ? sizeof(half) : sizeof(half4);
    void* data = malloc(lutElementCount * elementSize);

    auto [textureFormat, format, type] =
            selectLutTextureParams(builder->format, mIsOneDimensional);
    assert_invariant(FTexture::isTextureFormatSupported(engine, textureFormat));
    assert_invariant(FTexture::validatePixelFormatAndType(textureFormat, format, type));

    void* converted = nullptr;
    if (type == PixelDataType::UINT_2_10_10_10_REV) {
        // convert input to UINT_2_10_10_10_REV if needed
        converted = malloc(lutElementCount * sizeof(uint32_t));
    }

    auto hdrColorAt = [builder, config](size_t r, size_t g, size_t b) {
        float3 v = float3{r, g, b} * (1.0f / float(config.lutDimension - 1u));

        // LogC encoding
        v = LogC_to_linear(v);

        // Kill negative values near 0.0f due to imprecision in the log conversion
        v = max(v, 0.0f);

        if (builder->hasAdjustments) {
            // Exposure
            v = adjustExposure(v, builder->exposure);

            // Purkinje shift ("low-light" vision)
            v = scotopicAdaptation(v, builder->nightAdaptation);
        }

        // Move to color grading color space
        v = config.colorGradingIn * v;

        if (builder->hasAdjustments) {
            // White balance
            v = chromaticAdaptation(v, config.adaptationTransform);

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
            v = contrast(v, builder->contrast);

            // Back to linear space
            v = LogC_to_linear(v);

            // Vibrance in linear space
            v = vibrance(v, config.colorGradingLuminance, builder->vibrance);

            // Saturation in linear space
            v = saturation(v, config.colorGradingLuminance, builder->saturation);

            // Kill negative values before curves
            v = max(v, 0.0f);

            // RGB curves
            v = curves(v,
                    builder->shadowGamma, builder->midPoint, builder->highlightScale);
        }

        // Tone mapping
        if (builder->luminanceScaling) {
            v = luminanceScaling(v, *builder->toneMapper, config.colorGradingLuminance);
        } else {
            v = (*builder->toneMapper)(v);
        }

        // Go back to display color space
        v = config.colorGradingOut * v;

        // Apply gamut mapping
        if (builder->gamutMapping) {
            // TODO: This should depend on the output color space
            v = gamutMapping_sRGB(v);
        }

        // TODO: We should convert to the output color space if we use a working
        //       color space that's not sRGB
        // TODO: Allow the user to customize the output color space

        // We need to clamp for the output transfer function
        v = saturate(v);

        // Apply OETF
        v = config.oetf(v);

        return v;
    };

    //auto now = std::chrono::steady_clock::now();

    if (mIsOneDimensional) {
        half* UTILS_RESTRICT p = (half*) data;
        if (mIsLDR) {
            for (size_t rgb = 0; rgb < config.lutDimension; rgb++) {
                float3 v = float3(rgb) * (1.0f / float(config.lutDimension - 1u));

                v = (*builder->toneMapper)(float3(v));

                // We need to clamp for the output transfer function
                v = saturate(v);

                // Apply OETF
                v = config.oetf(v);

                *p++ = half(v.r);
            }
        } else {
            for (size_t rgb = 0; rgb < config.lutDimension; rgb++) {
                *p = half(hdrColorAt(rgb, rgb, rgb).r);
            }
        }
    } else {
        // Multithreadedly generate the tone mapping 3D look-up table using 32 jobs
        // Slices are 8 KiB (128 cache lines) apart.
        // This takes about 3-6ms on Android in Release
        JobSystem& js = engine.getJobSystem();
        auto *slices = js.createJob();
        for (size_t b = 0; b < config.lutDimension; b++) {
            auto* job = js.createJob(slices,
                    [data, converted, b, &config, builder, &hdrColorAt](
                            JobSystem&, JobSystem::Job*) {
                half4* UTILS_RESTRICT p =
                        (half4*) data + b * config.lutDimension * config.lutDimension;
                for (size_t g = 0; g < config.lutDimension; g++) {
                    for (size_t r = 0; r < config.lutDimension; r++) {
                        *p++ = half4{hdrColorAt(r, g, b), 0.0f};
                    }
                }

                if (converted) {
                    uint32_t* const UTILS_RESTRICT dst = (uint32_t*) converted +
                            b * config.lutDimension * config.lutDimension;
                    half4* UTILS_RESTRICT src = (half4*) data +
                            b * config.lutDimension * config.lutDimension;
                    // we use a vectorize width of 8 because, on ARMv8 it allows the compiler to
                    // write eight 32-bits results in one go.
                    const size_t count = (config.lutDimension * config.lutDimension) & ~0x7u; // tell the compiler that we're a multiple of 8
#if defined(__clang__)
#pragma clang loop vectorize_width(8)
#endif
                    for (size_t i = 0; i < count; ++i) {
                        float4 v{src[i]};
                        uint32_t pr = uint32_t(std::floor(v.x * 1023.0f + 0.5f));
                        uint32_t pg = uint32_t(std::floor(v.y * 1023.0f + 0.5f));
                        uint32_t pb = uint32_t(std::floor(v.z * 1023.0f + 0.5f));
                        dst[i] = (pb << 20u) | (pg << 10u) | pr;
                    }
                }
            });
            js.run(job);
        }

        // TODO: Should we do a runAndRetain() and defer the wait() + texture creation until
        //       getHwHandle() is invoked?
        js.runAndWait(slices);
    }

    //std::chrono::duration<float, std::milli> duration = std::chrono::steady_clock::now() - now;
    //slog.d << "LUT generation time: " << duration.count() << " ms" << io::endl;

    if (converted) {
        free(data);
        data = converted;
        elementSize = sizeof(uint32_t);
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

} //namespace filament
