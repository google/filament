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

#include "LocalProgramCache.h"

#include "MaterialParser.h"

#include "details/Engine.h"
#include "details/Material.h"

#include <backend/DriverApiForward.h>

#include <utils/Panic.h>

#include <cstdlib>
#include <limits>

namespace filament {

using namespace backend;
using namespace utils;

LocalProgramCache::LocalProgramCache(LocalProgramCache const& other)
        : mMaterial(other.mMaterial),
          mCachedPrograms(other.mCachedPrograms.size()),
          mSpecializationConstants(other.mSpecializationConstants.clone()) {}

LocalProgramCache& LocalProgramCache::operator=(LocalProgramCache const& other) {
    assert_invariant(mMaterial == nullptr);
    assert_invariant(mCachedPrograms.empty());
    assert_invariant(mSpecializationConstants.empty());

    mMaterial = other.mMaterial;
    mSpecializationConstants = other.mSpecializationConstants.clone();
    if (mMaterial != nullptr) {
        mCachedPrograms = FixedCapacityVector<Handle<HwProgram>>(other.mCachedPrograms.size());
    }

    return *this;
}

uint32_t LocalProgramCache::getCacheSize(MaterialDomain const materialDomain) {
    switch (materialDomain) {
        case MaterialDomain::SURFACE:
            return 1u << (VARIANT_BITS + DYNAMIC_SPEC_CONST_KEY_BITS);
        case MaterialDomain::POST_PROCESS:
            return 1u << (POST_PROCESS_VARIANT_BITS + DYNAMIC_SPEC_CONST_KEY_BITS);
        case MaterialDomain::COMPUTE:
            return 1u;
    }
    return 1u;
}

LocalProgramCache::CacheKey LocalProgramCache::mapCacheEntryKey(Variant const variant,
        DynamicSpecConstKey specKey, std::size_t const cacheSize) {
    // Decouple depth variants from the dynamic specialization space
    if (Variant::isValidDepthVariant(variant)) {
        specKey = DynamicSpecConstKey{ 0 };
    }
    constexpr CacheKey SPEC_KEY_MASK = (CacheKey{ 1 } << DYNAMIC_SPEC_CONST_KEY_BITS) - 1u;
    CacheKey const key = (CacheKey{ variant.key } << DYNAMIC_SPEC_CONST_KEY_BITS) |
                         (CacheKey{ specKey.key } & SPEC_KEY_MASK);
    FILAMENT_CHECK_POSTCONDITION(key < cacheSize)
            << "Program cache index out of bounds: variant="
            << static_cast<uint32_t>(variant.key)
            << ", specKey=" << static_cast<uint32_t>(specKey.key)
            << ", index=" << key << ", size=" << cacheSize;
    return key;
}

void LocalProgramCache::initializeForMaterial(FEngine& engine, FMaterial const& material,
        utils::FixedCapacityVector<backend::Program::SpecializationConstant>
                specializationConstants) {
    assert_invariant(mMaterial == nullptr);
    assert_invariant(mCachedPrograms.empty());
    assert_invariant(mSpecializationConstants.empty());

    mMaterial = &material;

    mSpecializationConstants =
            engine.getMaterialCache().getSpecializationConstantsInternPool().acquire(
                    std::move(specializationConstants));

    mCachedPrograms =
            FixedCapacityVector<Handle<HwProgram>>(getCacheSize(material.getMaterialDomain()));

    material.getDefinition().acquirePrograms(engine, mCachedPrograms.as_slice(),
            material.getMaterialParser(), mSpecializationConstants.get(),
            material.isDefaultMaterial());
}

void LocalProgramCache::initializeForMaterialInstance(FEngine& engine, FMaterial const& material) {
    assert_invariant(mMaterial == nullptr);
    assert_invariant(mCachedPrograms.empty());
    assert_invariant(mSpecializationConstants.empty());

    mMaterial = &material;
    LocalProgramCache const& programs = material.getPrograms();

    mSpecializationConstants =
            engine.getMaterialCache().getSpecializationConstantsInternPool().acquire(
                    programs.getSpecializationConstants());

    mCachedPrograms =
            FixedCapacityVector<Handle<HwProgram>>(getCacheSize(material.getMaterialDomain()));
}

Handle<HwProgram> LocalProgramCache::prepareProgramSlow(DriverApi& driver, Variant const variant,
        DynamicSpecConstKey specKey, CompilerPriorityQueue const priorityQueue) const noexcept {
    assert_invariant(mMaterial != nullptr);

    FEngine& engine = mMaterial->getEngine();

    Handle<HwProgram> result;
    CacheKey const mappedKey = mapCacheEntryKey(variant, specKey, mCachedPrograms.size());
    if (mMaterial->isSharedVariant(variant)) {
        FMaterial const* defaultMaterial = engine.getDefaultMaterial();
        assert_invariant(defaultMaterial);
        LocalProgramCache const& defaultPrograms = defaultMaterial->getPrograms();
        CacheKey const defaultMappedKey =
                mapCacheEntryKey(variant, specKey, defaultPrograms.mCachedPrograms.size());
        result = defaultPrograms.mCachedPrograms[defaultMappedKey];
        if (!result) {
            result = defaultPrograms.prepareProgram(driver, variant, specKey, priorityQueue);
        }
    } else {
        result = mMaterial->getDefinition().prepareProgram(engine, driver,
                mMaterial->getMaterialParser(), getProgramSpecialization(variant, specKey),
                priorityQueue);
    }

    FILAMENT_CHECK_POSTCONDITION(result)
            << "Requested variant " << variant << " with specKey "
            << (uint32_t) specKey.key << " does not exist for material " << mMaterial->getName();

    return mCachedPrograms[mappedKey] = result;
}

ProgramSpecialization LocalProgramCache::getProgramSpecialization(Variant variant,
        DynamicSpecConstKey specKey) const noexcept {
    assert_invariant(mMaterial != nullptr);

    return ProgramSpecialization{
        .materialCrc32 = mMaterial->getMaterialParser().getCrc32(),
        .variant = variant,
        .specKey = specKey,
        .specializationConstants = mSpecializationConstants.get(),
    };
}

void LocalProgramCache::terminate(FEngine& engine) {
    assert_invariant(mMaterial != nullptr);

    mMaterial->getDefinition().releasePrograms(engine, mCachedPrograms.as_slice(),
            mMaterial->getMaterialParser(), mSpecializationConstants.get(),
            mMaterial->isDefaultMaterial());
    engine.getMaterialCache().releaseMaterial(engine, mMaterial->getDefinition());
}

void LocalProgramCache::clear(FEngine& engine) {
    assert_invariant(mMaterial != nullptr);

    mMaterial->getDefinition().releasePrograms(engine, mCachedPrograms.as_slice(),
            mMaterial->getMaterialParser(), mSpecializationConstants.get(),
            mMaterial->isDefaultMaterial());
}

Variant LocalProgramCache::filterVariantForGetProgram(Variant variant) const noexcept {
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

Program::SpecializationConstant LocalProgramCache::getConstantImpl(uint32_t id) const noexcept {
    return mSpecializationConstants.get()[id];
}

Program::SpecializationConstant LocalProgramCache::getConstantImpl(
        std::string_view name) const noexcept {
    assert_invariant(mMaterial != nullptr);

    auto const& constants = mMaterial->getDefinition().specializationConstantsNameToIndex;
    auto it = constants.find(name);
    FILAMENT_CHECK_PRECONDITION(it != constants.end()) << "Constant " << name << " does not exist";

    return getConstantImpl(it->second + CONFIG_MAX_INTERNAL_SPEC_CONSTANTS);
}

void LocalProgramCache::setConstants(
        std::initializer_list<std::pair<uint32_t, Program::SpecializationConstant>>
                constants) noexcept {
    assert_invariant(mMaterial != nullptr);

    auto newSpecializationConstants =
            FixedCapacityVector<Program::SpecializationConstant>(mSpecializationConstants.get());

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

void LocalProgramCache::setConstants(
        std::initializer_list<std::pair<std::string_view, Program::SpecializationConstant>>
                constants) noexcept {
    assert_invariant(mMaterial != nullptr);

    auto newSpecializationConstants =
            FixedCapacityVector<Program::SpecializationConstant>(mSpecializationConstants.get());

    bool hasChanged = false;
    for (const auto& [name, value] : constants) {
        MaterialDefinition const& definition = mMaterial->getDefinition();
        auto it = definition.specializationConstantsNameToIndex.find(name);
        if (it != definition.specializationConstantsNameToIndex.cend()) {
            uint32_t id = it->second + CONFIG_MAX_INTERNAL_SPEC_CONSTANTS;
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

void LocalProgramCache::setConstants(
        FixedCapacityVector<Program::SpecializationConstant> constants) noexcept {
    assert_invariant(mMaterial != nullptr);

    setConstantsImpl(std::move(constants));
}

void LocalProgramCache::setConstantsImpl(
        FixedCapacityVector<Program::SpecializationConstant> constants) noexcept {
    FEngine& engine = mMaterial->getEngine();

    auto& internPool = engine.getMaterialCache().getSpecializationConstantsInternPool();
    MaterialParser const& materialParser = mMaterial->getMaterialParser();
    MaterialDefinition const& definition = mMaterial->getDefinition();
    const bool isDefaultMaterial = mMaterial->isDefaultMaterial();

    // Release old resources...
    definition.releasePrograms(engine, mCachedPrograms.as_slice(), materialParser,
            mSpecializationConstants.get(), isDefaultMaterial);

    // Then acquire new ones.
    mSpecializationConstants = internPool.acquire(std::move(constants));
    definition.acquirePrograms(engine, mCachedPrograms.as_slice(), materialParser,
            mSpecializationConstants.get(), isDefaultMaterial);
}

template int32_t LocalProgramCache::getConstant<int32_t>(uint32_t id) const noexcept;
template float LocalProgramCache::getConstant<float>(uint32_t id) const noexcept;
template bool LocalProgramCache::getConstant<bool>(uint32_t id) const noexcept;

template int32_t LocalProgramCache::getConstant<int32_t>(std::string_view name) const noexcept;
template float LocalProgramCache::getConstant<float>(std::string_view name) const noexcept;
template bool LocalProgramCache::getConstant<bool>(std::string_view name) const noexcept;

} // namespace filament
