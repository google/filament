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

#include "ColorPassDescriptorSet.h"

#include "Froxelizer.h"
#include "HwDescriptorSetLayoutFactory.h"
#include "ShadowMapManager.h"
#include "TypedUniformBuffer.h"

#include "components/LightManager.h"

#include "details/Camera.h"
#include "details/Engine.h"
#include "details/IndirectLight.h"
#include "details/Texture.h"

#include <filament/Engine.h>
#include <filament/Exposure.h>
#include <filament/Options.h>
#include <filament/TextureSampler.h>
#include <filament/MaterialEnums.h>
#include <filament/Viewport.h>

#include <private/filament/EngineEnums.h>
#include <private/filament/DescriptorSets.h>
#include <private/filament/UibStructs.h>

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <math/mat4.h>
#include <math/mat3.h>
#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <utils/compiler.h>
#include <utils/debug.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <random>

#include <stddef.h>
#include <stdint.h>

namespace filament {

using namespace backend;
using namespace math;

uint8_t ColorPassDescriptorSet::getIndex(
        bool const lit, bool const ssr, bool const fog) noexcept {

    uint8_t index = 0;

    if (!lit) {
        // this will remove samplers unused when unit
        index |= 0x1;
    }

    if (ssr) {
        // this will add samplers needed for screen-space SSR
        index |= 0x2;
    }

    if (!fog) {
        // this will remove samplers needed for fog
        index |= 0x4;
    }

    assert_invariant(index < DESCRIPTOR_LAYOUT_COUNT);
    return index;
}


ColorPassDescriptorSet::ColorPassDescriptorSet(FEngine& engine,
        TypedUniformBuffer<PerViewUib>& uniforms) noexcept
        : mUniforms(uniforms) {

    constexpr UserVariantFilterMask filterFog = UserVariantFilterMask(UserVariantFilterBit::FOG);
    constexpr UserVariantFilterMask keepFog = UserVariantFilterMask(0);

    for (bool const lit: { false, true }) {
        for (bool const ssr: { false, true }) {
            for (bool const fog: { false, true }) {
                auto index = getIndex(lit, ssr, fog);
                mDescriptorSetLayout[index] = {
                        engine.getDescriptorSetLayoutFactory(),
                        engine.getDriverApi(),
                        descriptor_sets::getPerViewDescriptorSetLayout(
                                MaterialDomain::SURFACE,
                                fog ? keepFog : filterFog,
                                lit,
                                ssr ? ReflectionMode::SCREEN_SPACE : ReflectionMode::DEFAULT,
                                ssr ? RefractionMode::SCREEN_SPACE : RefractionMode::NONE)
                };
                mDescriptorSet[index] = DescriptorSet{ mDescriptorSetLayout[index] };
            }
        }
    }

    setBuffer(+PerViewBindingPoints::FRAME_UNIFORMS,
            uniforms.getUboHandle(), 0, uniforms.getSize());

    if (engine.getDFG().isValid()) {
        TextureSampler const sampler(TextureSampler::MagFilter::LINEAR);
        setSampler(+PerViewBindingPoints::IBL_DFG_LUT,
                engine.getDFG().getTexture(), sampler.getSamplerParams());
    }
}

void ColorPassDescriptorSet::init(
        BufferObjectHandle lights,
        BufferObjectHandle recordBuffer,
        BufferObjectHandle froxelBuffer) noexcept {
    for (auto&& descriptorSet: mDescriptorSet) {
        descriptorSet.setBuffer(+PerViewBindingPoints::LIGHTS,
                lights, 0, sizeof(LightsUib));
        descriptorSet.setBuffer(+PerViewBindingPoints::RECORD_BUFFER,
                recordBuffer, 0, sizeof(FroxelRecordUib));
        descriptorSet.setBuffer(+PerViewBindingPoints::FROXEL_BUFFER,
                froxelBuffer, 0, sizeof(FroxelsUib));
    }
}

void ColorPassDescriptorSet::terminate(HwDescriptorSetLayoutFactory& factory, DriverApi& driver) {
    for (auto&& entry : mDescriptorSet) {
        entry.terminate(driver);
    }
    for (auto&& entry : mDescriptorSetLayout) {
        entry.terminate(factory, driver);
    }
}

void ColorPassDescriptorSet::prepareCamera(FEngine& engine, const CameraInfo& camera) noexcept {
    mat4f const& viewFromWorld = camera.view;
    mat4f const& worldFromView = camera.model;
    mat4f const& clipFromView  = camera.projection;

    const mat4f viewFromClip{ inverse((mat4)camera.projection) };
    const mat4f worldFromClip{ highPrecisionMultiply(worldFromView, viewFromClip) };

    auto& s = mUniforms.edit();
    s.viewFromWorldMatrix = viewFromWorld;    // view
    s.worldFromViewMatrix = worldFromView;    // model
    s.clipFromViewMatrix  = clipFromView;     // projection
    s.viewFromClipMatrix  = viewFromClip;     // 1/projection
    s.worldFromClipMatrix = worldFromClip;    // 1/(projection * view)
    s.userWorldFromWorldMatrix = mat4f(inverse(camera.worldTransform));
    s.clipTransform = camera.clipTransform;
    s.cameraFar = camera.zf;
    s.oneOverFarMinusNear = 1.0f / (camera.zf - camera.zn);
    s.nearOverFarMinusNear = camera.zn / (camera.zf - camera.zn);

    mat4f const& headFromWorld = camera.view;
    Engine::Config const& config = engine.getConfig();
    for (int i = 0; i < config.stereoscopicEyeCount; i++) {
        mat4f const& eyeFromHead = camera.eyeFromView[i];   // identity for monoscopic rendering
        mat4f const& clipFromEye = camera.eyeProjection[i];
        // clipFromEye * eyeFromHead * headFromWorld
        s.clipFromWorldMatrix[i] = highPrecisionMultiply(
                clipFromEye, highPrecisionMultiply(eyeFromHead, headFromWorld));
    }

    // with a clip-space of [-w, w] ==> z' = -z
    // with a clip-space of [0,  w] ==> z' = (w - z)/2
    s.clipControl = engine.getDriverApi().getClipSpaceParams();
}

void ColorPassDescriptorSet::prepareLodBias(float const bias, float2 const derivativesScale) noexcept {
    auto& s = mUniforms.edit();
    s.lodBias = bias;
    s.derivativesScale = derivativesScale;
}

void ColorPassDescriptorSet::prepareExposure(float const ev100) noexcept {
    const float exposure = Exposure::exposure(ev100);
    auto& s = mUniforms.edit();
    s.exposure = exposure;
    s.ev100 = ev100;
}

void ColorPassDescriptorSet::prepareViewport(
        const filament::Viewport& physicalViewport,
        const filament::Viewport& logicalViewport) noexcept {
    float4 const physical{ physicalViewport.left, physicalViewport.bottom,
                           physicalViewport.width, physicalViewport.height };
    float4 const logical{ logicalViewport.left, logicalViewport.bottom,
                          logicalViewport.width, logicalViewport.height };
    auto& s = mUniforms.edit();
    s.resolution = { physical.zw, 1.0f / physical.zw };
    s.logicalViewportScale = physical.zw / logical.zw;
    s.logicalViewportOffset = -logical.xy / logical.zw;
}

void ColorPassDescriptorSet::prepareTime(FEngine& engine, float4 const& userTime) noexcept {
    auto& s = mUniforms.edit();
    const uint64_t oneSecondRemainder = engine.getEngineTime().count() % 1000000000;
    const float fraction = float(double(oneSecondRemainder) / 1000000000.0);
    s.time = fraction;
    s.userTime = userTime;
}

void ColorPassDescriptorSet::prepareTemporalNoise(FEngine& engine,
        TemporalAntiAliasingOptions const& options) noexcept {
    std::uniform_real_distribution<float> uniformDistribution{ 0.0f, 1.0f };
    auto& s = mUniforms.edit();
    const float temporalNoise = uniformDistribution(engine.getRandomEngine());
    s.temporalNoise = options.enabled ? temporalNoise : 0.0f;
}

void ColorPassDescriptorSet::prepareFog(FEngine& engine, const CameraInfo& cameraInfo,
        mat4 const& userWorldFromFog, FogOptions const& options, FIndirectLight const* ibl) noexcept {

    auto packHalf2x16 = [](half2 v) -> uint32_t {
        short2 s;
        memcpy(&s[0], &v[0], sizeof(s));
        return s.y << 16 | s.x;
    };

    // Fog should be calculated in the "user's world coordinates" so that it's not
    // affected by the IBL rotation.
    // fogFromWorldMatrix below is only used to transform the view vector in the shader, which is
    // why we store the cofactor matrix.

    mat4f const viewFromWorld       = cameraInfo.view;
    mat4 const worldFromUserWorld   = cameraInfo.worldTransform;
    mat4 const worldFromFog         = worldFromUserWorld * userWorldFromFog;
    mat4 const viewFromFog          = viewFromWorld * worldFromFog;

    mat4 const fogFromView          = inverse(viewFromFog);
    mat3 const fogFromWorld         = inverse(worldFromFog.upperLeft());

    // camera position relative to the fog's origin
    auto const userCameraPosition = fogFromView[3].xyz;

    const float heightFalloff = std::max(0.0f, options.heightFalloff);

    // precalculate the constant part of density integral
    const float density = -float(heightFalloff * (userCameraPosition.y - options.height));

    auto& s = mUniforms.edit();

    // note: this code is written so that near/far/minLod/maxLod could be user settable
    //       currently they're inferred.
    Handle<HwTexture> fogColorTextureHandle;
    if (options.skyColor) {
        fogColorTextureHandle = downcast(options.skyColor)->getHwHandleForSampling();
        half2 const minMaxMip{ 0.0f, float(options.skyColor->getLevels()) - 1.0f };
        s.fogMinMaxMip = packHalf2x16(minMaxMip);
        s.fogOneOverFarMinusNear = 1.0f / (cameraInfo.zf - cameraInfo.zn);
        s.fogNearOverFarMinusNear = cameraInfo.zn / (cameraInfo.zf - cameraInfo.zn);
    }
    if (!fogColorTextureHandle && options.fogColorFromIbl) {
        if (ibl) {
            // When using the IBL, because we don't have mip levels, we don't have a mop to
            // select based on the distance. However, we can cheat a little and use
            // mip_roughnessOne-1 as the horizon base color and mip_roughnessOne as the near
            // camera base color. This will give a distant fog that's a bit too sharp, but it
            // improves the effect overall.
            fogColorTextureHandle = ibl->getReflectionHwHandle();
            float const levelCount = float(ibl->getLevelCount());
            half2 const minMaxMip{ levelCount - 2.0f, levelCount - 1.0f };
            s.fogMinMaxMip = packHalf2x16(minMaxMip);
            s.fogOneOverFarMinusNear = 1.0f / (cameraInfo.zf - cameraInfo.zn);
            s.fogNearOverFarMinusNear = cameraInfo.zn / (cameraInfo.zf - cameraInfo.zn);
        }
    }

    setSampler(+PerViewBindingPoints::FOG,
            fogColorTextureHandle ?
                    fogColorTextureHandle : engine.getDummyCubemap()->getHwHandleForSampling(), {
                    .filterMag = SamplerMagFilter::LINEAR,
                    .filterMin = SamplerMinFilter::LINEAR_MIPMAP_LINEAR
            });

    s.fogStart             = options.distance;
    s.fogMaxOpacity        = options.maximumOpacity;
    s.fogHeightFalloff     = heightFalloff;
    s.fogCutOffDistance    = options.cutOffDistance;
    s.fogColor             = options.color;
    s.fogDensity           = { options.density, density, options.density * std::exp(density) };
    s.fogInscatteringStart = options.inScatteringStart;
    s.fogInscatteringSize  = options.inScatteringSize;
    s.fogColorFromIbl      = fogColorTextureHandle ? 1.0f : 0.0f;
    s.fogFromWorldMatrix   = mat3f{ cof(fogFromWorld) };
}

void ColorPassDescriptorSet::prepareSSAO(Handle<HwTexture> ssao,
        AmbientOcclusionOptions const& options) noexcept {
    // High quality sampling is enabled only if AO itself is enabled and upsampling quality is at
    // least set to high and of course only if upsampling is needed.
    const bool highQualitySampling = options.upsampling >= QualityLevel::HIGH
            && options.resolution < 1.0f;

    // LINEAR filtering is only needed when AO is enabled and low-quality upsampling is used.
    setSampler(+PerViewBindingPoints::SSAO, ssao, {
        .filterMag = options.enabled && !highQualitySampling ?
                SamplerMagFilter::LINEAR : SamplerMagFilter::NEAREST
    });

    const float edgeDistance = 1.0f / options.bilateralThreshold;
    auto& s = mUniforms.edit();
    s.aoSamplingQualityAndEdgeDistance =
            options.enabled ? (highQualitySampling ? edgeDistance : 0.0f) : -1.0f;
    s.aoBentNormals = options.enabled && options.bentNormals ? 1.0f : 0.0f;
}

void ColorPassDescriptorSet::prepareBlending(bool const needsAlphaChannel) noexcept {
    mUniforms.edit().needsAlphaChannel = needsAlphaChannel ? 1.0f : 0.0f;
}

void ColorPassDescriptorSet::prepareMaterialGlobals(
        std::array<float4, 4> const& materialGlobals) noexcept {
    mUniforms.edit().custom[0] = materialGlobals[0];
    mUniforms.edit().custom[1] = materialGlobals[1];
    mUniforms.edit().custom[2] = materialGlobals[2];
    mUniforms.edit().custom[3] = materialGlobals[3];
}

void ColorPassDescriptorSet::prepareSSR(Handle<HwTexture> ssr,
        bool const disableSSR,
        float const refractionLodOffset,
        ScreenSpaceReflectionsOptions const& ssrOptions) noexcept {

    setSampler(+PerViewBindingPoints::SSR, ssr, {
        .filterMag = SamplerMagFilter::LINEAR,
        .filterMin = SamplerMinFilter::LINEAR_MIPMAP_LINEAR
    });

    auto& s = mUniforms.edit();
    s.refractionLodOffset = refractionLodOffset;
    s.ssrDistance = (ssrOptions.enabled && !disableSSR) ? ssrOptions.maxDistance : 0.0f;
}

void ColorPassDescriptorSet::prepareHistorySSR(Handle<HwTexture> ssr,
        mat4f const& historyProjection,
        mat4f const& uvFromViewMatrix,
        ScreenSpaceReflectionsOptions const& ssrOptions) noexcept {

    setSampler(+PerViewBindingPoints::SSR, ssr, {
        .filterMag = SamplerMagFilter::LINEAR,
        .filterMin = SamplerMinFilter::LINEAR
    });

    auto& s = mUniforms.edit();
    s.ssrReprojection = historyProjection;
    s.ssrUvFromViewMatrix = uvFromViewMatrix;
    s.ssrThickness = ssrOptions.thickness;
    s.ssrBias = ssrOptions.bias;
    s.ssrDistance = ssrOptions.enabled ? ssrOptions.maxDistance : 0.0f;
    s.ssrStride = ssrOptions.stride;
}

void ColorPassDescriptorSet::prepareStructure(Handle<HwTexture> structure) noexcept {
    // sampler must be NEAREST
    setSampler(+PerViewBindingPoints::STRUCTURE, structure, {});
}

void ColorPassDescriptorSet::prepareDirectionalLight(FEngine& engine,
        float exposure,
        float3 const& sceneSpaceDirection,
        LightManagerInstance directionalLight) noexcept {
    FLightManager const& lcm = engine.getLightManager();
    auto& s = mUniforms.edit();

    float const shadowFar = lcm.getShadowFar(directionalLight);
    s.shadowFarAttenuationParams = shadowFar > 0.0f ?
            0.5f * float2{ 10.0f, 10.0f / (shadowFar * shadowFar) } : float2{ 1.0f, 0.0f };

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
            // Currently we have only a single directional light, so it's probably likely that it's
            // also the Sun. However, conceptually, most directional lights won't be sun lights.
            float const radius = lcm.getSunAngularRadius(directionalLight);
            float const haloSize = lcm.getSunHaloSize(directionalLight);
            float const haloFalloff = lcm.getSunHaloFalloff(directionalLight);
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

void ColorPassDescriptorSet::prepareAmbientLight(FEngine& engine, FIndirectLight const& ibl,
        float const intensity, float const exposure) noexcept {
    auto& s = mUniforms.edit();

    // Set up uniforms and sampler for the IBL, guaranteed to be non-null at this point.
    float const iblRoughnessOneLevel = float(ibl.getLevelCount() - 1);
    s.iblRoughnessOneLevel = iblRoughnessOneLevel;
    s.iblLuminance = intensity * exposure;
    std::transform(ibl.getSH(), ibl.getSH() + 9, s.iblSH, [](float3 const v) {
        return float4(v, 0.0f);
    });

    // We always sample from the reflection texture, so provide a dummy texture if necessary.
    Handle<HwTexture> reflection = ibl.getReflectionHwHandle();
    if (!reflection) {
        reflection = engine.getDummyCubemap()->getHwHandle();
    }
    setSampler(+PerViewBindingPoints::IBL_SPECULAR,
            reflection, {
                    .filterMag = SamplerMagFilter::LINEAR,
                    .filterMin = SamplerMinFilter::LINEAR_MIPMAP_LINEAR
            });
}

void ColorPassDescriptorSet::prepareDynamicLights(Froxelizer& froxelizer) noexcept {
    auto& s = mUniforms.edit();
    froxelizer.updateUniforms(s);
    float const f = froxelizer.getLightFar();
    // TODO: make the falloff rate a parameter
    s.lightFarAttenuationParams = 0.5f * float2{ 10.0f, 10.0f / (f * f) };
}

void ColorPassDescriptorSet::prepareShadowMapping(BufferObjectHandle shadowUniforms, bool const highPrecision) noexcept {
    auto& s = mUniforms.edit();
    constexpr float low  = 5.54f; // ~ std::log(std::numeric_limits<math::half>::max()) * 0.5f;
    constexpr float high = 42.0f; // ~ std::log(std::numeric_limits<float>::max()) * 0.5f;
    s.vsmExponent = highPrecision ? high : low;
    setBuffer(+PerViewBindingPoints::SHADOWS, shadowUniforms, 0, sizeof(ShadowUib));
}

void ColorPassDescriptorSet::prepareShadowSampling(PerViewUib& uniforms,
        ShadowMappingUniforms const& shadowMappingUniforms) noexcept {
    uniforms.cascadeSplits              = shadowMappingUniforms.cascadeSplits;
    uniforms.ssContactShadowDistance    = shadowMappingUniforms.ssContactShadowDistance;
    uniforms.directionalShadows         = int32_t(shadowMappingUniforms.directionalShadows);
    uniforms.cascades                   = int32_t(shadowMappingUniforms.cascades);
}

void ColorPassDescriptorSet::prepareShadowVSM(Handle<HwTexture> texture,
        ShadowMappingUniforms const& shadowMappingUniforms,
        VsmShadowOptions const& options) noexcept {
    constexpr float low  = 5.54f; // ~ std::log(std::numeric_limits<math::half>::max()) * 0.5f;
    constexpr float high = 42.0f; // ~ std::log(std::numeric_limits<float>::max()) * 0.5f;
    SamplerMinFilter filterMin = SamplerMinFilter::LINEAR;
    if (options.anisotropy > 0 || options.mipmapping) {
        filterMin = SamplerMinFilter::LINEAR_MIPMAP_LINEAR;
    }
    setSampler(+PerViewBindingPoints::SHADOW_MAP,
            texture, {
                    .filterMag = SamplerMagFilter::LINEAR,
                    .filterMin = filterMin,
                    .anisotropyLog2 = options.anisotropy,
            });
    auto& s = mUniforms.edit();
    s.shadowSamplingType = SHADOW_SAMPLING_RUNTIME_EVSM;
    s.vsmExponent = options.highPrecision ? high : low;
    s.vsmDepthScale = options.minVarianceScale * 0.01f * s.vsmExponent;
    s.vsmLightBleedReduction = options.lightBleedReduction;
    prepareShadowSampling(s, shadowMappingUniforms);
}

void ColorPassDescriptorSet::prepareShadowPCF(Handle<HwTexture> texture,
        ShadowMappingUniforms const& shadowMappingUniforms) noexcept {
    setSampler(+PerViewBindingPoints::SHADOW_MAP,
            texture, {
                    .filterMag = SamplerMagFilter::LINEAR,
                    .filterMin = SamplerMinFilter::LINEAR,
                    .compareMode = SamplerCompareMode::COMPARE_TO_TEXTURE,
                    .compareFunc = SamplerCompareFunc::GE
            });
    auto& s = mUniforms.edit();
    s.shadowSamplingType = SHADOW_SAMPLING_RUNTIME_PCF;
    prepareShadowSampling(s, shadowMappingUniforms);
}

void ColorPassDescriptorSet::prepareShadowDPCF(Handle<HwTexture> texture,
        ShadowMappingUniforms const& shadowMappingUniforms,
        SoftShadowOptions const& options) noexcept {
    setSampler(+PerViewBindingPoints::SHADOW_MAP, texture, {});
    auto& s = mUniforms.edit();
    s.shadowSamplingType = SHADOW_SAMPLING_RUNTIME_DPCF;
    s.shadowPenumbraRatioScale = options.penumbraRatioScale;
    prepareShadowSampling(s, shadowMappingUniforms);
}

void ColorPassDescriptorSet::prepareShadowPCSS(Handle<HwTexture> texture,
        ShadowMappingUniforms const& shadowMappingUniforms,
        SoftShadowOptions const& options) noexcept {
    setSampler(+PerViewBindingPoints::SHADOW_MAP, texture, {});
    auto& s = mUniforms.edit();
    s.shadowSamplingType = SHADOW_SAMPLING_RUNTIME_PCSS;
    s.shadowPenumbraRatioScale = options.penumbraRatioScale;
    prepareShadowSampling(s, shadowMappingUniforms);
}

void ColorPassDescriptorSet::prepareShadowPCFDebug(Handle<HwTexture> texture,
        ShadowMappingUniforms const& shadowMappingUniforms) noexcept {
    setSampler(+PerViewBindingPoints::SHADOW_MAP, texture, {
            .filterMag = SamplerMagFilter::NEAREST,
            .filterMin = SamplerMinFilter::NEAREST
    });
    auto& s = mUniforms.edit();
    s.shadowSamplingType = SHADOW_SAMPLING_RUNTIME_PCF;
    prepareShadowSampling(s, shadowMappingUniforms);
}

void ColorPassDescriptorSet::commit(DriverApi& driver) noexcept {
    if (mUniforms.isDirty()) {
        driver.updateBufferObject(mUniforms.getUboHandle(),
                mUniforms.toBufferDescriptor(driver), 0);
    }
    for (size_t i = 0; i < DESCRIPTOR_LAYOUT_COUNT; i++) {
        mDescriptorSet[i].commit(mDescriptorSetLayout[i], driver);
    }
}

void ColorPassDescriptorSet::unbindSamplers(DriverApi&) noexcept {
    // this needs to reset the sampler that are only set in RendererUtils::colorPass(), because
    // this descriptor-set is also used for ssr/picking/structure and these could be stale
    // it would be better to use a separate descriptor-set for those two cases so that we don't
    // have to do this
    setSampler(+PerViewBindingPoints::STRUCTURE, {}, {});
    setSampler(+PerViewBindingPoints::SHADOW_MAP, {}, {});
    setSampler(+PerViewBindingPoints::SSAO, {}, {});
    setSampler(+PerViewBindingPoints::SSR, {}, {});
}

void ColorPassDescriptorSet::setSampler(descriptor_binding_t const binding,
        TextureHandle th, SamplerParams const params) noexcept {
    for (size_t i = 0; i < DESCRIPTOR_LAYOUT_COUNT; i++) {
        auto samplers = mDescriptorSetLayout[i].getSamplerDescriptors();
        if (samplers[binding]) {
            mDescriptorSet[i].setSampler(binding, th, params);
        }
    }
}

void ColorPassDescriptorSet::setBuffer(descriptor_binding_t const binding,
        BufferObjectHandle boh, uint32_t const offset, uint32_t const size) noexcept {
    for (size_t i = 0; i < DESCRIPTOR_LAYOUT_COUNT; i++) {
        auto ubos = mDescriptorSetLayout[i].getUniformBufferDescriptors();
        if (ubos[binding]) {
            mDescriptorSet[i].setBuffer(binding, boh, offset, size);
        }
    }
}

} // namespace filament

