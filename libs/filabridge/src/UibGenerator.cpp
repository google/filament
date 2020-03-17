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

#include "private/filament/UibGenerator.h"

#include "private/filament/UniformInterfaceBlock.h"

#include <private/filament/EngineEnums.h>
#include <backend/DriverEnums.h>

namespace filament {

using namespace backend;

static_assert(sizeof(PerRenderableUib) % 256 == 0,
        "sizeof(Transform) should be a multiple of 256");

static_assert(CONFIG_MAX_BONE_COUNT * sizeof(PerRenderableUibBone) <= 16384,
        "Bones exceed max UBO size");


UniformInterfaceBlock const& UibGenerator::getPerViewUib() noexcept  {
    // IMPORTANT NOTE: Respect std140 layout, don't update without updating Engine::PerViewUib
    static UniformInterfaceBlock uib = UniformInterfaceBlock::Builder()
            .name("FrameUniforms")
            // transforms
            .add("viewFromWorldMatrix",     1, UniformInterfaceBlock::Type::MAT4, Precision::HIGH)
            .add("worldFromViewMatrix",     1, UniformInterfaceBlock::Type::MAT4, Precision::HIGH)
            .add("clipFromViewMatrix",      1, UniformInterfaceBlock::Type::MAT4, Precision::HIGH)
            .add("viewFromClipMatrix",      1, UniformInterfaceBlock::Type::MAT4, Precision::HIGH)
            .add("clipFromWorldMatrix",     1, UniformInterfaceBlock::Type::MAT4, Precision::HIGH)
            .add("worldFromClipMatrix",     1, UniformInterfaceBlock::Type::MAT4, Precision::HIGH)
            .add("lightFromWorldMatrix",    1, UniformInterfaceBlock::Type::MAT4, Precision::HIGH)
            // view
            .add("resolution",              1, UniformInterfaceBlock::Type::FLOAT4, Precision::HIGH)
            // camera
            .add("cameraPosition",          1, UniformInterfaceBlock::Type::FLOAT3, Precision::HIGH)
            // time
            .add("time",                    1, UniformInterfaceBlock::Type::FLOAT, Precision::HIGH)
            // directional light
            .add("lightColorIntensity",     1, UniformInterfaceBlock::Type::FLOAT4)
            .add("sun",                     1, UniformInterfaceBlock::Type::FLOAT4)
            .add("lightDirection",          1, UniformInterfaceBlock::Type::FLOAT3)
            .add("fParamsX",                1, UniformInterfaceBlock::Type::UINT)
            // shadow
            .add("shadowBias",              1, UniformInterfaceBlock::Type::FLOAT3)
            .add("oneOverFroxelDimensionY", 1, UniformInterfaceBlock::Type::FLOAT)
            // froxels
            .add("zParams",                 1, UniformInterfaceBlock::Type::FLOAT4)
            .add("fParams",                 1, UniformInterfaceBlock::Type::UINT2)
            .add("origin",                  1, UniformInterfaceBlock::Type::FLOAT2)
            // froxels (again, for alignment purposes)
            .add("oneOverFroxelDimension",  1, UniformInterfaceBlock::Type::FLOAT)
            // ibl
            .add("iblLuminance",            1, UniformInterfaceBlock::Type::FLOAT)
            // camera
            .add("exposure",                1, UniformInterfaceBlock::Type::FLOAT)
            .add("ev100",                   1, UniformInterfaceBlock::Type::FLOAT)
            // ibl
            .add("iblSH",                   9, UniformInterfaceBlock::Type::FLOAT3)
            // user time
            .add("userTime",                1, UniformInterfaceBlock::Type::FLOAT4)
            // ibl max mip level
            .add("iblMaxMipLevel",          1, UniformInterfaceBlock::Type::FLOAT2)
            .add("refractionLodOffset",     1, UniformInterfaceBlock::Type::FLOAT)
            .add("directionalShadows",      1, UniformInterfaceBlock::Type::UINT)
            // view
            .add("worldOffset",             1, UniformInterfaceBlock::Type::FLOAT3)
            .add("ssContactShadowDistance", 1, UniformInterfaceBlock::Type::FLOAT)
            // fog
            .add("fogStart",                1, UniformInterfaceBlock::Type::FLOAT)
            .add("fogMinOpacity",           1, UniformInterfaceBlock::Type::FLOAT)
            .add("fogHeight",               1, UniformInterfaceBlock::Type::FLOAT)
            .add("fogHeightFalloff",        1, UniformInterfaceBlock::Type::FLOAT)
            .add("fogColor",                1, UniformInterfaceBlock::Type::FLOAT3)
            .add("fogDensity",              1, UniformInterfaceBlock::Type::FLOAT)
            .add("fogInscatteringStart",    1, UniformInterfaceBlock::Type::FLOAT)
            .add("fogInscatteringSize",     1, UniformInterfaceBlock::Type::FLOAT)
            // more camera stuff
            .add("cameraNear",              1, UniformInterfaceBlock::Type::FLOAT)
            .add("cameraFar",               1, UniformInterfaceBlock::Type::FLOAT)

            // bring size to 1 KiB
            .add("padding2",                12, UniformInterfaceBlock::Type::FLOAT4)
            .build();
    return uib;
}

UniformInterfaceBlock const& UibGenerator::getPerRenderableUib() noexcept {
    static UniformInterfaceBlock uib =  UniformInterfaceBlock::Builder()
            .name("ObjectUniforms")
            .add("worldFromModelMatrix",       1, UniformInterfaceBlock::Type::MAT4, Precision::HIGH)
            .add("worldFromModelNormalMatrix", 1, UniformInterfaceBlock::Type::MAT3, Precision::HIGH)
            .add("morphWeights", 1, UniformInterfaceBlock::Type::FLOAT4, Precision::HIGH)
            .add("skinningEnabled", 1, UniformInterfaceBlock::Type::INT)
            .add("morphingEnabled", 1, UniformInterfaceBlock::Type::INT)
            .add("screenSpaceContactShadows", 1, UniformInterfaceBlock::Type::INT)
            .add("padding0", 1, UniformInterfaceBlock::Type::FLOAT)
            .build();
    return uib;
}

UniformInterfaceBlock const& UibGenerator::getLightsUib() noexcept {
    static UniformInterfaceBlock uib = UniformInterfaceBlock::Builder()
            .name("LightsUniforms")
            .add("lights", CONFIG_MAX_LIGHT_COUNT, UniformInterfaceBlock::Type::MAT4, Precision::HIGH)
            .build();
    return uib;
}

UniformInterfaceBlock const& UibGenerator::getShadowUib() noexcept {
    static UniformInterfaceBlock uib = UniformInterfaceBlock::Builder()
            .name("ShadowUniforms")
            .add("spotLightFromWorldMatrix", CONFIG_MAX_SHADOW_CASTING_SPOTS, UniformInterfaceBlock::Type::MAT4, Precision::HIGH)
            .add("directionShadowBias", CONFIG_MAX_SHADOW_CASTING_SPOTS, UniformInterfaceBlock::Type::FLOAT4, Precision::HIGH)
            .build();
    return uib;
}

UniformInterfaceBlock const& UibGenerator::getPerRenderableBonesUib() noexcept {
    static UniformInterfaceBlock uib = UniformInterfaceBlock::Builder()
            .name("BonesUniforms")
            .add("bones", CONFIG_MAX_BONE_COUNT * 4, UniformInterfaceBlock::Type::FLOAT4, Precision::MEDIUM)
            .build();
    return uib;
}

} // namespace filament
