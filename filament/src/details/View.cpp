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

#include "Culler.h"
#include "FrameHistory.h"
#include "Froxelizer.h"
#include "RenderPrimitive.h"
#include "ResourceAllocator.h"
#include "ShadowMapManager.h"

#include "details/Engine.h"
#include "details/IndirectLight.h"
#include "details/RenderTarget.h"
#include "details/Renderer.h"
#include "details/Scene.h"
#include "details/Skybox.h"

#include <filament/Exposure.h>
#include <filament/DebugRegistry.h>
#include <filament/TextureSampler.h>
#include <filament/View.h>

#include <private/filament/UibStructs.h>
#include <private/filament/EngineEnums.h>

#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/Profiler.h>
#include <utils/Slice.h>
#include <utils/Systrace.h>
#include <utils/Zip2Iterator.h>

#include <math/scalar.h>
#include <math/fast.h>

#include <array>
#include <memory>
#include <tuple>

using namespace utils;

namespace filament {

using namespace backend;
using namespace math;

static constexpr float PID_CONTROLLER_Ki = 0.002f;
static constexpr float PID_CONTROLLER_Kd = 0.0f;

FView::FView(FEngine& engine)
        : mCommonRenderableDescriptorSet(engine.getPerRenderableDescriptorSetLayout()),
          mFroxelizer(engine),
          mFogEntity(engine.getEntityManager().create()),
          mIsStereoSupported(engine.getDriverApi().isStereoSupported()),
          mUniforms(engine.getDriverApi()),
          mColorPassDescriptorSet(engine, mUniforms) {

    DriverApi& driver = engine.getDriverApi();

    FDebugRegistry& debugRegistry = engine.getDebugRegistry();

    debugRegistry.registerProperty("d.view.camera_at_origin",
            &engine.debug.view.camera_at_origin);

    // The integral term is used to fight back the dead-band below, we limit how much it can act.
    mPidController.setIntegralLimits(-100.0f, 100.0f);

    // Dead-band, 1% for scaling down, 5% for scaling up. This stabilizes all the jitters.
    mPidController.setOutputDeadBand(-0.01f, 0.05f);

#ifndef NDEBUG
    // This can fail if another view has already registered this data source
    mDebugState->owner = debugRegistry.registerDataSource("d.view.frame_info",
            [weak = std::weak_ptr<DebugState>(mDebugState)]() -> DebugRegistry::DataSource {
                // the View could have been destroyed by the time we do this
                auto const state = weak.lock();
                if (!state) {
                    return { nullptr, 0 };
                }
                // Lazily allocate the buffer for the debug data source, and mark this
                // data source as active. It can never go back to inactive.
                assert_invariant(!state->debugFrameHistory);
                state->active = true;
                state->debugFrameHistory =
                        std::make_unique<std::array<DebugRegistry::FrameHistory, 5 * 60>>();
                return { state->debugFrameHistory->data(), state->debugFrameHistory->size() };
            });

    if (UTILS_UNLIKELY(mDebugState->owner)) {
        // publish the properties (they will be initialized in the main loop)
        debugRegistry.registerProperty("d.view.pid.kp", &engine.debug.view.pid.kp);
        debugRegistry.registerProperty("d.view.pid.ki", &engine.debug.view.pid.ki);
        debugRegistry.registerProperty("d.view.pid.kd", &engine.debug.view.pid.kd);
    }
#endif

    // allocate UBOs
    mLightUbh = driver.createBufferObject(CONFIG_MAX_LIGHT_COUNT * sizeof(LightsUib),
            BufferObjectBinding::UNIFORM, BufferUsage::DYNAMIC);

    mIsDynamicResolutionSupported = driver.isFrameTimeSupported();

    mDefaultColorGrading = mColorGrading = engine.getDefaultColorGrading();

    mColorPassDescriptorSet.init(
            mLightUbh,
            mFroxelizer.getRecordBuffer(),
            mFroxelizer.getFroxelBuffer());
}

FView::~FView() noexcept = default;

void FView::terminate(FEngine& engine) {
    // Here we would cleanly free resources we've allocated, or we own (currently none).

    while (mActivePickingQueriesList) {
        FPickingQuery* const pQuery = mActivePickingQueriesList;
        mActivePickingQueriesList = pQuery->next;
        pQuery->callback(pQuery->result, pQuery);
        FPickingQuery::put(pQuery);
    }

    DriverApi& driver = engine.getDriverApi();
    driver.destroyBufferObject(mLightUbh);
    driver.destroyBufferObject(mRenderableUbh);
    clearFrameHistory(engine);

    ShadowMapManager::terminate(engine, mShadowMapManager);
    mUniforms.terminate(driver);
    mColorPassDescriptorSet.terminate(engine.getDescriptorSetLayoutFactory(), driver);
    mFroxelizer.terminate(driver);
    mCommonRenderableDescriptorSet.terminate(driver);

    engine.getEntityManager().destroy(mFogEntity);

#ifndef NDEBUG
    if (UTILS_UNLIKELY(mDebugState->owner)) {
        engine.getDebugRegistry().unregisterDataSource("d.view.frame_info");
    }
#endif
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

        dynamicResolution.sharpness = clamp(dynamicResolution.sharpness, 0.0f, 2.0f);
    }
}

void FView::setDynamicLightingOptions(float zLightNear, float zLightFar) noexcept {
    mFroxelizer.setOptions(zLightNear, zLightFar);
}

