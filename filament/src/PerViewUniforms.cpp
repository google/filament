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

#include <utils/Log.h>

namespace filament {

using namespace backend;
using namespace math;

PerViewUniforms::PerViewUniforms(FEngine& engine) noexcept
        : mSamplers(PerViewSib::SAMPLER_COUNT) {
    DriverApi& driver = engine.getDriverApi();

    mSamplerGroupHandle = driver.createSamplerGroup(
            mSamplers.getSize(), utils::FixedSizeString<32>("Per-view samplers"));

    mUniformBufferHandle = driver.createBufferObject(mUniforms.getSize(),
            BufferObjectBinding::UNIFORM, BufferUsage::DYNAMIC);

    if (engine.getDFG().isValid()) {
        TextureSampler const sampler(TextureSampler::MagFilter::LINEAR);
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

void PerViewUniforms::prepareLodBias(float bias, float2 derivativesScale) noexcept {
    auto& s = mUniforms.edit();
    s.lodBias = bias;
    s.derivativesScale = derivativesScale;
}

void PerViewUniforms::prepareExposure(float ev100) noexcept {
    const float exposure = Exposure::exposure(ev100);
    auto& s = mUniforms.edit();
    s.exposure = exposure;
    s.ev100 = ev100;
}

void PerViewUniforms::prepareViewport(
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
    utils::slog.e << "resolution=" << s.resolution << " logical=" <<
            s.logicalViewportScale << " " << s.logicalViewportOffset << utils::io::endl;
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

void PerViewUniforms::prepareFog(FEngine& engine, const CameraInfo& cameraInfo,
        mat4 const& userWorldFromFog, FogOptions const& options, FIndirectLight const* ibl) noexcept {

    auto packHalf2x16 = [](math::half2 v) -> uint32_t {
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
        fogColorTextureHandle = downcast(options.skyColor)->getHwHandle();
        math::half2 const minMaxMip{ 0.0f, float(options.skyColor->getLevels()) - 1.0f };
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
            math::half2 const minMaxMip{ levelCount - 2.0f, levelCount - 1.0f };
            s.fogMinMaxMip = packHalf2x16(minMaxMip);
            s.fogOneOverFarMinusNear = 1.0f / (cameraInfo.zf - cameraInfo.zn);
            s.fogNearOverFarMinusNear = cameraInfo.zn / (cameraInfo.zf - cameraInfo.zn);
        }
    }

    mSamplers.setSampler(PerViewSib::FOG, {
            fogColorTextureHandle ? fogColorTextureHandle : engine.getDummyCubemap()->getHwHandle(), {
                    .filterMag = SamplerMagFilter::LINEAR,
                    .filterMin = SamplerMinFilter::LINEAR_MIPMAP_LINEAR
            }});

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

void PerViewUniforms::prepareMaterialGlobals(
        std::array<math::float4, 4> const& materialGlobals) noexcept {
    mUniforms.edit().custom[0] = materialGlobals[0];
    mUniforms.edit().custom[1] = materialGlobals[1];
    mUniforms.edit().custom[2] = materialGlobals[2];
    mUniforms.edit().custom[3] = materialGlobals[3];
}

void PerViewUniforms::prepareSSR(Handle<HwTexture> ssr,
        bool disableSSR,
        float refractionLodOffset,
        ScreenSpaceReflectionsOptions const& ssrOptions) noexcept {

    mSamplers.setSampler(PerViewSib::SSR, { ssr, {
        .filterMag = SamplerMagFilter::LINEAR,
        .filterMin = SamplerMinFilter::LINEAR_MIPMAP_LINEAR
    }});

    auto& s = mUniforms.edit();
    s.refractionLodOffset = refractionLodOffset;
    s.ssrDistance = (ssrOptions.enabled && !disableSSR) ? ssrOptions.maxDistance : 0.0f;
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
    FLightManager const& lcm = engine.getLightManager();
    auto& s = mUniforms.edit();

    float const shadowFar = lcm.getShadowFar(directionalLight);
    // TODO: make the falloff rate a parameter
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

void PerViewUniforms::prepareAmbientLight(FEngine& engine, FIndirectLight const& ibl,
        float intensity, float exposure) noexcept {
    auto& s = mUniforms.edit();

    // Set up uniforms and sampler for the IBL, guaranteed to be non-null at this point.
    float const iblRoughnessOneLevel = ibl.getLevelCount() - 1.0f;
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
    float const f = froxelizer.getLightFar();
    // TODO: make the falloff rate a parameter
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

void PerViewUniforms::prepareShadowPCFDebug(Handle<HwTexture> texture,
        ShadowMappingUniforms const& shadowMappingUniforms) noexcept {
    mSamplers.setSampler(PerViewSib::SHADOW_MAP, { texture, {
            .filterMag = SamplerMagFilter::NEAREST,
            .filterMin = SamplerMinFilter::NEAREST
    }});
    auto& s = mUniforms.edit();
    s.shadowSamplingType = SHADOW_SAMPLING_RUNTIME_PCF;
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

