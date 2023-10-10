/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TNT_FILAMENT_DETAILS_VIEW_H
#define TNT_FILAMENT_DETAILS_VIEW_H

#include <filament/View.h>

#include <filament/Renderer.h>

#include "downcast.h"

#include "Allocators.h"
#include "FrameHistory.h"
#include "FrameInfo.h"
#include "Froxelizer.h"
#include "PerViewUniforms.h"
#include "PIDController.h"
#include "ShadowMap.h"
#include "ShadowMapManager.h"
#include "TypedUniformBuffer.h"

#include "details/Camera.h"
#include "details/ColorGrading.h"
#include "details/RenderTarget.h"
#include "details/Scene.h"

#include <private/filament/EngineEnums.h>

#include "private/backend/DriverApi.h"

#include <backend/Handle.h>

#include <utils/compiler.h>
#include <utils/Allocator.h>
#include <utils/StructureOfArrays.h>
#include <utils/Range.h>
#include <utils/Slice.h>

#include <math/scalar.h>
#include <math/mat4.h>

namespace utils {
class JobSystem;
} // namespace utils;

// Avoid warnings for using the deprecated APIs.
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined(_MSC_VER)
#pragma warning push
#pragma warning disable : 4996
#endif

namespace filament {

class FEngine;
class FMaterialInstance;
class FRenderer;
class FScene;

// ------------------------------------------------------------------------------------------------

class FView : public View {
public:
    using Range = utils::Range<uint32_t>;

    explicit FView(FEngine& engine);
    ~FView() noexcept;

    void terminate(FEngine& engine);

    CameraInfo computeCameraInfo(FEngine& engine) const noexcept;

    // note: viewport/cameraInfo are passed by value to make it clear that prepare cannot
    // keep references on them that would outlive the scope of prepare() (e.g. with JobSystem).
    void prepare(FEngine& engine, backend::DriverApi& driver, ArenaScope& arena,
            filament::Viewport viewport, CameraInfo cameraInfo,
            math::float4 const& userTime, bool needsAlphaChannel) noexcept;

    void bindPerViewUniformsAndSamplers(FEngine::DriverApi& driver) const noexcept;

    void setScene(FScene* scene) { mScene = scene; }
    FScene const* getScene() const noexcept { return mScene; }
    FScene* getScene() noexcept { return mScene; }

    void setCullingCamera(FCamera* camera) noexcept { mCullingCamera = camera; }
    void setViewingCamera(FCamera* camera) noexcept { mViewingCamera = camera; }

    void setViewport(Viewport const& viewport) noexcept;
    Viewport const& getViewport() const noexcept {
        return mViewport;
    }

    bool getClearTargetColor() const noexcept {
        // don't clear the color buffer if we have a skybox
        return !isSkyboxVisible();
    }
    bool isSkyboxVisible() const noexcept;

    void setFrustumCullingEnabled(bool culling) noexcept { mCulling = culling; }
    bool isFrustumCullingEnabled() const noexcept { return mCulling; }

    void setFrontFaceWindingInverted(bool inverted) noexcept { mFrontFaceWindingInverted = inverted; }
    bool isFrontFaceWindingInverted() const noexcept { return mFrontFaceWindingInverted; }


    void setVisibleLayers(uint8_t select, uint8_t values) noexcept;
    uint8_t getVisibleLayers() const noexcept {
        return mVisibleLayers;
    }

    void setName(const char* name) noexcept {
        mName = utils::CString(name);
    }

    // returns the view's name. The pointer is owned by View.
    const char* getName() const noexcept {
        return mName.c_str_safe();
    }

    void prepareUpscaler(math::float2 scale) const noexcept;
    void prepareCamera(FEngine& engine, const CameraInfo& cameraInfo) const noexcept;

    void prepareViewport(
            const Viewport& physicalViewport,
            const filament::Viewport& logicalViewport) const noexcept;

    void prepareShadowing(FEngine& engine, FScene::RenderableSoa& renderableData,
            FScene::LightSoa const& lightData, CameraInfo const& cameraInfo) noexcept;
    void prepareLighting(FEngine& engine, ArenaScope& arena, CameraInfo const& cameraInfo) noexcept;

