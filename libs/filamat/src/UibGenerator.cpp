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

#include "UibGenerator.h"
#include "private/filament/UibStructs.h"

#include "private/filament/UniformInterfaceBlock.h"

#include <private/filament/EngineEnums.h>
#include <backend/DriverEnums.h>

namespace filament {

using namespace backend;

static_assert(CONFIG_MAX_SHADOW_CASCADES == 4,
        "Changing CONFIG_MAX_SHADOW_CASCADES affects PerView size and breaks materials.");

UniformInterfaceBlock const& UibGenerator::getPerViewUib() noexcept  {
    // IMPORTANT NOTE: Respect std140 layout, don't update without updating Engine::PerViewUib
    static UniformInterfaceBlock uib = UniformInterfaceBlock::Builder()
            .name(PerViewUib::_name)
            // transforms
            .add("viewFromWorldMatrix",     1, UniformInterfaceBlock::Type::MAT4, Precision::HIGH)
            .add("worldFromViewMatrix",     1, UniformInterfaceBlock::Type::MAT4, Precision::HIGH)
            .add("clipFromViewMatrix",      1, UniformInterfaceBlock::Type::MAT4, Precision::HIGH)
            .add("viewFromClipMatrix",      1, UniformInterfaceBlock::Type::MAT4, Precision::HIGH)
            .add("clipFromWorldMatrix",     1, UniformInterfaceBlock::Type::MAT4, Precision::HIGH)
            .add("worldFromClipMatrix",     1, UniformInterfaceBlock::Type::MAT4, Precision::HIGH)

            .add("clipControl",             1, UniformInterfaceBlock::Type::FLOAT2)
            .add("time",                    1, UniformInterfaceBlock::Type::FLOAT, Precision::HIGH)
            .add("temporalNoise",           1, UniformInterfaceBlock::Type::FLOAT, Precision::HIGH)
            .add("userTime",                1, UniformInterfaceBlock::Type::FLOAT4, Precision::HIGH)

            // ------------------------------------------------------------------------------------
            // values below should only be accessed in surface materials
            // ------------------------------------------------------------------------------------

            .add("origin",                  1, UniformInterfaceBlock::Type::FLOAT2, Precision::HIGH)
            .add("offset",                  1, UniformInterfaceBlock::Type::FLOAT2, Precision::HIGH)
            .add("resolution",              1, UniformInterfaceBlock::Type::FLOAT4, Precision::HIGH)

            .add("lodBias",                 1, UniformInterfaceBlock::Type::FLOAT)
            .add("refractionLodOffset",     1, UniformInterfaceBlock::Type::FLOAT)
            .add("padding1",                1, UniformInterfaceBlock::Type::FLOAT)
            .add("padding2",                1, UniformInterfaceBlock::Type::FLOAT)

            .add("cameraPosition",          1, UniformInterfaceBlock::Type::FLOAT3, Precision::HIGH)
            .add("oneOverFarMinusNear",     1, UniformInterfaceBlock::Type::FLOAT, Precision::HIGH)
            .add("worldOffset",             1, UniformInterfaceBlock::Type::FLOAT3)
            .add("nearOverFarMinusNear",    1, UniformInterfaceBlock::Type::FLOAT, Precision::HIGH)
            .add("cameraFar",               1, UniformInterfaceBlock::Type::FLOAT)
            .add("exposure",                1, UniformInterfaceBlock::Type::FLOAT, Precision::HIGH) // high precision to work around #3602 (qualcomm)
            .add("ev100",                   1, UniformInterfaceBlock::Type::FLOAT)
            .add("needsAlphaChannel",       1, UniformInterfaceBlock::Type::FLOAT)

            // AO
            .add("aoSamplingQualityAndEdgeDistance", 1, UniformInterfaceBlock::Type::FLOAT)
            .add("aoBentNormals",           1, UniformInterfaceBlock::Type::FLOAT)
            .add("aoReserved0",             1, UniformInterfaceBlock::Type::FLOAT)
            .add("aoReserved1",             1, UniformInterfaceBlock::Type::FLOAT)

            // ------------------------------------------------------------------------------------
            // Dynamic Lighting [variant: DYN]
            // ------------------------------------------------------------------------------------
            .add("zParams",                 1, UniformInterfaceBlock::Type::FLOAT4)
            .add("fParams",                 1, UniformInterfaceBlock::Type::UINT3)
            .add("lightChannels",           1, UniformInterfaceBlock::Type::UINT)
            .add("froxelCountXY",           1, UniformInterfaceBlock::Type::FLOAT2)

            .add("iblLuminance",            1, UniformInterfaceBlock::Type::FLOAT)
            .add("iblRoughnessOneLevel",    1, UniformInterfaceBlock::Type::FLOAT)
            .add("iblSH",                   9, UniformInterfaceBlock::Type::FLOAT3)

            // ------------------------------------------------------------------------------------
            // Directional Lighting [variant: DIR]
            // ------------------------------------------------------------------------------------
            .add("lightDirection",          1, UniformInterfaceBlock::Type::FLOAT3)
            .add("padding0",                1, UniformInterfaceBlock::Type::FLOAT)
            .add("lightColorIntensity",     1, UniformInterfaceBlock::Type::FLOAT4)
            .add("sun",                     1, UniformInterfaceBlock::Type::FLOAT4)
            .add("lightFarAttenuationParams",1, UniformInterfaceBlock::Type::FLOAT2)

            // ------------------------------------------------------------------------------------
            // Directional light shadowing [variant: SRE | DIR]
            // ------------------------------------------------------------------------------------
            .add("directionalShadows",      1, UniformInterfaceBlock::Type::UINT)
            .add("ssContactShadowDistance", 1, UniformInterfaceBlock::Type::FLOAT)

            .add("cascadeSplits",           1, UniformInterfaceBlock::Type::FLOAT4, Precision::HIGH)
            .add("cascades",                1, UniformInterfaceBlock::Type::UINT)
            .add("shadowBulbRadiusLs",      1, UniformInterfaceBlock::Type::FLOAT)
            .add("shadowBias",              1, UniformInterfaceBlock::Type::FLOAT)
            .add("shadowPenumbraRatioScale",1, UniformInterfaceBlock::Type::FLOAT)
            .add("lightFromWorldMatrix",    4, UniformInterfaceBlock::Type::MAT4, Precision::HIGH)

            // ------------------------------------------------------------------------------------
            // VSM shadows [variant: VSM]
            // ------------------------------------------------------------------------------------
            .add("vsmExponent",             1, UniformInterfaceBlock::Type::FLOAT)
            .add("vsmDepthScale",           1, UniformInterfaceBlock::Type::FLOAT)
            .add("vsmLightBleedReduction",  1, UniformInterfaceBlock::Type::FLOAT)
            .add("shadowSamplingType",      1, UniformInterfaceBlock::Type::UINT)

            // ------------------------------------------------------------------------------------
            // Fog [variant: FOG]
            // ------------------------------------------------------------------------------------
            .add("fogStart",                1, UniformInterfaceBlock::Type::FLOAT)
            .add("fogMaxOpacity",           1, UniformInterfaceBlock::Type::FLOAT)
            .add("fogHeight",               1, UniformInterfaceBlock::Type::FLOAT)
            .add("fogHeightFalloff",        1, UniformInterfaceBlock::Type::FLOAT)
            .add("fogColor",                1, UniformInterfaceBlock::Type::FLOAT3)
            .add("fogDensity",              1, UniformInterfaceBlock::Type::FLOAT)
            .add("fogInscatteringStart",    1, UniformInterfaceBlock::Type::FLOAT)
            .add("fogInscatteringSize",     1, UniformInterfaceBlock::Type::FLOAT)
            .add("fogColorFromIbl",         1, UniformInterfaceBlock::Type::FLOAT)
            .add("fogReserved0",            1, UniformInterfaceBlock::Type::FLOAT)

            // ------------------------------------------------------------------------------------
            // Screen-space reflections [variant: SSR (i.e.: VSM | SRE)]
            // ------------------------------------------------------------------------------------
            .add("ssrReprojection",         1, UniformInterfaceBlock::Type::MAT4, Precision::HIGH)
            .add("ssrUvFromViewMatrix",     1, UniformInterfaceBlock::Type::MAT4, Precision::HIGH)
            .add("ssrThickness",            1, UniformInterfaceBlock::Type::FLOAT)
            .add("ssrBias",                 1, UniformInterfaceBlock::Type::FLOAT)
            .add("ssrDistance",             1, UniformInterfaceBlock::Type::FLOAT)
            .add("ssrStride",               1, UniformInterfaceBlock::Type::FLOAT)

            // bring PerViewUib to 2 KiB
            .add("reserved", sizeof(PerViewUib::reserved)/16, UniformInterfaceBlock::Type::FLOAT4)
            .build();
    return uib;
}

UniformInterfaceBlock const& UibGenerator::getPerRenderableUib() noexcept {
    static UniformInterfaceBlock uib =  UniformInterfaceBlock::Builder()
            .name(PerRenderableUib::_name)
            .add("worldFromModelMatrix",       1, UniformInterfaceBlock::Type::MAT4, Precision::HIGH)
            .add("worldFromModelNormalMatrix", 1, UniformInterfaceBlock::Type::MAT3, Precision::HIGH)
            .add("morphTargetCount", 1, UniformInterfaceBlock::Type::UINT)
            .add("flagsChannels", 1, UniformInterfaceBlock::Type::UINT)
            .add("objectId", 1, UniformInterfaceBlock::Type::UINT)
            .add("userData", 1, UniformInterfaceBlock::Type::FLOAT)
            .add("reserved", 8, UniformInterfaceBlock::Type::FLOAT4)
            .build();
    return uib;
}

UniformInterfaceBlock const& UibGenerator::getLightsUib() noexcept {
    static UniformInterfaceBlock uib = UniformInterfaceBlock::Builder()
            .name(LightsUib::_name)
            .add("lights", CONFIG_MAX_LIGHT_COUNT, UniformInterfaceBlock::Type::MAT4, Precision::HIGH)
            .build();
    return uib;
}

UniformInterfaceBlock const& UibGenerator::getShadowUib() noexcept {
    static UniformInterfaceBlock uib = UniformInterfaceBlock::Builder()
            .name(ShadowUib::_name)
            .add("shadows", CONFIG_MAX_SHADOW_CASTING_SPOTS, "ShadowData", sizeof(ShadowUib::ShadowData))
            .build();
    return uib;
}

UniformInterfaceBlock const& UibGenerator::getPerRenderableBonesUib() noexcept {
    static UniformInterfaceBlock uib = UniformInterfaceBlock::Builder()
            .name(PerRenderableUibBone::_name)
            .add("bones", CONFIG_MAX_BONE_COUNT, "BoneData", sizeof(PerRenderableUibBone::BoneData))
            .build();
    return uib;
}

UniformInterfaceBlock const& UibGenerator::getPerRenderableMorphingUib() noexcept {
    static UniformInterfaceBlock uib = UniformInterfaceBlock::Builder()
            .name(PerRenderableMorphingUib::_name)
            .add("weights", CONFIG_MAX_MORPH_TARGET_COUNT, UniformInterfaceBlock::Type::FLOAT4)
            .build();
    return uib;
}

UniformInterfaceBlock const& UibGenerator::getFroxelRecordUib() noexcept {
    static UniformInterfaceBlock uib = UniformInterfaceBlock::Builder()
            .name(FroxelRecordUib::_name)
            .add("records", 1024, UniformInterfaceBlock::Type::UINT4, Precision::HIGH)
            .build();
    return uib;
}

} // namespace filament
