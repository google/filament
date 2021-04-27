/*
 * Copyright (C) 2020 The Android Open Source Project
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

#ifndef TNT_FILAMENT_TONE_MAPPING_H
#define TNT_FILAMENT_TONE_MAPPING_H

#include "ColorSpace.h"

#include <utils/compiler.h>

#include <math/mat3.h>
#include <math/vec3.h>
#include <math/scalar.h>

namespace filament {

using namespace math;

//------------------------------------------------------------------------------
// ACES operations, from https://github.com/ampas/aces-dev
//------------------------------------------------------------------------------

namespace aces {

float3 ACES(float3 color, float brightness) noexcept;

} // namespace aces

//------------------------------------------------------------------------------
// Tone mapping operators
//------------------------------------------------------------------------------

namespace tonemap {

constexpr float3 Linear(float3 x) noexcept {
    return x;
}

constexpr float3 Reinhard(float3 x) noexcept {
    return x / (1.0f + dot(x, LUMA_REC709));
}

constexpr float3 Filmic(float3 x) noexcept {
    // Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
    constexpr float a = 2.51f;
    constexpr float b = 0.03f;
    constexpr float c = 2.43f;
    constexpr float d = 0.59f;
    constexpr float e = 0.14f;
    return (x * (a * x + b)) / (x * (c * x + d) + e);
}

float3 ACES(float3 x) noexcept;

float3 ACES_Legacy(float3 x) noexcept;

float3 EVILS(float3 x) noexcept;

/**
 * Converts the input HDR RGB color into one of 16 debug colors that represent
 * the pixel's exposure. When the output is cyan, the input color represents
 * middle gray (18% exposure). Every exposure stop above or below middle gray
 * causes a color shift.
 *
 * The relationship between exposures and colors is:
 *
 * -5EV  - black
 * -4EV  - darkest blue
 * -3EV  - darker blue
 * -2EV  - dark blue
 * -1EV  - blue
 *  OEV  - cyan
 * +1EV  - dark green
 * +2EV  - green
 * +3EV  - yellow
 * +4EV  - yellow-orange
 * +5EV  - orange
 * +6EV  - bright red
 * +7EV  - red
 * +8EV  - magenta
 * +9EV  - purple
 * +10EV - white
 */
float3 DisplayRange(float3 x) noexcept;

} // namespace tonemap

} // namespace filament

#endif //TNT_FILAMENT_TONE_MAPPING_H
