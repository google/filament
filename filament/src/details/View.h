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

#include "upcast.h"

#include "UniformBuffer.h"

#include "details/Allocators.h"
#include "details/Camera.h"
#include "details/Froxelizer.h"
#include "details/RenderTarget.h"
#include "details/ShadowMap.h"
#include "details/Scene.h"

#include "private/backend/DriverApi.h"

#include <backend/Handle.h>

#include <utils/compiler.h>
#include <utils/Allocator.h>
#include <utils/StructureOfArrays.h>
#include <utils/Slice.h>
#include <utils/Range.h>

#include <math/scalar.h>

#include <array>

namespace utils {
class JobSystem;
} // namespace utils;

namespace filament {
namespace details {

class FEngine;
class FMaterialInstance;
class FRenderer;
class FScene;

class FView : public View {
public:
    using Range = utils::Range<uint32_t>;

    explicit FView(FEngine& engine);
    ~FView() noexcept;

    void terminate(FEngine& engine);

    void prepare(FEngine& engine, backend::DriverApi& driver, ArenaScope& arena,
            Viewport const& viewport, math::float4 const& userTime) noexcept;

    void setScene(FScene* scene) { mScene = scene; }
    FScene const* getScene() const noexcept { return mScene; }
    FScene* getScene() noexcept { return mScene; }

    void setCullingCamera(FCamera* camera) noexcept { mCullingCamera = camera; }
    void setViewingCamera(FCamera* camera) noexcept { mViewingCamera = camera; }

    CameraInfo const& getCameraInfo() const noexcept { return mViewingCameraInfo; }

    void setViewport(Viewport const& viewport) noexcept;
    Viewport const& getViewport() const noexcept {
        return mViewport;
    }

    void setClearColor(LinearColorA const& clearColor) noexcept;
    LinearColorA const& getClearColor() const noexcept {
        return mClearColor;
    }

    void setClearTargets(bool color, bool depth, bool stencil) noexcept;
    bool getClearTargetColor() const noexcept {
        // don't clear the color buffer if we have a skybox
        return mClearTargetColor && !isSkyboxVisible();
    }
    bool getClearTargetDepth() const noexcept {
        return mClearTargetDepth;
    }
    bool getClearTargetStencil() const noexcept {
        return mClearTargetStencil;
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
        return mName.c_str();
    }

    void prepareCamera(const CameraInfo& camera, const Viewport& viewport) const noexcept;
    void prepareShadowing(FEngine& engine, backend::DriverApi& driver,
            FScene::RenderableSoa& renderableData, FScene::LightSoa const& lightData) noexcept;
    void prepareLighting(FEngine& engine, FEngine::DriverApi& driver,
            ArenaScope& arena, Viewport const& viewport) noexcept;
    void prepareSSAO(backend::Handle<backend::HwTexture> ssao) const noexcept;
    void cleanupSSAO() const noexcept;
    void froxelize(FEngine& engine) const noexcept;
    void commitUniforms(backend::DriverApi& driver) const noexcept;
    void commitFroxels(backend::DriverApi& driverApi) const noexcept;

    bool hasDirectionalLight() const noexcept { return mHasDirectionalLight; }
    bool hasDynamicLighting() const noexcept { return mHasDynamicLighting; }
    bool hasShadowing() const noexcept { return mHasShadowing & mDirectionalShadowMap.hasVisibleShadows(); }

    void updatePrimitivesLod(
            FEngine& engine, const CameraInfo& camera,
            FScene::RenderableSoa& renderableData, Range visible) noexcept;

    void setShadowsEnabled(bool enabled) noexcept { mShadowingEnabled = enabled; }

    ShadowMap const& getShadowMap() const { return mDirectionalShadowMap; }
    ShadowMap& getShadowMap() { return mDirectionalShadowMap; }

    FCamera const* getDirectionalLightCamera() const noexcept {
        return &mDirectionalShadowMap.getDebugCamera();
    }

    void setRenderTarget(FRenderTarget* renderTarget, TargetBufferFlags discard) noexcept {
        mRenderTarget = renderTarget;
        mDiscardedTargetBuffers = discard;
    }

    void setRenderTarget(TargetBufferFlags discard) noexcept {
        mDiscardedTargetBuffers = discard;
    }

