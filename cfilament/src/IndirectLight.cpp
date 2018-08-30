/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include <filament/IndirectLight.h>

#include "API.h"

using namespace filament;

IndirectLight::Builder *Filament_IndirectLight_CreateBuilder() {
  return new IndirectLight::Builder();
}

void Filament_IndirectLight_DestroyBuilder(IndirectLight::Builder *builder) {
  delete builder;
}

IndirectLight *Filament_IndirectLight_BuilderBuild(IndirectLight::Builder *builder, Engine *engine) {
  return builder->build(*engine);
}

void Filament_IndirectLight_BuilderReflections(IndirectLight::Builder *builder, Texture *texture) {
  builder->reflections(texture);
}

void Filament_IndirectLight_Irradiance(IndirectLight::Builder *builder,
                                       uint8_t bands,
                                       math::float3 *sh) {
  builder->irradiance(bands, sh);
}

void Filament_IndirectLight_IrradianceAsTexture(IndirectLight::Builder *builder, Texture *texture) {
  builder->irradiance(texture);
}

void Filament_IndirectLight_Intensity(IndirectLight::Builder *builder, float envIntensity) {
  builder->intensity(envIntensity);
}

void Filament_IndirectLight_Rotation(IndirectLight::Builder *builder,
                                     float v0, float v1, float v2,
                                     float v3, float v4, float v5,
                                     float v6, float v7, float v8) {
  builder->rotation(math::mat3f{v0, v1, v2, v3, v4, v5, v6, v7, v8});
}

void Filament_IndirectLight_SetIntensity(IndirectLight *indirectLight, float intensity) {
  indirectLight->setIntensity(intensity);
}

float Filament_IndirectLight_GetIntensity(IndirectLight *indirectLight) {
  return indirectLight->getIntensity();
}

void Filament_IndirectLight_SetRotation(IndirectLight *indirectLight,
                                        float v0, float v1, float v2,
                                        float v3, float v4, float v5,
                                        float v6, float v7, float v8) {
  indirectLight->setRotation(math::mat3f{v0, v1, v2, v3, v4, v5, v6, v7, v8});
}
