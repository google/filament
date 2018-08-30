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

#include <memory>

#include <filament/Material.h>

#include "API.h"

using namespace filament;

Material *Filament_Material_BuilderBuild(Engine *engine, void *buffer,
                                         int size) {
  Material *material = Material::Builder().package(buffer, size).build(*engine);
  return material;
}

MaterialInstance *
Filament_Material_GetDefaultInstance(Material *material) {
  return material->getDefaultInstance();
}

MaterialInstance *
Filament_Material_CreateInstance(Material *material) {
  return material->createInstance();
}

const char *Filament_Material_GetName(Material *material) {
  return material->getName();
}

filament::Shading Filament_Material_GetShading(Material *material) {
  return material->getShading();
}

filament::Interpolation
Filament_Material_GetInterpolation(Material *material) {
  return material->getInterpolation();
}

filament::BlendingMode
Filament_Material_GetBlendingMode(Material *material) {
  return material->getBlendingMode();
}

filament::VertexDomain
Filament_Material_GetVertexDomain(Material *material) {
  return material->getVertexDomain();
}

filament::driver::CullingMode
Filament_Material_GetCullingMode(Material *material) {
  return material->getCullingMode();
}

FBool Filament_Material_IsColorWriteEnabled(Material *material) {
  return material->isColorWriteEnabled();
}

FBool Filament_Material_IsDepthWriteEnabled(Material *material) {
  return material->isDepthWriteEnabled();
}

FBool Filament_Material_IsDepthCullingEnabled(Material *material) {
  return material->isDepthCullingEnabled();
}

FBool Filament_Material_IsDoubleSided(Material *material) {
  return material->isDoubleSided();
}

float Filament_Material_GetMaskThreshold(Material *material) {
  return material->getMaskThreshold();
}

int Filament_Material_GetParameterCount(Material *material) {
  return material->getParameterCount();
}

static_assert(sizeof(FParameter) == 16, "static size is wrong");

void Filament_Material_GetParameters(Material *material,
                                     FParameter *paramsOut,
                                     int count) {
  auto parameters = std::make_unique<Material::ParameterInfo[]>(count);
  size_t received = material->getParameters(&parameters[0], (size_t) count);
  assert(received == count);
  for (int i = 0; i < count; i++) {
    auto &paramIn = parameters[i];
    auto &paramOut = paramsOut[i];
    printf("NAME: %s\n", paramIn.name);
    paramOut.name = paramIn.name;
    paramOut.is_sampler = paramIn.isSampler;
    if (paramIn.isSampler) {
      paramOut.type = paramIn.type;
    } else {
      paramOut.sampler_type = paramIn.samplerType;
    }
    paramOut.count = paramIn.count;
    paramOut.precision = paramIn.precision;
  }
}

uint32_t Filament_Material_GetRequiredAttributes(Material *material) {
  return material->getRequiredAttributes().getValue();
}

FBool Filament_Material_HasParameter(Material *material,
                                    const char *name) {
  return material->hasParameter(name);
}
