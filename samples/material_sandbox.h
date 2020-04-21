/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef TNT_FILAMENT_SAMPLES_MATERIAL_SANDBOX_H
#define TNT_FILAMENT_SAMPLES_MATERIAL_SANDBOX_H

#include <filament/Color.h>
#include <filament/Engine.h>
#include <filament/LightManager.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/View.h>

#include <utils/Entity.h>
#include <utils/EntityManager.h>

#include <math/vec3.h>
#include <math/vec4.h>

#include "generated/resources/resources.h"

constexpr uint8_t MATERIAL_MODEL_UNLIT =       0;
constexpr uint8_t MATERIAL_MODEL_LIT =         1;
constexpr uint8_t MATERIAL_MODEL_SUBSURFACE =  2;
constexpr uint8_t MATERIAL_MODEL_CLOTH =       3;
constexpr uint8_t MATERIAL_MODEL_SPECGLOSS =   4;

constexpr uint8_t MATERIAL_UNLIT                = 0;
constexpr uint8_t MATERIAL_LIT                  = 1;
constexpr uint8_t MATERIAL_SUBSURFACE           = 2;
constexpr uint8_t MATERIAL_CLOTH                = 3;
constexpr uint8_t MATERIAL_SPECGLOSS            = 4;
constexpr uint8_t MATERIAL_TRANSPARENT          = 5;
constexpr uint8_t MATERIAL_FADE                 = 6;
constexpr uint8_t MATERIAL_THIN_REFRACTION      = 7;
constexpr uint8_t MATERIAL_SOLID_REFRACTION     = 8;
constexpr uint8_t MATERIAL_THIN_SS_REFRACTION   = 9;
constexpr uint8_t MATERIAL_SOLID_SS_REFRACTION  = 10;
constexpr uint8_t MATERIAL_COUNT                = 11;

constexpr uint8_t BLENDING_OPAQUE           = 0;
constexpr uint8_t BLENDING_TRANSPARENT      = 1;
constexpr uint8_t BLENDING_FADE             = 2;
constexpr uint8_t BLENDING_THIN_REFRACTION  = 3;
constexpr uint8_t BLENDING_SOLID_REFRACTION = 4;

struct SandboxParameters {
    const filament::Material* material[MATERIAL_COUNT];
    filament::MaterialInstance* materialInstance[MATERIAL_COUNT];
    filament::sRGBColor color = {0.69f, 0.69f, 0.69f};
    float alpha = 1.0f;
    float roughness = 0.6f;
    float metallic = 0.0f;
    float reflectance = 0.5f;
    float clearCoat = 0.0f;
    float clearCoatRoughness = 0.0f;
    float anisotropy = 0.0f;
    float thickness = 1.0f;
    float subsurfacePower = 12.234f;
    float glossiness = 0.0f;
    float specularAntiAliasingVariance = 0.0f;
    float specularAntiAliasingThreshold = 0.0f;
    float transmission = 1.0f;
    float distance = 1.0f;
    float ior = 1.5;
    float emissiveExposureWeight = 1.0f;
    float emissiveEV = 0.0f;
    filament::sRGBColor transmittanceColor =  { 1.0f };
    filament::sRGBColor specularColor = { 0.0f };
    filament::sRGBColor subsurfaceColor = { 0.0f };
    filament::sRGBColor sheenColor = { 0.83f, 0.0f, 0.0f };
    filament::sRGBColor emissiveColor = { 0.0f, 0.0f, 0.0f };
    int currentMaterialModel = MATERIAL_MODEL_LIT;
    int currentBlending = BLENDING_OPAQUE;
    bool ssr = false;
    bool castShadows = true;
    filament::sRGBColor lightColor = {0.98f, 0.92f, 0.89f};
    float lightIntensity = 110000.0f;
    filament::math::float3 lightDirection = {0.6f, -1.0f, -0.8f};
    float iblIntensity = 30000.0f;
    float iblRotation = 0.0f;
    float sunHaloSize = 10.0f;
    float sunHaloFalloff = 80.0f;
    float sunAngularRadius = 1.9f;
    bool directionalLightEnabled = true;
    utils::Entity light;
    utils::Entity spotLight;
    bool hasSpotLight = false;
    bool spotLightEnabled = false;
    filament::sRGBColor spotLightColor = {1.0f, 1.0f, 1.0f};
    float spotLightIntensity = 200000.0f;
    bool spotLightCastShadows = true;
    filament::math::float3 spotLightPosition;
    float spotLightConeAngle = 3.14159 / 4.0f;
    float spotLightConeFade = 0.9f;
    bool hasDirectionalLight = true;
    bool fxaa = true;
    bool tonemapping = true;
    bool msaa = false;
    bool dithering = true;
    bool stableShadowMap = false;
    float normalBias = 1.0;
    float constantBias = 0.001;
    float polygonOffsetConstant = 0.5;
    float polygonOffsetSlope = 2.0;
    bool ssao = false;
    filament::View::AmbientOcclusionOptions ssaoOptions;
    filament::View::BloomOptions bloomOptions;
    filament::View::FogOptions fogOptions;
    bool screenSpaceContactShadows = false;
    int stepCount = 8;
    float maxShadowDistance = 0.3;
    float cameraAperture = 16.0f;
    float cameraSpeed = 125.0f;
    float cameraISO = 100.0f;
};

