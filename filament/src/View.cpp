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

#include "details/Engine.h"
#include "details/Culler.h"
#include "details/DFG.h"
#include "details/Froxelizer.h"
#include "details/IndirectLight.h"
#include "details/MaterialInstance.h"
#include "details/Renderer.h"
#include "details/RenderTarget.h"
#include "details/Scene.h"
#include "details/Skybox.h"

#include <filament/Exposure.h>

#include <private/filament/SibGenerator.h>
#include <private/filament/UibGenerator.h>

#include <utils/Allocator.h>
#include <utils/Profiler.h>
#include <utils/Slice.h>
#include <utils/Systrace.h>

#include <math/scalar.h>
#include <math/fast.h>

#include <memory>


using namespace filament::math;
using namespace utils;

namespace filament {

using namespace backend;

namespace details {

// values of the 'VISIBLE_MASK' after culling (0: not visible)
static constexpr size_t VISIBLE_RENDERABLE_BIT = 0u;
static constexpr size_t VISIBLE_SHADOW_CASTER_BIT = 1u;
static constexpr uint8_t VISIBLE_RENDERABLE = 1u << VISIBLE_RENDERABLE_BIT;
static constexpr uint8_t VISIBLE_SHADOW_CASTER = 1u << VISIBLE_SHADOW_CASTER_BIT;
static constexpr uint8_t VISIBLE_ALL = VISIBLE_RENDERABLE | VISIBLE_SHADOW_CASTER;

FView::FView(FEngine& engine)
    : mFroxelizer(engine),
      mPerViewUb(PerViewUib::getUib().getSize()),
      mPerViewSb(PerViewSib::SAMPLER_COUNT),
      mDirectionalShadowMap(engine) {
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

    mIsDynamicResolutionSupported = driver.isFrameTimeSupported();
}

FView::~FView() noexcept = default;

void FView::terminate(FEngine& engine) {
    // Here we would cleanly free resources we've allocated or we own (currently none).
    DriverApi& driver = engine.getDriverApi();
    driver.destroyUniformBuffer(mPerViewUbh);
    driver.destroyUniformBuffer(mLightUbh);
    driver.destroySamplerGroup(mPerViewSbh);
    driver.destroyUniformBuffer(mRenderableUbh);
    mDirectionalShadowMap.terminate(driver);
    mFroxelizer.terminate(driver);
}

void FView::setViewport(filament::Viewport const& viewport) noexcept {
    mViewport = viewport;
}

void FView::setDynamicResolutionOptions(DynamicResolutionOptions const& options) noexcept {
    DynamicResolutionOptions& dynamicResolution = mDynamicResolution;
    dynamicResolution = options;

    // only enable if dynamic resolution is supported
    dynamicResolution.enabled = dynamicResolution.enabled && mIsDynamicResolutionSupported;
    if (dynamicResolution.enabled) {
        // if enabled, sanitize the parameters

        // History can't be more than 32 frames (~0.5s)
        dynamicResolution.history = std::min(dynamicResolution.history, uint8_t(MAX_FRAMETIME_HISTORY));

        // History must at least be 3 frames
        dynamicResolution.history = std::max(dynamicResolution.history, uint8_t(3));

        // can't ask more 240 fps
        dynamicResolution.targetFrameTimeMilli =
                std::max(dynamicResolution.targetFrameTimeMilli, 1000.0f / 240.0f);

        // can't ask less than 1 fps
        dynamicResolution.targetFrameTimeMilli =
                std::min(dynamicResolution.targetFrameTimeMilli, 1000.0f);

        // headroom can't be larger than frame time, or less than 0
        dynamicResolution.headRoomRatio = std::min(dynamicResolution.headRoomRatio, 1.0f);
        dynamicResolution.headRoomRatio = std::max(dynamicResolution.headRoomRatio, 0.0f);

        // minScale cannot be 0 or negative
        dynamicResolution.minScale = max(dynamicResolution.minScale, float2(1.0f / 1024.0f));

        // maxScale cannot be < minScale
        dynamicResolution.maxScale = max(dynamicResolution.maxScale, dynamicResolution.minScale);

        // clamp maxScale to 2x because we're doing bilinear filtering, so super-sampling
        // is not useful above that.
        dynamicResolution.maxScale = min(dynamicResolution.maxScale, float2(2.0f));

        // reset the history, so we start from a known (and current) state
        mFrameTimeHistorySize = 0;
        mScale = 1.0f;
        mDynamicWorkloadScale = 1.0f;
    }
}

void FView::setDynamicLightingOptions(float zLightNear, float zLightFar) noexcept {
    mFroxelizer.setOptions(zLightNear, zLightFar);
}

// this is to avoid a call to memmove
template<class InputIterator, class OutputIterator>
static inline
void move_backward(InputIterator first, InputIterator last, OutputIterator result) {
    while (first != last) {
        *--result = *--last;
    }
}

float2 FView::updateScale(duration frameTime) noexcept {
    DynamicResolutionOptions const& options = mDynamicResolution;
    if (options.enabled) {

        if (UTILS_UNLIKELY(frameTime.count() <= std::numeric_limits<float>::epsilon())) {
            mScale = 1.0f;
            return mScale;
        }

        // keep an history of frame times
        auto& history = mFrameTimeHistory;

        // this is like doing { pop_back(); push_front(); }
        details::move_backward(history.begin(), history.end() - 1, history.end());
        history.front() = frameTime;
        mFrameTimeHistorySize = std::min(++mFrameTimeHistorySize, size_t(MAX_FRAMETIME_HISTORY));

        if (UTILS_UNLIKELY(mFrameTimeHistorySize < 3)) {
            // don't make any decision if we don't have enough data
            mScale = 1.0f;
            return mScale;
        }

        // apply a median filter to get a good representation of the frame time of the last
        // N frames.
        std::array<duration, MAX_FRAMETIME_HISTORY> median; // NOLINT -- it's initialized below
        size_t size = std::min(mFrameTimeHistorySize, median.size());
        std::uninitialized_copy_n(history.begin(), size, median.begin());
        std::sort(median.begin(), median.begin() + size);
        duration filteredFrameTime = median[size / 2];

        // how much we need to scale the current workload to fit in our target, at this instant
        const float targetWithHeadroom = options.targetFrameTimeMilli * (1 - options.headRoomRatio);
        const float workloadScale = targetWithHeadroom / filteredFrameTime.count();

        // low-pass: y += b * (x - y)
        const float oneOverTau = options.scaleRate;
        const float x = mScale.x * mScale.y * workloadScale;
        mDynamicWorkloadScale += (1.0f - std::exp(-oneOverTau)) * (x - mDynamicWorkloadScale);

        // scaling factor we need to apply on the whole surface
        const float scale = mDynamicWorkloadScale;

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

        // now tweak the scaling factor to get multiples of 4 (to help quad-shading)
        mScale = (floor(mScale * float2{ w, h } / 4) * 4) / float2{ w, h };

        // always clamp to the min/max scale range
        mScale = clamp(mScale, options.minScale, options.maxScale);

//#define DEBUG_DYNAMIC_RESOLUTION
#if !defined(NDEBUG) && defined(DEBUG_DYNAMIC_RESOLUTION)
        static int sLogCounter = 15;
        if (!--sLogCounter) {
            sLogCounter = 15;
            slog.d << frameTime.count()
                   << ", " << filteredFrameTime.count()
                   << ", " << workloadScale
                   << ", " << mDynamicWorkloadScale
                   << ", " << mScale.x
                   << ", " << mScale.y
                   << ", " << mScale.x * mScale.y
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

void FView::setClearColor(float4 const& clearColor) noexcept {
    mClearColor = clearColor;
}

void FView::setClearTargets(bool color, bool depth, bool stencil) noexcept {
    mClearTargetColor = color;
    mClearTargetDepth = depth;
    mClearTargetStencil = stencil;
}

void FView::setVisibleLayers(uint8_t select, uint8_t values) noexcept {
    mVisibleLayers = (mVisibleLayers & ~select) | (values & select);
}

bool FView::isSkyboxVisible() const noexcept {
    FSkybox const* skybox = mScene ? mScene->getSkybox() : nullptr;
    return skybox != nullptr && (skybox->getLayerMask() & mVisibleLayers);
}

void FView::prepareShadowing(FEngine& engine, backend::DriverApi& driver,
        FScene::RenderableSoa& renderableData, FScene::LightSoa const& lightData) noexcept {
    SYSTRACE_CALL();

    // setup shadow mapping
    // TODO: for now we only consider THE directional light

    auto& lcm = engine.getLightManager();

    // dominant directional light is always as index 0
    FLightManager::Instance directionalLight = lightData.elementAt<FScene::LIGHT_INSTANCE>(0);
    mHasShadowing = mShadowingEnabled && directionalLight && lcm.isShadowCaster(directionalLight);
    if (UTILS_UNLIKELY(mHasShadowing)) {
        // compute the frustum for this light
        ShadowMap& shadowMap = mDirectionalShadowMap;
        shadowMap.update(lightData, 0, mScene, mViewingCameraInfo, mVisibleLayers);
        if (shadowMap.hasVisibleShadows()) {
            // Cull shadow casters
            UniformBuffer& u = mPerViewUb;
            Frustum const& frustum = shadowMap.getCamera().getFrustum();
            FView::prepareVisibleShadowCasters(engine.getJobSystem(), frustum, renderableData);

            // allocates shadowmap driver resources
            shadowMap.prepare(driver, mPerViewSb);

            mat4f const& lightFromWorldMatrix = shadowMap.getLightSpaceMatrix();
            u.setUniform(offsetof(PerViewUib, lightFromWorldMatrix), lightFromWorldMatrix);

            const float texelSizeWorldSpace = shadowMap.getTexelSizeWorldSpace();
            const float normalBias = lcm.getShadowNormalBias(directionalLight);
            u.setUniform(offsetof(PerViewUib, shadowBias),
                    float3{ 0, normalBias * texelSizeWorldSpace, 0 });
        }
    }
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

    // IBL
    FIndirectLight const* const ibl = scene->getIndirectLight();
    if (UTILS_LIKELY(ibl)) {
        float2 iblMaxMipLevel{ ibl->getMaxMipLevel(), 1u << ibl->getMaxMipLevel() };
        u.setUniform(offsetof(PerViewUib, iblMaxMipLevel), iblMaxMipLevel);
        u.setUniform(offsetof(PerViewUib, iblLuminance), ibl->getIntensity() * exposure);
        u.setUniformArray(offsetof(PerViewUib, iblSH), ibl->getSH(), 9);
        if (ibl->getReflectionMap()) {
            mPerViewSb.setSampler(PerViewSib::IBL_SPECULAR, {
                    ibl->getReflectionMap(), {
                            .filterMag = SamplerMagFilter::LINEAR,
                            .filterMin = SamplerMinFilter::LINEAR_MIPMAP_LINEAR
                    }});
        }
    } else {
        FSkybox const* const skybox = scene->getSkybox();
        const float intensity = skybox ? skybox->getIntensity() : FIndirectLight::DEFAULT_INTENSITY;
        u.setUniform(offsetof(PerViewUib, iblLuminance), intensity * exposure);
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
    // the viewing camera), we can set worldOriginCamera to identity when mViewingCamera
    // is set: e.g.
    //      worldOriginCamera = mViewingCamera ? mat4f{} : worldOriginScene

    const mat4f worldOriginCamera = worldOriginScene;
    const mat4f model{ worldOriginCamera * camera->getModelMatrix() };
    mViewingCameraInfo = CameraInfo{
            // projection with infinite z-far
            .projection         = mat4f{ camera->getProjectionMatrix() },
            // projection used for culling, with finite z-far
            .cullingProjection  = mat4f{ camera->getCullingProjectionMatrix() },
            // camera model matrix -- apply the world origin to it
            .model              = model,
            // camera view matrix
            .view               = FCamera::getViewMatrix(model),
            // near plane
            .zn                 = camera->getNear(),
            // far plane
            .zf                 = camera->getCullingFar(),
            // exposure
            .ev100              = Exposure::ev100(*camera),
            // world offset to allow users to determine the API-level camera position
            .worldOffset        = camera->getPosition(),
            // world origin transform, use only for debugging
            .worldOrigin        = worldOriginCamera
    };
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

    auto prepareVisibleLightsJob = js.runAndRetain(js.createJob(nullptr,
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
         * (this will set the VISIBLE_SHADOW_CASTER bit)
         */

        prepareShadowing(engine, driver, renderableData, scene->getLightData());

        /*
         * partition the array of renderable w.r.t their visibility:
         *
         * Sort the SoA so that invisible objects are first, then renderables,
         * then both renderable and casters, then casters only -- this operation is somewhat heavy
         * as it sorts the whole SoA. We use std::partition instead of sort(), which gives us
         * O(3.N) instead of O(N.log(N)) application of swap().
         */

        // calculate the sorting key for all elements, based on their visibility
        uint8_t const* layers = renderableData.data<FScene::LAYERS>();
        auto const* visibility = renderableData.data<FScene::VISIBILITY_STATE>();
        computeVisibilityMasks(getVisibleLayers(), layers, visibility, cullingMask.begin(),
                renderableData.size());

        auto const beginRenderables = renderableData.begin();
        auto beginCasters = partition(beginRenderables, renderableData.end(), VISIBLE_RENDERABLE);
        auto beginCastersOnly = partition(beginCasters, renderableData.end(), VISIBLE_ALL);
        auto endCastersOnly = partition(beginCastersOnly, renderableData.end(),
                VISIBLE_SHADOW_CASTER);

        // convert to indices
        uint32_t iEnd = uint32_t(endCastersOnly - beginRenderables);
        mVisibleRenderables = Range{ 0, uint32_t(beginCastersOnly - beginRenderables) };
        mVisibleShadowCasters = Range{ uint32_t(beginCasters - beginRenderables), iEnd };
        merged = Range{ 0, iEnd };

        // update those UBOs
        const size_t size = merged.size() * sizeof(PerRenderableUib);
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
        scene->updateUBOs(merged, mRenderableUbh);
    }

    /*
     * Prepare lighting -- this is where we update the lights UBOs, set-up the IBL,
     * set-up the froxelization parameters.
     * Relies on FScene::prepare() and prepareVisibleLights()
     */

    js.waitAndRelease(prepareVisibleLightsJob);
    prepareLighting(engine, driver, arena, viewport);

    /*
     * Update driver state
     */

    const uint64_t oneSecondRemainder = engine.getEngineTime().count() % 1000000000;
    const float fraction = float(double(oneSecondRemainder) / 1000000000.0);
    mPerViewUb.setUniform(offsetof(PerViewUib, time), fraction);
    mPerViewUb.setUniform(offsetof(PerViewUib, userTime), userTime);

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
        uint8_t* UTILS_RESTRICT visibleMask, size_t count) const {
    // __restrict__ seems to only be taken into account as function parameters. This is very
    // important here, otherwise, this loop doesn't get vectorized.
    // This is vectorized 16x.
    count = (count + 0xFu) & ~0xFu; // capacity guaranteed to be multiple of 16
    for (size_t i = 0; i < count; ++i) {
        Culler::result_type mask = visibleMask[i];
        FRenderableManager::Visibility v = visibility[i];
        bool inVisibleLayer = layers[i] & visibleLayers;
        bool visRenderables   = (!v.culling || (mask & VISIBLE_RENDERABLE))    && inVisibleLayer;
        bool visShadowCasters = (!v.culling || (mask & VISIBLE_SHADOW_CASTER)) && inVisibleLayer && v.castShadows;
        visibleMask[i] = Culler::result_type(visRenderables) |
                         Culler::result_type(visShadowCasters << 1u);
    }
}

UTILS_NOINLINE
/* static */ FScene::RenderableSoa::iterator FView::partition(
        FScene::RenderableSoa::iterator begin,
        FScene::RenderableSoa::iterator end,
        uint8_t mask) noexcept {
    return std::partition(begin, end, [mask](auto it) {
        return it.template get<FScene::VISIBLE_MASK>() == mask;
    });
}

void FView::prepareCamera(const CameraInfo& camera, const filament::Viewport& viewport) const noexcept {
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

    const float w = viewport.width;
    const float h = viewport.height;
    u.setUniform(offsetof(PerViewUib, resolution), float4{ w, h, 1.0f / w, 1.0f / h });
    u.setUniform(offsetof(PerViewUib, origin), float2{ viewport.left, viewport.bottom });

    u.setUniform(offsetof(PerViewUib, cameraPosition), float3{camera.getPosition()});
    u.setUniform(offsetof(PerViewUib, worldOffset), camera.worldOffset);
}

void FView::prepareSSAO(Handle<HwTexture> ssao) const noexcept {
    mPerViewSb.setSampler(PerViewSib::SSAO, ssao, {
            .filterMag = SamplerMagFilter::LINEAR
    });
}

void FView::cleanupSSAO() const noexcept {
    mPerViewSb.setSampler(PerViewSib::SSAO, {}, {});
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

UTILS_NOINLINE
void FView::prepareVisibleShadowCasters(JobSystem& js,
        Frustum const& lightFrustum, FScene::RenderableSoa& renderableData) noexcept {
    SYSTRACE_CALL();
    FView::cullRenderables(js, renderableData, lightFrustum, VISIBLE_SHADOW_CASTER_BIT);
}

void FView::cullRenderables(JobSystem& js,
        FScene::RenderableSoa& renderableData, Frustum const& frustum, size_t bit) noexcept {

    float3 const* worldAABBCenter = renderableData.data<FScene::WORLD_AABB_CENTER>();
    float3 const* worldAABBExtent = renderableData.data<FScene::WORLD_AABB_EXTENT>();
    uint8_t     * visibleArray    = renderableData.data<FScene::VISIBLE_MASK>();

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
    auto job = jobs::parallel_for(js, nullptr, 0, (uint32_t)renderableData.size(),
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

} // namespace details

// ------------------------------------------------------------------------------------------------
// Trampoline calling into private implementation
// ------------------------------------------------------------------------------------------------

using namespace details;

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


void View::setClearColor(float4 const& clearColor) noexcept {
    upcast(this)->setClearColor(clearColor);
}

float4 const& View::getClearColor() const noexcept {
    return upcast(this)->getClearColor();
}

void View::setClearTargets(bool color, bool depth, bool stencil) noexcept {
    return upcast(this)->setClearTargets(color, depth, stencil);
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

void View::setShadowsEnabled(bool enabled) noexcept {
    upcast(this)->setShadowsEnabled(enabled);
}

void View::setRenderTarget(RenderTarget* renderTarget, TargetBufferFlags discard) noexcept {
    upcast(this)->setRenderTarget(upcast(renderTarget), discard);
}

void View::setRenderTarget(TargetBufferFlags discard) noexcept {
    upcast(this)->setRenderTarget(discard);
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

void View::setToneMapping(ToneMapping type) noexcept {
    upcast(this)->setToneMapping(type);
}

View::ToneMapping View::getToneMapping() const noexcept {
    return upcast(this)->getToneMapping();
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

void View::setDepthPrepass(View::DepthPrepass prepass) noexcept {
    upcast(this)->setDepthPrepass(prepass);
}

View::DepthPrepass View::getDepthPrepass() const noexcept {
    return upcast(this)->getDepthPrepass();
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

View::AmbientOcclusionOptions const& View::getAmbientOcclusionOptions() const noexcept {
    return upcast(this)->getAmbientOcclusionOptions();
}


} // namespace filament
