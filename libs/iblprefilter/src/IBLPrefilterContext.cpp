/*
 * Copyright (C) 2021 The Android Open Source Project
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

#include "filament-iblprefilter/IBLPrefilterContext.h"

#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/MaterialEnums.h>
#include <filament/RenderTarget.h>
#include <filament/RenderableManager.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/Texture.h>
#include <filament/TextureSampler.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>
#include <filament/Viewport.h>

#include <backend/DriverEnums.h>

#include <private/utils/Tracing.h>

#include <utils/compiler.h>
#include <utils/EntityManager.h>
#include <utils/Panic.h>

#include <math/scalar.h>
#include <math/vec4.h>

#include <algorithm>
#include <cmath>
#include <utility>

#include <stddef.h>
#include <stdint.h>

#include "generated/resources/iblprefilter_materials.h"

namespace {

using namespace filament::math;
using namespace filament;

constexpr float4 sFullScreenTriangleVertices[3] = {
    { -1.0f, -1.0f, 1.0f, 1.0f },
    { 3.0f,  -1.0f, 1.0f, 1.0f },
    { -1.0f, 3.0f,  1.0f, 1.0f }
};

constexpr uint16_t sFullScreenTriangleIndices[3] = { 0, 1, 2 };

static float lodToPerceptualRoughness(float lod) noexcept {
    // Inverse perceptualRoughness-to-LOD mapping:
    // The LOD-to-perceptualRoughness mapping is a quadratic fit for
    // log2(perceptualRoughness)+iblMaxMipLevel when iblMaxMipLevel is 4.
    // We found empirically that this mapping works very well for a 256 cubemap with 5 levels used,
    // but also scales well for other iblMaxMipLevel values.
    const float a = 2.0f;
    const float b = -1.0f;
    return (lod != 0)
           ? saturate((std::sqrt(a * a + 4.0f * b * lod) - a) / (2.0f * b))
           : 0.0f;
}

template<typename T>
constexpr T log4(T x) {
    return std::log2(x) * T(0.5);
}

static void cleanupMaterialInstance(MaterialInstance const* mi, Engine& engine, RenderableManager& rcm,
    RenderableManager::Instance const& ci) {
    // mi is already nullptr, there is no need to clean up again.
    if (mi == nullptr)
        return;

    rcm.clearMaterialInstanceAt(ci, 0);
    engine.destroy(mi);
}

constexpr Texture::Usage COMMON_USAGE =
        Texture::Usage::COLOR_ATTACHMENT | Texture::Usage::SAMPLEABLE;
constexpr Texture::Usage MIPMAP_USAGE = Texture::Usage::GEN_MIPMAPPABLE;

} // namespace


IBLPrefilterContext::IBLPrefilterContext(Engine& engine)
        : mEngine(engine) {
    utils::EntityManager& em = utils::EntityManager::get();
    mCameraEntity = em.create();
    mFullScreenQuadEntity = em.create();

    mIntegrationMaterial = Material::Builder().package(
            IBLPREFILTER_MATERIALS_IBLPREFILTER_DATA,
            IBLPREFILTER_MATERIALS_IBLPREFILTER_SIZE).build(engine);

    mIrradianceIntegrationMaterial = Material::Builder().package(
            IBLPREFILTER_MATERIALS_IBLPREFILTER_DATA,
            IBLPREFILTER_MATERIALS_IBLPREFILTER_SIZE)
                    .constant("irradiance", true)
                    .build(engine);

    mVertexBuffer = VertexBuffer::Builder()
            .vertexCount(3)
            .bufferCount(1)
            .attribute(POSITION, 0,
                    VertexBuffer::AttributeType::FLOAT4, 0)
            .build(engine);

    mIndexBuffer = IndexBuffer::Builder()
            .indexCount(3)
            .bufferType(IndexBuffer::IndexType::USHORT)
            .build(engine);


    mVertexBuffer->setBufferAt(engine, 0,
            { sFullScreenTriangleVertices, sizeof(sFullScreenTriangleVertices) });

    mIndexBuffer->setBuffer(engine,
            { sFullScreenTriangleIndices, sizeof(sFullScreenTriangleIndices) });

    RenderableManager::Builder(1)
            .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, mVertexBuffer, mIndexBuffer)
            .culling(false)
            .castShadows(false)
            .receiveShadows(false)
            .build(engine, mFullScreenQuadEntity);

    mView = engine.createView();
    mScene = engine.createScene();
    mRenderer = engine.createRenderer();
    mCamera = engine.createCamera(mCameraEntity);

    mScene->addEntity(mFullScreenQuadEntity);

    View* const view = mView;
    view->setCamera(mCamera);
    view->setScene(mScene);
    view->setScreenSpaceRefractionEnabled(false);
    view->setShadowingEnabled(false);
    view->setPostProcessingEnabled(false);
    view->setFrustumCullingEnabled(false);
}

IBLPrefilterContext::~IBLPrefilterContext() noexcept {
    utils::EntityManager& em = utils::EntityManager::get();
    auto& engine = mEngine;
    engine.destroy(mView);
    engine.destroy(mScene);
    engine.destroy(mRenderer);
    engine.destroy(mVertexBuffer);
    engine.destroy(mIndexBuffer);
    engine.destroy(mIntegrationMaterial);
    engine.destroy(mIrradianceIntegrationMaterial);
    engine.destroy(mFullScreenQuadEntity);
    engine.destroyCameraComponent(mCameraEntity);
    em.destroy(mFullScreenQuadEntity);
}


IBLPrefilterContext::IBLPrefilterContext(IBLPrefilterContext&& rhs) noexcept
        : mEngine(rhs.mEngine) {
    this->operator=(std::move(rhs));
}

IBLPrefilterContext& IBLPrefilterContext::operator=(IBLPrefilterContext&& rhs) noexcept {
    using std::swap;
    if (this != & rhs) {
        swap(mRenderer, rhs.mRenderer);
        swap(mScene, rhs.mScene);
        swap(mVertexBuffer, rhs.mVertexBuffer);
        swap(mIndexBuffer, rhs.mIndexBuffer);
        swap(mCamera, rhs.mCamera);
        swap(mFullScreenQuadEntity, rhs.mFullScreenQuadEntity);
        swap(mCameraEntity, rhs.mCameraEntity);
        swap(mView, rhs.mView);
        swap(mIntegrationMaterial, rhs.mIntegrationMaterial);
        swap(mIrradianceIntegrationMaterial, rhs.mIrradianceIntegrationMaterial);
    }
    return *this;
}

// ------------------------------------------------------------------------------------------------

IBLPrefilterContext::EquirectangularToCubemap::EquirectangularToCubemap(
        IBLPrefilterContext& context,
        Config const& config)
        : mContext(context), mConfig(config) {
    Engine& engine = mContext.mEngine;
    mEquirectMaterial = Material::Builder().package(
            IBLPREFILTER_MATERIALS_EQUIRECTTOCUBE_DATA,
            IBLPREFILTER_MATERIALS_EQUIRECTTOCUBE_SIZE).build(engine);
}

IBLPrefilterContext::EquirectangularToCubemap::EquirectangularToCubemap(
        IBLPrefilterContext& context) : EquirectangularToCubemap(context, {}) {
}

IBLPrefilterContext::EquirectangularToCubemap::~EquirectangularToCubemap() noexcept {
    Engine& engine = mContext.mEngine;
    engine.destroy(mEquirectMaterial);
}

IBLPrefilterContext::EquirectangularToCubemap::EquirectangularToCubemap(
        EquirectangularToCubemap&& rhs) noexcept
        : mContext(rhs.mContext) {
    using std::swap;
    swap(mEquirectMaterial, rhs.mEquirectMaterial);
}

IBLPrefilterContext::EquirectangularToCubemap&
IBLPrefilterContext::EquirectangularToCubemap::operator=(
        EquirectangularToCubemap&& rhs) noexcept {
    using std::swap;
    if (this != &rhs) {
        swap(mEquirectMaterial, rhs.mEquirectMaterial);
    }
    return *this;
}

Texture* IBLPrefilterContext::EquirectangularToCubemap::operator()(
        Texture const* equirect, Texture* outCube) {
    FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_FILAMENT);
    using namespace backend;

    const TextureCubemapFace faces[2][3] = {
            { TextureCubemapFace::POSITIVE_X, TextureCubemapFace::POSITIVE_Y, TextureCubemapFace::POSITIVE_Z },
            { TextureCubemapFace::NEGATIVE_X, TextureCubemapFace::NEGATIVE_Y, TextureCubemapFace::NEGATIVE_Z }
    };

    Engine& engine = mContext.mEngine;
    View* const view = mContext.mView;
    Renderer* const renderer = mContext.mRenderer;
    MaterialInstance* const mi = mEquirectMaterial->createInstance();

    FILAMENT_CHECK_PRECONDITION(equirect != nullptr) << "equirect is null!";

    FILAMENT_CHECK_PRECONDITION(equirect->getTarget() == Texture::Sampler::SAMPLER_2D)
            << "equirect must be a 2D texture.";

    UTILS_UNUSED_IN_RELEASE
    const uint8_t maxLevelCount = std::max(1, std::ilogbf(float(equirect->getWidth())) + 1);

    FILAMENT_CHECK_PRECONDITION(equirect->getLevels() == maxLevelCount)
            << "equirect must have " << +maxLevelCount << " mipmap levels allocated.";

    if (outCube == nullptr) {
        outCube = Texture::Builder()
                .sampler(Texture::Sampler::SAMPLER_CUBEMAP)
                .format(Texture::InternalFormat::R11F_G11F_B10F)
                .usage(COMMON_USAGE | MIPMAP_USAGE)
                .width(256).height(256).levels(0xFF)
                .build(engine);
    }

    FILAMENT_CHECK_PRECONDITION(outCube->getTarget() == Texture::Sampler::SAMPLER_CUBEMAP)
            << "outCube must be a Cubemap texture.";

    const uint32_t dim = outCube->getWidth();

    RenderableManager& rcm = engine.getRenderableManager();
    auto const ci = rcm.getInstance(mContext.mFullScreenQuadEntity);

    TextureSampler environmentSampler;
    environmentSampler.setMagFilter(SamplerMagFilter::LINEAR);
    environmentSampler.setMinFilter(SamplerMinFilter::LINEAR_MIPMAP_LINEAR);
    environmentSampler.setAnisotropy(16.0f); // maybe make this an option

    mi->setParameter("equirect", equirect, environmentSampler);

    // We need mipmaps because we're sampling down
    equirect->generateMipmaps(engine);

    view->setViewport({ 0, 0, dim, dim });

    RenderTarget::Builder builder;
    builder.texture(RenderTarget::AttachmentPoint::COLOR0, outCube)
           .texture(RenderTarget::AttachmentPoint::COLOR1, outCube)
           .texture(RenderTarget::AttachmentPoint::COLOR2, outCube);

    mi->setParameter("mirror", mConfig.mirror ? -1.0f : 1.0f);

    for (size_t i = 0; i < 2; i++) {
        // This is a workaround for internal bug b/419664914 to duplicate same material for each draw.
        // TODO: properly address the bug and remove this workaround.
#if defined(__EMSCRIPTEN__)
        MaterialInstance *const tempMi = MaterialInstance::duplicate(mi);
#else
        MaterialInstance *const tempMi = mi;
#endif
        rcm.setMaterialInstanceAt(ci, 0, tempMi);

        tempMi->setParameter("side", i == 0 ? 1.0f : -1.0f);
        tempMi->commit(engine);

        builder.face(RenderTarget::AttachmentPoint::COLOR0, faces[i][0])
               .face(RenderTarget::AttachmentPoint::COLOR1, faces[i][1])
               .face(RenderTarget::AttachmentPoint::COLOR2, faces[i][2]);

        RenderTarget* const rt = builder.build(engine);
        view->setRenderTarget(rt);
        renderer->renderStandaloneView(view);
        engine.destroy(rt);

#if defined(__EMSCRIPTEN__)
        cleanupMaterialInstance(tempMi, engine, rcm, ci);
#endif
    }

    rcm.clearMaterialInstanceAt(ci, 0);
    engine.destroy(mi);

    return outCube;
}

// ------------------------------------------------------------------------------------------------

IBLPrefilterContext::IrradianceFilter::IrradianceFilter(IBLPrefilterContext& context,
        Config config)
        : mContext(context),
         mSampleCount(std::min(config.sampleCount, uint16_t(2048))) {

    FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_FILAMENT);
    using namespace backend;

    Engine& engine = mContext.mEngine;
    View* const view = mContext.mView;
    Renderer* const renderer = mContext.mRenderer;

    mKernelMaterial = Material::Builder().package(
            IBLPREFILTER_MATERIALS_GENERATEKERNEL_DATA,
            IBLPREFILTER_MATERIALS_GENERATEKERNEL_SIZE)
                    .constant("irradiance", true)
                    .build(engine);

    // { L.x, L.y, L.z, lod }
    mKernelTexture = Texture::Builder()
            .sampler(Texture::Sampler::SAMPLER_2D)
            .format(Texture::InternalFormat::RGBA16F)
            .usage(COMMON_USAGE)
            .width(1)
            .height(mSampleCount)
            .build(engine);

    MaterialInstance* const mi = mKernelMaterial->createInstance();
    mi->setParameter("size", uint2{ 1, mSampleCount });
    mi->setParameter("sampleCount", float(mSampleCount));
    mi->commit(engine);

    RenderableManager& rcm = engine.getRenderableManager();
    auto const ci = rcm.getInstance(mContext.mFullScreenQuadEntity);
    rcm.setMaterialInstanceAt(ci, 0, mi);

    RenderTarget* const rt = RenderTarget::Builder()
            .texture(RenderTarget::AttachmentPoint::COLOR0, mKernelTexture)
            .build(engine);

    view->setRenderTarget(rt);
    view->setViewport({ 0, 0, 1, mSampleCount });

    renderer->renderStandaloneView(view);

    cleanupMaterialInstance(mi, engine, rcm, ci);
    engine.destroy(rt);
}

UTILS_NOINLINE
IBLPrefilterContext::IrradianceFilter::IrradianceFilter(IBLPrefilterContext& context)
        : IrradianceFilter(context, {}) {
}

IBLPrefilterContext::IrradianceFilter::~IrradianceFilter() noexcept {
    Engine& engine = mContext.mEngine;
    engine.destroy(mKernelTexture);
    engine.destroy(mKernelMaterial);
}

IBLPrefilterContext::IrradianceFilter::IrradianceFilter(
        IrradianceFilter&& rhs) noexcept
        : mContext(rhs.mContext) {
    this->operator=(std::move(rhs));
}

IBLPrefilterContext::IrradianceFilter& IBLPrefilterContext::IrradianceFilter::operator=(
        IrradianceFilter&& rhs) noexcept {
    using std::swap;
    if (this != & rhs) {
        swap(mKernelMaterial, rhs.mKernelMaterial);
        swap(mKernelTexture, rhs.mKernelTexture);
        mSampleCount = rhs.mSampleCount;
    }
    return *this;
}

Texture* IBLPrefilterContext::IrradianceFilter::operator()(Options options,
        Texture const* environmentCubemap, Texture* outIrradianceTexture) {

    FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_FILAMENT);
    using namespace backend;

    FILAMENT_CHECK_PRECONDITION(environmentCubemap != nullptr) << "environmentCubemap is null!";

    FILAMENT_CHECK_PRECONDITION(
            environmentCubemap->getTarget() == Texture::Sampler::SAMPLER_CUBEMAP)
            << "environmentCubemap must be a cubemap.";

    UTILS_UNUSED_IN_RELEASE
    const uint8_t maxLevelCount = uint8_t(std::log2(environmentCubemap->getWidth()) + 0.5f) + 1u;

    FILAMENT_CHECK_PRECONDITION(environmentCubemap->getLevels() == maxLevelCount)
            << "environmentCubemap must have " << +maxLevelCount << " mipmap levels allocated.";

    if (outIrradianceTexture == nullptr) {
        outIrradianceTexture =
                Texture::Builder()
                        .sampler(Texture::Sampler::SAMPLER_CUBEMAP)
                        .format(Texture::InternalFormat::R11F_G11F_B10F)
                        .usage(COMMON_USAGE |
                                (options.generateMipmap ? MIPMAP_USAGE : Texture::Usage::NONE))
                        .width(256)
                        .height(256)
                        .levels(0xff)
                        .build(mContext.mEngine);
    }

    FILAMENT_CHECK_PRECONDITION(
            outIrradianceTexture->getTarget() == Texture::Sampler::SAMPLER_CUBEMAP)
            << "outReflectionsTexture must be a cubemap.";

    const TextureCubemapFace faces[2][3] = {
            { TextureCubemapFace::POSITIVE_X, TextureCubemapFace::POSITIVE_Y, TextureCubemapFace::POSITIVE_Z },
            { TextureCubemapFace::NEGATIVE_X, TextureCubemapFace::NEGATIVE_Y, TextureCubemapFace::NEGATIVE_Z }
    };

    Engine& engine = mContext.mEngine;
    View* const view = mContext.mView;
    Renderer* const renderer = mContext.mRenderer;
    MaterialInstance* const mi = mContext.mIrradianceIntegrationMaterial->createInstance();

    RenderableManager& rcm = engine.getRenderableManager();
    auto const ci = rcm.getInstance(mContext.mFullScreenQuadEntity);

    const uint32_t sampleCount = mSampleCount;
    const float linear = options.hdrLinear;
    const float compress = options.hdrMax;
    const uint32_t dim = outIrradianceTexture->getWidth();
    const float omegaP = (4.0f * f::PI) / float(6 * dim * dim);

    TextureSampler environmentSampler;
    environmentSampler.setMagFilter(SamplerMagFilter::LINEAR);
    environmentSampler.setMinFilter(SamplerMinFilter::LINEAR_MIPMAP_LINEAR);

    mi->setParameter("environment", environmentCubemap, environmentSampler);
    mi->setParameter("kernel", mKernelTexture, TextureSampler{ SamplerMagFilter::NEAREST });
    mi->setParameter("compress", float2{ linear, compress });
    mi->setParameter("lodOffset", options.lodOffset - log4(omegaP));
    mi->setParameter("sampleCount", sampleCount);

    if (options.generateMipmap) {
        // We need mipmaps for prefiltering
        environmentCubemap->generateMipmaps(engine);
    }

    RenderTarget::Builder builder;
    builder.texture(RenderTarget::AttachmentPoint::COLOR0, outIrradianceTexture)
           .texture(RenderTarget::AttachmentPoint::COLOR1, outIrradianceTexture)
           .texture(RenderTarget::AttachmentPoint::COLOR2, outIrradianceTexture);


    view->setViewport({ 0, 0, dim, dim });

    for (size_t i = 0; i < 2; i++) {
        // This is a workaround for internal bug b/419664914 to duplicate same material for each draw.
        // TODO: properly address the bug and remove this workaround.
#if defined(__EMSCRIPTEN__)
        MaterialInstance *const tempMi = MaterialInstance::duplicate(mi);
#else
        MaterialInstance *const tempMi = mi;
#endif
        rcm.setMaterialInstanceAt(ci, 0, tempMi);

        tempMi->setParameter("side", i == 0 ? 1.0f : -1.0f);
        tempMi->commit(engine);

        builder.face(RenderTarget::AttachmentPoint::COLOR0, faces[i][0])
               .face(RenderTarget::AttachmentPoint::COLOR1, faces[i][1])
               .face(RenderTarget::AttachmentPoint::COLOR2, faces[i][2]);

        RenderTarget* const rt = builder.build(engine);
        view->setRenderTarget(rt);
        renderer->renderStandaloneView(view);
        engine.destroy(rt);

#if defined(__EMSCRIPTEN__)
        cleanupMaterialInstance(tempMi, engine, rcm, ci);
#endif
    }

    rcm.clearMaterialInstanceAt(ci, 0);
    engine.destroy(mi);

    return outIrradianceTexture;
}

UTILS_NOINLINE
Texture* IBLPrefilterContext::IrradianceFilter::operator()(
        Texture const* environmentCubemap, Texture* outIrradianceTexture) {
    return operator()({}, environmentCubemap, outIrradianceTexture);
}

// ------------------------------------------------------------------------------------------------

IBLPrefilterContext::SpecularFilter::SpecularFilter(IBLPrefilterContext& context, Config config)
    : mContext(context) {

    auto lodToPerceptualRoughness = [](const float lod) -> float {
        // Inverse perceptualRoughness-to-LOD mapping:
        // The LOD-to-perceptualRoughness mapping is a quadratic fit for
        // log2(perceptualRoughness)+iblMaxMipLevel when iblMaxMipLevel is 4.
        // We found empirically that this mapping works very well for a 256 cubemap with 5 levels used,
        // but also scales well for other iblMaxMipLevel values.
        const float a = 2.0f;
        const float b = -1.0f;
        return (lod != 0.0f) ? saturate((sqrt(a * a + 4.0f * b * lod) - a) / (2.0f * b)) : 0.0f;
    };

    FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_FILAMENT);
    using namespace backend;

    Engine& engine = mContext.mEngine;
    View* const view = mContext.mView;
    Renderer* const renderer = mContext.mRenderer;


    mSampleCount = std::min(config.sampleCount, uint16_t(2048));
    mLevelCount = std::max(config.levelCount, uint8_t(1u));

    mKernelMaterial = Material::Builder().package(
            IBLPREFILTER_MATERIALS_GENERATEKERNEL_DATA,
            IBLPREFILTER_MATERIALS_GENERATEKERNEL_SIZE).build(engine);

    // { L.x, L.y, L.z, lod }
    mKernelTexture = Texture::Builder()
            .sampler(Texture::Sampler::SAMPLER_2D)
            .format(Texture::InternalFormat::RGBA16F)
            .usage(COMMON_USAGE)
            .width(mLevelCount)
            .height(mSampleCount)
            .build(engine);

    float roughnessArray[16] = {};
    for (size_t i = 0, c = mLevelCount; i < c; i++) {
        float const perceptualRoughness = lodToPerceptualRoughness(
                saturate(float(i) * (1.0f / (float(mLevelCount) - 1.0f))));
        float const roughness = perceptualRoughness * perceptualRoughness;
        roughnessArray[i] = roughness;
    }

    MaterialInstance* const mi = mKernelMaterial->createInstance();
    mi->setParameter("size", uint2{ mLevelCount, mSampleCount });
    mi->setParameter("sampleCount", float(mSampleCount));
    mi->setParameter("roughness", roughnessArray, 16);
    mi->commit(engine);

    RenderableManager& rcm = engine.getRenderableManager();
    auto const ci = rcm.getInstance(mContext.mFullScreenQuadEntity);
    rcm.setMaterialInstanceAt(ci, 0, mi);

    RenderTarget* const rt = RenderTarget::Builder()
            .texture(RenderTarget::AttachmentPoint::COLOR0, mKernelTexture)
            .build(engine);

    view->setRenderTarget(rt);
    view->setViewport({ 0, 0, mLevelCount, mSampleCount });

    renderer->renderStandaloneView(view);

    cleanupMaterialInstance(mi, engine, rcm, ci);
    engine.destroy(rt);
}

UTILS_NOINLINE
IBLPrefilterContext::SpecularFilter::SpecularFilter(IBLPrefilterContext& context)
    : SpecularFilter(context, {}) {
}

IBLPrefilterContext::SpecularFilter::~SpecularFilter() noexcept {
    Engine& engine = mContext.mEngine;
    engine.destroy(mKernelTexture);
    engine.destroy(mKernelMaterial);
}

IBLPrefilterContext::SpecularFilter::SpecularFilter(SpecularFilter&& rhs) noexcept
        : mContext(rhs.mContext) {
    this->operator=(std::move(rhs));
}

IBLPrefilterContext::SpecularFilter&
IBLPrefilterContext::SpecularFilter::operator=(SpecularFilter&& rhs) noexcept {
    using std::swap;
    if (this != & rhs) {
        swap(mKernelMaterial, rhs.mKernelMaterial);
        swap(mKernelTexture, rhs.mKernelTexture);
        mSampleCount = rhs.mSampleCount;
        mLevelCount = rhs.mLevelCount;
    }
    return *this;
}

UTILS_NOINLINE
Texture* IBLPrefilterContext::SpecularFilter::operator()(
        Texture const* environmentCubemap, Texture* outReflectionsTexture) {
    return operator()({}, environmentCubemap, outReflectionsTexture);
}

Texture* IBLPrefilterContext::SpecularFilter::operator()(
        Options options,
        Texture const* environmentCubemap, Texture* outReflectionsTexture) {

    FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_FILAMENT);
    using namespace backend;

    FILAMENT_CHECK_PRECONDITION(environmentCubemap != nullptr) << "environmentCubemap is null!";

    FILAMENT_CHECK_PRECONDITION(
            environmentCubemap->getTarget() == Texture::Sampler::SAMPLER_CUBEMAP)
            << "environmentCubemap must be a cubemap.";

    UTILS_UNUSED_IN_RELEASE
    const uint8_t maxLevelCount = uint8_t(std::log2(environmentCubemap->getWidth()) + 0.5f) + 1u;

    FILAMENT_CHECK_PRECONDITION(environmentCubemap->getLevels() == maxLevelCount)
            << "environmentCubemap must have " << +maxLevelCount << " mipmap levels allocated.";

    if (outReflectionsTexture == nullptr) {
        const uint8_t levels = mLevelCount;

        // default texture is 256 or larger to accommodate the level count requested
        const uint32_t dim = std::max(256u, 1u << (levels - 1u));

        outReflectionsTexture =
                Texture::Builder()
                        .sampler(Texture::Sampler::SAMPLER_CUBEMAP)
                        .format(Texture::InternalFormat::R11F_G11F_B10F)
                        .usage(COMMON_USAGE |
                                (options.generateMipmap ? MIPMAP_USAGE : Texture::Usage::NONE))
                        .width(dim)
                        .height(dim)
                        .levels(levels)
                        .build(mContext.mEngine);
    }

    FILAMENT_CHECK_PRECONDITION(
            outReflectionsTexture->getTarget() == Texture::Sampler::SAMPLER_CUBEMAP)
            << "outReflectionsTexture must be a cubemap.";

    FILAMENT_CHECK_PRECONDITION(mLevelCount <= outReflectionsTexture->getLevels())
            << "outReflectionsTexture has " << +outReflectionsTexture->getLevels() << " levels but "
            << +mLevelCount << " are requested.";

    const TextureCubemapFace faces[2][3] = {
            { TextureCubemapFace::POSITIVE_X, TextureCubemapFace::POSITIVE_Y, TextureCubemapFace::POSITIVE_Z },
            { TextureCubemapFace::NEGATIVE_X, TextureCubemapFace::NEGATIVE_Y, TextureCubemapFace::NEGATIVE_Z }
    };

    Engine& engine = mContext.mEngine;
    View* const view = mContext.mView;
    Renderer* const renderer = mContext.mRenderer;
    MaterialInstance* const mi = mContext.mIntegrationMaterial->createInstance();

    RenderableManager& rcm = engine.getRenderableManager();
    auto const ci = rcm.getInstance(mContext.mFullScreenQuadEntity);

    const uint32_t sampleCount = mSampleCount;
    const float linear = options.hdrLinear;
    const float compress = options.hdrMax;
    const uint8_t levels = outReflectionsTexture->getLevels();
    uint32_t dim = outReflectionsTexture->getWidth();
    const float omegaP = (4.0f * f::PI) / float(6 * dim * dim);

    TextureSampler environmentSampler;
    environmentSampler.setMagFilter(SamplerMagFilter::LINEAR);
    environmentSampler.setMinFilter(SamplerMinFilter::LINEAR_MIPMAP_LINEAR);

    mi->setParameter("environment", environmentCubemap, environmentSampler);
    mi->setParameter("kernel", mKernelTexture, TextureSampler{ SamplerMagFilter::NEAREST });
    mi->setParameter("compress", float2{ linear, compress });
    mi->setParameter("lodOffset", options.lodOffset - log4(omegaP));

    if (options.generateMipmap) {
        // We need mipmaps for prefiltering
        environmentCubemap->generateMipmaps(engine);
    }

    RenderTarget::Builder builder;
    builder.texture(RenderTarget::AttachmentPoint::COLOR0, outReflectionsTexture)
           .texture(RenderTarget::AttachmentPoint::COLOR1, outReflectionsTexture)
           .texture(RenderTarget::AttachmentPoint::COLOR2, outReflectionsTexture);

    for (size_t lod = 0; lod < levels; lod++) {
        FILAMENT_TRACING_NAME(FILAMENT_TRACING_CATEGORY_FILAMENT, "executeFilterLOD");

        mi->setParameter("sampleCount", uint32_t(lod == 0 ? 1u : sampleCount));
        mi->setParameter("attachmentLevel", uint32_t(lod));

        if (lod == levels - 1) {
            // this is the last lod, use a more aggressive filtering because this level is also
            // used for the diffuse brdf by filament, and we need it to be very smooth.
            // So we set the lod offset to at least 2.
            mi->setParameter("lodOffset", std::max(2.0f, options.lodOffset) - log4(omegaP));
        }

        builder.mipLevel(RenderTarget::AttachmentPoint::COLOR0, lod)
               .mipLevel(RenderTarget::AttachmentPoint::COLOR1, lod)
               .mipLevel(RenderTarget::AttachmentPoint::COLOR2, lod);

        view->setViewport({ 0, 0, dim, dim });

        for (size_t i = 0; i < 2; i++) {
            // This is a workaround for internal bug b/419664914 to duplicate same material for each draw.
            // TODO: properly address the bug and remove this workaround.
#if defined(__EMSCRIPTEN__)
            MaterialInstance *const tempMi = MaterialInstance::duplicate(mi);
#else
            MaterialInstance *const tempMi = mi;
#endif
            rcm.setMaterialInstanceAt(ci, 0, tempMi);

            tempMi->setParameter("side", i == 0 ? 1.0f : -1.0f);
            tempMi->commit(engine);

            builder.face(RenderTarget::AttachmentPoint::COLOR0, faces[i][0])
                   .face(RenderTarget::AttachmentPoint::COLOR1, faces[i][1])
                   .face(RenderTarget::AttachmentPoint::COLOR2, faces[i][2]);

            RenderTarget* const rt = builder.build(engine);
            view->setRenderTarget(rt);
            renderer->renderStandaloneView(view);
            engine.destroy(rt);

#if defined(__EMSCRIPTEN__)
            cleanupMaterialInstance(tempMi, engine, rcm, ci);
#endif
        }

        dim >>= 1;
    }

    rcm.clearMaterialInstanceAt(ci, 0);
    engine.destroy(mi);

    return outReflectionsTexture;
}
