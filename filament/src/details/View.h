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

#ifndef TNT_FILAMENT_DETAILS_VIEW_H
#define TNT_FILAMENT_DETAILS_VIEW_H

#include <filament/View.h>

#include "upcast.h"

#include "details/Allocators.h"
#include "details/Camera.h"
#include "details/Froxelizer.h"
#include "details/ShadowMap.h"
#include "details/Scene.h"

#include "driver/DriverApi.h"
#include "driver/Handle.h"
#include "driver/UniformBuffer.h"

#include <utils/compiler.h>
#include <utils/Allocator.h>
#include <utils/StructureOfArrays.h>
#include <utils/Slice.h>
#include <utils/Range.h>

#include <deque>

namespace utils {
class JobSystem;
} // namespace utils;

namespace filament {
namespace details {

class FEngine;
class FMaterialInstance;
class FRenderer;
class FScene;
class Froxelizer;

class FView : public View {
public:
    using Range = utils::Range<uint32_t>;

    explicit FView(FEngine& engine);
    ~FView() noexcept;

    void terminate(FEngine& engine);

    void prepare(FEngine& engine, driver::DriverApi& driver, ArenaScope& arena,
            Viewport const& viewport) noexcept;

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

    void setCulling(bool culling) noexcept { mCulling = culling; }
    bool isCullingEnabled() const noexcept { return mCulling; }

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
    void prepareShadowing(FEngine& engine, driver::DriverApi& driver,
            FScene::RenderableSoa& renderableData, FScene::LightSoa const& lightData) noexcept;
    void prepareLighting(
            FEngine& engine, FEngine::DriverApi& driver, ArenaScope& arena, Viewport const& viewport) noexcept;
    void froxelize(FEngine& engine) const noexcept;
    void commitUniforms(driver::DriverApi& driverApi) const noexcept;
    void commitFroxels(driver::DriverApi& driverApi) const noexcept;

    bool hasDirectionalLight() const noexcept { return mHasDirectionalLight; }
    bool hasDynamicLighting() const noexcept { return mHasDynamicLighting; }
    bool hasShadowing() const noexcept { return mHasShadowing & mDirectionalShadowMap.hasVisibleShadows(); }

    void prepareVisibleRenderables(utils::JobSystem& js, FScene::RenderableSoa& renderableData) const noexcept;

    void prepareVisibleShadowCasters(utils::JobSystem& js, FScene::RenderableSoa& renderableData,
                                     Frustum const& lightFrustum) const noexcept;

    void updatePrimitivesLod(
            FEngine& engine, const CameraInfo& camera,
            FScene::RenderableSoa& renderableData, Range visibles) noexcept;

    static void cullRenderables(utils::JobSystem& js, FScene::RenderableSoa& renderableData,
                                Frustum const& frustum, size_t bit) noexcept;

    void setShadowsEnabled(bool enabled) noexcept { mShadowingEnabled = enabled; }

    ShadowMap const& getShadowMap() const { return mDirectionalShadowMap; }

    FCamera const* getDirectionalLightCamera() const noexcept {
        return &mDirectionalShadowMap.getDebugCamera();
    }

