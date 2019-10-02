/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include <math/scalar.h>
#include <math/vec3.h>

#include <random>
#include <iostream>

using namespace filament::math;

static float lerp(float a, float b, float f) {
    return a + f * (b - a);
}

int main(int argc, char** argv) {
    std::uniform_real_distribution<float> random(0.0, 1.0);
    std::default_random_engine generator;

    size_t sphereSampleCount = 16;
    std::cout << "const uint kSphereSampleCount = " << sphereSampleCount << "u;" << std::endl;
    std::cout << "const vec3 kSphereSamples[] = vec3[](" << std::endl;
    for (size_t i = 0; i < sphereSampleCount; i++) {
        float s = float(i) / sphereSampleCount;
        float r = random(generator);
        float3 d = {
                random(generator) * 2 - 1,
                random(generator) * 2 - 1,
                random(generator) * 2 - 1
        };
        d = normalize(d);
        d = d * r * lerp(0.1f, 1.0f, s * s);
        if (!(i & 1u)) {
            std::cout << "    ";
        }
        std::cout << " vec3(" << d.x << ", " << d.y << ", " << d.z << "),";
        if (i & 1u) {
            std::cout << std::endl;
        }
    }
    std::cout << ");" << std::endl;


    size_t noiseSampleCount = 16;
    std::cout << "const uint kNoiseSampleCount = " << sphereSampleCount << "u;" << std::endl;
    std::cout << "const vec3 kNoiseSamples[] = vec3[](" << std::endl;
    for (size_t i = 0; i < noiseSampleCount; i++) {
        float3 d = {
                random(generator) * 2 - 1,
                random(generator) * 2 - 1,
                random(generator) * 2 - 1,
        };
        d = normalize(d);
        if ((i & 0x1u) == 0) {
            std::cout << "    ";
        }
        std::cout << " vec3(" << d.x << ", " << d.y << ", " << d.z << "),";
        if ((i & 0x1u) == 0x1) {
            std::cout << std::endl;
        }
    }
    std::cout << ");" << std::endl;


    /*
     * This calculates a modified version of the "spiral" pattern from
     * "Scalable Ambient Obscurance" by Morgan McGuire, that is used in g3d
     * (https://casual-effects.com/g3d/www/index.html).
     *
     * We modify it further here to avoid the sin/cos per sample. The original algorithm is as
     * follows:
     *
     *  s: sample count
     *  i: sample index
     *  phi: random angle
     *
     *  radius = (i + phi + 0.5) / (s - 0.5)
     *  radius = radius^2
     *  angle = radius * (t * 2pi) + phi * 2pi
     *  result = { cos(angle), sin(angle), radius }
     *
     * The idea below is to express the above result as:
     *
     *     cos(angle + f(phi)), sin(angle + f(phi), radius + g(phi)
     *
     * This way, we can precompute cos/sin/radius that doesn't depend on phi, and also
     * precompute a "noise" table that depend only on phi (but not i). At runtime, we can
     * recombine both expressions using the trigonometric identities for cos(a+b) and sin(a+b).
     *
     *
     * Unfortunately, because angle depends on radius^2, it's not possible to separate phi and i,
     * as the final expression is:
     *
     *   g(phi) = phi^2   * 2.0 * F_PI * kSpiralTurns * K
     *          + phi     * 2.0 * F_PI * (1.0 + kSpiralTurns * K)
     *          + i * phi * 4.0 * F_PI * kSpiralTurns * K;   // K is a constant
     *
     * g(phi) has a term in "i * phi" which links both expressions.
     *
     * In the code below we simply assume i=1, which seems to give good results.
     *
     */

    const float kSpiralTurns = 7.0;
    const size_t spiralSampleCount = 7;
    std::cout << "const uint kSpiralSampleCount = " << spiralSampleCount << "u;" << std::endl;
    std::cout << "const vec3 kSpiralSamples[] = vec3[](" << std::endl;
    const float dalpha = 1.0f / (spiralSampleCount - 0.5f);
    for (size_t i = 0; i < spiralSampleCount; i++) {
        float radius = (i + 0.5f) * dalpha;
        float angle = float(radius * radius * (2 * F_PI * kSpiralTurns));
        if ((i & 0x1u) == 0) {
            std::cout << "    ";
        }
        std::cout << " vec3(" << std::cos(angle) << ", " << std::sin(angle) << ", " << radius << "),";
        if ((i & 0x1u) == 0x1) {
            std::cout << std::endl;
        }
    }
    std::cout << ");" << std::endl;


    size_t trigNoiseSampleCount = 16;
    std::cout << "const uint kTrigNoiseSampleCount = " << sphereSampleCount << "u;" << std::endl;
    std::cout << "const vec3 kTrigNoiseSamples[] = vec3[](" << std::endl;
    for (size_t i = 0; i < trigNoiseSampleCount; i++) {
        float phi = random(generator);
        float dr = phi * dalpha;
        float dphi = float(2.0 * F_PI * kSpiralTurns * phi * phi * dalpha * dalpha
                   + phi * 2.0 * F_PI * (1.0 + kSpiralTurns * dalpha * dalpha)
                   + phi * 4.0 * F_PI * kSpiralTurns * dalpha * dalpha);
        float3 d = { std::cos(dphi), std::sin(dphi), dr };
        if ((i & 0x1u) == 0) {
            std::cout << "    ";
        }
        std::cout << " vec3(" << d.x << ", " << d.y << ", " << d.z << "),";
        if ((i & 0x1u) == 0x1) {
            std::cout << std::endl;
        }
    }
    std::cout << ");" << std::endl;


    float weightSum = 0;
    size_t gaussianWidth = 9;   // must be odd
    const size_t gaussianSampleCount = (gaussianWidth + 1) / 2;
    std::cout << "const int kGaussianCount = " << gaussianSampleCount << ";" << std::endl;
    std::cout << "const int kRadius = kGaussianCount - 1;" << std::endl;
    std::cout << "const float kGaussianSamples[] = float[](" << std::endl;
    for (size_t i = 0; i < gaussianSampleCount; i++) {
        float x = i;

        // q: standard deviation
        // A gaussian filter requires 6q-1 values to keep its gaussian nature
        // (see en.wikipedia.org/wiki/Gaussian_filter)
        //
        // Cut-off frequency definition:
        //      fc = 1.1774 / (2pi * q)       (half power frequency or 0.707 amplitude)

        float q = (gaussianWidth + 1.0f) / 6.0f;  // ~1.667 for 9 taps
        float g = (1.0 / (std::sqrt(2.0 * F_PI) * q)) * std::exp(-(x * x) / (2.0 * q * q));
        weightSum += g * (i == 0 ? 1.0f : 2.0f);

        if ((i & 0x7u) == 0) {
            std::cout << "    ";
        }
        std::cout << g << ", ";
        if ((i & 0x7u) == 0x7) {
            std::cout << std::endl;
        }
    }
    std::cout << ");" << std::endl;
    std::cout << "const float kGaussianWeightSum = " << weightSum << ";" << std::endl;

    return 0;
}
