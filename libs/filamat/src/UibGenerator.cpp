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
    static UniformInterfaceBlock uib = UniformInterfaceBlock::Builder()
            .name(PerViewUib::_name)
            // transforms
            .add("viewFromWorldMatrix",     UniformInterfaceBlock::Type::MAT4, Precision::HIGH)
            .add("worldFromViewMatrix",     UniformInterfaceBlock::Type::MAT4, Precision::HIGH)
            .add("clipFromViewMatrix",      UniformInterfaceBlock::Type::MAT4, Precision::HIGH)
            .add("viewFromClipMatrix",      UniformInterfaceBlock::Type::MAT4, Precision::HIGH)
            .add("clipFromWorldMatrix",     UniformInterfaceBlock::Type::MAT4, Precision::HIGH)
            .add("worldFromClipMatrix",     UniformInterfaceBlock::Type::MAT4, Precision::HIGH)
            .add("clipTransform",           UniformInterfaceBlock::Type::FLOAT4, Precision::HIGH)

            .add("clipControl",             UniformInterfaceBlock::Type::FLOAT2)
            .add("time",                    UniformInterfaceBlock::Type::FLOAT, Precision::HIGH)
            .add("temporalNoise",           UniformInterfaceBlock::Type::FLOAT, Precision::HIGH)
            .add("userTime",                UniformInterfaceBlock::Type::FLOAT4, Precision::HIGH)

            // ------------------------------------------------------------------------------------
            // values below should only be accessed in surface materials
            // ------------------------------------------------------------------------------------

            .add("origin",                  UniformInterfaceBlock::Type::FLOAT2, Precision::HIGH)
            .add("offset",                  UniformInterfaceBlock::Type::FLOAT2, Precision::HIGH)
            .add("resolution",              UniformInterfaceBlock::Type::FLOAT4, Precision::HIGH)

            .add("lodBias",                 UniformInterfaceBlock::Type::FLOAT)
            .add("refractionLodOffset",     UniformInterfaceBlock::Type::FLOAT)
            .add("padding1",                UniformInterfaceBlock::Type::FLOAT)
            .add("padding2",                UniformInterfaceBlock::Type::FLOAT)

            .add("cameraPosition",          UniformInterfaceBlock::Type::FLOAT3, Precision::HIGH)
            .add("oneOverFarMinusNear",     UniformInterfaceBlock::Type::FLOAT, Precision::HIGH)
            .add("worldOffset",             UniformInterfaceBlock::Type::FLOAT3)
            .add("nearOverFarMinusNear",    UniformInterfaceBlock::Type::FLOAT, Precision::HIGH)
            .add("cameraFar",               UniformInterfaceBlock::Type::FLOAT)
            .add("exposure",                UniformInterfaceBlock::Type::FLOAT, Precision::HIGH) // high precision to work around #3602 (qualcomm)
            .add("ev100",                   UniformInterfaceBlock::Type::FLOAT)
            .add("needsAlphaChannel",       UniformInterfaceBlock::Type::FLOAT)

            // AO
            .add("aoSamplingQualityAndEdgeDistance", UniformInterfaceBlock::Type::FLOAT)
            .add("aoBentNormals",           UniformInterfaceBlock::Type::FLOAT)
            .add("aoReserved0",             UniformInterfaceBlock::Type::FLOAT)
            .add("aoReserved1",             UniformInterfaceBlock::Type::FLOAT)

            // ------------------------------------------------------------------------------------
            // Dynamic Lighting [variant: DYN]
            // ------------------------------------------------------------------------------------
            .add("zParams",                 UniformInterfaceBlock::Type::FLOAT4)
            .add("fParams",                 UniformInterfaceBlock::Type::UINT3)
            .add("lightChannels",           UniformInterfaceBlock::Type::UINT)
            .add("froxelCountXY",           UniformInterfaceBlock::Type::FLOAT2)

            .add("iblLuminance",            UniformInterfaceBlock::Type::FLOAT)
            .add("iblRoughnessOneLevel",    UniformInterfaceBlock::Type::FLOAT)
            .add("iblSH",                   9, UniformInterfaceBlock::Type::FLOAT3)

            // ------------------------------------------------------------------------------------
            // Directional Lighting [variant: DIR]
            // ------------------------------------------------------------------------------------
            .add("lightDirection",            UniformInterfaceBlock::Type::FLOAT3)
            .add("padding0",                  UniformInterfaceBlock::Type::FLOAT)
            .add("lightColorIntensity",       UniformInterfaceBlock::Type::FLOAT4)
            .add("sun",                       UniformInterfaceBlock::Type::FLOAT4)
            .add("lightFarAttenuationParams", UniformInterfaceBlock::Type::FLOAT2)

            // ------------------------------------------------------------------------------------
            // Directional light shadowing [variant: SRE | DIR]
            // ------------------------------------------------------------------------------------
            .add("directionalShadows",      UniformInterfaceBlock::Type::UINT)
            .add("ssContactShadowDistance", UniformInterfaceBlock::Type::FLOAT)

            .add("cascadeSplits",            UniformInterfaceBlock::Type::FLOAT4, Precision::HIGH)
            .add("cascades",                 UniformInterfaceBlock::Type::UINT)
            .add("shadowBulbRadiusLs",       UniformInterfaceBlock::Type::FLOAT)
            .add("shadowBias",               UniformInterfaceBlock::Type::FLOAT)
            .add("shadowPenumbraRatioScale", UniformInterfaceBlock::Type::FLOAT)
            .add("lightFromWorldMatrix",    4, UniformInterfaceBlock::Type::MAT4, Precision::HIGH)

            // ------------------------------------------------------------------------------------
            // VSM shadows [variant: VSM]
            // ------------------------------------------------------------------------------------
            .add("vsmExponent",             UniformInterfaceBlock::Type::FLOAT)
            .add("vsmDepthScale",           UniformInterfaceBlock::Type::FLOAT)
            .add("vsmLightBleedReduction",  UniformInterfaceBlock::Type::FLOAT)
            .add("shadowSamplingType",      UniformInterfaceBlock::Type::UINT)

            // ------------------------------------------------------------------------------------
            // Fog [variant: FOG]
            // ------------------------------------------------------------------------------------
            .add("fogStart",                UniformInterfaceBlock::Type::FLOAT)
            .add("fogMaxOpacity",           UniformInterfaceBlock::Type::FLOAT)
            .add("fogHeight",               UniformInterfaceBlock::Type::FLOAT)
            .add("fogHeightFalloff",        UniformInterfaceBlock::Type::FLOAT)
            .add("fogColor",                UniformInterfaceBlock::Type::FLOAT3)
            .add("fogDensity",              UniformInterfaceBlock::Type::FLOAT)
            .add("fogInscatteringStart",    UniformInterfaceBlock::Type::FLOAT)
            .add("fogInscatteringSize",     UniformInterfaceBlock::Type::FLOAT)
            .add("fogColorFromIbl",         UniformInterfaceBlock::Type::FLOAT)
            .add("fogReserved0",            UniformInterfaceBlock::Type::FLOAT)

            // ------------------------------------------------------------------------------------
            // Screen-space reflections [variant: SSR (i.e.: VSM | SRE)]
            // ------------------------------------------------------------------------------------
            .add("ssrReprojection",         UniformInterfaceBlock::Type::MAT4, Precision::HIGH)
            .add("ssrUvFromViewMatrix",     UniformInterfaceBlock::Type::MAT4, Precision::HIGH)
            .add("ssrThickness",            UniformInterfaceBlock::Type::FLOAT)
            .add("ssrBias",                 UniformInterfaceBlock::Type::FLOAT)
            .add("ssrDistance",             UniformInterfaceBlock::Type::FLOAT)
            .add("ssrStride",               UniformInterfaceBlock::Type::FLOAT)

            // bring PerViewUib to 2 KiB
            .add("reserved", sizeof(PerViewUib::reserved)/16, UniformInterfaceBlock::Type::FLOAT4)
            .build();
    return uib;
}

UniformInterfaceBlock const& UibGenerator::getPerRenderableUib() noexcept {
    static UniformInterfaceBlock uib =  UniformInterfaceBlock::Builder()
            .name(PerRenderableUib::_name)
            .add("data", 64, "PerRenderableData", sizeof(PerRenderableData))
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
            .name(PerRenderableBoneUib::_name)
            .add("bones", CONFIG_MAX_BONE_COUNT, "BoneData", sizeof(PerRenderableBoneUib::BoneData))
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
