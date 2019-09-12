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
#include "details/DFG.h"

#include "private/backend/Program.h"

#include "FilamentAPI-impl.h"

#include <backend/DriverEnums.h>

#include <private/filament/SibGenerator.h>
#include <private/filament/UibGenerator.h>
#include <private/filament/Variant.h>

#include <private/filament/SamplerInterfaceBlock.h>
#include <private/filament/UniformInterfaceBlock.h>

#include <MaterialParser.h>

#include <utils/Panic.h>

#include <sstream>

using namespace utils;
using namespace filaflat;

namespace filament {

using namespace details;
using namespace backend;


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
    MaterialParser* materialParser = FMaterial::createParser(
            upcast(engine).getBackend(), mImpl->mPayload, mImpl->mSize);

    uint32_t v;
    materialParser->getShaderModels(&v);
    utils::bitset32 shaderModels;
    shaderModels.setValue(v);

    backend::ShaderModel shaderModel = upcast(engine).getDriver().getShaderModel();
    if (!shaderModels.test(static_cast<uint32_t>(shaderModel))) {
        CString name;
        materialParser->getName(&name);
        slog.e << "The material '" << name.c_str_safe() << "' was not built for ";
        switch (shaderModel) {
            case backend::ShaderModel::GL_ES_30: slog.e << "mobile.\n"; break;
            case backend::ShaderModel::GL_CORE_41: slog.e << "desktop.\n"; break;
            case backend::ShaderModel::UNKNOWN: /* should never happen */ break;
        }
        slog.e << "Compiled material contains shader models 0x"
                << io::hex << shaderModels.getValue() << io::dec << "." << io::endl;
        return nullptr;
    }

    mImpl->mMaterialParser = materialParser;

    Material* result = upcast(engine).createMaterial(*this);

#if FILAMENT_ENABLE_MATDBG
    matdbg::DebugServer* server = upcast(engine).debug.server;
    if (server) {
        CString name;
        materialParser->getName(&name);
        server->addMaterial(name, mImpl->mPayload, mImpl->mSize, result);
    }
#endif

    return result;
}

namespace details {

static void addSamplerGroup(Program& pb, uint8_t bindingPoint, SamplerInterfaceBlock const& sib,
        SamplerBindingMap const& map) {
    const size_t samplerCount = sib.getSize();
    if (samplerCount) {
        std::vector<Program::Sampler> samplers(samplerCount);
        auto const& list = sib.getSamplerInfoList();
        for (size_t i = 0, c = samplerCount; i < c; ++i) {
            CString uniformName(
                    SamplerInterfaceBlock::getUniformName(sib.getName().c_str(),
                            list[i].name.c_str()));
            uint8_t binding;
            UTILS_UNUSED bool ok = map.getSamplerBinding(bindingPoint, (uint8_t)i, &binding);
            assert(ok);
            samplers[i] = { std::move(uniformName), binding };
        }
        pb.setSamplerGroup(bindingPoint, samplers.data(), samplers.size());
    }
}

FMaterial::FMaterial(FEngine& engine, const Material::Builder& builder)
        : mEngine(engine),
          mMaterialId(engine.getMaterialId())
{
    MaterialParser* parser = builder->mMaterialParser;
    mMaterialParser = parser;

    UTILS_UNUSED_IN_RELEASE bool nameOk = parser->getName(&mName);
    assert(nameOk);

    UTILS_UNUSED_IN_RELEASE bool sibOK = parser->getSIB(&mSamplerInterfaceBlock);
    assert(sibOK);

    UTILS_UNUSED_IN_RELEASE bool uibOK = parser->getUIB(&mUniformInterfaceBlock);
    assert(uibOK);

    // Populate sampler bindings for the backend that will consume this Material.
    mSamplerBindings.populate(&mSamplerInterfaceBlock);

    parser->getShading(&mShading);
    parser->getBlendingMode(&mBlendingMode);
    parser->getInterpolation(&mInterpolation);
    parser->getVertexDomain(&mVertexDomain);
    parser->getMaterialDomain(&mMaterialDomain);
    parser->getRequiredAttributes(&mRequiredAttributes);

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
        case BlendingMode::OPAQUE:
        case BlendingMode::MASKED:
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

    bool depthWriteSet;
    parser->getDepthWriteSet(&depthWriteSet);
    if (depthWriteSet) {
        bool depthWrite;
        parser->getDepthWrite(&depthWrite);
        mRasterState.depthWrite = depthWrite;
    }

    // if doubleSided() was called we override culling()
    bool doubleSideSet;
    parser->getDoubleSidedSet(&doubleSideSet);
    parser->getDoubleSided(&mDoubleSided);
    parser->getCullingMode(&mCullingMode);
    bool depthTest;
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
        for (uint8_t i = 0, n = cachedPrograms.size(); i < n; ++i) {
            if (Variant(i).isDepthPass()) {
                cachedPrograms[i] = engine.getDefaultMaterial()->getProgram(i);
            }
        }
    }

