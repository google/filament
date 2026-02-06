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

MaterialPrograms::MaterialPrograms(MaterialPrograms const& other)
        : mMaterial(other.mMaterial),
          mCachedPrograms(other.mCachedPrograms.size()),
          mSpecializationConstants((other.mMaterial != nullptr)
                                           ? other.mMaterial->getEngine()
                                                     .getMaterialCache()
                                                     .getSpecializationConstantsInternPool()
                                                     .acquire(other.mSpecializationConstants)
                                           : SpecializationConstants()) {}

MaterialPrograms& MaterialPrograms::operator=(MaterialPrograms const& other) {
    assert_invariant(mMaterial == nullptr);
    assert_invariant(mCachedPrograms.empty());
    assert_invariant(mSpecializationConstants.empty());

    mMaterial = other.mMaterial;
    if (mMaterial != nullptr) {
        mCachedPrograms = FixedCapacityVector<Handle<HwProgram>>(other.mCachedPrograms.size());
        mSpecializationConstants = other.mMaterial->getEngine()
                .getMaterialCache()
                .getSpecializationConstantsInternPool()
                .acquire(other.mSpecializationConstants);
    }

    return *this;
}

void MaterialPrograms::initializeForMaterial(FEngine& engine, FMaterial const& material,
        utils::FixedCapacityVector<backend::Program::SpecializationConstant>
                specializationConstants) {
    assert_invariant(mMaterial == nullptr);
    assert_invariant(mCachedPrograms.empty());
    assert_invariant(mSpecializationConstants.empty());

    mMaterial = &material;

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
    mCachedPrograms = FixedCapacityVector<Handle<HwProgram>>(cachedProgramsSize);

    material.getDefinition().acquirePrograms(engine, mCachedPrograms.as_slice(),
            material.getMaterialParser(), mSpecializationConstants, material.isDefaultMaterial());
}

void MaterialPrograms::initializeForMaterialInstance(FEngine& engine, FMaterial const& material) {
    assert_invariant(mMaterial == nullptr);
    assert_invariant(mCachedPrograms.empty());
    assert_invariant(mSpecializationConstants.empty());

    mMaterial = &material;
    MaterialPrograms const& programs = material.getPrograms();

    mSpecializationConstants =
            engine.getMaterialCache().getSpecializationConstantsInternPool().acquire(
                    programs.getSpecializationConstants());

    mCachedPrograms =
            FixedCapacityVector<Handle<HwProgram>>(material.getPrograms().mCachedPrograms.size());
}

Handle<HwProgram> MaterialPrograms::prepareProgramSlow(DriverApi& driver, Variant const variant,
        CompilerPriorityQueue const priorityQueue) const noexcept {
    assert_invariant(mMaterial != nullptr);

    FEngine& engine = mMaterial->getEngine();
    if (mMaterial->isSharedVariant(variant)) {
        FMaterial const* defaultMaterial = engine.getDefaultMaterial();
        assert_invariant(defaultMaterial);
        MaterialPrograms const& defaultPrograms = defaultMaterial->getPrograms();
        Handle<HwProgram> program = defaultPrograms.mCachedPrograms[variant.key];
        if (program) {
            return mCachedPrograms[variant.key] = program;
        }
        return mCachedPrograms[variant.key] =
                defaultPrograms.prepareProgram(driver, variant, priorityQueue);
    }
    return mCachedPrograms[variant.key] = mMaterial->getDefinition().prepareProgram(engine, driver,
                   mMaterial->getMaterialParser(), getProgramSpecialization(variant), priorityQueue);
}

ProgramSpecialization MaterialPrograms::getProgramSpecialization(Variant variant) const noexcept {
    assert_invariant(mMaterial != nullptr);

    return ProgramSpecialization {
        .materialCrc32 = mMaterial->getMaterialParser().getCrc32(),
        .variant = variant,
        .specializationConstants = mSpecializationConstants,
    };
}

void MaterialPrograms::terminate(FEngine& engine) {
    assert_invariant(mMaterial != nullptr);

    mMaterial->getDefinition().releasePrograms(engine, mCachedPrograms.as_slice(),
            mMaterial->getMaterialParser(), mSpecializationConstants,
            mMaterial->isDefaultMaterial());
    engine.getMaterialCache().releaseMaterial(engine, mMaterial->getDefinition());
    engine.getMaterialCache().getSpecializationConstantsInternPool().release(
            mSpecializationConstants);
}

void MaterialPrograms::clear(FEngine& engine) {
    assert_invariant(mMaterial != nullptr);

    mMaterial->getDefinition().releasePrograms(engine, mCachedPrograms.as_slice(),
            mMaterial->getMaterialParser(), mSpecializationConstants,
            mMaterial->isDefaultMaterial());
}