float2 FView::updateScale(FEngine& engine,
        filament::details::FrameInfo const& info,
        Renderer::FrameRateOptions const& frameRateOptions,
        Renderer::DisplayInfo const& displayInfo) noexcept {

#ifndef NDEBUG
    if (UTILS_LIKELY(!mDebugState->active)) {
        // if we're not active, update the debug properties with the normal values
        // and use that for configuring the PID controller.
        engine.debug.view.pid.kp = 1.0f - std::exp(-frameRateOptions.scaleRate);
        engine.debug.view.pid.ki = PID_CONTROLLER_Ki;
        engine.debug.view.pid.kd = PID_CONTROLLER_Kd;
    }
#endif

    DynamicResolutionOptions const& options = mDynamicResolution;
    if (options.enabled) {
        if (!UTILS_UNLIKELY(info.valid)) {
            // always clamp to the min/max scale range
            mScale = clamp(1.0f, options.minScale, options.maxScale);
            return mScale;
        }

#ifndef NDEBUG
        const float Kp = engine.debug.view.pid.kp;
        const float Ki = engine.debug.view.pid.ki;
        const float Kd = engine.debug.view.pid.kd;
#else
        const float Kp = (1.0f - std::exp(-frameRateOptions.scaleRate));
        const float Ki = PID_CONTROLLER_Ki;
        const float Kd = PID_CONTROLLER_Kd;
#endif
        mPidController.setParallelGains(Kp, Ki, Kd);

        // all values in ms below
        using std::chrono::duration;
        const float dt = 1.0f; // we don't really need dt here, setting it to 1, means our parameters are in "frames"
        const float target = (1000.0f * float(frameRateOptions.interval)) / displayInfo.refreshRate;
        const float targetWithHeadroom = target * (1.0f - frameRateOptions.headRoomRatio);
        float const measured = duration<float, std::milli>{ info.denoisedFrameTime }.count();
        float const out = mPidController.update(measured / targetWithHeadroom, 1.0f, dt);

        // maps pid command to a scale (absolute or relative, see below)
         const float command = out < 0.0f ? (1.0f / (1.0f - out)) : (1.0f + out);

        /*
         * There is two ways we can control the scale factor, either by having the PID controller
         * output a new scale factor directly (like a "position" control), or having it evaluate
         * a relative scale factor (like a "velocity" control).
         * More experimentation is needed to figure out which works better in more cases.
         */

        // direct scaling ("position" control)
        //const float scale = command;
        // relative scaling ("velocity" control)
        const float scale = mScale.x * mScale.y * command;

        const float w = float(mViewport.width);
        const float h = float(mViewport.height);
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

            // finally, write the scale factors
            float& majorRef = w > h ? mScale.x : mScale.y;
            float& minorRef = w > h ? mScale.y : mScale.x;
            majorRef = std::sqrt(homogeneousScale) * majorScale;
            minorRef = std::sqrt(homogeneousScale) * minorScale;
        } else {
            // when scaling up, we're always using homogeneous scaling.
            mScale = std::sqrt(scale);
        }

        // always clamp to the min/max scale range
        const auto s = mScale;
        mScale = clamp(s, options.minScale, options.maxScale);

        // disable the integration term when we're outside the controllable range
        // (i.e. we clamped). This help not to have to wait too long for the Integral term
        // to kick in after a clamping event.
        mPidController.setIntegralInhibitionEnabled(mScale != s);
    } else {
        mScale = 1.0f;
    }

#ifndef NDEBUG
    // only for debugging...
    if (UTILS_UNLIKELY(mDebugState->active && mDebugState->debugFrameHistory)) {
        auto* const debugFrameHistory = mDebugState->debugFrameHistory.get();
        using namespace std::chrono;
        using duration_ms = duration<float, std::milli>;
        const float target = (1000.0f * float(frameRateOptions.interval)) / displayInfo.refreshRate;
        const float targetWithHeadroom = target * (1.0f - frameRateOptions.headRoomRatio);
        std::move(debugFrameHistory->begin() + 1,
                debugFrameHistory->end(), debugFrameHistory->begin());
        debugFrameHistory->back() = {
                .target             = target,
                .targetWithHeadroom = targetWithHeadroom,
                .frameTime          = duration_cast<duration_ms>(info.frameTime).count(),
                .frameTimeDenoised  = duration_cast<duration_ms>(info.denoisedFrameTime).count(),
                .scale              = mScale.x * mScale.y,
                .pid_e              = mPidController.getError(),
                .pid_i              = mPidController.getIntegral(),
                .pid_d              = mPidController.getDerivative()
        };
    }
#endif

    return mScale;
}

void FView::setVisibleLayers(uint8_t select, uint8_t values) noexcept {
    mVisibleLayers = (mVisibleLayers & ~select) | (values & select);
}

bool FView::isSkyboxVisible() const noexcept {
    FSkybox const* skybox = mScene ? mScene->getSkybox() : nullptr;
    return skybox != nullptr && (skybox->getLayerMask() & mVisibleLayers);
}

void FView::prepareShadowing(FEngine& engine, FScene::RenderableSoa& renderableData,
        FScene::LightSoa const& lightData, CameraInfo const& cameraInfo) noexcept {
    SYSTRACE_CALL();

    mHasShadowing = false;
    mNeedsShadowMap = false;
    if (!mShadowingEnabled) {
        return;
    }

    auto& lcm = engine.getLightManager();

    ShadowMapManager::Builder builder;

    // dominant directional light is always as index 0
    FLightManager::Instance const directionalLight = lightData.elementAt<FScene::LIGHT_INSTANCE>(0);
    const bool hasDirectionalShadows = directionalLight && lcm.isShadowCaster(directionalLight);
    if (UTILS_UNLIKELY(hasDirectionalShadows)) {
        const auto& shadowOptions = lcm.getShadowOptions(directionalLight);
        assert_invariant(shadowOptions.shadowCascades >= 1 &&
                shadowOptions.shadowCascades <= CONFIG_MAX_SHADOW_CASCADES);
        builder.directionalShadowMap(0, &shadowOptions);
    }

    // Find all shadow-casting spotlights.
    size_t shadowMapCount = CONFIG_MAX_SHADOW_CASCADES;

    // We allow a max of CONFIG_MAX_SHADOWMAPS point/spotlight shadows. Any additional
    // shadow-casting spotlights are ignored.
    // Note that pointlight shadows cost 6 shadowmaps, reducing the total count.
    for (size_t l = FScene::DIRECTIONAL_LIGHTS_COUNT; l < lightData.size(); l++) {

        // when we get here all the lights should be visible
        assert_invariant(lightData.elementAt<FScene::VISIBILITY>(l));

        FLightManager::Instance const li = lightData.elementAt<FScene::LIGHT_INSTANCE>(l);

        if (UTILS_LIKELY(!li)) {
            continue; // invalid instance
        }

        if (UTILS_LIKELY(!lcm.isShadowCaster(li))) {
            // Because we early exit here, we need to make sure we mark the light as non-casting.
            // See `ShadowMapManager::updateSpotShadowMaps` for const_cast<> justification.
            auto& shadowInfo = const_cast<FScene::ShadowInfo&>(
                lightData.elementAt<FScene::SHADOW_INFO>(l));
            shadowInfo.castsShadows = false;
            continue; // doesn't cast shadows
        }

        const bool spotLight = lcm.isSpotLight(li);

        const size_t maxShadowMapCount = engine.getMaxShadowMapCount();
        const size_t shadowMapCountNeeded = spotLight ? 1 : 6;
        if (shadowMapCount + shadowMapCountNeeded <= maxShadowMapCount) {
            shadowMapCount += shadowMapCountNeeded;
            const auto& shadowOptions = lcm.getShadowOptions(li);
            builder.shadowMap(l, spotLight, &shadowOptions);
        }

        if (shadowMapCount >= maxShadowMapCount) {
            break; // we ran out of spotlight shadow casting
        }
    }

    if (builder.hasShadowMaps()) {
        ShadowMapManager::createIfNeeded(engine, mShadowMapManager);
        auto shadowTechnique = mShadowMapManager->update(builder, engine, *this,
                cameraInfo, renderableData, lightData);

        mHasShadowing = any(shadowTechnique);
        mNeedsShadowMap = any(shadowTechnique & ShadowMapManager::ShadowTechnique::SHADOW_MAP);
    }
}

