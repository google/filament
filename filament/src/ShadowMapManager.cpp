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

#include "ShadowMapManager.h"

#include "RenderPass.h"
#include "ShadowMap.h"

#include "details/Texture.h"
#include "details/View.h"

#include <fg/FrameGraph.h>

#include <private/filament/SibGenerator.h>

#include <utils/debug.h>
#include <utils/FixedCapacityVector.h>

namespace filament {

using namespace backend;
using namespace math;

ShadowMapManager::ShadowMapManager(FEngine& engine) { // NOLINT(cppcoreguidelines-pro-type-member-init)
    // initialize our ShadowMap array in-place
    UTILS_NOUNROLL
    for (auto& entry : mShadowMapCache) {
        new (&entry) ShadowMap(engine);
    }
    FDebugRegistry& debugRegistry = engine.getDebugRegistry();
    debugRegistry.registerProperty("d.shadowmap.visualize_cascades",
            &engine.debug.shadowmap.visualize_cascades);
    debugRegistry.registerProperty("d.shadowmap.tightly_bound_scene",
            &engine.debug.shadowmap.tightly_bound_scene);
}

ShadowMapManager::~ShadowMapManager() {
    // destroy the ShadowMap array in-place
    UTILS_NOUNROLL
    for (auto& entry : mShadowMapCache) {
        std::launder(reinterpret_cast<ShadowMap*>(&entry))->~ShadowMap();
    }
}

ShadowMapManager::ShadowTechnique ShadowMapManager::update(FEngine& engine, FView& view,
        CameraInfo const& cameraInfo, TypedUniformBuffer<ShadowUib>& shadowUb,
        FScene::RenderableSoa& renderableData, FScene::LightSoa& lightData) noexcept {
    ShadowTechnique shadowTechnique = {};

    calculateTextureRequirements(engine, view, lightData);

    ShadowMap::SceneInfo sceneInfo(view.getVisibleLayers());

    // Compute scene-dependent values shared across all shadow maps
    ShadowMap::initSceneInfo(*view.getScene(), cameraInfo.view, sceneInfo);

    shadowTechnique |= updateCascadeShadowMaps(
            engine, view, cameraInfo, renderableData, lightData, sceneInfo);

    shadowTechnique |= updateSpotShadowMaps(
            engine, view, cameraInfo, renderableData, lightData, sceneInfo, shadowUb);

    return shadowTechnique;
}

void ShadowMapManager::reset() noexcept {
    mCascadeShadowMaps.clear();
    mSpotShadowMaps.clear();
}

void ShadowMapManager::setShadowCascades(size_t lightIndex,
        LightManager::ShadowOptions const* options) noexcept {
    assert_invariant(options->shadowCascades <= CONFIG_MAX_SHADOW_CASCADES);
    for (size_t c = 0; c < options->shadowCascades; c++) {
        auto* shadowMap = getCascadeShadowMap(c);
        mCascadeShadowMaps.emplace_back(shadowMap, lightIndex, options);
    }
}

void ShadowMapManager::addSpotShadowMap(size_t lightIndex,
        LightManager::ShadowOptions const* options) noexcept {
    const size_t c = mSpotShadowMaps.size();
    assert_invariant(c < CONFIG_MAX_SHADOW_CASTING_SPOTS);
    auto* shadowMap = getSpotShadowMap(c);
    mSpotShadowMaps.emplace_back(shadowMap, lightIndex, options);
}

FrameGraphId<FrameGraphTexture> ShadowMapManager::render(FrameGraph& fg,
        FEngine& engine, backend::DriverApi& driver,
        RenderPass const& pass, FView& view) noexcept {

    constexpr size_t MAX_SHADOW_LAYERS =
            CONFIG_MAX_SHADOW_CASCADES + CONFIG_MAX_SHADOW_CASTING_SPOTS;

    const TextureFormat vsmTextureFormat = TextureFormat::RG16F;

    // make a copy here, because it's a very small structure
    const TextureAtlasRequirements textureRequirements = mTextureAtlasRequirements;
    assert_invariant(textureRequirements.layers <= MAX_SHADOW_LAYERS);

    struct ShadowPass {
        ShadowMapEntry const* shadowMapEntry;
        utils::Range<uint32_t> range;
        FScene::VisibleMaskType visibilityMask;
    };

    auto passList = utils::FixedCapacityVector<ShadowPass>::with_capacity(MAX_SHADOW_LAYERS);

    FScene* scene = view.getScene();
    assert_invariant(scene);

    // these loops create a list of the shadow maps that need to be rendered (i.e. that have
    // visible shadows).

    // Directional, cascaded shadowmaps
    auto const directionalShadowCastersRange = view.getVisibleDirectionalShadowCasters();
    if (!directionalShadowCastersRange.empty()) {
        for (const auto& map : mCascadeShadowMaps) {
            if (map.hasVisibleShadows()) {
                passList.push_back({
                    &map, directionalShadowCastersRange, VISIBLE_DIR_SHADOW_RENDERABLE });
            }
        }
    }

    // Spotlight shadowmaps
    auto const spotShadowCastersRange = view.getVisibleSpotShadowCasters();
    if (!spotShadowCastersRange.empty()) {
        for (size_t i = 0, c = mSpotShadowMaps.size(); i < c; i++) {
            const auto& map = mSpotShadowMaps[i];
            if (map.hasVisibleShadows()) {
                passList.push_back({
                    &map, spotShadowCastersRange, VISIBLE_SPOT_SHADOW_RENDERABLE_N(i) });
            }
        }
    }

    assert_invariant(passList.size() <= textureRequirements.layers);

    // -------------------------------------------------------------------------------------------

    struct PrepareShadowPassData {
        FrameGraphId<FrameGraphTexture> shadows;        // the actual shadowmap
    };

    auto& prepareShadowPass = fg.addPass<PrepareShadowPassData>("Prepare Shadow Pass",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.shadows = builder.createTexture("Shadowmap", {
                        .width = textureRequirements.size, .height = textureRequirements.size,
                        .depth = textureRequirements.layers,
                        .levels = textureRequirements.levels,
                        .type = SamplerType::SAMPLER_2D_ARRAY,
                        .format = view.hasVSM() ? vsmTextureFormat : mTextureFormat
                });
            },
            [=](FrameGraphResources const& resources, auto const& data, DriverApi& driver) { });


