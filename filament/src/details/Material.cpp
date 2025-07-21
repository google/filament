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
#include <private/filament/PushConstantInfo.h>
#include <private/filament/Variant.h>

#include <filament/Material.h>
#include <filament/MaterialEnums.h>

#if FILAMENT_ENABLE_MATDBG
#include <matdbg/DebugServer.h>
#endif

#include <filaflat/ChunkContainer.h>

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
#include <utils/bitset.h>
#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/ostream.h>

#include <algorithm>
#include <array>
#include <iterator>
#include <memory>
#include <mutex>
#include <new>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <variant>

#include <stddef.h>
#include <stdint.h>

namespace filament {

using namespace backend;
using namespace filaflat;
using namespace utils;

static std::unique_ptr<MaterialParser> createParser(Backend const backend,
        FixedCapacityVector<ShaderLanguage> languages, const void* data, size_t size) {
    // unique_ptr so we don't leak MaterialParser on failures below
    auto materialParser = std::make_unique<MaterialParser>(languages, data, size);

    MaterialParser::ParseResult const materialResult = materialParser->parse();

    if (UTILS_UNLIKELY(materialResult == MaterialParser::ParseResult::ERROR_MISSING_BACKEND)) {
        std::string languageNames;
        for (auto it = languages.begin(); it != languages.end(); ++it) {
            languageNames.append(shaderLanguageToString(*it));
            if (std::next(it) != languages.end()) {
                languageNames.append(", ");
            }
        }

        FILAMENT_CHECK_POSTCONDITION(
                materialResult != MaterialParser::ParseResult::ERROR_MISSING_BACKEND)
                << "the material was not built for any of the " << to_string(backend)
                << " backend's supported shader languages (" << languageNames.c_str() << ")\n";
    }

    if (backend == Backend::NOOP) {
        return materialParser;
    }

    FILAMENT_CHECK_POSTCONDITION(materialResult == MaterialParser::ParseResult::SUCCESS)
            << "could not parse the material package";

    uint32_t version = 0;
    materialParser->getMaterialVersion(&version);
    FILAMENT_CHECK_POSTCONDITION(version == MATERIAL_VERSION)
            << "Material version mismatch. Expected " << MATERIAL_VERSION << " but received "
            << version << ".";

    assert_invariant(backend != Backend::DEFAULT && "Default backend has not been resolved.");

    return materialParser;
}

struct Material::BuilderDetails {
    const void* mPayload = nullptr;
    size_t mSize = 0;
    bool mDefaultMaterial = false;
    int32_t mShBandsCount = 3;
    Builder::ShadowSamplingQuality mShadowSamplingQuality = Builder::ShadowSamplingQuality::LOW;
    std::unordered_map<
        CString,
        std::variant<int32_t, float, bool>,
        CString::Hasher> mConstantSpecializations;
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
    std::unique_ptr<MaterialParser> materialParser = createParser(
        downcast(engine).getBackend(), downcast(engine).getShaderLanguage(),
        mImpl->mPayload, mImpl->mSize);

    if (!materialParser) {
        return nullptr;
    }

    uint32_t v = 0;
    materialParser->getShaderModels(&v);
    bitset32 shaderModels;
    shaderModels.setValue(v);

    ShaderModel const shaderModel = downcast(engine).getShaderModel();
    if (!shaderModels.test(static_cast<uint32_t>(shaderModel))) {
        CString name;
        materialParser->getName(&name);
        char shaderModelsString[16];
        snprintf(shaderModelsString, sizeof(shaderModelsString), "%#x", shaderModels.getValue());
        LOG(ERROR) << "The material '" << name.c_str_safe() << "' was not built for "
                   << toString(shaderModel) << ".";
        LOG(ERROR) << "Compiled material contains shader models " << shaderModelsString << ".";
        return nullptr;
    }

    // Print a warning if the material's stereo type doesn't align with the engine's setting.
    MaterialDomain materialDomain;
    UserVariantFilterMask variantFilterMask;
    materialParser->getMaterialDomain(&materialDomain);
    materialParser->getMaterialVariantFilterMask(&variantFilterMask);
    bool const hasStereoVariants = !(variantFilterMask & UserVariantFilterMask(UserVariantFilterBit::STE));
    if (materialDomain == MaterialDomain::SURFACE && hasStereoVariants) {
        StereoscopicType const engineStereoscopicType = engine.getConfig().stereoscopicType;
        // Default materials are always compiled with either 'instanced' or 'multiview'.
        // So, we only verify compatibility if the engine is set up for stereo.
        if (engineStereoscopicType != StereoscopicType::NONE) {
            StereoscopicType materialStereoscopicType = StereoscopicType::NONE;
            materialParser->getStereoscopicType(&materialStereoscopicType);
            if (materialStereoscopicType != engineStereoscopicType) {
                CString name;
                materialParser->getName(&name);
                LOG(WARNING) << "The stereoscopic type in the compiled material '"
                             << name.c_str_safe() << "' is " << (int) materialStereoscopicType
                             << ", which is not compatiable with the engine's setting "
                             << (int) engineStereoscopicType << ".";
            }
        }
    }

