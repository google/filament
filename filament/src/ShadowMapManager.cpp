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
#include "AtlasAllocator.h"
#include "RenderPass.h"
#include "ShadowMap.h"

#include <filament/Frustum.h>
#include <filament/LightManager.h>
#include <filament/Options.h>

#include <iterator>
#include <private/filament/EngineEnums.h>

#include "components/RenderableManager.h"

#include "details/Camera.h"
#include "details/DebugRegistry.h"
#include "details/Texture.h"
#include "details/View.h"

#include "fg/FrameGraph.h"
#include "fg/FrameGraphId.h"
#include "fg/FrameGraphRenderPass.h"
#include "fg/FrameGraphTexture.h"

#include <backend/DriverApiForward.h>
#include <backend/DriverEnums.h>

#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/FixedCapacityVector.h>
#include <utils/BitmaskEnum.h>
#include <utils/Range.h>
#include <utils/Slice.h>

#include <math/half.h>
#include <math/mat4.h>
#include <math/vec4.h>
#include <math/scalar.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <limits>
#include <new>
#include <memory>

#include <stdint.h>
#include <stddef.h>

namespace filament {

using namespace backend;
using namespace math;

ShadowMapManager::ShadowMapManager(FEngine& engine)
    : mIsDepthClampSupported(engine.getDriverApi().isDepthClampSupported()) {
    FDebugRegistry& debugRegistry = engine.getDebugRegistry();
    debugRegistry.registerProperty("d.shadowmap.visualize_cascades",
            &engine.debug.shadowmap.visualize_cascades);
    debugRegistry.registerProperty("d.shadowmap.disable_light_frustum_align",
            &engine.debug.shadowmap.disable_light_frustum_align);
    debugRegistry.registerProperty("d.shadowmap.depth_clamp",
            &engine.debug.shadowmap.depth_clamp);

    mFeatureShadowAllocator = engine.features.engine.shadows.use_shadow_atlas;
}

ShadowMapManager::~ShadowMapManager() {
    // destroy the ShadowMap array in-place
    if (UTILS_UNLIKELY(mInitialized)) {
        UTILS_NOUNROLL
        for (auto& entry: mShadowMapCache) {
            std::destroy_at(std::launder(reinterpret_cast<ShadowMap*>(&entry)));
        }
    }
}

void ShadowMapManager::createIfNeeded(FEngine& engine,
        std::unique_ptr<ShadowMapManager>& inOutShadowMapManager) {
    if (UTILS_UNLIKELY(!inOutShadowMapManager)) {
        inOutShadowMapManager.reset(new ShadowMapManager(engine));
    }
}

void ShadowMapManager::terminate(FEngine& engine,
        std::unique_ptr<ShadowMapManager>& shadowMapManager) {
    if (shadowMapManager) {
        shadowMapManager->terminate(engine);
    }
}

size_t ShadowMapManager::getMaxShadowMapCount() const noexcept {
    return mFeatureShadowAllocator ? CONFIG_MAX_SHADOWMAPS : CONFIG_MAX_SHADOW_LAYERS;
}

void ShadowMapManager::terminate(FEngine& engine) {
    if (UTILS_UNLIKELY(mInitialized)) {
        DriverApi& driver = engine.getDriverApi();
        driver.destroyBufferObject(mShadowUbh);
        UTILS_NOUNROLL
        for (auto& entry: mShadowMapCache) {
            std::launder(reinterpret_cast<ShadowMap*>(&entry))->terminate(engine);
        }
    }
}

ShadowMapManager::ShadowTechnique ShadowMapManager::update(
        Builder const& builder,
        FEngine& engine, FView& view,
        CameraInfo const& cameraInfo,
        FScene::RenderableSoa& renderableData, FScene::LightSoa const& lightData) noexcept {

    if (!builder.mDirectionalShadowMapCount && !builder.mSpotShadowMapCount) {
        // no shadows were recorder
        return ShadowTechnique::NONE;
    }

    // initialize the shadowmap array the first time
    if (UTILS_UNLIKELY(!mInitialized)) {
        mInitialized = true;
        // initialize our ShadowMap array in-place
        mShadowUbh = engine.getDriverApi().createBufferObject(mShadowUb.getSize(),
                BufferObjectBinding::UNIFORM, BufferUsage::DYNAMIC);
        UTILS_NOUNROLL
        for (auto& entry: mShadowMapCache) {
            new(&entry) ShadowMap(engine);
        }
    }

    mDirectionalShadowMapCount = builder.mDirectionalShadowMapCount;
    mSpotShadowMapCount = builder.mSpotShadowMapCount;

    for (auto const& entry : builder.mShadowMaps) {
        auto& shadowMap = getShadowMap(entry.shadowIndex);
        shadowMap.initialize(
                entry.lightIndex,
                entry.shadowType,
                entry.shadowIndex,
                entry.face,
                entry.options);
    }

    ShadowTechnique shadowTechnique = {};

    calculateTextureRequirements(engine, view, lightData);

    // Compute scene-dependent values shared across all shadow maps
    ShadowMap::SceneInfo const info{ *view.getScene(), view.getVisibleLayers() };

    shadowTechnique |= updateCascadeShadowMaps(
            engine, view, cameraInfo, renderableData, lightData, info);

    shadowTechnique |= updateSpotShadowMaps(
            engine, lightData);

    mSceneInfo = info;

    return shadowTechnique;
}

ShadowMapManager::Builder& ShadowMapManager::Builder::directionalShadowMap(size_t const lightIndex,
        LightManager::ShadowOptions const* options) noexcept {
    assert_invariant(options->shadowCascades <= CONFIG_MAX_SHADOW_CASCADES);
    // this updates getCascadedShadowMap()
    mDirectionalShadowMapCount = options->shadowCascades;
    for (size_t c = 0; c < options->shadowCascades; c++) {
        mShadowMaps.push_back({
                .lightIndex = lightIndex,
                .shadowType = ShadowType::DIRECTIONAL,
                .shadowIndex = uint8_t(c),
                .face = 0,
                .options = options });
    }
    return *this;
}

ShadowMapManager::Builder& ShadowMapManager::Builder::shadowMap(size_t const lightIndex, bool const spotlight,
        LightManager::ShadowOptions const* options) noexcept {
    if (spotlight) {
        const size_t c = mSpotShadowMapCount++;
        const size_t i = c + CONFIG_MAX_SHADOW_CASCADES;
        assert_invariant(i < CONFIG_MAX_SHADOWMAPS);
        mShadowMaps.push_back({
                .lightIndex = lightIndex,
                .shadowType = ShadowType::SPOT,
                .shadowIndex = uint8_t(i),
                .face = 0,
                .options = options });
    } else {
        // point-light, generate 6 independent shadowmaps
        for (size_t face = 0; face < 6; face++) {
            const size_t c = mSpotShadowMapCount++;
            const size_t i = c + CONFIG_MAX_SHADOW_CASCADES;
            assert_invariant(i < CONFIG_MAX_SHADOWMAPS);
            mShadowMaps.push_back({
                    .lightIndex = lightIndex,
                    .shadowType = ShadowType::POINT,
                    .shadowIndex = uint8_t(i),
                    .face = uint8_t(face),
                    .options = options });
        }
    }
    return *this;
}

FrameGraphId<FrameGraphTexture> ShadowMapManager::render(FEngine& engine, FrameGraph& fg,
        RenderPassBuilder const& passBuilder,
        FView& view, CameraInfo const& mainCameraInfo,
        float4 const& userTime) noexcept {

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
        struct ShadowPass {  // 112 bytes
            mutable RenderPass::Executor executor;
            ShadowMap* shadowMap;
            utils::Range<uint32_t> range;
            FScene::VisibleMaskType visibilityMask;
        };
        // the actual shadow map atlas (currently a 2D texture array)
        FrameGraphId<FrameGraphTexture> shadows;
        // a RenderPass per shadow map
        utils::FixedCapacityVector<ShadowPass> passList;
    };

