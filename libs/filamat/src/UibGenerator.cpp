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
            .add({
            { "viewFromWorldMatrix",    0, UniformInterfaceBlock::Type::MAT4,   Precision::HIGH },
            { "worldFromViewMatrix",    0, UniformInterfaceBlock::Type::MAT4,   Precision::HIGH },
            { "clipFromViewMatrix",     0, UniformInterfaceBlock::Type::MAT4,   Precision::HIGH },
            { "viewFromClipMatrix",     0, UniformInterfaceBlock::Type::MAT4,   Precision::HIGH },
            { "clipFromWorldMatrix",    0, UniformInterfaceBlock::Type::MAT4,   Precision::HIGH },
            { "worldFromClipMatrix",    0, UniformInterfaceBlock::Type::MAT4,   Precision::HIGH },
            { "clipTransform",          0, UniformInterfaceBlock::Type::FLOAT4, Precision::HIGH },

            { "clipControl",            0, UniformInterfaceBlock::Type::FLOAT2                  },
            { "time",                   0, UniformInterfaceBlock::Type::FLOAT,  Precision::HIGH },
            { "temporalNoise",          0, UniformInterfaceBlock::Type::FLOAT,  Precision::HIGH },
            { "userTime",               0, UniformInterfaceBlock::Type::FLOAT4, Precision::HIGH },

            // ------------------------------------------------------------------------------------
            // values below should only be accessed in surface materials
            // ------------------------------------------------------------------------------------

            { "origin",                 0, UniformInterfaceBlock::Type::FLOAT2, Precision::HIGH },
            { "offset",                 0, UniformInterfaceBlock::Type::FLOAT2, Precision::HIGH },
            { "resolution",             0, UniformInterfaceBlock::Type::FLOAT4, Precision::HIGH },

            { "lodBias",                0, UniformInterfaceBlock::Type::FLOAT                   },
            { "refractionLodOffset",    0, UniformInterfaceBlock::Type::FLOAT                   },
            { "padding1",               0, UniformInterfaceBlock::Type::FLOAT                   },
            { "padding2",               0, UniformInterfaceBlock::Type::FLOAT                   },

            { "cameraPosition",         0, UniformInterfaceBlock::Type::FLOAT3, Precision::HIGH },
            { "oneOverFarMinusNear",    0, UniformInterfaceBlock::Type::FLOAT,  Precision::HIGH },
            { "worldOffset",            0, UniformInterfaceBlock::Type::FLOAT3                  },
            { "nearOverFarMinusNear",   0, UniformInterfaceBlock::Type::FLOAT,  Precision::HIGH },
            { "cameraFar",              0, UniformInterfaceBlock::Type::FLOAT                   },
            { "exposure",               0, UniformInterfaceBlock::Type::FLOAT,  Precision::HIGH }, // high precision to work around #3602 (qualcom),
            { "ev100",                  0, UniformInterfaceBlock::Type::FLOAT                   },
            { "needsAlphaChannel",      0, UniformInterfaceBlock::Type::FLOAT                   },

            // AO
            { "aoSamplingQualityAndEdgeDistance", 0, UniformInterfaceBlock::Type::FLOAT         },
            { "aoBentNormals",          0, UniformInterfaceBlock::Type::FLOAT                   },
            { "aoReserved0",            0, UniformInterfaceBlock::Type::FLOAT                   },
            { "aoReserved1",            0, UniformInterfaceBlock::Type::FLOAT                   },

            // ------------------------------------------------------------------------------------
            // Dynamic Lighting [variant: DYN]
            // ------------------------------------------------------------------------------------
            { "zParams",                0, UniformInterfaceBlock::Type::FLOAT4                  },
            { "fParams",                0, UniformInterfaceBlock::Type::UINT3                   },
            { "lightChannels",          0, UniformInterfaceBlock::Type::UINT                    },
            { "froxelCountXY",          0, UniformInterfaceBlock::Type::FLOAT2                  },

            { "iblLuminance",           0, UniformInterfaceBlock::Type::FLOAT                   },
            { "iblRoughnessOneLevel",   0, UniformInterfaceBlock::Type::FLOAT                   },
            { "iblSH",                  9, UniformInterfaceBlock::Type::FLOAT3                  },

            // ------------------------------------------------------------------------------------
            // Directional Lighting [variant: DIR]
            // ------------------------------------------------------------------------------------
            { "lightDirection",         0, UniformInterfaceBlock::Type::FLOAT3                  },
            { "padding0",               0, UniformInterfaceBlock::Type::FLOAT                   },
            { "lightColorIntensity",    0, UniformInterfaceBlock::Type::FLOAT4                  },
            { "sun",                    0, UniformInterfaceBlock::Type::FLOAT4                  },
            { "lightFarAttenuationParams", 0, UniformInterfaceBlock::Type::FLOAT2               },

            // ------------------------------------------------------------------------------------
            // Directional light shadowing [variant: SRE | DIR]
            // ------------------------------------------------------------------------------------
            { "directionalShadows",     0, UniformInterfaceBlock::Type::UINT                    },
            { "ssContactShadowDistance",0, UniformInterfaceBlock::Type::FLOAT                   },

            { "cascadeSplits",          0, UniformInterfaceBlock::Type::FLOAT4, Precision::HIGH },
            { "cascades",               0, UniformInterfaceBlock::Type::UINT                    },
            { "shadowBulbRadiusLs",     0, UniformInterfaceBlock::Type::FLOAT                   },
            { "shadowBias",             0, UniformInterfaceBlock::Type::FLOAT                   },
            { "shadowPenumbraRatioScale", 0, UniformInterfaceBlock::Type::FLOAT                 },
            { "lightFromWorldMatrix",   4, UniformInterfaceBlock::Type::MAT4, Precision::HIGH   },

            // ------------------------------------------------------------------------------------
            // VSM shadows [variant: VSM]
            // ------------------------------------------------------------------------------------
            { "vsmExponent",             0, UniformInterfaceBlock::Type::FLOAT                  },
            { "vsmDepthScale",           0, UniformInterfaceBlock::Type::FLOAT                  },
            { "vsmLightBleedReduction",  0, UniformInterfaceBlock::Type::FLOAT                  },
            { "shadowSamplingType",      0, UniformInterfaceBlock::Type::UINT                   },

            // ------------------------------------------------------------------------------------
            // Fog [variant: FOG]
            // ------------------------------------------------------------------------------------
            { "fogStart",                0, UniformInterfaceBlock::Type::FLOAT                  },
            { "fogMaxOpacity",           0, UniformInterfaceBlock::Type::FLOAT                  },
            { "fogHeight",               0, UniformInterfaceBlock::Type::FLOAT                  },
            { "fogHeightFalloff",        0, UniformInterfaceBlock::Type::FLOAT                  },
            { "fogColor",                0, UniformInterfaceBlock::Type::FLOAT3                 },
            { "fogDensity",              0, UniformInterfaceBlock::Type::FLOAT                  },
            { "fogInscatteringStart",    0, UniformInterfaceBlock::Type::FLOAT                  },
            { "fogInscatteringSize",     0, UniformInterfaceBlock::Type::FLOAT                  },
            { "fogColorFromIbl",         0, UniformInterfaceBlock::Type::FLOAT                  },
            { "fogReserved0",            0, UniformInterfaceBlock::Type::FLOAT                  },

            // ------------------------------------------------------------------------------------
            // Screen-space reflections [variant: SSR (i.e.: VSM | SRE)]
            // ------------------------------------------------------------------------------------
            { "ssrReprojection",         0, UniformInterfaceBlock::Type::MAT4, Precision::HIGH  },
            { "ssrUvFromViewMatrix",     0, UniformInterfaceBlock::Type::MAT4, Precision::HIGH  },
            { "ssrThickness",            0, UniformInterfaceBlock::Type::FLOAT                  },
            { "ssrBias",                 0, UniformInterfaceBlock::Type::FLOAT                  },
            { "ssrDistance",             0, UniformInterfaceBlock::Type::FLOAT                  },
            { "ssrStride",               0, UniformInterfaceBlock::Type::FLOAT                  },

            // bring PerViewUib to 2 KiB
            { "reserved", sizeof(PerViewUib::reserved)/16, UniformInterfaceBlock::Type::FLOAT4 }
            })
            .build();

    return uib;
}