void FView::prepareLighting(FEngine& engine, CameraInfo const& cameraInfo) noexcept {
    SYSTRACE_CALL();
    SYSTRACE_CONTEXT();

    FScene* const scene = mScene;
    auto const& lightData = scene->getLightData();

    /*
     * Dynamic lights
     */

    if (hasDynamicLighting()) {
        scene->prepareDynamicLights(cameraInfo, mLightUbh);
    }

    // here the array of visible lights has been shrunk to CONFIG_MAX_LIGHT_COUNT
    SYSTRACE_VALUE32("visibleLights", lightData.size() - FScene::DIRECTIONAL_LIGHTS_COUNT);

    /*
     * Exposure
     */

    const float exposure = Exposure::exposure(cameraInfo.ev100);
    mColorPassDescriptorSet.prepareExposure(cameraInfo.ev100);

    /*
     * Indirect light (IBL)
     */

    // If the scene does not have an IBL, use the black 1x1 IBL and honor the fallback intensity
    // associated with the skybox.
    float intensity;
    FIndirectLight const* ibl = scene->getIndirectLight();
    if (UTILS_LIKELY(ibl)) {
        intensity = ibl->getIntensity();
    } else {
        ibl = engine.getDefaultIndirectLight();
        FSkybox const* const skybox = scene->getSkybox();
        intensity = skybox ? skybox->getIntensity() : FIndirectLight::DEFAULT_INTENSITY;
    }
    mColorPassDescriptorSet.prepareAmbientLight(engine, *ibl, intensity, exposure);

    /*
     * Directional light (always at index 0)
     */

    FLightManager::Instance const directionalLight = lightData.elementAt<FScene::LIGHT_INSTANCE>(0);
    const float3 sceneSpaceDirection = lightData.elementAt<FScene::DIRECTION>(0); // guaranteed normalized
    mColorPassDescriptorSet.prepareDirectionalLight(engine, exposure, sceneSpaceDirection, directionalLight);
}

CameraInfo FView::computeCameraInfo(FEngine& engine) const noexcept {
    FScene const* const scene = getScene();

    /*
     * We apply a "world origin" to "everything" in order to implement the IBL rotation.
     * The "world origin" is also used to keep the origin close to the camera position to
     * improve fp precision in the shader for large scenes.
     */
    double3 translation;
    mat3 rotation;

    /*
     * Calculate all camera parameters needed to render this View for this frame.
     */
    FCamera const* const camera = mViewingCamera ? mViewingCamera : mCullingCamera;
    if (engine.debug.view.camera_at_origin) {
        // this moves the camera to the origin, effectively doing all shader computations in
        // view-space, which improves floating point precision in the shader by staying around
        // zero, where fp precision is highest. This also ensures that when the camera is placed
        // very far from the origin, objects are still rendered and lit properly.
        translation = -camera->getPosition();
    }

    FIndirectLight const* const ibl = scene->getIndirectLight();
    if (ibl) {
        // the IBL transformation must be a rigid transform
        rotation = mat3{ transpose(scene->getIndirectLight()->getRotation()) };
        // it is important to orthogonalize the matrix when converting it to doubles, because
        // as float, it only has about a 1e-8 precision on the size of the basis vectors
        rotation = orthogonalize(rotation);
    }
    return { *camera, mat4{ rotation } * mat4::translation(translation) };
}

