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

#include <private/backend/DriverApi.h>
#include <private/backend/DriverApiForward.h>
#include <private/backend/SamplerGroup.h>
#include <private/backend/SamplerGroup.h>

#include <private/filament/EngineEnums.h>

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <math/vec3.h>

#include <array>
#include <memory>
#include <vector>

namespace filament {

class FView;

namespace details {

class ShadowMap;
class RenderPass;

class ShadowMapManager {
public:

    ShadowMapManager(FEngine& engine);
    ~ShadowMapManager();

    void terminate(backend::DriverApi& driverApi) noexcept;

    // Reset shadow map layout.
    void reset() noexcept;

    void setShadowCascades(size_t lightIndex, size_t cascades) noexcept;
    void addSpotShadowMap(size_t lightIndex) noexcept;

    // Allocates shadow texture based on the shadows maps and their requirements.
    void prepare(FEngine& engine, backend::DriverApi& driver, backend::SamplerGroup& samplerGroup,
             FScene::LightSoa& lightData) noexcept;

    // Updates all of the shadow maps and performs culling.
    // Returns true if any of the shadow maps have visible shadows.
    bool update(FEngine& engine, FView& view, UniformBuffer& perViewUb, UniformBuffer& shadowUb,
            FScene::RenderableSoa& renderableData, FScene::LightSoa& lightData) noexcept;

    // Renders all of the shadow maps.
    void render(FEngine& engine, FView& view, backend::DriverApi& driver, RenderPass& pass) noexcept;

    const ShadowMap* getCascadeShadowMap(size_t c) const noexcept {
        return mCascadeShadowMapCache[c].get();
    }

private:

    bool updateCascadeShadowMaps(FEngine& engine, FView& view, UniformBuffer& perViewUb,
            FScene::RenderableSoa& renderableData, FScene::LightSoa& lightData) noexcept;
    bool updateSpotShadowMaps(FEngine& engine, FView& view, UniformBuffer& shadowUb,
            FScene::RenderableSoa& renderableData, FScene::LightSoa& lightData) noexcept;
    void fillWithDebugPattern(backend::DriverApi& driverApi,
            backend::Handle<backend::HwTexture> texture) const noexcept;
    void destroyResources(backend::DriverApi& driver) noexcept;

    struct ShadowLayout {
        uint8_t layer = 0;
        uint32_t size = 0;
    };

    class ShadowMapEntry {
    public:
        ShadowMapEntry() = default;
        ShadowMapEntry(ShadowMap* shadowMap, const size_t light) :
                mShadowMap(shadowMap),
                mLightIndex(light),
                mLayout({}) {}

        explicit operator bool() const { return mShadowMap != nullptr; }

        ShadowMap* getShadowMap() const { return mShadowMap; }
        size_t getLightIndex() const { return mLightIndex; }
        const ShadowLayout& getLayout() const { return mLayout; }
        bool hasVisibleShadows() const { return mHasVisibleShadows; }

        void setHasVisibleShadows(bool hasVisibleShadows) { mHasVisibleShadows = hasVisibleShadows; }
        void setLayout(const ShadowLayout& layout) { mLayout = layout; }

    private:
        ShadowMap* mShadowMap = nullptr;
        size_t mLightIndex = 0;
        ShadowLayout mLayout = {};
        bool mHasVisibleShadows = false;
    };

    struct TextureState {
        uint16_t size;
        uint8_t layers;

        TextureState(uint16_t size, uint8_t layers) : size(size), layers(layers) {}

        bool operator==(const TextureState& rhs) const {
            return size == rhs.size &&
                   layers == rhs.layers;
        }
    } mTextureState;

    class CascadeSplits {
    public:
        struct Params {
            math::mat4f proj = {};
            float near = 0.0f;
            float far = 0.0f;
            size_t cascadeCount = 1;

            bool operator!=(const Params& rhs) const {
                return proj != rhs.proj ||
                       near != rhs.near ||
                       far != rhs.far ||
                       cascadeCount != rhs.cascadeCount;
            }
        };

        CascadeSplits() : CascadeSplits(Params {}) {}
        CascadeSplits(Params p);

        const float* beginWs() const { return mSplitsWs; }
        const float* endWs() const { return mSplitsWs + mSplitCount; }
        const float* beginCs() const { return mSplitsCs; }
        const float* endCs() const { return mSplitsCs + mSplitCount; }

    private:
        float mSplitsWs[CONFIG_MAX_SHADOW_CASCADES + 1];
        float mSplitsCs[CONFIG_MAX_SHADOW_CASCADES + 1];
        size_t mSplitCount;

    } mCascadeSplits;
    CascadeSplits::Params mCascadeSplitParams;

    // 16-bits seems enough.
    // TODO: make it an option.
    // TODO: iOS does not support the DEPTH16 texture format.
    backend::TextureFormat mTextureFormat = backend::TextureFormat::DEPTH16;
    float mTextureZResolution = 1.0f / (1u << 16u);

    backend::Handle<backend::HwTexture> mShadowMapTexture;
    std::vector<backend::Handle<backend::HwRenderTarget>> mRenderTargets;
    std::vector<ShadowMapEntry> mCascadeShadowMaps;
    std::vector<ShadowMapEntry> mSpotShadowMaps;

    std::array<std::unique_ptr<ShadowMap>, CONFIG_MAX_SHADOW_CASCADES> mCascadeShadowMapCache;
    std::array<std::unique_ptr<ShadowMap>, CONFIG_MAX_SHADOW_CASTING_SPOTS> mSpotShadowMapCache;
};

}
}

#endif //TNT_FILAMENT_DETAILS_SHADOWMAPMANAGER_H
