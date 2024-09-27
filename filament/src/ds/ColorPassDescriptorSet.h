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

#include "DescriptorSet.h"

#include "TypedUniformBuffer.h"

#include <private/filament/UibStructs.h>

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <utils/EntityInstance.h>

#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>
#include <math/mat4.h>

#include <array>

#include <stddef.h>
#include <stdint.h>

namespace filament {

class DescriptorSetLayout;
class HwDescriptorSetLayoutFactory;

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

/*
 * PerViewUniforms manages the UBO and samplers needed to render the color passes. Internally it
 * holds onto handles for the PER_VIEW UBO and SamplerGroup. This class maintains a shadow copy
 * of the UBO/sampler data, so it is possible to partially update it between commits.
 */

class ColorPassDescriptorSet {

    using LightManagerInstance = utils::EntityInstance<LightManager>;
    using TextureHandle = backend::Handle<backend::HwTexture>;

    static constexpr uint32_t const SHADOW_SAMPLING_RUNTIME_PCF   = 0u;
    static constexpr uint32_t const SHADOW_SAMPLING_RUNTIME_EVSM  = 1u;
    static constexpr uint32_t const SHADOW_SAMPLING_RUNTIME_DPCF  = 2u;
    static constexpr uint32_t const SHADOW_SAMPLING_RUNTIME_PCSS  = 3u;

public:

    static uint8_t getIndex(bool lit, bool ssr, bool fog)  noexcept;

    ColorPassDescriptorSet(FEngine& engine,
            TypedUniformBuffer<PerViewUib>& uniforms) noexcept;

    void init(
            backend::BufferObjectHandle lights,
            backend::BufferObjectHandle recordBuffer,
            backend::BufferObjectHandle froxelBuffer) noexcept;

    void terminate(HwDescriptorSetLayoutFactory& factory, backend::DriverApi& driver);

    void prepareCamera(FEngine& engine, const CameraInfo& camera) noexcept;
    void prepareLodBias(float bias, math::float2 derivativesScale) noexcept;

    /*
     * @param viewport  viewport (should be same as RenderPassParams::viewport)
     * @param xoffset   horizontal rendering offset *within* the viewport.
     *                  Non-zero when we have guard bands.
     * @param yoffset   vertical rendering offset *within* the viewport.
     *                  Non-zero when we have guard bands.
     */
    void prepareViewport(
            const filament::Viewport& physicalViewport,
            const filament::Viewport& logicalViewport) noexcept;

    void prepareTime(FEngine& engine, math::float4 const& userTime) noexcept;
    void prepareTemporalNoise(FEngine& engine, TemporalAntiAliasingOptions const& options) noexcept;
    void prepareExposure(float ev100) noexcept;
    void prepareFog(FEngine& engine, const CameraInfo& cameraInfo,
            math::mat4 const& fogTransform, FogOptions const& options,
            FIndirectLight const* ibl) noexcept;
    void prepareStructure(TextureHandle structure) noexcept;
    void prepareSSAO(TextureHandle ssao, AmbientOcclusionOptions const& options) noexcept;
    void prepareBlending(bool needsAlphaChannel) noexcept;
    void prepareMaterialGlobals(std::array<math::float4, 4> const& materialGlobals) noexcept;

    // screen-space reflection and/or refraction (SSR)
    void prepareSSR(TextureHandle ssr,
            bool disableSSR,
            float refractionLodOffset,
            ScreenSpaceReflectionsOptions const& ssrOptions) noexcept;

    void prepareHistorySSR(TextureHandle ssr,
            math::mat4f const& historyProjection,
            math::mat4f const& uvFromViewMatrix,
            ScreenSpaceReflectionsOptions const& ssrOptions) noexcept;

    void prepareShadowMapping(backend::BufferObjectHandle shadowUniforms, bool highPrecision) noexcept;

    void prepareDirectionalLight(FEngine& engine, float exposure,
            math::float3 const& sceneSpaceDirection, LightManagerInstance instance) noexcept;

    void prepareAmbientLight(FEngine& engine,
            FIndirectLight const& ibl, float intensity, float exposure) noexcept;

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

    void prepareShadowPCFDebug(TextureHandle texture,
            ShadowMappingUniforms const& shadowMappingUniforms) noexcept;

    // update local data into GPU UBO
    void commit(backend::DriverApi& driver) noexcept;

    // bind this UBO
    void bind(backend::DriverApi& driver, uint8_t index) const noexcept {
        mDescriptorSet[index].bind(driver, DescriptorSetBindingPoints::PER_VIEW);
    }

private:
    static constexpr size_t DESCRIPTOR_LAYOUT_COUNT = 8;

    void setSampler(backend::descriptor_binding_t binding,
            backend::TextureHandle th, backend::SamplerParams params) noexcept;

    void setBuffer(backend::descriptor_binding_t binding,
            backend::BufferObjectHandle boh, uint32_t offset, uint32_t size) noexcept;

    TypedUniformBuffer<PerViewUib>& mUniforms;
    std::array<DescriptorSetLayout, DESCRIPTOR_LAYOUT_COUNT> mDescriptorSetLayout;
    std::array<DescriptorSet, DESCRIPTOR_LAYOUT_COUNT> mDescriptorSet;
    static void prepareShadowSampling(PerViewUib& uniforms,
            ShadowMappingUniforms const& shadowMappingUniforms) noexcept;
};

} // namespace filament

#endif //TNT_FILAMENT_PERVIEWUNIFORMS_H