void FView::prepare(FEngine& engine, DriverApi& driver, RootArenaScope& rootArenaScope,
        filament::Viewport viewport, CameraInfo cameraInfo,
        float4 const& userTime, bool needsAlphaChannel) noexcept {

        SYSTRACE_CALL();
        SYSTRACE_CONTEXT();

    JobSystem& js = engine.getJobSystem();

    /*
     * Prepare the scene -- this is where we gather all the objects added to the scene,
     * and in particular their world-space AABB.
     */

    auto getFrustum = [this, &cameraInfo]() -> Frustum {
        if (UTILS_LIKELY(mViewingCamera == nullptr)) {
            // In the common case when we don't have a viewing camera, cameraInfo.view is
            // already the culling view matrix
            return Frustum{ mat4f{ highPrecisionMultiply(cameraInfo.cullingProjection, cameraInfo.view) }};
        } else {
            // Otherwise, we need to recalculate it from the culling camera.
            // Note: it is correct to always do the math from mCullingCamera, but it hides the
            // intent of the code, which is that we should only depend on CameraInfo here.
            // This is an extremely uncommon case.
            const mat4 projection = mCullingCamera->getCullingProjectionMatrix();
            const mat4 view = inverse(cameraInfo.worldTransform * mCullingCamera->getModelMatrix());
            return Frustum{ mat4f{ projection * view }};
        }
    };

    const Frustum cullingFrustum = getFrustum();

    FScene* const scene = getScene();

    /*
     * Gather all information needed to render this scene. Apply the world origin to all
     * objects in the scene.
     */
    scene->prepare(js, rootArenaScope,
            cameraInfo.worldTransform,
            hasVSM());

    /*
     * Light culling: runs in parallel with Renderable culling (below)
     */

    JobSystem::Job* froxelizeLightsJob = nullptr;
    JobSystem::Job* prepareVisibleLightsJob = nullptr;
    size_t const lightCount = scene->getLightData().size();
    if (lightCount > FScene::DIRECTIONAL_LIGHTS_COUNT) {
        // create and start the prepareVisibleLights job
        // note: this job updates LightData (non const)
        // allocate a scratch buffer for distances outside the job below, so we don't need
        // to use a locked allocator; the downside is that we need to account for the worst case.
        size_t const positionalLightCount = lightCount - FScene::DIRECTIONAL_LIGHTS_COUNT;
        float* const distances = rootArenaScope.allocate<float>(
                (positionalLightCount + 3u) & ~3u, CACHELINE_SIZE);

        prepareVisibleLightsJob = js.runAndRetain(js.createJob(nullptr,
                [&engine, distances, positionalLightCount, &viewMatrix = cameraInfo.view, &cullingFrustum,
                 &lightData = scene->getLightData()]
                        (JobSystem&, JobSystem::Job*) {
                    prepareVisibleLights(engine.getLightManager(),
                            { distances, distances + positionalLightCount },
                            viewMatrix, cullingFrustum, lightData);
                }));
    }

    // this is used later (in Renderer.cpp) to wait for froxelization to finishes
    setFroxelizerSync(froxelizeLightsJob);

    Range merged;
    FScene::RenderableSoa& renderableData = scene->getRenderableData();

    { // all the operations in this scope must happen sequentially

        Slice<Culler::result_type> cullingMask = renderableData.slice<FScene::VISIBLE_MASK>();
        std::uninitialized_fill(cullingMask.begin(), cullingMask.end(), 0);

        /*
         * Culling: as soon as possible we perform our camera-culling
         * (this will set the VISIBLE_RENDERABLE bit)
         */

        prepareVisibleRenderables(js, cullingFrustum, renderableData);


        /*
         * Shadowing: compute the shadow camera and cull shadow casters
         * (this will set the VISIBLE_DIR_SHADOW_CASTER bit and VISIBLE_SPOT_SHADOW_CASTER bits)
         */

        // prepareShadowing relies on prepareVisibleLights().
        if (prepareVisibleLightsJob) {
            js.waitAndRelease(prepareVisibleLightsJob);
        }

        // lightData is const from this point on (can only happen after prepareVisibleLightsJob)
        auto const& lightData = scene->getLightData();

        // now we know if we have dynamic lighting (i.e.: dynamic lights are visible)
        mHasDynamicLighting = lightData.size() > FScene::DIRECTIONAL_LIGHTS_COUNT;

        // we also know if we have a directional light
        FLightManager::Instance const directionalLight =
                lightData.elementAt<FScene::LIGHT_INSTANCE>(0);
        mHasDirectionalLighting = directionalLight.isValid();

        // As soon as prepareVisibleLight finishes, we can kick-off the froxelization
        if (hasDynamicLighting()) {
            auto& froxelizer = mFroxelizer;
            if (froxelizer.prepare(driver, rootArenaScope, viewport,
                    cameraInfo.projection, cameraInfo.zn, cameraInfo.zf)) {
                // TODO: might be more consistent to do this in prepareLighting(), but it's not
                //       strictly necessary
                mColorPassDescriptorSet.prepareDynamicLights(mFroxelizer);
            }
            // We need to pass viewMatrix by value here because it extends the scope of this
            // function.
            std::function<void(JobSystem&, JobSystem::Job*)> froxelizerWork =
                    [&froxelizer = mFroxelizer, &engine, viewMatrix = cameraInfo.view, &lightData]
                            (JobSystem&, JobSystem::Job*) {
                        froxelizer.froxelizeLights(engine, viewMatrix, lightData);
                    };
            froxelizeLightsJob = js.runAndRetain(js.createJob(nullptr, std::move(froxelizerWork)));
        }

        setFroxelizerSync(froxelizeLightsJob);

        prepareShadowing(engine, renderableData, lightData, cameraInfo);

        /*
         * Partition the SoA so that renderables are partitioned w.r.t their visibility into the
         * following groups:
         *
         * 1. visible (main camera) renderables
         * 2. visible (main camera) renderables and directional shadow casters
         * 3. directional shadow casters only
         * 4. potential punctual light shadow casters only
         * 5. definitely invisible renderables
         *
         * Note that the first three groups are partitioned based only on the lowest two bits of the
         * VISIBLE_MASK (VISIBLE_RENDERABLE and VISIBLE_DIR_SHADOW_CASTER), and thus can also
         * contain punctual light shadow casters as well. The fourth group contains *only* punctual
         * shadow casters.
         *
         * This operation is somewhat heavy as it sorts the whole SoA. We use std::partition instead
         * of sort(), which gives us O(4.N) instead of O(N.log(N)) application of swap().
         */

        // TODO: we need to compare performance of doing this partitioning vs not doing it.
        //       and rely on checking visibility in the loops

        SYSTRACE_NAME_BEGIN("Partitioning");

        // calculate the sorting key for all elements, based on their visibility
        uint8_t const* layers = renderableData.data<FScene::LAYERS>();
        auto const* visibility = renderableData.data<FScene::VISIBILITY_STATE>();
        computeVisibilityMasks(getVisibleLayers(), layers, visibility, cullingMask.begin(),
                renderableData.size());

        auto const beginRenderables = renderableData.begin();

        auto beginDirCasters = partition(beginRenderables, renderableData.end(),
                VISIBLE_RENDERABLE | VISIBLE_DIR_SHADOW_RENDERABLE,
                VISIBLE_RENDERABLE);

        auto beginDirCastersOnly = partition(beginDirCasters, renderableData.end(),
                VISIBLE_RENDERABLE | VISIBLE_DIR_SHADOW_RENDERABLE,
                VISIBLE_RENDERABLE | VISIBLE_DIR_SHADOW_RENDERABLE);

        auto endDirCastersOnly = partition(beginDirCastersOnly, renderableData.end(),
                VISIBLE_RENDERABLE | VISIBLE_DIR_SHADOW_RENDERABLE,
                VISIBLE_DIR_SHADOW_RENDERABLE);

        auto endPotentialSpotCastersOnly = partition(endDirCastersOnly, renderableData.end(),
                VISIBLE_DYN_SHADOW_RENDERABLE,
                VISIBLE_DYN_SHADOW_RENDERABLE);

        // convert to indices
        mVisibleRenderables = { 0, uint32_t(beginDirCastersOnly - beginRenderables) };

        mVisibleDirectionalShadowCasters = {
                uint32_t(beginDirCasters - beginRenderables),
                uint32_t(endDirCastersOnly - beginRenderables)};

        merged = { 0, uint32_t(endPotentialSpotCastersOnly - beginRenderables) };
        if (!needsShadowMap() || !mShadowMapManager->hasSpotShadows()) {
            // we know we don't have spot shadows, we can reduce the range to not even include
            // the potential spot casters
            merged = { 0, uint32_t(endDirCastersOnly - beginRenderables) };
        }

        mSpotLightShadowCasters = merged;

        SYSTRACE_NAME_END();

        // TODO: when any spotlight is used, `merged` ends-up being the whole list. However,
        //       some of the items will end-up not being visible by any light. Can we do better?
        //       e.g. could we deffer some of the prepareVisibleRenderables() to later?
        scene->prepareVisibleRenderables(merged);

        // update those UBOs
        const size_t size = merged.size() * sizeof(PerRenderableData);
        if (size) {
            if (mRenderableUBOSize < size) {
                // allocate 1/3 extra, with a minimum of 16 objects
                const size_t count = std::max(size_t(16u), (4u * merged.size() + 2u) / 3u);
                mRenderableUBOSize = uint32_t(count * sizeof(PerRenderableData));
                driver.destroyBufferObject(mRenderableUbh);
                mRenderableUbh = driver.createBufferObject(
                        mRenderableUBOSize + sizeof(PerRenderableUib),
                        BufferObjectBinding::UNIFORM, BufferUsage::DYNAMIC);
            } else {
                // TODO: should we shrink the underlying UBO at some point?
            }
            assert_invariant(mRenderableUbh);
            scene->updateUBOs(merged, mRenderableUbh);

            mCommonRenderableDescriptorSet.setBuffer(
                    +PerRenderableBindingPoints::OBJECT_UNIFORMS, mRenderableUbh,
                    0, sizeof(PerRenderableUib));

            mCommonRenderableDescriptorSet.commit(
                    engine.getPerRenderableDescriptorSetLayout(), driver);
        }
    }

    { // this must happen after mRenderableUbh is created/updated
        // prepare skinning, morphing and hybrid instancing
        auto& sceneData = scene->getRenderableData();
        for (uint32_t const i : merged) {
            auto const& skinning = sceneData.elementAt<FScene::SKINNING_BUFFER>(i);
            auto const& morphing = sceneData.elementAt<FScene::MORPHING_BUFFER>(i);
            auto const& instance = sceneData.elementAt<FScene::INSTANCES>(i);

            // FIXME: when only one is active the UBO handle of the other is null
            //        (probably a problem on vulkan)
            if (UTILS_UNLIKELY(skinning.handle || morphing.handle || instance.handle)) {
                auto const ci = sceneData.elementAt<FScene::RENDERABLE_INSTANCE>(i);
                FRenderableManager& rcm = engine.getRenderableManager();
                auto& descriptorSet = rcm.getDescriptorSet(ci);

                // initialize the descriptor set the first time it's needed
                if (UTILS_UNLIKELY(!descriptorSet.getHandle())) {
                    descriptorSet = DescriptorSet{ engine.getPerRenderableDescriptorSetLayout() };
                }

                descriptorSet.setBuffer(+PerRenderableBindingPoints::OBJECT_UNIFORMS,
                        instance.handle ? instance.handle : mRenderableUbh,
                        0, sizeof(PerRenderableUib));

                if (UTILS_UNLIKELY(skinning.handle || morphing.handle)) {

                    descriptorSet.setBuffer(+PerRenderableBindingPoints::BONES_UNIFORMS,
                            skinning.handle, 0, sizeof(PerRenderableBoneUib));

                    descriptorSet.setSampler(+PerRenderableBindingPoints::BONES_INDICES_AND_WEIGHTS,
                            skinning.boneIndicesAndWeightHandle, {});

                    descriptorSet.setBuffer(+PerRenderableBindingPoints::MORPHING_UNIFORMS,
                            morphing.handle, 0, sizeof(PerRenderableMorphingUib));

                    descriptorSet.setSampler(+PerRenderableBindingPoints::MORPH_TARGET_POSITIONS,
                            morphing.morphTargetBuffer->getPositionsHandle(), {});

                    descriptorSet.setSampler(+PerRenderableBindingPoints::MORPH_TARGET_TANGENTS,
                            morphing.morphTargetBuffer->getTangentsHandle(), {});
                }

                descriptorSet.commit(engine.getPerRenderableDescriptorSetLayout(), driver);

                // write the descriptor-set handle to the sceneData array for access later
                sceneData.elementAt<FScene::DESCRIPTOR_SET_HANDLE>(i) = descriptorSet.getHandle();
            } else {
                // use the shared descriptor-set
                sceneData.elementAt<FScene::DESCRIPTOR_SET_HANDLE>(i) =
                        mCommonRenderableDescriptorSet.getHandle();
            }
        }
    }

    /*
     * Prepare lighting -- this is where we update the lights UBOs, set up the IBL,
     * set up the froxelization parameters.
     * Relies on FScene::prepare() and prepareVisibleLights()
     */

    prepareLighting(engine, cameraInfo);

    /*
     * Update driver state
     */

    auto const& tcm = engine.getTransformManager();
    auto const fogTransform = tcm.getWorldTransformAccurate(tcm.getInstance(mFogEntity));

    mColorPassDescriptorSet.prepareTime(engine, userTime);
    mColorPassDescriptorSet.prepareFog(engine, cameraInfo, fogTransform, mFogOptions,
            scene->getIndirectLight());
    mColorPassDescriptorSet.prepareTemporalNoise(engine, mTemporalAntiAliasingOptions);
    mColorPassDescriptorSet.prepareBlending(needsAlphaChannel);
    mColorPassDescriptorSet.prepareMaterialGlobals(mMaterialGlobals);
}

