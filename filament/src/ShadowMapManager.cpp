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

#include <utils/debug.h>
#include <utils/FixedCapacityVector.h>

namespace filament {

using namespace backend;
using namespace math;

ShadowMapManager::ShadowMapManager(FEngine& engine)
        : mEngine(engine) {
    // initialize our ShadowMap array in-place
    UTILS_NOUNROLL
    for (auto& entry : mShadowMapCache) {
        new (&entry) ShadowMap(engine);
    }

    mShadowUbh = engine.getDriverApi().createBufferObject(mShadowUb.getSize(),
            BufferObjectBinding::UNIFORM, BufferUsage::DYNAMIC);

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
        std::destroy_at(std::launder(reinterpret_cast<ShadowMap*>(&entry)));
    }
}

void ShadowMapManager::terminate(FEngine& engine) {
    DriverApi& driver = engine.getDriverApi();
    driver.destroyBufferObject(mShadowUbh);
    UTILS_NOUNROLL
    for (auto& entry : mShadowMapCache) {
        std::launder(reinterpret_cast<ShadowMap*>(&entry))->terminate(engine);
    }
}


ShadowMapManager::ShadowTechnique ShadowMapManager::update(FEngine& engine, FView& view,
        CameraInfo const& cameraInfo,
        FScene::RenderableSoa& renderableData, FScene::LightSoa& lightData) noexcept {
    ShadowTechnique shadowTechnique = {};

    calculateTextureRequirements(engine, view, lightData);

    // Compute scene-dependent values shared across all shadow maps
    ShadowMap::initSceneInfo(mSceneInfo,
            view.getVisibleLayers(), *view.getScene(), cameraInfo.view);

    shadowTechnique |= updateCascadeShadowMaps(
            engine, view, cameraInfo, renderableData, lightData, mSceneInfo);

    shadowTechnique |= updateSpotShadowMaps(
            engine, lightData);

    return shadowTechnique;
}

void ShadowMapManager::reset() noexcept {
    mCascadeShadowMaps.clear();
    mSpotShadowMaps.clear();
}

void ShadowMapManager::setDirectionalShadowMap(size_t lightIndex,
        LightManager::ShadowOptions const* options) noexcept {
    assert_invariant(options->shadowCascades <= CONFIG_MAX_SHADOW_CASCADES);
    for (size_t c = 0; c < options->shadowCascades; c++) {
        auto* pShadowMap = getCascadeShadowMap(c);
        pShadowMap->initialize(lightIndex, ShadowType::DIRECTIONAL, c, options);
        mCascadeShadowMaps.push_back(pShadowMap);
    }
}

void ShadowMapManager::addShadowMap(size_t lightIndex, bool spotlight,
        LightManager::ShadowOptions const* options) noexcept {
    const size_t c = mSpotShadowMaps.size();
    assert_invariant(c < CONFIG_MAX_SHADOWMAP_PUNCTUAL);
    auto* pShadowMap = getPointOrSpotShadowMap(c);
    pShadowMap->initialize(lightIndex, spotlight ? ShadowType::SPOT : ShadowType::POINT, c, options);
    mSpotShadowMaps.push_back(pShadowMap);
}

