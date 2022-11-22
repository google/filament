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

#include "MaterialParser.h"

#include "details/Engine.h"

#include "FilamentAPI-impl.h"

#include <private/filament/Variant.h>

#include <private/filament/EngineEnums.h>
#include <private/filament/SamplerInterfaceBlock.h>
#include <private/filament/BufferInterfaceBlock.h>

#include <backend/DriverEnums.h>
#include <backend/Program.h>

#include <utils/CString.h>
#include <utils/Panic.h>

namespace filament {

using namespace backend;
using namespace filaflat;
using namespace utils;

static MaterialParser* createParser(Backend backend, const void* data, size_t size) {
    // unique_ptr so we don't leak MaterialParser on failures below
    auto materialParser = std::make_unique<MaterialParser>(backend, data, size);

    MaterialParser::ParseResult materialResult = materialParser->parse();

    if (backend == Backend::NOOP) {
        return materialParser.release();
    }

    ASSERT_PRECONDITION(materialResult != MaterialParser::ParseResult::ERROR_MISSING_BACKEND,
                "the material was not built for the %s backend\n", backendToString(backend));

    ASSERT_PRECONDITION(materialResult == MaterialParser::ParseResult::SUCCESS,
                "could not parse the material package");

    uint32_t version = 0;
    materialParser->getMaterialVersion(&version);
    ASSERT_PRECONDITION(version == MATERIAL_VERSION,
            "Material version mismatch. Expected %d but received %d.", MATERIAL_VERSION, version);

    assert_invariant(backend != Backend::DEFAULT && "Default backend has not been resolved.");

    return materialParser.release();
}

struct Material::BuilderDetails {
    const void* mPayload = nullptr;
    size_t mSize = 0;
    MaterialParser* mMaterialParser = nullptr;
    bool mDefaultMaterial = false;
};

FMaterial::DefaultMaterialBuilder::DefaultMaterialBuilder() : Material::Builder() {
    mImpl->mDefaultMaterial = true;
}

using BuilderType = Material;
BuilderType::Builder::Builder() noexcept = default;
BuilderType::Builder::~Builder() noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder&& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder&& rhs) noexcept = default;

Material::Builder& Material::Builder::package(const void* payload, size_t size) {
    mImpl->mPayload = payload;
    mImpl->mSize = size;
    return *this;
}

Material* Material::Builder::build(Engine& engine) {
    std::unique_ptr<MaterialParser> materialParser{ createParser(
            downcast(engine).getBackend(), mImpl->mPayload, mImpl->mSize) };

    if (materialParser == nullptr) {
        return nullptr;
    }

    uint32_t v = 0;
    materialParser->getShaderModels(&v);
    utils::bitset32 shaderModels;
    shaderModels.setValue(v);

    ShaderModel shaderModel = downcast(engine).getShaderModel();
    if (!shaderModels.test(static_cast<uint32_t>(shaderModel))) {
        CString name;
        materialParser->getName(&name);
        slog.e << "The material '" << name.c_str_safe() << "' was not built for ";
        switch (shaderModel) {
            case ShaderModel::MOBILE:
                slog.e << "mobile.\n";
                break;
            case ShaderModel::DESKTOP:
                slog.e << "desktop.\n";
                break;
        }
        slog.e << "Compiled material contains shader models 0x"
                << io::hex << shaderModels.getValue() << io::dec << "." << io::endl;
        return nullptr;
    }

    mImpl->mMaterialParser = materialParser.release();

    return downcast(engine).createMaterial(*this);
}