void FView::computeVisibilityMasks(
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

        const bool visibleRenderable = inVisibleLayer &&
                (!v.culling || (mask & VISIBLE_RENDERABLE));

        const bool visibleDirectionalShadowRenderable = (v.castShadows && inVisibleLayer) &&
                (!v.culling || (mask & VISIBLE_DIR_SHADOW_RENDERABLE));

        const bool potentialSpotShadowRenderable = v.castShadows && inVisibleLayer;

        using Type = Culler::result_type;

        visibleMask[i] =
                Type(visibleRenderable << VISIBLE_RENDERABLE_BIT) |
                Type(visibleDirectionalShadowRenderable << VISIBLE_DIR_SHADOW_RENDERABLE_BIT) |
                Type(potentialSpotShadowRenderable << VISIBLE_DYN_SHADOW_RENDERABLE_BIT);
    }
}

UTILS_NOINLINE
/* static */ FScene::RenderableSoa::iterator FView::partition(
        FScene::RenderableSoa::iterator begin,
        FScene::RenderableSoa::iterator end,
        Culler::result_type mask, Culler::result_type value) noexcept {
    return std::partition(begin, end, [mask, value](auto it) {
        // Mask VISIBLE_MASK to ignore higher bits related to spot shadows. We only partition based
        // on renderable and directional shadow visibility.
        return (it.template get<FScene::VISIBLE_MASK>() & mask) == value;
    });
}

