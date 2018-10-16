/*
 * Copyright (C) 2015 The Android Open Source Project
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

#ifndef TNT_FILAMENT_DETAILS_LIGHTDATA_H
#define TNT_FILAMENT_DETAILS_LIGHTDATA_H

#include "UniformBuffer.h"

#include <driver/Handle.h>

#include <math/vec4.h>

namespace filament {
namespace details {

class FEngine;

class GpuLightBuffer {
public:
    using LightIndex = uint16_t;

    struct LightParameters {
        math::float4 positionFalloff;   // { float3(pos), 1/falloff^2 }
        math::float4 colorIntensity;    // { float3(col), intensity }
        math::float4 directionIES;      // { float3(dir), IES index }
        math::float4 spotScaleOffset;   // { scale, offset, unused, unused }
    };

    explicit GpuLightBuffer(FEngine& engine) noexcept;

    void commit(FEngine& engine) noexcept;

    void terminate(FEngine& engine);

    GpuLightBuffer(GpuLightBuffer const& rhs) = delete;
    GpuLightBuffer(GpuLightBuffer&& rhs) = delete;
    GpuLightBuffer& operator=(GpuLightBuffer const& rhs) = delete;
    GpuLightBuffer& operator=(GpuLightBuffer&& rhs) = delete;
    ~GpuLightBuffer() noexcept;

    LightParameters& getLightParameters(LightIndex h) noexcept {
        // This assumes the layout of the LightsUniforms uniform buffer
        // it is defined in UibGenerator.cpp
        LightParameters* lights = (LightParameters *)mLightsUb.getBuffer();
        return lights[h];
    }

    void invalidate(LightIndex h, size_t count) noexcept {
        mLightsUb.invalidateUniforms(h * sizeof(LightParameters), count * sizeof(LightParameters));
    }

private:
    void commitSlow(FEngine& engine) noexcept;
    Handle<HwUniformBuffer> mLightUbh;
    mutable UniformBuffer mLightsUb;
};

} // namespace details
} // namespace filament

#endif // TNT_FILAMENT_DETAILS_LIGHTDATA_H
