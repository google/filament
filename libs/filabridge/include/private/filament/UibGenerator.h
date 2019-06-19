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

#ifndef TNT_FILABRIDGE_UIBGENERATOR_H
#define TNT_FILABRIDGE_UIBGENERATOR_H


#include <math/mat4.h>
#include <math/vec4.h>

namespace filament {

class UniformInterfaceBlock;

class UibGenerator {
public:
    static UniformInterfaceBlock const& getPerViewUib() noexcept;
    static UniformInterfaceBlock const& getPerRenderableUib() noexcept;
    static UniformInterfaceBlock const& getLightsUib() noexcept;
    static UniformInterfaceBlock const& getPostProcessingUib() noexcept;
    static UniformInterfaceBlock const& getPerRenderableBonesUib() noexcept;
};

/*
 * These structures are only used to call offsetof() and make it easy to visualize the UBO.
 *
 * IMPORTANT NOTE: Respect std140 layout, don't update without updating getUib()
 */

struct PerViewUib { // NOLINT(cppcoreguidelines-pro-type-member-init)
    static const UniformInterfaceBlock& getUib() noexcept {
        return UibGenerator::getPerViewUib();
    }
    filament::math::mat4f viewFromWorldMatrix;
    filament::math::mat4f worldFromViewMatrix;
    filament::math::mat4f clipFromViewMatrix;
    filament::math::mat4f viewFromClipMatrix;
    filament::math::mat4f clipFromWorldMatrix;
    filament::math::mat4f worldFromClipMatrix;
    filament::math::mat4f lightFromWorldMatrix;

    filament::math::float4 resolution; // viewport width, height, 1/width, 1/height

    filament::math::float3 cameraPosition;
    float time; // time in seconds, with a 1 second period

    filament::math::float4 lightColorIntensity; // directional light

    filament::math::float4 sun; // cos(sunAngle), sin(sunAngle), 1/(sunAngle*HALO_SIZE-sunAngle), HALO_EXP

    filament::math::float3 lightDirection;
    uint32_t fParamsX; // stride-x

    filament::math::float3 shadowBias; // unused, normal bias, unused
    float oneOverFroxelDimensionY;

    filament::math::float4 zParams; // froxel Z parameters

    filament::math::uint2 fParams; // stride-y, stride-z
    filament::math::float2 origin; // viewport left, viewport bottom

    float oneOverFroxelDimensionX;
    float iblLuminance;
    float exposure;
    float ev100;

    alignas(16) filament::math::float4 iblSH[9]; // actually float3 entries (std140 requires float4 alignment)

    filament::math::float4 userTime;  // time(s), (double)time - (float)time, 0, 0

    filament::math::float2 iblMaxMipLevel; // maxlevel, float(1<<maxlevel)
    filament::math::float2 padding0;

    // bring PerViewUib to 1 KiB
    filament::math::float4 padding1[16];
};


// PerRenderableUib must have an alignment of 256 to be compatible with all versions of GLES.
struct alignas(256) PerRenderableUib {
    filament::math::mat4f worldFromModelMatrix;
    filament::math::mat3f worldFromModelNormalMatrix;
};

struct LightsUib {
    static const UniformInterfaceBlock& getUib() noexcept {
        return UibGenerator::getLightsUib();
    }
    filament::math::float4 positionFalloff;   // { float3(pos), 1/falloff^2 }
    filament::math::float4 colorIntensity;    // { float3(col), intensity }
    filament::math::float4 directionIES;      // { float3(dir), IES index }
    filament::math::float4 spotScaleOffset;   // { scale, offset, unused, unused }
};

struct PostProcessingUib {
    static const UniformInterfaceBlock& getUib() noexcept {
        return UibGenerator::getPostProcessingUib();
    }
    filament::math::float2 uvScale;
    float time;             // time in seconds, with a 1 second period, used for dithering
    float yOffset;
    int dithering;          // type of dithering 0=none, 1=enabled
};

// This is not the UBO proper, but just an element of a bone array.
struct PerRenderableUibBone {
    filament::math::quatf q = { 1, 0, 0, 0 };
    filament::math::float4 t = {};
    filament::math::float4 s = { 1, 1, 1, 0 };
    filament::math::float4 ns = { 1, 1, 1, 0 };
};

} // namespace filament

#endif // TNT_FILABRIDGE_UIBGENERATOR_H