FMaterial::FMaterial(FEngine& engine, const Material::Builder& builder)
        : mEngine(engine),
          mMaterialId(engine.getMaterialId())
{
    MaterialParser* parser = builder->mMaterialParser;
    mMaterialParser = parser;

    UTILS_UNUSED_IN_RELEASE bool nameOk = parser->getName(&mName);
    assert_invariant(nameOk);

    uint8_t featureLevel = 1;
    parser->getFeatureLevel(&featureLevel);
    assert_invariant(featureLevel <= 3);
    mFeatureLevel = [featureLevel]() -> FeatureLevel {
        switch (featureLevel) {
            default:
            case 1: return FeatureLevel::FEATURE_LEVEL_1;
            case 2: return FeatureLevel::FEATURE_LEVEL_2;
            case 3: return FeatureLevel::FEATURE_LEVEL_3;
        }
    }();

    UTILS_UNUSED_IN_RELEASE bool success;

    success = parser->getSIB(&mSamplerInterfaceBlock);
    assert_invariant(success);

    success = parser->getUIB(&mUniformInterfaceBlock);
    assert_invariant(success);

    // read the uniform binding list
    utils::FixedCapacityVector<std::pair<utils::CString, uint8_t>> uniformBlockBindings;
    success = parser->getUniformBlockBindings(&mUniformBlockBindings);
    assert_invariant(success || mFeatureLevel >= FeatureLevel::FEATURE_LEVEL_2);

    success = parser->getSamplerBlockBindings(
            &mSamplerGroupBindingInfoList, &mSamplerBindingToNameMap);
    assert_invariant(success);

#if FILAMENT_ENABLE_MATDBG
    // Register the material with matdbg.
    matdbg::DebugServer* server = downcast(engine).debug.server;
    if (UTILS_UNLIKELY(server)) {
        auto details = builder.mImpl;
        mDebuggerId = server->addMaterial(mName, details->mPayload, details->mSize, this);
    }
#endif

    // Older materials will not have a subpass chunk; this should not be an error.
    if (!parser->getSubpasses(&mSubpassInfo)) {
        mSubpassInfo.isValid = false;
    }

    parser->getShading(&mShading);
    parser->getMaterialProperties(&mMaterialProperties);
    parser->getBlendingMode(&mBlendingMode);
    parser->getInterpolation(&mInterpolation);
    parser->getVertexDomain(&mVertexDomain);
    parser->getMaterialDomain(&mMaterialDomain);
    parser->getRequiredAttributes(&mRequiredAttributes);
    parser->getRefractionMode(&mRefractionMode);
    parser->getRefractionType(&mRefractionType);
    parser->getReflectionMode(&mReflectionMode);

    if (mBlendingMode == BlendingMode::MASKED) {
        parser->getMaskThreshold(&mMaskThreshold);
    }

    // The fade blending mode only affects shading. For proper sorting we need to
    // treat this blending mode as a regular transparent blending operation.
    if (UTILS_UNLIKELY(mBlendingMode == BlendingMode::FADE)) {
        mRenderBlendingMode = BlendingMode::TRANSPARENT;
    } else {
        mRenderBlendingMode = mBlendingMode;
    }

    if (mShading == Shading::UNLIT) {
        parser->hasShadowMultiplier(&mHasShadowMultiplier);
    }

    mIsVariantLit = mShading != Shading::UNLIT || mHasShadowMultiplier;

    // create raster state
    using BlendFunction = RasterState::BlendFunction;
    using DepthFunc = RasterState::DepthFunc;
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
    }

    bool depthWriteSet = false;
    parser->getDepthWriteSet(&depthWriteSet);
    if (depthWriteSet) {
        bool depthWrite = false;
        parser->getDepthWrite(&depthWrite);
        mRasterState.depthWrite = depthWrite;
    }

    // if doubleSided() was called we override culling()
    bool doubleSideSet = false;
    parser->getDoubleSidedSet(&doubleSideSet);
    parser->getDoubleSided(&mDoubleSided);
    parser->getCullingMode(&mCullingMode);
    bool depthTest = false;
    parser->getDepthTest(&depthTest);

    if (doubleSideSet) {
        mDoubleSidedCapability = true;
        mRasterState.culling = mDoubleSided ? CullingMode::NONE : mCullingMode;
    } else {
        mRasterState.culling = mCullingMode;
    }

    parser->getTransparencyMode(&mTransparencyMode);
    parser->hasCustomDepthShader(&mHasCustomDepthShader);
    mIsDefaultMaterial = builder->mDefaultMaterial;

    // pre-cache the shared variants -- these variants are shared with the default material.
    if (UTILS_UNLIKELY(!mIsDefaultMaterial && !mHasCustomDepthShader)) {
        auto& cachedPrograms = mCachedPrograms;
        for (Variant::type_t k = 0, n = VARIANT_COUNT; k < n; ++k) {
            const Variant variant(k);
            if (Variant::isValidDepthVariant(variant)) {
                FMaterial const* const pMaterial = engine.getDefaultMaterial();
                pMaterial->prepareProgram(variant);
                cachedPrograms[k] = pMaterial->getProgram(variant);
            }
        }
    }

    bool colorWrite = false;
    parser->getColorWrite(&colorWrite);
    mRasterState.colorWrite = colorWrite;
    mRasterState.depthFunc = depthTest ? DepthFunc::GE : DepthFunc::A;
    mRasterState.alphaToCoverage = mBlendingMode == BlendingMode::MASKED;

    parser->hasSpecularAntiAliasing(&mSpecularAntiAliasing);
    if (mSpecularAntiAliasing) {
        parser->getSpecularAntiAliasingVariance(&mSpecularAntiAliasingVariance);
        parser->getSpecularAntiAliasingThreshold(&mSpecularAntiAliasingThreshold);
    }

    // we can only initialize the default instance once we're initialized ourselves
    mDefaultInstance.initDefaultInstance(engine, this);
}

