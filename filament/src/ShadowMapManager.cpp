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

ShadowMapManager::ShadowMapManager(FEngine& engine) : mTextureState(0, 0) {
    for (size_t i = 0; i < mCascadeShadowMapCache.size(); i++) {
        mCascadeShadowMapCache[i] = std::make_unique<ShadowMap>(engine);
    }
    for (size_t i = 0; i < mSpotShadowMapCache.size(); i++) {
        mSpotShadowMapCache[i] = std::make_unique<ShadowMap>(engine);
    }
    FDebugRegistry& debugRegistry = engine.getDebugRegistry();
    debugRegistry.registerProperty("d.shadowmap.visualize_cascades",
            &engine.debug.shadowmap.visualize_cascades);
}

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
    // texture of that size. Each cascade / shadow map gets its own layer in the array texture.
    // The directional shadow cascades start on layer 0, followed by spot lights.
    uint8_t layer = 0;
    uint32_t maxDimension = 0;
    for (auto& cascade : mCascadeShadowMaps) {
        // Shadow map size should be the same for all cascades.
        const uint32_t dim = getShadowMapSize(cascade.getLightIndex());
        maxDimension = std::max(maxDimension, dim);
        cascade.setLayout({
            .layer = layer++,
            .size = dim
        });
    }
    for (auto& spotShadowMap : mSpotShadowMaps) {
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

bool ShadowMapManager::update(FEngine& engine, FView& view, UniformBuffer& perViewUb,
        UniformBuffer& shadowUb, FScene::RenderableSoa& renderableData,
        FScene::LightSoa& lightData) noexcept {
    bool hasShadowing = false;
    hasShadowing |= updateCascadeShadowMaps(engine, view, perViewUb, renderableData, lightData);
    hasShadowing |= updateSpotShadowMaps(engine, view, shadowUb, renderableData, lightData);
    return hasShadowing;
}

void ShadowMapManager::reset() noexcept {
    mCascadeShadowMaps.clear();
    mSpotShadowMaps.clear();
}

void ShadowMapManager::setShadowCascades(size_t lightIndex, size_t cascades) noexcept {
    assert(cascades <= CONFIG_MAX_SHADOW_CASCADES);
    for (size_t c = 0; c < cascades; c++) {
        mCascadeShadowMaps.emplace_back(mCascadeShadowMapCache[c].get(), lightIndex);
    }
}

void ShadowMapManager::addSpotShadowMap(size_t lightIndex) noexcept {
    const size_t maps = mSpotShadowMaps.size();
    assert(maps < CONFIG_MAX_SHADOW_CASTING_SPOTS);
    mSpotShadowMaps.emplace_back(mSpotShadowMapCache[maps].get(), lightIndex);
}

void ShadowMapManager::render(FEngine& engine, FView& view, backend::DriverApi& driver,
        RenderPass& pass) noexcept {
    if (UTILS_UNLIKELY(engine.debug.shadowmap.checkerboard)) {
        // TODO: eventually this will be handled as a optional pass in the framegraph
        fillWithDebugPattern(driver, mShadowMapTexture);
        return;
    }

    size_t currentRt = 0;
    for (size_t i = 0; i < mCascadeShadowMaps.size(); i++, currentRt++) {
        const auto& map = mCascadeShadowMaps[i];
        if (!map.hasVisibleShadows()) {
            continue;
        }

        const uint32_t dim = map.getLayout().size;
        filament::Viewport viewport{1, 1, dim - 2, dim - 2};
        map.getShadowMap()->render(driver, mRenderTargets[currentRt],
                viewport, view.getVisibleDirectionalShadowCasters(), pass, view);
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

bool ShadowMapManager::updateCascadeShadowMaps(FEngine& engine, FView& view,
            UniformBuffer& perViewUb, FScene::RenderableSoa& renderableData,
            FScene::LightSoa& lightData) noexcept {
    bool hasShadowing = false;

    FScene* scene = view.getScene();
    const CameraInfo& viewingCameraInfo = view.getCameraInfo();
    uint8_t visibleLayers = view.getVisibleLayers();
    const uint16_t textureSize = mTextureState.size;
    auto& lcm = engine.getLightManager();

    if (mCascadeShadowMaps.size() > 0) {
        // Even if we have more than one cascade, we cull directional shadow casters against the
        // entire camera frustum, as if we only had a single cascade.
        ShadowMap& map = *mCascadeShadowMaps[0].getShadowMap();
        const size_t textureDimension = mCascadeShadowMaps[0].getLayout().size;
        const ShadowMap::ShadowMapLayout layout {
                .zResolution = mTextureZResolution,
                .atlasDimension = textureSize,
                .textureDimension = textureDimension,
                .shadowDimension = textureDimension - 2
        };
        map.update(lightData, 0, scene, viewingCameraInfo, visibleLayers,
                layout, {});
        Frustum const& frustum = map.getCamera().getFrustum();
        FView::cullRenderables(engine.getJobSystem(), renderableData, frustum,
                VISIBLE_DIR_SHADOW_CASTER_BIT);

        // Set shadowBias, using the first directional cascade.
        const float texelSizeWorldSpace = map.getTexelSizeWorldSpace();
        const float normalBias = lcm.getShadowNormalBias(0);
        perViewUb.setUniform(offsetof(PerViewUib, shadowBias),
                float3{0, normalBias * texelSizeWorldSpace, 0});
    }

    // We divide the camera frustum into N cascades. This gives us N + 1 split positions.
    // The first split position is the near plane; the last split position is the far plane.
    const CascadeSplits::Params p {
        .proj = viewingCameraInfo.cullingProjection,
        .near = -viewingCameraInfo.zn,
        .far = -viewingCameraInfo.zf,
        .cascadeCount = mCascadeShadowMaps.size()
    };
    if (p != mCascadeSplitParams) {
        mCascadeSplits = CascadeSplits(p);
        mCascadeSplitParams = p;
    }

    const CascadeSplits& splits = mCascadeSplits;

    // The split positions uniform is a float4. To save space, we chop off the first split position
    // (which is the near plane, and doesn't need to be communicated to the shaders).
    static_assert(CONFIG_MAX_SHADOW_CASCADES <= 5,
            "At most, a float4 can fit 4 split positions for 5 shadow cascades");
    float4 wsSplitPositionUniform;
    std::fill_n(&wsSplitPositionUniform[0], 4, -std::numeric_limits<float>::infinity());
    std::copy(splits.beginWs() + 1, splits.endWs(), &wsSplitPositionUniform[0]);

    float csSplitPosition[CONFIG_MAX_SHADOW_CASCADES + 1];
    std::copy(splits.beginCs(), splits.endCs(), csSplitPosition);

    // Update cascade split uniform.
    perViewUb.setUniform(offsetof(PerViewUib, cascadeSplits), wsSplitPositionUniform);

    uint32_t directionalShadows = 0;
    uint32_t cascadeHasVisibleShadows = 0;
    float screenSpaceShadowDistance = 0.0;
    for (size_t i = 0; i < mCascadeShadowMaps.size(); i++) {
        auto& entry = mCascadeShadowMaps[i];

        // Compute the frustum for the directional light.
        ShadowMap& shadowMap = *entry.getShadowMap();
        UTILS_UNUSED_IN_RELEASE size_t l = entry.getLightIndex();
        assert(l == 0);

        const size_t textureDimension = entry.getLayout().size;
        const ShadowMap::ShadowMapLayout layout{
                .zResolution = mTextureZResolution,
                .atlasDimension = textureSize,
                .textureDimension = textureDimension,
                .shadowDimension = textureDimension - 2
        };
        ShadowMap::CascadeParameters cascadeParams {
            .csNear = csSplitPosition[i],
            .csFar = csSplitPosition[i + 1]
        };
        shadowMap.update(lightData, 0, scene, viewingCameraInfo, visibleLayers, layout, cascadeParams);
        if (shadowMap.hasVisibleShadows()) {
            entry.setHasVisibleShadows(true);

            mat4f const& lightFromWorldMatrix = shadowMap.getLightSpaceMatrix();
            perViewUb.setUniform(offsetof(PerViewUib, lightFromWorldMatrix) +
                    sizeof(mat4f) * i, lightFromWorldMatrix);

            hasShadowing = true;
            directionalShadows |= 0x1u;
            cascadeHasVisibleShadows |= 0x1u << i;
        }
    }

    // screen-space contact shadows for the directional light
    FLightManager::Instance directionalLight = lightData.elementAt<FScene::LIGHT_INSTANCE>(0);
    LightManager::ShadowOptions const& options = lcm.getShadowOptions(directionalLight);
    screenSpaceShadowDistance = options.maxShadowDistance;
    directionalShadows |= std::min(uint8_t(255u), options.stepCount) << 8u;
    if (options.screenSpaceContactShadows) {
        hasShadowing = true;
        directionalShadows |= 0x2u;
    }

    perViewUb.setUniform(offsetof(PerViewUib, directionalShadows), directionalShadows);
    perViewUb.setUniform(offsetof(PerViewUib, ssContactShadowDistance), screenSpaceShadowDistance);

    uint32_t cascades = 0;
    if (engine.debug.shadowmap.visualize_cascades) {
        cascades |= 0x10u;
    }
    cascades |= uint32_t(mCascadeShadowMaps.size());
    cascades |= cascadeHasVisibleShadows << 8u;
    perViewUb.setUniform(offsetof(PerViewUib, cascades), cascades);

    return hasShadowing;
}

bool ShadowMapManager::updateSpotShadowMaps(FEngine& engine, FView& view, UniformBuffer& shadowUb,
        FScene::RenderableSoa& renderableData, FScene::LightSoa& lightData) noexcept {
    bool hasShadowing = false;

    FScene* scene = view.getScene();
    const CameraInfo& viewingCameraInfo = view.getCameraInfo();
    uint8_t visibleLayers = view.getVisibleLayers();
    const uint16_t textureSize = mTextureState.size;

    // shadow-map shadows for point/spot lights
    auto& lcm = engine.getLightManager();
    FScene::ShadowInfo* const shadowInfo = lightData.data<FScene::SHADOW_INFO>();
    for (size_t i = 0, c = mSpotShadowMaps.size(); i < c; i++) {
        auto& entry = mSpotShadowMaps[i];

        // compute the frustum for this light
        ShadowMap& shadowMap = *entry.getShadowMap();
        size_t l = entry.getLightIndex();

        const size_t textureDimension = entry.getLayout().size;
        const ShadowMap::ShadowMapLayout layout{
                .zResolution = mTextureZResolution,
                .atlasDimension = textureSize,
                .textureDimension = textureDimension,
                .shadowDimension = textureDimension - 2
        };
        shadowMap.update(lightData, l, scene, viewingCameraInfo, visibleLayers, layout, {});

        FLightManager::Instance light = lightData.elementAt<FScene::LIGHT_INSTANCE>(l);
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

            const float3 dir = lightData.elementAt<FScene::DIRECTION>(l);
            const float texelSizeWorldSpace = shadowMap.getTexelSizeWorldSpace();
            const float normalBias = lcm.getShadowNormalBias(light);
            u.setUniform(offsetof(ShadowUib, directionShadowBias) + sizeof(float4) * i,
                    float4{ dir.x, dir.y, dir.z, normalBias * texelSizeWorldSpace });

            hasShadowing = true;
        }
    }

    // screen-space contact shadows for point/spot lights
    auto pInstance = lightData.data<FScene::LIGHT_INSTANCE>();
    for (size_t i = 0, c = lightData.size(); i < c; i++) {
        // screen-space contact shadows
        LightManager::ShadowOptions const& shadowOptions = lcm.getShadowOptions(pInstance[i]);
        if (shadowOptions.screenSpaceContactShadows) {
            hasShadowing = true;
            shadowInfo[i].contactShadows = true;
            // TODO: distance/steps are always taken from the directional light currently
        }
    }

    return hasShadowing;
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

void ShadowMapManager::destroyResources(DriverApi& driver) noexcept {
    for (auto r : mRenderTargets) {
        driver.destroyRenderTarget(r);
    }
    mRenderTargets.clear();
    if (mShadowMapTexture) {
        driver.destroyTexture(mShadowMapTexture);
    }
}

ShadowMapManager::CascadeSplits::CascadeSplits(Params p) : mSplitCount(p.cascadeCount + 1) {
    auto uniformSplit = [](float near, float far, size_t cascades) {
        return [near, far, cascades](size_t split) {
            if (cascades == 0) {
                return 0.0f;
            }
            return near + (far - near) * ((float) split / cascades);
        };
    };
    auto uniformSplitCalculator = uniformSplit(p.near, p.far, p.cascadeCount);
    for (size_t s = 0; s < mSplitCount; s++) {
        mSplitsWs[s] = uniformSplitCalculator(s);
        mSplitsCs[s] = mat4f::project(p.proj, float3(0.0f, 0.0f, mSplitsWs[s])).z;
    }
}

} // namespace filament
