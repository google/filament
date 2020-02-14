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

#include <backend/DriverEnums.h>

#include <private/filament/SamplerInterfaceBlock.h>

namespace filament {

SamplerInterfaceBlock const& SibGenerator::getPerViewSib() noexcept {
    using Type = SamplerInterfaceBlock::Type;
    using Format = SamplerInterfaceBlock::Format;
    using Precision = SamplerInterfaceBlock::Precision;

    // TODO: ideally we'd want this to be constexpr, this is a compile time structure
    static SamplerInterfaceBlock sib = SamplerInterfaceBlock::Builder()
            .name("Light")
            .add("shadowMap",     Type::SAMPLER_2D_ARRAY,   Format::SHADOW, Precision::LOW)
            .add("records",       Type::SAMPLER_2D,         Format::UINT,   Precision::MEDIUM)
            .add("froxels",       Type::SAMPLER_2D,         Format::UINT,   Precision::MEDIUM)
            .add("iblDFG",        Type::SAMPLER_2D,         Format::FLOAT,  Precision::MEDIUM)
            .add("iblSpecular",   Type::SAMPLER_CUBEMAP,    Format::FLOAT,  Precision::MEDIUM)
            .add("ssao",          Type::SAMPLER_2D,         Format::FLOAT,  Precision::MEDIUM)
            .add("ssr",           Type::SAMPLER_2D,         Format::FLOAT,  Precision::MEDIUM)
            .build();

    assert(sib.getSize() == PerViewSib::SAMPLER_COUNT);

    return sib;
}

SamplerInterfaceBlock const* SibGenerator::getSib(uint8_t bindingPoint) noexcept {
    switch (bindingPoint) {
        case BindingPoints::PER_VIEW:
            return &getPerViewSib();
        case BindingPoints::PER_RENDERABLE:
            return nullptr;
        case BindingPoints::LIGHTS:
            return nullptr;
        default:
            return nullptr;
    }
}

} // namespace filament