    return downcast(engine).createMaterial(*this, std::move(materialParser));
}

FMaterial::FMaterial(FEngine& engine, const Builder& builder,
        std::unique_ptr<MaterialParser> materialParser)
        : mIsDefaultMaterial(builder->mDefaultMaterial),
          mEngine(engine),
          mMaterialId(engine.getMaterialId()),
          mMaterialParser(std::move(materialParser)) {
    MaterialParser* const parser = mMaterialParser.get();

    UTILS_UNUSED_IN_RELEASE bool const nameOk = parser->getName(&mName);
    assert_invariant(nameOk);

    mFeatureLevel = [parser]() -> FeatureLevel {
        // code written this way so the IDE will complain when/if we add a FeatureLevel
        uint8_t level = 1;
        parser->getFeatureLevel(&level);
        assert_invariant(level <= 3);
        FeatureLevel featureLevel = FeatureLevel::FEATURE_LEVEL_1;
        switch (FeatureLevel(level)) {
            case FeatureLevel::FEATURE_LEVEL_0:
            case FeatureLevel::FEATURE_LEVEL_1:
            case FeatureLevel::FEATURE_LEVEL_2:
            case FeatureLevel::FEATURE_LEVEL_3:
                featureLevel = FeatureLevel(level);
                break;
        }
        return featureLevel;
    }();

    UTILS_UNUSED_IN_RELEASE bool success;

    success = parser->getCacheId(&mCacheId);
    assert_invariant(success);

    success = parser->getSIB(&mSamplerInterfaceBlock);
    assert_invariant(success);

    success = parser->getUIB(&mUniformInterfaceBlock);
    assert_invariant(success);

    if (UTILS_UNLIKELY(parser->getShaderLanguage() == ShaderLanguage::ESSL1)) {
        success = parser->getAttributeInfo(&mAttributeInfo);
        assert_invariant(success);

        success = parser->getBindingUniformInfo(&mBindingUniformInfo);
        assert_invariant(success);
    }

    // Older materials will not have a subpass chunk; this should not be an error.
    if (!parser->getSubpasses(&mSubpassInfo)) {
        mSubpassInfo.isValid = false;
    }

    parser->getShading(&mShading);
    parser->getMaterialProperties(&mMaterialProperties);
    parser->getInterpolation(&mInterpolation);
    parser->getVertexDomain(&mVertexDomain);
    parser->getMaterialDomain(&mMaterialDomain);
    parser->getMaterialVariantFilterMask(&mVariantFilterMask);
    parser->getRequiredAttributes(&mRequiredAttributes);
    parser->getRefractionMode(&mRefractionMode);
    parser->getRefractionType(&mRefractionType);
    parser->getReflectionMode(&mReflectionMode);
    parser->getTransparencyMode(&mTransparencyMode);
    parser->getDoubleSided(&mDoubleSided);
    parser->getCullingMode(&mCullingMode);

    if (mShading == Shading::UNLIT) {
        parser->hasShadowMultiplier(&mHasShadowMultiplier);
    }

    mIsVariantLit = mShading != Shading::UNLIT || mHasShadowMultiplier;

    // color write
    bool colorWrite = false;
    parser->getColorWrite(&colorWrite);
    mRasterState.colorWrite = colorWrite;

    // depth test
    bool depthTest = false;
    parser->getDepthTest(&depthTest);
    mRasterState.depthFunc = depthTest ? RasterState::DepthFunc::GE : RasterState::DepthFunc::A;

    // if doubleSided() was called we override culling()
    bool doubleSideSet = false;
    parser->getDoubleSidedSet(&doubleSideSet);
    if (doubleSideSet) {
        mDoubleSidedCapability = true;
        mRasterState.culling = mDoubleSided ? CullingMode::NONE : mCullingMode;
    } else {
        mRasterState.culling = mCullingMode;
    }

    // specular anti-aliasing
    parser->hasSpecularAntiAliasing(&mSpecularAntiAliasing);
    if (mSpecularAntiAliasing) {
        parser->getSpecularAntiAliasingVariance(&mSpecularAntiAliasingVariance);
        parser->getSpecularAntiAliasingThreshold(&mSpecularAntiAliasingThreshold);
    }

    parser->hasCustomDepthShader(&mHasCustomDepthShader);

    bool const isLit = mIsVariantLit || mHasShadowMultiplier;
    bool const isSSR = mReflectionMode == ReflectionMode::SCREEN_SPACE ||
            mRefractionMode == RefractionMode::SCREEN_SPACE;
    bool const hasFog = !(mVariantFilterMask & UserVariantFilterMask(UserVariantFilterBit::FOG));

    mPerViewLayoutIndex = ColorPassDescriptorSet::getIndex(isLit, isSSR, hasFog);

    processBlendingMode(parser);
    processSpecializationConstants(engine, builder, parser);
    processPushConstants(engine, parser);
    processDescriptorSets(engine, parser);
    precacheDepthVariants(engine);

#if FILAMENT_ENABLE_MATDBG
    // Register the material with matdbg.
    matdbg::DebugServer* server = downcast(engine).debug.server;
    if (UTILS_UNLIKELY(server)) {
        auto const details = builder.mImpl;
        mDebuggerId = server->addMaterial(mName, details->mPayload, details->mSize, this);
    }
#endif
}

FMaterial::~FMaterial() noexcept = default;

void FMaterial::invalidate(Variant::type_t variantMask, Variant::type_t variantValue) noexcept {
    // Note: This API is not public at the moment, so it's okay to have some debugging logs
    // and extra checks.
    if (mMaterialDomain == MaterialDomain::SURFACE &&
        !mIsDefaultMaterial &&
        !mHasCustomDepthShader) {
        // it would be unsafe to invalidate any of the cached depth variant
        if (UTILS_UNLIKELY(!((variantMask & Variant::DEP) && !(variantValue & Variant::DEP)))) {
            char variantMaskString[16];
            snprintf(variantMaskString, sizeof(variantMaskString), "%#x", +variantMask);
            char variantValueString[16];
            snprintf(variantValueString, sizeof(variantValueString), "%#x", +variantValue);
            LOG(WARNING) << "FMaterial::invalidate(" << variantMaskString << ", "
                         << variantValueString << ") would corrupt the depth variant cache";
        }
        variantMask |= Variant::DEP;
        variantValue &= ~Variant::DEP;
    }
    destroyPrograms(mEngine, variantMask, variantValue);
}

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
        if (engine.features.engine.debug.assert_destroy_material_before_material_instance) {
            FILAMENT_CHECK_PRECONDITION(pos->second.empty())
                    << "destroying material \"" << this->getName().c_str_safe() << "\" but "
                    << pos->second.size() << " instances still alive.";
        } else {
            if (UTILS_UNLIKELY(!pos->second.empty())) {
                LOG(ERROR) << "destroying material \"" << this->getName().c_str_safe() << "\" but "
                           << pos->second.size() << " instances still alive.";
            }
        }
    }

