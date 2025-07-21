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

#include "SibGenerator.h"

#include "private/filament/Variant.h"
#include "private/filament/EngineEnums.h"
#include "private/filament/SamplerInterfaceBlock.h"
#include "private/filament/SibStructs.h"

#include <backend/DriverEnums.h>

#include <utils/debug.h>

namespace filament {

namespace {
constexpr bool FILTERABLE = true;
constexpr bool MULTISAMPLE = true;
constexpr backend::ShaderStageFlags ALL_STAGES = backend::ShaderStageFlags::ALL_SHADER_STAGE_FLAGS;
} // namespace

SamplerInterfaceBlock const& SibGenerator::getPerViewSib(Variant variant) noexcept {
    using Type = SamplerInterfaceBlock::Type;
    using Format = SamplerInterfaceBlock::Format;
    using Precision = SamplerInterfaceBlock::Precision;

    // What is happening here is that depending on the variant, some samplers' type or format
    // can change (e.g.: when VSM is used the shadowmap sampler is a regular float sampler),
    // so we return a different SamplerInterfaceBlock based on the variant.
    //
    // The samplers' name and binding (i.e. ordering) must match in all SamplerInterfaceBlocks
    // because this information is stored per-material and not per-shader.
    //
    // For the SSR (reflections) SamplerInterfaceBlock, only two samplers are ever used, for this
    // reason we name them "unused*" to ensure we're not using them by mistake (type/format don't
    // matter).

    static SamplerInterfaceBlock const sibPcf{ SamplerInterfaceBlock::Builder()
            .name("sampler0")
            .stageFlags(backend::ShaderStageFlags::FRAGMENT)
            .add(  {{ "shadowMap",   +PerViewBindingPoints::SHADOW_MAP,     Type::SAMPLER_2D_ARRAY, Format::SHADOW, Precision::MEDIUM, FILTERABLE, !MULTISAMPLE, ALL_STAGES },
                    { "iblDFG",      +PerViewBindingPoints::IBL_DFG_LUT,    Type::SAMPLER_2D,       Format::FLOAT,  Precision::MEDIUM, FILTERABLE, !MULTISAMPLE, ALL_STAGES },
                    { "iblSpecular", +PerViewBindingPoints::IBL_SPECULAR,   Type::SAMPLER_CUBEMAP,  Format::FLOAT,  Precision::MEDIUM, FILTERABLE, !MULTISAMPLE, ALL_STAGES },
                    { "ssao",        +PerViewBindingPoints::SSAO,           Type::SAMPLER_2D_ARRAY, Format::FLOAT,  Precision::MEDIUM, FILTERABLE, !MULTISAMPLE, ALL_STAGES },
                    { "ssr",         +PerViewBindingPoints::SSR,            Type::SAMPLER_2D_ARRAY, Format::FLOAT,  Precision::MEDIUM, FILTERABLE, !MULTISAMPLE, ALL_STAGES },
                    { "structure",   +PerViewBindingPoints::STRUCTURE,      Type::SAMPLER_2D,       Format::FLOAT,  Precision::HIGH  , FILTERABLE, !MULTISAMPLE, ALL_STAGES },
                    { "fog",         +PerViewBindingPoints::FOG,            Type::SAMPLER_CUBEMAP,  Format::FLOAT,  Precision::MEDIUM, FILTERABLE, !MULTISAMPLE, ALL_STAGES }}
            )
            .build() };

    static SamplerInterfaceBlock const sibVsm{ SamplerInterfaceBlock::Builder()
            .name("sampler0")
            .stageFlags(backend::ShaderStageFlags::FRAGMENT)
            .add(  {{ "shadowMap",   +PerViewBindingPoints::SHADOW_MAP,     Type::SAMPLER_2D_ARRAY, Format::FLOAT,  Precision::HIGH,   FILTERABLE, !MULTISAMPLE, ALL_STAGES },
                    { "iblDFG",      +PerViewBindingPoints::IBL_DFG_LUT,    Type::SAMPLER_2D,       Format::FLOAT,  Precision::MEDIUM, FILTERABLE, !MULTISAMPLE, ALL_STAGES },
                    { "iblSpecular", +PerViewBindingPoints::IBL_SPECULAR,   Type::SAMPLER_CUBEMAP,  Format::FLOAT,  Precision::MEDIUM, FILTERABLE, !MULTISAMPLE, ALL_STAGES },
                    { "ssao",        +PerViewBindingPoints::SSAO,           Type::SAMPLER_2D_ARRAY, Format::FLOAT,  Precision::MEDIUM, FILTERABLE, !MULTISAMPLE, ALL_STAGES },
                    { "ssr",         +PerViewBindingPoints::SSR,            Type::SAMPLER_2D_ARRAY, Format::FLOAT,  Precision::MEDIUM, FILTERABLE, !MULTISAMPLE, ALL_STAGES },
                    { "structure",   +PerViewBindingPoints::STRUCTURE,      Type::SAMPLER_2D,       Format::FLOAT,  Precision::HIGH  , FILTERABLE, !MULTISAMPLE, ALL_STAGES },
                    { "fog",         +PerViewBindingPoints::FOG,            Type::SAMPLER_CUBEMAP,  Format::FLOAT,  Precision::MEDIUM, FILTERABLE, !MULTISAMPLE, ALL_STAGES }}
            )
            .build() };

    static SamplerInterfaceBlock const sibSsr{ SamplerInterfaceBlock::Builder()
            .name("sampler0")
            .stageFlags(backend::ShaderStageFlags::FRAGMENT)
            .add(  {{ "ssr",         +PerViewBindingPoints::SSR,            Type::SAMPLER_2D,       Format::FLOAT,  Precision::MEDIUM, FILTERABLE, !MULTISAMPLE, ALL_STAGES },
                    { "structure",   +PerViewBindingPoints::STRUCTURE,      Type::SAMPLER_2D,       Format::FLOAT,  Precision::HIGH,   FILTERABLE, !MULTISAMPLE, ALL_STAGES }}
            )
            .build() };

    if (Variant::isSSRVariant(variant)) {
        return sibSsr;
    } else if (Variant::isVSMVariant(variant)) {
        return sibVsm;
    } else {
        return sibPcf;
    }
}

SamplerInterfaceBlock const& SibGenerator::getPerRenderableSib(Variant) noexcept {
    using Type = SamplerInterfaceBlock::Type;
    using Format = SamplerInterfaceBlock::Format;
    using Precision = SamplerInterfaceBlock::Precision;

    static SamplerInterfaceBlock const sib = SamplerInterfaceBlock::Builder()
            .name("sampler1")
            .stageFlags(backend::ShaderStageFlags::VERTEX)
            .add({  {"positions",          +PerRenderableBindingPoints::MORPH_TARGET_POSITIONS,    Type::SAMPLER_2D_ARRAY, Format::FLOAT, Precision::HIGH, FILTERABLE,  !MULTISAMPLE, ALL_STAGES },
                    {"tangents",           +PerRenderableBindingPoints::MORPH_TARGET_TANGENTS,     Type::SAMPLER_2D_ARRAY, Format::INT,   Precision::HIGH, !FILTERABLE, !MULTISAMPLE, ALL_STAGES },
                    {"indicesAndWeights",  +PerRenderableBindingPoints::BONES_INDICES_AND_WEIGHTS, Type::SAMPLER_2D,       Format::FLOAT, Precision::HIGH, FILTERABLE,  !MULTISAMPLE, ALL_STAGES }}
            )
            .build();

    return sib;
}

SamplerInterfaceBlock const* SibGenerator::getSib(DescriptorSetBindingPoints set, Variant variant) noexcept {
    switch (set) {
        case DescriptorSetBindingPoints::PER_VIEW:
            return &getPerViewSib(variant);
        case DescriptorSetBindingPoints::PER_RENDERABLE:
            return &getPerRenderableSib(variant);
        default:
            return nullptr;
    }
}

} // namespace filament