    // -------------------------------------------------------------------------------------------

    const float vsmMoment2 = std::numeric_limits<half>::max();
    const float vsmMoment1 = std::sqrt(vsmMoment2);
    const float4 vsmClearColor{ vsmMoment1, vsmMoment2, 0.0f, 0.0f };

    struct ShadowPassData {
        FrameGraphId<FrameGraphTexture> tempBlurSrc{};  // temporary shadowmap when blurring
        uint32_t blurRt{};
        uint32_t shadowRt{};
    };

    auto shadows = prepareShadowPass.getData().shadows;

    auto& ppm = engine.getPostProcessManager();

    for (auto const& entry : passList) {
        const auto layer = entry.shadowMapEntry->getLayer();
        const auto* options = entry.shadowMapEntry->getShadowOptions();

        auto& shadowPass = fg.addPass<ShadowPassData>("Shadow Pass",
                [&](FrameGraph::Builder& builder, auto& data) {
                    const bool blur = view.hasVSM() && options->vsm.blurWidth > 0.0f;

                    FrameGraphRenderPass::Descriptor renderTargetDesc{};

                    auto attachment = builder.createSubresource(prepareShadowPass->shadows,
                            "Shadowmap Layer", { .layer = layer });

                    if (view.hasVSM()) {
                        // Each shadow pass has its own sample count, but textures are created with
                        // a default count of 1 because we're using "magic resolve" (sample count is
                        // set on the render target).
                        // When rendering VSM shadow maps, we still need a depth texture for sorting.
                        // We specify the sample count here because we don't need automatic resolve.
                        auto depth = builder.createTexture("Temporary VSM Depth Texture", {
                                .width = textureRequirements.size, .height = textureRequirements.size,
                                .samples = options->vsm.msaaSamples,
                                .format = mTextureFormat,
                        });

                        // Temporary (resolved) texture used to render the shadowmap when blurring
                        // is needed -- it'll be used as the source of the blur.
                        data.tempBlurSrc = builder.createTexture("Temporary Shadowmap", {
                                .width = textureRequirements.size, .height = textureRequirements.size,
                                .format = vsmTextureFormat
                        });

                        depth = builder.write(depth,
                                FrameGraphTexture::Usage::DEPTH_ATTACHMENT);

                        attachment = builder.write(attachment,
                                FrameGraphTexture::Usage::COLOR_ATTACHMENT);

                        renderTargetDesc.attachments.color[0] = attachment;
                        renderTargetDesc.attachments.depth = depth;
                        renderTargetDesc.clearFlags =
                                TargetBufferFlags::COLOR | TargetBufferFlags::DEPTH;
                        // we need to clear the shadow map with the max EVSM moments
                        renderTargetDesc.clearColor = vsmClearColor;
                        renderTargetDesc.samples = options->vsm.msaaSamples;

                        if (blur) {
                            data.tempBlurSrc = builder.write(data.tempBlurSrc,
                                    FrameGraphTexture::Usage::COLOR_ATTACHMENT);

                            data.blurRt = builder.declareRenderPass("Temp Shadow RT", {
                                    .attachments = {
                                            .color = { data.tempBlurSrc },
                                            .depth = depth },
                                    .clearColor = vsmClearColor,
                                    .samples = options->vsm.msaaSamples,
                                    .clearFlags = TargetBufferFlags::COLOR
                                                  | TargetBufferFlags::DEPTH
                            });
                        }
                    } else {
                        // the shadowmap layer
                        attachment = builder.write(attachment,
                                FrameGraphTexture::Usage::DEPTH_ATTACHMENT);
                        renderTargetDesc.attachments.depth = attachment;
                        renderTargetDesc.clearFlags = TargetBufferFlags::DEPTH;
                    }

                    // finally, create the shadowmap render target -- one per layer.
                    data.shadowRt = builder.declareRenderPass("Shadow RT", renderTargetDesc);
                },
                [=, &engine, &view](FrameGraphResources const& resources,
                        auto const& data, DriverApi& driver) {

                    ShadowMap& shadowMap = entry.shadowMapEntry->getShadowMap();
                    const CameraInfo cameraInfo(shadowMap.getCamera());

                    // updatePrimitivesLod must be run before RenderPass::appendCommands.
                    view.updatePrimitivesLod(engine, cameraInfo, scene->getRenderableData(), entry.range);

                    // generate and sort the commands for rendering the shadow map
                    RenderPass entryPass(pass);
                    shadowMap.render(*scene, entry.range, entry.visibilityMask, &entryPass);

                    const auto& executor = entryPass.getExecutor();
                    const bool blur = view.hasVSM() && options->vsm.blurWidth > 0.0f;

                    view.prepareCamera(cameraInfo);

                    // We set a viewport with a 1-texel border for when we index outside the
                    // texture.
                    // DON'T CHANGE this unless ShadowMap::getTextureCoordsMapping() is updated too.
                    // see: ShadowMap::getTextureCoordsMapping()
                    //
                    // For floating-point depth textures, the 1-texel border could be set to
                    // FLOAT_MAX to avoid clamping in the shadow shader (see sampleDepth inside
                    // shadowing.fs). Unfortunately, the APIs don't seem let us clear depth
                    // attachments to anything greater than 1.0, so we'd need a way to do this other
                    // than clearing.
                    const uint32_t dim = options->mapSize;
                    filament::Viewport viewport{ 1, 1, dim - 2, dim - 2 };
                    view.prepareViewport(viewport);

                    // set uniforms needed to render this ShadowMap
                    // Currently these uniforms are owned by View and are global, but eventually
                    // this will set a separate per shadowmap UBO
                    view.prepareShadowMap();

                    view.commitUniforms(driver);

                    // render either directly into the shadowmap, or to the temporary texture for
                    // blurring.
                    auto rt = resources.getRenderPassInfo(blur ? data.blurRt : data.shadowRt);
                    rt.params.viewport = viewport;

                    executor.execute("Shadow Pass", rt.target, rt.params);
                });


        // now emit the blurring passes
        if (view.hasVSM()) {
            const float blurWidth = options->vsm.blurWidth;
            if (blurWidth > 0.0f) {
                const float sigma = (blurWidth + 1.0f) / 6.0f;
                size_t kernelWidth = std::ceil((blurWidth - 5.0f) / 4.0f);
                kernelWidth = kernelWidth * 4 + 5;
                const float ratio = float(kernelWidth + 1) / sigma;
                ppm.gaussianBlurPass(fg,
                        shadowPass->tempBlurSrc,
                        shadows,
                        false, kernelWidth, ratio);
            }

            // If the shadow texture has more than one level, mipmapping was requested, either directly
            // or indirectly via anisotropic filtering.
            // So generate the mipmaps for each layer
            if (textureRequirements.levels > 1) {
                for (size_t level = 0; level < textureRequirements.levels - 1; level++) {
                    const bool finalize = level == textureRequirements.levels - 2;
                    shadows = ppm.vsmMipmapPass(fg, shadows, layer, level,
                            vsmClearColor, finalize);
                }
            }
        }
    }

