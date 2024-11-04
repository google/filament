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

#ifndef TNT_RAINBOWGENERATOR_H
#define TNT_RAINBOWGENERATOR_H

#include <stdint.h>
#include <stddef.h>

#include <math/vec3.h>
#include <math/scalar.h>

#include <vector>

namespace utils {
    class JobSystem;
};

struct Rainbow {
    using linear_sRGB_t = filament::math::float3;
    float s;        // input parameter scale factor
    float o;        // input parameter offset
    float scale;    // output scale factor
    std::vector<linear_sRGB_t> data;
};

class RainbowGenerator {
public:
    using radian_t = float;
    using celcius_t = float;

    RainbowGenerator();
    ~RainbowGenerator();

    // LUT size
    RainbowGenerator& lut(uint32_t count) noexcept;

    // LUT indexed by cosine
    RainbowGenerator& cosine(bool enabled) noexcept;

    // min deviation
    RainbowGenerator& minDeviation(radian_t min) noexcept;

    // max deviation
    RainbowGenerator& maxDeviation(radian_t max) noexcept;

    // number of samples for the calculation
    RainbowGenerator& samples(uint32_t count) noexcept;

    // air temperature
    RainbowGenerator& temperature(celcius_t t) noexcept;

    // sun arc
    RainbowGenerator& sunArc(radian_t arc) noexcept;

    // bulid the rainbow LUT
    Rainbow build(utils::JobSystem& js);

private:
    size_t mLutSize = 256;
    radian_t mMinDeviation = 30.0f * filament::math::f::DEG_TO_RAD;
    radian_t mMaxDeviation = 60.0f * filament::math::f::DEG_TO_RAD;
    radian_t mSunArc = 1.0f * filament::math::f::DEG_TO_RAD;
    uint32_t mSampleCount = 65536;
    float mAirTemperature = 20.0f;
    bool mCosine = false;
};

#endif //TNT_RAINBOWGENERATOR_H
