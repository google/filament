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

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <math/vec3.h>

#include <vector>

namespace filament {

class FView;

namespace details {

class ShadowMap;
class RenderPass;

class ShadowMapManager {
public:

    ShadowMapManager();
    ~ShadowMapManager();

    void terminate(backend::DriverApi& driverApi) noexcept;

    // Reset shadow map layout.
    void reset() noexcept;

    void setDirectionalShadowMap(ShadowMap& shadowMap, size_t lightIndex) noexcept;
    void addSpotShadowMap(ShadowMap& shadowMap, size_t lightIndex) noexcept;

    // Allocates shadow texture based on the shadows maps and their requirements.
    void prepare(FEngine& engine, backend::DriverApi& driver, backend::SamplerGroup& samplerGroup,
             FScene::LightSoa& lightData) noexcept;

    // Updates all of the shadow maps and performs culling.
    // Returns true if any of the shadow maps have visible shadows.
    bool update(FEngine& engine, FView& view, UniformBuffer& perViewUb, UniformBuffer& shadowUb,
            FScene::RenderableSoa& renderableData, FScene::LightSoa& lightData) noexcept;

    // Renders all of the shadow maps.
    void render(FEngine& engine, FView& view, backend::DriverApi& driver, RenderPass& pass) noexcept;

private:

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

    // 16-bits seems enough.
    // TODO: make it an option.
    // TODO: iOS does not support the DEPTH16 texture format.
    backend::TextureFormat mTextureFormat = backend::TextureFormat::DEPTH16;
    float mTextureZResolution = 1.0f / (1u << 16u);

    backend::Handle<backend::HwTexture> mShadowMapTexture;
    std::vector<backend::Handle<backend::HwRenderTarget>> mRenderTargets;
    ShadowMapEntry mDirectionalShadowMap;
    std::vector<ShadowMapEntry> mSpotShadowMaps;
};

}
}

#endif //TNT_FILAMENT_DETAILS_SHADOWMAPMANAGER_H
