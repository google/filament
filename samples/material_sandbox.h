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

#include <utils/Entity.h>
#include <utils/EntityManager.h>

#include <math/vec3.h>

static constexpr uint8_t MATERIAL_UNLIT_PACKAGE[] = {
    #include "generated/material/sandboxUnlit.inc"
};

static constexpr uint8_t MATERIAL_LIT_PACKAGE[] = {
    #include "generated/material/sandboxLit.inc"
};

static constexpr uint8_t MATERIAL_LIT_FADE_PACKAGE[] = {
    #include "generated/material/sandboxLitFade.inc"
};

static constexpr uint8_t MATERIAL_LIT_TRANSPARENT_PACKAGE[] = {
    #include "generated/material/sandboxLitTransparent.inc"
};

static constexpr uint8_t MATERIAL_SUBSURFACE_PACKAGE[] = {
    #include "generated/material/sandboxSubsurface.inc"
};

static constexpr uint8_t MATERIAL_CLOTH_PACKAGE[] = {
    #include "generated/material/sandboxCloth.inc"
};

constexpr uint8_t MATERIAL_MODEL_UNLIT =       0;
constexpr uint8_t MATERIAL_MODEL_LIT =         1;
constexpr uint8_t MATERIAL_MODEL_SUBSURFACE =  2;
constexpr uint8_t MATERIAL_MODEL_CLOTH =       3;

constexpr uint8_t MATERIAL_UNLIT =       0;
constexpr uint8_t MATERIAL_LIT =         1;
constexpr uint8_t MATERIAL_SUBSURFACE =  2;
constexpr uint8_t MATERIAL_CLOTH =       3;
constexpr uint8_t MATERIAL_TRANSPARENT = 4;
constexpr uint8_t MATERIAL_FADE =        5;
constexpr uint8_t MATERIAL_COUNT =       6;

constexpr uint8_t BLENDING_OPAQUE      = 0;
constexpr uint8_t BLENDING_TRANSPARENT = 1;
constexpr uint8_t BLENDING_FADE        = 2;

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
    filament::sRGBColor subsurfaceColor = {0.0f};
    filament::sRGBColor sheenColor = {0.83f, 0.0f, 0.0f};
    int currentMaterialModel = MATERIAL_MODEL_LIT;
    int currentBlending = BLENDING_OPAQUE;
    bool castShadows = true;
    filament::sRGBColor lightColor = {0.98f, 0.92f, 0.89f};
    float lightIntensity = 110000.0f;
    math::float3 lightDirection = {0.6f, -1.0f, -0.8f};
    float iblIntensity = 30000.0f;
    float iblRotation = 0.0f;
    float sunHaloSize = 10.0f;
    float sunHaloFalloff = 80.0f;
    float sunAngularRadius = 1.9f;
    bool directionalLightEnabled = true;
    utils::Entity light;
    bool hasDirectionalLight = true;
};

inline void createInstances(SandboxParameters& params, filament::Engine& engine) {
    using namespace filament;
    using namespace utils;
    params.material[MATERIAL_UNLIT] = Material::Builder()
            .package((void*) MATERIAL_UNLIT_PACKAGE, sizeof(MATERIAL_UNLIT_PACKAGE))
            .build(engine);
    params.materialInstance[MATERIAL_UNLIT] =
            params.material[MATERIAL_UNLIT]->createInstance();

    params.material[MATERIAL_LIT] = Material::Builder()
            .package((void*) MATERIAL_LIT_PACKAGE, sizeof(MATERIAL_LIT_PACKAGE))
            .build(engine);
    params.materialInstance[MATERIAL_LIT] =
            params.material[MATERIAL_LIT]->createInstance();

    params.material[MATERIAL_TRANSPARENT] = Material::Builder()
            .package((void*) MATERIAL_LIT_TRANSPARENT_PACKAGE,
                    sizeof(MATERIAL_LIT_TRANSPARENT_PACKAGE))
            .build(engine);
    params.materialInstance[MATERIAL_TRANSPARENT] =
            params.material[MATERIAL_TRANSPARENT]->createInstance();

    params.material[MATERIAL_FADE] = Material::Builder()
            .package((void*) MATERIAL_LIT_FADE_PACKAGE, sizeof(MATERIAL_LIT_FADE_PACKAGE))
            .build(engine);
    params.materialInstance[MATERIAL_FADE] =
            params.material[MATERIAL_FADE]->createInstance();

    params.material[MATERIAL_SUBSURFACE] = Material::Builder()
            .package((void*) MATERIAL_SUBSURFACE_PACKAGE, sizeof(MATERIAL_SUBSURFACE_PACKAGE))
            .build(engine);
    params.materialInstance[MATERIAL_SUBSURFACE] =
            params.material[MATERIAL_SUBSURFACE]->createInstance();

    params.material[MATERIAL_CLOTH] = Material::Builder()
            .package((void*) MATERIAL_CLOTH_PACKAGE, sizeof(MATERIAL_CLOTH_PACKAGE))
            .build(engine);
    params.materialInstance[MATERIAL_CLOTH] =
            params.material[MATERIAL_CLOTH]->createInstance();

    params.light = EntityManager::get().create();
    LightManager::Builder(LightManager::Type::SUN)
            .color(Color::toLinear<ACCURATE>(params.lightColor))
            .intensity(params.lightIntensity)
            .direction(params.lightDirection)
            .castShadows(true)
            .sunAngularRadius(params.sunAngularRadius)
            .sunHaloSize(params.sunHaloSize)
            .sunHaloFalloff(params.sunHaloFalloff)
            .build(engine, params.light);
}

