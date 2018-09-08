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

#include <filament/LightManager.h>

#include "API.h"

using namespace filament;
using namespace utils;

FBool Filament_LightManager_HasComponent(LightManager *lm, FEntity entity) {
    return lm->hasComponent(convertEntity(entity));
}

int Filament_LightManager_GetInstance(LightManager *lm, FEntity entity) {
    return lm->getInstance(convertEntity(entity));
}

void Filament_LightManager_Destroy(LightManager *lm, FEntity entity) {
    lm->destroy(convertEntity(entity));
}

LightManager::Builder *Filament_LightManager_CreateBuilder(LightManager::Type lightType) {
    return new LightManager::Builder(lightType);
}

void Filament_LightManager_DestroyBuilder(LightManager::Builder *builder) {
    delete builder;
}

void Filament_LightManager_BuilderCastShadows(LightManager::Builder *builder, FBool enable) {
    builder->castShadows(enable);
}

void Filament_LightManager_BuilderShadowOptions(
        LightManager::Builder *builder, uint32_t mapSize, float constantBias,
        float normalBias, float shadowFar) {
    builder->shadowOptions(LightManager::ShadowOptions{mapSize, constantBias,
                                                       normalBias, shadowFar});
}

void Filament_LightManager_BuilderCastLight(LightManager::Builder *builder, FBool enabled) {
    builder->castLight(enabled);
}

void Filament_LightManager_BuilderPosition(LightManager::Builder *builder, math::float3 *position) {
    builder->position(*position);
}

void Filament_LightManager_BuilderDirection(LightManager::Builder *builder, math::float3 *direction) {
    builder->direction(*direction);
}

void Filament_LightManager_BuilderColor(LightManager::Builder *builder, LinearColor color) {
    builder->color(color);
}

void
Filament_LightManager_BuilderIntensity(LightManager::Builder *builder, float intensity) {
    builder->intensity(intensity);
}

void Filament_LightManager_BuilderIntensityWatts(
        LightManager::Builder *builder, float watts, float efficiency) {
    builder->intensity(watts, efficiency);
}

void
Filament_LightManager_BuilderFalloff(LightManager::Builder *builder, float radius) {
    builder->falloff(radius);
}

void Filament_LightManager_BuilderSpotLightCone(
        LightManager::Builder *builder, float inner, float outer) {
    builder->spotLightCone(inner, outer);
}

void Filament_LightManager_BuilderAngularRadius(
        LightManager::Builder *builder, float angularRadius) {
    builder->sunAngularRadius(angularRadius);
}

void
Filament_LightManager_BuilderHaloSize(LightManager::Builder *builder, float haloSize) {
    builder->sunHaloSize(haloSize);
}

void Filament_LightManager_BuilderHaloFalloff(
        LightManager::Builder *builder, float haloFalloff) {
    builder->sunHaloFalloff(haloFalloff);
}

FBool Filament_LightManager_BuilderBuild(LightManager::Builder *builder,
        Engine *engine,
        FEntity entity) {
    return builder->build(*engine, convertEntity(entity)) == LightManager::Builder::Success;
}

void Filament_LightManager_SetPosition(LightManager *lm, int i,
        math::float3 *position) {
    lm->setPosition((LightManager::Instance) i, *position);
}

void Filament_LightManager_GetPosition(LightManager *lm, int i,
        math::float3 *out) {
    *out = lm->getPosition((LightManager::Instance) i);
}

void Filament_LightManager_SetDirection(LightManager *lm, int i,
        math::float3 *direction) {
    lm->setDirection((LightManager::Instance) i, *direction);
}

void Filament_LightManager_GetDirection(LightManager *lm, int i,
        math::float3 *out) {
    *out = lm->getDirection((LightManager::Instance) i);
}

void Filament_LightManager_SetColor(LightManager *lm, int i,
        float linearR, float linearG,
        float linearB) {
    lm->setColor((LightManager::Instance) i, {linearR, linearG, linearB});
}

void Filament_LightManager_GetColor(LightManager *lm, int i,
        math::float3 *out) {
    *out = lm->getColor((LightManager::Instance) i);
}

void Filament_LightManager_SetIntensity(LightManager *lm, int i,
        float intensity) {
    lm->setIntensity((LightManager::Instance) i, intensity);
}

void Filament_LightManager_SetIntensityWatts(LightManager *lm, int i,
        float watts,
        float efficiency) {
    lm->setIntensity((LightManager::Instance) i, watts, efficiency);
}

float Filament_LightManager_GetIntensity(LightManager *lm, int i) {
    return lm->getIntensity((LightManager::Instance) i);
}

void Filament_LightManager_SetFalloff(LightManager *lm, int i,
        float falloff) {
    lm->setFalloff((LightManager::Instance) i, falloff);
}

float Filament_LightManager_GetFalloff(LightManager *lm, int i) {
    return lm->getFalloff((LightManager::Instance) i);
}

void Filament_LightManager_SetSpotLightCone(LightManager *lm, int i,
        float inner, float outer) {
    lm->setSpotLightCone((LightManager::Instance) i, inner, outer);
}

void Filament_LightManager_SetSunAngularRadius(LightManager *lm, int i,
        float angularRadius) {
    lm->setSunAngularRadius((LightManager::Instance) i, angularRadius);
}

float Filament_LightManager_GetSunAngularRadius(LightManager *lm,
        int i) {
    return lm->getSunAngularRadius((LightManager::Instance) i);
}

void Filament_LightManager_SetSunHaloSize(LightManager *lm, int i,
        float haloSize) {
    lm->setSunHaloSize((LightManager::Instance) i, haloSize);
}

float Filament_LightManager_GetHaloSize(LightManager *lm, int i) {
    return lm->getSunHaloSize((LightManager::Instance) i);
}

void Filament_LightManager_SetSunHaloFalloff(LightManager *lm, int i,
        float haloFalloff) {
    lm->setSunHaloFalloff((LightManager::Instance) i, haloFalloff);
}

float Filament_LightManager_GetHaloFalloff(LightManager *lm, int i) {
    return lm->getSunHaloFalloff((LightManager::Instance) i);
}
