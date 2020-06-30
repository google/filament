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

#include "FilamentAPI-impl.h"

#include "ColorSpace.h"

#include "ToneMapping.h"

#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <utils/JobSystem.h>
#include <utils/Systrace.h>

#include <functional>

namespace filament {

using namespace utils;
using namespace math;
using namespace backend;

static constexpr size_t LUT_DIMENSION = 32u;

//------------------------------------------------------------------------------
// Builder
//------------------------------------------------------------------------------

struct ColorGrading::BuilderDetails {
    ToneMapping toneMapping = ToneMapping::ACES_LEGACY;
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
    // Keep last
    bool hasAdjustments     = false;

    bool operator!=(const BuilderDetails &rhs) const {
        return !(rhs == *this);
    }

    bool operator==(const BuilderDetails &rhs) const {
        // Note: Do NOT compare hasAdjustments
        return toneMapping == rhs.toneMapping &&
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
               highlightScale == rhs.highlightScale;
    }
};

using BuilderType = ColorGrading;
BuilderType::Builder::Builder() noexcept = default;
BuilderType::Builder::~Builder() noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder&& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder&& rhs) noexcept = default;

ColorGrading::Builder& ColorGrading::Builder::toneMapping(ToneMapping toneMapping) noexcept {
    mImpl->toneMapping = toneMapping;
    return *this;
}

