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

#include "details/Tonemapper.h"

#include "details/Engine.h"

#include <image/ColorTransform.h>

#include <utils/JobSystem.h>
#include <utils/Systrace.h>

// When defined, the ACES tone mapper will match the brightness of the "ACES sRGB" tone mapper
// It is *not* correct, but it helps for compatibility
#define TONEMAP_ACES_MATCH_BRIGHTNESS

#include "ACES.h"

namespace filament {

using namespace utils;
using namespace math;
using namespace backend;

static constexpr size_t LUT_DIMENSION = 32u;

Tonemapper::Tonemapper(FEngine& engine) {
    SYSTRACE_CALL();

    DriverApi& driver = engine.getDriverApi();

    constexpr size_t lutElementCount = LUT_DIMENSION * LUT_DIMENSION * LUT_DIMENSION;
    constexpr size_t elementSize = sizeof(half4);
    void* const data = malloc(lutElementCount * elementSize);

    //auto now = std::chrono::steady_clock::now();

    // Multithreadedly generate the tone mapping 3D look-up table using 32 jobs
    // Slices are 8 KiB (128 cache lines) apart.
    // This takes about 3-6ms on Android in Release mode
    JobSystem& js = engine.getJobSystem();
    auto slices = js.createJob();
    for (size_t b = 0; b < LUT_DIMENSION; b++) {
        auto job = js.createJob(slices,[data, b](JobSystem&, JobSystem::Job*) {
            half4* UTILS_RESTRICT p = (half4*)data + b * LUT_DIMENSION * LUT_DIMENSION;
            for (size_t g = 0; g < LUT_DIMENSION; g++) {
                for (size_t r = 0; r < LUT_DIMENSION; r++) {
                    float3 v = float3{ r, g, b } * (1.0f / (LUT_DIMENSION - 1u));
                    // LogC encoding
                    v.x = lutToLinear(v.x);
                    v.y = lutToLinear(v.y);
                    v.z = lutToLinear(v.z);
                    // ACES tone mapping
                    v = tonemap_ACES(v);
                    // sRGB encoding
                    v = image::linearTosRGB(v);
                    *p++ = half4{ v, 0 };
                }
            }
        });
        js.run(job);
    }
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
            });
}

Tonemapper::~Tonemapper() noexcept = default;

void Tonemapper::terminate(FEngine& engine) {
    DriverApi& driver = engine.getDriverApi();
    driver.destroyTexture(mLutHandle);
}

float3 Tonemapper::tonemap_Reinhard(float3 x) noexcept {
    return x / (1.0f + dot(x, float3{ 0.2126f, 0.7152f, 0.0722f }));
}

float3 Tonemapper::Tonemap_ACES_sRGB(float3 x) noexcept {
    // Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
    const float a = 2.51f;
    const float b = 0.03f;
    const float c = 2.43f;
    const float d = 0.59f;
    const float e = 0.14f;
    return (x * (a * x + b)) / (x * (c * x + d) + e);
}

UTILS_ALWAYS_INLINE
float3 Tonemapper::tonemap_ACES(float3 x) noexcept {
    return aces::ACES(x);
}

float3 Tonemapper::tonemap_ACES_Rec2020_1k(float3 x) noexcept {
    // Narkowicz 2016, "HDR Display – First Steps"
    const float a = 15.8f;
    const float b = 2.12f;
    const float c = 1.2f;
    const float d = 5.92f;
    const float e = 1.9f;
    return (x * (a * x + b)) / (x * (c * x + d) + e);
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
float3 Tonemapper::tonemap_DisplayRange(float3 x) noexcept {
    // 16 debug colors + 1 duplicated at the end for easy indexing

    const float3 debugColors[17] = {
            { 0.0,     0.0,     0.0 },         // black
            { 0.0,     0.0,     0.1647 },      // darkest blue
            { 0.0,     0.0,     0.3647 },      // darker blue
            { 0.0,     0.0,     0.6647 },      // dark blue
            { 0.0,     0.0,     0.9647 },      // blue
            { 0.0,     0.9255,  0.9255 },      // cyan
            { 0.0,     0.5647,  0.0 },         // dark green
            { 0.0,     0.7843,  0.0 },         // green
            { 1.0,     1.0,     0.0 },         // yellow
            { 0.90588, 0.75294, 0.0 },         // yellow-orange
            { 1.0,     0.5647,  0.0 },         // orange
            { 1.0,     0.0,     0.0 },         // bright red
            { 0.8392,  0.0,     0.0 },         // red
            { 1.0,     0.0,     1.0 },         // magenta
            { 0.6,     0.3333,  0.7882 },      // purple
            { 1.0,     1.0,     1.0 },         // white
            { 1.0,     1.0,     1.0 }          // white
    };

    // The 5th color in the array (cyan) represents middle gray (18%)
    // Every stop above or below middle gray causes a color shift
    float v = log2(dot(x, float3{ 0.2126f, 0.7152f, 0.0722f }) / 0.18f);
    v = clamp(v + 5.0f, 0.0f, 15.0f);
    size_t index = size_t(v);
    return mix(debugColors[index], debugColors[index + 1], v - float(index));
}

float Tonemapper::lutToLinear(float x) noexcept {
    // Alexa LogC EI 1000 curve
    const float ia = 1.0f / 5.555556f;
    const float b = 0.047996f;
    const float ic = 1.0f / 0.244161f;
    const float d = 0.386036f;
    return (std::pow(10.0f, (x - d) * ic) - b) * ia;
}

float Tonemapper::linearToLut(float x) noexcept {
    // Alexa LogC EI 1000 curve
    const float a = 5.555556f;
    const float b = 0.047996f;
    const float c = 0.244161f;
    const float d = 0.386036f;
    return c * std::log10(a * x + b) + d;
}

} //namespace filament
