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

    static constexpr uint32_t const SHADOW_SAMPLING_RUNTIME_PCF  = 0u;
    static constexpr uint32_t const SHADOW_SAMPLING_RUNTIME_VSM  = 1u;
    static constexpr uint32_t const SHADOW_SAMPLING_RUNTIME_DPCF = 2u;

public:
    explicit PerViewUniforms(FEngine& engine) noexcept;

    void terminate(backend::DriverApi& driver);

    void prepareCamera(const CameraInfo& camera) noexcept;
    void prepareUpscaler(math::float2 scale, DynamicResolutionOptions const& options) noexcept;
    void prepareViewport(const filament::Viewport& viewport) noexcept;
    void prepareTime(math::float4 const& userTime) noexcept;
    void prepareTemporalNoise(TemporalAntiAliasingOptions const& options) noexcept;
    void prepareExposure(float ev100) noexcept;
    void prepareFog(const CameraInfo& camera, FogOptions const& options) noexcept;
    void prepareStructure(TextureHandle structure) noexcept;
    void prepareSSAO(TextureHandle ssao, AmbientOcclusionOptions const& options) noexcept;
    void prepareSSR(TextureHandle ssr, float refractionLodOffset) noexcept;
    void prepareShadowMapping(ShadowMappingUniforms const& shadowMappingUniforms,
            VsmShadowOptions const& options) noexcept;

    void prepareDirectionalLight(float exposure,
            math::float3 const& sceneSpaceDirection, LightManagerInstance instance) noexcept;

    void prepareAmbientLight(FIndirectLight const& ibl, float intensity, float exposure) noexcept;

    void prepareDynamicLights(Froxelizer& froxelizer) noexcept;

    // maybe these should have their own UBO, they're needed only when GENERATING the shadowmaps
    void prepareShadowVSM(TextureHandle texture, VsmShadowOptions const& options) noexcept;
    void prepareShadowPCF(TextureHandle texture) noexcept;
    void prepareShadowDPCF(TextureHandle texture) noexcept;

    // update local data into GPU UBO
    void commit(backend::DriverApi& driver) noexcept;

    // bind this UBO
    void bind(backend::DriverApi& driver) noexcept;

    void unbindSamplers() noexcept;

private:
    FEngine& mEngine;
    math::float2 mClipControl{};
    TypedUniformBuffer<PerViewUib> mPerViewUb;
    backend::SamplerGroup mPerViewSb;
    backend::Handle<backend::HwSamplerGroup> mPerViewSbh;
    backend::Handle<backend::HwBufferObject> mPerViewUbh;
    std::uniform_real_distribution<float> mUniformDistribution{0.0f, 1.0f};
};

} // namespace filament

#endif //TNT_FILAMENT_PERVIEWUNIFORMS_H
