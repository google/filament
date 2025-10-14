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

#include "details/Material.h"
#include "details/Engine.h"

#include "Froxelizer.h"
#include "MaterialParser.h"

#include "ds/ColorPassDescriptorSet.h"

#include "FilamentAPI-impl.h"

#include <private/filament/EngineEnums.h>
#include <private/filament/DescriptorSets.h>
#include <private/filament/SamplerInterfaceBlock.h>
#include <private/filament/BufferInterfaceBlock.h>
#include <private/filament/Variant.h>

#include <filament/Material.h>
#include <filament/MaterialEnums.h>

#if FILAMENT_ENABLE_MATDBG
#include <matdbg/DebugServer.h>
#endif

#include <filaflat/ChunkContainer.h>

#include <backend/DriverApiForward.h>
#include <backend/DriverEnums.h>
#include <backend/CallbackHandler.h>
#include <backend/Program.h>

#include <utils/BitmaskEnum.h>
#include <utils/CString.h>
#include <utils/FixedCapacityVector.h>
#include <utils/Hash.h>
#include <utils/Invocable.h>
#include <utils/Logger.h>
#include <utils/Panic.h>
#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/ostream.h>

#include <algorithm>
#include <array>
#include <new>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <variant>

#include <stddef.h>
#include <stdint.h>

