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
    using radian_t = float;

    filament::math::float3 generate(radian_t phi, radian_t dphi) const noexcept;

    size_t mAngleCount = 256;
    float mTemprature = 20.0f;
    float mMinWavelength = CIE_XYZ_START;                       // 390
    float mMaxWavelength = CIE_XYZ_START + CIE_XYZ_COUNT;       // 830
};


#endif //TNT_RAINBOWGENERATOR_H