    void prepareSSAO(backend::Handle<backend::HwTexture> ssao) const noexcept;
    void prepareSSR(backend::Handle<backend::HwTexture> ssr, float refractionLodOffset,
            ScreenSpaceReflectionsOptions const& ssrOptions) const noexcept;
    void prepareStructure(backend::Handle<backend::HwTexture> structure) const noexcept;
    void prepareShadow(backend::Handle<backend::HwTexture> structure) const noexcept;
    void prepareShadowMapping(bool highPrecision) const noexcept;

    void cleanupRenderPasses() const noexcept;
    void commitUniforms(backend::DriverApi& driver) const noexcept;
    void commitFroxels(backend::DriverApi& driverApi) const noexcept;

    utils::JobSystem::Job* getFroxelizerSync() const noexcept { return mFroxelizerSync; }
    void setFroxelizerSync(utils::JobSystem::Job* sync) noexcept { mFroxelizerSync = sync; }

    bool hasDirectionalLight() const noexcept { return mHasDirectionalLight; }
    bool hasDynamicLighting() const noexcept { return mHasDynamicLighting; }
    bool hasShadowing() const noexcept { return mHasShadowing; }
    bool needsShadowMap() const noexcept { return mNeedsShadowMap; }
    bool hasFog() const noexcept { return mFogOptions.enabled && mFogOptions.density > 0.0f; }
    bool hasVSM() const noexcept { return mShadowType == ShadowType::VSM; }
    bool hasDPCF() const noexcept { return mShadowType == ShadowType::DPCF; }
    bool hasPCSS() const noexcept { return mShadowType == ShadowType::PCSS; }
    bool hasPicking() const noexcept { return mActivePickingQueriesList != nullptr; }
    bool hasInstancedStereo() const noexcept { return mStereoscopicOptions.enabled; }

    FrameGraphId<FrameGraphTexture> renderShadowMaps(FEngine& engine, FrameGraph& fg,
            CameraInfo const& cameraInfo, math::float4 const& userTime,
            RenderPass const& pass) noexcept;

    void updatePrimitivesLod(
            FEngine& engine, const CameraInfo& camera,
            FScene::RenderableSoa& renderableData, Range visible) noexcept;

    void setShadowingEnabled(bool enabled) noexcept { mShadowingEnabled = enabled; }

    bool isShadowingEnabled() const noexcept { return mShadowingEnabled; }

    void setScreenSpaceRefractionEnabled(bool enabled) noexcept { mScreenSpaceRefractionEnabled = enabled; }

    bool isScreenSpaceRefractionEnabled() const noexcept { return mScreenSpaceRefractionEnabled; }

    bool isScreenSpaceReflectionEnabled() const noexcept { return mScreenSpaceReflectionsOptions.enabled; }

    void setStencilBufferEnabled(bool enabled) noexcept { mStencilBufferEnabled = enabled; }

    bool isStencilBufferEnabled() const noexcept { return mStencilBufferEnabled; }

    void setStereoscopicOptions(StereoscopicOptions const& options);

    FCamera const* getDirectionalLightCamera() const noexcept {
        return &mShadowMapManager.getShadowMap(0)->getDebugCamera();
    }

    void setRenderTarget(FRenderTarget* renderTarget) noexcept {
        assert_invariant(!renderTarget || !mMultiSampleAntiAliasingOptions.enabled ||
                !renderTarget->hasSampleableDepth());
        mRenderTarget = renderTarget;
    }

    FRenderTarget* getRenderTarget() const noexcept {
        return mRenderTarget;
    }

    void setSampleCount(uint8_t count) noexcept {
        count = uint8_t(count < 1u ? 1u : count);
        mMultiSampleAntiAliasingOptions.sampleCount = count;
        mMultiSampleAntiAliasingOptions.enabled = count > 1u;
    }

    uint8_t getSampleCount() const noexcept {
        return mMultiSampleAntiAliasingOptions.sampleCount;
    }

    void setAntiAliasing(AntiAliasing type) noexcept {
        mAntiAliasing = type;
    }

    AntiAliasing getAntiAliasing() const noexcept {
        return mAntiAliasing;
    }

    void setTemporalAntiAliasingOptions(TemporalAntiAliasingOptions options) noexcept ;

