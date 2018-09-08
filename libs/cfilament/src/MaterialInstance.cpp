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

#include <filament/MaterialInstance.h>
#include <filament/Texture.h>
#include <filament/TextureSampler.h>

#include <math/mat3.h>
#include <math/mat4.h>
#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include "API.h"

using namespace filament;

void Filament_MaterialInstance_SetParameterBool(MaterialInstance *instance, char *name, FBool x) {
    instance->setParameter(name, bool(x));
}

void Filament_MaterialInstance_SetParameterBool2(MaterialInstance *instance, char *name, FBool x, FBool y) {
    instance->setParameter(name, math::bool2{x, y});
}

void Filament_MaterialInstance_SetParameterBool3(MaterialInstance *instance, char *name, FBool x, FBool y, FBool z) {
    instance->setParameter(name, math::bool3{x, y, z});
}

void Filament_MaterialInstance_SetParameterBool4(MaterialInstance *instance, char *name, FBool x, FBool y, FBool z,
        FBool w) {
    instance->setParameter(name, math::bool4{x, y, z, w});
}

void Filament_MaterialInstance_SetParameterInt(MaterialInstance *instance, char *name, int x) {
    instance->setParameter(name, int32_t(x));
}

void Filament_MaterialInstance_SetParameterInt2(MaterialInstance *instance, char *name, int x, int y) {
    instance->setParameter(name, math::int2{x, y});
}

void Filament_MaterialInstance_SetParameterInt3(MaterialInstance *instance, char *name, int x, int y, int z) {
    instance->setParameter(name, math::int3{x, y, z});
}

void Filament_MaterialInstance_SetParameterInt4(MaterialInstance *instance, char *name, int x, int y, int z, int w) {
    instance->setParameter(name, math::int4{x, y, z, w});
}

void Filament_MaterialInstance_SetParameterFloat(MaterialInstance *instance, char *name, float x) {
    instance->setParameter(name, float(x));
}

void Filament_MaterialInstance_SetParameterFloat2(MaterialInstance *instance, char *name, float x, float y) {
    instance->setParameter(name, math::float2{x, y});
}

void Filament_MaterialInstance_SetParameterFloat3(MaterialInstance *instance, char *name, float x, float y, float z) {
    instance->setParameter(name, math::float3{x, y, z});
}

void Filament_MaterialInstance_SetParameterFloat4(MaterialInstance *instance, char *name, float x, float y, float z,
        float w) {
    instance->setParameter(name, math::float4{x, y, z, w});
}

void Filament_MaterialInstance_SetBooleanParameterArray(
        MaterialInstance *instance, char *name, FBool *v, size_t count) {
    instance->setParameter(name, (bool *) v, count);
}

void Filament_MaterialInstance_SetIntParameterArray(
        MaterialInstance *instance, char *name, int32_t *v, size_t count) {
    instance->setParameter(name, v, count);
}

void Filament_MaterialInstance_SetFloatParameterArray(
        MaterialInstance *instance, char *name, float *v, size_t count) {
    instance->setParameter(name, v, count);
}

void
Filament_MaterialInstance_SetParameterTexture(MaterialInstance *instance, char *name, Texture *texture, int sampler_) {
    auto &sampler = reinterpret_cast<TextureSampler &>(sampler_);
    instance->setParameter(name, texture, sampler);
}

void Filament_MaterialInstance_SetScissor(MaterialInstance *instance,
        uint32_t left, uint32_t bottom,
        uint32_t width, uint32_t height) {
    instance->setScissor(left, bottom, width, height);
}

void Filament_MaterialInstance_UnsetScissor(MaterialInstance *instance) {
    instance->unsetScissor();
}
