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

#include "AtlasAllocator.h"
#include "ShadowMap.h"
#include "ds/TypedBuffer.h"

#include <filament/LightManager.h>
#include <filament/Options.h>
#include <filament/Viewport.h>

#include <private/filament/EngineEnums.h>
#include <private/filament/UibStructs.h>

#include "components/RenderableManager.h"

#include "details/Engine.h"
#include "details/Scene.h"

#include "fg/FrameGraphId.h"
#include "fg/FrameGraphTexture.h"

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <utils/BitmaskEnum.h>
#include <utils/compiler.h>
#include <utils/FixedCapacityVector.h>
#include <utils/debug.h>
#include <utils/Range.h>
#include <utils/Slice.h>

#include <math/mat4.h>
#include <math/vec4.h>

#include <array>
#include <memory>
#include <new>
#include <type_traits>
#include <vector>

#include <stdint.h>
#include <stddef.h>

namespace filament {

class Camera;
class FCamera;
class FView;
class FrameGraph;
class RenderPass;
class RenderPassBuilder;

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

    class Builder {
        friend class ShadowMapManager;
        uint32_t mDirectionalShadowMapCount = 0;
        uint32_t mSpotShadowMapCount = 0;
        struct ShadowMap {
            size_t lightIndex;
            ShadowType shadowType;
            uint16_t shadowIndex;
            uint8_t face;
            LightManager::ShadowOptions const* options;
        };
        std::vector<ShadowMap> mShadowMaps;
    public:
        Builder& directionalShadowMap(size_t lightIndex,
                LightManager::ShadowOptions const* options) noexcept;

        Builder& shadowMap(size_t lightIndex, bool spotlight,
                LightManager::ShadowOptions const* options) noexcept;

        bool hasShadowMaps() const noexcept {
            return mDirectionalShadowMapCount || mSpotShadowMapCount;
        }
    };

    ~ShadowMapManager();

    static void createIfNeeded(FEngine& engine,
            std::unique_ptr<ShadowMapManager>& inOutShadowMapManager);

    static void terminate(FEngine& engine,
            std::unique_ptr<ShadowMapManager>& shadowMapManager);

    size_t getMaxShadowMapCount() const noexcept;

    // Updates all the shadow maps and performs culling.
    // Returns true if any of the shadow maps have visible shadows.
    ShadowTechnique update(Builder const& builder,
            FEngine& engine, FView& view,
            CameraInfo const& cameraInfo,
            FScene::RenderableSoa& renderableData, FScene::LightSoa const& lightData) noexcept;

    // Renders all the shadow maps.
    FrameGraphId<FrameGraphTexture> render(FEngine& engine, FrameGraph& fg,
            RenderPassBuilder const& passBuilder,
            FView& view, CameraInfo const& mainCameraInfo, math::float4 const& userTime) noexcept;

    // valid after calling update() above
    ShadowMappingUniforms getShadowMappingUniforms() const noexcept {
        return mShadowMappingUniforms;
    }

    auto& getShadowUniformsHandle() const { return mShadowUbh; }

    bool hasSpotShadows() const { return mSpotShadowMapCount > 0; }

    // for debugging only
    utils::FixedCapacityVector<Camera const*> getDirectionalShadowCameras() const noexcept;

private:
    explicit ShadowMapManager(FEngine& engine);

    void terminate(FEngine& engine);

    static void updateNearFarPlanes(math::mat4f* projection,
            float nearDistance, float farDistance) noexcept;

    ShadowTechnique updateCascadeShadowMaps(FEngine& engine,
            FView& view, CameraInfo cameraInfo, FScene::RenderableSoa& renderableData,
            FScene::LightSoa const& lightData, ShadowMap::SceneInfo sceneInfo) noexcept;

    ShadowTechnique updateSpotShadowMaps(FEngine& engine,
            FScene::LightSoa const& lightData) const noexcept;

    void calculateTextureRequirements(FEngine&, FView& view,
            FScene::LightSoa const&) noexcept;