    const TemporalAntiAliasingOptions& getTemporalAntiAliasingOptions() const noexcept {
        return mTemporalAntiAliasingOptions;
    }

    void setMultiSampleAntiAliasingOptions(MultiSampleAntiAliasingOptions options) noexcept;

    const MultiSampleAntiAliasingOptions& getMultiSampleAntiAliasingOptions() const noexcept {
        return mMultiSampleAntiAliasingOptions;
    }

    void setScreenSpaceReflectionsOptions(ScreenSpaceReflectionsOptions options) noexcept;

    const ScreenSpaceReflectionsOptions& getScreenSpaceReflectionsOptions() const noexcept {
        return mScreenSpaceReflectionsOptions;
    }

    void setGuardBandOptions(GuardBandOptions options) noexcept;

    GuardBandOptions const& getGuardBandOptions() const noexcept {
        return mGuardBandOptions;
    }

    void setColorGrading(FColorGrading* colorGrading) noexcept {
        mColorGrading = colorGrading == nullptr ? mDefaultColorGrading : colorGrading;
    }

    const FColorGrading* getColorGrading() const noexcept {
        return mColorGrading;
    }

    void setDithering(Dithering dithering) noexcept {
        mDithering = dithering;
    }

    Dithering getDithering() const noexcept {
        return mDithering;
    }

    const StereoscopicOptions& getStereoscopicOptions() const noexcept {
        return mStereoscopicOptions;
    }

    bool hasPostProcessPass() const noexcept {
        return mHasPostProcessPass;
    }

    math::float2 updateScale(FEngine& engine,
            FrameInfo const& info,
            Renderer::FrameRateOptions const& frameRateOptions,
            Renderer::DisplayInfo const& displayInfo) noexcept;

    void setDynamicResolutionOptions(View::DynamicResolutionOptions const& options) noexcept;

    DynamicResolutionOptions getDynamicResolutionOptions() const noexcept {
        return mDynamicResolution;
    }

    void setRenderQuality(RenderQuality const& renderQuality) noexcept {
        mRenderQuality = renderQuality;
    }

    RenderQuality getRenderQuality() const noexcept {
        return mRenderQuality;
    }

    void setDynamicLightingOptions(float zLightNear, float zLightFar) noexcept;

    void setPostProcessingEnabled(bool enabled) noexcept {
        mHasPostProcessPass = enabled;
    }

    void setAmbientOcclusion(AmbientOcclusion ambientOcclusion) noexcept {
        mAmbientOcclusionOptions.enabled = ambientOcclusion == AmbientOcclusion::SSAO;
    }

    AmbientOcclusion getAmbientOcclusion() const noexcept {
        return mAmbientOcclusionOptions.enabled ? AmbientOcclusion::SSAO : AmbientOcclusion::NONE;
    }

    void setAmbientOcclusionOptions(AmbientOcclusionOptions options) noexcept;

    ShadowType getShadowType() const noexcept {
        return mShadowType;
    }

    void setShadowType(ShadowType shadow) noexcept {
        mShadowType = shadow;
    }

    void setVsmShadowOptions(VsmShadowOptions options) noexcept;

    VsmShadowOptions getVsmShadowOptions() const noexcept {
        return mVsmShadowOptions;
    }

    void setSoftShadowOptions(SoftShadowOptions options) noexcept;

    SoftShadowOptions getSoftShadowOptions() const noexcept {
        return mSoftShadowOptions;
    }

    AmbientOcclusionOptions const& getAmbientOcclusionOptions() const noexcept {
        return mAmbientOcclusionOptions;
    }

    void setBloomOptions(BloomOptions options) noexcept;

    BloomOptions getBloomOptions() const noexcept {
        return mBloomOptions;
    }

    void setFogOptions(FogOptions options) noexcept;

    FogOptions getFogOptions() const noexcept {
        return mFogOptions;
    }

    void setDepthOfFieldOptions(DepthOfFieldOptions options) noexcept;

    DepthOfFieldOptions getDepthOfFieldOptions() const noexcept {
        return mDepthOfFieldOptions;
    }

    void setVignetteOptions(VignetteOptions options) noexcept;

    VignetteOptions getVignetteOptions() const noexcept {
        return mVignetteOptions;
    }

