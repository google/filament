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

#include "CIE.h"

#include <stdint.h>
#include <stddef.h>

#include <math/vec3.h>

#include <vector>

namespace utils {
    class JobSystem;
};

class RainbowGenerator {
public:
    RainbowGenerator();
    ~RainbowGenerator();

    void build(utils::JobSystem& js);

private:
    using nanometer_t = float;
    using celcius_t = float;
    using radian_t = float;

    filament::math::float3 generate(radian_t phi) const noexcept;

    static float iorFromWavelength(nanometer_t wavelength, celcius_t temperature) noexcept;
    static radian_t lowestAngle(float ior) noexcept;
    static radian_t maxIncidentAngle(float n) noexcept;
    static radian_t incident(float n, radian_t phi) noexcept;

    static float illuminantD65(float w) noexcept;
    static float3 XYZ_to_sRGB(const float3& v) noexcept;
    static float3 linear_to_sRGB(float3 linear) noexcept;
    static float fresnel(radian_t Bi, radian_t Bt) noexcept;

    size_t mAngleCount = 256;
    float mTemprature = 20.0f;
    float mMinWavelength = CIE_D65_START;
    float mMaxWavelength = CIE_D65_END;
};


#endif //TNT_RAINBOWGENERATOR_H
