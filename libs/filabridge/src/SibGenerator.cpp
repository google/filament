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

SamplerInterfaceBlock const& SibGenerator::getPerViewSib(uint8_t variantKey) noexcept {
    using Type = SamplerInterfaceBlock::Type;
    using Format = SamplerInterfaceBlock::Format;
    using Precision = SamplerInterfaceBlock::Precision;

    auto buildSib = [] (bool hasVsm) {
        auto builder = SamplerInterfaceBlock::Builder();

        builder
            .name("Light");

        if (hasVsm) {
            builder.add("shadowMap", Type::SAMPLER_2D_ARRAY, Format::FLOAT,  Precision::HIGH);
        } else {
            builder.add("shadowMap", Type::SAMPLER_2D_ARRAY, Format::SHADOW, Precision::MEDIUM);
        }

        return builder
            .add("iblDFG",        Type::SAMPLER_2D,         Format::FLOAT,   Precision::MEDIUM)
            .add("iblSpecular",   Type::SAMPLER_CUBEMAP,    Format::FLOAT,   Precision::MEDIUM)
            .add("ssao",          Type::SAMPLER_2D,         Format::FLOAT,   Precision::MEDIUM)
            .add("ssr",           Type::SAMPLER_2D,         Format::FLOAT,   Precision::MEDIUM)
            .add("structure",     Type::SAMPLER_2D,         Format::FLOAT,   Precision::MEDIUM)
            .build();
    };

    // TODO: ideally we'd want these to be constexpr, these are compile time structures.

    static SamplerInterfaceBlock sibPcf = buildSib(false);
    static SamplerInterfaceBlock sibVsm = buildSib(true);

    // SamplerBindingMap relies the assumption that Sibs have the same names and offsets
    // regardless of variant.
    assert(sibPcf.getSize() == PerViewSib::SAMPLER_COUNT);
    assert(sibVsm.getSize() == PerViewSib::SAMPLER_COUNT);

    Variant v(variantKey);

    return v.hasVsm() ? sibVsm : sibPcf;
}

SamplerInterfaceBlock const* SibGenerator::getSib(uint8_t bindingPoint, uint8_t variantKey) noexcept {
    switch (bindingPoint) {
        case BindingPoints::PER_VIEW:
            return &getPerViewSib(variantKey);
        case BindingPoints::PER_RENDERABLE:
            return nullptr;
        case BindingPoints::LIGHTS:
            return nullptr;
        default:
            return nullptr;
    }
}

} // namespace filament
