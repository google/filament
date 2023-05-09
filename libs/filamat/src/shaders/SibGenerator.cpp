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

namespace filament {

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
    // matter). This will not affect SamplerBindingMap because it always uses the default variant,
    // and so the correct information will be stored in the material file.

    static SamplerInterfaceBlock const sibPcf{ SamplerInterfaceBlock::Builder()
            .name("Light")
            .stageFlags(backend::ShaderStageFlags::FRAGMENT)
            .add(  {{ "shadowMap",   Type::SAMPLER_2D_ARRAY, Format::SHADOW, Precision::MEDIUM },
                    { "iblDFG",      Type::SAMPLER_2D,       Format::FLOAT,  Precision::MEDIUM },
                    { "iblSpecular", Type::SAMPLER_CUBEMAP,  Format::FLOAT,  Precision::MEDIUM },
                    { "ssao",        Type::SAMPLER_2D_ARRAY, Format::FLOAT,  Precision::MEDIUM },
                    { "ssr",         Type::SAMPLER_2D_ARRAY, Format::FLOAT,  Precision::MEDIUM },
                    { "structure",   Type::SAMPLER_2D,       Format::FLOAT,  Precision::HIGH   },
                    { "fog",         Type::SAMPLER_CUBEMAP,  Format::FLOAT,  Precision::MEDIUM }}
            )
            .build() };

    static SamplerInterfaceBlock const sibVsm{ SamplerInterfaceBlock::Builder()
            .name("Light")
            .stageFlags(backend::ShaderStageFlags::FRAGMENT)
            .add(  {{ "shadowMap",   Type::SAMPLER_2D_ARRAY, Format::FLOAT,  Precision::HIGH   },
                    { "iblDFG",      Type::SAMPLER_2D,       Format::FLOAT,  Precision::MEDIUM },
                    { "iblSpecular", Type::SAMPLER_CUBEMAP,  Format::FLOAT,  Precision::MEDIUM },
                    { "ssao",        Type::SAMPLER_2D_ARRAY, Format::FLOAT,  Precision::MEDIUM },
                    { "ssr",         Type::SAMPLER_2D_ARRAY, Format::FLOAT,  Precision::MEDIUM },
                    { "structure",   Type::SAMPLER_2D,       Format::FLOAT,  Precision::HIGH   },
                    { "fog",         Type::SAMPLER_CUBEMAP,  Format::FLOAT,  Precision::MEDIUM }}
            )
            .build() };

    static SamplerInterfaceBlock const sibSsr{ SamplerInterfaceBlock::Builder()
            .name("Light")
            .stageFlags(backend::ShaderStageFlags::FRAGMENT)
            .add(  {{ "unused0" },
                    { "unused1" },
                    { "unused2" },
                    { "unused3" },
                    { "ssr",         Type::SAMPLER_2D,       Format::FLOAT,  Precision::MEDIUM },
                    { "structure",   Type::SAMPLER_2D,       Format::FLOAT,  Precision::HIGH   },
                    { "unused5" }}
            )
            .build() };

    assert_invariant(sibPcf.getSize() == PerViewSib::SAMPLER_COUNT);
    assert_invariant(sibVsm.getSize() == PerViewSib::SAMPLER_COUNT);
    assert_invariant(sibSsr.getSize() == PerViewSib::SAMPLER_COUNT);

    if (Variant::isSSRVariant(variant)) {
        return sibSsr;
    } else if (Variant::isVSMVariant(variant)) {
        return sibVsm;
    } else {
        return sibPcf;
    }
}

SamplerInterfaceBlock const& SibGenerator::getPerRenderPrimitiveMorphingSib(Variant) noexcept {
    using Type = SamplerInterfaceBlock::Type;
    using Format = SamplerInterfaceBlock::Format;
    using Precision = SamplerInterfaceBlock::Precision;

    static SamplerInterfaceBlock const sib = SamplerInterfaceBlock::Builder()
            .name("MorphTargetBuffer")
            .stageFlags(backend::ShaderStageFlags::VERTEX)
            .add({  { "positions", Type::SAMPLER_2D_ARRAY, Format::FLOAT, Precision::HIGH },
                    { "tangents",  Type::SAMPLER_2D_ARRAY, Format::INT,   Precision::HIGH }})
            .build();

    return sib;
}

SamplerInterfaceBlock const& SibGenerator::getPerRenderPrimitiveBonesSib(Variant variant) noexcept {
    using Type = SamplerInterfaceBlock::Type;
    using Format = SamplerInterfaceBlock::Format;
    using Precision = SamplerInterfaceBlock::Precision;

    static SamplerInterfaceBlock sib = SamplerInterfaceBlock::Builder()
            .name("BonesBuffer")
            .stageFlags(backend::ShaderStageFlags::VERTEX)
            .add({{"indicesAndWeights", Type::SAMPLER_2D, Format::FLOAT, Precision::HIGH }})
            .build();

    return sib;
}

SamplerInterfaceBlock const* SibGenerator::getSib(SamplerBindingPoints bindingPoint, Variant variant) noexcept {
    switch (bindingPoint) {
        case SamplerBindingPoints::PER_VIEW:
            return &getPerViewSib(variant);
        case SamplerBindingPoints::PER_RENDERABLE_MORPHING:
            return &getPerRenderPrimitiveMorphingSib(variant);
        case SamplerBindingPoints::PER_RENDERABLE_SKINNING:
            return &getPerRenderPrimitiveBonesSib(variant);
        default:
            return nullptr;
    }
}

} // namespace filament