    bool colorWrite;
    parser->getColorWrite(&colorWrite);
    mRasterState.colorWrite = colorWrite;
    mRasterState.depthFunc = depthTest ? DepthFunc::LE : DepthFunc::A;
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
    destroyPrograms(engine);
    mDefaultInstance.terminate(engine);
}

FMaterialInstance* FMaterial::createInstance() const noexcept {
    return mEngine.createMaterialInstance(this);
}

bool FMaterial::hasParameter(const char* name) const noexcept {
    if (!mUniformInterfaceBlock.hasUniform(name)) {
        return mSamplerInterfaceBlock.hasSampler(name);
    }
    return true;
}

backend::Handle<backend::HwProgram> FMaterial::getProgramSlow(uint8_t variantKey) const noexcept {
    switch (getMaterialDomain()) {
        case MaterialDomain::SURFACE:
            return getSurfaceProgramSlow(variantKey);

        case MaterialDomain::POST_PROCESS:
            return getPostProcessProgramSlow(variantKey);
    }
}

backend::Handle<backend::HwProgram> FMaterial::getSurfaceProgramSlow(uint8_t variantKey)
    const noexcept {
    // filterVariant() has already been applied in generateCommands(), shouldn't be needed here
    // if we're unlit, we don't have any bits that correspond to lit materials
    assert( variantKey == Variant::filterVariant(variantKey, isVariantLit()) );

    assert(!Variant::isReserved(variantKey));

    uint8_t vertexVariantKey = Variant::filterVariantVertex(variantKey);
    uint8_t fragmentVariantKey = Variant::filterVariantFragment(variantKey);

    Program pb = getProgramBuilderWithVariants(variantKey, vertexVariantKey, fragmentVariantKey);
    pb
        .setUniformBlock(BindingPoints::PER_VIEW, UibGenerator::getPerViewUib().getName())
        .setUniformBlock(BindingPoints::LIGHTS, UibGenerator::getLightsUib().getName())
        .setUniformBlock(BindingPoints::PER_RENDERABLE, UibGenerator::getPerRenderableUib().getName())
        .setUniformBlock(BindingPoints::PER_MATERIAL_INSTANCE, mUniformInterfaceBlock.getName());

    if (Variant(variantKey).hasSkinningOrMorphing()) {
        pb.setUniformBlock(BindingPoints::PER_RENDERABLE_BONES,
                UibGenerator::getPerRenderableBonesUib().getName());
    }

    addSamplerGroup(pb, BindingPoints::PER_VIEW, SibGenerator::getPerViewSib(), mSamplerBindings);
    addSamplerGroup(pb, BindingPoints::PER_MATERIAL_INSTANCE, mSamplerInterfaceBlock, mSamplerBindings);

    return createAndCacheProgram(std::move(pb), variantKey);
}

backend::Handle<backend::HwProgram> FMaterial::getPostProcessProgramSlow(uint8_t variantKey)
    const noexcept {

    Program pb = getProgramBuilderWithVariants(variantKey, variantKey, variantKey);
    pb
            .setUniformBlock(BindingPoints::PER_VIEW, UibGenerator::getPerViewUib().getName())
            .setUniformBlock(BindingPoints::PER_MATERIAL_INSTANCE, mUniformInterfaceBlock.getName());

    addSamplerGroup(pb, BindingPoints::PER_MATERIAL_INSTANCE, mSamplerInterfaceBlock, mSamplerBindings);

    return createAndCacheProgram(std::move(pb), variantKey);
}