inline void createInstances(SandboxParameters& params, filament::Engine& engine) {
    using namespace filament;
    using namespace utils;
    params.material[MATERIAL_UNLIT] = Material::Builder()
            .package(RESOURCES_SANDBOXUNLIT_DATA, RESOURCES_SANDBOXUNLIT_SIZE)
            .build(engine);
    params.materialInstance[MATERIAL_UNLIT] =
            params.material[MATERIAL_UNLIT]->createInstance();

    params.material[MATERIAL_LIT] = Material::Builder()
            .package(RESOURCES_SANDBOXLIT_DATA, RESOURCES_SANDBOXLIT_SIZE)
            .build(engine);
    params.materialInstance[MATERIAL_LIT] =
            params.material[MATERIAL_LIT]->createInstance();

    params.material[MATERIAL_TRANSPARENT] = Material::Builder()
            .package(RESOURCES_SANDBOXLITTRANSPARENT_DATA, RESOURCES_SANDBOXLITTRANSPARENT_SIZE)
            .build(engine);
    params.materialInstance[MATERIAL_TRANSPARENT] =
            params.material[MATERIAL_TRANSPARENT]->createInstance();

    params.material[MATERIAL_FADE] = Material::Builder()
            .package(RESOURCES_SANDBOXLITFADE_DATA, RESOURCES_SANDBOXLITFADE_SIZE)
            .build(engine);
    params.materialInstance[MATERIAL_FADE] =
            params.material[MATERIAL_FADE]->createInstance();

    params.material[MATERIAL_THIN_REFRACTION] = Material::Builder()
            .package(RESOURCES_SANDBOXLITTHINREFRACTION_DATA, RESOURCES_SANDBOXLITTHINREFRACTION_SIZE)
            .build(engine);
    params.materialInstance[MATERIAL_THIN_REFRACTION] =
            params.material[MATERIAL_THIN_REFRACTION]->createInstance();

    params.material[MATERIAL_THIN_SS_REFRACTION] = Material::Builder()
            .package(RESOURCES_SANDBOXLITTHINREFRACTIONSSR_DATA, RESOURCES_SANDBOXLITTHINREFRACTIONSSR_SIZE)
            .build(engine);
    params.materialInstance[MATERIAL_THIN_SS_REFRACTION] =
            params.material[MATERIAL_THIN_SS_REFRACTION]->createInstance();

    params.material[MATERIAL_SOLID_REFRACTION] = Material::Builder()
            .package(RESOURCES_SANDBOXLITSOLIDREFRACTION_DATA, RESOURCES_SANDBOXLITSOLIDREFRACTION_SIZE)
            .build(engine);
    params.materialInstance[MATERIAL_SOLID_REFRACTION] =
            params.material[MATERIAL_SOLID_REFRACTION]->createInstance();

    params.material[MATERIAL_SOLID_SS_REFRACTION] = Material::Builder()
            .package(RESOURCES_SANDBOXLITSOLIDREFRACTIONSSR_DATA, RESOURCES_SANDBOXLITSOLIDREFRACTIONSSR_SIZE)
            .build(engine);
    params.materialInstance[MATERIAL_SOLID_SS_REFRACTION] =
            params.material[MATERIAL_SOLID_SS_REFRACTION]->createInstance();

    params.material[MATERIAL_SUBSURFACE] = Material::Builder()
            .package(RESOURCES_SANDBOXSUBSURFACE_DATA, RESOURCES_SANDBOXSUBSURFACE_SIZE)
            .build(engine);
    params.materialInstance[MATERIAL_SUBSURFACE] =
            params.material[MATERIAL_SUBSURFACE]->createInstance();

    params.material[MATERIAL_CLOTH] = Material::Builder()
            .package(RESOURCES_SANDBOXCLOTH_DATA, RESOURCES_SANDBOXCLOTH_SIZE)
            .build(engine);
    params.materialInstance[MATERIAL_CLOTH] =
            params.material[MATERIAL_CLOTH]->createInstance();

    params.material[MATERIAL_SPECGLOSS] = Material::Builder()
            .package(RESOURCES_SANDBOXSPECGLOSS_DATA, RESOURCES_SANDBOXSPECGLOSS_SIZE)
            .build(engine);
    params.materialInstance[MATERIAL_SPECGLOSS] =
            params.material[MATERIAL_SPECGLOSS]->createInstance();

    params.light = EntityManager::get().create();
    LightManager::Builder(LightManager::Type::SUN)
            .color(Color::toLinear<ACCURATE>(params.lightColor))
            .intensity(params.lightIntensity)
            .direction(normalize(params.lightDirection))
            .castShadows(true)
            .sunAngularRadius(params.sunAngularRadius)
            .sunHaloSize(params.sunHaloSize)
            .sunHaloFalloff(params.sunHaloFalloff)
            .build(engine, params.light);

    params.spotLight = EntityManager::get().create();
    LightManager::Builder(LightManager::Type::SPOT)
            .color(Color::toLinear<ACCURATE>(params.spotLightColor))
            .intensity(params.spotLightIntensity)
            .direction({0.0f, -1.0f, 0.0f})
            .spotLightCone(params.spotLightConeAngle * params.spotLightConeFade,
                    params.spotLightConeAngle)
            .castShadows(params.spotLightCastShadows)
            .falloff(10.0f)
            .build(engine, params.spotLight);
}

#endif // TNT_FILAMENT_SAMPLES_MATERIAL_SANDBOX_H