namespace filament {

namespace {

using namespace backend;
using namespace filaflat;
using namespace utils;
using UboBatchingMode = Material::UboBatchingMode;

bool shouldEnableBatching(FEngine& engine, UboBatchingMode batchingMode, MaterialDomain domain) {
    return batchingMode != UboBatchingMode::DISABLED && engine.isUboBatchingEnabled() &&
           domain == MaterialDomain::SURFACE;
}

} // anonymous namespace

struct Material::BuilderDetails {
    const void* mPayload = nullptr;
    size_t mSize = 0;
    bool mDefaultMaterial = false;
    int32_t mShBandsCount = 3;
    Builder::ShadowSamplingQuality mShadowSamplingQuality = Builder::ShadowSamplingQuality::LOW;
    UboBatchingMode mUboBatchingMode = UboBatchingMode::DEFAULT;
    std::unordered_map<
        CString,
        std::variant<int32_t, float, bool>> mConstantSpecializations;
};

FMaterial::DefaultMaterialBuilder::DefaultMaterialBuilder() {
    mImpl->mDefaultMaterial = true;
}

using BuilderType = Material;
BuilderType::Builder::Builder() noexcept = default;
BuilderType::Builder::~Builder() noexcept = default;
BuilderType::Builder::Builder(Builder const& rhs) noexcept = default;
BuilderType::Builder::Builder(Builder&& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(Builder const& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(Builder&& rhs) noexcept = default;

Material::Builder& Material::Builder::package(const void* payload, size_t const size) {
    mImpl->mPayload = payload;
    mImpl->mSize = size;
    return *this;
}

Material::Builder& Material::Builder::sphericalHarmonicsBandCount(size_t const shBandCount) noexcept {
    mImpl->mShBandsCount = math::clamp(shBandCount, size_t(1), size_t(3));
    return *this;
}

Material::Builder& Material::Builder::shadowSamplingQuality(ShadowSamplingQuality const quality) noexcept {
    mImpl->mShadowSamplingQuality = quality;
    return *this;
}

Material::Builder& Material::Builder::uboBatching(UboBatchingMode const mode) noexcept {
    mImpl->mUboBatchingMode = mode;
    return *this;
}

template<typename T, typename>
Material::Builder& Material::Builder::constant(const char* name, size_t nameLength, T value) {
    FILAMENT_CHECK_PRECONDITION(name != nullptr) << "name cannot be null";
    mImpl->mConstantSpecializations[{name, nameLength}] = value;
    return *this;
}

template Material::Builder& Material::Builder::constant<int32_t>(const char*, size_t, int32_t);
template Material::Builder& Material::Builder::constant<float>(const char*, size_t, float);
template Material::Builder& Material::Builder::constant<bool>(const char*, size_t, bool);


const char* toString(ShaderModel model) {
    switch (model) {
        case ShaderModel::MOBILE:
            return "mobile";
        case ShaderModel::DESKTOP:
            return "desktop";
    }
}

Material* Material::Builder::build(Engine& engine) const {
    MaterialDefinition* r = downcast(engine).getMaterialCache().acquireMaterial(downcast(engine),
            mImpl->mPayload, mImpl->mSize);
    if (r) {
        return downcast(engine).createMaterial(*this, *r);
    }
    return nullptr;
}

FMaterial::FMaterial(FEngine& engine, const Builder& builder, MaterialDefinition const& definition)
        : mDefinition(definition),
          mIsDefaultMaterial(builder->mDefaultMaterial),
          mUseUboBatching(shouldEnableBatching(engine, builder->mUboBatchingMode,
                  definition.materialDomain)),
          mEngine(engine),
          mMaterialId(engine.getMaterialId()) {

    FILAMENT_CHECK_PRECONDITION(!mUseUboBatching || engine.isUboBatchingEnabled())
            << "UBO batching is not enabled.";

    mDepthPrecacheDisabled = engine.getDriverApi().isWorkaroundNeeded(
            Workaround::DISABLE_DEPTH_PRECACHE_FOR_DEFAULT_MATERIAL);

    FixedCapacityVector<Program::SpecializationConstant> specializationConstants =
            processSpecializationConstants(builder);
    mSpecializationConstants =
            engine.getMaterialCache().getSpecializationConstantsInternPool().acquire(
                    std::move(specializationConstants));

    size_t cachedProgramsSize;
    switch (mDefinition.materialDomain) {
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

    mDefinition.acquirePrograms(engine, mCachedPrograms.as_slice(), mSpecializationConstants,
            mIsDefaultMaterial);

#if FILAMENT_ENABLE_MATDBG
    // Register the material with matdbg.
    matdbg::DebugServer* server = downcast(engine).debug.server;
    if (UTILS_UNLIKELY(server)) {
        auto const details = builder.mImpl;
        mDebuggerId = server->addMaterial(mDefinition.name, details->mPayload, details->mSize, this);
    }
#endif
}

FMaterial::~FMaterial() noexcept = default;

void FMaterial::terminate(FEngine& engine) {
    if (mDefaultMaterialInstance) {
        mDefaultMaterialInstance->setDefaultInstance(false);
        engine.destroy(mDefaultMaterialInstance);
        mDefaultMaterialInstance = nullptr;
    }

    // ensure we've destroyed all instances before destroying the material
    auto const& materialInstanceResourceList = engine.getMaterialInstanceResourceList();
    auto pos = materialInstanceResourceList.find(this);
    if (UTILS_LIKELY(pos != materialInstanceResourceList.cend())) {
        auto const& featureFlags = engine.features.engine.debug;
        FILAMENT_FLAG_GUARDED_CHECK_PRECONDITION(pos->second.empty(),
                featureFlags.assert_destroy_material_before_material_instance)
                << "destroying material \"" << this->getName().c_str_safe() << "\" but "
                << pos->second.size() << " instances still alive.";
    }

#if FILAMENT_ENABLE_MATDBG
    // Unregister the material with matdbg.
    matdbg::DebugServer* server = engine.debug.server;
    if (UTILS_UNLIKELY(server)) {
        server->removeMaterial(mDebuggerId);
    }
#endif

    mDefinition.releasePrograms(engine, mCachedPrograms.as_slice(), mSpecializationConstants,
            mIsDefaultMaterial);
    engine.getMaterialCache().releaseMaterial(engine, mDefinition);
    engine.getMaterialCache().getSpecializationConstantsInternPool().release(
            mSpecializationConstants);
}

backend::DescriptorSetLayout const& FMaterial::getPerViewDescriptorSetLayoutDescription(
        Variant const variant, bool const useVsmDescriptorSetLayout) const noexcept {
    if (mDefinition.materialDomain == MaterialDomain::SURFACE) {
        if (Variant::isValidDepthVariant(variant)) {
            // Use the layout description used to create the per view depth variant layout.
            return descriptor_sets::getDepthVariantLayout();
        }
        if (Variant::isSSRVariant(variant)) {
            // Use the layout description used to create the per view SSR variant layout.
            return descriptor_sets::getSsrVariantLayout();
        }
    }
    if (useVsmDescriptorSetLayout) {
        return mDefinition.perViewDescriptorSetLayoutVsmDescription;
    }
    return mDefinition.perViewDescriptorSetLayoutDescription;
}

filament::DescriptorSetLayout const& FMaterial::getPerViewDescriptorSetLayout(
        Variant const variant, bool const useVsmDescriptorSetLayout) const noexcept {
    if (mDefinition.materialDomain == MaterialDomain::SURFACE) {
        // `variant` is only sensical for MaterialDomain::SURFACE
        if (Variant::isValidDepthVariant(variant)) {
            return mEngine.getPerViewDescriptorSetLayoutDepthVariant();
        }
        if (Variant::isSSRVariant(variant)) {
            return mEngine.getPerViewDescriptorSetLayoutSsrVariant();
        }
    }
    // mDefinition.perViewDescriptorSetLayout{Vsm} is already resolved for MaterialDomain
    if (useVsmDescriptorSetLayout) {
        return mDefinition.perViewDescriptorSetLayoutVsm;
    }
    return mDefinition.perViewDescriptorSetLayout;
}

backend::DescriptorSetLayout const& FMaterial::getDescriptorSetLayoutDescription(Variant variant)
        const noexcept {
    if (!isSharedVariant(variant)) {
        return mDefinition.descriptorSetLayoutDescription;
    }
    if (UTILS_UNLIKELY(!mDefaultMaterial)) {
        return mDefinition.descriptorSetLayoutDescription;
    }
    return mDefaultMaterial->getDescriptorSetLayoutDescription();
}

void FMaterial::compile(CompilerPriorityQueue const priority,
        UserVariantFilterMask variantSpec,
        CallbackHandler* handler,
        Invocable<void(Material*)>&& callback) noexcept {

    DriverApi& driver = mEngine.getDriverApi();

    // Turn off the STE variant if stereo is not supported.
    if (!mIsStereoSupported) {
        variantSpec &= ~UserVariantFilterMask(UserVariantFilterBit::STE);
    }

    UserVariantFilterMask const variantFilter =
            ~variantSpec & UserVariantFilterMask(UserVariantFilterBit::ALL);
    ShaderModel const shaderModel = mEngine.getShaderModel();
    bool const isStereoSupported = mEngine.getDriverApi().isStereoSupported();

    if (UTILS_LIKELY(mIsParallelShaderCompileSupported)) {
        for (auto const variant: mDefinition.getVariants()) {
            if (!variantFilter || variant == Variant::filterUserVariant(variant, variantFilter)) {
                if (mDefinition.hasVariant(variant, shaderModel, isStereoSupported)) {
                    prepareProgram(driver, variant, priority);
                }
            }
        }
    }

    if (callback) {
        struct Callback {
            Invocable<void(Material*)> f;
            Material* m;
            static void func(void* user) {
                auto* const c = static_cast<Callback*>(user);
                c->f(c->m);
                delete c;
            }
        };
        auto* const user = new(std::nothrow) Callback{ std::move(callback), this };
        driver.compilePrograms(priority, handler, &Callback::func, user);
    } else {
        driver.compilePrograms(priority, nullptr, nullptr, nullptr);
    }
}

Handle<HwProgram> FMaterial::prepareProgramSlow(Variant const variant,
        CompilerPriorityQueue const priorityQueue) const noexcept {
    if (isSharedVariant(variant)) {
        FMaterial const* defaultMaterial = mEngine.getDefaultMaterial();
        FILAMENT_CHECK_PRECONDITION(defaultMaterial);
        return mCachedPrograms[variant.key] =
                defaultMaterial->prepareProgram(variant, priorityQueue);
    }
    return mCachedPrograms[variant.key] = mEngine.getMaterialCache().prepareProgram(mEngine,
                   mDefinition, getProgramSpecialization(variant), priorityQueue);
}

[[nodiscard]]
Handle<HwProgram> FMaterial::getProgramSlow(Variant const variant) const noexcept {
    if (isSharedVariant(variant)) {
        FMaterial const* defaultMaterial = mEngine.getDefaultMaterial();
        FILAMENT_CHECK_PRECONDITION(defaultMaterial);
        return mCachedPrograms[variant.key] = defaultMaterial->getProgram(variant);
    }
    return mCachedPrograms[variant.key] =
                   mEngine.getMaterialCache().getProgram(getProgramSpecialization(variant));
}

#if FILAMENT_ENABLE_MATDBG
void FMaterial::updateActiveProgramsForMatdbg(Variant const variant) const noexcept {
    assert_invariant((size_t)variant.key < VARIANT_COUNT);
    std::unique_lock lock(mActiveProgramsLock);
    if (getMaterialDomain() == MaterialDomain::SURFACE) {
        auto vert = Variant::filterVariantVertex(variant);
        auto frag = Variant::filterVariantFragment(variant);
        mActivePrograms.set(vert.key);
        mActivePrograms.set(frag.key);
    } else {
        mActivePrograms.set(variant.key);
    }
    lock.unlock();
}
#endif

FMaterialInstance* FMaterial::createInstance(const char* name) const noexcept {
    if (mDefaultMaterialInstance) {
        // if we have a default instance, use it to create a new one
        return FMaterialInstance::duplicate(mDefaultMaterialInstance, name);
    } else {
        // but if we don't, just create an instance with all the default parameters
        return mEngine.createMaterialInstance(this, name);
    }
}

FMaterialInstance* FMaterial::getDefaultInstance() noexcept {
    if (UTILS_UNLIKELY(!mDefaultMaterialInstance)) {
        mDefaultMaterialInstance =
                mEngine.createMaterialInstance(this, mDefinition.name.c_str());
        mDefaultMaterialInstance->setDefaultInstance(true);
    }
    return mDefaultMaterialInstance;
}

bool FMaterial::hasParameter(const char* name) const noexcept {
    return mDefinition.uniformInterfaceBlock.hasField(name) ||
           mDefinition.samplerInterfaceBlock.hasSampler(name) ||
            mDefinition.subpassInfo.name == CString(name);
}

bool FMaterial::isSampler(const char* name) const noexcept {
    return mDefinition.samplerInterfaceBlock.hasSampler(name);
}

BufferInterfaceBlock::FieldInfo const* FMaterial::reflect(
        std::string_view const name) const noexcept {
    return mDefinition.uniformInterfaceBlock.getFieldInfo(name);
}

MaterialParser const& FMaterial::getMaterialParser() const noexcept {
#if FILAMENT_ENABLE_MATDBG
    if (mEditedMaterialParser) {
        return *mEditedMaterialParser;
    }
#endif
    return mDefinition.getMaterialParser();
}

ProgramSpecialization FMaterial::getProgramSpecialization(Variant const variant) const noexcept {
    return ProgramSpecialization {
        .programCacheId = mDefinition.cacheId,
        .variant = variant,
        .specializationConstants = mSpecializationConstants,
    };
}

size_t FMaterial::getParameters(ParameterInfo* parameters, size_t count) const noexcept {
    count = std::min(count, getParameterCount());

    const auto& uniforms = mDefinition.uniformInterfaceBlock.getFieldInfoList();
    size_t i = 0;
    size_t const uniformCount = std::min(count, size_t(uniforms.size()));
    for ( ; i < uniformCount; i++) {
        ParameterInfo& info = parameters[i];
        const auto& uniformInfo = uniforms[i];
        info.name = uniformInfo.name.c_str();
        info.isSampler = false;
        info.isSubpass = false;
        info.type = uniformInfo.type;
        info.count = std::max(1u, uniformInfo.size);
        info.precision = uniformInfo.precision;
    }

    const auto& samplers = mDefinition.samplerInterfaceBlock.getSamplerInfoList();
    size_t const samplerCount = samplers.size();
    for (size_t j = 0; i < count && j < samplerCount; i++, j++) {
        ParameterInfo& info = parameters[i];
        const auto& samplerInfo = samplers[j];
        info.name = samplerInfo.name.c_str();
        info.isSampler = true;
        info.isSubpass = false;
        info.samplerType = samplerInfo.type;
        info.count = 1;
        info.precision = samplerInfo.precision;
    }

    if (mDefinition.subpassInfo.isValid && i < count) {
        ParameterInfo& info = parameters[i];
        info.name = mDefinition.subpassInfo.name.c_str();
        info.isSampler = false;
        info.isSubpass = true;
        info.subpassType = mDefinition.subpassInfo.type;
        info.count = 1;
        info.precision = mDefinition.subpassInfo.precision;
    }

    return count;
}

#if FILAMENT_ENABLE_MATDBG

// Swaps in an edited version of the original package that was used to create the material. The
// edited package was stashed in response to a debugger event. This is invoked only when the
// Material Debugger is attached. The only editable features of a material package are the shader
// source strings, so here we trigger a rebuild of the HwProgram objects.
void FMaterial::applyPendingEdits() noexcept {
    const char* name = mDefinition.name.c_str();
    DLOG(INFO) << "Applying edits to " << (name ? name : "(untitled)");
    destroyPrograms(mEngine); // FIXME: this will not destroy the shared variants
    latchPendingEdits();
}

void FMaterial::setPendingEdits(std::unique_ptr<MaterialParser> pendingEdits) noexcept {
    std::lock_guard const lock(mPendingEditsLock);
    std::swap(pendingEdits, mPendingEdits);
}

bool FMaterial::hasPendingEdits() const noexcept {
    std::lock_guard const lock(mPendingEditsLock);
    return bool(mPendingEdits);
}

void FMaterial::latchPendingEdits() noexcept {
    std::lock_guard const lock(mPendingEditsLock);
    mEditedMaterialParser = std::move(mPendingEdits);
}

/**
 * Callback handlers for the debug server, potentially called from any thread. These methods are
 * never called during normal operation and exist for debugging purposes only.
 *
 * @{
 */

void FMaterial::onEditCallback(void* userdata, const CString&, const void* packageData,
        size_t const packageSize) {
    FMaterial* material = downcast(static_cast<Material*>(userdata));
    FEngine const& engine = material->mEngine;

    // This is called on a web server thread, so we defer clearing the program cache
    // and swapping out the MaterialParser until the next getProgram call.
    std::unique_ptr<MaterialParser> pending = MaterialDefinition::createParser(
            engine.getBackend(), engine.getShaderLanguage(), packageData, packageSize);
    material->setPendingEdits(std::move(pending));
}

void FMaterial::onQueryCallback(void* userdata, VariantList* pActiveVariants) {
    FMaterial const* material = downcast(static_cast<Material*>(userdata));
    std::lock_guard const lock(material->mActiveProgramsLock);
    *pActiveVariants = material->mActivePrograms;
    material->mActivePrograms.reset();
}

/** @}*/

#endif // FILAMENT_ENABLE_MATDBG

void FMaterial::setSpecializationConstants(SpecializationConstantsBuilder&& builder) noexcept {
    if (builder.mConstants.empty()) {
        // Nothing was changed.
        return;
    }

    auto& internPool = mEngine.getMaterialCache().getSpecializationConstantsInternPool();

    // Release old resources...
    mDefinition.releasePrograms(mEngine, mCachedPrograms.as_slice(), mSpecializationConstants,
            mIsDefaultMaterial);
    internPool.release(mSpecializationConstants);

    // Then acquire new ones
    mSpecializationConstants = internPool.acquire(std::move(builder.mConstants));
    mDefinition.acquirePrograms(mEngine, mCachedPrograms.as_slice(), mSpecializationConstants,
            mIsDefaultMaterial);
}

FixedCapacityVector<Program::SpecializationConstant> FMaterial::processSpecializationConstants(
        Builder const& builder) {
    FixedCapacityVector<Program::SpecializationConstant> specializationConstants =
            mDefinition.specializationConstants;

    specializationConstants[+ReservedSpecializationConstants::CONFIG_SH_BANDS_COUNT] =
            builder->mShBandsCount;
    specializationConstants[+ReservedSpecializationConstants::CONFIG_SHADOW_SAMPLING_METHOD] =
            int32_t(builder->mShadowSamplingQuality);

    // Verify that all the constant specializations exist in the material and that their types
    // match.
    for (auto const& [name, value] : builder->mConstantSpecializations) {
        std::string_view const key{ name.data(), name.size() };
        auto pos = mDefinition.specializationConstantsNameToIndex.find(key);
        FILAMENT_CHECK_PRECONDITION(pos != mDefinition.specializationConstantsNameToIndex.end())
                << "The material " << mDefinition.name.c_str_safe()
                << " does not have a constant parameter named " << name.c_str() << ".";
        constexpr char const* const types[3] = {"an int", "a float", "a bool"};
        auto const& constant = mDefinition.materialConstants[pos->second];
        switch (constant.type) {
            case ConstantType::INT:
                FILAMENT_CHECK_PRECONDITION(std::holds_alternative<int32_t>(value))
                        << "The constant parameter " << name.c_str() << " on material "
                        << mDefinition.name.c_str_safe() << " is of type int, but "
                        << types[value.index()] << " was provided.";
                break;
            case ConstantType::FLOAT:
                FILAMENT_CHECK_PRECONDITION(std::holds_alternative<float>(value))
                        << "The constant parameter " << name.c_str() << " on material "
                        << mDefinition.name.c_str_safe() << " is of type float, but "
                        << types[value.index()] << " was provided.";
                break;
            case ConstantType::BOOL:
                FILAMENT_CHECK_PRECONDITION(std::holds_alternative<bool>(value))
                        << "The constant parameter " << name.c_str() << " on material "
                        << mDefinition.name.c_str_safe() << " is of type bool, but "
                        << types[value.index()] << " was provided.";
                break;
        }
        uint32_t const index = pos->second + CONFIG_MAX_RESERVED_SPEC_CONSTANTS;
        specializationConstants[index] = value;
    }
    return specializationConstants;
}

descriptor_binding_t FMaterial::getSamplerBinding(
        std::string_view const& name) const {
    return mDefinition.samplerInterfaceBlock.getSamplerInfo(name)->binding;
}

const char* FMaterial::getParameterTransformName(std::string_view samplerName) const noexcept {
    auto const& sib = getSamplerInterfaceBlock();
    SamplerInterfaceBlock::SamplerInfo const* info = sib.getSamplerInfo(samplerName);
    if (!info || info->transformName.empty()) {
        return nullptr;
    }
    return info->transformName.c_str();
}

template FMaterial::SpecializationConstantsBuilder& FMaterial::SpecializationConstantsBuilder::set<int32_t>(uint32_t id, int32_t value) noexcept;
template FMaterial::SpecializationConstantsBuilder& FMaterial::SpecializationConstantsBuilder::set<float>(uint32_t id, float value) noexcept;
template FMaterial::SpecializationConstantsBuilder& FMaterial::SpecializationConstantsBuilder::set<bool>(uint32_t id, bool value) noexcept;

template FMaterial::SpecializationConstantsBuilder& FMaterial::SpecializationConstantsBuilder::set<int32_t>(std::string_view name, int32_t value) noexcept;
template FMaterial::SpecializationConstantsBuilder& FMaterial::SpecializationConstantsBuilder::set<float>(std::string_view name, float value) noexcept;
template FMaterial::SpecializationConstantsBuilder& FMaterial::SpecializationConstantsBuilder::set<bool>(std::string_view name, bool value) noexcept;

} // namespace filament
