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

#include "private/filament/BufferInterfaceBlock.h"

#include <private/filament/EngineEnums.h>
#include <backend/DriverEnums.h>

#include <utils/debug.h>

#include <stdlib.h>

namespace filament {

using namespace backend;

BufferInterfaceBlock const& UibGenerator::get(UibGenerator::Ubo ubo) noexcept {
    assert_invariant(ubo != Ubo::MaterialParams);
    switch (ubo) {
        case Ubo::FrameUniforms:
            return getPerViewUib();
        case Ubo::ObjectUniforms:
            return getPerRenderableUib();
        case Ubo::BonesUniforms:
            return getPerRenderableBonesUib();
        case Ubo::MorphingUniforms:
            return getPerRenderableMorphingUib();
        case Ubo::LightsUniforms:
            return getLightsUib();
        case Ubo::ShadowUniforms:
            return getShadowUib();
        case Ubo::FroxelRecordUniforms:
            return getFroxelRecordUib();
        case Ubo::FroxelsUniforms:
            return getFroxelsUib();
        case Ubo::MaterialParams:
            abort();
    }
}

UibGenerator::Binding UibGenerator::getBinding(UibGenerator::Ubo ubo) noexcept {
    switch (ubo) {
        case Ubo::FrameUniforms:
            return { +DescriptorSetBindingPoints::PER_VIEW,
                     +PerViewBindingPoints::FRAME_UNIFORMS };
        case Ubo::ObjectUniforms:
            return { +DescriptorSetBindingPoints::PER_RENDERABLE,
                     +PerRenderableBindingPoints::OBJECT_UNIFORMS };
        case Ubo::BonesUniforms:
            return { +DescriptorSetBindingPoints::PER_RENDERABLE,
                     +PerRenderableBindingPoints::BONES_UNIFORMS };
        case Ubo::MorphingUniforms:
            return { +DescriptorSetBindingPoints::PER_RENDERABLE,
                     +PerRenderableBindingPoints::MORPHING_UNIFORMS };
        case Ubo::LightsUniforms:
            return { +DescriptorSetBindingPoints::PER_VIEW,
                     +PerViewBindingPoints::LIGHTS };
        case Ubo::ShadowUniforms:
            return { +DescriptorSetBindingPoints::PER_VIEW,
                     +PerViewBindingPoints::SHADOWS };
        case Ubo::FroxelRecordUniforms:
            return { +DescriptorSetBindingPoints::PER_VIEW,
                     +PerViewBindingPoints::RECORD_BUFFER };
        case Ubo::FroxelsUniforms:
            return { +DescriptorSetBindingPoints::PER_VIEW,
                     +PerViewBindingPoints::FROXEL_BUFFER };
        case Ubo::MaterialParams:
            return { +DescriptorSetBindingPoints::PER_MATERIAL,
                     +PerMaterialBindingPoints::MATERIAL_PARAMS };
    }
}

static_assert(CONFIG_MAX_SHADOW_CASCADES == 4,
        "Changing CONFIG_MAX_SHADOW_CASCADES affects PerView size and breaks materials.");

BufferInterfaceBlock const& UibGenerator::getPerViewUib() noexcept  {
    using Type = BufferInterfaceBlock::Type;

    static BufferInterfaceBlock const uib = BufferInterfaceBlock::Builder()
            .name(PerViewUib::_name)
            .add({
            { "viewFromWorldMatrix",    0, Type::MAT4,   Precision::HIGH, 0, FeatureLevel::FEATURE_LEVEL_0 },
            { "worldFromViewMatrix",    0, Type::MAT4,   Precision::HIGH, 0, FeatureLevel::FEATURE_LEVEL_0 },
            { "clipFromViewMatrix",     0, Type::MAT4,   Precision::HIGH, 0, FeatureLevel::FEATURE_LEVEL_0 },
            { "viewFromClipMatrix",     0, Type::MAT4,   Precision::HIGH, 0, FeatureLevel::FEATURE_LEVEL_0 },
            { "clipFromWorldMatrix",    CONFIG_MAX_STEREOSCOPIC_EYES,
                                           Type::MAT4,   Precision::HIGH, 0, FeatureLevel::FEATURE_LEVEL_0 },
            { "worldFromClipMatrix",    0, Type::MAT4,   Precision::HIGH, 0, FeatureLevel::FEATURE_LEVEL_0 },
            { "userWorldFromWorldMatrix",0,Type::MAT4,   Precision::HIGH, 0, FeatureLevel::FEATURE_LEVEL_0 },
            { "clipTransform",          0, Type::FLOAT4, Precision::HIGH, 0, FeatureLevel::FEATURE_LEVEL_0 },

            { "clipControl",            0, Type::FLOAT2, Precision::HIGH, 0, FeatureLevel::FEATURE_LEVEL_0 },
            { "time",                   0, Type::FLOAT,  Precision::HIGH, 0, FeatureLevel::FEATURE_LEVEL_0 },
            { "temporalNoise",          0, Type::FLOAT,  Precision::HIGH, 0, FeatureLevel::FEATURE_LEVEL_0 },
            { "userTime",               0, Type::FLOAT4, Precision::HIGH, 0, FeatureLevel::FEATURE_LEVEL_0 },

            // ------------------------------------------------------------------------------------
            // values below should only be accessed in surface materials
            // ------------------------------------------------------------------------------------

            { "resolution",             0, Type::FLOAT4, Precision::HIGH, 0, FeatureLevel::FEATURE_LEVEL_0 },
            { "logicalViewportScale",   0, Type::FLOAT2, Precision::HIGH, 0, FeatureLevel::FEATURE_LEVEL_0 },
            { "logicalViewportOffset",  0, Type::FLOAT2, Precision::HIGH, 0, FeatureLevel::FEATURE_LEVEL_0 },

            { "lodBias",                0, Type::FLOAT, Precision::DEFAULT, 0, FeatureLevel::FEATURE_LEVEL_0 },
            { "refractionLodOffset",    0, Type::FLOAT, Precision::DEFAULT, 0, FeatureLevel::FEATURE_LEVEL_0 },
            { "derivativesScale",       0, Type::FLOAT2                  },

            { "oneOverFarMinusNear",    0, Type::FLOAT,  Precision::HIGH, 0, FeatureLevel::FEATURE_LEVEL_0 },
            { "nearOverFarMinusNear",   0, Type::FLOAT,  Precision::HIGH, 0, FeatureLevel::FEATURE_LEVEL_0 },
            { "cameraFar",              0, Type::FLOAT,  Precision::HIGH, 0, FeatureLevel::FEATURE_LEVEL_0 },
            { "exposure",               0, Type::FLOAT,  Precision::HIGH, 0, FeatureLevel::FEATURE_LEVEL_0 }, // high precision to work around #3602 (qualcom),
            { "ev100",                  0, Type::FLOAT,  Precision::DEFAULT, 0, FeatureLevel::FEATURE_LEVEL_0 },
            { "needsAlphaChannel",      0, Type::FLOAT,  Precision::DEFAULT, 0, FeatureLevel::FEATURE_LEVEL_0 },

            // AO
            { "aoSamplingQualityAndEdgeDistance", 0, Type::FLOAT         },
            { "aoBentNormals",          0, Type::FLOAT                   },

            // ------------------------------------------------------------------------------------
            // Dynamic Lighting [variant: DYN]
            // ------------------------------------------------------------------------------------
            { "zParams",                0, Type::FLOAT4                  },
            { "fParams",                0, Type::UINT3                   },
            { "lightChannels",          0, Type::INT                     },
            { "froxelCountXY",          0, Type::FLOAT2                  },

            { "iblLuminance",           0, Type::FLOAT,  Precision::DEFAULT, 0, FeatureLevel::FEATURE_LEVEL_0 },
            { "iblRoughnessOneLevel",   0, Type::FLOAT,  Precision::DEFAULT, 0, FeatureLevel::FEATURE_LEVEL_0 },
            { "iblSH",                  9, Type::FLOAT3                  },

            // ------------------------------------------------------------------------------------
            // Directional Lighting [variant: DIR]
            // ------------------------------------------------------------------------------------
            { "lightDirection",         0, Type::FLOAT3, Precision::HIGH, 0, FeatureLevel::FEATURE_LEVEL_0 },
            { "padding0",               0, Type::FLOAT                   },
            { "lightColorIntensity",    0, Type::FLOAT4, Precision::DEFAULT, 0, FeatureLevel::FEATURE_LEVEL_0 },
            { "sun",                    0, Type::FLOAT4, Precision::DEFAULT, 0, FeatureLevel::FEATURE_LEVEL_0 },
            { "shadowFarAttenuationParams", 0, Type::FLOAT2, Precision::HIGH },

            // ------------------------------------------------------------------------------------
            // Directional light shadowing [variant: SRE | DIR]
            // ------------------------------------------------------------------------------------
            { "directionalShadows",       0, Type::INT                      },
            { "ssContactShadowDistance",  0, Type::FLOAT                    },

            { "cascadeSplits",             0, Type::FLOAT4, Precision::HIGH },
            { "cascades",                  0, Type::INT                     },
            { "shadowPenumbraRatioScale",  0, Type::FLOAT                   },
            { "lightFarAttenuationParams", 0, Type::FLOAT2, Precision::HIGH },

            // ------------------------------------------------------------------------------------
            // VSM shadows [variant: VSM]
            // ------------------------------------------------------------------------------------
            { "vsmExponent",             0, Type::FLOAT                  },
            { "vsmDepthScale",           0, Type::FLOAT                  },
            { "vsmLightBleedReduction",  0, Type::FLOAT                  },
            { "shadowSamplingType",      0, Type::UINT                   },

            // ------------------------------------------------------------------------------------
            // Fog [variant: FOG]
            // ------------------------------------------------------------------------------------
            { "fogDensity",              0, Type::FLOAT3,Precision::HIGH, 0, FeatureLevel::FEATURE_LEVEL_0 },
            { "fogStart",                0, Type::FLOAT, Precision::HIGH, 0, FeatureLevel::FEATURE_LEVEL_0 },
            { "fogMaxOpacity",           0, Type::FLOAT, Precision::DEFAULT, 0, FeatureLevel::FEATURE_LEVEL_0 },
            { "fogMinMaxMip",            0, Type::UINT,  Precision::HIGH },
            { "fogHeightFalloff",        0, Type::FLOAT, Precision::HIGH, 0, FeatureLevel::FEATURE_LEVEL_0 },
            { "fogCutOffDistance",       0, Type::FLOAT, Precision::HIGH, 0, FeatureLevel::FEATURE_LEVEL_0 },
            { "fogColor",                0, Type::FLOAT3, Precision::DEFAULT, 0, FeatureLevel::FEATURE_LEVEL_0 },
            { "fogColorFromIbl",         0, Type::FLOAT, Precision::DEFAULT, 0, FeatureLevel::FEATURE_LEVEL_0 },
            { "fogInscatteringStart",    0, Type::FLOAT, Precision::HIGH, 0, FeatureLevel::FEATURE_LEVEL_0 },
            { "fogInscatteringSize",     0, Type::FLOAT, Precision::DEFAULT, 0, FeatureLevel::FEATURE_LEVEL_0 },
            { "fogOneOverFarMinusNear",  0, Type::FLOAT, Precision::HIGH },
            { "fogNearOverFarMinusNear", 0, Type::FLOAT, Precision::HIGH },
            { "fogFromWorldMatrix",      0, Type::MAT3, Precision::HIGH, 0, FeatureLevel::FEATURE_LEVEL_0 },

            // ------------------------------------------------------------------------------------
            // Screen-space reflections [variant: SSR (i.e.: VSM | SRE)]
            // ------------------------------------------------------------------------------------
            { "ssrReprojection",         0, Type::MAT4,  Precision::HIGH },
            { "ssrUvFromViewMatrix",     0, Type::MAT4,  Precision::HIGH },
            { "ssrThickness",            0, Type::FLOAT                  },
            { "ssrBias",                 0, Type::FLOAT                  },
            { "ssrDistance",             0, Type::FLOAT                  },
            { "ssrStride",               0, Type::FLOAT                  },

            // --------------------------------------------------------------------------------------------
            // user defined global variables
            // --------------------------------------------------------------------------------------------
            { "custom",                  4, Type::FLOAT4, Precision::HIGH, 0, FeatureLevel::FEATURE_LEVEL_0 },

            // --------------------------------------------------------------------------------------------
            // for feature level 0 / es2 usage
            // --------------------------------------------------------------------------------------------
            { "rec709",                  0, Type::INT,  Precision::DEFAULT, 0, FeatureLevel::FEATURE_LEVEL_0 },
            { "es2Reserved0",            0, Type::FLOAT                  },
            { "es2Reserved1",            0, Type::FLOAT                  },
            { "es2Reserved2",            0, Type::FLOAT                  },

            // bring PerViewUib to 2 KiB
            { "reserved", sizeof(PerViewUib::reserved)/16, Type::FLOAT4 }
            })
            .build();

    return uib;
}

BufferInterfaceBlock const& UibGenerator::getPerRenderableUib() noexcept {
    static BufferInterfaceBlock const uib =  BufferInterfaceBlock::Builder()
            .name(PerRenderableUib::_name)
            .add({{ "data", CONFIG_MAX_INSTANCES, BufferInterfaceBlock::Type::STRUCT, {}, 0, {},
                    "PerRenderableData", sizeof(PerRenderableData), "CONFIG_MAX_INSTANCES" }})
            .build();
    return uib;
}

BufferInterfaceBlock const& UibGenerator::getLightsUib() noexcept {
    static BufferInterfaceBlock const uib = BufferInterfaceBlock::Builder()
            .name(LightsUib::_name)
            .add({{ "lights", CONFIG_MAX_LIGHT_COUNT,
                    BufferInterfaceBlock::Type::MAT4, Precision::HIGH }})
            .build();
    return uib;
}

BufferInterfaceBlock const& UibGenerator::getShadowUib() noexcept {
    static BufferInterfaceBlock const uib = BufferInterfaceBlock::Builder()
            .name(ShadowUib::_name)
            .add({{ "shadows", CONFIG_MAX_SHADOWMAPS,
                    BufferInterfaceBlock::Type::STRUCT, {}, 0, {},
                    "ShadowData", sizeof(ShadowUib::ShadowData) }})
            .build();
    return uib;
}

BufferInterfaceBlock const& UibGenerator::getPerRenderableBonesUib() noexcept {
    static BufferInterfaceBlock const uib = BufferInterfaceBlock::Builder()
            .name(PerRenderableBoneUib::_name)
            .add({{ "bones", CONFIG_MAX_BONE_COUNT,
                    BufferInterfaceBlock::Type::STRUCT, {}, 0, {},
                    "BoneData", sizeof(PerRenderableBoneUib::BoneData) }})
            .build();
    return uib;
}

BufferInterfaceBlock const& UibGenerator::getPerRenderableMorphingUib() noexcept {
    static BufferInterfaceBlock const uib = BufferInterfaceBlock::Builder()
            .name(PerRenderableMorphingUib::_name)
            .add({{ "weights", CONFIG_MAX_MORPH_TARGET_COUNT,
                    BufferInterfaceBlock::Type::FLOAT4 }})
            .build();
    return uib;
}

BufferInterfaceBlock const& UibGenerator::getFroxelRecordUib() noexcept {
    static BufferInterfaceBlock const uib = BufferInterfaceBlock::Builder()
            .name(FroxelRecordUib::_name)
            .add({{ "records", 1024, BufferInterfaceBlock::Type::UINT4, Precision::HIGH }})
            .build();
    return uib;
}

BufferInterfaceBlock const& UibGenerator::getFroxelsUib() noexcept {
    static BufferInterfaceBlock const uib = BufferInterfaceBlock::Builder()
            .name(FroxelsUib::_name)
            .add({{ "records", 1024, BufferInterfaceBlock::Type::UINT4, Precision::HIGH, 0, {},
                    {}, {}, "CONFIG_FROXEL_BUFFER_HEIGHT"}})
            .build();
    return uib;
}

} // namespace filament
