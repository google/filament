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
#include "driver/Program.h"

#include "FilamentAPI-impl.h"

#include <private/filament/SibGenerator.h>
#include <private/filament/UibGenerator.h>
#include <private/filament/Variant.h>

#include <private/filament/SamplerInterfaceBlock.h>
#include <private/filament/UniformInterfaceBlock.h>

#include <filaflat/MaterialParser.h>

#include <utils/Panic.h>

#include <sstream>

using namespace utils;
using namespace filaflat;

namespace filament {

using namespace details;
using namespace driver;


struct Material::BuilderDetails {
    const void* mPayload = nullptr;
    size_t mSize = 0;
    filaflat::MaterialParser* mMaterialParser = nullptr;
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
    MaterialParser* materialParser = new MaterialParser(
            upcast(engine).getBackend(), mImpl->mPayload, mImpl->mSize);
    bool materialOK = materialParser->parse() && materialParser->isShadingMaterial();
    if (!ASSERT_POSTCONDITION_NON_FATAL(materialOK, "could not parse the material package")) {
        return nullptr;
    }

    assert(upcast(engine).getBackend() != Backend::DEFAULT && "Default backend has not been resolved.");

    uint32_t v;
    materialParser->getShaderModels(&v);
    utils::bitset32 shaderModels;
    shaderModels.setValue(v);

    uint32_t sm = static_cast<uint32_t>(upcast(engine).getDriver().getShaderModel());
    CString name;
    materialParser->getName(&name);
    if (!ASSERT_POSTCONDITION_NON_FATAL(shaderModels.test(sm),
            "the material '%s' does not contain shaders compatible with this platform; "
            "need shader model %d but have 0x%02x", name.c_str_safe(), sm,
            shaderModels.getValue())) {
        return nullptr;
    }

    mImpl->mMaterialParser = materialParser;

    return upcast(engine).createMaterial(*this);
}

namespace details {

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

    // Sampler bindings are only required for Vulkan.
    UTILS_UNUSED_IN_RELEASE bool sbOK = parser->getSamplerBindingMap(&mSamplerBindings);
    assert(engine.getBackend() == Backend::OPENGL || sbOK);

    parser->getShading(&mShading);
    parser->getBlendingMode(&mBlendingMode);
    parser->getInterpolation(&mInterpolation);
    parser->getVertexDomain(&mVertexDomain);
    parser->getRequiredAttributes(&mRequiredAttributes);
    if (mBlendingMode == BlendingMode::MASKED) {
        parser->getMaskThreshold(&mMaskThreshold);
    }
    if (mShading == Shading::UNLIT) {
        parser->hasShadowMultiplier(&mHasShadowMultiplier);
    }
    mIsVariantLit = mShading != Shading::UNLIT || mHasShadowMultiplier;

    // create raster state
    using BlendFunction = Driver::RasterState::BlendFunction;
    using DepthFunc = Driver::RasterState::DepthFunc;
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
        // double sided disables face culling
        if (mDoubleSided) {
            mRasterState.culling = CullingMode::NONE;
        } else {
            // the cull mode is double sided but we set the double-sided bit to false,
            // revert culling to default
            if (mCullingMode == CullingMode::NONE) {
                mRasterState.culling = CullingMode::BACK;
            } else {
                // use the front/back/front&back mode set by the user
                mRasterState.culling = mCullingMode;
            }
        }
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

    // we can only initialize the default instance once we're initialized ourselves
    mDefaultInstance.initDefaultInstance(engine, this);
}

FMaterial::~FMaterial() noexcept {
    delete mMaterialParser;
}

void FMaterial::terminate(FEngine& engine) {
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

Handle<HwProgram> FMaterial::getProgramSlow(uint8_t variantKey) const noexcept {
    const ShaderModel sm = mEngine.getDriver().getShaderModel();

    assert(!Variant::isReserved(variantKey));

    uint8_t vertexVariantKey = Variant::filterVariantVertex(variantKey);
    uint8_t fragmentVariantKey = Variant::filterVariantFragment(variantKey);

    /*
     * Vertex shader
     */

    filaflat::ShaderBuilder& vsBuilder = mEngine.getVertexShaderBuilder();

    UTILS_UNUSED_IN_RELEASE bool vsOK = mMaterialParser->getShader(sm,
            vertexVariantKey, ShaderType::VERTEX, vsBuilder);

    ASSERT_POSTCONDITION(vsOK && vsBuilder.size() > 0,
            "The material '%s' has not been compiled to include the required "
            "GLSL or SPIR-V chunks for the vertex shader (variant=0x%x, filtered=0x%x).",
            mName.c_str(), variantKey, vertexVariantKey);

    CString vs(vsBuilder.getShader(), (CString::size_type) vsBuilder.size());

    /*
     * Fragment shader
     */

    filaflat::ShaderBuilder& fsBuilder = mEngine.getFragmentShaderBuilder();

    UTILS_UNUSED_IN_RELEASE bool fsOK = mMaterialParser->getShader(sm,
            fragmentVariantKey, ShaderType::FRAGMENT, fsBuilder);

    ASSERT_POSTCONDITION(fsOK && fsBuilder.size() > 0,
            "The material '%s' has not been compiled to include the required "
            "GLSL or SPIR-V chunks for the fragment shader (variant=0x%x, filterer=0x%x).",
            mName.c_str(), variantKey, fragmentVariantKey);
    CString fs(fsBuilder.getShader(), (CString::size_type) fsBuilder.size());

    Program pb;
    pb      .diagnostics(mName, variantKey)
            .withVertexShader(std::move(vs))
            .withFragmentShader(std::move(fs))
            .withSamplerBindings(&mSamplerBindings)
            .addUniformBlock(BindingPoints::PER_VIEW, &UibGenerator::getPerViewUib())
            .addUniformBlock(BindingPoints::LIGHTS, &UibGenerator::getLightsUib())
            .addUniformBlock(BindingPoints::PER_RENDERABLE, &UibGenerator::getPerRenderableUib())
            .addUniformBlock(BindingPoints::PER_MATERIAL_INSTANCE, &mUniformInterfaceBlock)
            .addSamplerBlock(BindingPoints::PER_VIEW, &SibGenerator::getPerViewSib())
            .addSamplerBlock(BindingPoints::PER_MATERIAL_INSTANCE, &mSamplerInterfaceBlock);

    if (Variant(variantKey).hasSkinning()) {
        pb.addUniformBlock(BindingPoints::PER_RENDERABLE_BONES, &UibGenerator::getPerRenderableBonesUib());
    }

    auto program = mEngine.getDriverApi().createProgram(std::move(pb));
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