#if FILAMENT_ENABLE_MATDBG
    // Unregister the material with matdbg.
    matdbg::DebugServer* server = engine.debug.server;
    if (UTILS_UNLIKELY(server)) {
        server->removeMaterial(mDebuggerId);
    }
#endif

    destroyPrograms(engine);

    DriverApi& driver = engine.getDriverApi();
    mPerViewDescriptorSetLayout.terminate(engine.getDescriptorSetLayoutFactory(), driver);
    mPerViewDescriptorSetLayoutVsm.terminate(engine.getDescriptorSetLayoutFactory(), driver);
    mDescriptorSetLayout.terminate(engine.getDescriptorSetLayoutFactory(), driver);
}

filament::DescriptorSetLayout const& FMaterial::getPerViewDescriptorSetLayout(
        Variant const variant, bool const useVsmDescriptorSetLayout) const noexcept {
    if (Variant::isValidDepthVariant(variant)) {
        assert_invariant(mMaterialDomain == MaterialDomain::SURFACE);
        return mEngine.getPerViewDescriptorSetLayoutDepthVariant();
    }
    if (Variant::isSSRVariant(variant)) {
        assert_invariant(mMaterialDomain == MaterialDomain::SURFACE);
        return mEngine.getPerViewDescriptorSetLayoutSsrVariant();
    }
    if (useVsmDescriptorSetLayout) {
        assert_invariant(mMaterialDomain == MaterialDomain::SURFACE);
        return mPerViewDescriptorSetLayoutVsm;
    }
    return mPerViewDescriptorSetLayout;
}

