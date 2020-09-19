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

ShadowMapManager::ShadowMapManager(FEngine& engine) {
    for (auto& entry : mCascadeShadowMapCache) {
        entry = std::make_unique<ShadowMap>(engine);
    }
    for (auto& entry : mSpotShadowMapCache) {
        entry = std::make_unique<ShadowMap>(engine);
    }
    FDebugRegistry& debugRegistry = engine.getDebugRegistry();
    debugRegistry.registerProperty("d.shadowmap.visualize_cascades",
            &engine.debug.shadowmap.visualize_cascades);
    debugRegistry.registerProperty("d.shadowmap.tightly_bound_scene",
            &engine.debug.shadowmap.tightly_bound_scene);
}

ShadowMapManager::~ShadowMapManager() = default;

ShadowMapManager::ShadowTechnique ShadowMapManager::update(
        FEngine& engine, FView& view, UniformBuffer& perViewUb,
        UniformBuffer& shadowUb, FScene::RenderableSoa& renderableData,
        FScene::LightSoa& lightData) noexcept {
    calculateTextureRequirements(engine, lightData);
    ShadowTechnique shadowTechnique = {};
    shadowTechnique |= updateCascadeShadowMaps(engine, view, perViewUb, renderableData, lightData);
    shadowTechnique |= updateSpotShadowMaps(engine, view, shadowUb, renderableData, lightData);
    return shadowTechnique;
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

void ShadowMapManager::render(FrameGraph& fg, FEngine& engine, FView& view,
        backend::DriverApi& driver, RenderPass& pass) noexcept {
    constexpr size_t MAX_SHADOW_LAYERS =
        CONFIG_MAX_SHADOW_CASCADES + CONFIG_MAX_SHADOW_CASTING_SPOTS;
    struct ShadowPassData {
        FrameGraphId<FrameGraphTexture> shadows;
        FrameGraphId<FrameGraphTexture> tempShadow;
        FrameGraphRenderTargetHandle rt[MAX_SHADOW_LAYERS];
    };

    using ShadowPass = std::pair<const ShadowMapEntry*, RenderPass>;
    std::vector<ShadowPass> passes;
    passes.reserve(MAX_SHADOW_LAYERS);

    // These loops fill render passes with appropriate rendering commands for each shadow map.
    // The actual render pass execution is deferred to the frame graph.
    for (const auto& map : mCascadeShadowMaps) {
        if (!map.hasVisibleShadows()) {
            continue;
        }

        map.getShadowMap()->render(driver, view.getVisibleDirectionalShadowCasters(), pass, view);

        assert(map.getLayout().layer < mTextureRequirements.layers);
        passes.emplace_back(&map, pass);
    }
    for (size_t i = 0; i < mSpotShadowMaps.size(); i++) {
        const auto& map = mSpotShadowMaps[i];
        if (!map.hasVisibleShadows()) {
            continue;
        }

        pass.setVisibilityMask(VISIBLE_SPOT_SHADOW_CASTER_N(i));
        map.getShadowMap()->render(driver, view.getVisibleSpotShadowCasters(), pass, view);
        pass.clearVisibilityMask();

        assert(map.getLayout().layer < mTextureRequirements.layers);
        passes.emplace_back(&map, pass);
    }
    assert(passes.size() <= mTextureRequirements.layers);

    const bool fillWithCheckerboard = engine.debug.shadowmap.checkerboard && !view.hasVsm();

    auto& shadowPass = fg.addPass<ShadowPassData>("Shadow Pass",
            [&](FrameGraph::Builder& builder, auto& data) {
                FrameGraphTexture::Descriptor shadowTexture {
                    .width = mTextureRequirements.size, .height = mTextureRequirements.size,
                    .depth = mTextureRequirements.layers,
                    .levels = 1,
                    .type = SamplerType::SAMPLER_2D_ARRAY,
                    .format = mTextureFormat,
                    .usage = TextureUsage::DEPTH_ATTACHMENT | TextureUsage::SAMPLEABLE
                        | (fillWithCheckerboard ? TextureUsage::UPLOADABLE : (TextureUsage) 0)
                };

                if (view.hasVsm()) {
                    // TODO: support 16-bit VSM depth textures.
                    shadowTexture.format = TextureFormat::RG32F;
                    shadowTexture.usage = TextureUsage::COLOR_ATTACHMENT | TextureUsage::SAMPLEABLE;
                }

                data.shadows = builder.createTexture("Shadow Texture", shadowTexture);
                data.shadows = builder.write(data.shadows);

                if (view.hasVsm()) {
                    data.tempShadow = builder.createTexture("Temporary Shadow Texture", {
                        .width = mTextureRequirements.size, .height = mTextureRequirements.size,
                        .depth = 1,
                        .levels = 1,
                        .type = SamplerType::SAMPLER_2D,
                        .format = TextureFormat::DEPTH16,
                        .usage = TextureUsage::DEPTH_ATTACHMENT
                    });
                    // We specify "read" for the temporary shadow texture, so it isn't culled.
                    data.tempShadow = builder.write(builder.read(data.tempShadow));
                }

                // Create a render target for each layer of the texture array.
                for (uint8_t i = 0u; i < mTextureRequirements.layers; i++) {
                    FrameGraphRenderTarget::Descriptor renderTarget {};
                    if (view.hasVsm()) {
                        renderTarget.attachments = { { data.shadows, 0u, i }, { data.tempShadow } };
                        renderTarget.clearFlags = TargetBufferFlags::COLOR | TargetBufferFlags::DEPTH;
                    } else {
                        renderTarget.attachments = { {}, { data.shadows, 0u, i } };
                        renderTarget.clearFlags = TargetBufferFlags::DEPTH;
                    }
                    data.rt[i] = builder.createRenderTarget("Shadow RT", renderTarget);
                }
            },
            [=, passes = std::move(passes), &view, &engine](FrameGraphPassResources const& resources,
                    auto const& data, DriverApi& driver) mutable {
                for (auto& [map, pass] : passes) {
                    FCamera const& camera = map->getShadowMap()->getCamera();
                    filament::CameraInfo cameraInfo(camera);
                    view.prepareCamera(cameraInfo);

                    // we set a viewport with a 1-texel border for when we index outside of the
                    // texture
                    // DON'T CHANGE this unless ShadowMap::getTextureCoordsMapping() is updated too.
                    // see: ShadowMap::getTextureCoordsMapping()
                    // For floating-point depth textures, the 1-texel border could be set to
                    // FLOAT_MAX to avoid clamping in the shadow shader (see sampleDepth inside
                    // shadowing.fs). Unfortunately, the APIs don't seem let us clear depth
                    // attachments to anything greater than 1.0, so we'd need a way to do this other
                    // than clearing.
                    const uint32_t dim = map->getLayout().size;
                    filament::Viewport viewport { 1, 1, dim - 2, dim - 2 };
                    view.prepareViewport(viewport);

                    view.commitUniforms(driver);

                    const auto layer = map->getLayout().layer;
                    auto rt = resources.get(data.rt[layer]);
                    rt.params.viewport = viewport;

                    auto polygonOffset = map->getShadowMap()->getPolygonOffset();
                    pass.overridePolygonOffset(&polygonOffset);

                    pass.execute("Shadow Pass", rt.target, rt.params);
                }

                engine.flush(); // Wake-up the driver thread
            });

    auto shadows = shadowPass.getData().shadows;

    if (UTILS_UNLIKELY(fillWithCheckerboard)) {
        struct DebugPatternData {
            FrameGraphId<FrameGraphTexture> shadows;
        };

        auto& debugPatternPass = fg.addPass<DebugPatternData>("Shadow Debug Pattern Pass",
                [&](FrameGraph::Builder& builder, DebugPatternData& data) {
                    assert(shadows.isValid());
                    data.shadows = builder.write(shadows);
                },
                [=](FrameGraphPassResources const& resources, DebugPatternData const& data,
                    DriverApi& driver) {
                    fillWithDebugPattern(driver, resources.getTexture(data.shadows),
                            mTextureRequirements.size);
                });

        shadows = debugPatternPass.getData().shadows;
    }

    fg.getBlackboard().put("shadows", shadows);
}

void ShadowMapManager::prepareShadow(backend::Handle<backend::HwTexture> texture,
        backend::SamplerGroup& viewSib) const noexcept {
    viewSib.setSampler(PerViewSib::SHADOW_MAP, {
            texture, {
                    .filterMag = SamplerMagFilter::LINEAR,
                    .filterMin = SamplerMinFilter::LINEAR,
                    .compareMode = SamplerCompareMode::COMPARE_TO_TEXTURE,
                    .compareFunc = SamplerCompareFunc::GE
            }});
}

ShadowMapManager::ShadowTechnique ShadowMapManager::updateCascadeShadowMaps(
        FEngine& engine, FView& view,
        UniformBuffer& perViewUb, FScene::RenderableSoa& renderableData,
        FScene::LightSoa& lightData) noexcept {
    FScene* scene = view.getScene();
    const CameraInfo& viewingCameraInfo = view.getCameraInfo();
    uint8_t visibleLayers = view.getVisibleLayers();
    const uint16_t textureSize = mTextureRequirements.size;
    auto& lcm = engine.getLightManager();

    FLightManager::Instance directionalLight = lightData.elementAt<FScene::LIGHT_INSTANCE>(0);
    LightManager::ShadowOptions const& options = lcm.getShadowOptions(directionalLight);

    ShadowMap::CascadeParameters cascadeParams;

    if (!mCascadeShadowMaps.empty()) {
        // Compute scene-dependent values shared across all cascades.
        ShadowMap::computeSceneCascadeParams(lightData, 0, view, viewingCameraInfo, visibleLayers,
                cascadeParams);

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
                layout, cascadeParams);
        Frustum const& frustum = map.getCamera().getFrustum();
        FView::cullRenderables(engine.getJobSystem(), renderableData, frustum,
                VISIBLE_DIR_SHADOW_CASTER_BIT);

        // Set shadowBias, using the first directional cascade.
        const float texelSizeWorldSpace = map.getTexelSizeWorldSpace();
        const float normalBias = lcm.getShadowNormalBias(0);
        perViewUb.setUniform(offsetof(PerViewUib, shadowBias),
                float3{0, normalBias * texelSizeWorldSpace, 0});

        // Set the directional light position. Only used when VSM is active.
        perViewUb.setUniform(offsetof(PerViewUib, lightPosition), cascadeParams.wsLightPosition);
    }

    // Adjust the near and far planes to tighly bound the scene.
    float vsNear = -viewingCameraInfo.zn;
    float vsFar = -viewingCameraInfo.zf;
    if (engine.debug.shadowmap.tightly_bound_scene) {
        vsNear = std::min(vsNear, cascadeParams.vsNearFar.x);
        vsFar = std::max(vsFar, cascadeParams.vsNearFar.y);
    }

    const size_t cascadeCount = mCascadeShadowMaps.size();

    // We divide the camera frustum into N cascades. This gives us N + 1 split positions.
    // The first split position is the near plane; the last split position is the far plane.
    std::array<float, CascadeSplits::SPLIT_COUNT> splitPercentages{};
    splitPercentages[0] = 0.0f;
    size_t i = 1;
    for (; i < cascadeCount; i++) {
        splitPercentages[i] = options.cascadeSplitPositions[i - 1];
    }
    splitPercentages[i] = 1.0f;

    const CascadeSplits::Params p {
        .proj = viewingCameraInfo.cullingProjection,
        .near = vsNear,
        .far = vsFar,
        .cascadeCount = cascadeCount,
        .splitPositions = splitPercentages
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

    ShadowTechnique shadowTechnique{};
    uint32_t directionalShadowsMask = 0;
    uint32_t cascadeHasVisibleShadows = 0;
    float screenSpaceShadowDistance = 0.0f;
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
        cascadeParams.csNearFar = { csSplitPosition[i], csSplitPosition[i + 1] };
        shadowMap.update(lightData, 0, scene, viewingCameraInfo, visibleLayers, layout, cascadeParams);
        if (shadowMap.hasVisibleShadows()) {
            entry.setHasVisibleShadows(true);

            mat4f const& lightFromWorldMatrix = shadowMap.getLightSpaceMatrix();
            perViewUb.setUniform(offsetof(PerViewUib, lightFromWorldMatrix) +
                    sizeof(mat4f) * i, lightFromWorldMatrix);

            shadowTechnique |= ShadowTechnique::SHADOW_MAP;
            cascadeHasVisibleShadows |= 0x1u << i;
        }
    }

    // screen-space contact shadows for the directional light
    screenSpaceShadowDistance = options.maxShadowDistance;
    if (options.screenSpaceContactShadows) {
        shadowTechnique |= ShadowTechnique::SCREEN_SPACE;
    }
    directionalShadowsMask |= std::min(uint8_t(255u), options.stepCount) << 8u;

    if (any(shadowTechnique & ShadowTechnique::SHADOW_MAP)) {
        directionalShadowsMask |= 0x1u;
    }
    if (any(shadowTechnique & ShadowTechnique::SCREEN_SPACE)) {
        directionalShadowsMask |= 0x2u;
    }

    perViewUb.setUniform(offsetof(PerViewUib, directionalShadows), directionalShadowsMask);
    perViewUb.setUniform(offsetof(PerViewUib, ssContactShadowDistance), screenSpaceShadowDistance);

    uint32_t cascades = 0;
    if (engine.debug.shadowmap.visualize_cascades) {
        cascades |= 0x10u;
    }
    cascades |= uint32_t(mCascadeShadowMaps.size());
    cascades |= cascadeHasVisibleShadows << 8u;
    perViewUb.setUniform(offsetof(PerViewUib, cascades), cascades);

    return shadowTechnique;
}

