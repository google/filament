/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include "RainbowGenerator.h"
#include "CIE.h"
#include "rainbow.h"

#include <utils/JobSystem.h>

#include <cmath>
#include <random>
#include <vector>

#include <stdint.h>
#include <stddef.h>

using namespace utils;
using namespace filament::math;
using namespace rainbow;

RainbowGenerator::RainbowGenerator() = default;

RainbowGenerator::~RainbowGenerator() = default;

RainbowGenerator& RainbowGenerator::lut(uint32_t count) noexcept {
    mAngleCount = count;
    return *this;
}

RainbowGenerator& RainbowGenerator::deviation(radian_t min, radian_t max) noexcept {
    mMinDeviation = min;
    mMaxDeviation = max;
    return *this;
}

RainbowGenerator& RainbowGenerator::samples(uint32_t count) noexcept {
    mSampleCount = count;
    return *this;
}

RainbowGenerator& RainbowGenerator::secondary(bool enabled) noexcept {
    mSecondaryRainbow = enabled;
    return *this;
}

RainbowGenerator& RainbowGenerator::temperature(celcius_t t) noexcept {
    mAirTemperature = t;
    return *this;
}

RainbowGenerator& RainbowGenerator::sunArc(radian_t arc) noexcept {
    mSunArc = arc;
    return *this;
}

Rainbow RainbowGenerator::build(JobSystem&) {
    uint32_t const angleCount = mAngleCount;
    float const minDeviation = mMinDeviation;
    float const maxDeviation = mMaxDeviation;

    Rainbow rainbow{
            .minDeviation = minDeviation,
            .maxDeviation = maxDeviation,
            .data = std::vector<Rainbow::linear_sRGB_t>(angleCount, Rainbow::linear_sRGB_t{})
    };

    // The sun appears as about a degree in the sky
    std::default_random_engine rng{ std::random_device{}() };
    std::uniform_real_distribution<float> dist{ -mSunArc * 0.5f, mSunArc * 0.5f };

    int32_t const count = int32_t(mSampleCount);
    float const s = float(2 * angleCount) / float((maxDeviation - minDeviation) * float(count) * CIE_XYZ_COUNT);

    for (size_t j = 0; j < CIE_XYZ_COUNT; j++) {
        // Current wavelength
        float const w = float(CIE_XYZ_START + j);
        float const n = indexOfRefraction(w);

        for (int32_t i = 0; i < count; i++) {
            float const impact = float(i * 2 - count) / float(count);
            radian_t const impactAngle = dist(rng);
            radian_t const incident = std::asin(impact) - impactAngle;

            radian_t const refracted = refract(n, incident);
            // water-air fresnel is equal to 1 - air-water fresnel, so we only need to
            // air-water non-polarized fresnel

            // intensity reflected upon entering the droplet (air-water)
            float const Raw = fresnel(incident, refracted);

            // intensity reflected upon exiting the droplet (water-air)
            float const Rwa = fresnel(refracted, incident);

            // intensity transmitted at air-water interface
            float const Taw = 1 - Raw;

            // intensity transmitted at water-air interface
            float const Twa = 1 - Rwa;

            for (int order = 0; order < 2; order++) {
                float const internalBounces = float(order + 1);
                radian_t const phi = rainbow::deviation(order, incident, refracted, impactAngle);
                if (phi >= minDeviation && phi < maxDeviation) {
                    size_t const index = (size_t)std::round(
                            ((phi - minDeviation) / (maxDeviation - minDeviation)) * float(angleCount));
                    if (index < angleCount) {
                        float const T = Taw * std::pow(Rwa,  internalBounces) * Twa;
                        rainbow.data[index] += (T * s) * (CIE_XYZ[j] / 118.518f);
                    }
                }
            }
        }
    }

    return rainbow;
}

//vec3 sun = frameUniforms.lightColorIntensity.rgb *
//           (frameUniforms.lightColorIntensity.a * (4.0 * PI));
//vec3 direction = normalize(variable_eyeDirection.xyz);
//float cosAngle = dot(direction, -frameUniforms.lightDirection);
//float angle = acos(cosAngle) * 180.0 / 3.14159;
//float first = 35.0;
//float range = (60.0 - 35.0);
//float s = saturate((angle - first)/range);
//int index = int(s * 255);
//fragColor.rgb += rainbow[index]*sun;
