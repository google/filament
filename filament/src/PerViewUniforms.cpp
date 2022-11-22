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

#include "PerViewUniforms.h"

#include "DFG.h"
#include "Froxelizer.h"
#include "ShadowMapManager.h"

#include "details/Camera.h"
#include "details/Engine.h"
#include "details/IndirectLight.h"
#include "details/Texture.h"

#include <filament/Exposure.h>
#include <filament/Options.h>
#include <filament/TextureSampler.h>

#include <private/filament/EngineEnums.h>
#include <private/filament/SibStructs.h>

#include <math/mat4.h>

namespace filament {

using namespace backend;
using namespace math;

PerViewUniforms::PerViewUniforms(FEngine& engine) noexcept
        : mSamplers(PerViewSib::SAMPLER_COUNT) {
    DriverApi& driver = engine.getDriverApi();

    mSamplerGroupHandle = driver.createSamplerGroup(mSamplers.getSize());

    mUniformBufferHandle = driver.createBufferObject(mUniforms.getSize(),
            BufferObjectBinding::UNIFORM, BufferUsage::DYNAMIC);

    if (engine.getDFG().isValid()) {
        TextureSampler sampler(TextureSampler::MagFilter::LINEAR);
        mSamplers.setSampler(PerViewSib::IBL_DFG_LUT,
                { engine.getDFG().getTexture(), sampler.getSamplerParams() });
    }
}

void PerViewUniforms::terminate(DriverApi& driver) {
    driver.destroyBufferObject(mUniformBufferHandle);
    driver.destroySamplerGroup(mSamplerGroupHandle);
}

void PerViewUniforms::prepareCamera(FEngine& engine, const CameraInfo& camera) noexcept {
    mat4f const& viewFromWorld = camera.view;
    mat4f const& worldFromView = camera.model;
    mat4f const& clipFromView  = camera.projection;

    const mat4f viewFromClip{ inverse((mat4)camera.projection) };
    const mat4f clipFromWorld{ highPrecisionMultiply(clipFromView, viewFromWorld) };
    const mat4f worldFromClip{ highPrecisionMultiply(worldFromView, viewFromClip) };

    auto& s = mUniforms.edit();
    s.viewFromWorldMatrix = viewFromWorld;    // view
    s.worldFromViewMatrix = worldFromView;    // model
    s.clipFromViewMatrix  = clipFromView;     // projection
    s.viewFromClipMatrix  = viewFromClip;     // 1/projection
    s.clipFromWorldMatrix = clipFromWorld;    // projection * view
    s.worldFromClipMatrix = worldFromClip;    // 1/(projection * view)
    s.clipTransform = camera.clipTransfrom;
    s.cameraPosition = float3{ camera.getPosition() };
    s.worldOffset = camera.getWorldOffset();
    s.cameraFar = camera.zf;
    s.oneOverFarMinusNear = 1.0f / (camera.zf - camera.zn);
    s.nearOverFarMinusNear = camera.zn / (camera.zf - camera.zn);

    // with a clip-space of [-w, w] ==> z' = -z
    // with a clip-space of [0,  w] ==> z' = (w - z)/2
    s.clipControl = engine.getDriverApi().getClipSpaceParams();
}

void PerViewUniforms::prepareLodBias(float bias) noexcept {
    auto& s = mUniforms.edit();
    s.lodBias = bias;
}

void PerViewUniforms::prepareExposure(float ev100) noexcept {
    const float exposure = Exposure::exposure(ev100);
    auto& s = mUniforms.edit();
    s.exposure = exposure;
    s.ev100 = ev100;
}

void PerViewUniforms::prepareViewport(const filament::Viewport& viewport,
        uint32_t xoffset, uint32_t yoffset) noexcept {
    const float w = float(viewport.width);
    const float h = float(viewport.height);
    auto& s = mUniforms.edit();
    s.resolution = float4{ w, h, 1.0f / w, 1.0f / h };
    s.origin = float2{ viewport.left, viewport.bottom };
    s.offset = float2{ xoffset, yoffset };
}

void PerViewUniforms::prepareTime(FEngine& engine, math::float4 const& userTime) noexcept {
    auto& s = mUniforms.edit();
    const uint64_t oneSecondRemainder = engine.getEngineTime().count() % 1000000000;
    const float fraction = float(double(oneSecondRemainder) / 1000000000.0);
    s.time = fraction;
    s.userTime = userTime;
}

void PerViewUniforms::prepareTemporalNoise(FEngine& engine,
        TemporalAntiAliasingOptions const& options) noexcept {
    std::uniform_real_distribution<float> uniformDistribution{ 0.0f, 1.0f };
    auto& s = mUniforms.edit();
    const float temporalNoise = uniformDistribution(engine.getRandomEngine());
    s.temporalNoise = options.enabled ? temporalNoise : 0.0f;
}

void PerViewUniforms::prepareFog(float3 const& cameraPosition, FogOptions const& options) noexcept {
    // this can't be too high because we need density / heightFalloff to produce something
    // close to fogOptions.density in the fragment shader which use 16-bits floats.
    constexpr float epsilon = 0.001f;
    const float heightFalloff = std::max(epsilon, options.heightFalloff);

    // precalculate the constant part of density  integral and correct for exp2() in the shader
    const float density = ((options.density / heightFalloff) *
            std::exp(-heightFalloff * (cameraPosition.y - options.height)))
                    * float(1.0f / F_LN2);

    auto& s = mUniforms.edit();
    s.fogStart             = options.distance;
    s.fogMaxOpacity        = options.maximumOpacity;
    s.fogHeight            = options.height;
    s.fogHeightFalloff     = heightFalloff;
    s.fogColor             = options.color;
    s.fogDensity           = density;
    s.fogInscatteringStart = options.inScatteringStart;
    s.fogInscatteringSize  = options.inScatteringSize;
    s.fogColorFromIbl      = options.fogColorFromIbl ? 1.0f : 0.0f;
}

void PerViewUniforms::prepareSSAO(Handle<HwTexture> ssao,
        AmbientOcclusionOptions const& options) noexcept {
    // High quality sampling is enabled only if AO itself is enabled and upsampling quality is at
    // least set to high and of course only if upsampling is needed.
    const bool highQualitySampling = options.upsampling >= QualityLevel::HIGH
            && options.resolution < 1.0f;

    // LINEAR filtering is only needed when AO is enabled and low-quality upsampling is used.
    mSamplers.setSampler(PerViewSib::SSAO, { ssao, {
        .filterMag = options.enabled && !highQualitySampling ?
                SamplerMagFilter::LINEAR : SamplerMagFilter::NEAREST
    }});

    const float edgeDistance = 1.0f / options.bilateralThreshold;
    auto& s = mUniforms.edit();
    s.aoSamplingQualityAndEdgeDistance =
            options.enabled ? (highQualitySampling ? edgeDistance : 0.0f) : -1.0f;
    s.aoBentNormals = options.enabled && options.bentNormals ? 1.0f : 0.0f;
}

void PerViewUniforms::prepareBlending(bool needsAlphaChannel) noexcept {
    mUniforms.edit().needsAlphaChannel = needsAlphaChannel ? 1.0f : 0.0f;
}

void PerViewUniforms::prepareSSR(Handle<HwTexture> ssr,
        float refractionLodOffset,
        ScreenSpaceReflectionsOptions const& ssrOptions) noexcept {

    mSamplers.setSampler(PerViewSib::SSR, { ssr, {
        .filterMag = SamplerMagFilter::LINEAR,
        .filterMin = SamplerMinFilter::LINEAR_MIPMAP_LINEAR
    }});

    auto& s = mUniforms.edit();
    s.refractionLodOffset = refractionLodOffset;
    s.ssrDistance = ssrOptions.enabled ? ssrOptions.maxDistance : 0.0f;
}

void PerViewUniforms::prepareHistorySSR(Handle<HwTexture> ssr,
        math::mat4f const& historyProjection,
        math::mat4f const& uvFromViewMatrix,
        ScreenSpaceReflectionsOptions const& ssrOptions) noexcept {

    mSamplers.setSampler(PerViewSib::SSR, { ssr, {
        .filterMag = SamplerMagFilter::LINEAR,
        .filterMin = SamplerMinFilter::LINEAR
    }});

    auto& s = mUniforms.edit();
    s.ssrReprojection = historyProjection;
    s.ssrUvFromViewMatrix = uvFromViewMatrix;
    s.ssrThickness = ssrOptions.thickness;
    s.ssrBias = ssrOptions.bias;
    s.ssrDistance = ssrOptions.enabled ? ssrOptions.maxDistance : 0.0f;
    s.ssrStride = ssrOptions.stride;
}

void PerViewUniforms::prepareStructure(Handle<HwTexture> structure) noexcept {
    // sampler must be NEAREST
    mSamplers.setSampler(PerViewSib::STRUCTURE, { structure, {}});
}

void PerViewUniforms::prepareDirectionalLight(FEngine& engine,
        float exposure,
        float3 const& sceneSpaceDirection,
        PerViewUniforms::LightManagerInstance directionalLight) noexcept {
    FLightManager& lcm = engine.getLightManager();
    auto& s = mUniforms.edit();

    const float3 l = -sceneSpaceDirection; // guaranteed normalized

    if (directionalLight.isValid()) {
        const float4 colorIntensity = {
                lcm.getColor(directionalLight), lcm.getIntensity(directionalLight) * exposure };

        s.lightDirection = l;
        s.lightColorIntensity = colorIntensity;
        s.lightChannels = lcm.getLightChannels(directionalLight);

        const bool isSun = lcm.isSunLight(directionalLight);
        // The last parameter must be < 0.0f for regular directional lights
        float4 sun{ 0.0f, 0.0f, 0.0f, -1.0f };
        if (UTILS_UNLIKELY(isSun && colorIntensity.w > 0.0f)) {
            // currently we have only a single directional light, so it's probably likely that it's
            // also the Sun. However, conceptually, most directional lights won't be sun lights.
            float radius = lcm.getSunAngularRadius(directionalLight);
            float haloSize = lcm.getSunHaloSize(directionalLight);
            float haloFalloff = lcm.getSunHaloFalloff(directionalLight);
            sun.x = std::cos(radius);
            sun.y = std::sin(radius);
            sun.z = 1.0f / (std::cos(radius * haloSize) - sun.x);
            sun.w = haloFalloff;
        }
        s.sun = sun;
    } else {
        // Disable the sun if there's no directional light
        s.sun = float4{ 0.0f, 0.0f, 0.0f, -1.0f };
    }
}

void PerViewUniforms::prepareAmbientLight(FEngine& engine, FIndirectLight const& ibl,
        float intensity, float exposure) noexcept {
    auto& s = mUniforms.edit();

    // Set up uniforms and sampler for the IBL, guaranteed to be non-null at this point.
    float iblRoughnessOneLevel = ibl.getLevelCount() - 1.0f;
    s.iblRoughnessOneLevel = iblRoughnessOneLevel;
    s.iblLuminance = intensity * exposure;
    std::transform(ibl.getSH(), ibl.getSH() + 9, s.iblSH, [](float3 v) {
        return float4(v, 0.0f);
    });

    // We always sample from the reflection texture, so provide a dummy texture if necessary.
    Handle<HwTexture> reflection = ibl.getReflectionHwHandle();
    if (!reflection) {
        reflection = engine.getDummyCubemap()->getHwHandle();
    }
    mSamplers.setSampler(PerViewSib::IBL_SPECULAR, {
            reflection, {
                    .filterMag = SamplerMagFilter::LINEAR,
                    .filterMin = SamplerMinFilter::LINEAR_MIPMAP_LINEAR
            }});
}

void PerViewUniforms::prepareDynamicLights(Froxelizer& froxelizer) noexcept {
    auto& s = mUniforms.edit();
    froxelizer.updateUniforms(s);
    float f = froxelizer.getLightFar();
    mSamplers.setSampler(PerViewSib::FROXELS, { froxelizer.getFroxelTexture() });
    s.lightFarAttenuationParams = 0.5f * float2{ 10.0f, 10.0f / (f * f) };
}

void PerViewUniforms::prepareShadowMapping(bool highPrecision) noexcept {
    auto& s = mUniforms.edit();
    constexpr float low  = 5.54f; // ~ std::log(std::numeric_limits<math::half>::max()) * 0.5f;
    constexpr float high = 42.0f; // ~ std::log(std::numeric_limits<float>::max()) * 0.5f;
    s.vsmExponent = highPrecision ? high : low;
}

void PerViewUniforms::prepareShadowSampling(PerViewUib& uniforms,
        ShadowMappingUniforms const& shadowMappingUniforms) noexcept {
    uniforms.cascadeSplits              = shadowMappingUniforms.cascadeSplits;
    uniforms.ssContactShadowDistance    = shadowMappingUniforms.ssContactShadowDistance;
    uniforms.directionalShadows         = shadowMappingUniforms.directionalShadows;
    uniforms.cascades                   = shadowMappingUniforms.cascades;
}

void PerViewUniforms::prepareShadowVSM(Handle<HwTexture> texture,
        ShadowMappingUniforms const& shadowMappingUniforms,
        VsmShadowOptions const& options) noexcept {
    constexpr float low  = 5.54f; // ~ std::log(std::numeric_limits<math::half>::max()) * 0.5f;
    constexpr float high = 42.0f; // ~ std::log(std::numeric_limits<float>::max()) * 0.5f;
    SamplerMinFilter filterMin = SamplerMinFilter::LINEAR;
    if (options.anisotropy > 0 || options.mipmapping) {
        filterMin = SamplerMinFilter::LINEAR_MIPMAP_LINEAR;
    }
    mSamplers.setSampler(PerViewSib::SHADOW_MAP, {
            texture, {
                    .filterMag = SamplerMagFilter::LINEAR,
                    .filterMin = filterMin,
                    .anisotropyLog2 = options.anisotropy,
            }});
    auto& s = mUniforms.edit();
    s.shadowSamplingType = SHADOW_SAMPLING_RUNTIME_EVSM;
    s.vsmExponent = options.highPrecision ? high : low;
    s.vsmDepthScale = options.minVarianceScale * 0.01f * s.vsmExponent;
    s.vsmLightBleedReduction = options.lightBleedReduction;
    PerViewUniforms::prepareShadowSampling(s, shadowMappingUniforms);
}

void PerViewUniforms::prepareShadowPCF(Handle<HwTexture> texture,
        ShadowMappingUniforms const& shadowMappingUniforms) noexcept {
    mSamplers.setSampler(PerViewSib::SHADOW_MAP, {
            texture, {
                    .filterMag = SamplerMagFilter::LINEAR,
                    .filterMin = SamplerMinFilter::LINEAR,
                    .compareMode = SamplerCompareMode::COMPARE_TO_TEXTURE,
                    .compareFunc = SamplerCompareFunc::GE
            }});
    auto& s = mUniforms.edit();
    s.shadowSamplingType = SHADOW_SAMPLING_RUNTIME_PCF;
    PerViewUniforms::prepareShadowSampling(s, shadowMappingUniforms);
}

void PerViewUniforms::prepareShadowDPCF(Handle<HwTexture> texture,
        ShadowMappingUniforms const& shadowMappingUniforms,
        SoftShadowOptions const& options) noexcept {
    mSamplers.setSampler(PerViewSib::SHADOW_MAP, { texture, {}});
    auto& s = mUniforms.edit();
    s.shadowSamplingType = SHADOW_SAMPLING_RUNTIME_DPCF;
    s.shadowPenumbraRatioScale = options.penumbraRatioScale;
    PerViewUniforms::prepareShadowSampling(s, shadowMappingUniforms);
}

void PerViewUniforms::prepareShadowPCSS(Handle<HwTexture> texture,
        ShadowMappingUniforms const& shadowMappingUniforms,
        SoftShadowOptions const& options) noexcept {
    mSamplers.setSampler(PerViewSib::SHADOW_MAP, { texture, {}});
    auto& s = mUniforms.edit();
    s.shadowSamplingType = SHADOW_SAMPLING_RUNTIME_PCSS;
    s.shadowPenumbraRatioScale = options.penumbraRatioScale;
    PerViewUniforms::prepareShadowSampling(s, shadowMappingUniforms);
}

void PerViewUniforms::commit(backend::DriverApi& driver) noexcept {
    if (mUniforms.isDirty()) {
        driver.updateBufferObject(mUniformBufferHandle, mUniforms.toBufferDescriptor(driver), 0);
    }
    if (mSamplers.isDirty()) {
        driver.updateSamplerGroup(mSamplerGroupHandle, mSamplers.toBufferDescriptor(driver));
    }
}

void PerViewUniforms::bind(backend::DriverApi& driver) noexcept {
    driver.bindUniformBuffer(+UniformBindingPoints::PER_VIEW, mUniformBufferHandle);
    driver.bindSamplers(+SamplerBindingPoints::PER_VIEW, mSamplerGroupHandle);
}

void PerViewUniforms::unbindSamplers() noexcept {
    auto& samplerGroup = mSamplers;
    samplerGroup.clearSampler(PerViewSib::SSAO);
    samplerGroup.clearSampler(PerViewSib::SSR);
    samplerGroup.clearSampler(PerViewSib::STRUCTURE);
    samplerGroup.clearSampler(PerViewSib::SHADOW_MAP);
}

} // namespace filament