    return shadows;
}

ShadowMapManager::ShadowTechnique ShadowMapManager::updateCascadeShadowMaps(FEngine& engine,
        FView& view, CameraInfo const& cameraInfo, FScene::RenderableSoa& renderableData,
        FScene::LightSoa& lightData, ShadowMap::SceneInfo& sceneInfo) noexcept {
    FScene* scene = view.getScene();
    auto& lcm = engine.getLightManager();

    FLightManager::Instance directionalLight = lightData.elementAt<FScene::LIGHT_INSTANCE>(0);
    FLightManager::ShadowOptions const& options = lcm.getShadowOptions(directionalLight);
    FLightManager::ShadowParams const& params = lcm.getShadowParams(directionalLight);

    const ShadowMap::ShadowMapInfo shadowMapInfo{
            .atlasDimension   = mTextureAtlasRequirements.size,
            .textureDimension = uint16_t(options.mapSize),
            .shadowDimension  = uint16_t(options.mapSize - 2u),
            .vsm = view.hasVSM(),
            .polygonOffset = { // handle reversed Z
                    .slope    = view.hasVSM() ? 0.0f : -params.options.polygonOffsetSlope,
                    .constant = view.hasVSM() ? 0.0f : -params.options.polygonOffsetConstant
            }
    };

    if (!mCascadeShadowMaps.empty()) {
        // Even if we have more than one cascade, we cull directional shadow casters against the
        // entire camera frustum, as if we only had a single cascade.
        ShadowMapEntry& entry = mCascadeShadowMaps[0];
        ShadowMap& shadowMap = entry.getShadowMap();

        shadowMap.updateDirectional(lightData, 0, cameraInfo, shadowMapInfo, *scene, sceneInfo);

        Frustum const& frustum = shadowMap.getCamera().getCullingFrustum();
        FView::cullRenderables(engine.getJobSystem(), renderableData, frustum,
                VISIBLE_DIR_SHADOW_RENDERABLE_BIT);

        // Set shadowBias, using the first directional cascade.
        // when computing the required bias we need a half-texel size, so we multiply by 0.5 here.
        // note: normalBias is set to zero for VSM
        const float normalBias = shadowMapInfo.vsm ? 0.0f : 0.5f * lcm.getShadowNormalBias(0);
        // Texel size is constant for directional light (although that's not true when LISPSM
        // is used, but in that case we're pretending it is).
        const float wsTexelSize = shadowMap.getTexelSizAtOneMeterWs();
        mShadowMappingUniforms.shadowBias = normalBias * wsTexelSize;
        mShadowMappingUniforms.shadowBulbRadiusLs =
                mSoftShadowOptions.penumbraScale * options.shadowBulbRadius / wsTexelSize;
    }

    // Adjust the near and far planes to tightly bound the scene.
    float vsNear = -cameraInfo.zn;
    float vsFar = -cameraInfo.zf;
    if (engine.debug.shadowmap.tightly_bound_scene) {
        vsNear = std::min(vsNear, sceneInfo.vsNearFar.x);
        vsFar = std::max(vsFar, sceneInfo.vsNearFar.y);
    }

    const size_t cascadeCount = mCascadeShadowMaps.size();

    // We divide the camera frustum into N cascades. This gives us N + 1 split positions.
    // The first split position is the near plane; the last split position is the far plane.
    std::array<float, CascadeSplits::SPLIT_COUNT> splitPercentages{};
    splitPercentages[cascadeCount] = 1.0f;
    for (size_t i = 1 ; i < cascadeCount; i++) {
        splitPercentages[i] = options.cascadeSplitPositions[i - 1];
    }

    const CascadeSplits::Params p {
        .proj = cameraInfo.cullingProjection,
        .near = vsNear,
        .far = vsFar,
        .cascadeCount = cascadeCount,
        .splitPositions = splitPercentages
    };
    if (p != mCascadeSplitParams) {
        mCascadeSplits = CascadeSplits{ p };
        mCascadeSplitParams = p;
    }

    const CascadeSplits& splits = mCascadeSplits;

    // The split positions uniform is a float4. To save space, we chop off the first split position
    // (which is the near plane, and doesn't need to be communicated to the shaders).
    static_assert(CONFIG_MAX_SHADOW_CASCADES <= 5,
            "At most, a float4 can fit 4 split positions for 5 shadow cascades");
    float4 wsSplitPositionUniform{ -std::numeric_limits<float>::infinity() };
    std::copy(splits.beginWs() + 1, splits.endWs(), &wsSplitPositionUniform[0]);

    float csSplitPosition[CONFIG_MAX_SHADOW_CASCADES + 1];
    std::copy(splits.beginCs(), splits.endCs(), csSplitPosition);

    mShadowMappingUniforms.cascadeSplits = wsSplitPositionUniform;

    ShadowTechnique shadowTechnique{};
    uint32_t directionalShadowsMask = 0;
    uint32_t cascadeHasVisibleShadows = 0;
    for (size_t i = 0, c = mCascadeShadowMaps.size(); i < c; i++) {
        auto& entry = mCascadeShadowMaps[i];

        // Compute the frustum for the directional light.
        ShadowMap& shadowMap = entry.getShadowMap();
        assert_invariant(entry.getLightIndex() == 0);

        sceneInfo.csNearFar = { csSplitPosition[i], csSplitPosition[i + 1] };

        shadowMap.updateDirectional(lightData, 0,
                cameraInfo, shadowMapInfo,
                *scene, sceneInfo);

        if (shadowMap.hasVisibleShadows()) {
            mShadowMappingUniforms.lightFromWorldMatrix[i] = shadowMap.getLightSpaceMatrix();
            shadowTechnique |= ShadowTechnique::SHADOW_MAP;
            cascadeHasVisibleShadows |= 0x1u << i;
        }
    }

    // screen-space contact shadows for the directional light
    float screenSpaceShadowDistance = options.maxShadowDistance;
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

    uint32_t cascades = 0;
    if (engine.debug.shadowmap.visualize_cascades) {
        cascades |= 0x10u;
    }
    cascades |= uint32_t(mCascadeShadowMaps.size());
    cascades |= cascadeHasVisibleShadows << 8u;

    mShadowMappingUniforms.directionalShadows = directionalShadowsMask;
    mShadowMappingUniforms.ssContactShadowDistance = screenSpaceShadowDistance;
    mShadowMappingUniforms.cascades = cascades;

    return shadowTechnique;
}