UniformInterfaceBlock const& UibGenerator::getPerRenderableUib() noexcept {
    static UniformInterfaceBlock uib =  UniformInterfaceBlock::Builder()
            .name(PerRenderableUib::_name)
            .add({{ "data", CONFIG_MAX_INSTANCES, UniformInterfaceBlock::Type::STRUCT, {},
                    "PerRenderableData", sizeof(PerRenderableData), "CONFIG_MAX_INSTANCES" }})
            .build();
    return uib;
}

UniformInterfaceBlock const& UibGenerator::getLightsUib() noexcept {
    static UniformInterfaceBlock uib = UniformInterfaceBlock::Builder()
            .name(LightsUib::_name)
            .add({{ "lights", CONFIG_MAX_LIGHT_COUNT,
                    UniformInterfaceBlock::Type::MAT4, Precision::HIGH }})
            .build();
    return uib;
}

UniformInterfaceBlock const& UibGenerator::getShadowUib() noexcept {
    static UniformInterfaceBlock uib = UniformInterfaceBlock::Builder()
            .name(ShadowUib::_name)
            .add({{ "shadows", CONFIG_MAX_SHADOW_CASTING_SPOTS,
                    UniformInterfaceBlock::Type::STRUCT, {},
                    "ShadowData", sizeof(ShadowUib::ShadowData) }})
            .build();
    return uib;
}

UniformInterfaceBlock const& UibGenerator::getPerRenderableBonesUib() noexcept {
    static UniformInterfaceBlock uib = UniformInterfaceBlock::Builder()
            .name(PerRenderableBoneUib::_name)
            .add({{ "bones", CONFIG_MAX_BONE_COUNT,
                     UniformInterfaceBlock::Type::STRUCT, {},
                     "BoneData", sizeof(PerRenderableBoneUib::BoneData) }})
            .build();
    return uib;
}

UniformInterfaceBlock const& UibGenerator::getPerRenderableMorphingUib() noexcept {
    static UniformInterfaceBlock uib = UniformInterfaceBlock::Builder()
            .name(PerRenderableMorphingUib::_name)
            .add({{ "weights", CONFIG_MAX_MORPH_TARGET_COUNT,
                    UniformInterfaceBlock::Type::FLOAT4 }})
            .build();
    return uib;
}

UniformInterfaceBlock const& UibGenerator::getFroxelRecordUib() noexcept {
    static UniformInterfaceBlock uib = UniformInterfaceBlock::Builder()
            .name(FroxelRecordUib::_name)
            .add({{ "records", 1024, UniformInterfaceBlock::Type::UINT4, Precision::HIGH }})
            .build();
    return uib;
}

} // namespace filament