void FView::prepareUpscaler(float2 scale,
        TemporalAntiAliasingOptions const& taaOptions,
        DynamicResolutionOptions const& dsrOptions) const noexcept {
    SYSTRACE_CALL();
    float bias = 0.0f;
    float2 derivativesScale{ 1.0f };
    if (dsrOptions.enabled && dsrOptions.quality >= QualityLevel::HIGH) {
        bias = std::log2(std::min(scale.x, scale.y));
    }
    if (taaOptions.enabled) {
        bias += taaOptions.lodBias;
        if (taaOptions.upscaling) {
            derivativesScale = 0.5f;
        }
    }
    mColorPassDescriptorSet.prepareLodBias(bias, derivativesScale);
}

void FView::prepareCamera(FEngine& engine, const CameraInfo& cameraInfo) const noexcept {
    SYSTRACE_CALL();
    mColorPassDescriptorSet.prepareCamera(engine, cameraInfo);
}

void FView::prepareViewport(
        const filament::Viewport& physicalViewport,
        const filament::Viewport& logicalViewport) const noexcept {
    SYSTRACE_CALL();
    // TODO: we should pass viewport.{left|bottom} to the backend, so it can offset the
    //       scissor properly.
    mColorPassDescriptorSet.prepareViewport(physicalViewport, logicalViewport);
}

void FView::prepareSSAO(Handle<HwTexture> ssao) const noexcept {
    mColorPassDescriptorSet.prepareSSAO(ssao, mAmbientOcclusionOptions);
}

void FView::prepareSSR(Handle<HwTexture> ssr,
        bool disableSSR,
        float refractionLodOffset,
        ScreenSpaceReflectionsOptions const& ssrOptions) const noexcept {
    mColorPassDescriptorSet.prepareSSR(ssr, disableSSR, refractionLodOffset, ssrOptions);
}

void FView::prepareStructure(Handle<HwTexture> structure) const noexcept {
    // sampler must be NEAREST
    mColorPassDescriptorSet.prepareStructure(structure);
}

void FView::prepareShadow(Handle<HwTexture> texture) const noexcept {
    // when needsShadowMap() is not set, this method only just sets a dummy texture
    // in the needed samplers (in that case `texture` is actually a dummy texture).
    ShadowMapManager::ShadowMappingUniforms uniforms;
    if (needsShadowMap()) {
        uniforms = mShadowMapManager->getShadowMappingUniforms();
    }
    switch (mShadowType) {
        case ShadowType::PCF:
            mColorPassDescriptorSet.prepareShadowPCF(texture, uniforms);
            break;
        case ShadowType::VSM:
            mColorPassDescriptorSet.prepareShadowVSM(texture, uniforms, mVsmShadowOptions);
            break;
        case ShadowType::DPCF:
            mColorPassDescriptorSet.prepareShadowDPCF(texture, uniforms, mSoftShadowOptions);
            break;
        case ShadowType::PCSS:
            mColorPassDescriptorSet.prepareShadowPCSS(texture, uniforms, mSoftShadowOptions);
            break;
        case ShadowType::PCFd:
            mColorPassDescriptorSet.prepareShadowPCFDebug(texture, uniforms);
            break;
    }
}

void FView::prepareShadowMapping(bool highPrecision) const noexcept {
    if (mHasShadowing) {
        assert_invariant(mShadowMapManager);
        mColorPassDescriptorSet.prepareShadowMapping(
                mShadowMapManager->getShadowUniformsHandle(), highPrecision);
    }
}

void FView::commitUniformsAndSamplers(DriverApi& driver) const noexcept {
    mColorPassDescriptorSet.commit(driver);
}

void FView::unbindSamplers(DriverApi& driver) noexcept {
    mColorPassDescriptorSet.unbindSamplers(driver);
}

void FView::commitFroxels(DriverApi& driverApi) const noexcept {
    if (mHasDynamicLighting) {
        mFroxelizer.commit(driverApi);
    }
}

UTILS_NOINLINE
void FView::prepareVisibleRenderables(JobSystem& js,
        Frustum const& frustum, FScene::RenderableSoa& renderableData) const noexcept {
    SYSTRACE_CALL();
    if (UTILS_LIKELY(isFrustumCullingEnabled())) {
        cullRenderables(js, renderableData, frustum, VISIBLE_RENDERABLE_BIT);
    } else {
        std::uninitialized_fill(renderableData.begin<FScene::VISIBLE_MASK>(),
                  renderableData.end<FScene::VISIBLE_MASK>(), VISIBLE_RENDERABLE);
    }
}

void FView::cullRenderables(JobSystem&,
        FScene::RenderableSoa& renderableData, Frustum const& frustum, size_t bit) noexcept {
    SYSTRACE_CALL();

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

    // Note: we can't use jobs::parallel_for() here because Culler::intersects() must process
    //       multiples of eight primitives.
    // Moreover, even with a large number of primitives, the overhead of the JobSystem is too
    // large compared to the run time of Culler::intersects, e.g.: ~100us for 4000 primitives
    // on Pixel4.
    functor(0, renderableData.size());
}

