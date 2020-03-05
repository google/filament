/*
 * Copyright (C) 2020 The Android Open Source Project
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

#include "details/ShadowMap.h"
#include "details/ShadowMapManager.h"
#include "details/View.h"

#include "RenderPass.h"

#include <private/filament/SibGenerator.h>

namespace filament {

using namespace backend;
using namespace math;

namespace details {

ShadowMapManager::ShadowMapManager() : mTextureState(0, 0) {}

ShadowMapManager::~ShadowMapManager() {
    assert(mRenderTargets.empty());
}

void ShadowMapManager::terminate(DriverApi& driverApi) noexcept {
    destroyResources(driverApi);
}

void ShadowMapManager::prepare(FEngine& engine, DriverApi& driver, SamplerGroup& samplerGroup,
        FScene::LightSoa& lightData) noexcept {
    auto& lcm = engine.getLightManager();

    auto getShadowMapSize = [&](size_t lightIndex) {
        FLightManager::Instance light = lightData.elementAt<FScene::LIGHT_INSTANCE>(lightIndex);
        // The minimum size is 3 texels, as we require a 1 texel border.
        return std::max(3u, lcm.getShadowMapSize(light));
    };

    // Lay out the shadow maps. For now, we take the largest requested dimension and allocate a
    // texture of that size. Each shadow map gets its own layer in the array texture. The
    // directional shadow map is always on layer 0.
    uint8_t layer = 0;
    uint32_t maxDimension = 0;
    if (mDirectionalShadowMap) {
        uint32_t dim = getShadowMapSize(mDirectionalShadowMap.getLightIndex());
        maxDimension = std::max(maxDimension, dim);
        mDirectionalShadowMap.setLayout({
            .layer = layer++,
            .size = dim
        });
    }
    for (auto & spotShadowMap : mSpotShadowMaps) {
        uint32_t dim = getShadowMapSize(spotShadowMap.getLightIndex());
        maxDimension = std::max(maxDimension, dim);
        spotShadowMap.setLayout({
            .layer = layer++,
            .size = dim
        });
    }

    const uint16_t layersNeeded = layer;
    const uint32_t dim = maxDimension;

    if (layersNeeded == 0) {
        return;
    }

    // If we already have a texture with the same dimensions and layer count, there's no need to
    // create a new one.
    const TextureState newState(dim, layersNeeded);
    if (mTextureState == newState) {
        // nothing to do here.
        assert(mShadowMapTexture);
        return;
    }

    // destroy the current rendertargets and texture
    destroyResources(driver);

    mShadowMapTexture = driver.createTexture(
            SamplerType::SAMPLER_2D_ARRAY, 1, mTextureFormat, 1, dim, dim, layersNeeded,
            TextureUsage::DEPTH_ATTACHMENT |TextureUsage::SAMPLEABLE);
    mTextureState = newState;

    // Create a render target, one for each layer.
    for (uint16_t l = 0; l < layersNeeded; l++) {
        Handle<HwRenderTarget> rt = driver.createRenderTarget(
                TargetBufferFlags::DEPTH, dim, dim, 1,
                {}, { mShadowMapTexture, 0, l }, {});
        mRenderTargets.push_back(rt);
    }

    samplerGroup.setSampler(PerViewSib::SHADOW_MAP, {
            mShadowMapTexture, {
                    .filterMag = SamplerMagFilter::LINEAR,
                    .filterMin = SamplerMinFilter::LINEAR,
                    .compareMode = SamplerCompareMode::COMPARE_TO_TEXTURE,
                    .compareFunc = SamplerCompareFunc::LE
            }});
}

void ShadowMapManager::destroyResources(DriverApi& driver) noexcept {
    for (auto r : mRenderTargets) {
        driver.destroyRenderTarget(r);
    }
    mRenderTargets.clear();
    if (mShadowMapTexture) {
        driver.destroyTexture(mShadowMapTexture);
    }
}

bool ShadowMapManager::update(FEngine& engine, FView& view, UniformBuffer& perViewUb,
        UniformBuffer& shadowUb, FScene::RenderableSoa& renderableData,
        FScene::LightSoa& lightData) noexcept {
    bool hasShadowing = false;

    FScene* scene = view.getScene();
    const CameraInfo& viewingCameraInfo = view.getCameraInfo();
    uint8_t visibleLayers = view.getVisibleLayers();
    const uint16_t textureSize = mTextureState.size;

    auto& lcm = engine.getLightManager();
    if (mDirectionalShadowMap) {
        // Compute the frustum for the directional light.
        ShadowMap& shadowMap = *mDirectionalShadowMap.getShadowMap();
        size_t l = mDirectionalShadowMap.getLightIndex();
        assert(l == 0);

        const size_t textureDimension = mDirectionalShadowMap.getLayout().size;
        const ShadowMap::ShadowMapLayout layout{
                .zResolution = mTextureZResolution,
                .atlasDimension = textureSize,
                .textureDimension = textureDimension,
                .shadowDimension = textureDimension - 2
        };
        shadowMap.update(lightData, 0, scene, viewingCameraInfo, visibleLayers, layout);
        if (shadowMap.hasVisibleShadows()) {
            mDirectionalShadowMap.setHasVisibleShadows(true);

            // Cull shadow casters
            UniformBuffer& u = perViewUb;
            Frustum const& frustum = shadowMap.getCamera().getFrustum();
            FView::cullRenderables(engine.getJobSystem(), renderableData, frustum,
                    VISIBLE_DIR_SHADOW_CASTER_BIT);

            mat4f const& lightFromWorldMatrix = shadowMap.getLightSpaceMatrix();
            u.setUniform(offsetof(PerViewUib, lightFromWorldMatrix), lightFromWorldMatrix);

            FLightManager::Instance light = lightData.elementAt<FScene::LIGHT_INSTANCE>(l);
            const float texelSizeWorldSpace = shadowMap.getTexelSizeWorldSpace();
            const float normalBias = lcm.getShadowNormalBias(light);
            u.setUniform(offsetof(PerViewUib, shadowBias),
                    float3{0, normalBias * texelSizeWorldSpace, 0});

            hasShadowing = true;
        }
    }

    perViewUb.setUniform(offsetof(PerViewUib, directionalShadows),
            mDirectionalShadowMap && mDirectionalShadowMap.getShadowMap()->hasVisibleShadows());

    FScene::ShadowInfo* const shadowInfo = lightData.data<FScene::SHADOW_INFO>();

    for (size_t i = 0; i < mSpotShadowMaps.size(); i++) {
        auto& entry = mSpotShadowMaps[i];

        // compute the frustum for this light
        ShadowMap& shadowMap = *entry.getShadowMap();
        size_t l = mSpotShadowMaps[i].getLightIndex();

        const size_t textureDimension = mSpotShadowMaps[i].getLayout().size;
        const ShadowMap::ShadowMapLayout layout{
                .zResolution = mTextureZResolution,
                .atlasDimension = textureSize,
                .textureDimension = textureDimension,
                .shadowDimension = textureDimension - 2
        };
        shadowMap.update(lightData, l, scene, viewingCameraInfo, visibleLayers, layout);
        if (shadowMap.hasVisibleShadows()) {
            entry.setHasVisibleShadows(true);

            // Cull shadow casters
            UniformBuffer& u = shadowUb;
            Frustum const& frustum = shadowMap.getCamera().getFrustum();
            FView::cullRenderables(engine.getJobSystem(), renderableData, frustum,
                    VISIBLE_SPOT_SHADOW_CASTER_N_BIT(i));

            mat4f const& lightFromWorldMatrix = shadowMap.getLightSpaceMatrix();
            u.setUniform(offsetof(ShadowUib, spotLightFromWorldMatrix) +
                         sizeof(mat4f) * i, lightFromWorldMatrix);

            shadowInfo[l].castsShadows = true;
            shadowInfo[l].index = i;
            shadowInfo[l].layer = mSpotShadowMaps[i].getLayout().layer;

            FLightManager::Instance light = lightData.elementAt<FScene::LIGHT_INSTANCE>(l);
            const float3 dir = lightData.elementAt<FScene::DIRECTION>(l);
            const float texelSizeWorldSpace = shadowMap.getTexelSizeWorldSpace();
            const float normalBias = lcm.getShadowNormalBias(light);
            u.setUniform(offsetof(ShadowUib, directionShadowBias) + sizeof(float4) * i,
                    float4{ dir.x, dir.y, dir.z, normalBias * texelSizeWorldSpace });

            hasShadowing = true;
        }
    }

    return hasShadowing;
}

void ShadowMapManager::reset() noexcept {
    mDirectionalShadowMap = {};
    mSpotShadowMaps.clear();
}

void ShadowMapManager::setDirectionalShadowMap(ShadowMap& shadowMap, size_t lightIndex) noexcept {
    mDirectionalShadowMap = {&shadowMap, lightIndex};
}

void ShadowMapManager::addSpotShadowMap(ShadowMap& shadowMap, size_t lightIndex) noexcept {
    mSpotShadowMaps.emplace_back(&shadowMap, lightIndex);
}

void ShadowMapManager::render(FEngine& engine, FView& view, backend::DriverApi& driver,
        RenderPass& pass) noexcept {
    if (UTILS_UNLIKELY(engine.debug.shadowmap.checkerboard)) {
        // TODO: eventually this will be handled as a optional pass in the framegraph
        fillWithDebugPattern(driver, mShadowMapTexture);
        return;
    }

    size_t currentRt = 0;
    if (mDirectionalShadowMap) {
        if (mDirectionalShadowMap.hasVisibleShadows()) {
            const uint32_t dim = mDirectionalShadowMap.getLayout().size;
            filament::Viewport viewport{1, 1, dim - 2, dim - 2};
            mDirectionalShadowMap.getShadowMap()->render(driver, mRenderTargets[currentRt],
                    viewport, view.getVisibleDirectionalShadowCasters(), pass, view);
        }
        currentRt++;
    }
    assert(mShadowMapTexture);
    for (size_t i = 0; i < mSpotShadowMaps.size(); i++, currentRt++) {
        const auto& map = mSpotShadowMaps[i];
        if (!map.hasVisibleShadows()) {
            continue;
        }
        const uint32_t dim = map.getLayout().size;
        // we set a viewport with a 1-texel border for when we index outside of the texture
        // DON'T CHANGE this unless ShadowMap::getTextureCoordsMapping() is updated too.
        // see: ShadowMap::getTextureCoordsMapping()
        // For floating-point depth textures, the 1-texel border could be set to FLOAT_MAX to avoid
        // clamping in the shadow shader (see sampleDepth inside shadowing.fs). Unfortunately, the APIs
        // don't seem let us clear depth attachments to anything greater than 1.0, so we'd need a way to
        // do this other than clearing.
        filament::Viewport viewport {1, 1, dim - 2, dim - 2};
        pass.setVisibilityMask(VISIBLE_SPOT_SHADOW_CASTER_N(i));
        map.getShadowMap()->render(driver, mRenderTargets[currentRt], viewport,
                view.getVisibleSpotShadowCasters(), pass, view);
        pass.clearVisibilityMask();
    }
}

UTILS_NOINLINE
void ShadowMapManager::fillWithDebugPattern(backend::DriverApi& driverApi,
        Handle<HwTexture> texture) const noexcept {
    const size_t dim = mTextureState.size;
    size_t size = dim * dim;
    uint8_t* ptr = (uint8_t*)malloc(size);
    // TODO: this only fills the first layer of the shadow map texture.
    driverApi.update2DImage(texture, 0, 0, 0, dim, dim, {
            ptr, size, PixelDataFormat::DEPTH_COMPONENT, PixelDataType::UBYTE,
            [] (void* buffer, size_t, void*) { free(buffer); }
    });
    for (size_t y = 0; y < dim; ++y) {
        for (size_t x = 0; x < dim; ++x) {
            ptr[x + y * dim] = ((x ^ y) & 0x8u) ? 0u : 0xFFu;
        }
    }
}

}
}
