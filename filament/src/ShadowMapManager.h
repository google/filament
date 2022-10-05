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

#include <private/backend/DriverApi.h>
#include "backend/DriverApiForward.h"

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
    std::array<math::mat4f, CONFIG_MAX_SHADOW_CASCADES> lightFromWorldMatrix;
    math::float4 cascadeSplits;
    float shadowBulbRadiusLs;
    float shadowBias;
    float ssContactShadowDistance;
    uint32_t directionalShadows;
    uint32_t cascades;
    bool elvsm;
};

class ShadowMapManager {
public:

    using ShadowMappingUniforms = filament::ShadowMappingUniforms;

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

    void setShadowCascades(size_t lightIndex, LightManager::ShadowOptions const* options) noexcept;
    void addSpotShadowMap(size_t lightIndex, LightManager::ShadowOptions const* options) noexcept;

    // Updates all of the shadow maps and performs culling.
    // Returns true if any of the shadow maps have visible shadows.
    ShadowMapManager::ShadowTechnique update(FEngine& engine, FView& view,
            CameraInfo const& cameraInfo,
            FScene::RenderableSoa& renderableData, FScene::LightSoa& lightData) noexcept;

    // Renders all the shadow maps.
    FrameGraphId<FrameGraphTexture> render(FrameGraph& fg, FEngine& engine,
            RenderPass const& pass, FView& view) noexcept;

    ShadowMap* getCascadeShadowMap(size_t cascade) noexcept {
        assert_invariant(cascade < CONFIG_MAX_SHADOW_CASCADES);
        return std::launder(reinterpret_cast<ShadowMap*>(&mShadowMapCache[cascade]));
    }

    ShadowMap const* getCascadeShadowMap(size_t cascade) const noexcept {
        return const_cast<ShadowMapManager*>(this)->getCascadeShadowMap(cascade);
    }

    ShadowMap* getSpotShadowMap(size_t spot) noexcept {
        assert_invariant(spot < CONFIG_MAX_SHADOW_CASTING_SPOTS);
        return std::launder(reinterpret_cast<ShadowMap*>(
                &mShadowMapCache[CONFIG_MAX_SHADOW_CASCADES + spot]));
    }

    ShadowMap const* getSpotShadowMap(size_t spot) const noexcept {
        return const_cast<ShadowMapManager*>(this)->getSpotShadowMap(spot);
    }

    // valid after calling update() above
    ShadowMappingUniforms getShadowMappingUniforms() const noexcept {
        return mShadowMappingUniforms;
    }

    auto& getShadowUniforms() const { return mShadowUb; }

    auto& getShadowUniformsHandle() const { return mShadowUbh; }

private:
    ShadowMapManager::ShadowTechnique updateCascadeShadowMaps(FEngine& engine,
            FView& view, CameraInfo const& cameraInfo, FScene::RenderableSoa& renderableData,
            FScene::LightSoa& lightData, ShadowMap::SceneInfo& sceneInfo) noexcept;

    ShadowMapManager::ShadowTechnique updateSpotShadowMaps(FEngine& engine,
            FView& view, CameraInfo const& cameraInfo, FScene::RenderableSoa& renderableData,
            FScene::LightSoa& lightData, ShadowMap::SceneInfo& sceneInfo) noexcept;

    void calculateTextureRequirements(FEngine& engine, FView& view, FScene::LightSoa& lightData) noexcept;

    class ShadowMapEntry {
    public:
        ShadowMapEntry() = default;
        ShadowMapEntry(ShadowMap* shadowMap, size_t light,
                LightManager::ShadowOptions const* options) :
                mShadowMap(shadowMap), mOptions(options), mLightIndex(light) {
        }

        explicit operator bool() const { return mShadowMap != nullptr; }

        void setLayer(uint8_t layer) noexcept { mLayer = layer; }
        uint8_t getLayer() const noexcept { return mLayer; }

        LightManager::ShadowOptions const* getShadowOptions() const noexcept { return mOptions; }
        ShadowMap& getShadowMap() const { return *mShadowMap; }
        size_t getLightIndex() const { return mLightIndex; }

        bool hasVisibleShadows() const { return mShadowMap->hasVisibleShadows(); }

    private:
        ShadowMap* mShadowMap = nullptr;
        LightManager::ShadowOptions const* mOptions = nullptr;
        uint32_t mLightIndex = 0;
        uint8_t mLayer = 0;
    };

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

    // Atlas requirements, updated in ShadowMapManager::update(),
    // consumed in ShadowMapManager::render()
    struct TextureAtlasRequirements {
        uint16_t size = 0;
        uint8_t layers = 0;
        uint8_t levels = 0;
        backend::TextureFormat format = backend::TextureFormat::DEPTH16;
    } mTextureAtlasRequirements;

    SoftShadowOptions mSoftShadowOptions;

    CascadeSplits::Params mCascadeSplitParams;
    CascadeSplits mCascadeSplits;

    mutable TypedUniformBuffer<ShadowUib> mShadowUb;
    backend::Handle<backend::HwBufferObject> mShadowUbh;

    ShadowMappingUniforms mShadowMappingUniforms;

    utils::FixedCapacityVector<ShadowMapEntry> mCascadeShadowMaps{
            utils::FixedCapacityVector<ShadowMapEntry>::with_capacity(
                    CONFIG_MAX_SHADOW_CASCADES) };

    utils::FixedCapacityVector<ShadowMapEntry> mSpotShadowMaps{
            utils::FixedCapacityVector<ShadowMapEntry>::with_capacity(
                    CONFIG_MAX_SHADOW_CASTING_SPOTS) };

    // inline storage for all our ShadowMap objects, we can't easily use a std::array<> directly.
    // because ShadowMap doesn't have a default ctor, and we avoid out-of-line allocations.
    // Each ShadowMap is currently 128 bytes.
    using ShadowMapStorage = std::aligned_storage<sizeof(ShadowMap), alignof(ShadowMap)>::type;
    std::array<ShadowMapStorage,
            CONFIG_MAX_SHADOW_CASCADES + CONFIG_MAX_SHADOW_CASTING_SPOTS> mShadowMapCache;
};

} // namespace filament

template<> struct utils::EnableBitMaskOperators<filament::ShadowMapManager::ShadowTechnique>
        : public std::true_type {};

#endif //TNT_FILAMENT_DETAILS_SHADOWMAPMANAGER_H