ShadowMapManager::ShadowTechnique ShadowMapManager::updateSpotShadowMaps(FEngine& engine,
        FView& view, CameraInfo const& cameraInfo, FScene::RenderableSoa& renderableData,
        FScene::LightSoa& lightData, ShadowMap::SceneInfo& sceneInfo,
        TypedUniformBuffer<ShadowUib>& shadowUb) noexcept {

    auto& lcm = engine.getLightManager();

    // shadow-map shadows for point/spotlights
    ShadowTechnique shadowTechnique{};
    FScene::ShadowInfo* const shadowInfo = lightData.data<FScene::SHADOW_INFO>();
    for (size_t i = 0, c = mSpotShadowMaps.size(); i < c; i++) {
        auto& entry = mSpotShadowMaps[i];

        // compute the frustum for this light
        ShadowMap& shadowMap = entry.getShadowMap();
        const size_t lightIndex = entry.getLightIndex();
        const FLightManager::Instance li = lightData.elementAt<FScene::LIGHT_INSTANCE>(lightIndex);
        FLightManager::ShadowParams params = lcm.getShadowParams(li);

        FLightManager::ShadowOptions const* const options = entry.getShadowOptions();
        const ShadowMap::ShadowMapInfo shadowMapInfo{
                .atlasDimension = mTextureAtlasRequirements.size,
                .textureDimension = uint16_t(options->mapSize),
                .shadowDimension = uint16_t(options->mapSize - 2u),
                .spotIndex = uint16_t(i),
                .vsm = view.hasVSM(),
                .polygonOffset = { // handle reversed Z
                        .slope    = view.hasVSM() ? 0.0f : -params.options.polygonOffsetSlope,
                        .constant = view.hasVSM() ? 0.0f : -params.options.polygonOffsetConstant
                }
        };

        // for spotlights, we cull shadow casters first because we already know the frustum,
        // this will help us find better near/far plane later

        const auto position  = lightData.elementAt<FScene::POSITION_RADIUS>(lightIndex).xyz;
        const auto direction = lightData.elementAt<FScene::DIRECTION>(lightIndex);
        const auto radius    = lightData.elementAt<FScene::POSITION_RADIUS>(lightIndex).w;
        const auto outerConeAngle = lcm.getSpotLightOuterCone(li);

        const mat4f Mv = ShadowMap::getDirectionalLightViewMatrix(direction, position);
        const mat4f Mp = mat4f::perspective(outerConeAngle * f::RAD_TO_DEG * 2.0f,
                1.0f, 0.01f, radius);
        const mat4f MpMv(math::highPrecisionMultiply(Mp, Mv));
        const Frustum frustum(MpMv);

        // Cull shadow casters
        FView::cullRenderables(engine.getJobSystem(), renderableData, frustum,
                VISIBLE_SPOT_SHADOW_RENDERABLE_N_BIT(i));

        shadowMap.updateSpot(lightData, lightIndex,
                cameraInfo, shadowMapInfo,
                *view.getScene(), sceneInfo);

        if (shadowMap.hasVisibleShadows()) {
            shadowInfo[lightIndex].castsShadows = true;
            shadowInfo[lightIndex].index = i;
            shadowInfo[lightIndex].layer = entry.getLayer();

            const float wsTexelSizeAtOneMeter = shadowMap.getTexelSizAtOneMeterWs();
            // note: normalBias is set to zero for VSM
            const float normalBias = shadowMapInfo.vsm ? 0.0f : options->normalBias;

            auto& s = shadowUb.edit();
            const float n = shadowMap.getCamera().getNear();
            const float f = shadowMap.getCamera().getCullingFar();
            s.shadows[i].lightFromWorldMatrix = shadowMap.getLightSpaceMatrix();
            s.shadows[i].direction = direction;
            s.shadows[i].normalBias = normalBias * wsTexelSizeAtOneMeter;
            s.shadows[i].lightFromWorldZ = shadowMap.getLightFromWorldZ();
            s.shadows[i].texelSizeAtOneMeter = wsTexelSizeAtOneMeter;
            s.shadows[i].nearOverFarMinusNear = n / (f - n);
            s.shadows[i].bulbRadiusLs =
                    mSoftShadowOptions.penumbraScale * options->shadowBulbRadius / wsTexelSizeAtOneMeter;

            shadowTechnique |= ShadowTechnique::SHADOW_MAP;
        }
    }

    // screen-space contact shadows for point/spotlights
    auto *pLightInstances = lightData.data<FScene::LIGHT_INSTANCE>();
    for (size_t i = 0, c = lightData.size(); i < c; i++) {
        // screen-space contact shadows
        LightManager::ShadowOptions const& shadowOptions = lcm.getShadowOptions(pLightInstances[i]);
        if (shadowOptions.screenSpaceContactShadows) {
            shadowTechnique |= ShadowTechnique::SCREEN_SPACE;
            shadowInfo[i].contactShadows = true;
            // TODO: distance/steps are always taken from the directional light currently
        }
    }

    return shadowTechnique;
}