    void setBlendMode(BlendMode blendMode) noexcept {
        mBlendMode = blendMode;
    }

    BlendMode getBlendMode() const noexcept {
        return mBlendMode;
    }

    Range const& getVisibleRenderables() const noexcept {
        return mVisibleRenderables;
    }

    Range const& getVisibleDirectionalShadowCasters() const noexcept {
        return mVisibleDirectionalShadowCasters;
    }

    Range const& getVisibleSpotShadowCasters() const noexcept {
        return mSpotLightShadowCasters;
    }

    FCamera const& getCameraUser() const noexcept { return *mCullingCamera; }
    FCamera& getCameraUser() noexcept { return *mCullingCamera; }
    void setCameraUser(FCamera* camera) noexcept { setCullingCamera(camera); }

    backend::Handle<backend::HwRenderTarget> getRenderTargetHandle() const noexcept {
        backend::Handle<backend::HwRenderTarget> kEmptyHandle;
        return mRenderTarget == nullptr ? kEmptyHandle : mRenderTarget->getHwHandle();
    }

    backend::TargetBufferFlags getRenderTargetAttachmentMask() const noexcept {
        if (mRenderTarget == nullptr) {
            return backend::TargetBufferFlags::NONE;
        } else {
            return mRenderTarget->getAttachmentMask();
        }
    }

    static void cullRenderables(utils::JobSystem& js, FScene::RenderableSoa& renderableData,
            Frustum const& frustum, size_t bit) noexcept;

    PerViewUniforms const& getPerViewUniforms() const noexcept { return mPerViewUniforms; }
    PerViewUniforms& getPerViewUniforms() noexcept { return mPerViewUniforms; }

    // Returns the frame history FIFO. This is typically used by the FrameGraph to access
    // previous frame data.
    FrameHistory& getFrameHistory() noexcept { return mFrameHistory; }
    FrameHistory const& getFrameHistory() const noexcept { return mFrameHistory; }

    // Clean-up the oldest frame and save the current frame information.
    // This is typically called after all operations for this View's rendering are complete.
    // (e.g.: after the FrameGraph execution).
    void commitFrameHistory(FEngine& engine) noexcept;

    // create the picking query
    View::PickingQuery& pick(uint32_t x, uint32_t y, backend::CallbackHandler* handler,
            View::PickingQueryResultCallback callback) noexcept;

    void executePickingQueries(backend::DriverApi& driver,
            backend::RenderTargetHandle handle, float scale) noexcept;

    void setMaterialGlobal(uint32_t index, math::float4 const& value);

    math::float4 getMaterialGlobal(uint32_t index) const;

    utils::Entity getFogEntity() const noexcept {
        return mFogEntity;
    }

private:

    struct FPickingQuery : public PickingQuery {
    private:
        FPickingQuery(uint32_t x, uint32_t y,
                backend::CallbackHandler* handler,
                View::PickingQueryResultCallback callback) noexcept
                : PickingQuery{}, x(x), y(y), handler(handler), callback(callback) {}
        ~FPickingQuery() noexcept = default;
    public:
        // TODO: use a small pool
        static FPickingQuery* get(uint32_t x, uint32_t y, backend::CallbackHandler* handler,
                View::PickingQueryResultCallback callback) noexcept {
            return new FPickingQuery(x, y, handler, callback);
        }
        static void put(FPickingQuery* pQuery) noexcept {
            delete pQuery;
        }
        mutable FPickingQuery* next = nullptr;
        // picking query parameters
        uint32_t const x;
        uint32_t const y;
        backend::CallbackHandler* const handler;
        View::PickingQueryResultCallback const callback;
        // picking query result
        PickingQueryResult result;
    };

    void prepareVisibleRenderables(utils::JobSystem& js,
            Frustum const& frustum, FScene::RenderableSoa& renderableData) const noexcept;

    static void prepareVisibleLights(FLightManager const& lcm, ArenaScope& rootArena,
            math::mat4f const& viewMatrix, Frustum const& frustum,
            FScene::LightSoa& lightData) noexcept;

    static inline void computeLightCameraDistances(float* distances,
            math::mat4f const& viewMatrix, const math::float4* spheres, size_t count) noexcept;

