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

#ifndef TNT_FILAMENT_DETAILS_SHADOWMAPMANAGER_H
#define TNT_FILAMENT_DETAILS_SHADOWMAPMANAGER_H

#include <filament/Viewport.h>

#include "ShadowMap.h"
#include "TypedUniformBuffer.h"

#include "details/Engine.h"
#include "details/Scene.h"

#include <private/filament/EngineEnums.h>

#include <backend/DriverApiForward.h>
#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <utils/FixedCapacityVector.h>

#include <math/vec3.h>

#include <array>
#include <memory>

namespace filament {

class FView;
class FrameGraph;
class RenderPass;

struct ShadowMappingUniforms {
    math::float4 cascadeSplits;
    float ssContactShadowDistance;
    uint32_t directionalShadows;
    uint32_t cascades;
};

class ShadowMapManager {
public:

    using ShadowMappingUniforms = ShadowMappingUniforms;

    using ShadowType = ShadowMap::ShadowType;

    enum class ShadowTechnique : uint8_t {
        NONE = 0x0u,
        SHADOW_MAP = 0x1u,
        SCREEN_SPACE = 0x2u,
    };


    explicit ShadowMapManager(FEngine& engine);
    ~ShadowMapManager();

    void terminate(FEngine& engine);

    // Reset shadow map layout.
    void reset() noexcept;

    void setDirectionalShadowMap(size_t lightIndex,
            LightManager::ShadowOptions const* options) noexcept;

    void addShadowMap(size_t lightIndex, bool spotlight,
            LightManager::ShadowOptions const* options) noexcept;

    // Updates all the shadow maps and performs culling.
    // Returns true if any of the shadow maps have visible shadows.
    ShadowMapManager::ShadowTechnique update(FEngine& engine, FView& view,
            CameraInfo const& cameraInfo,
            FScene::RenderableSoa& renderableData, FScene::LightSoa const& lightData) noexcept;

    // Renders all the shadow maps.
    FrameGraphId<FrameGraphTexture> render(FEngine& engine, FrameGraph& fg, RenderPass const& pass,
            FView& view, CameraInfo const& mainCameraInfo, math::float4 const& userTime) noexcept;

    ShadowMap* getShadowMap(size_t index) noexcept {
        assert_invariant(index < CONFIG_MAX_SHADOWMAPS);
        return std::launder(reinterpret_cast<ShadowMap*>(&mShadowMapCache[index]));
    }

    ShadowMap const* getShadowMap(size_t index) const noexcept {
        return const_cast<ShadowMapManager*>(this)->getShadowMap(index);
    }

    // valid after calling update() above
    ShadowMappingUniforms getShadowMappingUniforms() const noexcept {
        return mShadowMappingUniforms;
    }

    auto& getShadowUniformsHandle() const { return mShadowUbh; }

    bool hasSpotShadows() const { return !mSpotShadowMaps.empty(); }

private:
    ShadowMapManager::ShadowTechnique updateCascadeShadowMaps(FEngine& engine,
            FView& view, CameraInfo const& cameraInfo, FScene::RenderableSoa& renderableData,
            FScene::LightSoa const& lightData, ShadowMap::SceneInfo sceneInfo) noexcept;

    ShadowMapManager::ShadowTechnique updateSpotShadowMaps(FEngine& engine,
            FScene::LightSoa const& lightData) noexcept;

    void calculateTextureRequirements(FEngine&, FView& view,
            FScene::LightSoa const&) noexcept;

    void prepareSpotShadowMap(ShadowMap& shadowMap,
            FEngine& engine, FView& view, CameraInfo const& mainCameraInfo,
            FScene::RenderableSoa& renderableData, utils::Range<uint32_t> range,
            FScene::LightSoa& lightData, ShadowMap::SceneInfo const& sceneInfo) noexcept;