    FRenderTarget* getRenderTarget() const noexcept {
        return mRenderTarget;
    }

    void setSampleCount(uint8_t count) noexcept {
        mSampleCount = uint8_t(count < 1u ? 1u : count);
    }

    uint8_t getSampleCount() const noexcept {
        return mSampleCount;
    }

    void setAntiAliasing(AntiAliasing type) noexcept {
        mAntiAliasing = type;
    }

    AntiAliasing getAntiAliasing() const noexcept {
        return mAntiAliasing;
    }

    void setToneMapping(ToneMapping type) noexcept {
        mToneMapping = type;
    }

    ToneMapping getToneMapping() const noexcept {
        return mToneMapping;
    }

    void setDithering(Dithering dithering) noexcept {
        mDithering = dithering;
    }

    Dithering getDithering() const noexcept {
        return mDithering;
    }

    TargetBufferFlags getDiscardedTargetBuffers() const noexcept { return mDiscardedTargetBuffers; }

    bool hasPostProcessPass() const noexcept {
        return mHasPostProcessPass;
    }

    math::float2 updateScale(std::chrono::duration<float, std::milli> frameTime) noexcept;

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

    void setDepthPrepass(DepthPrepass prepass) noexcept {
#ifdef __EMSCRIPTEN__
        if (prepass == View::DepthPrepass::ENABLED) {
            utils::slog.w << "WARNING: " <<
                "Depth prepass cannot be enabled on web due to invariance requirements." <<
                utils::io::endl;
            return;
        }
#endif
        mDepthPrepass = prepass;
    }

    DepthPrepass getDepthPrepass() const noexcept {
        return mDepthPrepass;
    }

    void setAmbientOcclusion(AmbientOcclusion ambientOcclusion) noexcept {
        mAmbientOcclusion = ambientOcclusion;
    }

    AmbientOcclusion getAmbientOcclusion() const noexcept {
        return mAmbientOcclusion;
    }

    void setAmbientOcclusionOptions(AmbientOcclusionOptions const& options) noexcept {
        mAmbientOcclusionOptions = options;
        mAmbientOcclusionOptions.radius = math::clamp(0.0f, 10.0f, mAmbientOcclusionOptions.radius);
        mAmbientOcclusionOptions.bias = math::clamp(0.0f, 0.1f, mAmbientOcclusionOptions.bias);
        mAmbientOcclusionOptions.power = math::clamp(0.0f, 1.0f, mAmbientOcclusionOptions.power);
    }

    AmbientOcclusionOptions const& getAmbientOcclusionOptions() const noexcept {
        return mAmbientOcclusionOptions;
    }

    Range const& getVisibleRenderables() const noexcept {
        return mVisibleRenderables;
    }

    Range const& getVisibleShadowCasters() const noexcept {
        return mVisibleShadowCasters;
    }

    uint8_t getClearFlags() const noexcept {
        uint8_t clearFlags = 0;
        if (getClearTargetColor())     clearFlags |= (uint8_t)TargetBufferFlags::COLOR;
        if (getClearTargetDepth())     clearFlags |= (uint8_t)TargetBufferFlags::DEPTH;
        if (getClearTargetStencil())   clearFlags |= (uint8_t)TargetBufferFlags::STENCIL;
        return clearFlags;
    }

    FCamera& getCameraUser() noexcept { return *mCullingCamera; }
    void setCameraUser(FCamera* camera) noexcept { setCullingCamera(camera); }

    backend::Handle<backend::HwRenderTarget> getRenderTargetHandle() const noexcept {
        constexpr backend::Handle<backend::HwRenderTarget> kEmptyHandle;
        return mRenderTarget == nullptr ? kEmptyHandle : mRenderTarget->getHwHandle();
    }

private:
    static constexpr size_t MAX_FRAMETIME_HISTORY = 32u;

    void prepareVisibleRenderables(utils::JobSystem& js,
            Frustum const& frustum, FScene::RenderableSoa& renderableData) const noexcept;

    static void prepareVisibleShadowCasters(utils::JobSystem& js,
            Frustum const& lightFrustum, FScene::RenderableSoa& renderableData) noexcept;

