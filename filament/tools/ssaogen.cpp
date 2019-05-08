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
    std::cout << "const uint kSphereSampleCount = " << sphereSampleCount << ";" << std::endl;
    std::cout << "const vec3 kSphereSamples[kSampleCount] = vec3[](" << std::endl;
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
        if (!(i & 1)) {
            std::cout << "   ";
        }
        std::cout << " vec3(" << d.x << "," << d.y << "," << d.z << "),";
        if (i & 1) {
            std::cout << std::endl;
        }
    }
    std::cout << ");" << std::endl;


    size_t noiseSampleCount = 16;
    std::cout << "const uint kNoiseSampleCount = " << sphereSampleCount << ";" << std::endl;
    std::cout << "const vec3 kNoiseSamples[kNoiseSampleCount] = vec3[](" << std::endl;
    for (size_t i = 0; i < noiseSampleCount; i++) {
        float3 d = {
                random(generator) * 2 - 1,
                random(generator) * 2 - 1,
                random(generator) * 2 - 1,
        };
        d = normalize(d);
        if ((i & 0x3) == 0) {
            std::cout << "   ";
        }
        std::cout << " vec3(" << d.x << "," << d.y << "," << d.z << "),";
        if ((i & 0x3) == 0x3) {
            std::cout << std::endl;
        }
    }
    std::cout << ");" << std::endl;

    return 0;
}