void FView::prepareVisibleLights(FLightManager const& lcm,
        Slice<float> scratch,
        mat4f const& viewMatrix, Frustum const& frustum,
        FScene::LightSoa& lightData) noexcept {
    SYSTRACE_CALL();
    assert_invariant(lightData.size() > FScene::DIRECTIONAL_LIGHTS_COUNT);

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
        FLightManager::Instance const li = instanceArray[i];
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
    assert_invariant(visibleLightCount == size_t(last - lightData.begin()));


    /*
     * Some lights might be left out if there are more than the GPU buffer allows (i.e. 256).
     *
     * We always sort lights by distance to the camera so that:
     * - we can build light trees later
     * - lights farther from the camera are dropped when in excess
     *   Note this doesn't always work well, e.g. for search-lights, we might need to also
     *   take the radius into account.
     * - This helps our limited numbers of spot-shadow as well.
     */

    // number of point/spotlights
    size_t const positionalLightCount = visibleLightCount - FScene::DIRECTIONAL_LIGHTS_COUNT;
    if (positionalLightCount) {
        assert_invariant(positionalLightCount <= scratch.size());
        // pre-compute the lights' distance to the camera, for sorting below
        // - we don't skip the directional light, because we don't care, it's ignored during sorting
        float* const UTILS_RESTRICT distances = scratch.data();
        float4 const* const UTILS_RESTRICT spheres = lightData.data<FScene::POSITION_RADIUS>();
        computeLightCameraDistances(distances, viewMatrix, spheres, visibleLightCount);

        // skip directional light
        Zip2Iterator<FScene::LightSoa::iterator, float*> b = { lightData.begin(), distances };
        std::sort(b + FScene::DIRECTIONAL_LIGHTS_COUNT, b + visibleLightCount,
                [](auto const& lhs, auto const& rhs) { return lhs.second < rhs.second; });
    }

    // drop excess lights
    lightData.resize(std::min(visibleLightCount,
            CONFIG_MAX_LIGHT_COUNT + FScene::DIRECTIONAL_LIGHTS_COUNT));
}

// These methods need to exist so clang honors the __restrict__ keyword, which in turn
// produces much better vectorization. The ALWAYS_INLINE keyword makes sure we actually don't
// pay the price of the call!
UTILS_ALWAYS_INLINE
inline void FView::computeLightCameraDistances(
        float* UTILS_RESTRICT const distances,
        mat4f const& UTILS_RESTRICT viewMatrix,
        float4 const* UTILS_RESTRICT spheres, size_t count) noexcept {

    // without this, the vectorization is less efficient
    // we're guaranteed to have a multiple of 4 lights (at least)
    count = uint32_t(count + 3u) & ~3u;
    for (size_t i = 0 ; i < count; i++) {
        const float4 sphere = spheres[i];
        const float4 center = viewMatrix * sphere.xyz; // camera points towards the -z axis
        distances[i] = length(center);
    }
}

void FView::updatePrimitivesLod(FScene::RenderableSoa& renderableData,
        FEngine const& engine, CameraInfo const&, Range visible) noexcept {
    FRenderableManager const& rcm = engine.getRenderableManager();
    for (uint32_t const index : visible) {
        uint8_t const level = 0; // TODO: pick the proper level of detail
        auto ri = renderableData.elementAt<FScene::RENDERABLE_INSTANCE>(index);
        renderableData.elementAt<FScene::PRIMITIVES>(index) = rcm.getRenderPrimitives(ri, level);
    }
}

FrameGraphId<FrameGraphTexture> FView::renderShadowMaps(FEngine& engine, FrameGraph& fg,
        CameraInfo const& cameraInfo, float4 const& userTime,
        RenderPassBuilder const& passBuilder) noexcept {
    assert_invariant(needsShadowMap());
    return mShadowMapManager->render(engine, fg, passBuilder, *this, cameraInfo, userTime);
}

void FView::commitFrameHistory(FEngine& engine) noexcept {
    // Here we need to destroy resources in mFrameHistory.back()
    auto& disposer = engine.getResourceAllocatorDisposer();
    auto& frameHistory = mFrameHistory;

    FrameHistoryEntry& last = frameHistory.back();
    disposer.destroy(std::move(last.taa.color.handle));
    disposer.destroy(std::move(last.ssr.color.handle));

    // and then push the new history entry to the history stack
    frameHistory.commit();
}

void FView::clearFrameHistory(FEngine& engine) noexcept {
    // make sure we free all resources in the history
    auto& disposer = engine.getResourceAllocatorDisposer();
    auto& frameHistory = mFrameHistory;
    for (size_t i = 0; i < frameHistory.size(); ++i) {
        FrameHistoryEntry& last = frameHistory[i];
        disposer.destroy(std::move(last.taa.color.handle));
        disposer.destroy(std::move(last.ssr.color.handle));
    }
}

void FView::executePickingQueries(DriverApi& driver,
        RenderTargetHandle handle, float2 scale) noexcept {

    while (mActivePickingQueriesList) {
        FPickingQuery* const pQuery = mActivePickingQueriesList;
        mActivePickingQueriesList = pQuery->next;

        // adjust for dynamic resolution and structure buffer scale
        const uint32_t x = uint32_t(float(pQuery->x) * scale.x);
        const uint32_t y = uint32_t(float(pQuery->y) * scale.y);

        if (UTILS_UNLIKELY(driver.getFeatureLevel() == FeatureLevel::FEATURE_LEVEL_0)) {
            driver.readPixels(handle, x, y, 1, 1, {
                    &pQuery->result.reserved1, 4u, // 4
                    PixelDataFormat::RGBA, PixelDataType::UBYTE,
                    pQuery->handler, [](void*, size_t, void* user) {
                        FPickingQuery* pQuery = static_cast<FPickingQuery*>(user);
                        uint8_t const* const p =
                                reinterpret_cast<uint8_t const *>(&pQuery->result.reserved1);
                        uint32_t const r = p[0];
                        uint32_t const g = p[1];
                        uint32_t const b = p[2];
                        uint32_t const a = p[3];
                        int32_t const identity = int32_t(a << 16u | (b << 8u) | g);
                        float const depth = float(r) / 255.0f;
                        pQuery->result.renderable = Entity::import(identity);
                        pQuery->result.depth = depth;
                        pQuery->result.fragCoords = {
                                pQuery->x, pQuery->y, float(1.0 - depth) };
                        pQuery->callback(pQuery->result, pQuery);
                        FPickingQuery::put(pQuery);
                    }, pQuery
            });
        } else {
            driver.readPixels(handle, x, y, 1, 1, {
                    &pQuery->result.renderable, 4u * 4u, // 4*float
                    PixelDataFormat::RG, PixelDataType::FLOAT,
                    pQuery->handler, [](void*, size_t, void* user) {
                        FPickingQuery* const pQuery = static_cast<FPickingQuery*>(user);
                        // pQuery->result.renderable already contains the right value!
                        pQuery->result.fragCoords = {
                                pQuery->x, pQuery->y, float(1.0 - pQuery->result.depth) };
                        pQuery->callback(pQuery->result, pQuery);
                        FPickingQuery::put(pQuery);
                    }, pQuery
            });
        }
    }
}

