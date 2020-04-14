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

#include "FrameInfo.h"
#include "UniformBuffer.h"

#include "details/Allocators.h"
#include "details/Camera.h"
#include "details/Froxelizer.h"
#include "details/RenderTarget.h"
#include "details/ShadowMap.h"
#include "details/ShadowMapManager.h"
#include "details/Scene.h"

#include <private/filament/EngineEnums.h>

#include "private/backend/DriverApi.h"

#include <backend/Handle.h>

#include <utils/compiler.h>
#include <utils/Allocator.h>
#include <utils/StructureOfArrays.h>
#include <utils/Slice.h>
#include <utils/Range.h>

#include <math/scalar.h>

#include <array>
#include <memory>

namespace utils {
class JobSystem;
} // namespace utils;

namespace filament {
namespace details {

class FEngine;
class FMaterialInstance;
class FRenderer;
class FScene;

// The value of the 'VISIBLE_MASK' after culling. Each bit represents visibility in a frustum
// (either camera or light).
//
// bits                          7 6 5 4 3 2 1 0
// +-------------------------------------------+
// VISIBLE_RENDERABLE                          X
// VISIBLE_DIR_SHADOW_CASTER                 X
// VISIBLE_SPOT_SHADOW_CASTER_0            X
// VISIBLE_SPOT_SHADOW_CASTER_1          X
// ...

static constexpr size_t VISIBLE_RENDERABLE_BIT = 0u;
static constexpr size_t VISIBLE_DIR_SHADOW_CASTER_BIT = 1u;
static constexpr size_t VISIBLE_SPOT_SHADOW_CASTER_N_BIT(size_t n) { return n + 2; }

static constexpr uint8_t VISIBLE_RENDERABLE = 1u << VISIBLE_RENDERABLE_BIT;
static constexpr uint8_t VISIBLE_DIR_SHADOW_CASTER = 1u << VISIBLE_DIR_SHADOW_CASTER_BIT;
static constexpr uint8_t VISIBLE_SPOT_SHADOW_CASTER_N(size_t n) {
    return 1u << VISIBLE_SPOT_SHADOW_CASTER_N_BIT(n);
}

// ORing of all the VISIBLE_SPOT_SHADOW_CASTER bits
static constexpr uint8_t VISIBLE_SPOT_SHADOW_CASTER =
        (0xFF >> (sizeof(uint8_t) * 8u - CONFIG_MAX_SHADOW_CASTING_SPOTS)) << 2u;

// Because we're using a uint8_t for the visibility mask, we're limited to 6 spot light shadows.
// (2 of the bits are used for visible renderables + directional light shadow casters).
static_assert(CONFIG_MAX_SHADOW_CASTING_SPOTS <= 6,
        "CONFIG_MAX_SHADOW_CASTING_SPOTS cannot be higher than 6.");

// ------------------------------------------------------------------------------------------------

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
        return mName.c_str();
    }

    void prepareCamera(const CameraInfo& camera) const noexcept;
    void prepareViewport(const Viewport& viewport) const noexcept;
    void prepareShadowing(FEngine& engine, backend::DriverApi& driver,
            FScene::RenderableSoa& renderableData, FScene::LightSoa& lightData) noexcept;
    void prepareLighting(FEngine& engine, FEngine::DriverApi& driver,
            ArenaScope& arena, Viewport const& viewport) noexcept;
    void prepareSSAO(backend::Handle<backend::HwTexture> ssao) const noexcept;
    void prepareSSR(backend::Handle<backend::HwTexture> ssr, float refractionLodOffset) const noexcept;
    void prepareStructure(backend::Handle<backend::HwTexture> structure) const noexcept;
    void cleanupRenderPasses() const noexcept;
    void froxelize(FEngine& engine) const noexcept;
    void commitUniforms(backend::DriverApi& driver) const noexcept;
    void commitFroxels(backend::DriverApi& driverApi) const noexcept;

    bool hasDirectionalLight() const noexcept { return mHasDirectionalLight; }
    bool hasDynamicLighting() const noexcept { return mHasDynamicLighting; }
    bool hasShadowing() const noexcept { return mHasShadowing; }
    bool hasFog() const noexcept { return mFogOptions.enabled && mFogOptions.density > 0.0f; }

    void renderShadowMaps(FEngine& engine, FEngine::DriverApi& driver, RenderPass& pass) noexcept;

    void updatePrimitivesLod(
            FEngine& engine, const CameraInfo& camera,
            FScene::RenderableSoa& renderableData, Range visible) noexcept;

    void setShadowsEnabled(bool enabled) noexcept { mShadowingEnabled = enabled; }

    FCamera const* getDirectionalLightCamera() const noexcept {
        return &mDirectionalShadowMap.getDebugCamera();
    }

    void setRenderTarget(FRenderTarget* renderTarget) noexcept {
        mRenderTarget = renderTarget;
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

    bool hasPostProcessPass() const noexcept {
        return mHasPostProcessPass;
    }

    math::float2 updateScale(FrameInfo const& info) noexcept;

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
        mAmbientOcclusion = ambientOcclusion;
    }

    AmbientOcclusion getAmbientOcclusion() const noexcept {
        return mAmbientOcclusion;
    }

