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

#include <filament/MaterialInstance.h>

#include <filament/TextureSampler.h>

#include "details/MaterialInstance.h"

#include "RenderPass.h"

#include "details/Engine.h"
#include "details/Material.h"
#include "details/Texture.h"

#include <utils/Log.h>

using namespace filament::math;
using namespace utils;

namespace filament {

using namespace backend;

FMaterialInstance::FMaterialInstance(FEngine& engine, FMaterial const* material) noexcept
        : mMaterial(material),
          mCulling(CullingMode::BACK),
          mDepthFunc(RasterState::DepthFunc::LE),
          mColorWrite(false),
          mDepthWrite(false),
          mHasScissor(false),
          mIsDoubleSided(false),
          mTransparencyMode(TransparencyMode::DEFAULT) {

    FEngine::DriverApi& driver = engine.getDriverApi();

    if (!material->getUniformInterfaceBlock().isEmpty()) {
        mUniforms = UniformBuffer(material->getUniformInterfaceBlock().getSize());
        mUbHandle = driver.createBufferObject(mUniforms.getSize(),
                BufferObjectBinding::UNIFORM, backend::BufferUsage::STATIC);
    }

    if (!material->getSamplerInterfaceBlock().isEmpty()) {
        mSamplers = SamplerGroup(material->getSamplerInterfaceBlock().getSize());
        mSbHandle = driver.createSamplerGroup(
                mSamplers.getSize(), utils::FixedSizeString<32>(mMaterial->getName().c_str_safe()));
    }

    const RasterState& rasterState = material->getRasterState();
    // At the moment, only MaterialInstances have a stencil state, but in the future it should be
    // possible to set the stencil state directly on a material (through material definitions, or
    // MaterialBuilder).
    // TODO: Here is where we'd "inherit" the stencil state from the Material.
    // mStencilState = material->getStencilState();

    // We inherit the resolved culling mode rather than the builder-set culling mode.
    // This preserves the property whereby double-sidedness automatically disables culling.
    mCulling = rasterState.culling;
    mColorWrite = rasterState.colorWrite;
    mDepthWrite = rasterState.depthWrite;
    mDepthFunc = rasterState.depthFunc;

    mMaterialSortingKey = RenderPass::makeMaterialSortingKey(
            material->getId(), material->generateMaterialInstanceId());

    if (material->getBlendingMode() == BlendingMode::MASKED) {
        setMaskThreshold(material->getMaskThreshold());
    }

    if (material->hasDoubleSidedCapability()) {
        setDoubleSided(material->isDoubleSided());
    }

    if (material->hasSpecularAntiAliasing()) {
        setSpecularAntiAliasingVariance(material->getSpecularAntiAliasingVariance());
        setSpecularAntiAliasingThreshold(material->getSpecularAntiAliasingThreshold());
    }

    setTransparencyMode(material->getTransparencyMode());
}

FMaterialInstance::FMaterialInstance(FEngine& engine,
        FMaterialInstance const* other, const char* name)
        : mMaterial(other->mMaterial),
          mPolygonOffset(other->mPolygonOffset),
          mStencilState(other->mStencilState),
          mMaskThreshold(other->mMaskThreshold),
          mSpecularAntiAliasingVariance(other->mSpecularAntiAliasingVariance),
          mSpecularAntiAliasingThreshold(other->mSpecularAntiAliasingThreshold),
          mCulling(other->mCulling),
          mDepthFunc(other->mDepthFunc),
          mColorWrite(other->mColorWrite),
          mDepthWrite(other->mDepthWrite),
          mHasScissor(false),
          mIsDoubleSided(other->mIsDoubleSided),
          mScissorRect(other->mScissorRect),
          mName(name ? CString(name) : other->mName) {

    FEngine::DriverApi& driver = engine.getDriverApi();
    FMaterial const* const material = other->getMaterial();

    if (!material->getUniformInterfaceBlock().isEmpty()) {
        mUniforms.setUniforms(other->getUniformBuffer());
        mUbHandle = driver.createBufferObject(mUniforms.getSize(),
                BufferObjectBinding::UNIFORM, backend::BufferUsage::DYNAMIC);
    }

    if (!material->getSamplerInterfaceBlock().isEmpty()) {
        mSamplers = other->getSamplerGroup();
        mSbHandle = driver.createSamplerGroup(
                mSamplers.getSize(), utils::FixedSizeString<32>(mMaterial->getName().c_str_safe()));
    }

    if (material->hasDoubleSidedCapability()) {
        setDoubleSided(mIsDoubleSided);
    }

    if (material->getBlendingMode() == BlendingMode::MASKED) {
        setMaskThreshold(mMaskThreshold);
    }

    if (material->hasSpecularAntiAliasing()) {
        setSpecularAntiAliasingThreshold(mSpecularAntiAliasingThreshold);
        setSpecularAntiAliasingVariance(mSpecularAntiAliasingVariance);
    }

    setTransparencyMode(material->getTransparencyMode());

    mMaterialSortingKey = RenderPass::makeMaterialSortingKey(
            material->getId(), material->generateMaterialInstanceId());
}

FMaterialInstance* FMaterialInstance::duplicate(
        FMaterialInstance const* other, const char* name) noexcept {
    FMaterial const* const material = other->getMaterial();
    FEngine& engine = material->getEngine();
    return engine.createMaterialInstance(material, other, name);
}

FMaterialInstance::~FMaterialInstance() noexcept = default;

void FMaterialInstance::terminate(FEngine& engine) {
    FEngine::DriverApi& driver = engine.getDriverApi();
    driver.destroyBufferObject(mUbHandle);
    driver.destroySamplerGroup(mSbHandle);
}

void FMaterialInstance::commitSlow(DriverApi& driver) const {
    // update uniforms if needed
    if (mUniforms.isDirty()) {
        driver.updateBufferObject(mUbHandle, mUniforms.toBufferDescriptor(driver), 0);
    }
    if (mSamplers.isDirty()) {
        driver.updateSamplerGroup(mSbHandle, mSamplers.toBufferDescriptor(driver));
    }
}

// ------------------------------------------------------------------------------------------------

void FMaterialInstance::setParameter(std::string_view name,
        backend::Handle<backend::HwTexture> texture, backend::SamplerParams params) noexcept {
    size_t const index = mMaterial->getSamplerInterfaceBlock().getSamplerInfo(name)->offset;
    mSamplers.setSampler(index, { texture, params });
}

void FMaterialInstance::setParameterImpl(std::string_view name,
        FTexture const* texture, TextureSampler const& sampler) {

#ifndef NDEBUG
    // Per GLES3.x specification, depth texture can't be filtered unless in compare mode.
    if (texture && isDepthFormat(texture->getFormat())) {
        if (sampler.getCompareMode() == SamplerCompareMode::NONE) {
            SamplerMinFilter const minFilter = sampler.getMinFilter();
            SamplerMagFilter const magFilter = sampler.getMagFilter();
            if (magFilter == SamplerMagFilter::LINEAR ||
                minFilter == SamplerMinFilter::LINEAR ||
                minFilter == SamplerMinFilter::LINEAR_MIPMAP_LINEAR ||
                minFilter == SamplerMinFilter::LINEAR_MIPMAP_NEAREST ||
                minFilter == SamplerMinFilter::NEAREST_MIPMAP_LINEAR) {
                PANIC_LOG("Depth textures can't be sampled with a linear filter "
                          "unless the comparison mode is set to COMPARE_TO_TEXTURE. "
                          "(material: \"%s\", parameter: \"%.*s\")",
                          getMaterial()->getName().c_str(), name.size(), name.data());
            }
        }
    }
#endif

    Handle<HwTexture> handle{};
    if (UTILS_LIKELY(texture)) {
        handle = texture->getHwHandle();
    }
    setParameter(name, handle, sampler.getSamplerParams());
}

void FMaterialInstance::setMaskThreshold(float threshold) noexcept {
    setParameter("_maskThreshold", math::saturate(threshold));
    mMaskThreshold = math::saturate(threshold);
}

float FMaterialInstance::getMaskThreshold() const noexcept {
    return mMaskThreshold;
}

void FMaterialInstance::setSpecularAntiAliasingVariance(float variance) noexcept {
    setParameter("_specularAntiAliasingVariance", math::saturate(variance));
    mSpecularAntiAliasingVariance = math::saturate(variance);
}

float FMaterialInstance::getSpecularAntiAliasingVariance() const noexcept {
    return mSpecularAntiAliasingVariance;
}

void FMaterialInstance::setSpecularAntiAliasingThreshold(float threshold) noexcept {
    setParameter("_specularAntiAliasingThreshold", math::saturate(threshold * threshold));
    mSpecularAntiAliasingThreshold = std::sqrt(math::saturate(threshold * threshold));
}

float FMaterialInstance::getSpecularAntiAliasingThreshold() const noexcept {
    return mSpecularAntiAliasingThreshold;
}

void FMaterialInstance::setDoubleSided(bool doubleSided) noexcept {
    if (UTILS_UNLIKELY(!mMaterial->hasDoubleSidedCapability())) {
        slog.w << "Parent material does not have double-sided capability." << io::endl;
        return;
    }
    setParameter("_doubleSided", doubleSided);
    if (doubleSided) {
        setCullingMode(CullingMode::NONE);
    }
    mIsDoubleSided = doubleSided;
}

bool FMaterialInstance::isDoubleSided() const noexcept {
    return mIsDoubleSided;
}

void FMaterialInstance::setTransparencyMode(TransparencyMode mode) noexcept {
    mTransparencyMode = mode;
}

void FMaterialInstance::setDepthCulling(bool enable) noexcept {
    mDepthFunc = enable ? RasterState::DepthFunc::GE : RasterState::DepthFunc::A;
}

bool FMaterialInstance::isDepthCullingEnabled() const noexcept {
    return mDepthFunc != RasterState::DepthFunc::A;
}

const char* FMaterialInstance::getName() const noexcept {
    // To decide whether to use the parent material name as a fallback, we check for the nullness of
    // the instance's CString rather than calling empty(). This allows instances to override the
    // parent material's name with a blank string.
    if (mName.data() == nullptr) {
        return mMaterial->getName().c_str_safe();
    }
    return mName.c_str();
}

} // namespace filament