Program FMaterial::getProgramBuilderWithVariants(
        uint8_t variantKey,
        uint8_t vertexVariantKey,
        uint8_t fragmentVariantKey) const noexcept {
    const ShaderModel sm = mEngine.getDriver().getShaderModel();

    /*
     * Vertex shader
     */

    filaflat::ShaderBuilder& vsBuilder = mEngine.getVertexShaderBuilder();

    UTILS_UNUSED_IN_RELEASE bool vsOK = mMaterialParser->getShader(vsBuilder, sm,
            vertexVariantKey, ShaderType::VERTEX);

    ASSERT_POSTCONDITION(vsOK && vsBuilder.size() > 0,
            "The material '%s' has not been compiled to include the required "
            "GLSL or SPIR-V chunks for the vertex shader (variant=0x%x, filtered=0x%x).",
            mName.c_str(), variantKey, vertexVariantKey);

    /*
     * Fragment shader
     */

    filaflat::ShaderBuilder& fsBuilder = mEngine.getFragmentShaderBuilder();

    UTILS_UNUSED_IN_RELEASE bool fsOK = mMaterialParser->getShader(fsBuilder, sm,
            fragmentVariantKey, ShaderType::FRAGMENT);

    ASSERT_POSTCONDITION(fsOK && fsBuilder.size() > 0,
            "The material '%s' has not been compiled to include the required "
            "GLSL or SPIR-V chunks for the fragment shader (variant=0x%x, filterer=0x%x).",
            mName.c_str(), variantKey, fragmentVariantKey);

    Program pb;
    pb      .diagnostics(mName, variantKey)
            .withVertexShader(vsBuilder.data(), vsBuilder.size())
            .withFragmentShader(fsBuilder.data(), fsBuilder.size());
    return pb;
}

backend::Handle<backend::HwProgram> FMaterial::createAndCacheProgram(Program&& p,
        uint8_t variantKey) const noexcept {
    auto program = mEngine.getDriverApi().createProgram(std::move(p));
    assert(program);

    mCachedPrograms[variantKey] = program;
    return program;
}

size_t FMaterial::getParameters(ParameterInfo* parameters, size_t count) const noexcept {
    count = std::min(count, getParameterCount());

    const auto& uniforms = mUniformInterfaceBlock.getUniformInfoList();
    size_t i = 0;
    size_t uniformCount = std::min(count, uniforms.size());
    for ( ; i < uniformCount; i++) {
        ParameterInfo& info = parameters[i];
        const auto& uniformInfo = uniforms[i];
        info.name = uniformInfo.name.c_str();
        info.isSampler = false;
        info.type = uniformInfo.type;
        info.count = uniformInfo.size;
        info.precision = uniformInfo.precision;
    }

    const auto& samplers = mSamplerInterfaceBlock.getSamplerInfoList();
    for (size_t j = 0; i < count; i++, j++) {
        ParameterInfo& info = parameters[i];
        const auto& samplerInfo = samplers[j];
        info.name = samplerInfo.name.c_str();
        info.isSampler = true;
        info.samplerType = samplerInfo.type;
        info.count = 1;
        info.precision = samplerInfo.precision;
    }

    return count;
}

