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
    float const n0 = indexOfRefraction(350, mTemprature);
    float const n1 = indexOfRefraction(750, mTemprature);
    float const minDeviation = deviation(n0, maxIncidentAngle(n0)); // 2 phi = 39.7
    float const maxDeviation = deviation(n1, maxIncidentAngle(n1)); // 2 phi = 42.5
    std::cout << minDeviation * f::RAD_TO_DEG << std::endl;
    std::cout << maxDeviation * f::RAD_TO_DEG << std::endl;

    std::vector<float3> rainbow(angleCount, float3{});
    auto work = [=, &rainbow](uint32_t start, uint32_t count) {
        for (size_t i = 0; i < count; i++) {
            size_t const index = start + i;
            float const s = (float(index) + 0.5f) / float(angleCount);
            float const phi  = 0.5f * (minDeviation + (maxDeviation - minDeviation) * s);
            float const dphi = 0.25f * ((maxDeviation - minDeviation) / float(angleCount));
            float3  c = generate(phi, dphi);
            std::cout << c.r << ", " << c.g << ", " << c.b << std::endl;
            c = linear_to_sRGB(c);
            rainbow[index] = c;
        }
    };

    auto* renderableJob = jobs::parallel_for(js, nullptr,
            0, angleCount,
            std::cref(work),
            jobs::CountSplitter<64, 0>());

    js.runAndWait(renderableJob);


    auto* image = tga_new(angleCount, 16*3);
    for (size_t index = 0; index < angleCount; index++) {
        for (int i = 0; i < 3; i++) {
            for (int y = 0; y < 16; y++) {
                float3 const sun = sRGB_to_linear(float3(255, 161, 72) / 255.0f);
                float3 const sky = sRGB_to_linear(float3(135, 206, 235) / 255.0f);
                float3 c = rainbow[index];
                c = linear_to_sRGB(c);// * sun + sky);
                uint3 const rgb = uint3(saturate(c) * 255);
                tga_set_pixel(image, index, y+i*16, {
                        .b = (uint8_t)rgb.v[2],
                        .g = (uint8_t)rgb.v[1],
                        .r = (uint8_t)rgb.v[0]
                });
            }
        }
    }
    tga_write("toto.tga", image);
    tga_free(image);
}

float3 RainbowGenerator::generate(radian_t phi, radian_t dphi) const noexcept {
    auto func = [](float n, radian_t beta, radian_t phi) {
        return 2.0f * beta - std::asin(n * std::sin(beta)) - phi;
    };
    auto dfunc = [](float n, radian_t beta) {
        return 2 - n * std::cos(beta) / std::sqrt(1 - n * n * sin(beta) * sin(beta));
    };
    auto beta = [&](float n, radian_t guess, radian_t phi) {
        float r = guess;
        for (size_t j = 0; j < 8; j++) {
            r = r - func(n, r, phi) / dfunc(n, r);
        }
        return r;
    };

    float3 f0 = 0.0f;
    for (size_t i = 0; i < CIE_XYZ_COUNT; i++) {
        // Current wavelength
        float const w = float(CIE_XYZ_START + i);
        float const n = indexOfRefraction(w, mTemprature);
        if (2 * phi >= deviation(n, maxIncidentAngle(n))) {
            continue;
        }



        float T=0;

        // here we shouldn't sample "beta", but rather the offset from the center
        for (radian_t b = 0; b < f::PI_2; b += f::DEG_TO_RAD/100) {
            if (n * std::sin(b) <= 1) {
                radian_t p = deviation(n, b) * 0.5f;
                if (p >= phi - dphi && p <= phi + dphi) {


                    float const B = b;
                    float const Bi_a = 2.0f * B - p;         // by definition
                    float const Bt_w = B;                    // by definition
                    Fresnel const Faw = fresnel(Bi_a, Bt_w);
                    Fresnel const Fwa = fresnel(Bt_w, Bi_a);
                    T += Faw.t * Fwa.r * Fwa.t;

                }
            }
        }
//        float s = c;


        // apply the density of the rays in that direction
//        float const s = 1 / dfunc(n, B);
//        T *= s;   // fixme: what's the correct factor here?

        f0 += T * CIE_XYZ[i];
    }
    f0 /= 118.518f; // normalization factor
    f0 = XYZ_to_sRGB(f0);
    return f0;
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


#define TARGALIB_IMPLEMENTATION
#include "targa.h"