/*
 * Copyright (C) 2021 The Android Open Source Project
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

#ifndef TNT_FILAMENT_FSR_H
#define TNT_FILAMENT_FSR_H

#include <filament/Viewport.h>

#include <math/vec4.h>

#include <stdint.h>

namespace filament {

struct FSRScalingConfig {
    backend::Backend backend;
    Viewport input;   // region of source to be scaled
    uint32_t inputWidth;        // width of source
    uint32_t inputHeight;       // height of source
    uint32_t outputWidth;       // width of destination
    uint32_t outputHeight;      // height of destination
};

struct FSRSharpeningConfig {
    // The scale is {0.0 := maximum sharpness, to N>0, where N is the number of stops (halving)
    // of the reduction of sharpness}.
    float sharpness;
};

struct FSRUniforms {
    math::float4 EasuCon0;
    math::float4 EasuCon1;
    math::float4 EasuCon2;
    math::float4 EasuCon3;
    math::uint4 RcasCon;
};

void FSR_ScalingSetup(FSRUniforms* inoutUniforms, FSRScalingConfig config) noexcept;
void FSR_SharpeningSetup(FSRUniforms* inoutUniforms, FSRSharpeningConfig config) noexcept;

} // namespace filament

#endif // TNT_FILAMENT_FSR_H