FMaterial::~FMaterial() noexcept {
    delete mMaterialParser;
}

void FMaterial::terminate(FEngine& engine) {

#if FILAMENT_ENABLE_MATDBG
    // Unregister the material with matdbg.
    matdbg::DebugServer* server = downcast(mEngine).debug.server;
    if (UTILS_UNLIKELY(server)) {
        server->removeMaterial(mDebuggerId);
    }
#endif

    destroyPrograms(engine);
    mDefaultInstance.terminate(engine);
}

FMaterialInstance* FMaterial::createInstance(const char* name) const noexcept {
    return FMaterialInstance::duplicate(&mDefaultInstance, name);
}

bool FMaterial::hasParameter(const char* name) const noexcept {
    return mUniformInterfaceBlock.hasField(name) ||
           mSamplerInterfaceBlock.hasSampler(name) ||
            mSubpassInfo.name == utils::CString(name);
}

bool FMaterial::isSampler(const char* name) const noexcept {
    return mSamplerInterfaceBlock.hasSampler(name);
}

BufferInterfaceBlock::FieldInfo const* FMaterial::reflect(
        std::string_view name) const noexcept {
    return mUniformInterfaceBlock.getFieldInfo(name);
}

void FMaterial::prepareProgramSlow(Variant variant) const noexcept {
    assert_invariant(mEngine.hasFeatureLevel(mFeatureLevel));
    switch (getMaterialDomain()) {
        case MaterialDomain::SURFACE:
            getSurfaceProgramSlow(variant);
            break;
        case MaterialDomain::POST_PROCESS:
            getPostProcessProgramSlow(variant);
            break;
        case MaterialDomain::COMPUTE:
            // TODO: implement MaterialDomain::COMPUTE
            break;
    }
}

void FMaterial::getSurfaceProgramSlow(Variant variant) const noexcept {
    // filterVariant() has already been applied in generateCommands(), shouldn't be needed here
    // if we're unlit, we don't have any bits that correspond to lit materials
    assert_invariant(variant == Variant::filterVariant(variant, isVariantLit()) );

    assert_invariant(!Variant::isReserved(variant));

    Variant vertexVariant   = Variant::filterVariantVertex(variant);
    Variant fragmentVariant = Variant::filterVariantFragment(variant);

    Program pb{ getProgramBuilderWithVariants(variant, vertexVariant, fragmentVariant) };
    createAndCacheProgram(std::move(pb), variant);
}

void FMaterial::getPostProcessProgramSlow(Variant variant) const noexcept {
    Program pb{ getProgramBuilderWithVariants(variant, variant, variant) };
    createAndCacheProgram(std::move(pb), variant);
}