    void setRenderTarget(TargetBufferFlags discard) noexcept {
        mDiscardedTargetBuffers = discard;
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

    TargetBufferFlags getDiscardedTargetBuffers() const noexcept { return mDiscardedTargetBuffers; }

    bool hasPostProcessPass() const noexcept {
        return mHasPostProcessPass;
    }

    math::float2 updateScale(std::chrono::duration<float, std::milli> frameTime) noexcept;

    void setDynamicResolutionOptions(View::DynamicResolutionOptions const& options) noexcept;

    DynamicResolutionOptions getDynamicResolutionOptions() const noexcept {
        return mDynamicResolution;
    }

    void setDynamicLightingOptions(float zLightNear, float zLightFar) noexcept;

    void setPostProcessingEnabled(bool enabled) noexcept {
        mHasPostProcessPass = enabled;
    }

    void setDepthPrepass(DepthPrepass prepass) noexcept {
        mDepthPrepass = prepass;
    }

    DepthPrepass getDepthPrepass() const noexcept {
        return mDepthPrepass;
    }

    Range const& getVisibleRenderables() const noexcept {
        return mVisibleRenderables;
    }

    Range const& getVisibleShadowCasters() const noexcept {
        return mVisibleShadowCasters;
    }

    FCamera& getCameraUser() noexcept { return *mCullingCamera; }
    void setCameraUser(FCamera* camera) noexcept { setCullingCamera(camera); }

private:
    void prepareVisibleLights(
            FLightManager& lcm, utils::JobSystem& js, FScene::LightSoa& lightData) const;

    void computeVisibilityMasks(
            uint8_t visibleLayers, uint8_t const* layers,
            FRenderableManager::Visibility const* visibility, uint8_t* visibleMask,
            size_t count) const;

    void bindPerViewUniformsAndSamplers(FEngine::DriverApi& driver) const noexcept {
        driver.bindUniformBuffer(BindingPoints::PER_VIEW, getUbh());
        driver.bindSamplers(BindingPoints::PER_VIEW, getUsh());
    }

    // we don't inline this one, because the function is quite large and there is not much to
    // gain from inlining.
    static FScene::RenderableSoa::iterator partition(
            FScene::RenderableSoa::iterator begin, FScene::RenderableSoa::iterator end, uint8_t mask) noexcept;


    // these are accessed in the render loop, keep together
    Handle<HwSamplerBuffer> mPerViewSbh;
    Handle<HwUniformBuffer> mPerViewUbh;

    UniformBuffer& getUb() const noexcept { return mPerViewUb; }
    Handle<HwUniformBuffer> getUbh() const noexcept { return mPerViewUbh; }

    SamplerBuffer& getUs() const noexcept { return mPerViewSb; }
    Handle<HwSamplerBuffer> getUsh() const noexcept { return mPerViewSbh; }

    FScene* mScene = nullptr;
    FCamera* mCullingCamera = nullptr;
    FCamera* mViewingCamera = nullptr;

    CameraInfo mViewingCameraInfo;
    Frustum mCullingFrustum;

    mutable Froxelizer mFroxelizer;

    Viewport mViewport;
    LinearColorA mClearColor;
    bool mCulling = true;
    bool mClearTargetColor = true;
    bool mClearTargetDepth = true;
    bool mClearTargetStencil = false;
    TargetBufferFlags mDiscardedTargetBuffers = TargetBufferFlags::ALL;
    uint8_t mVisibleLayers = 0x1;
    uint8_t mSampleCount = 1;
    AntiAliasing mAntiAliasing = AntiAliasing::FXAA;
    bool mShadowingEnabled = true;
    bool mHasPostProcessPass = true;
    DepthPrepass mDepthPrepass = DepthPrepass::DEFAULT;

    using duration = std::chrono::duration<float, std::milli>;
    DynamicResolutionOptions mDynamicResolution;
    std::deque<duration> mFrameTimeHistory;

    math::float2 mScale = 1.0f;
    float mDynamicWorkloadScale = 1.0f;
    bool mIsDynamicResolutionSupported = false;

    mutable UniformBuffer mPerViewUb;
    mutable SamplerBuffer mPerViewSb;

    utils::CString mName;
    const bool mClipSpace01;

    // the following values are set by prepare()
    Range mVisibleRenderables;
    Range mVisibleShadowCasters;
    mutable bool mHasDirectionalLight = false;
    mutable bool mHasDynamicLighting = false;
    mutable bool mHasShadowing = false;
    mutable ShadowMap mDirectionalShadowMap;
};

FILAMENT_UPCAST(View)

} // namespace details
} // namespace filament

#endif // TNT_FILAMENT_DETAILS_VIEW_H
