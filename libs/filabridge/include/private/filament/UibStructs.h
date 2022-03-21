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

#ifndef TNT_FILABRIDGE_UIBSTRUCTS_H
#define TNT_FILABRIDGE_UIBSTRUCTS_H

#include <math/mat4.h>
#include <math/vec4.h>

#include <private/filament/EngineEnums.h>

#include <utils/CString.h>

/*
 * Here we define all the UBOs known by filament as C structs. It is used by filament to
 * fill the uniform values and get the interface block names. It is also used by filabridge to
 * get the interface block names.
 */

namespace filament {

/*
 * These structures are only used to call offsetof() and make it easy to visualize the UBO.
 *
 * IMPORTANT NOTE: Respect std140 layout, don't update without updating getUib()
 */

struct PerViewUib { // NOLINT(cppcoreguidelines-pro-type-member-init)
    static constexpr utils::StaticString _name{ "FrameUniforms" };
    math::mat4f viewFromWorldMatrix;
    math::mat4f worldFromViewMatrix;
    math::mat4f clipFromViewMatrix;
    math::mat4f viewFromClipMatrix;
    math::mat4f clipFromWorldMatrix;
    math::mat4f worldFromClipMatrix;
    std::array<math::mat4f, CONFIG_MAX_SHADOW_CASCADES> lightFromWorldMatrix;

    // position of cascade splits, in world space (not including the near plane)
    // -Inf stored in unused components
    math::float4 cascadeSplits;

    math::float4 resolution; // viewport width, height, 1/width, 1/height

    // camera position in view space (when camera_at_origin is enabled), i.e. it's (0,0,0).
    // Always add worldOffset in the shader to get the true world-space position of the camera.
    math::float3 cameraPosition;

    float time; // time in seconds, with a 1 second period

    math::float4 lightColorIntensity; // directional light

    math::float4 sun; // cos(sunAngle), sin(sunAngle), 1/(sunAngle*HALO_SIZE-sunAngle), HALO_EXP

    math::float2 lightFarAttenuationParams;     // a, a/far (a=1/pct-of-far)
    float needsAlphaChannel;
    uint32_t lightChannels;

    math::float3 lightDirection;
    uint32_t fParamsX; // stride-x

    float shadowBulbRadiusLs;       // light radius in light-space
    float shadowBias;               // normal bias
    float shadowPenumbraRatioScale; // For DPCF or PCSS, scale penumbra ratio for artistic use
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

    float iblRoughnessOneLevel;       // level for roughness == 1
    float cameraFar;                  // camera *culling* far-plane distance, always positive (projection far is at +inf)
    float refractionLodOffset;

    // bit 0: directional (sun) shadow enabled
    // bit 1: directional (sun) screen-space contact shadow enabled
    // bit 8-15: screen-space contact shadows ray casting steps
    uint32_t directionalShadows;

    math::float3 worldOffset; // this is (0,0,0) when camera_at_origin is disabled
    float ssContactShadowDistance;

    // fog
    float fogStart;
    float fogMaxOpacity;
    float fogHeight;
    float fogHeightFalloff;         // falloff * 1.44269
    math::float3 fogColor;
    float fogDensity;               // (density/falloff)*exp(-falloff*(camera.y - fogHeight))
    float fogInscatteringStart;
    float fogInscatteringSize;
    float fogColorFromIbl;

    // bit 0-3: cascade count
    // bit 4: visualize cascades
    // bit 8-11: cascade has visible shadows
    uint32_t cascades;

    float aoSamplingQualityAndEdgeDistance;     // 0: bilinear, !0: bilateral edge distance
    float aoBentNormals;                        // 0: no AO bent normal, >0.0 AO bent normals
    float aoReserved2;
    float aoReserved3;

    math::float2 clipControl;
    math::float2 padding1;

    float vsmExponent;
    float vsmDepthScale;
    float vsmLightBleedReduction;
    uint32_t shadowSamplingType;                // 0: vsm, 1: dpcf

    float lodBias;
    float oneOverFarMinusNear;          // 1 / (f-n), always positive
    float nearOverFarMinusNear;         // n / (f-n), always positive
    float temporalNoise;                // noise [0,1] when TAA is used, 0 otherwise

    // Screen-space reflections
    math::mat4f ssrReprojection;
    math::mat4f ssrUvFromViewMatrix;
    float ssrThickness;                 // ssr thickness, in world units
    float ssrBias;                      // ssr bias, in world units
    float ssrDistance;                  // ssr world raycast distance, 0 when ssr is off
    float ssrStride;                    // ssr texel stride, >= 1.0

