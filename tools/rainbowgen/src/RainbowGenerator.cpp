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
#include "targa.h"

#include <utils/JobSystem.h>

#include <math/scalar.h>
#include <math/vec2.h>
#include <math/vec3.h>
#include <math/mat3.h>

#include <algorithm>
#include <functional>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>

#include <stdint.h>
#include <stddef.h>

using namespace utils;
using namespace filament::math;
using namespace rainbow;

RainbowGenerator::RainbowGenerator() = default;

RainbowGenerator::~RainbowGenerator() = default;

void RainbowGenerator::build(JobSystem& js) {
    uint32_t const angleCount = mAngleCount;
    float const n0 = indexOfRefraction(350);
    float const n1 = indexOfRefraction(700);
    float const minDeviation = 30*f::DEG_TO_RAD; //deviation(n0, maxIncidentAngle(n0)); // phi = 39.7
    float const maxDeviation = 60*f::DEG_TO_RAD; //deviation(n1, maxIncidentAngle(n1)); // phi = 42.5
    std::cout << minDeviation * f::RAD_TO_DEG << std::endl;
    std::cout << maxDeviation * f::RAD_TO_DEG << std::endl;

    std::vector<float3> rainbow(angleCount, float3{});

    // The sun appears as about half a degree in the sky
    std::default_random_engine rng{ std::random_device{}() };
    std::uniform_real_distribution<float> dist{ -f::DEG_TO_RAD * 0.5f, f::DEG_TO_RAD * 0.5f };

    size_t count = 65536;
    float const s = 2.0f * float(angleCount) / ((maxDeviation - minDeviation) * count * CIE_XYZ_COUNT);

    for (size_t j = 0; j < CIE_XYZ_COUNT; j++) {
        // Current wavelength
        float const w = float(CIE_XYZ_START + j);
        float const n = indexOfRefraction(w);

        for (size_t i = 0; i < count; i++) {
            float const impact = (float(i) / count) * 2.0f - 1.0f;
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
                radian_t const phi = deviation(order, incident, refracted, impactAngle);
                if (phi >= minDeviation && phi < maxDeviation) {
                    size_t const index = (size_t)std::round(
                            ((phi - minDeviation) / (maxDeviation - minDeviation)) * angleCount);
                    if (index < angleCount) {
                        float const T = Taw * std::pow(Rwa,  internalBounces) * Twa;
                        rainbow[index] += (T * s) * (CIE_XYZ[j] / 118.518f);
                    }
                }
            }
        }
    }

    auto* image = tga_new(angleCount, 32);
    for (size_t index = 0; index < angleCount; index++) {
        float3 c = rainbow[index];
        c = XYZ_to_sRGB(c);

        printf("vec3( %g, %g, %g ),\n", c.r, c.g, c.b);

        c = linear_to_sRGB(c*1075);


        uint3 const rgb = uint3(saturate(c) * 255);
        for (int y=0;y<32;y++)
        tga_set_pixel(image, index, y, {
                .b = (uint8_t)rgb.v[2],
                .g = (uint8_t)rgb.v[1],
                .r = (uint8_t)rgb.v[0]
        });
    }
    tga_write("toto.tga", image);
    tga_free(image);
}

// ------------------------------------------------------------------------------------------------

//        float const a = cos(phi / 2);
//        float const c = tan(phi / 2);
//        float const d = -n / (2 * a * a);
//        float const e = 1 - c * c;
//        float const A = 4 * c * c + e * e;
//        float const B = 2 * d * e;
//        float const C = d * d - e * e - 4 * c * c;
//        float const D = c * c - d * d;
//        auto func = [A,B,C,D](float x) {
//            return A*x*x*x*x + B*x*x*x + C*x*x - B*x + D;
//        };
//        auto dfunc = [A,B,C](float x) {
//            return 4*A*x*x*x + 3*B*x*x + 2*C*x - B;
//        };



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

#define TARGALIB_IMPLEMENTATION
#include "targa.h"