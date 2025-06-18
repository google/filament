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

#include "RenderPass.h"

#include "ds/DescriptorSetLayout.h"

#include "details/Engine.h"
#include "details/Material.h"
#include "details/MaterialInstance.h"
#include "details/Texture.h"
#include "details/Stream.h"

#include "private/filament/EngineEnums.h"

#include <filament/MaterialEnums.h>
#include <filament/TextureSampler.h>

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <utils/BitmaskEnum.h>
#include <utils/CString.h>
#include <utils/Logger.h>
#include <utils/Panic.h>
#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/ostream.h>

#include <math/scalar.h>

#include <algorithm>
#include <cmath>
#include <mutex>
#include <string_view>
#include <utility>

#include <stddef.h>

using namespace filament::math;
using namespace utils;

namespace filament {

using namespace backend;

FMaterialInstance::FMaterialInstance(FEngine& engine, FMaterial const* material,
                                     const char* name) noexcept
        : mMaterial(material),
          mDescriptorSet("MaterialInstance", material->getDescriptorSetLayout()),
          mCulling(CullingMode::BACK),
          mShadowCulling(CullingMode::BACK),
          mDepthFunc(RasterState::DepthFunc::LE),
          mColorWrite(false),
          mDepthWrite(false),
          mHasScissor(false),
          mIsDoubleSided(false),
          mIsDefaultInstance(false),
          mTransparencyMode(TransparencyMode::DEFAULT),
          mName(name ? CString(name) : material->getName()) {

    FEngine::DriverApi& driver = engine.getDriverApi();

    // even if the material doesn't have any parameters, we allocate a small UBO because it's
    // expected by the per-material descriptor-set layout
    size_t const uboSize = std::max(size_t(16), material->getUniformInterfaceBlock().getSize());
    mUniforms = UniformBuffer(uboSize);
    mUbHandle = driver.createBufferObject(mUniforms.getSize(),
            BufferObjectBinding::UNIFORM, BufferUsage::STATIC);
    driver.setDebugTag(mUbHandle.getId(), material->getName());

    // set the UBO, always descriptor 0
    mDescriptorSet.setBuffer(material->getDescriptorSetLayout(),
        0, mUbHandle, 0, mUniforms.getSize());

    const RasterState& rasterState = material->getRasterState();
    // At the moment, only MaterialInstances have a stencil state, but in the future it should be
    // possible to set the stencil state directly on a material (through material definitions, or
    // MaterialBuilder).
    // TODO: Here is where we'd "inherit" the stencil state from the Material.
    // mStencilState = material->getStencilState();

    // We inherit the resolved culling mode rather than the builder-set culling mode.
    // This preserves the property whereby double-sidedness automatically disables culling.
    mCulling = rasterState.culling;
    mShadowCulling = rasterState.culling;
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
          mTextureParameters(other->mTextureParameters),
          mDescriptorSet(other->mDescriptorSet.duplicate(
                "MaterialInstance", mMaterial->getDescriptorSetLayout())),
          mPolygonOffset(other->mPolygonOffset),
          mStencilState(other->mStencilState),
          mMaskThreshold(other->mMaskThreshold),
          mSpecularAntiAliasingVariance(other->mSpecularAntiAliasingVariance),
          mSpecularAntiAliasingThreshold(other->mSpecularAntiAliasingThreshold),
          mCulling(other->mCulling),
          mShadowCulling(other->mShadowCulling),
          mDepthFunc(other->mDepthFunc),
          mColorWrite(other->mColorWrite),
          mDepthWrite(other->mDepthWrite),
          mHasScissor(false),
          mIsDoubleSided(other->mIsDoubleSided),
          mIsDefaultInstance(false),
          mScissorRect(other->mScissorRect),
          mName(name ? CString(name) : other->mName) {

    FEngine::DriverApi& driver = engine.getDriverApi();
    FMaterial const* const material = other->getMaterial();

    mUniforms.setUniforms(other->getUniformBuffer());
    mUbHandle = driver.createBufferObject(mUniforms.getSize(),
            BufferObjectBinding::UNIFORM, BufferUsage::DYNAMIC);
    driver.setDebugTag(mUbHandle.getId(), material->getName());

    // set the UBO, always descriptor 0
    mDescriptorSet.setBuffer(mMaterial->getDescriptorSetLayout(),
            0, mUbHandle, 0, mUniforms.getSize());

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

    // If the original descriptor set has been commited, the copy needs to commit as well.
    if (other->mDescriptorSet.getHandle()) {
        mDescriptorSet.commitSlow(mMaterial->getDescriptorSetLayout(), driver);
    }
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
    mDescriptorSet.terminate(driver);
    driver.destroyBufferObject(mUbHandle);
}

void FMaterialInstance::commitStreamUniformAssociations(FEngine::DriverApi& driver) {
    mHasStreamUniformAssociations = false;
    if (!mTextureParameters.empty()) {
        backend::BufferObjectStreamDescriptor descriptor;
        for (auto const& [binding, p]: mTextureParameters) {
            ssize_t offset = mMaterial->getUniformInterfaceBlock().getTransformFieldOffset(binding);
            if (offset >= 0) {
                mHasStreamUniformAssociations = true;
                auto stream = p.texture->getStream()->getHandle();
                descriptor.mStreams.push_back({uint32_t(offset), stream, BufferObjectStreamAssociationType::TRANSFORM_MATRIX});
            }
        }
        if (descriptor.mStreams.size() > 0) {
            driver.registerBufferObjectStreams(mUbHandle, std::move(descriptor));
        }
    }
}

void FMaterialInstance::commit(FEngine& engine) const {
    if (UTILS_LIKELY(mMaterial->getMaterialDomain() != MaterialDomain::SURFACE)) {
        commit(engine.getDriverApi());
    }
}

void FMaterialInstance::commit(DriverApi& driver) const {
    // update uniforms if needed
    if (mUniforms.isDirty() || mHasStreamUniformAssociations) {
        driver.updateBufferObject(mUbHandle, mUniforms.toBufferDescriptor(driver), 0);
    }
    if (!mTextureParameters.empty()) {
        for (auto const& [binding, p]: mTextureParameters) {
            assert_invariant(p.texture);
            // TODO: figure out a way to do this more efficiently (isValid() is a hashmap lookup)
            FEngine const& engine = mMaterial->getEngine();
            FILAMENT_CHECK_PRECONDITION(engine.isValid(p.texture))
                    << "Invalid texture still bound to MaterialInstance: '" << getName() << "'\n";
            Handle<HwTexture> const handle = p.texture->getHwHandleForSampling();
            assert_invariant(handle);
            mDescriptorSet.setSampler(mMaterial->getDescriptorSetLayout(),
                binding, handle, p.params);
        }
    }

    // TODO: eventually we should remove this in RELEASE builds
    fixMissingSamplers();

    // Commit descriptors if needed (e.g. when textures are updated,or the first time)
    mDescriptorSet.commit(mMaterial->getDescriptorSetLayout(), driver);
}

// ------------------------------------------------------------------------------------------------

void FMaterialInstance::setParameter(std::string_view const name,
        Handle<HwTexture> texture, SamplerParams const params) {
    auto const binding = mMaterial->getSamplerBinding(name);

    mDescriptorSet.setSampler(mMaterial->getDescriptorSetLayout(),
        binding, texture, params);
}

void FMaterialInstance::setParameterImpl(std::string_view const name,
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

    auto const binding = mMaterial->getSamplerBinding(name);

    if (texture) {
        auto const& descriptorSetLayout = mMaterial->getDescriptorSetLayout();
        DescriptorType const descriptorType = descriptorSetLayout.getDescriptorType(binding);
        TextureType const textureType = texture->getTextureType();

        FILAMENT_CHECK_PRECONDITION(
                DescriptorSet::isTextureCompatibleWithDescriptor(textureType, descriptorType))
                << "Texture format " << int(texture->getFormat())
                << " of type " << to_string(textureType)
                << " is not compatible with material \"" << getMaterial()->getName().c_str() << "\""
                << " parameter \"" << name << "\""
                << " of type " << to_string(descriptorType);
    }

    if (texture && texture->textureHandleCanMutate()) {
        mTextureParameters[binding] = { texture, sampler.getSamplerParams() };
    } else {
        // Ensure to erase the binding from mTextureParameters since it will not
        // be updated.
        mTextureParameters.erase(binding);

        Handle<HwTexture> handle{};
        if (texture) {
            handle = texture->getHwHandleForSampling();
            assert_invariant(handle == texture->getHwHandle());
        }
        mDescriptorSet.setSampler(mMaterial->getDescriptorSetLayout(),
            binding, handle, sampler.getSamplerParams());
    }
}

void FMaterialInstance::setMaskThreshold(float const threshold) noexcept {
    setParameter("_maskThreshold", saturate(threshold));
    mMaskThreshold = saturate(threshold);
}

float FMaterialInstance::getMaskThreshold() const noexcept {
    return mMaskThreshold;
}

void FMaterialInstance::setSpecularAntiAliasingVariance(float const variance) noexcept {
    setParameter("_specularAntiAliasingVariance", saturate(variance));
    mSpecularAntiAliasingVariance = saturate(variance);
}

float FMaterialInstance::getSpecularAntiAliasingVariance() const noexcept {
    return mSpecularAntiAliasingVariance;
}

void FMaterialInstance::setSpecularAntiAliasingThreshold(float const threshold) noexcept {
    setParameter("_specularAntiAliasingThreshold", saturate(threshold * threshold));
    mSpecularAntiAliasingThreshold = std::sqrt(saturate(threshold * threshold));
}

float FMaterialInstance::getSpecularAntiAliasingThreshold() const noexcept {
    return mSpecularAntiAliasingThreshold;
}

void FMaterialInstance::setDoubleSided(bool const doubleSided) noexcept {
    if (UTILS_UNLIKELY(!mMaterial->hasDoubleSidedCapability())) {
        LOG(WARNING) << "Parent material does not have double-sided capability.";
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

void FMaterialInstance::setTransparencyMode(TransparencyMode const mode) noexcept {
    mTransparencyMode = mode;
}

void FMaterialInstance::setDepthCulling(bool const enable) noexcept {
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

// ------------------------------------------------------------------------------------------------

void FMaterialInstance::use(FEngine::DriverApi& driver) const {

    if (UTILS_UNLIKELY(mMissingSamplerDescriptors.any())) {
        std::call_once(mMissingSamplersFlag, [this] {
            auto const& list = mMaterial->getSamplerInterfaceBlock().getSamplerInfoList();
            LOG(WARNING) << "sampler parameters not set in MaterialInstance \""
                         << mName.c_str_safe() << "\" or Material \""
                         << mMaterial->getName().c_str_safe() << "\":";
            mMissingSamplerDescriptors.forEachSetBit([&list](descriptor_binding_t binding) {
                auto const pos = std::find_if(list.begin(), list.end(), [binding](const auto& item) {
                    return item.binding == binding;
                });
                // just safety-check, should never fail
                if (UTILS_LIKELY(pos != list.end())) {
                    LOG(WARNING) << "[" << +binding << "] " << pos->name.c_str();
                }
            });
        });
        mMissingSamplerDescriptors.clear();
    }

    mDescriptorSet.bind(driver, DescriptorSetBindingPoints::PER_MATERIAL);
}

void FMaterialInstance::fixMissingSamplers() const {
    // Here we check that all declared sampler parameters are set, this is required by
    // Vulkan and Metal; GL is more permissive. If a sampler parameter is not set, we will
    // log a warning once per MaterialInstance in the system log and patch-in a dummy
    // texture.
    auto const& layout = mMaterial->getDescriptorSetLayout();
    auto const samplersDescriptors = layout.getSamplerDescriptors();
    auto const validDescriptors = mDescriptorSet.getValidDescriptors();
    auto const missingSamplerDescriptors =
            (validDescriptors & samplersDescriptors) ^ samplersDescriptors;

    // always record the missing samplers state at commit() time
    mMissingSamplerDescriptors = missingSamplerDescriptors;

    if (UTILS_UNLIKELY(missingSamplerDescriptors.any())) {
        // here we need to set the samplers that are missing
        auto const& list = mMaterial->getSamplerInterfaceBlock().getSamplerInfoList();
        missingSamplerDescriptors.forEachSetBit([this, &list](descriptor_binding_t binding) {
            auto const pos = std::find_if(list.begin(), list.end(), [binding](const auto& item) {
                return item.binding == binding;
            });
            // just safety-check, should never fail
            if (UTILS_LIKELY(pos != list.end())) {
                FEngine const& engine = mMaterial->getEngine();
                filament::DescriptorSetLayout const& layout = mMaterial->getDescriptorSetLayout();

                if (pos->format == SamplerFormat::FLOAT) {
                    // TODO: we only handle missing samplers that are FLOAT
                    switch (pos->type) {
                        case SamplerType::SAMPLER_2D:
                            mDescriptorSet.setSampler(layout,
                                    binding, engine.getZeroTexture(), {});
                            break;
                        case SamplerType::SAMPLER_2D_ARRAY:
                            mDescriptorSet.setSampler(layout,
                                    binding, engine.getZeroTextureArray(), {});
                            break;
                        case SamplerType::SAMPLER_CUBEMAP:
                            mDescriptorSet.setSampler(layout,
                                    binding, engine.getDummyCubemap()->getHwHandle(), {});
                            break;
                        default:
                            // we're currently not able to fix-up other sampler types
                            break;
                    }
                }
            }
        });
    }
}

} // namespace filament
