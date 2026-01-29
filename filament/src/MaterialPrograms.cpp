/*
 * Copyright (C) 2026 The Android Open Source Project
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

#include "MaterialPrograms.h"
#include "MaterialParser.h"

#include "details/Engine.h"
#include "details/Material.h"

#include <backend/DriverApiForward.h>

namespace filament {

using namespace backend;
using namespace utils;

MaterialPrograms::MaterialPrograms(FEngine& engine, FMaterial const& material,
        utils::FixedCapacityVector<backend::Program::SpecializationConstant>
                specializationConstants)
        : mMaterial(material) {
    mSpecializationConstants =
            engine.getMaterialCache().getSpecializationConstantsInternPool().acquire(
                    std::move(specializationConstants));

    size_t cachedProgramsSize;
    switch (material.getMaterialDomain()) {
        case filament::MaterialDomain::SURFACE:
            cachedProgramsSize = 1 << VARIANT_BITS;
            break;
        case filament::MaterialDomain::POST_PROCESS:
            cachedProgramsSize = 1 << POST_PROCESS_VARIANT_BITS;
            break;
        case filament::MaterialDomain::COMPUTE:
            cachedProgramsSize = 1;
            break;
    }
    mCachedPrograms = FixedCapacityVector<backend::Handle<backend::HwProgram>>(cachedProgramsSize);

    material.getDefinition().acquirePrograms(engine, mCachedPrograms.as_slice(),
            material.getMaterialParser(), mSpecializationConstants, material.isDefaultMaterial());
}

Handle<HwProgram> MaterialPrograms::prepareProgramSlow(DriverApi& driver, Variant const variant,
        CompilerPriorityQueue const priorityQueue) const noexcept {
    FEngine& engine = mMaterial.getEngine();
    if (mMaterial.isSharedVariant(variant)) {
        FMaterial const* defaultMaterial = engine.getDefaultMaterial();
        FILAMENT_CHECK_PRECONDITION(defaultMaterial);
        MaterialPrograms const& defaultPrograms = defaultMaterial->getPrograms();
        Handle<HwProgram> program = defaultPrograms.mCachedPrograms[variant.key];
        if (program) {
            return mCachedPrograms[variant.key] = program;
        }
        return mCachedPrograms[variant.key] =
                defaultPrograms.prepareProgram(driver, variant, priorityQueue);
    }
    return mCachedPrograms[variant.key] = mMaterial.getDefinition().prepareProgram(engine, driver,
                   mMaterial.getMaterialParser(), getProgramSpecialization(variant), priorityQueue);
}

ProgramSpecialization MaterialPrograms::getProgramSpecialization(Variant variant) const noexcept {
    return ProgramSpecialization {
        .materialCrc32 = mMaterial.getMaterialParser().getCrc32(),
        .variant = variant,
        .specializationConstants = mSpecializationConstants,
    };
}

void MaterialPrograms::terminate(FEngine& engine) {
    mMaterial.getDefinition().releasePrograms(engine, mCachedPrograms.as_slice(),
            mMaterial.getMaterialParser(), mSpecializationConstants, mMaterial.isDefaultMaterial());
    engine.getMaterialCache().releaseMaterial(engine, mMaterial.getDefinition());
    engine.getMaterialCache().getSpecializationConstantsInternPool().release(
            mSpecializationConstants);
}

void MaterialPrograms::release(FEngine& engine) {
    mMaterial.getDefinition().releasePrograms(engine, mCachedPrograms.as_slice(),
            mMaterial.getMaterialParser(), mSpecializationConstants, mMaterial.isDefaultMaterial());
}

} // namespace filament
