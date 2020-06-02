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
// When defined, the ACES tone mapper will match the brightness of the filmic ("ACES sRGB")
// tone mapper. It is *not* correct, but it helps for compatibility
// TODO: Always enable this, expose a float to control this (1.0 == real ACES)
#define TONEMAP_ACES_MATCH_BRIGHTNESS
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
    ToneMapping toneMapping = ToneMapping::ACES;
    float2 whiteBalance     = {0.0f, 0.0f};
    float3 outRed           = {1.0f, 0.0f, 0.0f};
    float3 outGreen         = {0.0f, 1.0f, 0.0f};
    float3 outBlue          = {0.0f, 0.0f, 1.0f};
    float3 shadows          = {1.0f, 1.0f, 1.0f};
    float3 midtones         = {1.0f, 1.0f, 1.0f};
    float3 highlights       = {1.0f, 1.0f, 1.0f};
    float4 tonalRanges      = {0.0f, 0.333f, 0.550f, 1.0f}; // defaults in DaVinci Resolve
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
    ranges.y = clamp(ranges.y, ranges.x + 1e-5f, ranges.z - 1e-5f); // darks
    ranges.z = clamp(ranges.z, ranges.x + 1e-5f, ranges.z - 1e-5f); // lights
    mImpl->tonalRanges = ranges;

    return *this;
}

ColorGrading* ColorGrading::Builder::build(Engine& engine) {
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

mat3f selectColorGradingTransform(ColorGrading::ToneMapping toneMapping) {
    switch (toneMapping) {
        case ColorGrading::ToneMapping::ACES:
            return sRGB_to_AP1;
        default:
            return mat3f{};
    }
}

float3 selectLumaTransform(ColorGrading::ToneMapping toneMapping) {
    switch (toneMapping) {
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

//------------------------------------------------------------------------------
// Tone mapping
//------------------------------------------------------------------------------

using ToneMapper = float3(*)(float3);

ToneMapper selectToneMapping(ColorGrading::ToneMapping toneMapping) {
    switch (toneMapping) {
        case ColorGrading::ToneMapping::LINEAR:
            return tonemap::Linear;
        case ColorGrading::ToneMapping::ACES:
            return tonemap::ACES;
        case ColorGrading::ToneMapping::FILMIC:
            return tonemap::Filmic;
        case ColorGrading::ToneMapping::REINHARD:
            return tonemap::Reinhard;
        case ColorGrading::ToneMapping::DISPLAY_RANGE:
            return tonemap::DisplayRange;
    }
    return tonemap::ACES;
}

//------------------------------------------------------------------------------
// Color grading implementation
//------------------------------------------------------------------------------

struct Config {
    mat3f colorGradingTransform;
    float3 lumaTransform;
    ToneMapper toneMapper;
};

FColorGrading::FColorGrading(FEngine& engine, const Builder& builder) {
    SYSTRACE_CALL();

    DriverApi& driver = engine.getDriverApi();

    constexpr size_t lutElementCount = LUT_DIMENSION * LUT_DIMENSION * LUT_DIMENSION;
    constexpr size_t elementSize = sizeof(half4);
    void* const data = malloc(lutElementCount * elementSize);

    Config config{
        .colorGradingTransform = selectColorGradingTransform(builder->toneMapping),
        .lumaTransform         = selectLumaTransform(builder->toneMapping),
        .toneMapper            = selectToneMapping(builder->toneMapping),
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
                    v = logCToLinear(v);

                    // White balance
                    v = chromaticAdaptation(v, builder->whiteBalance);

                    // Convert to color grading color space
                    v = config.colorGradingTransform * v;

                    // Channel mixer
                    v = channelMixer(v, builder->outRed, builder->outGreen, builder->outBlue);

                    // TODO: Should any of the color grading transforms be applied in ACEScc or
                    //       ACEScct isntead of ACEScg? The primaries are the same (AP1) but
                    //       ACEScc/cct use a log encoding which may be better suited to some kinds
                    //       of transforms

                    // Shadows/mid-tones/highlights
                    v = tonalRanges(v, config.lumaTransform,
                            builder->shadows, builder->midtones, builder->highlights,
                            builder->tonalRanges);

                    // Kill negative values before tone mapping
                    v = max(v, 0.0f);

                    // Tone mapping
                    v = config.toneMapper(v);

                    // Apply OECF
                    // TODO: allow to customize the output color space,
                    //       here we assume we are in the sRGB gamut already
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