FrameGraphId<FrameGraphTexture> ShadowMapManager::render(FrameGraph& fg, FEngine& engine,
        RenderPass const& pass, FView& view, CameraInfo const& mainCameraInfo) noexcept {

    const float moment2 = std::numeric_limits<half>::max();
    const float moment1 = std::sqrt(moment2);
    const float4 vsmClearColor{ moment1, moment2, -moment1, moment2 };

    FScene* scene = view.getScene();
    assert_invariant(scene);

    // make a copy here, because it's a very small structure
    const TextureAtlasRequirements textureRequirements = mTextureAtlasRequirements;
    assert_invariant(textureRequirements.layers <= CONFIG_MAX_SHADOW_LAYERS);

    // -------------------------------------------------------------------------------------------
    // Prepare Shadow Pass
    // -------------------------------------------------------------------------------------------

    struct PrepareShadowPassData {
        struct ShadowPass {
            mutable RenderPass pass;
            mutable CameraInfo cameraInfo;
            ShadowMap* shadowMap;
            utils::Range<uint32_t> range;
            FScene::VisibleMaskType visibilityMask;
            uint8_t face = 0;
        };
        // the actual shadow map atlas (currently a 2D texture array)
        FrameGraphId<FrameGraphTexture> shadows;
        // a RenderPass per shadow map
        utils::FixedCapacityVector<ShadowPass> passList;
    };

    auto& prepareShadowPass = fg.addPass<PrepareShadowPassData>("Prepare Shadow Pass",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.passList.reserve(CONFIG_MAX_SHADOW_LAYERS);
                data.shadows = builder.createTexture("Shadowmap", {
                        .width = textureRequirements.size, .height = textureRequirements.size,
                        .depth = textureRequirements.layers,
                        .levels = textureRequirements.levels,
                        .type = SamplerType::SAMPLER_2D_ARRAY,
                        .format = textureRequirements.format
                });

                // these loops create a list of the shadow maps that might need to be rendered
                auto& passList = data.passList;

                // Directional, cascaded shadow maps
                auto const directionalShadowCastersRange = view.getVisibleDirectionalShadowCasters();
                if (!directionalShadowCastersRange.empty()) {
                    for (auto* pShadowMap : mCascadeShadowMaps) {
                        // for the directional light, we already know if it has visible shadows.
                        if (pShadowMap->hasVisibleShadows()) {
                            passList.push_back({
                                    pass, {}, pShadowMap, directionalShadowCastersRange,
                                    VISIBLE_DIR_SHADOW_RENDERABLE });
                        }
                    }
                }

                // Point lights and Spotlight shadow maps
                auto const spotShadowCastersRange = view.getVisibleSpotShadowCasters();
                if (!spotShadowCastersRange.empty()) {
                    for (auto* pShadowMap : mSpotShadowMaps) {
                        assert_invariant(!pShadowMap->isDirectionalShadow());
                        passList.push_back({
                                pass, {}, pShadowMap, spotShadowCastersRange,
                                VISIBLE_DYN_SHADOW_RENDERABLE });
                        if (pShadowMap->isPointShadow()) {
                            // add the 5 extra faces
                            for (uint8_t face = 1; face < 6; face++) {
                                passList.push_back({
                                        pass, {}, pShadowMap, spotShadowCastersRange,
                                        VISIBLE_DYN_SHADOW_RENDERABLE, face });
                            }
                        }
                    }
                }

                assert_invariant(passList.size() <= textureRequirements.layers);

                // This pass must be declared as having a side effect because it never gets a
                // "read" from one of its resource (only writes), so the FrameGraph culls it.
                builder.sideEffect();
            },
            [this, &engine, &view, scene, &mainCameraInfo](
                    FrameGraphResources const& resources, auto const& data, DriverApi& driver) {
                // set uniforms needed to render this ShadowMap
                view.prepareShadowMap(view.getVsmShadowOptions().highPrecision);

                // Note: we could almost parallel_for the loop below, the problem currently is
                // that updatePrimitivesLod() updates temporary global state.
                // prepareSpotShadowMap() also update the visibility of renderable. These two
                // pieces of state are needed only until shadowMap.render() returns.
                // Conceptually, we could store this out-of-band.

                // Generate a RenderPass for each shadow map
                for (auto const& entry : data.passList) {
                    ShadowMap& shadowMap = *entry.shadowMap;

                    // for spot shadow map, we need to do the culling
                    switch (shadowMap.getShadowType()) {
                        case ShadowType::DIRECTIONAL:
                            // nothing to do, it's been done already
                            break;
                        case ShadowType::SPOT:
                            prepareSpotShadowMap(shadowMap, engine, view, mainCameraInfo,
                                    scene->getRenderableData(), entry.range,
                                    scene->getLightData(), mSceneInfo);
                            break;
                        case ShadowType::POINT:
                            preparePointShadowMap(shadowMap, engine, view, mainCameraInfo,
                                    scene->getRenderableData(), entry.range,
                                    scene->getLightData(), entry.face, mSceneInfo);
                            break;
                    }

                    if (shadowMap.hasVisibleShadows()) {
                        // TODO: this loop can generate a lot of commands. investigate if
                        //       we could compact or reduce the number of commands.
                        //       - we could move this into the execute of each pass, but
                        //         then we would have to update the UBO each time.
                        //       - we could also always use the same commands for all
                        //         shadow maps, but then we'd generate a lot of
                        //         unneeded draw call.

                        // generate and sort the commands for rendering the shadow map

                        // cameraInfo only valid after calling update
                        const CameraInfo cameraInfo{ shadowMap.getCamera() };

                        // updatePrimitivesLod must be run before RenderPass::appendCommands.
                        view.updatePrimitivesLod(engine,
                                cameraInfo, scene->getRenderableData(), entry.range);

                        if (!view.hasVSM()) {
                            auto li = scene->getLightData().elementAt<FScene::LIGHT_INSTANCE>(
                                    shadowMap.getLightIndex());
                            auto const& params = engine.getLightManager().getShadowParams(li);
                            const PolygonOffset polygonOffset = { // handle reversed Z
                                    .slope    = -params.options.polygonOffsetSlope,
                                    .constant = -params.options.polygonOffsetConstant
                            };
                            entry.pass.overridePolygonOffset(&polygonOffset);
                        }

                        constexpr const backend::Viewport disabledScissor{ 0, 0,
                                (uint32_t)std::numeric_limits<int32_t>::max(),
                                (uint32_t)std::numeric_limits<int32_t>::max() };

                        entry.pass.setCamera(cameraInfo);
                        entry.pass.setVisibilityMask(entry.visibilityMask);
                        entry.pass.setGeometry(scene->getRenderableData(),
                                entry.range, scene->getRenderableUBO());
                        entry.pass.overrideScissor(&disabledScissor);
                        entry.pass.appendCommands(RenderPass::SHADOW);
                        entry.pass.sortCommands();
                        entry.cameraInfo = cameraInfo;
                    }
                }

                // Finally update our UBO in one batch
                if (mShadowUb.isDirty()) {
                    driver.updateBufferObject(mShadowUbh,
                            mShadowUb.toBufferDescriptor(driver), 0);
                }
            });

    // -------------------------------------------------------------------------------------------
    // Shadow Passes
    // -------------------------------------------------------------------------------------------

    struct ShadowPassData {
        FrameGraphId<FrameGraphTexture> tempBlurSrc{};  // temporary shadowmap when blurring
        FrameGraphId<FrameGraphTexture> output;
        uint32_t blurRt{};
        uint32_t shadowRt{};
    };

    auto const& passList = prepareShadowPass.getData().passList;
    for (auto const& entry: passList) {
        if (!entry.shadowMap->hasVisibleShadows()) {
            continue;
        }
        
        const uint8_t layer = entry.shadowMap->getLayer() + entry.face;
        const auto* options = entry.shadowMap->getShadowOptions();

        auto& shadowPass = fg.addPass<ShadowPassData>("Shadow Pass",
                [&](FrameGraph::Builder& builder, auto& data) {
                    const bool blur = view.hasVSM() && options->vsm.blurWidth > 0.0f;

                    FrameGraphRenderPass::Descriptor renderTargetDesc{};

                    data.output = builder.createSubresource(prepareShadowPass->shadows,
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
                                .format = TextureFormat::DEPTH16,
                        });

                        // Temporary (resolved) texture used to render the shadowmap when blurring
                        // is needed -- it'll be used as the source of the blur.
                        data.tempBlurSrc = builder.createTexture("Temporary Shadowmap", {
                                .width = textureRequirements.size, .height = textureRequirements.size,
                                .format = textureRequirements.format
                        });

                        depth = builder.write(depth,
                                FrameGraphTexture::Usage::DEPTH_ATTACHMENT);

                        data.output = builder.write(data.output,
                                FrameGraphTexture::Usage::COLOR_ATTACHMENT);

                        renderTargetDesc.attachments.color[0] = data.output;
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
                        data.output = builder.write(data.output,
                                FrameGraphTexture::Usage::DEPTH_ATTACHMENT);
                        renderTargetDesc.attachments.depth = data.output;
                        renderTargetDesc.clearFlags = TargetBufferFlags::DEPTH;
                    }

                    // finally, create the shadowmap render target -- one per layer.
                    data.shadowRt = builder.declareRenderPass("Shadow RT", renderTargetDesc);
                },
                [=, &entry, &view](FrameGraphResources const& resources,
                        auto const& data, DriverApi& driver) {

                    // Note: we capture entry by reference here. That's actually okay because
                    // `entry` lives in `PrepareShadowPassData` which is guaranteed to still
                    // be alive when we execute here (all passes stay alive until the FrameGraph
                    // is destroyed).
                    // It wouldn't work to capture by copy because entry.pass wouldn't be
                    // initialized, as this happens in an `execute` block.

                    CameraInfo const& cameraInfo = entry.cameraInfo;

                    const auto& executor = entry.pass.getExecutor();
                    const bool blur = view.hasVSM() && options->vsm.blurWidth > 0.0f;

                    // set uniforms relative to the camera
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
                    if (entry.shadowMap->isPointShadow()) {
                        // for point-light we don't have a border
                        viewport = { 0, 0, dim, dim };
                    }

                    view.prepareViewport(viewport, 0, 0);

                    view.commitUniforms(driver);

                    // render either directly into the shadowmap, or to the temporary texture for
                    // blurring.
                    auto rt = resources.getRenderPassInfo(blur ? data.blurRt : data.shadowRt);
                    rt.params.viewport = viewport;

                    executor.execute("Shadow Pass", rt.target, rt.params);
                });


        // now emit the blurring passes if needed
        if (view.hasVSM()) {
            auto& ppm = engine.getPostProcessManager();

            const float blurWidth = options->vsm.blurWidth;
            if (blurWidth > 0.0f) {
                const float sigma = (blurWidth + 1.0f) / 6.0f;
                size_t kernelWidth = std::ceil((blurWidth - 5.0f) / 4.0f);
                kernelWidth = kernelWidth * 4 + 5;
                ppm.gaussianBlurPass(fg,
                        shadowPass->tempBlurSrc,
                        shadowPass->output,
                        false, kernelWidth, sigma);
            }

            // If the shadow texture has more than one level, mipmapping was requested, either directly
            // or indirectly via anisotropic filtering.
            // So generate the mipmaps for each layer
            if (textureRequirements.levels > 1) {
                for (size_t level = 0; level < textureRequirements.levels - 1; level++) {
                    const bool finalize = level == textureRequirements.levels - 2;
                    ppm.vsmMipmapPass(fg, prepareShadowPass->shadows, layer, level,
                            vsmClearColor, finalize);
                }
            }
        }
    }

    return prepareShadowPass->shadows;
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
            .atlasDimension      = mTextureAtlasRequirements.size,
            .textureDimension    = uint16_t(options.mapSize),
            .shadowDimension     = uint16_t(options.mapSize - 2u),
            .clipSpaceFlipped    = engine.getBackend() == Backend::VULKAN,
            .textureSpaceFlipped = engine.getBackend() == Backend::METAL,
            .vsm                 = view.hasVSM()
    };

    bool hasVisibleShadows = false;
    if (!mCascadeShadowMaps.empty()) {
        // Even if we have more than one cascade, we cull directional shadow casters against the
        // entire camera frustum, as if we only had a single cascade.
        ShadowMap& shadowMap = *mCascadeShadowMaps[0];

        auto shaderParameters = shadowMap.updateDirectional(mEngine,
                lightData, 0, cameraInfo, shadowMapInfo, *scene, sceneInfo);

        hasVisibleShadows = shadowMap.hasVisibleShadows();

        if (hasVisibleShadows) {
            Frustum const& frustum = shadowMap.getCamera().getCullingFrustum();
            FView::cullRenderables(engine.getJobSystem(), renderableData, frustum,
                    VISIBLE_DIR_SHADOW_RENDERABLE_BIT);

            // Set shadowBias, using the first directional cascade.
            // when computing the required bias we need a half-texel size, so we multiply by 0.5 here.
            // note: normalBias is set to zero for VSM
            const float normalBias = shadowMapInfo.vsm ? 0.0f : 0.5f * lcm.getShadowNormalBias(0);
            // Texel size is constant for directional light (although that's not true when LISPSM
            // is used, but in that case we're pretending it is).
            const float wsTexelSize = shaderParameters.texelSizeAtOneMeterWs;
            mShadowMappingUniforms.shadowBias = normalBias * wsTexelSize;
            mShadowMappingUniforms.shadowBulbRadiusLs =
                    mSoftShadowOptions.penumbraScale * options.shadowBulbRadius / wsTexelSize;
            mShadowMappingUniforms.elvsm = options.vsm.elvsm;
        }
    }

    ShadowTechnique shadowTechnique{};
    uint32_t directionalShadowsMask = 0;
    uint32_t cascadeHasVisibleShadows = 0;

    if (hasVisibleShadows) {
        // Adjust the near and far planes to tightly bound the scene.
        float vsNear = -cameraInfo.zn;
        float vsFar = -cameraInfo.zf;
        if (engine.debug.shadowmap.tightly_bound_scene && !params.options.stable) {
            vsNear = std::min(vsNear, sceneInfo.vsNearFar.x);
            vsFar = std::max(vsFar, sceneInfo.vsNearFar.y);
        }

        const size_t cascadeCount = mCascadeShadowMaps.size();

        // We divide the camera frustum into N cascades. This gives us N + 1 split positions.
        // The first split position is the near plane; the last split position is the far plane.
        std::array<float, CascadeSplits::SPLIT_COUNT> splitPercentages{};
        splitPercentages[cascadeCount] = 1.0f;
        for (size_t i = 1; i < cascadeCount; i++) {
            splitPercentages[i] = options.cascadeSplitPositions[i - 1];
        }

        const CascadeSplits::Params p{
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

        for (size_t i = 0, c = mCascadeShadowMaps.size(); i < c; i++) {
            assert_invariant(mCascadeShadowMaps[i]);

            // Compute the frustum for the directional light.
            ShadowMap& shadowMap = *mCascadeShadowMaps[i];
            assert_invariant(shadowMap.getLightIndex() == 0);

            sceneInfo.csNearFar = { csSplitPosition[i], csSplitPosition[i + 1] };

            auto shaderParameters = shadowMap.updateDirectional(mEngine,
                    lightData, 0, cameraInfo, shadowMapInfo, *scene, sceneInfo);

            if (shadowMap.hasVisibleShadows()) {
                mShadowMappingUniforms.lightFromWorldMatrix[i] = shaderParameters.lightSpace;
                shadowTechnique |= ShadowTechnique::SHADOW_MAP;
                cascadeHasVisibleShadows |= 0x1u << i;
            }
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

void ShadowMapManager::updateSpotVisibilityMasks(
        uint8_t visibleLayers,
        uint8_t const* UTILS_RESTRICT layers,
        FRenderableManager::Visibility const* UTILS_RESTRICT visibility,
        Culler::result_type* UTILS_RESTRICT visibleMask, size_t count) {
    // __restrict__ seems to only be taken into account as function parameters. This is very
    // important here, otherwise, this loop doesn't get vectorized.
    // This is vectorized 16x.
    count = (count + 0xFu) & ~0xFu; // capacity guaranteed to be multiple of 16
    for (size_t i = 0; i < count; ++i) {
        const Culler::result_type mask = visibleMask[i];
        const FRenderableManager::Visibility v = visibility[i];
        const bool inVisibleLayer = layers[i] & visibleLayers;

        const bool visSpotShadowRenderable = v.castShadows && inVisibleLayer &&
                (!v.culling || (mask & VISIBLE_DYN_SHADOW_RENDERABLE));

        using Type = Culler::result_type;

        visibleMask[i] &= ~Type(VISIBLE_DYN_SHADOW_RENDERABLE);
        visibleMask[i] |= Type(visSpotShadowRenderable << VISIBLE_DYN_SHADOW_RENDERABLE_BIT);
    }
}

void ShadowMapManager::prepareSpotShadowMap(ShadowMap& shadowMap,
        FEngine& engine, FView& view, CameraInfo const& mainCameraInfo,
        FScene::RenderableSoa& renderableData, utils::Range<uint32_t> range,
        FScene::LightSoa& lightData, ShadowMap::SceneInfo const& sceneInfo) noexcept {
    auto& lcm = engine.getLightManager();

    const size_t lightIndex = shadowMap.getLightIndex();
    const FLightManager::Instance li = lightData.elementAt<FScene::LIGHT_INSTANCE>(lightIndex);
    FLightManager::ShadowOptions const* const options = shadowMap.getShadowOptions();

    // compute the frustum for this light
    // for spotlights, we cull shadow casters first because we already know the frustum,
    // this will help us find better near/far plane later
    const auto position = lightData.elementAt<FScene::POSITION_RADIUS>(lightIndex).xyz;
    const auto direction = lightData.elementAt<FScene::DIRECTION>(lightIndex);
    const auto radius = lightData.elementAt<FScene::POSITION_RADIUS>(lightIndex).w;
    const auto outerConeAngle = lcm.getSpotLightOuterCone(li);

    // compute shadow map frustum for culling
    const mat4f Mv = ShadowMap::getDirectionalLightViewMatrix(direction, position);
    const mat4f Mp = mat4f::perspective(outerConeAngle * f::RAD_TO_DEG * 2.0f, 1.0f, 0.01f, radius);
    const mat4f MpMv(math::highPrecisionMultiply(Mp, Mv));
    const Frustum frustum(MpMv);

    // Cull shadow casters
    float3 const* worldAABBCenter = renderableData.data<FScene::WORLD_AABB_CENTER>();
    float3 const* worldAABBExtent = renderableData.data<FScene::WORLD_AABB_EXTENT>();
    FScene::VisibleMaskType* visibleArray = renderableData.data<FScene::VISIBLE_MASK>();
    Culler::intersects(
            visibleArray + range.first,
            frustum,
            worldAABBCenter + range.first,
            worldAABBExtent + range.first,
            range.size(),
            VISIBLE_DYN_SHADOW_RENDERABLE_BIT);

    // update their visibility mask
    uint8_t const* layers = renderableData.data<FScene::LAYERS>();
    auto const* visibility = renderableData.data<FScene::VISIBILITY_STATE>();
    updateSpotVisibilityMasks(
            view.getVisibleLayers(),
            layers + range.first,
            visibility + range.first,
            visibleArray + range.first,
            range.size());

    // update the shadow map frustum/camera
    const ShadowMap::ShadowMapInfo shadowMapInfo{
            .atlasDimension      = mTextureAtlasRequirements.size,
            .textureDimension    = uint16_t(options->mapSize),
            .shadowDimension     = uint16_t(options->mapSize - 2u),
            .clipSpaceFlipped    = engine.getBackend() == Backend::VULKAN,
            .textureSpaceFlipped = engine.getBackend() == Backend::METAL,
            .vsm                 = view.hasVSM()
    };

    auto shaderParameters = shadowMap.updateSpot(mEngine,
            lightData, lightIndex, mainCameraInfo, shadowMapInfo, *view.getScene(), sceneInfo);

    // and if we need to generate it, update all the UBO data
    if (shadowMap.hasVisibleShadows()) {
        const size_t shadowIndex = shadowMap.getShadowIndex();
        const float wsTexelSizeAtOneMeter = shaderParameters.texelSizeAtOneMeterWs;
        // note: normalBias is set to zero for VSM
        const float normalBias = shadowMapInfo.vsm ? 0.0f : options->normalBias;

        auto& s = mShadowUb.edit();
        const double n = shadowMap.getCamera().getNear();
        const double f = shadowMap.getCamera().getCullingFar();
        s.shadows[shadowIndex].lightFromWorldMatrix = shaderParameters.lightSpace;
        s.shadows[shadowIndex].direction = direction;
        s.shadows[shadowIndex].normalBias = normalBias * wsTexelSizeAtOneMeter;
        s.shadows[shadowIndex].lightFromWorldZ = shaderParameters.lightFromWorldZ;
        s.shadows[shadowIndex].texelSizeAtOneMeter = wsTexelSizeAtOneMeter;
        s.shadows[shadowIndex].nearOverFarMinusNear = float(n / (f - n));
        s.shadows[shadowIndex].elvsm = options->vsm.elvsm;
        s.shadows[shadowIndex].bulbRadiusLs =
                mSoftShadowOptions.penumbraScale * options->shadowBulbRadius
                / wsTexelSizeAtOneMeter;
    }
}

void ShadowMapManager::preparePointShadowMap(ShadowMap& shadowMap,
        FEngine& engine, FView& view, CameraInfo const& mainCameraInfo,
        FScene::RenderableSoa& renderableData, utils::Range<uint32_t> range,
        FScene::LightSoa& lightData, uint8_t face,
        ShadowMap::SceneInfo const& sceneInfo) noexcept {

    const size_t lightIndex = shadowMap.getLightIndex();
    FLightManager::ShadowOptions const* const options = shadowMap.getShadowOptions();

    // compute the frustum for this light
    // for spotlights, we cull shadow casters first because we already know the frustum,
    // this will help us find better near/far plane later
    const auto position = lightData.elementAt<FScene::POSITION_RADIUS>(lightIndex).xyz;
    const auto radius = lightData.elementAt<FScene::POSITION_RADIUS>(lightIndex).w;

    // compute shadow map frustum for culling
    const mat4f Mv = ShadowMap::getPointLightViewMatrix(TextureCubemapFace(face), position);
    const mat4f Mp = mat4f::perspective(90.0f, 1.0f, 0.01f, radius);
    const Frustum frustum{ math::highPrecisionMultiply(Mp, Mv) };

    // Cull shadow casters
    float3 const* worldAABBCenter = renderableData.data<FScene::WORLD_AABB_CENTER>();
    float3 const* worldAABBExtent = renderableData.data<FScene::WORLD_AABB_EXTENT>();
    FScene::VisibleMaskType* visibleArray = renderableData.data<FScene::VISIBLE_MASK>();
    Culler::intersects(
            visibleArray + range.first,
            frustum,
            worldAABBCenter + range.first,
            worldAABBExtent + range.first,
            range.size(),
            VISIBLE_DYN_SHADOW_RENDERABLE_BIT);

    // update their visibility mask
    uint8_t const* layers = renderableData.data<FScene::LAYERS>();
    auto const* visibility = renderableData.data<FScene::VISIBILITY_STATE>();
    updateSpotVisibilityMasks(
            view.getVisibleLayers(),
            layers + range.first,
            visibility + range.first,
            visibleArray + range.first,
            range.size());

    // update the shadow map frustum/camera
    const ShadowMap::ShadowMapInfo shadowMapInfo{
            .atlasDimension      = mTextureAtlasRequirements.size,
            .textureDimension    = uint16_t(options->mapSize),
            .shadowDimension     = uint16_t(options->mapSize), // point-lights don't have a border
            .clipSpaceFlipped    = engine.getBackend() == Backend::VULKAN,
            .textureSpaceFlipped = engine.getBackend() == Backend::METAL,
            .vsm                 = view.hasVSM()
    };

    auto shaderParameters = shadowMap.updatePoint(mEngine, lightData, lightIndex,
            mainCameraInfo, shadowMapInfo, *view.getScene(), sceneInfo, face);


    // and if we need to generate it, update all the UBO data
    // Note: this below is done for all six faces even if it sets identical values each time
    if (shadowMap.hasVisibleShadows()) {
        const size_t shadowIndex = shadowMap.getShadowIndex();
        const float wsTexelSizeAtOneMeter = shaderParameters.texelSizeAtOneMeterWs;
        // note: normalBias is set to zero for VSM
        const float normalBias = shadowMapInfo.vsm ? 0.0f : options->normalBias;

        auto& s = mShadowUb.edit();
        const double n = shadowMap.getCamera().getNear();
        const double f = shadowMap.getCamera().getCullingFar();

        s.shadows[shadowIndex].lightFromWorldMatrix = {}; // no texture matrix for point lights
        s.shadows[shadowIndex].direction = {};  // no direction of point lights
        s.shadows[shadowIndex].normalBias = normalBias * wsTexelSizeAtOneMeter;
        s.shadows[shadowIndex].lightFromWorldZ = {
                -((n + f) / (f - n)) * 0.5f + 0.5f,
                  (f * n) / (f - n),
                       -n / (f - n),
                     1.0f / (f - n),
        };
        s.shadows[shadowIndex].texelSizeAtOneMeter = wsTexelSizeAtOneMeter;
        s.shadows[shadowIndex].nearOverFarMinusNear = float(n / (f - n));
        s.shadows[shadowIndex].elvsm = options->vsm.elvsm;
        s.shadows[shadowIndex].bulbRadiusLs =
                mSoftShadowOptions.penumbraScale * options->shadowBulbRadius
                        / wsTexelSizeAtOneMeter;
    }
}

ShadowMapManager::ShadowTechnique ShadowMapManager::updateSpotShadowMaps(FEngine& engine,
        FScene::LightSoa& lightData) noexcept {

    FScene::ShadowInfo* const shadowInfo = lightData.data<FScene::SHADOW_INFO>();

    ShadowTechnique shadowTechnique{};
    if (!mSpotShadowMaps.empty()) {
        shadowTechnique |= ShadowTechnique::SHADOW_MAP;
        for (auto const* pShadowMap : mSpotShadowMaps) {
            const size_t lightIndex = pShadowMap->getLightIndex();
            shadowInfo[lightIndex].castsShadows = true;     // FIXME: is that set correctly?
            shadowInfo[lightIndex].index = pShadowMap->getShadowIndex();
            shadowInfo[lightIndex].layer = pShadowMap->getLayer();
        }
    }

    // screen-space contact shadows for point/spotlights
    auto& lcm = engine.getLightManager();
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
    bool elvsm = false;
    for (auto* pShadowMap : mCascadeShadowMaps) {
        // Shadow map size should be the same for all cascades.
        auto const& options = pShadowMap->getShadowOptions();
        maxDimension = std::max(maxDimension, options->mapSize);
        elvsm = elvsm || options->vsm.elvsm;
        pShadowMap->setLayer(layer++);
    }
    for (auto& pShadowMap : mSpotShadowMaps) {
        auto const& options = pShadowMap->getShadowOptions();
        maxDimension = std::max(maxDimension, options->mapSize);
        elvsm = elvsm || options->vsm.elvsm;
        pShadowMap->setLayer(layer++);
        if (pShadowMap->isPointShadow()) {
            // FIXME: we need to make sure we don't exceed 256
            layer += 5;
        }
    }

    const uint8_t layersNeeded = layer;

    // Generate mipmaps for VSM when anisotropy is enabled or when requested
    auto const& vsmShadowOptions = view.getVsmShadowOptions();
    const bool useMipmapping = view.hasVSM() &&
                               ((vsmShadowOptions.anisotropy > 0) || vsmShadowOptions.mipmapping);

    TextureFormat format = TextureFormat::DEPTH16;
    if (view.hasVSM()) {
        if (vsmShadowOptions.highPrecision) {
            if (elvsm) {
                format = TextureFormat::RGBA32F;
            } else {
                format = TextureFormat::RG32F;
            }
        } else {
            if (elvsm) {
                format = TextureFormat::RGBA16F;
            } else {
                format = TextureFormat::RG16F;
            }
        }
    }

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
            mipLevels,
            format
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