    VsmShadowOptions const& vsmShadowOptions = view.getVsmShadowOptions();

    auto& prepareShadowPass = fg.addPass<PrepareShadowPassData>("Prepare Shadow Pass",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.passList.reserve(getMaxShadowMapCount());
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
                // if the view has shadowing enabled, the SRE variant could be used, so we must
                // generate the shadow maps
                if (view.needsDirectionalShadowMaps()) {
                    auto const directionalShadowCastersRange = view.getVisibleDirectionalShadowCasters();
                    for (auto& shadowMap : getCascadedShadowMap()) {
                        // For the directional light, we already know if it has visible shadows.
                        // if hasVisibleShadows() is false, we're guaranteed the shader won't
                        // sample this shadow map (see PerViewUib::cascades and
                        // ShadowMapManager::updateCascadeShadowMaps)
                        if (shadowMap.hasVisibleShadows()) {
                            passList.push_back({
                                    {}, &shadowMap, directionalShadowCastersRange,
                                    VISIBLE_DIR_SHADOW_RENDERABLE });
                        }
                    }
                }

                // Point lights and Spotlight shadow maps
                // if the view has shadowing enabled, the SRE variant could be used, so we must
                // generate the shadow maps
                if (view.needsPointShadowMaps()) {
                    auto const spotShadowCastersRange = view.getVisibleSpotShadowCasters();
                    for (auto& shadowMap : getSpotShadowMaps()) {
                        assert_invariant(!shadowMap.isDirectionalShadow());

                        switch (shadowMap.getShadowType()) {
                            case ShadowType::DIRECTIONAL:
                                // we should never be here
                                break;
                            case ShadowType::SPOT:
                                prepareSpotShadowMap(shadowMap, engine, view, mainCameraInfo,
                                        scene->getLightData(), mSceneInfo);
                                break;
                            case ShadowType::POINT:
                                preparePointShadowMap(shadowMap, engine, view, mainCameraInfo,
                                        scene->getLightData());
                                break;
                        }

                        // For spot/point lights, shadowMap.hasVisibleShadows() doesn't guarantee
                        // the shader won't access the shadow map, so we must generate it.
                        passList.push_back({
                                {}, &shadowMap, spotShadowCastersRange,
                                VISIBLE_DYN_SHADOW_RENDERABLE });
                    }
                }

                assert_invariant(mFeatureShadowAllocator ||
                    passList.size() <= textureRequirements.layers);

                if (mFeatureShadowAllocator) {
                    // sort shadow passes by layer so that we can update all the shadow maps of
                    // a layer in one render pass.
                    std::sort(passList.begin(), passList.end(), [](
                            PrepareShadowPassData::ShadowPass const& lhs,
                            PrepareShadowPassData::ShadowPass const& rhs) {
                        return lhs.shadowMap->getLayer() < rhs.shadowMap->getLayer();
                    });
                }

                // This pass must be declared as having a side effect because it never gets a
                // "read" from one of its resource (only writes), so the FrameGraph culls it.
                builder.sideEffect();
            },
            [=, passBuilder = passBuilder,
                    &engine = const_cast<FEngine /*const*/ &>(engine), // FIXME: we want this const
                    &view = const_cast<FView const&>(view)]
                    (FrameGraphResources const&, auto const& data, DriverApi& driver) mutable {

                // Note: we could almost parallel_for the loop below, the problem currently is
                // that updatePrimitivesLod() updates temporary global state.
                // prepareSpotShadowMap() also update the visibility of renderable. These two
                // pieces of state are needed only until shadowMap.render() returns.
                // Conceptually, we could store this out-of-band.

                // Generate a RenderPass for each shadow map
                for (auto const& entry : data.passList) {
                    ShadowMap const& shadowMap = *entry.shadowMap;

                    // Note: this loop can generate a lot of commands that come out of the
                    //       "per frame command arena". The allocation persists until the
                    //       end of the frame.
                    //       One way to possibly mitigate this, would be to always use the
                    //       same command buffer for all shadow map, but then we'd generate
                    //       a lot of unneeded draw calls.
                    //       To do this efficiently, we'd need a way to cull draw calls already
                    //       recorded in the command buffer, per shadow map. Maybe this could
                    //       be done with indirect draw calls.

                    // Note: the output of culling below is stored in scene->getRenderableData()

                    switch (shadowMap.getShadowType()) {
                        case ShadowType::DIRECTIONAL:
                            // we should never be here
                            break;
                        case ShadowType::SPOT:
                            if (shadowMap.hasVisibleShadows()) {
                                ShadowMapManager::cullSpotShadowMap(shadowMap, engine, view,
                                        scene->getRenderableData(), entry.range,
                                        scene->getLightData());
                            }
                            break;
                        case ShadowType::POINT:
                            if (shadowMap.hasVisibleShadows()) {
                                ShadowMapManager::cullPointShadowMap(shadowMap, view,
                                        scene->getRenderableData(), entry.range,
                                        scene->getLightData());
                            }
                            break;
                    }

                    // cameraInfo only valid after calling update
                    const CameraInfo cameraInfo{ shadowMap.getCamera(), mainCameraInfo };

                    auto transaction = ShadowMap::open(driver);
                    ShadowMap::prepareCamera(transaction, driver, cameraInfo);
                    ShadowMap::prepareViewport(transaction, shadowMap.getViewport());
                    ShadowMap::prepareTime(transaction, engine, userTime);
                    ShadowMap::prepareShadowMapping(transaction,
                            vsmShadowOptions.highPrecision);
                    shadowMap.commit(transaction, engine, driver);

                    // updatePrimitivesLod must be run before RenderPass::appendCommands.
                    FView::updatePrimitivesLod(scene->getRenderableData(),
                            engine, cameraInfo, entry.range);

                    // generate and sort the commands for rendering the shadow map

                    RenderPass::RenderFlags renderPassFlags{};

                    bool const canUseDepthClamp =
                            shadowMap.getShadowType() == ShadowType::DIRECTIONAL &&
                            !view.hasVSM() &&
                            mIsDepthClampSupported &&
                            engine.debug.shadowmap.depth_clamp;

                    if (canUseDepthClamp) {
                        renderPassFlags |= RenderPass::HAS_DEPTH_CLAMP;
                    }

                    RenderPass const pass = passBuilder
                            .renderFlags(RenderPass::HAS_DEPTH_CLAMP, renderPassFlags)
                            .camera(cameraInfo)
                            .visibilityMask(entry.visibilityMask)
                            .geometry(scene->getRenderableData(), entry.range)
                            .commandTypeFlags(RenderPass::CommandTypeFlags::SHADOW)
                            .build(engine, driver);

                    entry.executor = pass.getExecutor();

                    if (!view.hasVSM()) {
                        auto const* options = shadowMap.getShadowOptions();
                        PolygonOffset const polygonOffset = { // handle reversed Z
                                .slope    = -options->polygonOffsetSlope,
                                .constant = -options->polygonOffsetConstant
                        };
                        entry.executor.overridePolygonOffset(&polygonOffset);
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

    fg.getBlackboard()["shadowmap"] = prepareShadowPass->shadows;

    struct ShadowPassData {
        FrameGraphId<FrameGraphTexture> tempBlurSrc{};  // temporary shadowmap when blurring
        FrameGraphId<FrameGraphTexture> output;
        uint32_t rt{};
    };

    auto const& passList = prepareShadowPass.getData().passList;
    auto first = passList.begin();
    while (first != passList.end()) {
        auto const& entry = *first;

        const uint8_t layer = entry.shadowMap->getLayer();
        const auto* options = entry.shadowMap->getShadowOptions();
        const auto msaaSamples = textureRequirements.msaaSamples;
        const bool blur = entry.shadowMap->hasVisibleShadows() &&
                view.hasVSM() && options->vsm.blurWidth > 0.0f;

        auto last = first;
        // loop over each shadow pass to find its layer range
        while (last != passList.end() && last->shadowMap->getLayer() == layer) {
            ++last;
        }

        assert_invariant(mFeatureShadowAllocator ||
            std::distance(first, last) == 1);

        // And render all shadow pass of a given layer as a single render pass
        auto& shadowPass = fg.addPass<ShadowPassData>("Shadow Pass",
                [&](FrameGraph::Builder& builder, auto& data) {

                    FrameGraphRenderPass::Descriptor renderTargetDesc{};

                    data.output = builder.createSubresource(prepareShadowPass->shadows,
                            "Shadowmap Layer", { .layer = layer });

                    if (UTILS_UNLIKELY(view.hasVSM())) {
                        // Each shadow pass has its own sample count, but textures are created with
                        // a default count of 1 because we're using "magic resolve" (sample count is
                        // set on the render target).
                        // When rendering VSM shadow maps, we still need a depth texture for sorting.
                        // We specify the sample count here because we don't need automatic resolve.
                        auto depth = builder.createTexture("Temporary VSM Depth Texture", {
                                .width = textureRequirements.size, .height = textureRequirements.size,
                                .samples = msaaSamples,
                                .format = TextureFormat::DEPTH16,
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
                        renderTargetDesc.samples = msaaSamples;

                        if (UTILS_UNLIKELY(blur)) {
                            // Temporary (resolved) texture used to render the shadowmap when blurring
                            // is needed -- it'll be used as the source of the blur.
                            data.tempBlurSrc = builder.createTexture("Temporary Shadowmap", {
                                    .width = textureRequirements.size,
                                    .height = textureRequirements.size,
                                    .format = textureRequirements.format
                            });

                            data.tempBlurSrc = builder.write(data.tempBlurSrc,
                                    FrameGraphTexture::Usage::COLOR_ATTACHMENT);

                            data.rt = builder.declareRenderPass("Temp Shadow RT", {
                                    .attachments = {
                                            .color = { data.tempBlurSrc },
                                            .depth = depth },
                                    .clearColor = vsmClearColor,
                                    .samples = msaaSamples,
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
                    auto rt = builder.declareRenderPass("Shadow RT", renderTargetDesc);

                    // render either directly into the shadowmap, or to the temporary texture for
                    // blurring.
                    data.rt = blur ? data.rt : rt;
                },
                [=, &engine](FrameGraphResources const& resources,
                        auto const& data, DriverApi& driver) {

                    // Note: we capture entry by reference here. That's actually okay because
                    // `entry` lives in `PrepareShadowPassData` which is guaranteed to still
                    // be alive when we execute here (all passes stay alive until the FrameGraph
                    // is destroyed).
                    // It wouldn't work to capture by copy because entry.executor wouldn't be
                    // initialized, as this happens in an `execute` block.

                    auto rt = resources.getRenderPassInfo(data.rt);

                    driver.beginRenderPass(rt.target, rt.params);

                    for (auto curr = first; curr != last; curr++) {
                        // if we know there are no visible shadows, we can skip rendering, but
                        // we need the render-pass to clear/initialize the shadow-map
                        // Note: this is always true for directional/cascade shadows.
                        if (curr->shadowMap->hasVisibleShadows()) {
                            curr->shadowMap->bind(driver);
                            curr->executor.overrideScissor(curr->shadowMap->getScissor());
                            curr->executor.execute(engine, driver);
                        }
                    }

                    driver.endRenderPass();
                });

        first = last;

        // now emit the blurring passes if needed
        if (UTILS_UNLIKELY(blur)) {
            auto& ppm = engine.getPostProcessManager();

            // FIXME: this `options` is for the first shadowmap in the list, but it applies to
            //        the whole layer. Blurring should happen per shadowmap, not for the whole
            //        layer.

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

            // FIXME: mipmapping here is broken because it'll access texels from adjacent
            //        shadow maps.

            // If the shadow texture has more than one level, mipmapping was requested, either directly
            // or indirectly via anisotropic filtering.
            // So generate the mipmaps for each layer
            if (textureRequirements.levels > 1) {
                for (size_t level = 0; level < textureRequirements.levels - 1; level++) {
                    ppm.vsmMipmapPass(fg, prepareShadowPass->shadows, layer, level, vsmClearColor);
                }
            }
        }
    }

    return prepareShadowPass->shadows;
}

ShadowMapManager::ShadowTechnique ShadowMapManager::updateCascadeShadowMaps(FEngine& engine,
        FView& view, CameraInfo cameraInfo, FScene::RenderableSoa& renderableData,
        FScene::LightSoa const& lightData, ShadowMap::SceneInfo sceneInfo) noexcept {

    FScene* scene = view.getScene();
    auto& lcm = engine.getLightManager();

    FLightManager::Instance const directionalLight = lightData.elementAt<FScene::LIGHT_INSTANCE>(0);
    FLightManager::ShadowOptions const& options = lcm.getShadowOptions(directionalLight);
    FLightManager::ShadowParams const& params = lcm.getShadowParams(directionalLight);

    // Adjust the camera's projection for the light's shadowFar
    if (UTILS_UNLIKELY(params.options.shadowFar > 0.0f)) {
        cameraInfo.zf = params.options.shadowFar;
        updateNearFarPlanes(&cameraInfo.cullingProjection, cameraInfo.zn, cameraInfo.zf);
    }

    const ShadowMap::ShadowMapInfo shadowMapInfo{
            .atlasDimension      = mTextureAtlasRequirements.size,
            .textureDimension    = uint16_t(options.mapSize),
            .shadowDimension     = uint16_t(options.mapSize - 2u),
            .textureSpaceFlipped = engine.getBackend() == Backend::METAL ||
                                   engine.getBackend() == Backend::VULKAN ||
                                   engine.getBackend() == Backend::WEBGPU,
            .vsm                 = view.hasVSM()
    };

    bool hasVisibleShadows = false;
    utils::Slice<ShadowMap> cascadedShadowMaps = getCascadedShadowMap();
    if (!cascadedShadowMaps.empty()) {
        // Even if we have more than one cascade, we cull directional shadow casters against the
        // entire camera frustum, as if we only had a single cascade.
        ShadowMap& shadowMap = cascadedShadowMaps[0];

        const auto direction = lightData.elementAt<FScene::SHADOW_DIRECTION>(0);

        // We compute the directional light's model matrix using the origin's as the light position.
        // The choice of the light's origin initially doesn't matter for a directional light.
        // This will be adjusted later because of how we compute the depth metric for VSM.
        const mat4f MvAtOrigin = ShadowMap::getDirectionalLightViewMatrix(direction,
                normalize(cameraInfo.worldTransform[0].xyz));

        // Compute scene-dependent values shared across all cascades
        ShadowMap::updateSceneInfoDirectional(MvAtOrigin, *scene, sceneInfo);

        // we always do culling without depth clamp, because objects behind the camera
        // must be rendered regardless
        shadowMap.updateDirectional(engine,
                lightData, 0, cameraInfo, shadowMapInfo, sceneInfo, false);

        hasVisibleShadows = shadowMap.hasVisibleShadows();

        if (hasVisibleShadows) {
            Frustum const& frustum = shadowMap.getCamera().getCullingFrustum();
            FView::cullRenderables(engine.getJobSystem(), renderableData, frustum,
                    VISIBLE_DIR_SHADOW_RENDERABLE_BIT);
        }
    }

    ShadowTechnique shadowTechnique{};
    uint32_t directionalShadowsMask = 0;
    uint32_t cascadeHasVisibleShadows = 0;

    if (hasVisibleShadows) {
        uint32_t const cascadeCount = cascadedShadowMaps.size();

        // We divide the camera frustum into N cascades. This gives us N + 1 split positions.
        // The first split position is the near plane; the last split position is the far plane.
        std::array<float, CascadeSplits::SPLIT_COUNT> splitPercentages{};
        splitPercentages[cascadeCount] = 1.0f;
        for (size_t i = 1; i < cascadeCount; i++) {
            splitPercentages[i] = options.cascadeSplitPositions[i - 1];
        }

        const CascadeSplits splits({
                .near = -cameraInfo.zn,
                .far = -cameraInfo.zf,
                .cascadeCount = cascadeCount,
                .splitPositions = splitPercentages
        });

        // The split positions uniform is a float4. To save space, we chop off the first split position
        // (which is the near plane, and doesn't need to be communicated to the shaders).
        static_assert(CONFIG_MAX_SHADOW_CASCADES <= 5,
                "At most, a float4 can fit 4 split positions for 5 shadow cascades");
        float4 wsSplitPositionUniform{ -std::numeric_limits<float>::infinity() };
        std::copy(splits.begin() + 1, splits.end(), &wsSplitPositionUniform[0]);

        mShadowMappingUniforms.cascadeSplits = wsSplitPositionUniform;

        // When computing the required bias we need a half-texel size, so we multiply by 0.5 here.
        // note: normalBias is set to zero for VSM
        const float normalBias = shadowMapInfo.vsm ? 0.0f : 0.5f * lcm.getShadowNormalBias(0);

        for (size_t i = 0, c = cascadedShadowMaps.size(); i < c; i++) {
            // Compute the frustum for the directional light.
            ShadowMap& shadowMap = cascadedShadowMaps[i];
            assert_invariant(shadowMap.getLightIndex() == 0);

            // update cameraInfo culling projection for the cascade
            float const* nearFarPlanes = splits.begin();
            cameraInfo.zn = -nearFarPlanes[i];
            cameraInfo.zf = -nearFarPlanes[i + 1];
            updateNearFarPlanes(&cameraInfo.cullingProjection, cameraInfo.zn, cameraInfo.zf);

            bool const canUseDepthClamp =
                    !view.hasVSM() &&
                    mIsDepthClampSupported &&
                    engine.debug.shadowmap.depth_clamp;

            auto shaderParameters = shadowMap.updateDirectional(engine,
                    lightData, 0, cameraInfo, shadowMapInfo, sceneInfo,
                    canUseDepthClamp);

            if (shadowMap.hasVisibleShadows()) {
                const size_t shadowIndex = shadowMap.getShadowIndex();
                assert_invariant(shadowIndex == i);

                // Texel size is constant for directional light (although that's not true when LISPSM
                // is used, but in that case we're pretending it is).
                const float wsTexelSize = shaderParameters.texelSizeAtOneMeterWs;

                auto& s = mShadowUb.edit();
                s.shadows[shadowIndex].layer = shadowMap.getLayer();
                s.shadows[shadowIndex].lightFromWorldMatrix = shaderParameters.lightSpace;
                s.shadows[shadowIndex].scissorNormalized = shaderParameters.scissorNormalized;
                s.shadows[shadowIndex].normalBias = normalBias * wsTexelSize;
                s.shadows[shadowIndex].texelSizeAtOneMeter = wsTexelSize;
                s.shadows[shadowIndex].elvsm = options.vsm.elvsm;
                s.shadows[shadowIndex].bulbRadiusLs =
                        mSoftShadowOptions.penumbraScale * options.shadowBulbRadius / wsTexelSize;

                shadowTechnique |= ShadowTechnique::SHADOW_MAP;
                cascadeHasVisibleShadows |= 0x1u << i;
            }
        }
    }

    // screen-space contact shadows for the directional light
    float const screenSpaceShadowDistance = options.maxShadowDistance;
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
    cascades |= uint32_t(cascadedShadowMaps.size());
    cascades |= cascadeHasVisibleShadows << 8u;

    mShadowMappingUniforms.directionalShadows = directionalShadowsMask;
    mShadowMappingUniforms.ssContactShadowDistance = screenSpaceShadowDistance;
    mShadowMappingUniforms.cascades = cascades;

    return shadowTechnique;
}

void ShadowMapManager::updateSpotVisibilityMasks(
        uint8_t const visibleLayers,
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

void ShadowMapManager::prepareSpotShadowMap(ShadowMap& shadowMap, FEngine& engine, FView& view,
        CameraInfo const& mainCameraInfo,
        FScene::LightSoa const& lightData, ShadowMap::SceneInfo const& sceneInfo) noexcept {

    const size_t lightIndex = shadowMap.getLightIndex();
    FLightManager::ShadowOptions const* const options = shadowMap.getShadowOptions();

    // update the shadow map frustum/camera
    const ShadowMap::ShadowMapInfo shadowMapInfo{
            .atlasDimension      = mTextureAtlasRequirements.size,
            .textureDimension    = uint16_t(options->mapSize),
            .shadowDimension     = uint16_t(options->mapSize - 2u),
            .textureSpaceFlipped = engine.getBackend() == Backend::METAL ||
                                   engine.getBackend() == Backend::VULKAN ||
                                   engine.getBackend() == Backend::WEBGPU,
            .vsm                 = view.hasVSM()
    };

    auto shaderParameters = shadowMap.updateSpot(engine,
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
        s.shadows[shadowIndex].layer = shadowMap.getLayer();
        s.shadows[shadowIndex].lightFromWorldMatrix = shaderParameters.lightSpace;
        s.shadows[shadowIndex].scissorNormalized = shaderParameters.scissorNormalized;
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

void ShadowMapManager::cullSpotShadowMap(ShadowMap const& shadowMap,
        FEngine const& engine, FView const& view,
        FScene::RenderableSoa& renderableData, utils::Range<uint32_t> range,
        FScene::LightSoa const& lightData) noexcept {
    auto& lcm = engine.getLightManager();

    const size_t lightIndex = shadowMap.getLightIndex();
    const FLightManager::Instance li = lightData.elementAt<FScene::LIGHT_INSTANCE>(lightIndex);

    // compute the frustum for this light
    // for spotlights, we cull shadow casters first because we already know the frustum,
    // this will help us find better near/far plane later
    const auto position = lightData.elementAt<FScene::POSITION_RADIUS>(lightIndex).xyz;
    const auto direction = lightData.elementAt<FScene::DIRECTION>(lightIndex);
    const auto radius = lightData.elementAt<FScene::POSITION_RADIUS>(lightIndex).w;
    const auto outerConeAngle = lcm.getSpotLightOuterCone(li);

    // compute shadow map frustum for culling
    const mat4f Mv = ShadowMap::getDirectionalLightViewMatrix(direction, { 0, 1, 0 }, position);
    const mat4f Mp = mat4f::perspective(outerConeAngle * f::RAD_TO_DEG * 2.0f, 1.0f, 0.01f, radius);
    const mat4f MpMv = highPrecisionMultiply(Mp, Mv);
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
}

void ShadowMapManager::preparePointShadowMap(ShadowMap& shadowMap,
        FEngine& engine, FView& view, CameraInfo const& mainCameraInfo,
        FScene::LightSoa const& lightData) const noexcept {

    const uint8_t face = shadowMap.getFace();
    const size_t lightIndex = shadowMap.getLightIndex();
    FLightManager::ShadowOptions const* const options = shadowMap.getShadowOptions();

    // update the shadow map frustum/camera
    const ShadowMap::ShadowMapInfo shadowMapInfo{
            .atlasDimension      = mTextureAtlasRequirements.size,
            .textureDimension    = uint16_t(options->mapSize),
            .shadowDimension     = uint16_t(options->mapSize), // point-lights don't have a border
            .textureSpaceFlipped = engine.getBackend() == Backend::METAL ||
                                   engine.getBackend() == Backend::VULKAN ||
                                   engine.getBackend() == Backend::WEBGPU,
            .vsm                 = view.hasVSM()
    };

    auto shaderParameters = shadowMap.updatePoint(engine, lightData, lightIndex,
            mainCameraInfo, shadowMapInfo, *view.getScene(), face);

    // and if we need to generate it, update all the UBO data
    if (shadowMap.hasVisibleShadows()) {
        const size_t shadowIndex = shadowMap.getShadowIndex();
        const float wsTexelSizeAtOneMeter = shaderParameters.texelSizeAtOneMeterWs;
        // note: normalBias is set to zero for VSM
        const float normalBias = shadowMapInfo.vsm ? 0.0f : options->normalBias;

        auto& s = mShadowUb.edit();
        const double n = shadowMap.getCamera().getNear();
        const double f = shadowMap.getCamera().getCullingFar();
        s.shadows[shadowIndex].layer = shadowMap.getLayer();
        s.shadows[shadowIndex].lightFromWorldMatrix = shaderParameters.lightSpace;
        s.shadows[shadowIndex].scissorNormalized = shaderParameters.scissorNormalized;
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

void ShadowMapManager::cullPointShadowMap(ShadowMap const& shadowMap, FView const& view,
        FScene::RenderableSoa& renderableData, utils::Range<uint32_t> const range,
        FScene::LightSoa const& lightData) noexcept {

    uint8_t const face = shadowMap.getFace();
    size_t const lightIndex = shadowMap.getLightIndex();

    // compute the frustum for this light
    // for spotlights, we cull shadow casters first because we already know the frustum,
    // this will help us find better near/far plane later
    auto const position = lightData.elementAt<FScene::POSITION_RADIUS>(lightIndex).xyz;
    auto const radius = lightData.elementAt<FScene::POSITION_RADIUS>(lightIndex).w;

    // compute shadow map frustum for culling
    mat4f const Mv = ShadowMap::getPointLightViewMatrix(TextureCubemapFace(face), position);
    mat4f const Mp = mat4f::perspective(90.0f, 1.0f, 0.01f, radius);
    Frustum const frustum{ highPrecisionMultiply(Mp, Mv) };

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
}

ShadowMapManager::ShadowTechnique ShadowMapManager::updateSpotShadowMaps(FEngine& engine,
        FScene::LightSoa const& lightData) const noexcept {

    // The const_cast here is a little ugly, but conceptually lightData should be const,
    // it's just that we're using it to store some temporary data. With SoA we can't have
    // a `mutable` element, so that's a workaround.
    FScene::ShadowInfo* const shadowInfo = const_cast<FScene::ShadowInfo*>(
            lightData.data<FScene::SHADOW_INFO>());

    ShadowTechnique shadowTechnique{};
    utils::Slice<ShadowMap> const spotShadowMaps = getSpotShadowMaps();
    if (!spotShadowMaps.empty()) {
        shadowTechnique |= ShadowTechnique::SHADOW_MAP;
        for (ShadowMap const& shadowMap : spotShadowMaps) {
            const size_t lightIndex = shadowMap.getLightIndex();
            // Gather the per-light (not per shadow map) information. For point lights we will
            // "see" 6 shadowmaps (one per face), we must use the first face one, the shader
            // knows how to find the entry for other faces (they're guaranteed to be sequential).
            if (shadowMap.getFace() == 0) {
                shadowInfo[lightIndex].castsShadows = true;     // FIXME: is that set correctly?
                shadowInfo[lightIndex].index = shadowMap.getShadowIndex();
            }
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
        FScene::LightSoa const&) noexcept {

    uint32_t maxDimension = 0;
    bool elvsm = false;

    for (ShadowMap const& shadowMap : getCascadedShadowMap()) {
        // Shadow map size should be the same for all cascades.
        auto const& options = shadowMap.getShadowOptions();
        maxDimension = std::max(maxDimension, options->mapSize);
        elvsm = elvsm || options->vsm.elvsm;
    }

    for (ShadowMap const& shadowMap : getSpotShadowMaps()) {
        auto const& options = shadowMap.getShadowOptions();
        maxDimension = std::max(maxDimension, options->mapSize);
        elvsm = elvsm || options->vsm.elvsm;
    }

    uint8_t layersNeeded = 0;

    std::function const allocateFromAtlas =
            [&layersNeeded, allocator = AtlasAllocator{ maxDimension }](
        ShadowMap* pShadowMap) mutable {
        // Allocate shadowmap from our Atlas Allocator
        auto const& options = pShadowMap->getShadowOptions();
        auto [layer, pos] = allocator.allocate(options->mapSize);
        assert_invariant(layer >= 0);
        assert_invariant(!pos.empty());
        pShadowMap->setAllocation(layer, pos);
        layersNeeded = std::max(uint8_t(layer + 1), layersNeeded);
    };

    std::function const allocateFromTextureArray =
            [&layersNeeded, layer = 0](ShadowMap* pShadowMap) mutable {
        // Layout the shadow maps. For now, we take the largest requested dimension and allocate a
        // texture of that size. Each cascade / shadow map gets its own layer in the array texture.
        // The directional shadow cascades start on layer 0, followed by spotlights.
        pShadowMap->setAllocation(layer, {});
        layersNeeded = ++layer;
    };

    auto& allocateShadowmapTexture = mFeatureShadowAllocator ?
        allocateFromAtlas : allocateFromTextureArray;

    for (ShadowMap& shadowMap : getCascadedShadowMap()) {
        allocateShadowmapTexture(&shadowMap);
    }
    for (ShadowMap& shadowMap : getSpotShadowMaps()) {
        allocateShadowmapTexture(&shadowMap);
    }

    // Generate mipmaps for VSM when anisotropy is enabled or when requested
    auto const& vsmShadowOptions = view.getVsmShadowOptions();
    const bool useMipmapping = view.hasVSM() &&
            ((vsmShadowOptions.anisotropy > 0) || vsmShadowOptions.mipmapping);

    uint8_t msaaSamples = vsmShadowOptions.msaaSamples;
    if (engine.getDriverApi().isWorkaroundNeeded(Workaround::DISABLE_BLIT_INTO_TEXTURE_ARRAY)) {
        msaaSamples = 1;
    }

    TextureFormat format = TextureFormat::DEPTH16;
    if (view.hasVSM()) {
        if (vsmShadowOptions.highPrecision) {
            format = elvsm ? TextureFormat::RGBA32F : TextureFormat::RG32F;
        } else {
            format = elvsm ? TextureFormat::RGBA16F : TextureFormat::RG16F;
        }
    }

    mSoftShadowOptions = view.getSoftShadowOptions();

    uint8_t mipLevels = 1u;
    if (useMipmapping) {
        // Limit the lowest mipmap level to 256x256.
        // This avoids artifacts on high derivative tangent surfaces.
        constexpr int lowMipmapLevel = 7;    // log2(256) - 1
        mipLevels = std::max(1, FTexture::maxLevelCount(maxDimension) - lowMipmapLevel);
    }

    // publish the debugging data
    engine.debug.shadowmap.display_shadow_texture_layer_count = layersNeeded;
    engine.debug.shadowmap.display_shadow_texture_level_count = mipLevels;

    mTextureAtlasRequirements = {
            (uint16_t)maxDimension,
            layersNeeded,
            mipLevels,
            msaaSamples,
            format
    };
}

ShadowMapManager::CascadeSplits::CascadeSplits(Params const& params) noexcept
        : mSplitCount(params.cascadeCount + 1) {
    for (size_t s = 0; s < mSplitCount; s++) {
        mSplitsWs[s] = params.near + (params.far - params.near) * params.splitPositions[s];
    }
}

void ShadowMapManager::updateNearFarPlanes(mat4f* projection,
        float nearDistance, float farDistance) noexcept {
    float const n = nearDistance;
    float const f = farDistance;

    /*
     *  Updating a projection matrix near and far planes:
     *
     *  We assume that the near and far plane equations are of the form:
     *           N = { 0, 0,  1,  n }
     *           F = { 0, 0, -1, -f }
     *
     *  We also assume that the lower-left 2x2 of the projection is all 0:
     *       P =   A   0   C   0
     *             0   B   D   0
     *             0   0   E   F
     *             0   0   G   H
     *
     *  It result that we need to calculate E and F while leaving all other parameter unchanged.
     *
     *  We know that:
     *       with N, F the near/far normalized plane equation parameters
     *            sn, sf arbitrary scale factors (they don't affect the planes)
     *            m is the transpose of projection (see Frustum.cpp)
     *
     *       sn * N == -m[3] - m[2]
     *       sf * F == -m[3] + m[2]
     *
     *       sn * N + sf * F == -2 * m[3]
     *       sn * N - sf * F == -2 * m[2]
     *
     *       sn * N.z + sf * F.z == -2 * m[3].z
     *       sn * N.w + sf * F.w == -2 * m[3].w
     *       sn * N.z - sf * F.z == -2 * m[2].z
     *       sn * N.w - sf * F.w == -2 * m[2].w
     *
     *       sn * N.z + sf * F.z == -2 * p[2].w
     *       sn * N.w + sf * F.w == -2 * p[3].w
     *       sn * N.z - sf * F.z == -2 * p[2].z
     *       sn * N.w - sf * F.w == -2 * p[3].z
     *
     *  We now need to solve for { p[2].z, p[3].z, sn, sf } :
     *
     *       sn == -2 * (p[2].w * F.w - p[3].w * F.z) / (N.z * F.w - N.w * F.z)
     *       sf == -2 * (p[2].w * N.w - p[3].w * N.z) / (F.z * N.w - F.w * N.z)
     *   p[2].z == (sf * F.z - sn * N.z) / 2
     *   p[3].z == (sf * F.w - sn * N.w) / 2
     */
    auto& p = *projection;
    float4 const N = { 0, 0,  1,  n };  // near plane equation
    float4 const F = { 0, 0, -1, -f };  // far plane equation
    // near plane equation scale factor
    float const sn = -2.0f * (p[2].w * F.w - p[3].w * F.z) / (N.z * F.w - N.w * F.z);
    // far plane equation scale factor
    float const sf = -2.0f * (p[2].w * N.w - p[3].w * N.z) / (F.z * N.w - F.w * N.z);
    // New values for the projection
    p[2].z = (sf * F.z - sn * N.z) * 0.5f;
    p[3].z = (sf * F.w - sn * N.w) * 0.5f;
}

utils::FixedCapacityVector<Camera const*>
ShadowMapManager::getDirectionalShadowCameras() const noexcept {
    if (!mInitialized) return {};
    auto const csm = getCascadedShadowMap();
    auto result = utils::FixedCapacityVector<Camera const*>::with_capacity(csm.size());
    for (ShadowMap const& sm : csm) {
        result.push_back(sm.hasVisibleShadows() ? sm.getDebugCamera() : nullptr);
    }
    return result;
}

} // namespace filament