    static void prepareVisibleLights(
            FLightManager const& lcm, utils::JobSystem& js, Frustum const& frustum,
            FScene::LightSoa& lightData) noexcept;

    static void cullRenderables(utils::JobSystem& js,
            FScene::RenderableSoa& renderableData, Frustum const& frustum, size_t bit) noexcept;

    void computeVisibilityMasks(
            uint8_t visibleLayers, uint8_t const* layers,
            FRenderableManager::Visibility const* visibility, uint8_t* visibleMask,
            size_t count) const;

    void bindPerViewUniformsAndSamplers(FEngine::DriverApi& driver) const noexcept {
        driver.bindUniformBuffer(BindingPoints::PER_VIEW, mPerViewUbh);
        driver.bindUniformBuffer(BindingPoints::LIGHTS, mLightUbh);
        driver.bindSamplers(BindingPoints::PER_VIEW, mPerViewSbh);
    }

    // we don't inline this one, because the function is quite large and there is not much to
    // gain from inlining.
    static FScene::RenderableSoa::iterator partition(
            FScene::RenderableSoa::iterator begin, FScene::RenderableSoa::iterator end, uint8_t mask) noexcept;

    // these are accessed in the render loop, keep together
    backend::Handle<backend::HwSamplerGroup> mPerViewSbh;
    backend::Handle<backend::HwUniformBuffer> mPerViewUbh;
    backend::Handle<backend::HwUniformBuffer> mLightUbh;
    backend::Handle<backend::HwUniformBuffer> mRenderableUbh;

    backend::Handle<backend::HwSamplerGroup> getUsh() const noexcept { return mPerViewSbh; }
    backend::Handle<backend::HwUniformBuffer> getUbh() const noexcept { return mPerViewUbh; }
    backend::Handle<backend::HwUniformBuffer> getLightUbh() const noexcept { return mLightUbh; }

    FScene* mScene = nullptr;
    FCamera* mCullingCamera = nullptr;
    FCamera* mViewingCamera = nullptr;

    CameraInfo mViewingCameraInfo;
    Frustum mCullingFrustum;

    mutable Froxelizer mFroxelizer;

    Viewport mViewport;
    LinearColorA mClearColor;
    bool mCulling = true;
    bool mFrontFaceWindingInverted = false;
    bool mClearTargetColor = true;
    bool mClearTargetDepth = true;
    bool mClearTargetStencil = false;

    TargetBufferFlags mDiscardedTargetBuffers = TargetBufferFlags::ALL;
    FRenderTarget* mRenderTarget = nullptr;

    uint8_t mVisibleLayers = 0x1;
    uint8_t mSampleCount = 1;
    AntiAliasing mAntiAliasing = AntiAliasing::FXAA;
    ToneMapping mToneMapping = ToneMapping::ACES;
    Dithering mDithering = Dithering::TEMPORAL;
    bool mShadowingEnabled = true;
    bool mHasPostProcessPass = true;
    DepthPrepass mDepthPrepass = DepthPrepass::DEFAULT;
    AmbientOcclusion mAmbientOcclusion = AmbientOcclusion::NONE;
    AmbientOcclusionOptions mAmbientOcclusionOptions{};

    using duration = std::chrono::duration<float, std::milli>;
    DynamicResolutionOptions mDynamicResolution;
    std::array<duration, MAX_FRAMETIME_HISTORY> mFrameTimeHistory;
    size_t mFrameTimeHistorySize = 0;

    math::float2 mScale = 1.0f;
    float mDynamicWorkloadScale = 1.0f;
    bool mIsDynamicResolutionSupported = false;

    RenderQuality mRenderQuality;

    mutable UniformBuffer mPerViewUb;
    mutable backend::SamplerGroup mPerViewSb;

    utils::CString mName;

    // the following values are set by prepare()
    Range mVisibleRenderables;
    Range mVisibleShadowCasters;
    uint32_t mRenderableUBOSize = 0;
    mutable bool mHasDirectionalLight = false;
    mutable bool mHasDynamicLighting = false;
    mutable bool mHasShadowing = false;
    mutable ShadowMap mDirectionalShadowMap;
};

FILAMENT_UPCAST(View)

} // namespace details
} // namespace filament

#endif // TNT_FILAMENT_DETAILS_VIEW_H
