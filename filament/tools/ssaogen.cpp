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

// The blue noise texture data below is converted from the images provided for
// download by Christoph Peters under the Creative Commons CC0 Public
// Domain Dedication (CC0 1.0) license.
// See http://momentsingraphics.de/BlueNoise.html
uint8_t kBlueNoise[] = {
    0x6f, 0x31, 0x8e, 0xa2, 0x71, 0xc3, 0x47, 0xb1, 0xc9, 0x32, 0x97, 0x5e, 0x42, 0x25, 0x55, 0xfc,
    0x19, 0x63, 0xef, 0xde, 0x20, 0xfa, 0x94, 0x13, 0x26, 0x6a, 0xdc, 0xaa, 0xc2, 0x8a, 0x0d, 0xa7,
    0x7d, 0xb2, 0x4f, 0x0f, 0x41, 0xad, 0x7b, 0x57, 0xd5, 0x83, 0xf7, 0x17, 0x74, 0x36, 0xe5, 0xd4,
    0x29, 0xca, 0x98, 0x84, 0xbd, 0x68, 0x35, 0xec, 0xa1, 0x3e, 0x01, 0xb5, 0x4d, 0xf1, 0x93, 0x44,
    0x02, 0xf4, 0x38, 0x5b, 0xe6, 0x05, 0xcc, 0x1c, 0xbb, 0x65, 0x90, 0xce, 0x21, 0x5c, 0xbe, 0x6b,
    0xdf, 0xa4, 0x72, 0x24, 0xd6, 0x9c, 0x8b, 0x46, 0xf5, 0x54, 0xe2, 0x30, 0x7e, 0x9e, 0x11, 0x87,
    0x53, 0xc4, 0x15, 0xfe, 0x4c, 0x2d, 0xb3, 0x73, 0x0c, 0x28, 0xa9, 0x69, 0xfd, 0xb0, 0xd3, 0x3b,
    0x64, 0xb4, 0x91, 0x7a, 0xac, 0x61, 0xeb, 0x81, 0xd7, 0x95, 0xc7, 0x08, 0x48, 0x1a, 0xee, 0x2c,
    0xe8, 0x1f, 0x45, 0x0b, 0xcd, 0x3a, 0x12, 0xc1, 0x58, 0x3c, 0x70, 0xdd, 0x8c, 0x56, 0x78, 0x99,
    0xd0, 0x82, 0xf3, 0xa0, 0xe0, 0x6e, 0x22, 0xf8, 0xa5, 0x18, 0xea, 0xb8, 0x34, 0xc6, 0xab, 0x06,
    0x6c, 0xbc, 0x33, 0x59, 0x89, 0xba, 0x9a, 0x4e, 0x2f, 0x86, 0x62, 0x9d, 0x23, 0xf9, 0x5f, 0x3f,
    0x10, 0x4b, 0xdb, 0x27, 0x00, 0x43, 0xe4, 0x79, 0xc5, 0xf0, 0x03, 0x4a, 0x7f, 0x14, 0xe3, 0x8f,
    0xf6, 0xaf, 0x77, 0xc8, 0xfb, 0x67, 0x92, 0x0e, 0xd1, 0xae, 0x6d, 0xda, 0xc0, 0x52, 0xcb, 0xa3,
    0x1d, 0x5d, 0x96, 0x16, 0xa6, 0xb6, 0x37, 0x1e, 0x5a, 0x40, 0x2a, 0x8d, 0xa8, 0x39, 0x75, 0x2e,
    0xd8, 0xe9, 0x3d, 0x80, 0x51, 0xed, 0xd9, 0x76, 0x9f, 0xff, 0xb9, 0x1b, 0xf2, 0x66, 0x04, 0x85,
    0x49, 0xbf, 0x09, 0xd2, 0x2b, 0x60, 0x07, 0x88, 0xe7, 0x50, 0x0a, 0x7c, 0xe1, 0xcf, 0x9b, 0xb7
};