ColorGrading::Builder& ColorGrading::Builder::whiteBalance(float temperature, float tint) noexcept {
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

ColorGrading::Builder& ColorGrading::Builder::contrast(float contrast) noexcept {
    mImpl->contrast = clamp(contrast, 0.0f, 2.0f);
    return *this;
}

ColorGrading::Builder& ColorGrading::Builder::vibrance(float vibrance) noexcept {
    mImpl->vibrance = clamp(vibrance, 0.0f, 2.0f);
    return *this;
}

ColorGrading::Builder& ColorGrading::Builder::saturation(float saturation) noexcept {
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

ColorGrading* ColorGrading::Builder::build(Engine& engine) {
    // We want to see if any of the default adjustment values have been modified
    // We skip the tonemapping operator on purpose since we always want to apply it
    BuilderDetails defaults;
    defaults.toneMapping = mImpl->toneMapping;
    bool hasAdjustments = defaults != *mImpl;
    mImpl->hasAdjustments = hasAdjustments;

    return upcast(engine).createColorGrading(*this);
}

//------------------------------------------------------------------------------
// White balance
//------------------------------------------------------------------------------

// Return the chromatic adaptation coefficients in LMS space for the given
// temperature/tint offsets. The chromatic adaption is perfomed following
// the von Kries method, using the CIECAT02 transform.
// See https://en.wikipedia.org/wiki/Chromatic_adaptation
// See https://en.wikipedia.org/wiki/CIECAM02#Chromatic_adaptation
UTILS_ALWAYS_INLINE
inline float3 adaptationTransform(float2 whiteBalance) {
    // See Mathematica notebook in docs/math/White Balance.nb
    float k = whiteBalance.x; // temperature
    float t = whiteBalance.y; // tint

    float x = ILLUMINANT_D65_xyY.x - k * (k < 0.0f ? 0.0214f : 0.066f);
    float y = chromaticityCoordinateIlluminantD(x) + t * 0.066f;

    float3 lms = XYZ_to_CIECAT02 * xyY_to_XYZ({x, y, 1.0f});
    return ILLUMINANT_D65_LMS / lms;
}

UTILS_ALWAYS_INLINE
inline float3 chromaticAdaptation(float3 v, float2 whiteBalance) {
    v = sRGB_to_LMS * v;
    v = adaptationTransform(whiteBalance) * v;
    v = LMS_to_sRGB * v;
    return v;
}

//------------------------------------------------------------------------------
// General color grading
//------------------------------------------------------------------------------

using ColorTransform = float3(*)(float3);

ColorTransform selectLinearToLogTransform(ColorGrading::ToneMapping toneMapping) {
    switch (toneMapping) {
        case ColorGrading::ToneMapping::ACES_LEGACY:
        case ColorGrading::ToneMapping::ACES:
            return linearAP1_to_ACEScct;
        default:
            return linear_to_LogC;
    }
}

ColorTransform selectLogToLinearTransform(ColorGrading::ToneMapping toneMapping) {
    switch (toneMapping) {
        case ColorGrading::ToneMapping::ACES_LEGACY:
        case ColorGrading::ToneMapping::ACES:
            return ACEScct_to_linearAP1;
        default:
            return LogC_to_linear;
    }
}

mat3f selectColorGradingTransformIn(ColorGrading::ToneMapping toneMapping) {
    switch (toneMapping) {
        case ColorGrading::ToneMapping::ACES_LEGACY:
        case ColorGrading::ToneMapping::ACES:
            return sRGB_to_AP1;
        case ColorGrading::ToneMapping::FILMIC:
            return mat3f{}; // stay in sRGB
        default:
            return sRGB_to_REC2020;
    }
}

mat3f selectColorGradingTransformOut(ColorGrading::ToneMapping toneMapping) {
    switch (toneMapping) {
        case ColorGrading::ToneMapping::ACES_LEGACY:
        case ColorGrading::ToneMapping::ACES:
            return AP1_to_sRGB;
        case ColorGrading::ToneMapping::FILMIC:
            return mat3f{}; // stay in sRGB
        default:
            return REC2020_to_sRGB;
    }
}

float3 selectLumaTransform(ColorGrading::ToneMapping toneMapping) {
    switch (toneMapping) {
        case ColorGrading::ToneMapping::ACES_LEGACY:
        case ColorGrading::ToneMapping::ACES:
            return LUMA_AP1;
        default:
            return LUMA_REC709;
    }
}

UTILS_ALWAYS_INLINE
inline constexpr float3 channelMixer(float3 v, float3 r, float3 g, float3 b) {
    return {dot(v, r), dot(v, g), dot(v, b)};
}

UTILS_ALWAYS_INLINE
inline constexpr float3 tonalRanges(
        float3 v, float3 luma, float3 shadows, float3 midtones, float3 highlights, float4 ranges) {
    // See the Mathematica notebook at docs/math/Shadows Midtones Highlight.nb for
    // details on how the curves were designed. The default curve values are based
    // on the defaults from the "Log" color wheels in DaVinci Resolve.
    float y = dot(v, luma);

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
inline constexpr float3 contrast(float3 v, float contrast) {
    // Matches contrast as applied in DaVinci Resolve
    return MIDDLE_GRAY_ACEScct + contrast * (v - MIDDLE_GRAY_ACEScct);
}

UTILS_ALWAYS_INLINE
inline constexpr float3 saturation(float3 v, float saturation) {
    const float3 y = dot(v, LUMA_REC709);
    return y + saturation * (v - y);
}

UTILS_ALWAYS_INLINE
inline float3 vibrance(float3 v, float vibrance) {
    float r = v.r - max(v.g, v.b);
    float s = (vibrance - 1.0f) / (1.0f + std::exp(-r * 3.0f)) + 1.0f;
    float3 l{(1.0f - s) * LUMA_REC709};
    return float3{
        dot(v, l + float3{s, 0.0f, 0.0f}),
        dot(v, l + float3{0.0f, s, 0.0f}),
        dot(v, l + float3{0.0f, 0.0f, s}),
    };

}

UTILS_ALWAYS_INLINE
inline float3 curves(float3 v, float3 shadowGamma, float3 midPoint, float3 highlightScale) {
    // "Practical HDR and Wide Color Techniques in Gran Turismo SPORT", Uchimura 2018
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
// Tone mapping
//------------------------------------------------------------------------------

ColorTransform selectToneMapping(ColorGrading::ToneMapping toneMapping) {
    switch (toneMapping) {
        case ColorGrading::ToneMapping::LINEAR:
            return tonemap::Linear;
        case ColorGrading::ToneMapping::ACES_LEGACY:
            return tonemap::ACES_Legacy;
        case ColorGrading::ToneMapping::ACES:
            return tonemap::ACES;
        case ColorGrading::ToneMapping::FILMIC:
            return tonemap::Filmic;
        case ColorGrading::ToneMapping::UCHIMURA:
            return tonemap::Uchimura;
        case ColorGrading::ToneMapping::REINHARD:
            return tonemap::Reinhard;
        case ColorGrading::ToneMapping::DISPLAY_RANGE:
            return tonemap::DisplayRange;
    }
}

//------------------------------------------------------------------------------
// Color grading implementation
//------------------------------------------------------------------------------

struct Config {
    mat3f colorGradingTransformIn;
    mat3f colorGradingTransformOut;
    float3 lumaTransform;
    ColorTransform linearToLogTransform;
    ColorTransform logToLinearTransform;
    ColorTransform toneMapper;
};

FColorGrading::FColorGrading(FEngine& engine, const Builder& builder) {
    SYSTRACE_CALL();

    DriverApi& driver = engine.getDriverApi();

    constexpr size_t lutElementCount = LUT_DIMENSION * LUT_DIMENSION * LUT_DIMENSION;
    constexpr size_t elementSize = sizeof(half4);
    void* const data = malloc(lutElementCount * elementSize);

    Config config{
        .colorGradingTransformIn  = selectColorGradingTransformIn(builder->toneMapping),
        .colorGradingTransformOut = selectColorGradingTransformOut(builder->toneMapping),
        .lumaTransform            = selectLumaTransform(builder->toneMapping),
        .linearToLogTransform     = selectLinearToLogTransform(builder->toneMapping),
        .logToLinearTransform     = selectLogToLinearTransform(builder->toneMapping),
        .toneMapper               = selectToneMapping(builder->toneMapping)
    };

    //auto now = std::chrono::steady_clock::now();

    // Multithreadedly generate the tone mapping 3D look-up table using 32 jobs
    // Slices are 8 KiB (128 cache lines) apart.
    // This takes about 3-6ms on Android in Release
    JobSystem& js = engine.getJobSystem();
    auto slices = js.createJob();
    for (size_t b = 0; b < LUT_DIMENSION; b++) {
        auto job = js.createJob(slices, [data, b, &config, builder](JobSystem&, JobSystem::Job*) {
            half4* UTILS_RESTRICT p = (half4*) data + b * LUT_DIMENSION * LUT_DIMENSION;
            for (size_t g = 0; g < LUT_DIMENSION; g++) {
                for (size_t r = 0; r < LUT_DIMENSION; r++) {
                    float3 v = float3{r, g, b} * (1.0f / (LUT_DIMENSION - 1u));

                    // LogC encoding
                    v = LogC_to_linear(v);

                    // TODO: Peformed in sRGB, should be in Rec.2020 or AP1
                    if (builder->hasAdjustments) {
                        // White balance
                        v = chromaticAdaptation(v, builder->whiteBalance);
                    }

                    // Convert to color grading color space
                    v = config.colorGradingTransformIn * v;

                    if (builder->hasAdjustments) {
                        // Kill negative values before the next transforms
                        v = max(v, 0.0f);

                        // Channel mixer
                        v = channelMixer(v, builder->outRed, builder->outGreen, builder->outBlue);

                        // Shadows/mid-tones/highlights
                        v = tonalRanges(v, config.lumaTransform,
                                builder->shadows, builder->midtones, builder->highlights,
                                builder->tonalRanges);

                        // The adjustments below behave better in log space using the ACEScct
                        // color space.
                        v = config.linearToLogTransform(v);

                        // ASC CDL
                        v = colorDecisionList(v, builder->slope, builder->offset, builder->power);

                        // Contrast in log space
                        v = contrast(v, builder->contrast);

                        // Back to linear space
                        v = config.logToLinearTransform(v);

                        // Vibrance in linear space
                        v = vibrance(v, builder->vibrance);

                        // Saturation in linear space
                        v = saturation(v, builder->saturation);

                        // Kill negative values before tone mapping
                        v = max(v, 0.0f);

                        // RGB curves
                        v = curves(v,
                                builder->shadowGamma, builder->midPoint, builder->highlightScale);
                    }

                    // Tone mapping
                    v = config.toneMapper(v);

                    // Convert to output color space
                    // TODO: allow to customize the output color space,
                    v = config.colorGradingTransformOut * v;

                    // Apply OECF
                    v = OECF_sRGB(v);

                    *p++ = half4{v, 0.0f};
                }
            }
        });
        js.run(job);
    }

    // TODO: Should we do a runAndRetain() and defer the wait() + texture creation until
    //       getHwHandle() is invoked?
    js.runAndWait(slices);

    //std::chrono::duration<float, std::milli> duration = std::chrono::steady_clock::now() - now;
    //slog.d << "LUT generation time: " << duration.count() << " ms" << io::endl;

    // create the texture, we use RGBA16F because RGB16F is not supported everywhere
    mLutHandle = driver.createTexture(SamplerType::SAMPLER_3D, 1,
            TextureFormat::RGBA16F, 0,
            LUT_DIMENSION, LUT_DIMENSION, LUT_DIMENSION, TextureUsage::DEFAULT);

    driver.update3DImage(mLutHandle, 0,
            0, 0, 0,
            LUT_DIMENSION, LUT_DIMENSION, LUT_DIMENSION, PixelBufferDescriptor{
                    data, lutElementCount * elementSize,
                    PixelDataFormat::RGBA, PixelDataType::HALF,
                    [](void* buffer, size_t, void*) { free(buffer); }
            }
    );
}

FColorGrading::~FColorGrading() noexcept = default;

void FColorGrading::terminate(FEngine& engine) {
    DriverApi& driver = engine.getDriverApi();
    driver.destroyTexture(mLutHandle);
}

} //namespace filament