Program FMaterial::getProgramBuilderWithVariants(
        Variant variant,
        Variant vertexVariant,
        Variant fragmentVariant) const noexcept {
    const ShaderModel sm = mEngine.getShaderModel();
    const bool isNoop = mEngine.getBackend() == Backend::NOOP;

    /*
     * Vertex shader
     */

    ShaderContent& vsBuilder = mEngine.getVertexShaderContent();

    UTILS_UNUSED_IN_RELEASE bool vsOK = mMaterialParser->getShader(vsBuilder, sm,
            vertexVariant, ShaderStage::VERTEX);

    ASSERT_POSTCONDITION(isNoop || (vsOK && !vsBuilder.empty()),
            "The material '%s' has not been compiled to include the required "
            "GLSL or SPIR-V chunks for the vertex shader (variant=0x%x, filtered=0x%x).",
            mName.c_str(), variant.key, vertexVariant.key);

    /*
     * Fragment shader
     */

    ShaderContent& fsBuilder = mEngine.getFragmentShaderContent();

    UTILS_UNUSED_IN_RELEASE bool fsOK = mMaterialParser->getShader(fsBuilder, sm,
            fragmentVariant, ShaderStage::FRAGMENT);

    ASSERT_POSTCONDITION(isNoop || (fsOK && !fsBuilder.empty()),
            "The material '%s' has not been compiled to include the required "
            "GLSL or SPIR-V chunks for the fragment shader (variant=0x%x, filtered=0x%x).",
            mName.c_str(), variant.key, fragmentVariant.key);

    Program program;
    program.shader(ShaderStage::VERTEX, vsBuilder.data(), vsBuilder.size())
           .shader(ShaderStage::FRAGMENT, fsBuilder.data(), fsBuilder.size())
           .uniformBlockBindings(mUniformBlockBindings)
           .diagnostics(mName,
                    [this, variant](io::ostream& out) -> io::ostream& {
                        return out << mName.c_str_safe()
                                   << ", variant=(" << io::hex << variant.key << io::dec << ")";
                    });

    UTILS_NOUNROLL
    for (size_t i = 0; i < Enum::count<SamplerBindingPoints>(); i++) {
        SamplerBindingPoints bindingPoint = (SamplerBindingPoints)i;
        auto const& info = mSamplerGroupBindingInfoList[i];
        if (info.count) {
            std::array<Program::Sampler, backend::MAX_SAMPLER_COUNT> samplers{};
            for (size_t j = 0, c = info.count; j < c; ++j) {
                uint8_t binding = info.bindingOffset + j;
                samplers[j] = { mSamplerBindingToNameMap[binding], binding };
            }
            program.setSamplerGroup(+bindingPoint, info.shaderStageFlags,
                    samplers.data(), info.count);
        }
    }

    program.specializationConstants({
            { 0, (int)mEngine.getSupportedFeatureLevel() },
            { 1, (int)CONFIG_MAX_INSTANCES }
    });

    return program;
}

void FMaterial::createAndCacheProgram(Program&& p, Variant variant) const noexcept {
    auto program = mEngine.getDriverApi().createProgram(std::move(p));
    assert_invariant(program);
    mCachedPrograms[variant.key] = program;
}

size_t FMaterial::getParameters(ParameterInfo* parameters, size_t count) const noexcept {
    count = std::min(count, getParameterCount());

    const auto& uniforms = mUniformInterfaceBlock.getFieldInfoList();
    size_t i = 0;
    size_t uniformCount = std::min(count, size_t(uniforms.size()));
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
    size_t samplerCount = samplers.size();
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
        i++;
    }

    return count;
}

// Swaps in an edited version of the original package that was used to create the material. The
// edited package was stashed in response to a debugger event. This is invoked only when the
// Material Debugger is attached. The only editable features of a material package are the shader
// source strings, so here we trigger a rebuild of the HwProgram objects.
void FMaterial::applyPendingEdits() noexcept {
    const char* name = mName.c_str();
    slog.d << "Applying edits to " << (name ? name : "(untitled)") << io::endl;
    destroyPrograms(mEngine);
    for (auto& program : mCachedPrograms) {
        program.clear();
    }
    delete mMaterialParser;
    mMaterialParser = mPendingEdits;
    mPendingEdits = nullptr;
}

/**
 * Callback handlers for the debug server, potentially called from any thread. These methods are
 * never called during normal operation and exist for debugging purposes only.
 *
 * @{
 */

#if FILAMENT_ENABLE_MATDBG

void FMaterial::onEditCallback(void* userdata, const utils::CString& name, const void* packageData,
        size_t packageSize) {
    FMaterial* material = downcast((Material*) userdata);
    FEngine& engine = material->mEngine;

    // This is called on a web server thread so we defer clearing the program cache
    // and swapping out the MaterialParser until the next getProgram call.
    material->mPendingEdits = createParser(engine.getBackend(), packageData, packageSize);
}

void FMaterial::onQueryCallback(void* userdata, VariantList* pVariants) {
    FMaterial* material = downcast((Material*) userdata);
    std::lock_guard<utils::Mutex> lock(material->mActiveProgramsLock);
    *pVariants = material->mActivePrograms;
    material->mActivePrograms.reset();
}

#endif

/** @}*/

void FMaterial::destroyPrograms(FEngine& engine) {
    DriverApi& driverApi = engine.getDriverApi();
    auto& cachedPrograms = mCachedPrograms;
    for (Variant::type_t k = 0, n = VARIANT_COUNT; k < n; ++k) {
        const Variant variant(k);
        if (!mIsDefaultMaterial) {
            // The depth variants may be shared with the default material, in which case
            // we should not free it now.
            bool isSharedVariant = Variant::isValidDepthVariant(variant) && !mHasCustomDepthShader;
            if (isSharedVariant) {
                // we don't own this variant, skip.
                continue;
            }
        }
        driverApi.destroyProgram(cachedPrograms[k]);
    }
}

} // namespace filament