    void setAmbientOcclusionOptions(AmbientOcclusionOptions options) noexcept {
        options.radius = math::max(0.0f, options.radius);
        options.bias = math::clamp(0.0f, 0.1f, options.bias);
        options.power = std::max(0.0f, options.power);
        options.resolution = math::clamp(0.0f, 1.0f, options.resolution);
        options.intensity = std::max(0.0f, options.intensity);
        mAmbientOcclusionOptions = options;
    }

    AmbientOcclusionOptions const& getAmbientOcclusionOptions() const noexcept {
        return mAmbientOcclusionOptions;
    }

    void setBloomOptions(BloomOptions options) noexcept {
        options.dirtStrength = math::saturate(options.dirtStrength);
        options.levels = math::clamp(options.levels, uint8_t(3), uint8_t(12));
        mBloomOptions = options;
    }

    void setFogOptions(FogOptions options) noexcept {
        options.distance = std::max(0.0f, options.distance);
        options.maximumOpacity = math::clamp(options.maximumOpacity, 0.0f, 1.0f);
        options.density = std::max(0.0f, options.density);
        options.heightFalloff = std::max(0.0f, options.heightFalloff);
        options.inScatteringSize = options.inScatteringSize;
        options.inScatteringStart = std::max(0.0f, options.inScatteringStart);
        mFogOptions = options;
    }

    BloomOptions getBloomOptions() const noexcept {
        return mBloomOptions;
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

    static void cullRenderables(utils::JobSystem& js, FScene::RenderableSoa& renderableData,
            Frustum const& frustum, size_t bit) noexcept;

    UniformBuffer& getViewUniforms() const { return mPerViewUb; }
    backend::SamplerGroup& getViewSamplers() const { return mPerViewSb; }
    UniformBuffer& getShadowUniforms() const { return mShadowUb; }

private:
    void prepareVisibleRenderables(utils::JobSystem& js,
            Frustum const& frustum, FScene::RenderableSoa& renderableData) const noexcept;

    static void prepareVisibleLights(
            FLightManager const& lcm, utils::JobSystem& js, Frustum const& frustum,
            FScene::LightSoa& lightData) noexcept;

    void computeVisibilityMasks(
            uint8_t visibleLayers, uint8_t const* layers,
            FRenderableManager::Visibility const* visibility, uint8_t* visibleMask,
            size_t count) const;

    void bindPerViewUniformsAndSamplers(FEngine::DriverApi& driver) const noexcept {
        driver.bindUniformBuffer(BindingPoints::PER_VIEW, mPerViewUbh);
        driver.bindUniformBuffer(BindingPoints::LIGHTS, mLightUbh);
        driver.bindUniformBuffer(BindingPoints::SHADOW, mShadowUbh);
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
    backend::Handle<backend::HwUniformBuffer> mShadowUbh;
    backend::Handle<backend::HwUniformBuffer> mRenderableUbh;

    FScene* mScene = nullptr;
    FCamera* mCullingCamera = nullptr;
    FCamera* mViewingCamera = nullptr;

    CameraInfo mViewingCameraInfo;
    Frustum mCullingFrustum{};

    mutable Froxelizer mFroxelizer;

    Viewport mViewport;
    bool mCulling = true;
    bool mFrontFaceWindingInverted = false;

    FRenderTarget* mRenderTarget = nullptr;

    uint8_t mVisibleLayers = 0x1;
    uint8_t mSampleCount = 1;
    AntiAliasing mAntiAliasing = AntiAliasing::FXAA;
    ToneMapping mToneMapping = ToneMapping::ACES;
    Dithering mDithering = Dithering::TEMPORAL;
    bool mShadowingEnabled = true;
    bool mHasPostProcessPass = true;
    AmbientOcclusion mAmbientOcclusion = AmbientOcclusion::NONE;
    AmbientOcclusionOptions mAmbientOcclusionOptions{};
    BloomOptions mBloomOptions;
    FogOptions mFogOptions;
    BlendMode mBlendMode = BlendMode::OPAQUE;

    DynamicResolutionOptions mDynamicResolution;
    math::float2 mScale = 1.0f;
    bool mIsDynamicResolutionSupported = false;

    RenderQuality mRenderQuality;

    mutable UniformBuffer mPerViewUb;
    mutable UniformBuffer mShadowUb;
    mutable backend::SamplerGroup mPerViewSb;

    utils::CString mName;

    // the following values are set by prepare()
    Range mVisibleRenderables;
    Range mVisibleDirectionalShadowCasters;
    Range mSpotLightShadowCasters;
    uint32_t mRenderableUBOSize = 0;
    mutable bool mHasDirectionalLight = false;
    mutable bool mHasDynamicLighting = false;
    mutable bool mHasShadowing = false;
    ShadowMapManager mShadowMapManager;
    mutable ShadowMap mDirectionalShadowMap;
    mutable std::array<std::unique_ptr<ShadowMap>, CONFIG_MAX_SHADOW_CASTING_SPOTS> mSpotShadowMap;
};

FILAMENT_UPCAST(View)

} // namespace details
} // namespace filament

#endif // TNT_FILAMENT_DETAILS_VIEW_H
