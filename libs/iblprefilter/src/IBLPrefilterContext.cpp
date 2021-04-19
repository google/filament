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

#include "iblprefilter/IBLPrefilterContext.h"

#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/RenderTarget.h>
#include <filament/RenderableManager.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/Texture.h>
#include <filament/TextureSampler.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>
#include <filament/Viewport.h>

#include <utils/Panic.h>
#include <utils/EntityManager.h>
#include <utils/Systrace.h>

#include <math/mat3.h>
#include <math/vec3.h>

#include "generated/resources/iblprefilter_materials.h"

using namespace filament::math;
using namespace filament;

constexpr static float4 sFullScreenTriangleVertices[3] = {
        { -1.0f, -1.0f, 1.0f, 1.0f },
        { 3.0f,  -1.0f, 1.0f, 1.0f },
        { -1.0f, 3.0f,  1.0f, 1.0f }
};

constexpr static const uint16_t sFullScreenTriangleIndices[3] = { 0, 1, 2 };

static float DistributionGGX(float NoH, float linearRoughness) noexcept {
    // NOTE: (aa-1) == (a-1)(a+1) produces better fp accuracy
    float a = linearRoughness;
    float f = (a - 1) * ((a + 1) * (NoH * NoH)) + 1;
    return (a * a) / ((float)F_PI * f * f);
}

static float3 hemisphereImportanceSampleDggx(float2 u, float a) { // pdf = D(a) * cosTheta
    const float phi = 2.0f * (float)F_PI * u.x;
    // NOTE: (aa-1) == (a-1)(a+1) produces better fp accuracy
    const float cosTheta2 = (1 - u.y) / (1 + (a + 1) * ((a - 1) * u.y));
    const float cosTheta = std::sqrt(cosTheta2);
    const float sinTheta = std::sqrt(1 - cosTheta2);
    return { sinTheta * std::cos(phi), sinTheta * std::sin(phi), cosTheta };
}

inline math::float2 hammersley(uint32_t i, float iN) {
    constexpr float tof = 0.5f / 0x80000000U;
    uint32_t bits = i;
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return { i * iN, bits * tof };
}

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
static inline constexpr T log4(T x) {
    return std::log2(x) * T(0.5);
}