Variant MaterialPrograms::filterVariantForGetProgram(Variant variant) const noexcept {
    if (UTILS_UNLIKELY(mMaterial->getEngine().features.material.enable_fog_as_postprocess)) {
        // if the fog as post-process feature is enabled, we need to proceed "as-if" the material
        // didn't have the FOG variant bit.
        if (mMaterial->getMaterialDomain() == MaterialDomain::SURFACE) {
            BlendingMode const blendingMode = mMaterial->getBlendingMode();
            bool const hasScreenSpaceRefraction =
                    mMaterial->getRefractionMode() == RefractionMode::SCREEN_SPACE;
            bool const isBlendingCommand = !hasScreenSpaceRefraction &&
                    (blendingMode != BlendingMode::OPAQUE && blendingMode != BlendingMode::MASKED);
            if (!isBlendingCommand) {
                variant.setFog(false);
            }
        }
    }
    return variant;
}

Program::SpecializationConstant MaterialPrograms::getConstantImpl(uint32_t id) const noexcept {
    return mSpecializationConstants[id];
}

Program::SpecializationConstant MaterialPrograms::getConstantImpl(
        std::string_view name) const noexcept {
    assert_invariant(mMaterial != nullptr);

    MaterialDefinition const& definition = mMaterial->getDefinition();
    auto it = definition.specializationConstantsNameToIndex.find(name);
    if (it != definition.specializationConstantsNameToIndex.cend()) {
        return getConstantImpl(it->second + CONFIG_MAX_RESERVED_SPEC_CONSTANTS);
    }

    std::string name_cstring(name);
    PANIC_PRECONDITION("No such constant exists: %s", name_cstring.c_str());
    return {};
}

void MaterialPrograms::setConstants(
        std::initializer_list<std::pair<uint32_t, Program::SpecializationConstant>>
                constants) noexcept {
    assert_invariant(mMaterial != nullptr);

    auto newSpecializationConstants =
            FixedCapacityVector<Program::SpecializationConstant>(mSpecializationConstants);

    bool hasChanged = false;
    for (const auto& [id, value] : constants) {
        if (newSpecializationConstants[id] != value) {
            newSpecializationConstants[id] = value;
            hasChanged = true;
        }
    }

    if (hasChanged) {
        setConstantsImpl(std::move(newSpecializationConstants));
    }
}

void MaterialPrograms::setConstants(
        std::initializer_list<std::pair<std::string_view, Program::SpecializationConstant>>
                constants) noexcept {
    assert_invariant(mMaterial != nullptr);

    auto newSpecializationConstants =
            FixedCapacityVector<Program::SpecializationConstant>(mSpecializationConstants);

    bool hasChanged = false;
    for (const auto& [name, value] : constants) {
        MaterialDefinition const& definition = mMaterial->getDefinition();
        auto it = definition.specializationConstantsNameToIndex.find(name);
        if (it != definition.specializationConstantsNameToIndex.cend()) {
            uint32_t id = it->second + CONFIG_MAX_RESERVED_SPEC_CONSTANTS;
            if (newSpecializationConstants[id] != value) {
                newSpecializationConstants[id] = value;
                hasChanged = true;
            }
        }
    }

    if (hasChanged) {
        setConstantsImpl(std::move(newSpecializationConstants));
    }
}

void MaterialPrograms::setConstantsImpl(
        FixedCapacityVector<Program::SpecializationConstant> constants) noexcept {
    FEngine& engine = mMaterial->getEngine();

    auto& internPool = engine.getMaterialCache().getSpecializationConstantsInternPool();
    MaterialParser const& materialParser = mMaterial->getMaterialParser();
    MaterialDefinition const& definition = mMaterial->getDefinition();
    const bool isDefaultMaterial = mMaterial->isDefaultMaterial();

    // Release old resources...
    definition.releasePrograms(engine, mCachedPrograms.as_slice(), materialParser,
            mSpecializationConstants, isDefaultMaterial);
    internPool.release(mSpecializationConstants);

    // Then acquire new ones.
    mSpecializationConstants = internPool.acquire(std::move(constants));
    definition.acquirePrograms(engine, mCachedPrograms.as_slice(), materialParser,
            mSpecializationConstants, isDefaultMaterial);
}

template int32_t MaterialPrograms::getConstant<int32_t>(uint32_t id) const noexcept;
template float MaterialPrograms::getConstant<float>(uint32_t id) const noexcept;
template bool MaterialPrograms::getConstant<bool>(uint32_t id) const noexcept;

template int32_t MaterialPrograms::getConstant<int32_t>(std::string_view name) const noexcept;
template float MaterialPrograms::getConstant<float>(std::string_view name) const noexcept;
template bool MaterialPrograms::getConstant<bool>(std::string_view name) const noexcept;

} // namespace filament
