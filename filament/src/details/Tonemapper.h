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

#ifndef TNT_FILAMENT_DETAILS_TONEMAPPER_H
#define TNT_FILAMENT_DETAILS_TONEMAPPER_H

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

namespace filament {

class FEngine;

class Tonemapper {
public:
    Tonemapper(FEngine& engine);

    ~Tonemapper() noexcept;

    // frees driver resources, object becomes invalid
    void terminate(FEngine& engine);

    backend::TextureHandle getHwHandle() const noexcept { return mLutHandle; }

private:
    static inline math::float3 tonemap_Reinhard(math::float3 x) noexcept;
    static inline math::float3 Tonemap_ACES_sRGB(math::float3 x) noexcept;
    static inline math::float3 tonemap_ACES(math::float3 x) noexcept;

    // Operators for HDR output
    static inline math::float3 tonemap_ACES_Rec2020_1k(math::float3 x) noexcept;

    // Operators for debugging
    static inline math::float3 tonemap_DisplayRange(math::float3 x) noexcept;

    static inline float lutToLinear(float x) noexcept;
    static inline float linearToLut(float x) noexcept;

    backend::TextureHandle mLutHandle;
};

} // namespace filament

#endif //TNT_FILAMENT_DETAILS_TONEMAPPER_H