// Swaps in an edited version of the original package that was used to create the material. The
// edited package was stashed in response to a debugger event. This is invoked only when the
// Material Debugger is attached. The only editable features of a material package are the shader
// source strings, so here we trigger a rebuild of the HwProgram objects.
void FMaterial::applyPendingEdits() noexcept {
    slog.d << "Applying edits to " << mName.c_str() << io::endl;
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

void FMaterial::onEditCallback(void* userdata, const utils::CString& name, const void* packageData,
        size_t packageSize) {
    FMaterial* material = upcast((Material*) userdata);
    FEngine& engine = material->mEngine;

    // This is called on a web server thread so we defer clearing the program cache
    // and swapping out the MaterialParser until the next getProgram call.
    material->mPendingEdits = FMaterial::createParser(engine.getBackend(), packageData,
            packageSize);
}

void FMaterial::onQueryCallback(void* userdata, uint16_t* pvariants) {
    FMaterial* material = upcast((Material*) userdata);
    uint16_t variants = 0;
    auto& cachedPrograms = material->mCachedPrograms;
    for (size_t i = 0, n = cachedPrograms.size(); i < n; ++i) {
        if (cachedPrograms[i]) {
            variants |= (1 << i);
        }
    }
    *pvariants = variants;
}

 /** @}*/
 
MaterialParser* FMaterial::createParser(backend::Backend backend, const void* data, size_t size) {
    MaterialParser* materialParser = new MaterialParser(backend, data, size);

    bool materialOK = materialParser->parse();
    if (!ASSERT_POSTCONDITION_NON_FATAL(materialOK, "could not parse the material package")) {
        return nullptr;
    }

    uint32_t version;
    materialParser->getMaterialVersion(&version);
    ASSERT_PRECONDITION(version == MATERIAL_VERSION, "Material version mismatch. Expected %d but "
            "received %d.", MATERIAL_VERSION, version);

    assert(backend != Backend::DEFAULT && "Default backend has not been resolved.");

    return materialParser;
}

void FMaterial::destroyPrograms(FEngine& engine) {
    DriverApi& driverApi = engine.getDriverApi();
    auto& cachedPrograms = mCachedPrograms;
    for (size_t i = 0, n = cachedPrograms.size(); i < n; ++i) {
        if (!mIsDefaultMaterial) {
            // The depth variants may be shared with the default material, in which case
            // we should not free it now.
            bool isSharedVariant = Variant(i).isDepthPass() && !mHasCustomDepthShader;
            if (isSharedVariant) {
                // we don't own this variant, skip.
                continue;
            }
        }
        driverApi.destroyProgram(cachedPrograms[i]);
    }
}

} // namespace details

// ------------------------------------------------------------------------------------------------
// Trampoline calling into private implementation
// ------------------------------------------------------------------------------------------------

using namespace details;

MaterialInstance* Material::createInstance() const noexcept {
    return upcast(this)->createInstance();
}

const char* Material::getName() const noexcept {
    return upcast(this)->getName().c_str();
}

Shading Material::getShading()  const noexcept {
    return upcast(this)->getShading();
}

Interpolation Material::getInterpolation() const noexcept {
    return upcast(this)->getInterpolation();
}

BlendingMode Material::getBlendingMode() const noexcept {
    return upcast(this)->getBlendingMode();
}

VertexDomain Material::getVertexDomain() const noexcept {
    return upcast(this)->getVertexDomain();
}

MaterialDomain Material::getMaterialDomain() const noexcept {
    return upcast(this)->getMaterialDomain();
}

CullingMode Material::getCullingMode() const noexcept {
    return upcast(this)->getCullingMode();
}

TransparencyMode Material::getTransparencyMode() const noexcept {
    return upcast(this)->getTransparencyMode();
}

bool Material::isColorWriteEnabled() const noexcept {
    return upcast(this)->isColorWriteEnabled();
}

bool Material::isDepthWriteEnabled() const noexcept {
    return upcast(this)->isDepthWriteEnabled();
}

bool Material::isDepthCullingEnabled() const noexcept {
    return upcast(this)->isDepthCullingEnabled();
}

bool Material::isDoubleSided() const noexcept {
    return upcast(this)->isDoubleSided();
}

float Material::getMaskThreshold() const noexcept {
    return upcast(this)->getMaskThreshold();
}

bool Material::hasShadowMultiplier() const noexcept {
    return upcast(this)->hasShadowMultiplier();
}

bool Material::hasSpecularAntiAliasing() const noexcept {
    return upcast(this)->hasSpecularAntiAliasing();
}

float Material::getSpecularAntiAliasingVariance() const noexcept {
    return upcast(this)->getSpecularAntiAliasingVariance();
}

float Material::getSpecularAntiAliasingThreshold() const noexcept {
    return upcast(this)->getSpecularAntiAliasingThreshold();
}

size_t Material::getParameterCount() const noexcept {
    return upcast(this)->getParameterCount();
}

size_t Material::getParameters(ParameterInfo* parameters, size_t count) const noexcept {
    return upcast(this)->getParameters(parameters, count);
}

AttributeBitset Material::getRequiredAttributes() const noexcept {
    return upcast(this)->getRequiredAttributes();
}

bool Material::hasParameter(const char* name) const noexcept {
    return upcast(this)->hasParameter(name);
}

MaterialInstance* Material::getDefaultInstance() noexcept {
    return upcast(this)->getDefaultInstance();
}

MaterialInstance const* Material::getDefaultInstance() const noexcept {
    return upcast(this)->getDefaultInstance();
}

} // namespace filament