    static void computeVisibilityMasks(
            uint8_t visibleLayers, uint8_t const* layers,
            FRenderableManager::Visibility const* visibility,
            Culler::result_type* visibleMask,
            size_t count);

    // Clean-up the whole history, free all resources. This is typically called when the View is
    // being terminated.
    void drainFrameHistory(FEngine& engine) noexcept;

    // we don't inline this one, because the function is quite large and there is not much to
    // gain from inlining.
    static FScene::RenderableSoa::iterator partition(
            FScene::RenderableSoa::iterator begin,
            FScene::RenderableSoa::iterator end,
            Culler::result_type mask, Culler::result_type value) noexcept;

    // these are accessed in the render loop, keep together
    backend::Handle<backend::HwBufferObject> mLightUbh;
    backend::Handle<backend::HwBufferObject> mRenderableUbh;

    FScene* mScene = nullptr;
    // The camera set by the user, used for culling and viewing
    FCamera* mCullingCamera = nullptr;
    // The optional (debug) camera, used only for viewing
    FCamera* mViewingCamera = nullptr;

    mutable Froxelizer mFroxelizer;
    utils::JobSystem::Job* mFroxelizerSync = nullptr;

    Viewport mViewport;
    bool mCulling = true;
    bool mFrontFaceWindingInverted = false;

    FRenderTarget* mRenderTarget = nullptr;

    uint8_t mVisibleLayers = 0x1;
    AntiAliasing mAntiAliasing = AntiAliasing::FXAA;
    Dithering mDithering = Dithering::TEMPORAL;
    bool mShadowingEnabled = true;
    bool mScreenSpaceRefractionEnabled = true;
    bool mHasPostProcessPass = true;
    bool mStencilBufferEnabled = false;
    AmbientOcclusionOptions mAmbientOcclusionOptions{};
    ShadowType mShadowType = ShadowType::PCF;
    VsmShadowOptions mVsmShadowOptions; // FIXME: this should probably be per-light
    SoftShadowOptions mSoftShadowOptions;
    BloomOptions mBloomOptions;
    FogOptions mFogOptions;
    DepthOfFieldOptions mDepthOfFieldOptions;
    VignetteOptions mVignetteOptions;
    TemporalAntiAliasingOptions mTemporalAntiAliasingOptions;
    MultiSampleAntiAliasingOptions mMultiSampleAntiAliasingOptions;
    ScreenSpaceReflectionsOptions mScreenSpaceReflectionsOptions;
    GuardBandOptions mGuardBandOptions;
    StereoscopicOptions mStereoscopicOptions;
    BlendMode mBlendMode = BlendMode::OPAQUE;
    const FColorGrading* mColorGrading = nullptr;
    const FColorGrading* mDefaultColorGrading = nullptr;
    utils::Entity mFogEntity{};
    bool mIsStereoSupported : 1;

    PIDController mPidController;
    DynamicResolutionOptions mDynamicResolution;
    math::float2 mScale = 1.0f;
    bool mIsDynamicResolutionSupported = false;

    RenderQuality mRenderQuality;

    mutable PerViewUniforms mPerViewUniforms;

    mutable FrameHistory mFrameHistory{};

    FPickingQuery* mActivePickingQueriesList = nullptr;

    utils::CString mName;

    // the following values are set by prepare()
    Range mVisibleRenderables;
    Range mVisibleDirectionalShadowCasters;
    Range mSpotLightShadowCasters;
    uint32_t mRenderableUBOSize = 0;
    mutable bool mHasDirectionalLight = false;
    mutable bool mHasDynamicLighting = false;
    mutable bool mHasShadowing = false;
    mutable bool mNeedsShadowMap = false;

    ShadowMapManager mShadowMapManager;

    std::array<math::float4, 4> mMaterialGlobals = {{
                                                            { 0, 0, 0, 1 },
                                                            { 0, 0, 0, 1 },
                                                            { 0, 0, 0, 1 },
                                                            { 0, 0, 0, 1 },
                                                    }};

#ifndef NDEBUG
    std::array<DebugRegistry::FrameHistory, 5*60> mDebugFrameHistory;
#endif
};

FILAMENT_DOWNCAST(View)

} // namespace filament

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(_MSC_VER)
#pragma warning pop
#endif

#endif // TNT_FILAMENT_DETAILS_VIEW_H