void FView::setTemporalAntiAliasingOptions(TemporalAntiAliasingOptions options) noexcept {
    options.feedback = clamp(options.feedback, 0.0f, 1.0f);
    options.filterWidth = std::max(0.2f, options.filterWidth); // below 0.2 causes issues
    mTemporalAntiAliasingOptions = options;
}

void FView::setMultiSampleAntiAliasingOptions(MultiSampleAntiAliasingOptions options) noexcept {
    options.sampleCount = uint8_t(options.sampleCount < 1u ? 1u : options.sampleCount);
    mMultiSampleAntiAliasingOptions = options;
    assert_invariant(!options.enabled || !mRenderTarget || !mRenderTarget->hasSampleableDepth());
}

void FView::setScreenSpaceReflectionsOptions(ScreenSpaceReflectionsOptions options) noexcept {
    options.thickness = std::max(0.0f, options.thickness);
    options.bias = std::max(0.0f, options.bias);
    options.maxDistance = std::max(0.0f, options.maxDistance);
    options.stride = std::max(1.0f, options.stride);
    mScreenSpaceReflectionsOptions = options;
}

void FView::setGuardBandOptions(GuardBandOptions options) noexcept {
    mGuardBandOptions = options;
}

void FView::setAmbientOcclusionOptions(AmbientOcclusionOptions options) noexcept {
    options.radius = max(0.0f, options.radius);
    options.power = std::max(0.0f, options.power);
    options.bias = clamp(options.bias, 0.0f, 0.1f);
    // snap to the closer of 0.5 or 1.0
    options.resolution = std::floor(
            clamp(options.resolution * 2.0f, 1.0f, 2.0f) + 0.5f) * 0.5f;
    options.intensity = std::max(0.0f, options.intensity);
    options.bilateralThreshold = std::max(0.0f, options.bilateralThreshold);
    options.minHorizonAngleRad = clamp(options.minHorizonAngleRad, 0.0f, f::PI_2);
    options.ssct.lightConeRad = clamp(options.ssct.lightConeRad, 0.0f, f::PI_2);
    options.ssct.shadowDistance = std::max(0.0f, options.ssct.shadowDistance);
    options.ssct.contactDistanceMax = std::max(0.0f, options.ssct.contactDistanceMax);
    options.ssct.intensity = std::max(0.0f, options.ssct.intensity);
    options.ssct.lightDirection = normalize(options.ssct.lightDirection);
    options.ssct.depthBias = std::max(0.0f, options.ssct.depthBias);
    options.ssct.depthSlopeBias = std::max(0.0f, options.ssct.depthSlopeBias);
    options.ssct.sampleCount = clamp((unsigned)options.ssct.sampleCount, 1u, 255u);
    options.ssct.rayCount = clamp((unsigned)options.ssct.rayCount, 1u, 255u);
    mAmbientOcclusionOptions = options;
}
void FView::setVsmShadowOptions(VsmShadowOptions options) noexcept {
    options.msaaSamples = std::max(uint8_t(0), options.msaaSamples);
    mVsmShadowOptions = options;
}

void FView::setSoftShadowOptions(SoftShadowOptions options) noexcept {
    options.penumbraScale = std::max(0.0f, options.penumbraScale);
    options.penumbraRatioScale = std::max(1.0f, options.penumbraRatioScale);
    mSoftShadowOptions = options;
}

void FView::setBloomOptions(BloomOptions options) noexcept {
    options.dirtStrength = saturate(options.dirtStrength);
    options.resolution = clamp(options.resolution, 2u, 2048u);
    options.levels = clamp(options.levels, uint8_t(1),
            FTexture::maxLevelCount(options.resolution));
    options.highlight = std::max(10.0f, options.highlight);
    mBloomOptions = options;
}

void FView::setFogOptions(FogOptions options) noexcept {
    options.distance = std::max(0.0f, options.distance);
    options.maximumOpacity = clamp(options.maximumOpacity, 0.0f, 1.0f);
    options.density = std::max(0.0f, options.density);
    options.heightFalloff = std::max(0.0f, options.heightFalloff);
    options.inScatteringSize = options.inScatteringSize;
    options.inScatteringStart = std::max(0.0f, options.inScatteringStart);
    mFogOptions = options;
}

void FView::setDepthOfFieldOptions(DepthOfFieldOptions options) noexcept {
    options.cocScale = std::max(0.0f, options.cocScale);
    options.maxApertureDiameter = std::max(0.0f, options.maxApertureDiameter);
    mDepthOfFieldOptions = options;
}

void FView::setVignetteOptions(VignetteOptions options) noexcept {
    options.roundness = saturate(options.roundness);
    options.midPoint = saturate(options.midPoint);
    options.feather = clamp(options.feather, 0.05f, 1.0f);
    mVignetteOptions = options;
}

View::PickingQuery& FView::pick(uint32_t x, uint32_t y, CallbackHandler* handler,
        PickingQueryResultCallback callback) noexcept {
    FPickingQuery* pQuery = FPickingQuery::get(x, y, handler, callback);
    pQuery->next = mActivePickingQueriesList;
    mActivePickingQueriesList = pQuery;
    return *pQuery;
}

void FView::setStereoscopicOptions(const StereoscopicOptions& options) noexcept {
    mStereoscopicOptions = options;
}

void FView::setMaterialGlobal(uint32_t index, float4 const& value) {
    FILAMENT_CHECK_PRECONDITION(index < 4)
            << "material global variable index (" << +index << ") out of range";
    mMaterialGlobals[index] = value;
}

float4 FView::getMaterialGlobal(uint32_t index) const {
    FILAMENT_CHECK_PRECONDITION(index < 4)
            << "material global variable index (" << +index << ") out of range";
    return mMaterialGlobals[index];
}

} // namespace filament