    void prepareSpotShadowMap(ShadowMap& shadowMap,
            FEngine& engine, FView& view, CameraInfo const& mainCameraInfo,
            FScene::LightSoa const& lightData, ShadowMap::SceneInfo const& sceneInfo) noexcept;

    static void cullSpotShadowMap(ShadowMap const& map,
            FEngine const& engine, FView const& view,
            FScene::RenderableSoa& renderableData, utils::Range<uint32_t> range,
            FScene::LightSoa const& lightData) noexcept;

    void preparePointShadowMap(ShadowMap& map,
            FEngine& engine, FView& view, CameraInfo const& mainCameraInfo,
            FScene::LightSoa const& lightData) const noexcept;

    static void cullPointShadowMap(ShadowMap const& shadowMap, FView const& view,
            FScene::RenderableSoa& renderableData, utils::Range<uint32_t> range,
            FScene::LightSoa const& lightData) noexcept;

    static void updateSpotVisibilityMasks(
            uint8_t visibleLayers,
            uint8_t const* UTILS_RESTRICT layers,
            FRenderableManager::Visibility const* UTILS_RESTRICT visibility,
            Culler::result_type* UTILS_RESTRICT visibleMask, size_t count);

    class CascadeSplits {
    public:
        constexpr static size_t SPLIT_COUNT = CONFIG_MAX_SHADOW_CASCADES + 1;

        struct Params {
            float near = 0.0f;
            float far = 0.0f;
            uint32_t cascadeCount = 1;
            std::array<float, SPLIT_COUNT> splitPositions = { 0.0f };
        };

        explicit CascadeSplits(Params const& params) noexcept;

        // Split positions in world-space.
        const float* begin() const { return mSplitsWs; }
        const float* end() const { return mSplitsWs + mSplitCount; }

    private:
        float mSplitsWs[SPLIT_COUNT];
        uint32_t mSplitCount;
    };

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

    mutable TypedBuffer<ShadowUib> mShadowUb;
    backend::Handle<backend::HwBufferObject> mShadowUbh;

    ShadowMappingUniforms mShadowMappingUniforms = {};

    ShadowMap::SceneInfo mSceneInfo;

    // Inline storage for all our ShadowMap objects, we can't easily use a std::array<> directly.
    // Because ShadowMap doesn't have a default ctor, and we avoid out-of-line allocations.
    // Each ShadowMap is currently 88 bytes (total of ~12KB for 128 shadow maps)
    using ShadowMapStorage = std::aligned_storage_t<sizeof(ShadowMap), alignof(ShadowMap)>;
    using ShadowMapCacheContainer = std::array<ShadowMapStorage, CONFIG_MAX_SHADOWMAPS>;
    ShadowMapCacheContainer mShadowMapCache;
    uint32_t mDirectionalShadowMapCount = 0;
    uint32_t mSpotShadowMapCount = 0;
    bool const mIsDepthClampSupported;
    bool mInitialized = false;
    bool mFeatureShadowAllocator = false;

    ShadowMap& getShadowMap(size_t const index) noexcept {
        assert_invariant(index < mShadowMapCache.size());
        return *std::launder(reinterpret_cast<ShadowMap*>(&mShadowMapCache[index]));
    }

    ShadowMap const& getShadowMap(size_t const index) const noexcept {
        return const_cast<ShadowMapManager*>(this)->getShadowMap(index);
    }

    utils::Slice<ShadowMap> getCascadedShadowMap() noexcept {
        ShadowMap const* const p = &getShadowMap(0);
        return { p, mDirectionalShadowMapCount };
    }

    utils::Slice<ShadowMap> getCascadedShadowMap() const noexcept {
        return const_cast<ShadowMapManager*>(this)->getCascadedShadowMap();
    }

    utils::Slice<ShadowMap> getSpotShadowMaps() const noexcept {
        ShadowMap const* const p = &getShadowMap(CONFIG_MAX_SHADOW_CASCADES);
        return { p, mSpotShadowMapCount };
    }
};

} // namespace filament

template<> struct utils::EnableBitMaskOperators<filament::ShadowMapManager::ShadowTechnique>
        : public std::true_type {};

#endif //TNT_FILAMENT_DETAILS_SHADOWMAPMANAGER_H
