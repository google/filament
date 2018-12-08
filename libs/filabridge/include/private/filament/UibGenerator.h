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
    math::mat4f viewFromWorldMatrix;
    math::mat4f worldFromViewMatrix;
    math::mat4f clipFromViewMatrix;
    math::mat4f viewFromClipMatrix;
    math::mat4f clipFromWorldMatrix;
    math::mat4f lightFromWorldMatrix;

    math::float4 resolution; // viewport width, height, 1/width, 1/height

    math::float3 cameraPosition;
    float time; // time in seconds, with a 1 second period

    math::float4 lightColorIntensity; // directional light

    math::float4 sun; // cos(sunAngle), sin(sunAngle), 1/(sunAngle*HALO_SIZE-sunAngle), HALO_EXP

    math::float3 lightDirection;
    uint32_t fParamsX; // stride-x

    math::float3 shadowBias; // constant bias, normal bias, unused
    float oneOverFroxelDimensionY;

    math::float4 zParams; // froxel Z parameters

    math::uint2 fParams; // stride-y, stride-z
    math::float2 origin; // viewport left, viewport bottom

    float oneOverFroxelDimensionX;
    float iblLuminance;
    float exposure;
    float ev100;

    alignas(16) math::float4 iblSH[9]; // actually float3 entries (std140 requires float4 alignment)

    math::float4 userTime;  // time(s), (double)time - (float)time, 0, 0
};


// PerRenderableUib must have an alignment of 256 to be compatible with all versions of GLES.
struct alignas(256) PerRenderableUib {
    math::mat4f worldFromModelMatrix;
    math::mat3f worldFromModelNormalMatrix;
};

struct LightsUib {
    static const UniformInterfaceBlock& getUib() noexcept {
        return UibGenerator::getLightsUib();
    }
    math::float4 positionFalloff;   // { float3(pos), 1/falloff^2 }
    math::float4 colorIntensity;    // { float3(col), intensity }
    math::float4 directionIES;      // { float3(dir), IES index }
    math::float4 spotScaleOffset;   // { scale, offset, unused, unused }
};

struct PostProcessingUib {
    static const UniformInterfaceBlock& getUib() noexcept {
        return UibGenerator::getPostProcessingUib();
    }
    math::float2 uvScale;
    float time;             // time in seconds, with a 1 second period, used for dithering
    float yOffset;
};

// This is not the UBO proper, but just an element of a bone array.
struct PerRenderableUibBone {
    math::quatf q = { 1, 0, 0, 0 };
    math::float4 t = {};
    math::float4 s = { 1, 1, 1, 0 };
    math::float4 ns = { 1, 1, 1, 0 };
};

} // namespace filament

#endif // TNT_FILABRIDGE_UIBGENERATOR_H
