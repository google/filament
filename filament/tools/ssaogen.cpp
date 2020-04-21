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

int main(int argc, char** argv) {
    std::uniform_real_distribution<float> random(0.0, 1.0);
    std::default_random_engine generator;

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

        //float q = (gaussianWidth + 1.0f) / 6.0f;  // ~1.333 for 7 taps
        float q = 2.0; // this works better, eventhough the filter is no longer really gaussian
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