    void preparePointShadowMap(ShadowMap& map,
            FEngine& engine, FView& view, CameraInfo const& mainCameraInfo,
            FScene::RenderableSoa& renderableData, utils::Range<uint32_t> range,
            FScene::LightSoa& lightData,
            ShadowMap::SceneInfo const& sceneInfo) noexcept;

    static void updateSpotVisibilityMasks(
            uint8_t visibleLayers,
            uint8_t const* UTILS_RESTRICT layers,
            FRenderableManager::Visibility const* UTILS_RESTRICT visibility,
            Culler::result_type* UTILS_RESTRICT visibleMask, size_t count);

    class CascadeSplits {
    public:
        constexpr static size_t SPLIT_COUNT = CONFIG_MAX_SHADOW_CASCADES + 1;

        struct Params {
            math::mat4f proj;
            float near = 0.0f;
            float far = 0.0f;
            size_t cascadeCount = 1;
            std::array<float, SPLIT_COUNT> splitPositions = { 0.0f };

            bool operator!=(const Params& rhs) const {
                return proj != rhs.proj ||
                       near != rhs.near ||
                       far != rhs.far ||
                       cascadeCount != rhs.cascadeCount ||
                       splitPositions != rhs.splitPositions;
            }
        };

        CascadeSplits() noexcept : CascadeSplits(Params{}) {}
        explicit CascadeSplits(Params const& params) noexcept;

        // Split positions in world-space.
        const float* beginWs() const { return mSplitsWs; }
        const float* endWs() const { return mSplitsWs + mSplitCount; }

        // Split positions in clip-space.
        const float* beginCs() const { return mSplitsCs; }
        const float* endCs() const { return mSplitsCs + mSplitCount; }

    private:
        float mSplitsWs[SPLIT_COUNT];
        float mSplitsCs[SPLIT_COUNT];
        size_t mSplitCount;
    };

    FEngine& mEngine;

    // Atlas requirements, updated in ShadowMapManager::update(),
    // consumed in ShadowMapManager::render()
    struct TextureAtlasRequirements {
        uint16_t size = 0;
        uint8_t layers = 0;
        uint8_t levels = 0;
        uint8_t msaaSamples = 1;
        backend::TextureFormat format = backend::TextureFormat::DEPTH16;
    } mTextureAtlasRequirements;

    SoftShadowOptions mSoftShadowOptions;

    CascadeSplits::Params mCascadeSplitParams;
    CascadeSplits mCascadeSplits;

    mutable TypedUniformBuffer<ShadowUib> mShadowUb;
    backend::Handle<backend::HwBufferObject> mShadowUbh;

    ShadowMappingUniforms mShadowMappingUniforms = {};

    ShadowMap::SceneInfo mSceneInfo;

    utils::FixedCapacityVector<ShadowMap*> mCascadeShadowMaps{
            utils::FixedCapacityVector<ShadowMap*>::with_capacity(
                    CONFIG_MAX_SHADOW_CASCADES) };

    utils::FixedCapacityVector<ShadowMap*> mSpotShadowMaps{
            utils::FixedCapacityVector<ShadowMap*>::with_capacity(
                    CONFIG_MAX_SHADOWMAPS - CONFIG_MAX_SHADOW_CASCADES) };

    // inline storage for all our ShadowMap objects, we can't easily use a std::array<> directly.
    // because ShadowMap doesn't have a default ctor, and we avoid out-of-line allocations.
    // Each ShadowMap is currently 40 bytes (total of 2.5KB for 64 shadow maps)
    using ShadowMapStorage = std::aligned_storage<sizeof(ShadowMap), alignof(ShadowMap)>::type;
    std::array<ShadowMapStorage, CONFIG_MAX_SHADOWMAPS> mShadowMapCache;
};

} // namespace filament

template<> struct utils::EnableBitMaskOperators<filament::ShadowMapManager::ShadowTechnique>
        : public std::true_type {};

#endif //TNT_FILAMENT_DETAILS_SHADOWMAPMANAGER_H