    // bring PerViewUib to 2 KiB
    math::float4 padding3[49];
};

// 2 KiB == 128 float4s
static_assert(sizeof(PerViewUib) == sizeof(math::float4) * 128,
        "PerViewUib should be exactly 2KiB");

// PerRenderableUib must have an alignment of 256 to be compatible with all versions of GLES.
struct alignas(256) PerRenderableUib { // NOLINT(cppcoreguidelines-pro-type-member-init)
    static constexpr utils::StaticString _name{ "ObjectUniforms" };
    math::mat4f worldFromModelMatrix;
    math::mat3f worldFromModelNormalMatrix;   // this gets expanded to 48 bytes during the copy to the UBO
    alignas(16) uint32_t morphTargetCount;
    uint32_t flags;                           // see packFlags() below
    uint32_t channels;                        // 0x000000ll
    uint32_t objectId;                        // used for picking
    // TODO: We need a better solution, this currently holds the average local scale for the renderable
    float userData;

    static uint32_t packFlags(bool skinning, bool morphing, bool contactShadows) noexcept {
        return (skinning ? 1 : 0) |
               (morphing ? 2 : 0) |
               (contactShadows ? 4 : 0);
    }
};
static_assert(sizeof(PerRenderableUib) % 256 == 0, "sizeof(Transform) should be a multiple of 256");

struct LightsUib { // NOLINT(cppcoreguidelines-pro-type-member-init)
    static constexpr utils::StaticString _name{ "LightsUniforms" };
    math::float4 positionFalloff;     // { float3(pos), 1/falloff^2 }
    math::float3 direction;           // dir
    float reserved1;                  // 0
    math::half4 colorIES;             // { half3(col),  IES index   }
    math::float2 spotScaleOffset;     // { scale, offset }
    float reserved3;                  // 0
    float intensity;                  // float
    uint32_t typeShadow;              // 0x00.ll.ii.ct (t: 0=point, 1=spot, c:contact, ii: index, ll: layer)
    uint32_t channels;                // 0x000c00ll (ll: light channels, c: caster)

    static uint32_t packTypeShadow(uint8_t type, bool contactShadow, uint8_t index, uint8_t layer) noexcept {
        return (type & 0xF) | (contactShadow ? 0x10 : 0x00) | (index << 8) | (layer << 16);
    }
    static uint32_t packChannels(uint8_t lightChannels, bool castShadows) noexcept {
        return lightChannels | (castShadows ? 0x10000 : 0);
    }
};
static_assert(sizeof(LightsUib) == 64, "the actual UBO is an array of 256 mat4");

// UBO for punctual (spot light) shadows.
struct ShadowUib { // NOLINT(cppcoreguidelines-pro-type-member-init)
    static constexpr utils::StaticString _name{ "ShadowUniforms" };
    struct alignas(16) ShadowData {
        math::mat4f lightFromWorldMatrix;
        math::float3 direction;
        float normalBias;
        math::float4 lightFromWorldZ;

        float texelSizeAtOneMeter;
        float bulbRadiusLs;
        float nearOverFarMinusNear;
    };
    ShadowData shadows[CONFIG_MAX_SHADOW_CASTING_SPOTS];
};
static_assert(sizeof(ShadowUib) <= 16384, "ShadowUib exceed max UBO size");

// UBO froxel record buffer.
struct FroxelRecordUib { // NOLINT(cppcoreguidelines-pro-type-member-init)
    static constexpr utils::StaticString _name{ "FroxelRecordUniforms" };
    math::uint4 records[1024];
};
static_assert(sizeof(FroxelRecordUib) == 16384, "FroxelRecordUib should be exactly 16KiB");

// This is not the UBO proper, but just an element of a bone array.
struct PerRenderableUibBone { // NOLINT(cppcoreguidelines-pro-type-member-init)
    static constexpr utils::StaticString _name{ "BonesUniforms" };
    struct alignas(16) BoneData {
        // bone transform, last row assumed [0,0,0,1]
        math::float4 transform[3];
        // 8 first cofactor matrix of transform's upper left
        math::uint4 cof;
    };
    BoneData bone;
};
static_assert(CONFIG_MAX_BONE_COUNT * sizeof(PerRenderableUibBone) <= 16384,
        "PerRenderableUibBone exceed max UBO size");

struct alignas(16) PerRenderableMorphingUib {
    static constexpr utils::StaticString _name{ "MorphingUniforms" };
    // The array stride(the bytes between array elements) is always rounded up to the size of a vec4 in std140.
    math::float4 weights[CONFIG_MAX_MORPH_TARGET_COUNT];
};
static_assert(sizeof(PerRenderableMorphingUib) <= 16384,
        "PerRenderableMorphingUib exceed max UBO size");

} // namespace filament

#endif // TNT_FILABRIDGE_UIBSTRUCTS_H