ShadowMapManager::ShadowTechnique ShadowMapManager::updateSpotShadowMaps(
        FEngine& engine, FView& view, UniformBuffer& shadowUb,
        FScene::RenderableSoa& renderableData, FScene::LightSoa& lightData) noexcept {

    ShadowTechnique shadowTechnique{};
    FScene* scene = view.getScene();
    const CameraInfo& viewingCameraInfo = view.getCameraInfo();
    uint8_t visibleLayers = view.getVisibleLayers();
    const uint16_t textureSize = mTextureRequirements.size;

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

            shadowTechnique |= ShadowTechnique::SHADOW_MAP;
        }
    }

    // screen-space contact shadows for point/spot lights
    auto *pInstance = lightData.data<FScene::LIGHT_INSTANCE>();
    for (size_t i = 0, c = lightData.size(); i < c; i++) {
        // screen-space contact shadows
        LightManager::ShadowOptions const& shadowOptions = lcm.getShadowOptions(pInstance[i]);
        if (shadowOptions.screenSpaceContactShadows) {
            shadowTechnique |= ShadowTechnique::SCREEN_SPACE;
            shadowInfo[i].contactShadows = true;
            // TODO: distance/steps are always taken from the directional light currently
        }
    }

    return shadowTechnique;
}

UTILS_NOINLINE
void ShadowMapManager::fillWithDebugPattern(backend::DriverApi& driverApi,
        Handle<HwTexture> texture, size_t dim) noexcept {
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

void ShadowMapManager::calculateTextureRequirements(FEngine& engine,
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
    uint16_t maxDimension = 0;
    for (auto& cascade : mCascadeShadowMaps) {
        // Shadow map size should be the same for all cascades.
        const uint16_t dim = getShadowMapSize(cascade.getLightIndex());
        maxDimension = std::max(maxDimension, dim);
        cascade.setLayout({
            .layer = layer++,
            .size = dim
        });
    }
    for (auto& spotShadowMap : mSpotShadowMaps) {
        const uint16_t dim = getShadowMapSize(spotShadowMap.getLightIndex());
        maxDimension = std::max(maxDimension, dim);
        spotShadowMap.setLayout({
            .layer = layer++,
            .size = dim
        });
    }

    const uint8_t layersNeeded = layer;
    mTextureRequirements = {
        maxDimension,
        layersNeeded
    };
}


ShadowMapManager::CascadeSplits::CascadeSplits(Params p) : mSplitCount(p.cascadeCount + 1) {
    for (size_t s = 0; s < mSplitCount; s++) {
        mSplitsWs[s] = p.near + (p.far - p.near) * p.splitPositions[s];
        mSplitsCs[s] = mat4f::project(p.proj, float3(0.0f, 0.0f, mSplitsWs[s])).z;
    }
}

} // namespace filament
