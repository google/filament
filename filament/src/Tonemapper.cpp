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

#include <filament/Color.h>

#include <utils/JobSystem.h>

namespace filament {

using namespace utils;
using namespace math;
using namespace backend;

static constexpr size_t LUT_DIMENSION = 32u;


Tonemapper::Tonemapper(FEngine& engine) {
    DriverApi& driver = engine.getDriverApi();

    // create the texture, we use RGBA16F because RGB16F is not supported everywhere
    mLutHandle = driver.createTexture(SamplerType::SAMPLER_3D, 1,
            TextureFormat::RGBA16F, 0,
            LUT_DIMENSION, LUT_DIMENSION, LUT_DIMENSION, TextureUsage::DEFAULT);

    // it's okay to allocate the data in the command stream because it's only 256 KiB
    constexpr size_t lutSize = LUT_DIMENSION * LUT_DIMENSION * LUT_DIMENSION;
    constexpr size_t elementSize = sizeof(half4);
    void* data = malloc(lutSize * elementSize);
    PixelBufferDescriptor lutData(data,
            lutSize * elementSize, PixelDataFormat::RGBA, PixelDataType::HALF,
            [](void* buffer, size_t, void*) {
                free(buffer);
            }
    );

    auto now = std::chrono::steady_clock::now();
    half4* p = (half4*)data;
    for (size_t b = 0; b < LUT_DIMENSION; b++) {
        for (size_t g = 0; g < LUT_DIMENSION; g++) {
            for (size_t r = 0; r < LUT_DIMENSION; r++) {
                float3 v = float3{ r, g, b } * (1.0 / (LUT_DIMENSION - 1u));
                v.x = lutToLinear(v.x);
                v.y = lutToLinear(v.y);
                v.z = lutToLinear(v.z);
                v = tonemap_ACES(v);
                v = Color::toSRGB(v);
                *p++ = half4{ v, 0 };
            }
        }
    }
    std::chrono::duration<float, std::milli> duration = std::chrono::steady_clock::now() - now;
    slog.d << "LUT generation time: " << duration.count() << " ms" << io::endl;

    driver.update3DImage(mLutHandle, 0,
            0, 0, 0,
            LUT_DIMENSION, LUT_DIMENSION, LUT_DIMENSION, std::move(lutData));
}

Tonemapper::~Tonemapper() noexcept = default;

void Tonemapper::terminate(FEngine& engine) {
    DriverApi& driver = engine.getDriverApi();
    driver.destroyTexture(mLutHandle);
}

float3 Tonemapper::tonemap_ACES(float3 x) noexcept {
    // Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
    const float a = 2.51f;
    const float b = 0.03f;
    const float c = 2.43f;
    const float d = 0.59f;
    const float e = 0.14f;
    return (x * (a * x + b)) / (x * (c * x + d) + e);
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