IBLPrefilterContext::IBLPrefilterContext(Engine& engine)
        : mEngine(engine) {
    utils::EntityManager& em = utils::EntityManager::get();
    mCameraEntity = em.create();
    mFullScreenQuadEntity = em.create();

    mMaterial = Material::Builder().package(
            IBLPREFILTER_MATERIALS_IBLPREFILTER_DATA,
            IBLPREFILTER_MATERIALS_IBLPREFILTER_SIZE).build(engine);

    mVertexBuffer = VertexBuffer::Builder()
            .vertexCount(3)
            .bufferCount(1)
            .attribute(VertexAttribute::POSITION, 0,
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
            .material(0, mMaterial->getDefaultInstance())
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
    engine.destroy(mMaterial);
    engine.destroy(mFullScreenQuadEntity);
    engine.destroyCameraComponent(mCameraEntity);
    em.destroy(mFullScreenQuadEntity);
}


IBLPrefilterContext::IBLPrefilterContext(IBLPrefilterContext&& rhs) noexcept
        : mEngine(rhs.mEngine) {
    this->operator=(std::move(rhs));
}

IBLPrefilterContext& IBLPrefilterContext::operator=(IBLPrefilterContext&& rhs) {
    using std::swap;
    if (this != & rhs) {
        swap(mRenderer, rhs.mRenderer);
        swap(mScene, rhs.mScene);
        swap(mVertexBuffer, rhs.mVertexBuffer);
        swap(mIndexBuffer, rhs.mIndexBuffer);
        swap(mCamera, rhs.mCamera);
        swap(mFullScreenQuadEntity, rhs.mFullScreenQuadEntity);
        swap(mCameraEntity, rhs.mCameraEntity);
        swap(mMaterial, rhs.mMaterial);
        swap(mView, rhs.mView);
    }
    return *this;
}

// ------------------------------------------------------------------------------------------------

IBLPrefilterContext::SpecularFilter::SpecularFilter(IBLPrefilterContext& context, Config config)
    : mContext(context), mConfig(config) {
    SYSTRACE_CALL();
    using namespace backend;

    Engine& engine = mContext.mEngine;

    mSampleCount = std::min(mConfig.sampleCount, uint16_t(2048));
    mLevelCount = std::max(mConfig.levelCount, uint8_t(1u));

    // { L.x, L.y, L.z, lod }
    mWeights = Texture::Builder()
            .sampler(Texture::Sampler::SAMPLER_2D)
            .format(Texture::InternalFormat::RGBA16F)
            .usage(Texture::Usage::UPLOADABLE | Texture::Usage::SAMPLEABLE)
            .width(mLevelCount)
            .height(mSampleCount)
            .build(engine);

    mWeightSum = new float[mLevelCount];

    const uint32_t sampleCount = mSampleCount;
    const size_t levels = mLevelCount;

    // TODO: do this with a shader

    for (uint32_t lod = 0 ; lod < levels; lod++) {
        SYSTRACE_NAME("computeFilterLOD");
        const float perceptualRoughness = lodToPerceptualRoughness(saturate(lod / (levels - 1.0f)));
        const float roughness = perceptualRoughness * perceptualRoughness;

        uint32_t effectiveSampleCount = sampleCount;
        if (lod == 0) {
            effectiveSampleCount = 1;
        }

        // compute the coefficients
        half4* const p = (half4*)engine.streamAlloc(effectiveSampleCount * sizeof(half4));

        float weight = 0.0f;
        for (size_t i = 0; i < effectiveSampleCount; i++) {
            const float2 u = hammersley(uint32_t(i), 1.0f / float(effectiveSampleCount));
            const float3 H = hemisphereImportanceSampleDggx(u, roughness);
            const float NoH = H.z;
            const float NoH2 = H.z * H.z;
            const float NoL = saturate(2 * NoH2 - 1);
            const float3 L(2 * NoH * H.x, 2 * NoH * H.y, NoL);

            const float pdf = DistributionGGX(NoH, roughness) / 4;
            const float invOmegaS = effectiveSampleCount * pdf;
            const float l = 1.0f - log4(invOmegaS);

            weight += NoL; // note: L.z == NoL
            p[i] = { L, l };
        }

        assert_invariant(lod < mLevelCount);
        mWeightSum[lod] = weight;

        // upload the coefficients
        mWeights->setImage(engine, 0,
                lod, 0, 1, effectiveSampleCount, {
                p, effectiveSampleCount * sizeof(half4),
                PixelDataFormat::RGBA, PixelDataType::HALF});
    }
}

UTILS_NOINLINE
IBLPrefilterContext::SpecularFilter::SpecularFilter(IBLPrefilterContext& context)
    : SpecularFilter(context, {}) {
}

IBLPrefilterContext::SpecularFilter::~SpecularFilter() noexcept {
    Engine& engine = mContext.mEngine;
    engine.destroy(mWeights);
    delete [] mWeightSum;
}

IBLPrefilterContext::SpecularFilter::SpecularFilter(SpecularFilter&& rhs) noexcept
        : mContext(rhs.mContext) {
    this->operator=(std::move(rhs));
}

IBLPrefilterContext::SpecularFilter& IBLPrefilterContext::SpecularFilter::operator=(SpecularFilter&& rhs) {
    using std::swap;
    if (this != & rhs) {
        swap(mWeights, rhs.mWeights);
        mSampleCount = rhs.mSampleCount;
        mConfig = rhs.mConfig;
    }
    return *this;
}

Texture* IBLPrefilterContext::SpecularFilter::createReflectionsTexture(
        ReflectionsOptions options) {
    Engine& engine = mContext.mEngine;

    const uint8_t maxLevels = uint8_t(std::log2(options.size)) + 1u;
    const uint8_t levels = std::min(maxLevels, mConfig.levelCount);

    ASSERT_PRECONDITION(mConfig.levelCount <= maxLevels,
            "Requested texture size of %u is too small to accommodate %u mipmap levels",
            +options.size, +mConfig.levelCount);

    const uint32_t dim = 1u << (maxLevels - 1u);

    Texture* const outCubemap = Texture::Builder()
            .sampler(Texture::Sampler::SAMPLER_CUBEMAP)
            .format(options.format)
            .usage(Texture::Usage::COLOR_ATTACHMENT | Texture::Usage::SAMPLEABLE | options.usage)
            .width(dim).height(dim).levels(levels)
            .build(engine);

    return outCubemap;
}

UTILS_NOINLINE
filament::Texture* IBLPrefilterContext::SpecularFilter::operator()(
        filament::Texture const* environmentCubemap,
        filament::Texture* outReflectionsTexture) {
    return operator()({}, environmentCubemap, outReflectionsTexture);
}

filament::Texture* IBLPrefilterContext::SpecularFilter::operator()(
        IBLPrefilterContext::SpecularFilter::Options options,
        filament::Texture const* environmentCubemap,
        filament::Texture* outReflectionsTexture) {

    SYSTRACE_CALL();
    using namespace backend;

    ASSERT_PRECONDITION(environmentCubemap != nullptr, "outReflectionsTexture is null!");

    ASSERT_PRECONDITION(environmentCubemap->getTarget() == Texture::Sampler::SAMPLER_CUBEMAP,
            "outReflectionsTexture must be a cubemap!");

    if (outReflectionsTexture == nullptr) {
        outReflectionsTexture = createReflectionsTexture({});
    }

    ASSERT_PRECONDITION(mConfig.levelCount <= outReflectionsTexture->getLevels(),
            "outReflectionsTexture has %u levels but %u are requested.",
            +outReflectionsTexture->getLevels(), +mConfig.levelCount);

    const TextureCubemapFace faces[2][3] = {
            { TextureCubemapFace::POSITIVE_X, TextureCubemapFace::POSITIVE_Y, TextureCubemapFace::POSITIVE_Z },
            { TextureCubemapFace::NEGATIVE_X, TextureCubemapFace::NEGATIVE_Y, TextureCubemapFace::NEGATIVE_Z }
    };

    Engine& engine = mContext.mEngine;
    View* const view = mContext.mView;
    Renderer* const renderer = mContext.mRenderer;
    Texture* const weights = mWeights;
    MaterialInstance* const mi = mContext.mMaterial->createInstance("IBLPrefilterContext::Filter");

    RenderableManager& rcm = engine.getRenderableManager();
    rcm.setMaterialInstanceAt(
            rcm.getInstance(mContext.mFullScreenQuadEntity), 0, mi);

    const uint8_t inputLevels = environmentCubemap->getLevels();
    const uint32_t sampleCount = mSampleCount;
    const float linear = options.hdrLinear;
    const float compress = options.hdrMax;
    const uint8_t levels = outReflectionsTexture->getLevels();
    uint32_t dim = outReflectionsTexture->getWidth();

    TextureSampler environmentSampler;
    environmentSampler.setMagFilter(SamplerMagFilter::LINEAR);
    environmentSampler.setMinFilter(SamplerMinFilter::LINEAR_MIPMAP_LINEAR);

    mi->setParameter("environment", environmentCubemap, environmentSampler);
    mi->setParameter("compress", float2{ linear, compress });


    if (options.generateMipmap) {
        // We need mipmaps for prefiltering
        environmentCubemap->generateMipmaps(engine);
    }

    RenderTarget::Builder builder;
    builder.texture(RenderTarget::AttachmentPoint::COLOR0, outReflectionsTexture)
           .texture(RenderTarget::AttachmentPoint::COLOR1, outReflectionsTexture)
           .texture(RenderTarget::AttachmentPoint::COLOR2, outReflectionsTexture);


    for (size_t lod = 0; lod < levels; lod++) {
        SYSTRACE_NAME("executeFilterLOD");

        const float omegaP = (4.0f * f::PI) / float(6 * dim * dim);

        mi->setParameter("kernel", weights, TextureSampler{ SamplerMagFilter::NEAREST });
        mi->setParameter("sampleCount", uint32_t(lod == 0 ? 1u : sampleCount));
        mi->setParameter("attachmentLevel", uint32_t(lod));
        mi->setParameter("invWeightSum", 1.0f / mWeightSum[lod]);
        mi->setParameter("log4OmegaP", log4(omegaP));

        builder.mipLevel(RenderTarget::AttachmentPoint::COLOR0, lod)
               .mipLevel(RenderTarget::AttachmentPoint::COLOR1, lod)
               .mipLevel(RenderTarget::AttachmentPoint::COLOR2, lod);

        view->setViewport({ 0, 0, dim, dim });

        for (size_t i = 0; i < 2; i++) {
            mi->setParameter("side", i == 0 ? 1.0f : -1.0f);

            builder.face(RenderTarget::AttachmentPoint::COLOR0, faces[i][0])
                   .face(RenderTarget::AttachmentPoint::COLOR1, faces[i][1])
                   .face(RenderTarget::AttachmentPoint::COLOR2, faces[i][2]);

            RenderTarget* const rt = builder.build(engine);
            view->setRenderTarget(rt);
            renderer->renderStandaloneView(view);
            engine.destroy(rt);
        }

        dim >>= 1;
    }

    engine.destroy(mi);

//    {
//        SYSTRACE_NAME("flushAndWait");
//        engine.flushAndWait();
//    }

    return outReflectionsTexture;
}
