/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include <filament/Viewport.h>

namespace filament {

Viewport Viewport::scale(math::float2 s) const noexcept {

    // round to nearest to avoid overlapping viewports
    // this could result to empty viewport, though.
    float l = floorf(0.5f + s.x * left);
    float b = floorf(0.5f + s.y * bottom);
    float r = floorf(0.5f + s.x * (left + width));
    float t = floorf(0.5f + s.y * (bottom + height));

    return { int32_t(l), int32_t(b), uint32_t(r - l), uint32_t(t - b) };
}

} // namespace filament