inline filament::MaterialInstance* updateInstances(SandboxParameters& params,
        filament::Engine& engine) {
    using namespace filament;
    int material = params.currentMaterialModel;
    if (material == MATERIAL_MODEL_LIT) {
        if (params.currentBlending == BLENDING_TRANSPARENT) material = MATERIAL_TRANSPARENT;
        if (params.currentBlending == BLENDING_FADE) material = MATERIAL_FADE;
    }
    MaterialInstance* materialInstance = params.materialInstance[material];
    if (params.currentMaterialModel == MATERIAL_MODEL_UNLIT) {
        materialInstance->setParameter("baseColor", RgbType::sRGB, params.color);
    }
    if (params.currentMaterialModel == MATERIAL_MODEL_LIT) {
        materialInstance->setParameter("baseColor", RgbType::sRGB, params.color);
        materialInstance->setParameter("roughness", params.roughness);
        materialInstance->setParameter("metallic", params.metallic);
        materialInstance->setParameter("reflectance", params.reflectance);
        materialInstance->setParameter("clearCoat", params.clearCoat);
        materialInstance->setParameter("clearCoatRoughness", params.clearCoatRoughness);
        materialInstance->setParameter("anisotropy", params.anisotropy);
        if (params.currentBlending != BLENDING_OPAQUE) {
            materialInstance->setParameter("alpha", params.alpha);
        }
    }
    if (params.currentMaterialModel == MATERIAL_MODEL_SUBSURFACE) {
        materialInstance->setParameter("baseColor", RgbType::sRGB, params.color);
        materialInstance->setParameter("roughness", params.roughness);
        materialInstance->setParameter("metallic", params.metallic);
        materialInstance->setParameter("reflectance", params.reflectance);
        materialInstance->setParameter("thickness", params.thickness);
        materialInstance->setParameter("subsurfacePower", params.subsurfacePower);
        materialInstance->setParameter("subsurfaceColor", RgbType::sRGB, params.subsurfaceColor);
    }
    if (params.currentMaterialModel == MATERIAL_MODEL_CLOTH) {
        materialInstance->setParameter("baseColor", RgbType::sRGB, params.color);
        materialInstance->setParameter("roughness", params.roughness);
        materialInstance->setParameter("sheenColor", RgbType::sRGB, params.sheenColor);
        materialInstance->setParameter("subsurfaceColor", RgbType::sRGB, params.subsurfaceColor);
    }

    auto& lcm = engine.getLightManager();
    auto lightInstance = lcm.getInstance(params.light);
    lcm.setColor(lightInstance, params.lightColor);
    lcm.setIntensity(lightInstance, params.lightIntensity);
    lcm.setDirection(lightInstance, params.lightDirection);
    lcm.setSunAngularRadius(lightInstance, params.sunAngularRadius);
    lcm.setSunHaloSize(lightInstance, params.sunHaloSize);
    lcm.setSunHaloFalloff(lightInstance, params.sunHaloFalloff);
    return materialInstance;
}

#endif // TNT_FILAMENT_SAMPLES_MATERIAL_SANDBOX_H