int main(int argc, char** argv) {
    std::uniform_real_distribution<float> random(0.0, 1.0);
    std::default_random_engine generator;

    size_t sphereSampleCount = 16;
    std::cout << "const uint kSphereSampleCount = " << sphereSampleCount << "u;" << std::endl;
    std::cout << "const vec3 kSphereSamples[] = vec3[](" << std::endl;
    for (size_t i = 0; i < sphereSampleCount; i++) {
        const bool first = i == 0;
        const bool last = i == sphereSampleCount - 1;
        const bool newline = (i & 0x1u) == 0x1;

        float s = float(i) / sphereSampleCount;
        float r = random(generator);
        float3 d = {
                random(generator) * 2 - 1,
                random(generator) * 2 - 1,
                random(generator) * 2 - 1
        };
        d = normalize(d);
        d = d * r * lerp(0.1f, 1.0f, s * s);

        if (first) {
            printf("    ");
        }
        printf("vec3(%9f, %9f, %9f)", d.x, d.y, d.z);
        if (!last) {
            printf(",");
            if (newline) {
                printf("\n    ");
            } else {
                printf(" ");
            }
        } else {
            printf("\n);\n");
        }
    }


    size_t noiseSampleCount = 16;
    std::cout << "const uint kNoiseSampleCount = " << sphereSampleCount << "u;" << std::endl;
    std::cout << "const vec3 kNoiseSamples[] = vec3[](" << std::endl;
    for (size_t i = 0; i < noiseSampleCount; i++) {
        const bool first = i == 0;
        const bool last = i == noiseSampleCount - 1;
        const bool newline = (i & 0x1u) == 0x1;

        float3 d = {
                random(generator) * 2 - 1,
                random(generator) * 2 - 1,
                random(generator) * 2 - 1,
        };
        d = normalize(d);

        if (first) {
            printf("    ");
        }
        printf("vec3(%9f, %9f, %9f)", d.x, d.y, d.z);
        if (!last) {
            printf(",");
            if (newline) {
                printf("\n    ");
            } else {
                printf(" ");
            }
        } else {
            printf("\n);\n");
        }
    }


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
        const bool first = i == 0;
        const bool last = i == spiralSampleCount - 1;
        const bool newline = (i & 0x1u) == 0x1;

        float radius = (i + 0.5f) * dalpha;
        float angle = float(radius * radius * (2 * F_PI * kSpiralTurns));

        if (first) {
            printf("    ");
        }
        printf("vec3(%9f, %9f, %9f)", std::cos(angle), std::sin(angle), radius);
        if (!last) {
            printf(",");
            if (newline) {
                printf("\n    ");
            } else {
                printf(" ");
            }
        } else {
            printf("\n);\n");
        }
    }

    const size_t trigNoiseSampleCount = 256;
    std::cout << "const uint kTrigNoiseSampleCount = " << trigNoiseSampleCount << "u;" << std::endl;
    std::cout << "const vec3 kTrigNoiseSamples[] = vec3[](" << std::endl;

    for (size_t i = 0; i < trigNoiseSampleCount; i++) {
        const bool first = i == 0;
        const bool last = i == trigNoiseSampleCount - 1;
        const bool newline = (i & 0x3u) == 0x3;

        float phi = kBlueNoise[i] / 255.0f;
        float dr = phi * dalpha;
        float dphi = float(2.0 * F_PI * kSpiralTurns * phi * phi * dalpha * dalpha
                   + phi * 2.0 * F_PI * (1.0 + kSpiralTurns * dalpha * dalpha)
                   + phi * 4.0 * F_PI * kSpiralTurns * dalpha * dalpha);
        float3 d = { std::cos(dphi), std::sin(dphi), dr };

        if (first) {
            printf("    ");
        }
        printf("vec3(%9f, %9f, %9f)", d.x, d.y, d.z);
        if (!last) {
            printf(",");
            if (newline) {
                printf("\n    ");
            } else {
                printf(" ");
            }
        } else {
            printf("\n);\n");
        }
    }


    float weightSum = 0;
    size_t gaussianWidth = 15;   // must be odd
    const size_t gaussianSampleCount = (gaussianWidth + 1) / 2;
    std::cout << "const int kGaussianCount = " << gaussianSampleCount << ";" << std::endl;
    std::cout << "const int kRadius = kGaussianCount - 1;" << std::endl;
    std::cout << "const float kGaussianSamples[] = float[](" << std::endl;
    for (size_t i = 0; i < gaussianSampleCount; i++) {
        const bool first = i == 0;
        const bool last = i == gaussianSampleCount - 1;
        const bool newline = (i & 0x7u) == 0x7;

        float x = i;

        // q: standard deviation
        // A gaussian filter requires 6q-1 values to keep its gaussian nature
        // (see en.wikipedia.org/wiki/Gaussian_filter)
        //
        // Cut-off frequency definition:
        //      fc = 1.1774 / (2pi * q)       (half power frequency or 0.707 amplitude)

        float q = (gaussianWidth + 1.0f) / 6.0f;  // ~2.667 for 15 taps
        float g = float((1.0 / (std::sqrt(2.0 * F_PI) * q)) * std::exp(-(x * x) / (2.0 * q * q)));
        weightSum += g * (i == 0 ? 1.0f : 2.0f);

        if (first) {
            printf("    ");
        }
        printf("%8f", g);
        if (!last) {
            printf(",");
            if (newline) {
                printf("\n    ");
            } else {
                printf(" ");
            }
        } else {
            printf("\n);\n");
        }
    }
    std::cout << "const float kGaussianWeightSum = " << weightSum << ";" << std::endl;

    return 0;
}

