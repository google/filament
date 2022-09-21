/*
 * Copyright (C) 2021 The Android Open Source Project
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

#ifndef TNT_FILAMENT_PERVIEWUNIFORMS_H
#define TNT_FILAMENT_PERVIEWUNIFORMS_H

#include <filament/Viewport.h>

#include <private/filament/UibStructs.h>
#include <private/backend/SamplerGroup.h>

#include "TypedUniformBuffer.h"

#include <backend/Handle.h>

#include <utils/EntityInstance.h>

#include <random>

namespace filament {

struct AmbientOcclusionOptions;
struct DynamicResolutionOptions;
struct FogOptions;
struct ScreenSpaceReflectionsOptions;
struct SoftShadowOptions;
struct TemporalAntiAliasingOptions;
struct VsmShadowOptions;

struct CameraInfo;
struct ShadowMappingUniforms;

class FEngine;
class FIndirectLight;
class Froxelizer;
class LightManager;

class PerViewUniforms {

    using LightManagerInstance = utils::EntityInstance<LightManager>;
    using TextureHandle = backend::Handle<backend::HwTexture>;

    static constexpr uint32_t const SHADOW_SAMPLING_RUNTIME_PCF   = 0u;
    static constexpr uint32_t const SHADOW_SAMPLING_RUNTIME_EVSM  = 1u;
    static constexpr uint32_t const SHADOW_SAMPLING_RUNTIME_DPCF  = 2u;
    static constexpr uint32_t const SHADOW_SAMPLING_RUNTIME_PCSS  = 3u;

public:
    explicit PerViewUniforms(FEngine& engine) noexcept;

    void terminate(backend::DriverApi& driver);

    void prepareCamera(const CameraInfo& camera) noexcept;
    void prepareUpscaler(math::float2 scale, DynamicResolutionOptions const& options) noexcept;

    /*
     * @param viewport  viewport (should be same as RenderPassParams::viewport)
     * @param xoffset   horizontal rendering offset *within* the viewport.
     *                  Non-zero when we have guard bands.
     * @param yoffset   vertical rendering offset *within* the viewport.
     *                  Non-zero when we have guard bands.
     */
    void prepareViewport(const filament::Viewport& viewport, uint32_t xoffset, uint32_t yoffset) noexcept;

    void prepareTime(math::float4 const& userTime) noexcept;
    void prepareTemporalNoise(TemporalAntiAliasingOptions const& options) noexcept;
    void prepareExposure(float ev100) noexcept;
    void prepareFog(math::float3 const& cameraPosition, FogOptions const& options) noexcept;
    void prepareStructure(TextureHandle structure) noexcept;
    void prepareSSAO(TextureHandle ssao, AmbientOcclusionOptions const& options) noexcept;
    void prepareBlending(bool needsAlphaChannel) noexcept;

    // screen-space reflection and/or refraction (SSR)
    void prepareSSR(TextureHandle ssr,
            float refractionLodOffset,
            ScreenSpaceReflectionsOptions const& ssrOptions) noexcept;

    void prepareHistorySSR(TextureHandle ssr,
            math::mat4f const& historyProjection,
            math::mat4f const& uvFromViewMatrix,
            ScreenSpaceReflectionsOptions const& ssrOptions) noexcept;

    void prepareShadowMapping(bool highPrecision) noexcept;

    void prepareDirectionalLight(float exposure,
            math::float3 const& sceneSpaceDirection, LightManagerInstance instance) noexcept;

    void prepareAmbientLight(FIndirectLight const& ibl, float intensity, float exposure) noexcept;

    void prepareDynamicLights(Froxelizer& froxelizer) noexcept;

    void prepareShadowVSM(TextureHandle texture,
            ShadowMappingUniforms const& shadowMappingUniforms,
            VsmShadowOptions const& options) noexcept;

    void prepareShadowPCF(TextureHandle texture,
            ShadowMappingUniforms const& shadowMappingUniforms) noexcept;

    void prepareShadowDPCF(TextureHandle texture,
            ShadowMappingUniforms const& shadowMappingUniforms,
            SoftShadowOptions const& options) noexcept;

    void prepareShadowPCSS(TextureHandle texture,
            ShadowMappingUniforms const& shadowMappingUniforms,
            SoftShadowOptions const& options) noexcept;

    // update local data into GPU UBO
    void commit(backend::DriverApi& driver) noexcept;

    // bind this UBO
    void bind(backend::DriverApi& driver) noexcept;

    void unbindSamplers() noexcept;

private:
    FEngine& mEngine;
    math::float2 mClipControl{};
    TypedUniformBuffer<PerViewUib> mUniforms;
    backend::SamplerGroup mSamplers;
    backend::Handle<backend::HwBufferObject> mUniformBufferHandle;
    backend::Handle<backend::HwSamplerGroup> mSamplerGroupHandle;
    std::uniform_real_distribution<float> mUniformDistribution{ 0.0f, 1.0f };
    static void prepareShadowSampling(PerViewUib& uniforms,
            ShadowMappingUniforms const& shadowMappingUniforms) noexcept;
};

} // namespace filament

#endif //TNT_FILAMENT_PERVIEWUNIFORMS_H
