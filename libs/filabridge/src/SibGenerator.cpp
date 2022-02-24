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

#include "private/filament/SibGenerator.h"

#include "private/filament/Variant.h"

#include <backend/DriverEnums.h>

#include <private/filament/SamplerInterfaceBlock.h>

namespace filament {

SamplerInterfaceBlock const& SibGenerator::getPerViewSib(Variant variant) noexcept {
    using Type = SamplerInterfaceBlock::Type;
    using Format = SamplerInterfaceBlock::Format;
    using Precision = SamplerInterfaceBlock::Precision;

    auto buildSib = [] (bool hasVsm) {
        auto builder = SamplerInterfaceBlock::Builder();

        builder
            .name("Light")
            .stageFlags({ .fragment = true });

        if (hasVsm) {
            builder.add("shadowMap", Type::SAMPLER_2D_ARRAY, Format::FLOAT,  Precision::HIGH);
        } else {
            builder.add("shadowMap", Type::SAMPLER_2D_ARRAY, Format::SHADOW, Precision::MEDIUM);
        }

        return builder
            .add("froxels",       Type::SAMPLER_2D,         Format::UINT,    Precision::MEDIUM)
            .add("iblDFG",        Type::SAMPLER_2D,         Format::FLOAT,   Precision::MEDIUM)
            .add("iblSpecular",   Type::SAMPLER_CUBEMAP,    Format::FLOAT,   Precision::MEDIUM)
            .add("ssao",          Type::SAMPLER_2D_ARRAY,   Format::FLOAT,   Precision::MEDIUM)
            .add("ssr",           Type::SAMPLER_2D,         Format::FLOAT,   Precision::MEDIUM)
            .add("structure",     Type::SAMPLER_2D,         Format::FLOAT,   Precision::HIGH)
            .build();
    };

    // TODO: ideally we'd want these to be constexpr, these are compile time structures.

    static SamplerInterfaceBlock sibPcf = buildSib(false);
    static SamplerInterfaceBlock sibVsm = buildSib(true);

    // SamplerBindingMap relies the assumption that Sibs have the same names and offsets
    // regardless of variant.
    assert(sibPcf.getSize() == PerViewSib::SAMPLER_COUNT);
    assert(sibVsm.getSize() == PerViewSib::SAMPLER_COUNT);

    return Variant::isVSMVariant(variant) ? sibVsm : sibPcf;
}

SamplerInterfaceBlock const& SibGenerator::getPerRenderPrimitiveMorphingSib(Variant variant) noexcept {
    using Type = SamplerInterfaceBlock::Type;
    using Format = SamplerInterfaceBlock::Format;
    using Precision = SamplerInterfaceBlock::Precision;

    static SamplerInterfaceBlock sib = SamplerInterfaceBlock::Builder()
            .name("MorphTargetBuffer")
            .stageFlags({ .vertex = true })
            .add("positions", Type::SAMPLER_2D_ARRAY, Format::FLOAT, Precision::HIGH)
            .add("tangents",  Type::SAMPLER_2D_ARRAY, Format::INT,   Precision::HIGH)
            .build();

    return sib;
}

SamplerInterfaceBlock const* SibGenerator::getSib(uint8_t bindingPoint, Variant variant) noexcept {
    switch (bindingPoint) {
        case BindingPoints::PER_VIEW:
            return &getPerViewSib(variant);
        case BindingPoints::PER_RENDERABLE:
            return nullptr;
        case BindingPoints::PER_RENDERABLE_MORPHING:
            return &getPerRenderPrimitiveMorphingSib(variant);
        case BindingPoints::LIGHTS:
            return nullptr;
        default:
            return nullptr;
    }
}

} // namespace filament
