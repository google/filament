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

#include <private/filament/SibGenerator.h>

#include <math/mat4.h>

namespace filament {

using namespace backend;
using namespace math;

PerViewUniforms::PerViewUniforms(FEngine& engine) noexcept
        : mEngine(engine),
          mPerViewSb(PerViewSib::SAMPLER_COUNT) {
    DriverApi& driver = engine.getDriverApi();

    mPerViewSbh = driver.createSamplerGroup(mPerViewSb.getSize());

    mPerViewUbh = driver.createBufferObject(mPerViewUb.getSize(),
            BufferObjectBinding::UNIFORM, BufferUsage::DYNAMIC);

    // with a clip-space of [-w, w] ==> z' = -z
    // with a clip-space of [0,  w] ==> z' = (w - z)/2
    mClipControl = driver.getClipSpaceParams();

    if (engine.getDFG()->isValid()) {
        TextureSampler sampler(TextureSampler::MagFilter::LINEAR);
        mPerViewSb.setSampler(PerViewSib::IBL_DFG_LUT,
                engine.getDFG()->getTexture(), sampler.getSamplerParams());
    }
}

void PerViewUniforms::terminate(FEngine& engine) {
    DriverApi& driver = engine.getDriverApi();
    driver.destroyBufferObject(mPerViewUbh);
    driver.destroySamplerGroup(mPerViewSbh);
}

void PerViewUniforms::prepareCamera(const CameraInfo& camera) noexcept {
    mat4f const& viewFromWorld = camera.view;
    mat4f const& worldFromView = camera.model;
    mat4f const& clipFromView  = camera.projection;

    const mat4f viewFromClip{ inverse((mat4)camera.projection) };
    const mat4f clipFromWorld{ highPrecisionMultiply(clipFromView, viewFromWorld) };
    const mat4f worldFromClip{ highPrecisionMultiply(worldFromView, viewFromClip) };

    auto& s = mPerViewUb.edit();
    s.viewFromWorldMatrix = viewFromWorld;    // view
    s.worldFromViewMatrix = worldFromView;    // model
    s.clipFromViewMatrix  = clipFromView;     // projection
    s.viewFromClipMatrix  = viewFromClip;     // 1/projection
    s.clipFromWorldMatrix = clipFromWorld;    // projection * view
    s.worldFromClipMatrix = worldFromClip;    // 1/(projection * view)
    s.cameraPosition = float3{ camera.getPosition() };
    s.worldOffset = camera.worldOffset;
    s.cameraFar = camera.zf;
    s.oneOverFarMinusNear = 1.0f / (camera.zf - camera.zn);
    s.nearOverFarMinusNear = camera.zn / (camera.zf - camera.zn);
    s.clipControl = mClipControl;
}

void PerViewUniforms::prepareUpscaler(math::float2 scale,
        DynamicResolutionOptions const& options) noexcept {
    auto& s = mPerViewUb.edit();
    if (options.quality >= QualityLevel::HIGH) {
        s.lodBias = std::log2(std::min(scale.x, scale.y));
    } else {
        s.lodBias = 0.0f;
    }
}

void PerViewUniforms::prepareExposure(float ev100) noexcept {
    const float exposure = Exposure::exposure(ev100);
    auto& s = mPerViewUb.edit();
    s.exposure = exposure;
    s.ev100 = ev100;
}

void PerViewUniforms::prepareViewport(const filament::Viewport &viewport) noexcept {
    const float w = viewport.width;
    const float h = viewport.height;
    auto& s = mPerViewUb.edit();
    s.resolution = float4{ w, h, 1.0f / w, 1.0f / h };
    s.origin = float2{ viewport.left, viewport.bottom };
}

void PerViewUniforms::prepareTime(FEngine& engine, float4 const& userTime) noexcept {
    auto& s = mPerViewUb.edit();
    const uint64_t oneSecondRemainder = engine.getEngineTime().count() % 1000000000;
    const float fraction = float(double(oneSecondRemainder) / 1000000000.0);
    s.time = fraction;
    s.userTime = userTime;
}

void PerViewUniforms::prepareFog(const CameraInfo& camera, FogOptions const& options) noexcept {
    // this can't be too high because we need density / heightFalloff to produce something
    // close to fogOptions.density in the fragment shader which use 16-bits floats.
    constexpr float epsilon = 0.001f;
    const float heightFalloff = std::max(epsilon, options.heightFalloff);

    // precalculate the constant part of density  integral and correct for exp2() in the shader
    const float density = ((options.density / heightFalloff) *
            std::exp(-heightFalloff * (camera.getPosition().y - options.height)))
                    * float(1.0f / F_LN2);

    auto& s = mPerViewUb.edit();
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

void PerViewUniforms::prepareSSAO(Handle<HwTexture> ssao, AmbientOcclusionOptions const& options) noexcept {
    // High quality sampling is enabled only if AO itself is enabled and upsampling quality is at
    // least set to high and of course only if upsampling is needed.
    const bool highQualitySampling = options.upsampling >= QualityLevel::HIGH
            && options.resolution < 1.0f;

    // LINEAR filtering is only needed when AO is enabled and low-quality upsampling is used.
    mPerViewSb.setSampler(PerViewSib::SSAO, ssao, {
        .filterMag = options.enabled && !highQualitySampling ?
                SamplerMagFilter::LINEAR : SamplerMagFilter::NEAREST
    });

    const float edgeDistance = 1.0f / options.bilateralThreshold;
    auto& s = mPerViewUb.edit();
    s.aoSamplingQualityAndEdgeDistance = options.enabled && highQualitySampling ? edgeDistance : 0.0f;
    s.aoBentNormals = options.enabled && options.bentNormals ? 1.0f : 0.0f;
}

void PerViewUniforms::prepareSSR(Handle<HwTexture> ssr, float refractionLodOffset) noexcept {
    mPerViewSb.setSampler(PerViewSib::SSR, ssr, {
        .filterMag = SamplerMagFilter::LINEAR,
        .filterMin = SamplerMinFilter::LINEAR_MIPMAP_LINEAR
    });
    auto& s = mPerViewUb.edit();
    s.refractionLodOffset = refractionLodOffset;
}

void PerViewUniforms::prepareStructure(Handle<HwTexture> structure) noexcept {
    // sampler must be NEAREST
    mPerViewSb.setSampler(PerViewSib::STRUCTURE, structure, {});
}

void PerViewUniforms::prepareDirectionalLight(
        float exposure,
        float3 const& sceneSpaceDirection,
        PerViewUniforms::LightManagerInstance directionalLight) noexcept {
    FLightManager& lcm = mEngine.getLightManager();
    auto& s = mPerViewUb.edit();

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

void PerViewUniforms::prepareAmbientLight(FIndirectLight const& ibl,
        float intensity, float exposure) noexcept {
    auto& engine = mEngine;
    auto& s = mPerViewUb.edit();

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
    mPerViewSb.setSampler(PerViewSib::IBL_SPECULAR, {
            reflection, {
                    .filterMag = SamplerMagFilter::LINEAR,
                    .filterMin = SamplerMinFilter::LINEAR_MIPMAP_LINEAR
            }});
}

void PerViewUniforms::prepareDynamicLights(Froxelizer& froxelizer) noexcept {
    auto& s = mPerViewUb.edit();
    froxelizer.updateUniforms(s);
    mPerViewSb.setSampler(PerViewSib::FROXELS, { froxelizer.getFroxelTexture() });
}

void PerViewUniforms::prepareShadowMapping(ShadowMappingUniforms const& shadowMappingUniforms,
        VsmShadowOptions const& options) noexcept {
    auto& s = mPerViewUb.edit();
    s.vsmExponent = options.exponent;  // fp16: max 5.54f, fp32: max 42.0
    s.vsmDepthScale = options.minVarianceScale * 0.01f * options.exponent;
    s.vsmLightBleedReduction = options.lightBleedReduction;

    s.lightFromWorldMatrix = shadowMappingUniforms.lightFromWorldMatrix;
    s.cascadeSplits = shadowMappingUniforms.cascadeSplits;
    s.shadowBias = shadowMappingUniforms.shadowBias;
    s.ssContactShadowDistance = shadowMappingUniforms.ssContactShadowDistance;
    s.directionalShadows = shadowMappingUniforms.directionalShadows;
    s.cascades = shadowMappingUniforms.cascades;
}

void PerViewUniforms::prepareShadowVSM(Handle<HwTexture> texture, VsmShadowOptions const& options) noexcept {
    SamplerMinFilter filterMin = SamplerMinFilter::LINEAR;
    if (options.anisotropy > 0 || options.mipmapping) {
        filterMin = SamplerMinFilter::LINEAR_MIPMAP_LINEAR;
    }
    mPerViewSb.setSampler(PerViewSib::SHADOW_MAP, {
            texture, {
                    .filterMag = SamplerMagFilter::LINEAR,
                    .filterMin = filterMin,
                    .anisotropyLog2 = options.anisotropy,
            }});
}

void PerViewUniforms::prepareShadowPCF(Handle<HwTexture> texture) noexcept {
    mPerViewSb.setSampler(PerViewSib::SHADOW_MAP, {
            texture, {
                    .filterMag = SamplerMagFilter::LINEAR,
                    .filterMin = SamplerMinFilter::LINEAR,
                    .compareMode = SamplerCompareMode::COMPARE_TO_TEXTURE, // ignored for VSM
                    .compareFunc = SamplerCompareFunc::GE                  // ignored for VSM
            }});
}

void PerViewUniforms::commit(backend::DriverApi& driver) noexcept {
    if (mPerViewUb.isDirty()) {
        driver.updateBufferObject(mPerViewUbh, mPerViewUb.toBufferDescriptor(driver), 0);
    }
    if (mPerViewSb.isDirty()) {
        driver.updateSamplerGroup(mPerViewSbh, std::move(mPerViewSb.toCommandStream()));
    }
}

void PerViewUniforms::bind(backend::DriverApi& driver) noexcept {
    driver.bindUniformBuffer(BindingPoints::PER_VIEW, mPerViewUbh);
    driver.bindSamplers(BindingPoints::PER_VIEW, mPerViewSbh);
}

void PerViewUniforms::unbindSamplers() noexcept {
    auto& samplerGroup = mPerViewSb;
    samplerGroup.clearSampler(PerViewSib::SSAO);
    samplerGroup.clearSampler(PerViewSib::SSR);
    samplerGroup.clearSampler(PerViewSib::STRUCTURE);
    samplerGroup.clearSampler(PerViewSib::SHADOW_MAP);
}

} // namespace filament

