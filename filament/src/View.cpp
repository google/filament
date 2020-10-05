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

#include "details/View.h"

#include "ResourceAllocator.h"

#include "details/Engine.h"
#include "details/Culler.h"
#include "details/DFG.h"
#include "details/Froxelizer.h"
#include "details/IndirectLight.h"
#include "details/Renderer.h"
#include "details/RenderTarget.h"
#include "details/Scene.h"
#include "details/Skybox.h"

#include <filament/Exposure.h>
#include <filament/TextureSampler.h>

#include <private/filament/SibGenerator.h>
#include <private/filament/UibGenerator.h>

#include <utils/Profiler.h>
#include <utils/Slice.h>
#include <utils/Systrace.h>

#include <math/scalar.h>
#include <math/fast.h>

#include <memory>
#include <filament/View.h>

using namespace filament::math;
using namespace utils;

namespace filament {

using namespace backend;

FView::FView(FEngine& engine)
    : mFroxelizer(engine),
      mPerViewUb(PerViewUib::getUib().getSize()),
      mShadowUb(ShadowUib::getUib().getSize()),
      mPerViewSb(PerViewSib::SAMPLER_COUNT),
      mShadowMapManager(engine) {
    DriverApi& driver = engine.getDriverApi();

    FDebugRegistry& debugRegistry = engine.getDebugRegistry();
    debugRegistry.registerProperty("d.view.camera_at_origin",
            &engine.debug.view.camera_at_origin);

    // set-up samplers
    mFroxelizer.getRecordBuffer().setSampler(PerViewSib::RECORDS, mPerViewSb);
    mFroxelizer.getFroxelBuffer().setSampler(PerViewSib::FROXELS, mPerViewSb);
    if (engine.getDFG()->isValid()) {
        TextureSampler sampler(TextureSampler::MagFilter::LINEAR);
        mPerViewSb.setSampler(PerViewSib::IBL_DFG_LUT,
                engine.getDFG()->getTexture(), sampler.getSamplerParams());
    }
    mPerViewSbh = driver.createSamplerGroup(mPerViewSb.getSize());

    // allocate ubos
    mPerViewUbh = driver.createUniformBuffer(mPerViewUb.getSize(), backend::BufferUsage::DYNAMIC);
    mLightUbh = driver.createUniformBuffer(CONFIG_MAX_LIGHT_COUNT * sizeof(LightsUib), backend::BufferUsage::DYNAMIC);
    mShadowUbh = driver.createUniformBuffer(mShadowUb.getSize(), backend::BufferUsage::DYNAMIC);

    mIsDynamicResolutionSupported = driver.isFrameTimeSupported();

    // with a clip-space of [-w, w] ==> z' = -z
    // with a clip-space of [0,  w] ==> z' = (w - z)/2
    mClipControl = driver.getClipSpaceParams();

    mDefaultColorGrading = mColorGrading = engine.getDefaultColorGrading();
}

FView::~FView() noexcept = default;

void FView::terminate(FEngine& engine) {
    // Here we would cleanly free resources we've allocated or we own (currently none).
    DriverApi& driver = engine.getDriverApi();
    driver.destroyUniformBuffer(mPerViewUbh);
    driver.destroyUniformBuffer(mLightUbh);
    driver.destroyUniformBuffer(mShadowUbh);
    driver.destroySamplerGroup(mPerViewSbh);
    driver.destroyUniformBuffer(mRenderableUbh);
    drainFrameHistory(engine);
    mFroxelizer.terminate(driver);
}

void FView::setViewport(filament::Viewport const& viewport) noexcept {
    // catch the cases were user had an underflow and didn't catch it.
    assert((int32_t)viewport.width > 0);
    assert((int32_t)viewport.height > 0);
    mViewport = viewport;
}

void FView::setDynamicResolutionOptions(DynamicResolutionOptions const& options) noexcept {
    DynamicResolutionOptions& dynamicResolution = mDynamicResolution;
    dynamicResolution = options;

    // only enable if dynamic resolution is supported
    dynamicResolution.enabled = dynamicResolution.enabled && mIsDynamicResolutionSupported;
    if (dynamicResolution.enabled) {
        // if enabled, sanitize the parameters

        // minScale cannot be 0 or negative
        dynamicResolution.minScale = max(dynamicResolution.minScale, float2(1.0f / 1024.0f));

        // maxScale cannot be < minScale
        dynamicResolution.maxScale = max(dynamicResolution.maxScale, dynamicResolution.minScale);

        // clamp maxScale to 2x because we're doing bilinear filtering, so super-sampling
        // is not useful above that.
        dynamicResolution.maxScale = min(dynamicResolution.maxScale, float2(2.0f));
    }
}

void FView::setDynamicLightingOptions(float zLightNear, float zLightFar) noexcept {
    mFroxelizer.setOptions(zLightNear, zLightFar);
}

float2 FView::updateScale(FrameInfo const& info) noexcept {
    DynamicResolutionOptions const& options = mDynamicResolution;
    if (options.enabled) {
        if (!UTILS_UNLIKELY(info.valid)) {
            mScale = 1.0f;
            return mScale;
        }

        // scaling factor we need to apply on the whole surface
        const float scale = info.scale;
        const float w = mViewport.width;
        const float h = mViewport.height;
        if (scale < 1.0f && !options.homogeneousScaling) {
            // figure out the major and minor axis
            const float major = std::max(w, h);
            const float minor = std::min(w, h);

            // the major axis is scaled down first, down to the minor axis
            const float maxMajorScale = minor / major;
            const float majorScale = std::max(scale, maxMajorScale);

            // then the minor axis is scaled down to the original aspect-ratio
            const float minorScale = std::max(scale / majorScale, majorScale * maxMajorScale);

            // if we have some scaling capacity left, scale homogeneously
            const float homogeneousScale = scale / (majorScale * minorScale);

            // finally write the scale factors
            float& majorRef = w > h ? mScale.x : mScale.y;
            float& minorRef = w > h ? mScale.y : mScale.x;
            majorRef = std::sqrt(homogeneousScale) * majorScale;
            minorRef = std::sqrt(homogeneousScale) * minorScale;
        } else {
            // when scaling up, we're always using homogeneous scaling.
            mScale = std::sqrt(scale);
        }

        // now tweak the scaling factor to get multiples of 8 (to help quad-shading)
        // i.e. 8x8=64 fragments, to try to help with warp sizes.
        mScale = (floor(mScale * float2{ w, h } / 8) * 8) / float2{ w, h };

        // always clamp to the min/max scale range
        mScale = clamp(mScale, options.minScale, options.maxScale);

//#define DEBUG_DYNAMIC_RESOLUTION
#if defined(DEBUG_DYNAMIC_RESOLUTION)
        static int sLogCounter = 15;
        if (!--sLogCounter) {
            sLogCounter = 15;
            slog.d << info.denoisedFrameTime.count() * 1000.0f << " ms"
                   << ", " << info.smoothedWorkLoad
                   << ", " << mScale.x
                   << ", " << mScale.y
                   << ", " << mViewport.width  * mScale.x
                   << ", " << mViewport.height * mScale.y
                   << io::endl;
        }
#endif
    } else {
        mScale = 1.0f;
    }

    return mScale;
}

void FView::setVisibleLayers(uint8_t select, uint8_t values) noexcept {
    mVisibleLayers = (mVisibleLayers & ~select) | (values & select);
}

bool FView::isSkyboxVisible() const noexcept {
    FSkybox const* skybox = mScene ? mScene->getSkybox() : nullptr;
    return skybox != nullptr && (skybox->getLayerMask() & mVisibleLayers);
}

void FView::prepareShadowing(FEngine& engine, backend::DriverApi& driver,
        FScene::RenderableSoa& renderableData, FScene::LightSoa& lightData) noexcept {
    SYSTRACE_CALL();

    mHasShadowing = false;
    mNeedsShadowMap = false;

    if (!mShadowingEnabled) {
        return;
    }

    mShadowMapManager.reset();

    auto& lcm = engine.getLightManager();

    // dominant directional light is always as index 0
    FLightManager::Instance directionalLight = lightData.elementAt<FScene::LIGHT_INSTANCE>(0);
    const bool hasDirectionalShadows = directionalLight && lcm.isShadowCaster(directionalLight);
    if (UTILS_UNLIKELY(hasDirectionalShadows)) {
        const auto& shadowOptions = lcm.getShadowOptions(directionalLight);
        assert(shadowOptions.shadowCascades >= 1 &&
                shadowOptions.shadowCascades <= CONFIG_MAX_SHADOW_CASCADES);
        mShadowMapManager.setShadowCascades(0, shadowOptions.shadowCascades);
    }

    // Find all shadow-casting spot lights.
    size_t shadowCastingSpotCount = 0;

    // We allow a max of CONFIG_MAX_SHADOW_CASTING_SPOTS spot light shadows. Any additional
    // shadow-casting spot lights are ignored.
    for (size_t l = 1; l < lightData.size(); l++) {
        FLightManager::Instance light = lightData.elementAt<FScene::LIGHT_INSTANCE>(l);

        // Invisible lights get culled and should not count towards the spot limit.
        bool visible = lightData.elementAt<FScene::VISIBILITY>(l) != 0;

        if (UTILS_LIKELY(!(light && lcm.isSpotLight(light) &&
                lcm.isShadowCaster(light) && visible))) {
            continue;
        }

        mShadowMapManager.addSpotShadowMap(l);

        shadowCastingSpotCount++;
        if (shadowCastingSpotCount > CONFIG_MAX_SHADOW_CASTING_SPOTS - 1) {
            break;
        }
    }

    auto shadowTechnique = mShadowMapManager.update(engine, *this,
            mPerViewUb, mShadowUb, renderableData,lightData);
    mHasShadowing = any(shadowTechnique);
    mNeedsShadowMap = any(shadowTechnique & ShadowMapManager::ShadowTechnique::SHADOW_MAP);
}

void FView::prepareLighting(FEngine& engine, FEngine::DriverApi& driver, ArenaScope& arena,
        filament::Viewport const& viewport) noexcept {
    SYSTRACE_CALL();

    UniformBuffer& u = mPerViewUb;
    const CameraInfo& camera = mViewingCameraInfo;
    FScene* const scene = mScene;

    scene->prepareDynamicLights(camera, arena, mLightUbh);

    // here the array of visible lights has been shrunk to CONFIG_MAX_LIGHT_COUNT
    auto const& lightData = scene->getLightData();

    // trace the number of visible lights
    SYSTRACE_VALUE32("visibleLights", lightData.size() - FScene::DIRECTIONAL_LIGHTS_COUNT);

    // Exposure
    const float ev100 = camera.ev100;
    const float exposure = Exposure::exposure(ev100);
    u.setUniform(offsetof(PerViewUib, exposure), exposure);
    u.setUniform(offsetof(PerViewUib, ev100), ev100);

    // If the scene does not have an IBL, use the black 1x1 IBL and honor the fallback intensity
    // associated with the skybox.
    FIndirectLight const* ibl = scene->getIndirectLight();
    float intensity{};
    if (UTILS_LIKELY(ibl)) {
        intensity = ibl->getIntensity();
    } else {
        ibl = engine.getDefaultIndirectLight();
        FSkybox const* const skybox = scene->getSkybox();
        intensity = skybox ? skybox->getIntensity() : FIndirectLight::DEFAULT_INTENSITY;
    }

    // Set up uniforms and sampler for the IBL, guaranteed to be non-null at this point.
    float iblRoughnessOneLevel = ibl->getLevelCount() - 1.0f;
    u.setUniform(offsetof(PerViewUib, iblRoughnessOneLevel), iblRoughnessOneLevel);
    u.setUniform(offsetof(PerViewUib, iblLuminance), intensity * exposure);
    u.setUniformArray(offsetof(PerViewUib, iblSH), ibl->getSH(), 9);
    if (ibl->getReflectionHwHandle()) {
        mPerViewSb.setSampler(PerViewSib::IBL_SPECULAR, {
                ibl->getReflectionHwHandle(), {
                        .filterMag = SamplerMagFilter::LINEAR,
                        .filterMin = SamplerMinFilter::LINEAR_MIPMAP_LINEAR
                }});
    }

    // Directional light (always at index 0)
    auto& lcm = engine.getLightManager();
    FLightManager::Instance directionalLight = lightData.elementAt<FScene::LIGHT_INSTANCE>(0);
    mHasDirectionalLight = directionalLight.isValid();
    if (mHasDirectionalLight) {
        const float3 l = -lightData.elementAt<FScene::DIRECTION>(0); // guaranteed normalized
        const float4 colorIntensity = {
                lcm.getColor(directionalLight), lcm.getIntensity(directionalLight) * exposure };

        u.setUniform(offsetof(PerViewUib, lightDirection), l);
        u.setUniform(offsetof(PerViewUib, lightColorIntensity), colorIntensity);

        const bool isSun = lcm.isSunLight(directionalLight);
        // The last parameter must be < 0.0f for regular directional lights
        float4 sun{ 0.0f, 0.0f, 0.0f, -1.0f };
        if (UTILS_UNLIKELY(isSun && colorIntensity.w > 0.0f)) {
            // currently we have only a single directional light, so it's probably likely that it's
            // also the Sun. However, conceptually, most directional lights won't be sun lights.
            float radius = lcm.getSunAngularRadius(directionalLight);
            float haloSize = lcm.getSunHaloSize(directionalLight);
            float haloFalloff = lcm.getSunHaloFalloff(directionalLight);
            sun.x = fast::cos(radius);
            sun.y = fast::sin(radius);
            sun.z = 1.0f / (fast::cos(radius * haloSize) - sun.x);
            sun.w = haloFalloff;
        }
        u.setUniform(offsetof(PerViewUib, sun), sun);
    } else {
        // Disable the sun if there's no directional light
        float4 sun{ 0.0f, 0.0f, 0.0f, -1.0f };
        u.setUniform(offsetof(PerViewUib, sun), sun);
    }

    // Dynamic lighting
    mHasDynamicLighting = scene->getLightData().size() > FScene::DIRECTIONAL_LIGHTS_COUNT;
    if (mHasDynamicLighting) {
        Froxelizer& froxelizer = mFroxelizer;
        if (froxelizer.prepare(driver, arena, viewport, camera.projection, camera.zn, camera.zf)) {
            froxelizer.updateUniforms(u); // update our uniform buffer if needed
        }
    }
}

void FView::prepare(FEngine& engine, backend::DriverApi& driver, ArenaScope& arena,
        filament::Viewport const& viewport, float4 const& userTime) noexcept {
    JobSystem& js = engine.getJobSystem();

    /*
     * Prepare the scene -- this is where we gather all the objects added to the scene,
     * and in particular their world-space AABB.
     */

    FScene* const scene = getScene();

    /*
     * We apply a "world origin" to "everything" in order to implement the IBL rotation.
     * The "world origin" could also be useful for other things, like keeping the origin
     * close to the camera position to improve fp precision in the shader for large scenes.
     */
    mat4f worldOriginScene;
    FIndirectLight const* const ibl = scene->getIndirectLight();
    if (ibl) {
        // the IBL transformation must be a rigid transform
        mat3f rotation{ scene->getIndirectLight()->getRotation() };
        // for a rigid-body transform, the inverse is the transpose
        worldOriginScene = mat4f{ transpose(rotation) };
    }

    /*
     * Calculate all camera parameters needed to render this View for this frame.
     */
    FCamera const* const camera = mViewingCamera ? mViewingCamera : mCullingCamera;

    if (engine.debug.view.camera_at_origin) {
        // this moves the camera to the origin, effectively doing all shader computations in
        // view-space, which improves floating point precision in the shader by staying around
        // zero, where fp precision is highest. This also ensures that when the camera is placed
        // very far from the origin, objects are still rendered and lit properly.
        worldOriginScene[3].xyz -= camera->getPosition();
    }

    // Note: for debugging (i.e. visualize what the camera / objects are doing, using
    // the viewing camera), we can set worldOriginScene to identity when mViewingCamera
    // is set
    mViewingCameraInfo = CameraInfo(*camera, worldOriginScene);

    mCullingFrustum = FCamera::getFrustum(
            mCullingCamera->getCullingProjectionMatrix(),
            FCamera::getViewMatrix(worldOriginScene * mCullingCamera->getModelMatrix()));

    /*
     * Gather all information needed to render this scene. Apply the world origin to all
     * objects in the scene.
     */
    scene->prepare(worldOriginScene);

    /*
     * Light culling: runs in parallel with Renderable culling (below)
     */

    auto *prepareVisibleLightsJob = js.runAndRetain(js.createJob(nullptr,
            [&frustum = mCullingFrustum, &engine, scene](JobSystem& js, JobSystem::Job*) {
                FView::prepareVisibleLights(
                        engine.getLightManager(), js, frustum, scene->getLightData());
            }));

    Range merged;
    FScene::RenderableSoa& renderableData = scene->getRenderableData();

    { // all the operations in this scope must happen sequentially

        Slice<Culler::result_type> cullingMask = renderableData.slice<FScene::VISIBLE_MASK>();
        std::uninitialized_fill(cullingMask.begin(), cullingMask.end(), 0);

        /*
         * Culling: as soon as possible we perform our camera-culling
         * (this will set the VISIBLE_RENDERABLE bit)
         */

        prepareVisibleRenderables(js, mCullingFrustum, renderableData);


        /*
         * Shadowing: compute the shadow camera and cull shadow casters
         * (this will set the VISIBLE_DIR_SHADOW_CASTER bit and VISIBLE_SPOT_SHADOW_CASTER bits)
         */

        // prepareShadowing relies on prepareVisibleLights().
        js.waitAndRelease(prepareVisibleLightsJob);
        prepareShadowing(engine, driver, renderableData, scene->getLightData());

        /*
         * Partition the SoA so that renderables are partitioned w.r.t their visibility into the
         * following groups:
         *
         * 1. renderables
         * 2. renderables and directional shadow casters
         * 3. directional shadow casters only
         * 4. punctual light shadow casters only
         * 5. invisible renderables
         *
         * Note that the first three groups are partitioned based only on the lowest two bits of the
         * VISIBLE_MASK (VISIBLE_RENDERABLE and VISIBLE_DIR_SHADOW_CASTER), and thus can also
         * contain punctual light shadow casters as well. The fourth group contains *only* punctual
         * shadow casters.
         *
         * This operation is somewhat heavy as it sorts the whole SoA. We use std::partition instead
         * of sort(), which gives us O(4.N) instead of O(N.log(N)) application of swap().
         */

        // calculate the sorting key for all elements, based on their visibility
        uint8_t const* layers = renderableData.data<FScene::LAYERS>();
        auto const* visibility = renderableData.data<FScene::VISIBILITY_STATE>();
        computeVisibilityMasks(getVisibleLayers(), layers, visibility, cullingMask.begin(),
                renderableData.size(), hasVsm());

        auto const beginRenderables = renderableData.begin();
        auto beginCasters = partition(beginRenderables, renderableData.end(), VISIBLE_RENDERABLE);
        auto beginCastersOnly = partition(beginCasters, renderableData.end(),
                VISIBLE_RENDERABLE | VISIBLE_DIR_SHADOW_RENDERABLE);
        auto beginSpotLightCastersOnly = partition(beginCastersOnly, renderableData.end(),
                VISIBLE_DIR_SHADOW_RENDERABLE);
        auto endSpotLightCastersOnly = std::partition(beginSpotLightCastersOnly,
                renderableData.end(), [](auto it) {
                    return (it.template get<FScene::VISIBLE_MASK>() & VISIBLE_SPOT_SHADOW_RENDERABLE);
                });

        // convert to indices
        uint32_t iEnd = uint32_t(beginSpotLightCastersOnly - beginRenderables);
        uint32_t iSpotLightCastersEnd = uint32_t(endSpotLightCastersOnly - beginRenderables);
        mVisibleRenderables = Range{ 0, uint32_t(beginCastersOnly - beginRenderables) };
        mVisibleDirectionalShadowCasters = Range{ uint32_t(beginCasters - beginRenderables), iEnd };
        mSpotLightShadowCasters = Range{ 0, iSpotLightCastersEnd };
        merged = Range{ 0, iSpotLightCastersEnd };

        // update those UBOs
        const size_t size = merged.size() * sizeof(PerRenderableUib);
        if (size) {
            if (mRenderableUBOSize < size) {
                // allocate 1/3 extra, with a minimum of 16 objects
                const size_t count = std::max(size_t(16u), (4u * merged.size() + 2u) / 3u);
                mRenderableUBOSize = uint32_t(count * sizeof(PerRenderableUib));
                driver.destroyUniformBuffer(mRenderableUbh);
                mRenderableUbh = driver.createUniformBuffer(mRenderableUBOSize,
                        backend::BufferUsage::STREAM);
            } else {
                // TODO: should we shrink the underlying UBO at some point?
            }
            assert(mRenderableUbh);
            scene->updateUBOs(merged, mRenderableUbh);
        }
    }

    /*
     * Prepare lighting -- this is where we update the lights UBOs, set-up the IBL,
     * set-up the froxelization parameters.
     * Relies on FScene::prepare() and prepareVisibleLights()
     */

    prepareLighting(engine, driver, arena, viewport);

    /*
     * Update driver state
     */

    auto& u = mPerViewUb;

    const uint64_t oneSecondRemainder = engine.getEngineTime().count() % 1000000000;
    const float fraction = float(double(oneSecondRemainder) / 1000000000.0);
    u.setUniform(offsetof(PerViewUib, time), fraction);
    u.setUniform(offsetof(PerViewUib, userTime), userTime);

    auto const& fogOptions = mFogOptions;

    // this can't be too high because we need density / heightFalloff to produce something
    // close to fogOptions.density in the fragment shader which use 16-bits floats.
    constexpr float epsilon = 0.001f;
    const float heightFalloff = std::max(epsilon, fogOptions.heightFalloff);

    // precalculate the constant part of density  integral and correct for exp2() in the shader
    const float density = ((fogOptions.density / heightFalloff) *
            std::exp(-heightFalloff * (camera->getPosition().y - fogOptions.height)))
                    * float(1.0f / F_LN2);

    u.setUniform(offsetof(PerViewUib, fogStart),             fogOptions.distance);
    u.setUniform(offsetof(PerViewUib, fogMaxOpacity),        fogOptions.maximumOpacity);
    u.setUniform(offsetof(PerViewUib, fogHeight),            fogOptions.height);
    u.setUniform(offsetof(PerViewUib, fogHeightFalloff),     heightFalloff);
    u.setUniform(offsetof(PerViewUib, fogColor),             fogOptions.color);
    u.setUniform(offsetof(PerViewUib, fogDensity),           density);
    u.setUniform(offsetof(PerViewUib, fogInscatteringStart), fogOptions.inScatteringStart);
    u.setUniform(offsetof(PerViewUib, fogInscatteringSize),  fogOptions.inScatteringSize);
    u.setUniform(offsetof(PerViewUib, fogColorFromIbl),      fogOptions.fogColorFromIbl ? 1.0f : 0.0f);

    // upload the renderables's dirty UBOs
    engine.getRenderableManager().prepare(driver,
            renderableData.data<FScene::RENDERABLE_INSTANCE>(), merged);

    // set uniforms and samplers
    bindPerViewUniformsAndSamplers(driver);
}

void FView::computeVisibilityMasks(
        uint8_t visibleLayers,
        uint8_t const* UTILS_RESTRICT layers,
        FRenderableManager::Visibility const* UTILS_RESTRICT visibility,
        uint8_t* UTILS_RESTRICT visibleMask, size_t count, bool hasVsm) {
    // __restrict__ seems to only be taken into account as function parameters. This is very
    // important here, otherwise, this loop doesn't get vectorized.
    // This is vectorized 16x.
    count = (count + 0xFu) & ~0xFu; // capacity guaranteed to be multiple of 16
    for (size_t i = 0; i < count; ++i) {
        Culler::result_type mask = visibleMask[i];
        FRenderableManager::Visibility v = visibility[i];
        bool inVisibleLayer = layers[i] & visibleLayers;

        // The logic below essentially does the following:
        //
        // if inVisibleLayer:
        //     if !v.culling:
        //         set all bits in visibleMask to 1
        // else:
        //     set all bits in visibleMask to 0
        // if !v.castShadows:
        //     if !vsm or !v.receivesShadows:       // with vsm, we also render shadow receivers
        //         set shadow visibility bits in visibleMask to 0
        //
        // It is written without if statements to avoid branches, which allows it to be vectorized 16x.

        const bool visRenderables   = (!v.culling || (mask & VISIBLE_RENDERABLE))    && inVisibleLayer;
        const bool vvsmRenderShadow = hasVsm && v.receiveShadows;
        const bool visShadowParticipant = v.castShadows || vvsmRenderShadow;
        const bool visShadowRenderable =
            (!v.culling || (mask & VISIBLE_DIR_SHADOW_RENDERABLE)) && inVisibleLayer && visShadowParticipant;
        visibleMask[i] = Culler::result_type(visRenderables) |
                Culler::result_type(visShadowRenderable << 1u);
        // this loop gets fully unrolled
        for (size_t j = 0; j < CONFIG_MAX_SHADOW_CASTING_SPOTS; ++j) {
            const bool vvsmSpotRenderShadow = hasVsm && v.receiveShadows;
            const bool visSpotShadowParticipant = v.castShadows || vvsmSpotRenderShadow;
            const bool visSpotShadowRenderable =
                (!v.culling || (mask & VISIBLE_SPOT_SHADOW_RENDERABLE_N(j))) &&
                        inVisibleLayer && visSpotShadowParticipant;
            visibleMask[i] |=
                Culler::result_type(visSpotShadowRenderable << VISIBLE_SPOT_SHADOW_RENDERABLE_N_BIT(j));
        }
    }
}

UTILS_NOINLINE
/* static */ FScene::RenderableSoa::iterator FView::partition(
        FScene::RenderableSoa::iterator begin,
        FScene::RenderableSoa::iterator end,
        uint8_t mask) noexcept {
    return std::partition(begin, end, [mask](auto it) {
        // Mask VISIBLE_MASK to ignore higher bits related to spot shadows. We only partition based
        // on renderable and directional shadow visibility.
        return (it.template get<FScene::VISIBLE_MASK>() &
                (VISIBLE_RENDERABLE | VISIBLE_DIR_SHADOW_RENDERABLE)) == mask;
    });
}

void FView::prepareCamera(const CameraInfo& camera) const noexcept {
    SYSTRACE_CALL();

    const mat4f viewFromWorld(camera.view);
    const mat4f worldFromView(camera.model);
    const mat4f projectionMatrix(camera.projection);

    const mat4f clipFromView(projectionMatrix);
    const mat4f viewFromClip(inverse(clipFromView));
    const mat4f clipFromWorld(clipFromView * viewFromWorld);
    const mat4f worldFromClip(worldFromView * viewFromClip);

    UniformBuffer& u = mPerViewUb;
    u.setUniform(offsetof(PerViewUib, viewFromWorldMatrix), viewFromWorld);    // view
    u.setUniform(offsetof(PerViewUib, worldFromViewMatrix), worldFromView);    // model
    u.setUniform(offsetof(PerViewUib, clipFromViewMatrix), clipFromView);      // projection
    u.setUniform(offsetof(PerViewUib, viewFromClipMatrix), viewFromClip);      // 1/projection
    u.setUniform(offsetof(PerViewUib, clipFromWorldMatrix), clipFromWorld);    // projection * view
    u.setUniform(offsetof(PerViewUib, worldFromClipMatrix), worldFromClip);    // 1/(projection * view)
    u.setUniform(offsetof(PerViewUib, cameraPosition), float3{camera.getPosition()});
    u.setUniform(offsetof(PerViewUib, worldOffset), camera.worldOffset);
    u.setUniform(offsetof(PerViewUib, cameraFar), camera.zf);
    u.setUniform(offsetof(PerViewUib, clipControl), mClipControl);

}

void FView::prepareViewport(const filament::Viewport &viewport) const noexcept {
    SYSTRACE_CALL();
    UniformBuffer& u = mPerViewUb;
    const float w = viewport.width;
    const float h = viewport.height;
    u.setUniform(offsetof(PerViewUib, resolution), float4{ w, h, 1.0f / w, 1.0f / h });
    u.setUniform(offsetof(PerViewUib, origin), float2{ viewport.left, viewport.bottom });
}

void FView::prepareSSAO(Handle<HwTexture> ssao) const noexcept {
    // High quality sampling is enabled only if AO itself is enabled and upsampling quality is at
    // least set to high and of course only if upsampling is needed.
    const bool highQualitySampling = mAmbientOcclusionOptions.upsampling >= QualityLevel::HIGH
            && mAmbientOcclusionOptions.resolution < 1.0f;

    // LINEAR filtering is only needed when AO is enabled and low-quality upsampling is used.
    mPerViewSb.setSampler(PerViewSib::SSAO, ssao, {
            .filterMag = mAmbientOcclusionOptions.enabled && !highQualitySampling ?
                         SamplerMagFilter::LINEAR : SamplerMagFilter::NEAREST
    });

    const float edgeDistance = 1.0 / 0.0625;// TODO: don't hardcode this
    mPerViewUb.setUniform(offsetof(PerViewUib, aoSamplingQualityAndEdgeDistance),
            mAmbientOcclusionOptions.enabled && highQualitySampling ? edgeDistance : 0.0f);
}

void FView::prepareSSR(backend::Handle<backend::HwTexture> ssr, float refractionLodOffset) const noexcept {
    mPerViewSb.setSampler(PerViewSib::SSR, ssr, {
            .filterMag = SamplerMagFilter::LINEAR,
            .filterMin = SamplerMinFilter::LINEAR_MIPMAP_LINEAR
    });
    mPerViewUb.setUniform(offsetof(PerViewUib, refractionLodOffset), refractionLodOffset);
}

void FView::prepareStructure(backend::Handle<backend::HwTexture> structure) const noexcept {
    // sampler must be NEAREST
    mPerViewSb.setSampler(PerViewSib::STRUCTURE, structure, {});
}

void FView::prepareShadow(backend::Handle<backend::HwTexture> texture) const noexcept {
    mShadowMapManager.prepareShadow(texture, mPerViewSb);
}

void FView::cleanupRenderPasses() const noexcept {
    auto& samplerGroup = mPerViewSb;
    samplerGroup.setSampler(PerViewSib::SSAO, {}, {});
    samplerGroup.setSampler(PerViewSib::SSR, {}, {});
    samplerGroup.setSampler(PerViewSib::STRUCTURE, {}, {});
}

void FView::froxelize(FEngine& engine) const noexcept {
    SYSTRACE_CALL();

    if (mHasDynamicLighting) {
        // froxelize lights
        mFroxelizer.froxelizeLights(engine, mViewingCameraInfo, mScene->getLightData());
    }
}

void FView::commitUniforms(backend::DriverApi& driver) const noexcept {
    if (mPerViewUb.isDirty()) {
        driver.loadUniformBuffer(mPerViewUbh, mPerViewUb.toBufferDescriptor(driver));
    }

    if (mShadowUb.isDirty()) {
        driver.loadUniformBuffer(mShadowUbh, mShadowUb.toBufferDescriptor(driver));
    }

    if (mPerViewSb.isDirty()) {
        driver.updateSamplerGroup(mPerViewSbh, std::move(mPerViewSb.toCommandStream()));
    }
}

void FView::commitFroxels(backend::DriverApi& driverApi) const noexcept {
    if (mHasDynamicLighting) {
        mFroxelizer.commit(driverApi);
    }
}

UTILS_NOINLINE
void FView::prepareVisibleRenderables(JobSystem& js,
        Frustum const& frustum, FScene::RenderableSoa& renderableData) const noexcept {
    SYSTRACE_CALL();
    if (UTILS_LIKELY(isFrustumCullingEnabled())) {
        FView::cullRenderables(js, renderableData, frustum, VISIBLE_RENDERABLE_BIT);
    } else {
        std::uninitialized_fill(renderableData.begin<FScene::VISIBLE_MASK>(),
                  renderableData.end<FScene::VISIBLE_MASK>(), VISIBLE_RENDERABLE);
    }
}

void FView::cullRenderables(JobSystem& js,
        FScene::RenderableSoa& renderableData, Frustum const& frustum, size_t bit) noexcept {

    float3 const* worldAABBCenter = renderableData.data<FScene::WORLD_AABB_CENTER>();
    float3 const* worldAABBExtent = renderableData.data<FScene::WORLD_AABB_EXTENT>();
    FScene::VisibleMaskType* visibleArray = renderableData.data<FScene::VISIBLE_MASK>();

    // culling job (this runs on multiple threads)
    auto functor = [&frustum, worldAABBCenter, worldAABBExtent, visibleArray, bit]
            (uint32_t index, uint32_t c) {
        Culler::intersects(
                visibleArray + index,
                frustum,
                worldAABBCenter + index,
                worldAABBExtent + index, c, bit);
    };

    // launch the computation on multiple threads
    auto *job = jobs::parallel_for(js, nullptr, 0, (uint32_t)renderableData.size(),
            std::ref(functor), jobs::CountSplitter<Culler::MODULO * Culler::MIN_LOOP_COUNT_HINT, 8>());
    js.runAndWait(job);
}

void FView::prepareVisibleLights(FLightManager const& lcm, utils::JobSystem&,
        Frustum const& frustum, FScene::LightSoa& lightData) noexcept {
    SYSTRACE_CALL();

    auto const* UTILS_RESTRICT sphereArray     = lightData.data<FScene::POSITION_RADIUS>();
    auto const* UTILS_RESTRICT directions      = lightData.data<FScene::DIRECTION>();
    auto const* UTILS_RESTRICT instanceArray   = lightData.data<FScene::LIGHT_INSTANCE>();
    auto      * UTILS_RESTRICT visibleArray    = lightData.data<FScene::VISIBILITY>();

    Culler::intersects(visibleArray, frustum, sphereArray, lightData.size());

    const float4* const UTILS_RESTRICT planes = frustum.getNormalizedPlanes();
    // the directional light is considered visible
    size_t visibleLightCount = FScene::DIRECTIONAL_LIGHTS_COUNT;
    // skip directional light
    for (size_t i = FScene::DIRECTIONAL_LIGHTS_COUNT; i < lightData.size(); i++) {
        FLightManager::Instance li = instanceArray[i];
        if (visibleArray[i]) {
            if (!lcm.isLightCaster(li)) {
                visibleArray[i] = 0;
                continue;
            }
            if (lcm.getIntensity(li) <= 0.0f) {
                visibleArray[i] = 0;
                continue;
            }
            // cull spotlights that cannot possibly intersect the view frustum
            if (lcm.isSpotLight(li)) {
                const float3 position = sphereArray[i].xyz;
                const float3 axis = directions[i];
                const float cosSqr = lcm.getCosOuterSquared(li);
                bool invisible = false;
                for (size_t j = 0; j < 6; ++j) {
                    const float p = dot(position + planes[j].xyz * planes[j].w, planes[j].xyz);
                    const float c = dot(planes[j].xyz, axis);
                    invisible |= ((1.0f - c * c) < cosSqr && c > 0 && p > 0);
                }
                if (invisible) {
                    visibleArray[i] = 0;
                    continue;
                }
            }
            visibleLightCount++;
        }
    }

    // Partition array such that all visible lights appear first
    UTILS_UNUSED_IN_RELEASE auto last =
            std::partition(lightData.begin() + FScene::DIRECTIONAL_LIGHTS_COUNT, lightData.end(),
                    [](auto const& it) {
                        return it.template get<FScene::VISIBILITY>() != 0;
                    });
    assert(visibleLightCount == size_t(last - lightData.begin()));

    lightData.resize(visibleLightCount);
}

void FView::updatePrimitivesLod(FEngine& engine, const CameraInfo&,
        FScene::RenderableSoa& renderableData, Range visible) noexcept {
    FRenderableManager const& rcm = engine.getRenderableManager();
    for (uint32_t index : visible) {
        uint8_t level = 0; // TODO: pick the proper level of detail
        auto ri = renderableData.elementAt<FScene::RENDERABLE_INSTANCE>(index);
        renderableData.elementAt<FScene::PRIMITIVES>(index) = rcm.getRenderPrimitives(ri, level);
    }
}

void FView::renderShadowMaps(FrameGraph& fg, FEngine& engine, FEngine::DriverApi& driver,
        RenderPass& pass) noexcept {
    mShadowMapManager.render(fg, engine, *this, driver, pass);
}

void FView::commitFrameHistory(FEngine& engine) noexcept {
    // Here we need to destroy resources in mFrameHistory.back()
    auto& frameHistory = mFrameHistory;
    FrameHistoryEntry& last = frameHistory.back();
    last.color.destroy(engine.getResourceAllocator());

    // and then push the new history entry to the history stack
    frameHistory.commit();
}

void FView::drainFrameHistory(FEngine& engine) noexcept {
    // make sure we free all resources in the history
    for (size_t i = 0; i < mFrameHistory.size(); ++i) {
        commitFrameHistory(engine);
    }
}

// ------------------------------------------------------------------------------------------------
// Trampoline calling into private implementation
// ------------------------------------------------------------------------------------------------

void View::setScene(Scene* scene) {
    return upcast(this)->setScene(upcast(scene));
}

Scene* View::getScene() noexcept {
    return upcast(this)->getScene();
}


void View::setCamera(Camera* camera) noexcept {
    upcast(this)->setCameraUser(upcast(camera));
}

Camera& View::getCamera() noexcept {
    return upcast(this)->getCameraUser();
}

void View::setViewport(filament::Viewport const& viewport) noexcept {
    upcast(this)->setViewport(viewport);
}

filament::Viewport const& View::getViewport() const noexcept {
    return upcast(this)->getViewport();
}

void View::setFrustumCullingEnabled(bool culling) noexcept {
    upcast(this)->setFrustumCullingEnabled(culling);
}

bool View::isFrustumCullingEnabled() const noexcept {
    return upcast(this)->isFrustumCullingEnabled();
}

void View::setDebugCamera(Camera* camera) noexcept {
    upcast(this)->setViewingCamera(upcast(camera));
}

void View::setVisibleLayers(uint8_t select, uint8_t values) noexcept {
    upcast(this)->setVisibleLayers(select, values);
}

void View::setName(const char* name) noexcept {
    upcast(this)->setName(name);
}

const char* View::getName() const noexcept {
    return upcast(this)->getName();
}

Camera const* View::getDirectionalLightCamera() const noexcept {
    return upcast(this)->getDirectionalLightCamera();
}

void View::setShadowingEnabled(bool enabled) noexcept {
    upcast(this)->setShadowingEnabled(enabled);
}

void View::setRenderTarget(RenderTarget* renderTarget) noexcept {
    upcast(this)->setRenderTarget(upcast(renderTarget));
}

RenderTarget* View::getRenderTarget() const noexcept {
    return upcast(this)->getRenderTarget();
}

void View::setSampleCount(uint8_t count) noexcept {
    upcast(this)->setSampleCount(count);
}

uint8_t View::getSampleCount() const noexcept {
    return upcast(this)->getSampleCount();
}

void View::setAntiAliasing(AntiAliasing type) noexcept {
    upcast(this)->setAntiAliasing(type);
}

View::AntiAliasing View::getAntiAliasing() const noexcept {
    return upcast(this)->getAntiAliasing();
}

void View::setTemporalAntiAliasingOptions(TemporalAntiAliasingOptions options) noexcept {
    upcast(this)->setTemporalAntiAliasingOptions(options);
}

const View::TemporalAntiAliasingOptions& View::getTemporalAntiAliasingOptions() const noexcept {
    return upcast(this)->getTemporalAntiAliasingOptions();
}

void View::setToneMapping(ToneMapping type) noexcept {
    upcast(this)->setToneMapping(type);
}

View::ToneMapping View::getToneMapping() const noexcept {
    return upcast(this)->getToneMapping();
}

void View::setColorGrading(ColorGrading* colorGrading) noexcept {
    return upcast(this)->setColorGrading(upcast(colorGrading));
}

const ColorGrading* View::getColorGrading() const noexcept {
    return upcast(this)->getColorGrading();
}

void View::setDithering(Dithering dithering) noexcept {
    upcast(this)->setDithering(dithering);
}

View::Dithering View::getDithering() const noexcept {
    return upcast(this)->getDithering();
}

void View::setDynamicResolutionOptions(const DynamicResolutionOptions& options) noexcept {
    upcast(this)->setDynamicResolutionOptions(options);
}

View::DynamicResolutionOptions View::getDynamicResolutionOptions() const noexcept {
    return upcast(this)->getDynamicResolutionOptions();
}

void View::setRenderQuality(const RenderQuality& renderQuality) noexcept {
    upcast(this)->setRenderQuality(renderQuality);
}

View::RenderQuality View::getRenderQuality() const noexcept {
    return upcast(this)->getRenderQuality();
}

void View::setPostProcessingEnabled(bool enabled) noexcept {
    upcast(this)->setPostProcessingEnabled(enabled);
}

bool View::isPostProcessingEnabled() const noexcept {
    return upcast(this)->hasPostProcessPass();
}

void View::setFrontFaceWindingInverted(bool inverted) noexcept {
    upcast(this)->setFrontFaceWindingInverted(inverted);
}

bool View::isFrontFaceWindingInverted() const noexcept {
    return upcast(this)->isFrontFaceWindingInverted();
}

void View::setDynamicLightingOptions(float zLightNear, float zLightFar) noexcept {
    upcast(this)->setDynamicLightingOptions(zLightNear, zLightFar);
}

void View::setAmbientOcclusion(View::AmbientOcclusion ambientOcclusion) noexcept {
    upcast(this)->setAmbientOcclusion(ambientOcclusion);
}

View::AmbientOcclusion View::getAmbientOcclusion() const noexcept {
    return upcast(this)->getAmbientOcclusion();
}

void View::setAmbientOcclusionOptions(View::AmbientOcclusionOptions const& options) noexcept {
    upcast(this)->setAmbientOcclusionOptions(options);
}

void View::setShadowType(View::ShadowType shadow) noexcept {
    upcast(this)->setShadowType(shadow);
}

View::AmbientOcclusionOptions const& View::getAmbientOcclusionOptions() const noexcept {
    return upcast(this)->getAmbientOcclusionOptions();
}

void View::setBloomOptions(View::BloomOptions options) noexcept {
    upcast(this)->setBloomOptions(options);
}

View::BloomOptions View::getBloomOptions() const noexcept {
    return upcast(this)->getBloomOptions();
}

void View::setFogOptions(View::FogOptions options) noexcept {
    upcast(this)->setFogOptions(options);
}

View::FogOptions View::getFogOptions() const noexcept {
    return upcast(this)->getFogOptions();
}

void View::setDepthOfFieldOptions(DepthOfFieldOptions options) noexcept {
    upcast(this)->setDepthOfFieldOptions(options);
}

View::DepthOfFieldOptions View::getDepthOfFieldOptions() const noexcept {
    return upcast(this)->getDepthOfFieldOptions();
}

void View::setVignetteOptions(View::VignetteOptions options) noexcept {
    upcast(this)->setVignetteOptions(options);
}

View::VignetteOptions View::getVignetteOptions() const noexcept {
    return upcast(this)->getVignetteOptions();
}

void View::setBlendMode(BlendMode blendMode) noexcept {
    upcast(this)->setBlendMode(blendMode);
}

View::BlendMode View::getBlendMode() const noexcept {
    return upcast(this)->getBlendMode();
}

uint8_t View::getVisibleLayers() const noexcept {
  return upcast(this)->getVisibleLayers();
}

bool View::isShadowingEnabled() const noexcept {
    return upcast(this)->isShadowingEnabled();
}

void View::setScreenSpaceRefractionEnabled(bool enabled) noexcept {
    upcast(this)->setScreenSpaceRefractionEnabled(enabled);
}

bool View::isScreenSpaceRefractionEnabled() const noexcept {
    return upcast(this)->isScreenSpaceRefractionEnabled();
}

} // namespace filament