void FMaterial::compile(CompilerPriorityQueue const priority,
        UserVariantFilterMask variantSpec,
        CallbackHandler* handler,
        Invocable<void(Material*)>&& callback) noexcept {

    // Turn off the STE variant if stereo is not supported.
    if (!mEngine.getDriverApi().isStereoSupported()) {
        variantSpec &= ~UserVariantFilterMask(UserVariantFilterBit::STE);
    }

    UserVariantFilterMask const variantFilter =
            ~variantSpec & UserVariantFilterMask(UserVariantFilterBit::ALL);

    if (UTILS_LIKELY(mEngine.getDriverApi().isParallelShaderCompileSupported())) {
        auto const& variants = isVariantLit() ?
                VariantUtils::getLitVariants() : VariantUtils::getUnlitVariants();
        for (auto const variant: variants) {
            if (!variantFilter || variant == Variant::filterUserVariant(variant, variantFilter)) {
                if (hasVariant(variant)) {
                    prepareProgram(variant, priority);
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
        mEngine.getDriverApi().compilePrograms(priority, handler, &Callback::func, user);
    } else {
        mEngine.getDriverApi().compilePrograms(priority, nullptr, nullptr, nullptr);
    }
}

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
        mDefaultMaterialInstance = mEngine.createMaterialInstance(this, mName.c_str());
        mDefaultMaterialInstance->setDefaultInstance(true);
    }
    return mDefaultMaterialInstance;
}

bool FMaterial::hasParameter(const char* name) const noexcept {
    return mUniformInterfaceBlock.hasField(name) ||
           mSamplerInterfaceBlock.hasSampler(name) ||
            mSubpassInfo.name == CString(name);
}

bool FMaterial::isSampler(const char* name) const noexcept {
    return mSamplerInterfaceBlock.hasSampler(name);
}

BufferInterfaceBlock::FieldInfo const* FMaterial::reflect(
        std::string_view const name) const noexcept {
    return mUniformInterfaceBlock.getFieldInfo(name);
}

bool FMaterial::hasVariant(Variant const variant) const noexcept {
    Variant vertexVariant, fragmentVariant;
    switch (getMaterialDomain()) {
        case MaterialDomain::SURFACE:
            vertexVariant = Variant::filterVariantVertex(variant);
            fragmentVariant = Variant::filterVariantFragment(variant);
            break;
        case MaterialDomain::POST_PROCESS:
            vertexVariant = fragmentVariant = variant;
            break;
        case MaterialDomain::COMPUTE:
            // TODO: implement MaterialDomain::COMPUTE
            return false;
    }
    const ShaderModel sm = mEngine.getShaderModel();
    if (!mMaterialParser->hasShader(sm, vertexVariant, ShaderStage::VERTEX)) {
        return false;
    }
    if (!mMaterialParser->hasShader(sm, fragmentVariant, ShaderStage::FRAGMENT)) {
        return false;
    }
    return true;
}

void FMaterial::prepareProgramSlow(Variant const variant,
        backend::CompilerPriorityQueue const priorityQueue) const noexcept {
    assert_invariant(mEngine.hasFeatureLevel(mFeatureLevel));
    switch (getMaterialDomain()) {
        case MaterialDomain::SURFACE:
            getSurfaceProgramSlow(variant, priorityQueue);
            break;
        case MaterialDomain::POST_PROCESS:
            getPostProcessProgramSlow(variant, priorityQueue);
            break;
        case MaterialDomain::COMPUTE:
            // TODO: implement MaterialDomain::COMPUTE
            break;
    }
}

void FMaterial::getSurfaceProgramSlow(Variant const variant,
        CompilerPriorityQueue const priorityQueue) const noexcept {
    // filterVariant() has already been applied in generateCommands(), shouldn't be needed here
    // if we're unlit, we don't have any bits that correspond to lit materials
    assert_invariant(variant == Variant::filterVariant(variant, isVariantLit()) );

    assert_invariant(!Variant::isReserved(variant));

    Variant const vertexVariant   = Variant::filterVariantVertex(variant);
    Variant const fragmentVariant = Variant::filterVariantFragment(variant);

    Program pb{ getProgramWithVariants(variant, vertexVariant, fragmentVariant) };
    pb.priorityQueue(priorityQueue);
    pb.multiview(
            mEngine.getConfig().stereoscopicType == StereoscopicType::MULTIVIEW &&
            Variant::isStereoVariant(variant));
    createAndCacheProgram(std::move(pb), variant);
}

void FMaterial::getPostProcessProgramSlow(Variant const variant,
        CompilerPriorityQueue const priorityQueue) const noexcept {
    Program pb{ getProgramWithVariants(variant, variant, variant) };
    pb.priorityQueue(priorityQueue);
    createAndCacheProgram(std::move(pb), variant);
}

Program FMaterial::getProgramWithVariants(
        Variant variant,
        Variant vertexVariant,
        Variant fragmentVariant) const {
    FEngine const& engine = mEngine;
    const ShaderModel sm = engine.getShaderModel();
    const bool isNoop = engine.getBackend() == Backend::NOOP;
    /*
     * Vertex shader
     */

    ShaderContent& vsBuilder = engine.getVertexShaderContent();

    UTILS_UNUSED_IN_RELEASE bool const vsOK = mMaterialParser->getShader(vsBuilder, sm,
            vertexVariant, ShaderStage::VERTEX);

    FILAMENT_CHECK_POSTCONDITION(isNoop || (vsOK && !vsBuilder.empty()))
            << "The material '" << mName.c_str()
            << "' has not been compiled to include the required GLSL or SPIR-V chunks for the "
               "vertex shader (variant="
            << +variant.key << ", filtered=" << +vertexVariant.key << ").";

    /*
     * Fragment shader
     */

    ShaderContent& fsBuilder = engine.getFragmentShaderContent();

    UTILS_UNUSED_IN_RELEASE bool const fsOK = mMaterialParser->getShader(fsBuilder, sm,
            fragmentVariant, ShaderStage::FRAGMENT);

    FILAMENT_CHECK_POSTCONDITION(isNoop || (fsOK && !fsBuilder.empty()))
            << "The material '" << mName.c_str()
            << "' has not been compiled to include the required GLSL or SPIR-V chunks for the "
               "fragment shader (variant="
            << +variant.key << ", filtered=" << +fragmentVariant.key << ").";

    Program program;
    program.shader(ShaderStage::VERTEX, vsBuilder.data(), vsBuilder.size())
            .shader(ShaderStage::FRAGMENT, fsBuilder.data(), fsBuilder.size())
            .shaderLanguage(mMaterialParser->getShaderLanguage())
            .diagnostics(mName,
                    [variant, vertexVariant, fragmentVariant](utils::CString const& name,
                            io::ostream& out) -> io::ostream& {
                        return out << name.c_str_safe() << ", variant=(" << io::hex << +variant.key
                                   << io::dec << "), vertexVariant=(" << io::hex
                                   << +vertexVariant.key << io::dec << "), fragmentVariant=("
                                   << io::hex << +fragmentVariant.key << io::dec << ")";
                    });

    if (UTILS_UNLIKELY(mMaterialParser->getShaderLanguage() == ShaderLanguage::ESSL1)) {
        assert_invariant(!mBindingUniformInfo.empty());
        for (auto const& [index, name, uniforms] : mBindingUniformInfo) {
            program.uniforms(uint32_t(index), name, uniforms);
        }
        program.attributes(mAttributeInfo);
    }

    program.descriptorBindings(+DescriptorSetBindingPoints::PER_VIEW,
            mProgramDescriptorBindings[+DescriptorSetBindingPoints::PER_VIEW]);
    program.descriptorBindings(+DescriptorSetBindingPoints::PER_RENDERABLE,
            mProgramDescriptorBindings[+DescriptorSetBindingPoints::PER_RENDERABLE]);
    program.descriptorBindings(+DescriptorSetBindingPoints::PER_MATERIAL,
            mProgramDescriptorBindings[+DescriptorSetBindingPoints::PER_MATERIAL]);
    program.specializationConstants(mSpecializationConstants);

    program.pushConstants(ShaderStage::VERTEX, mPushConstants[uint8_t(ShaderStage::VERTEX)]);
    program.pushConstants(ShaderStage::FRAGMENT, mPushConstants[uint8_t(ShaderStage::FRAGMENT)]);

    program.cacheId(hash::combine(size_t(mCacheId), variant.key));

    return program;
}

void FMaterial::createAndCacheProgram(Program&& p, Variant const variant) const noexcept {
    FEngine const& engine = mEngine;
    DriverApi& driverApi = mEngine.getDriverApi();

    bool const isShared = isSharedVariant(variant);

    // Check if the default material has this program cached
    if (isShared) {
        FMaterial const* const pDefaultMaterial = engine.getDefaultMaterial();
        if (pDefaultMaterial) {
            auto const program = pDefaultMaterial->mCachedPrograms[variant.key];
            if (program) {
                mCachedPrograms[variant.key] = program;
                return;
            }
        }
    }

    auto const program = driverApi.createProgram(std::move(p));
    driverApi.setDebugTag(program.getId(), mName);
    assert_invariant(program);
    mCachedPrograms[variant.key] = program;

    // If the default material doesn't already have this program cached, and all caching conditions
    // are met (Surface Domain and no custom depth shader), cache it now.
    // New Materials will inherit these program automatically.
    if (isShared) {
        FMaterial const* const pDefaultMaterial = engine.getDefaultMaterial();
        if (pDefaultMaterial && !pDefaultMaterial->mCachedPrograms[variant.key]) {
            // set the tag to the default material name
            driverApi.setDebugTag(program.getId(), mName);
            pDefaultMaterial->mCachedPrograms[variant.key] = program;
        }
    }
}

size_t FMaterial::getParameters(ParameterInfo* parameters, size_t count) const noexcept {
    count = std::min(count, getParameterCount());

    const auto& uniforms = mUniformInterfaceBlock.getFieldInfoList();
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

    const auto& samplers = mSamplerInterfaceBlock.getSamplerInfoList();
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

    if (mSubpassInfo.isValid && i < count) {
        ParameterInfo& info = parameters[i];
        info.name = mSubpassInfo.name.c_str();
        info.isSampler = false;
        info.isSubpass = true;
        info.subpassType = mSubpassInfo.type;
        info.count = 1;
        info.precision = mSubpassInfo.precision;
    }

    return count;
}

#if FILAMENT_ENABLE_MATDBG

// Swaps in an edited version of the original package that was used to create the material. The
// edited package was stashed in response to a debugger event. This is invoked only when the
// Material Debugger is attached. The only editable features of a material package are the shader
// source strings, so here we trigger a rebuild of the HwProgram objects.
void FMaterial::applyPendingEdits() noexcept {
    const char* name = mName.c_str();
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
    mMaterialParser = std::move(mPendingEdits);
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
    std::unique_ptr<MaterialParser> pending = createParser(
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

[[nodiscard]] Handle<HwProgram> FMaterial::getProgramWithMATDBG(Variant const variant) const noexcept {
#if FILAMENT_ENABLE_MATDBG
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
    if (isSharedVariant(variant)) {
        FMaterial const* const pDefaultMaterial = mEngine.getDefaultMaterial();
        if (pDefaultMaterial && pDefaultMaterial->mCachedPrograms[variant.key]) {
            return pDefaultMaterial->getProgram(variant);
        }
    }
#endif
    assert_invariant(mCachedPrograms[variant.key]);
    return mCachedPrograms[variant.key];
}

void FMaterial::destroyPrograms(FEngine& engine,
        Variant::type_t const variantMask, Variant::type_t const variantValue) {

    DriverApi& driverApi = engine.getDriverApi();
    auto& cachedPrograms = mCachedPrograms;

    switch (mMaterialDomain) {
        case MaterialDomain::SURFACE: {
            if (mIsDefaultMaterial || mHasCustomDepthShader) {
                // default material, or we have custom depth shaders, we destroy all variants
                for (size_t k = 0, n = VARIANT_COUNT; k < n; ++k) {
                    if ((k & variantMask) == variantValue) {
                        // Only destroy if the handle is valid. Not strictly needed, but we have a lot
                        // of variants, and this generates traffic in the command queue.
                        if (cachedPrograms[k]) {
                            driverApi.destroyProgram(std::move(cachedPrograms[k]));
                        }
                    }
                }
            } else {
                // The depth variants may be shared with the default material, in which case
                // we should not free them now.

                // During Engine::shutdown(), auto-cleanup destroys the default material first,
                // so this can be null, but this is only used for debugging.
                UTILS_UNUSED_IN_RELEASE
                auto const UTILS_NULLABLE pDefaultMaterial = engine.getDefaultMaterial();

                for (size_t k = 0, n = VARIANT_COUNT; k < n; ++k) {
                    if ((k & variantMask) == variantValue) {
                        // Only destroy if the handle is valid. Not strictly needed, but we have a lot
                        // of variant, and this generates traffic in the command queue.
                        if (cachedPrograms[k]) {
                            if (Variant::isValidDepthVariant(Variant(k))) {
                                // By construction this should always be true, because this
                                // field is populated only when a material creates the program
                                // for this variant.
                                // During Engine::shutdown, auto-cleanup destroys the
                                // default material first
                                assert_invariant(!pDefaultMaterial ||
                                        pDefaultMaterial->mCachedPrograms[k]);
                                // we don't own this variant, skip, but clear the entry.
                                cachedPrograms[k].clear();
                                continue;
                            }

                            driverApi.destroyProgram(std::move(cachedPrograms[k]));
                        }
                    }
                }
            }
            break;
        }
        case MaterialDomain::POST_PROCESS: {
            for (size_t k = 0, n = POST_PROCESS_VARIANT_COUNT; k < n; ++k) {
                if ((k & variantMask) == variantValue) {
                    // Only destroy if the handle is valid. Not strictly needed, but we have a lot
                    // of variant, and this generates traffic in the command queue.
                    if (cachedPrograms[k]) {
                        driverApi.destroyProgram(std::move(cachedPrograms[k]));
                    }
                }
            }
            break;
        }
        case MaterialDomain::COMPUTE: {
            // Compute programs don't have variants
            driverApi.destroyProgram(std::move(cachedPrograms[0]));
            break;
        }
    }
}

std::optional<uint32_t> FMaterial::getSpecializationConstantId(std::string_view const name) const noexcept {
    auto const pos = mSpecializationConstantsNameToIndex.find(name);
    if (pos != mSpecializationConstantsNameToIndex.end()) {
        return pos->second + CONFIG_MAX_RESERVED_SPEC_CONSTANTS;
    }
    return std::nullopt;
}

template<typename T, typename>
bool FMaterial::setConstant(uint32_t id, T value) noexcept {
    size_t const maxId = mMaterialConstants.size() + CONFIG_MAX_RESERVED_SPEC_CONSTANTS;
    if (UTILS_LIKELY(id < maxId)) {
        if (id >= CONFIG_MAX_RESERVED_SPEC_CONSTANTS) {
            // Constant from the material itself (as opposed to the reserved ones)
            auto const& constant = mMaterialConstants[id - CONFIG_MAX_RESERVED_SPEC_CONSTANTS];
            using ConstantType = ConstantType;
            switch (constant.type) {
                case ConstantType::INT:
                    if (!std::is_same_v<T, int32_t>) return false;
                    break;
                case ConstantType::FLOAT:
                    if (!std::is_same_v<T, float>) return false;
                    break;
                case ConstantType::BOOL:
                    if (!std::is_same_v<T, bool>) return false;
                    break;
            }
        }

        auto pos = std::find_if(
                mSpecializationConstants.begin(), mSpecializationConstants.end(),
                [id](Program::SpecializationConstant const& specializationConstant) {
                    return specializationConstant.id == id;
                });
        if (pos != mSpecializationConstants.end()) {
            if (std::get<T>(pos->value) != value) {
                pos->value = value;
                return true;
            }
        } else {
            mSpecializationConstants.push_back({ id, value });
            return true;
        }
    }
    return false;
}

void FMaterial::processBlendingMode(MaterialParser const* const parser) {
    parser->getBlendingMode(&mBlendingMode);

    if (mBlendingMode == BlendingMode::MASKED) {
        parser->getMaskThreshold(&mMaskThreshold);
    }

    if (mBlendingMode == BlendingMode::CUSTOM) {
        parser->getCustomBlendFunction(&mCustomBlendFunctions);
    }

    // blending mode
    switch (mBlendingMode) {
        // Do not change the MASKED behavior without checking for regressions with
        // AlphaBlendModeTest and TextureLinearInterpolationTest, with and without
        // View::BlendMode::TRANSLUCENT.
        case BlendingMode::MASKED:
        case BlendingMode::OPAQUE:
            mRasterState.blendFunctionSrcRGB   = BlendFunction::ONE;
            mRasterState.blendFunctionSrcAlpha = BlendFunction::ONE;
            mRasterState.blendFunctionDstRGB   = BlendFunction::ZERO;
            mRasterState.blendFunctionDstAlpha = BlendFunction::ZERO;
            mRasterState.depthWrite = true;
            break;
        case BlendingMode::TRANSPARENT:
        case BlendingMode::FADE:
            mRasterState.blendFunctionSrcRGB   = BlendFunction::ONE;
            mRasterState.blendFunctionSrcAlpha = BlendFunction::ONE;
            mRasterState.blendFunctionDstRGB   = BlendFunction::ONE_MINUS_SRC_ALPHA;
            mRasterState.blendFunctionDstAlpha = BlendFunction::ONE_MINUS_SRC_ALPHA;
            mRasterState.depthWrite = false;
            break;
        case BlendingMode::ADD:
            mRasterState.blendFunctionSrcRGB   = BlendFunction::ONE;
            mRasterState.blendFunctionSrcAlpha = BlendFunction::ONE;
            mRasterState.blendFunctionDstRGB   = BlendFunction::ONE;
            mRasterState.blendFunctionDstAlpha = BlendFunction::ONE;
            mRasterState.depthWrite = false;
            break;
        case BlendingMode::MULTIPLY:
            mRasterState.blendFunctionSrcRGB   = BlendFunction::ZERO;
            mRasterState.blendFunctionSrcAlpha = BlendFunction::ZERO;
            mRasterState.blendFunctionDstRGB   = BlendFunction::SRC_COLOR;
            mRasterState.blendFunctionDstAlpha = BlendFunction::SRC_COLOR;
            mRasterState.depthWrite = false;
            break;
        case BlendingMode::SCREEN:
            mRasterState.blendFunctionSrcRGB   = BlendFunction::ONE;
            mRasterState.blendFunctionSrcAlpha = BlendFunction::ONE;
            mRasterState.blendFunctionDstRGB   = BlendFunction::ONE_MINUS_SRC_COLOR;
            mRasterState.blendFunctionDstAlpha = BlendFunction::ONE_MINUS_SRC_COLOR;
            mRasterState.depthWrite = false;
            break;
        case BlendingMode::CUSTOM:
            mRasterState.blendFunctionSrcRGB   = mCustomBlendFunctions[0];
            mRasterState.blendFunctionSrcAlpha = mCustomBlendFunctions[1];
            mRasterState.blendFunctionDstRGB   = mCustomBlendFunctions[2];
            mRasterState.blendFunctionDstAlpha = mCustomBlendFunctions[3];
            mRasterState.depthWrite = false;
    }

    // depth write
    bool depthWriteSet = false;
    parser->getDepthWriteSet(&depthWriteSet);
    if (depthWriteSet) {
        bool depthWrite = false;
        parser->getDepthWrite(&depthWrite);
        mRasterState.depthWrite = depthWrite;
    }

    // alpha to coverage
    bool alphaToCoverageSet = false;
    parser->getAlphaToCoverageSet(&alphaToCoverageSet);
    if (alphaToCoverageSet) {
        bool alphaToCoverage = false;
        parser->getAlphaToCoverage(&alphaToCoverage);
        mRasterState.alphaToCoverage = alphaToCoverage;
    } else {
        mRasterState.alphaToCoverage = mBlendingMode == BlendingMode::MASKED;
    }
}

void FMaterial::processSpecializationConstants(FEngine& engine, Builder const& builder,
        MaterialParser const* const parser) {
    // Older materials won't have a constants chunk, but that's okay.
    parser->getConstants(&mMaterialConstants);
    for (size_t i = 0, c = mMaterialConstants.size(); i < c; i++) {
        auto& item = mMaterialConstants[i];
        // the key can be a string_view because mMaterialConstant owns the CString
        std::string_view const key{ item.name.data(), item.name.size() };
        mSpecializationConstantsNameToIndex[key] = i;
    }

    // Verify that all the constant specializations exist in the material and that their types match.
    // The first specialization constants are defined internally by Filament.
    // The subsequent constants are user-defined in the material.

    // Feature level 0 doesn't support instancing
    int const maxInstanceCount = (engine.getActiveFeatureLevel() == FeatureLevel::FEATURE_LEVEL_0)
                                 ? 1 : CONFIG_MAX_INSTANCES;

    // The 16u below denotes the 16 bytes in a uvec4, which is how the froxel buffer is stored.
    int const maxFroxelBufferHeight =
            int(Froxelizer::getFroxelBufferByteCount(engine.getDriverApi()) / 16u);

    bool const staticTextureWorkaround =
            engine.getDriverApi().isWorkaroundNeeded(Workaround::METAL_STATIC_TEXTURE_TARGET_ERROR);

    bool const powerVrShaderWorkarounds =
            engine.getDriverApi().isWorkaroundNeeded(Workaround::POWER_VR_SHADER_WORKAROUNDS);

    mSpecializationConstants.reserve(mMaterialConstants.size() + CONFIG_MAX_RESERVED_SPEC_CONSTANTS);
    mSpecializationConstants.push_back({
            +ReservedSpecializationConstants::BACKEND_FEATURE_LEVEL,
            int(engine.getSupportedFeatureLevel()) });
    mSpecializationConstants.push_back({
            +ReservedSpecializationConstants::CONFIG_MAX_INSTANCES,
            int(maxInstanceCount) });
    mSpecializationConstants.push_back({
            +ReservedSpecializationConstants::CONFIG_FROXEL_BUFFER_HEIGHT,
            int(maxFroxelBufferHeight) });
    mSpecializationConstants.push_back({
            +ReservedSpecializationConstants::CONFIG_DEBUG_DIRECTIONAL_SHADOWMAP,
            engine.debug.shadowmap.debug_directional_shadowmap });
    mSpecializationConstants.push_back({
            +ReservedSpecializationConstants::CONFIG_DEBUG_FROXEL_VISUALIZATION,
            engine.debug.lighting.debug_froxel_visualization });
    mSpecializationConstants.push_back({
            +ReservedSpecializationConstants::CONFIG_STATIC_TEXTURE_TARGET_WORKAROUND,
            staticTextureWorkaround });
    mSpecializationConstants.push_back({
            +ReservedSpecializationConstants::CONFIG_POWER_VR_SHADER_WORKAROUNDS,
            powerVrShaderWorkarounds });
    mSpecializationConstants.push_back({
            +ReservedSpecializationConstants::CONFIG_STEREO_EYE_COUNT,
            int(engine.getConfig().stereoscopicEyeCount) });
    mSpecializationConstants.push_back({
            +ReservedSpecializationConstants::CONFIG_SH_BANDS_COUNT,
            builder->mShBandsCount });
    mSpecializationConstants.push_back({
            +ReservedSpecializationConstants::CONFIG_SHADOW_SAMPLING_METHOD,
            int32_t(builder->mShadowSamplingQuality) });
    if (UTILS_UNLIKELY(parser->getShaderLanguage() == ShaderLanguage::ESSL1)) {
        // The actual value of this spec-constant is set in the OpenGLDriver backend.
        mSpecializationConstants.push_back({
                +ReservedSpecializationConstants::CONFIG_SRGB_SWAPCHAIN_EMULATION,
                false});
    }

    for (auto const& [name, value] : builder->mConstantSpecializations) {
        std::string_view const key{ name.data(), name.size() };
        auto pos = mSpecializationConstantsNameToIndex.find(key);
        FILAMENT_CHECK_PRECONDITION(pos != mSpecializationConstantsNameToIndex.end())
                << "The material " << mName.c_str_safe()
                << " does not have a constant parameter named " << name.c_str() << ".";
        constexpr char const* const types[3] = {"an int", "a float", "a bool"};
        auto const& constant = mMaterialConstants[pos->second];
        switch (constant.type) {
            case ConstantType::INT:
                FILAMENT_CHECK_PRECONDITION(std::holds_alternative<int32_t>(value))
                        << "The constant parameter " << name.c_str() << " on material "
                        << mName.c_str_safe() << " is of type int, but " << types[value.index()]
                        << " was provided.";
                break;
            case ConstantType::FLOAT:
                FILAMENT_CHECK_PRECONDITION(std::holds_alternative<float>(value))
                        << "The constant parameter " << name.c_str() << " on material "
                        << mName.c_str_safe() << " is of type float, but " << types[value.index()]
                        << " was provided.";
                break;
            case ConstantType::BOOL:
                FILAMENT_CHECK_PRECONDITION(std::holds_alternative<bool>(value))
                        << "The constant parameter " << name.c_str() << " on material "
                        << mName.c_str_safe() << " is of type bool, but " << types[value.index()]
                        << " was provided.";
                break;
        }
        uint32_t const index = pos->second + CONFIG_MAX_RESERVED_SPEC_CONSTANTS;
        mSpecializationConstants.push_back({ index, value });
    }
}

void FMaterial::processPushConstants(FEngine&, MaterialParser const* parser) {
    FixedCapacityVector<Program::PushConstant>& vertexConstants =
            mPushConstants[uint8_t(ShaderStage::VERTEX)];
    FixedCapacityVector<Program::PushConstant>& fragmentConstants =
            mPushConstants[uint8_t(ShaderStage::FRAGMENT)];

    CString structVarName;
    FixedCapacityVector<MaterialPushConstant> pushConstants;
    parser->getPushConstants(&structVarName, &pushConstants);

    vertexConstants.reserve(pushConstants.size());
    fragmentConstants.reserve(pushConstants.size());

    constexpr size_t MAX_NAME_LEN = 60;
    char buf[MAX_NAME_LEN];
    uint8_t vertexCount = 0, fragmentCount = 0;

    std::for_each(pushConstants.cbegin(), pushConstants.cend(),
            [&](MaterialPushConstant const& constant) {
                snprintf(buf, sizeof(buf), "%s.%s", structVarName.c_str(), constant.name.c_str());

                switch (constant.stage) {
                    case ShaderStage::VERTEX:
                        vertexConstants.push_back({CString(buf), constant.type});
                        vertexCount++;
                        break;
                    case ShaderStage::FRAGMENT:
                        fragmentConstants.push_back({CString(buf), constant.type});
                        fragmentCount++;
                        break;
                    case ShaderStage::COMPUTE:
                        break;
                }
            });
}

void FMaterial::precacheDepthVariants(FEngine& engine) {

    bool const disableDepthPrecacheForDefaultMaterial = engine.getDriverApi().isWorkaroundNeeded(
                               Workaround::DISABLE_DEPTH_PRECACHE_FOR_DEFAULT_MATERIAL);

    // pre-cache all depth variants inside the default material. Note that this should be
    // entirely optional; if we remove this pre-caching, these variants will be populated
    // later, when/if needed by createAndCacheProgram(). Doing it now potentially uses more
    // memory and increases init time, but reduces hiccups during the first frame.
    if (UTILS_UNLIKELY(mIsDefaultMaterial && !disableDepthPrecacheForDefaultMaterial)) {
        const bool stereoSupported = mEngine.getDriverApi().isStereoSupported();
        auto const allDepthVariants = VariantUtils::getDepthVariants();
        for (auto const variant: allDepthVariants) {
            // Don't precache any stereo variants if stereo is not supported.
            if (!stereoSupported && Variant::isStereoVariant(variant)) {
                continue;
            }
            assert_invariant(Variant::isValidDepthVariant(variant));
            if (hasVariant(variant)) {
                prepareProgram(variant);
            }
        }
        return;
    }

    // if possible pre-cache all depth variants from the default material
    if (mMaterialDomain == MaterialDomain::SURFACE &&
            !mIsDefaultMaterial &&
            !mHasCustomDepthShader) {
        FMaterial const* const pDefaultMaterial = engine.getDefaultMaterial();
        assert_invariant(pDefaultMaterial);
        auto const allDepthVariants = VariantUtils::getDepthVariants();
        for (auto const variant: allDepthVariants) {
            assert_invariant(Variant::isValidDepthVariant(variant));
            mCachedPrograms[variant.key] = pDefaultMaterial->mCachedPrograms[variant.key];
        }
    }
}

void FMaterial::processDescriptorSets(FEngine& engine, MaterialParser const* const parser) {
    UTILS_UNUSED_IN_RELEASE bool success;

    success = parser->getDescriptorBindings(&mProgramDescriptorBindings);
    assert_invariant(success);

    backend::DescriptorSetLayout descriptorSetLayout;
    success = parser->getDescriptorSetLayout(&descriptorSetLayout);
    assert_invariant(success);

    // get the PER_VIEW descriptor binding info
    bool const isLit = mIsVariantLit || mHasShadowMultiplier;
    bool const isSSR = mReflectionMode == ReflectionMode::SCREEN_SPACE ||
            mRefractionMode == RefractionMode::SCREEN_SPACE;
    bool const hasFog = !(mVariantFilterMask & UserVariantFilterMask(UserVariantFilterBit::FOG));

    auto perViewDescriptorSetLayout = descriptor_sets::getPerViewDescriptorSetLayout(
            mMaterialDomain, isLit, isSSR, hasFog, false);

    auto perViewDescriptorSetLayoutVsm = descriptor_sets::getPerViewDescriptorSetLayout(
            mMaterialDomain, isLit, isSSR, hasFog, true);

    // set the labels
    descriptorSetLayout.label = CString{ mName }.append("_perMat");
    perViewDescriptorSetLayout.label = CString{ mName }.append("_perView");
    perViewDescriptorSetLayoutVsm.label = CString{ mName }.append("_perViewVsm");

    // get the PER_RENDERABLE and PER_VIEW descriptor binding info
    for (auto&& [bindingPoint, descriptorSetLayout] : {
            std::pair{ DescriptorSetBindingPoints::PER_RENDERABLE,
                    descriptor_sets::getPerRenderableLayout() },
            std::pair{ DescriptorSetBindingPoints::PER_VIEW,
                    perViewDescriptorSetLayout }}) {
        Program::DescriptorBindingsInfo& descriptors = mProgramDescriptorBindings[+bindingPoint];
        descriptors.reserve(descriptorSetLayout.bindings.size());
        for (auto const& entry: descriptorSetLayout.bindings) {
            auto const& name = descriptor_sets::getDescriptorName(bindingPoint, entry.binding);
            descriptors.push_back({ name, entry.type, entry.binding });
        }
    }

    mDescriptorSetLayout = {
            engine.getDescriptorSetLayoutFactory(),
            engine.getDriverApi(), std::move(descriptorSetLayout) };

    mPerViewDescriptorSetLayout = {
            engine.getDescriptorSetLayoutFactory(),
            engine.getDriverApi(), std::move(perViewDescriptorSetLayout) };

    mPerViewDescriptorSetLayoutVsm = {
            engine.getDescriptorSetLayoutFactory(),
            engine.getDriverApi(), std::move(perViewDescriptorSetLayoutVsm) };
}

descriptor_binding_t FMaterial::getSamplerBinding(
        std::string_view const& name) const {
    return mSamplerInterfaceBlock.getSamplerInfo(name)->binding;
}

template bool FMaterial::setConstant<int32_t>(uint32_t id, int32_t value) noexcept;
template bool FMaterial::setConstant<float>(uint32_t id, float value) noexcept;
template bool FMaterial::setConstant<bool>(uint32_t id, bool value) noexcept;

} // namespace filament