void ShadowMapManager::calculateTextureRequirements(FEngine& engine, FView& view,
        FScene::LightSoa& lightData) noexcept {

    // Lay out the shadow maps. For now, we take the largest requested dimension and allocate a
    // texture of that size. Each cascade / shadow map gets its own layer in the array texture.
    // The directional shadow cascades start on layer 0, followed by spotlights.
    uint8_t layer = 0;
    uint32_t maxDimension = 0;
    for (auto& entry : mCascadeShadowMaps) {
        // Shadow map size should be the same for all cascades.
        auto const& options = entry.getShadowOptions();
        maxDimension = std::max(maxDimension, options->mapSize);
        entry.setLayer(layer++);
    }
    for (auto& entry : mSpotShadowMaps) {
        auto const& options = entry.getShadowOptions();
        maxDimension = std::max(maxDimension, options->mapSize);
        entry.setLayer(layer++);
    }

    const uint8_t layersNeeded = layer;

    // Generate mipmaps for VSM when anisotropy is enabled or when requested
    auto const& vsmShadowOptions = view.getVsmShadowOptions();
    const bool useMipmapping = view.hasVSM() &&
                               ((vsmShadowOptions.anisotropy > 0) || vsmShadowOptions.mipmapping);

    mSoftShadowOptions = view.getSoftShadowOptions();

    uint8_t mipLevels = 1u;
    if (useMipmapping) {
        // Limit the lowest mipmap level to 256x256.
        // This avoids artifacts on high derivative tangent surfaces.
        int lowMipmapLevel = 7;    // log2(256) - 1
        mipLevels = std::max(1, FTexture::maxLevelCount(maxDimension) - lowMipmapLevel);
    }

    mTextureAtlasRequirements = {
            (uint16_t)maxDimension,
            layersNeeded,
            mipLevels
    };
}

ShadowMapManager::CascadeSplits::CascadeSplits(Params const& params) noexcept
        : mSplitCount(params.cascadeCount + 1) {
    for (size_t s = 0; s < mSplitCount; s++) {
        mSplitsWs[s] = params.near + (params.far - params.near) * params.splitPositions[s];
        mSplitsCs[s] = mat4f::project(params.proj, float3(0.0f, 0.0f, mSplitsWs[s])).z;
    }
}

} // namespace filament
