/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include "details/Material.h"

namespace filament {

using namespace backend;

MaterialInstance* Material::createInstance(const char* name) const noexcept {
    return upcast(this)->createInstance(name);
}

const char* Material::getName() const noexcept {
    return upcast(this)->getName().c_str();
}

Shading Material::getShading()  const noexcept {
    return upcast(this)->getShading();
}

Interpolation Material::getInterpolation() const noexcept {
    return upcast(this)->getInterpolation();
}

BlendingMode Material::getBlendingMode() const noexcept {
    return upcast(this)->getBlendingMode();
}

VertexDomain Material::getVertexDomain() const noexcept {
    return upcast(this)->getVertexDomain();
}

MaterialDomain Material::getMaterialDomain() const noexcept {
    return upcast(this)->getMaterialDomain();
}

CullingMode Material::getCullingMode() const noexcept {
    return upcast(this)->getCullingMode();
}

TransparencyMode Material::getTransparencyMode() const noexcept {
    return upcast(this)->getTransparencyMode();
}

bool Material::isColorWriteEnabled() const noexcept {
    return upcast(this)->isColorWriteEnabled();
}

bool Material::isDepthWriteEnabled() const noexcept {
    return upcast(this)->isDepthWriteEnabled();
}

bool Material::isDepthCullingEnabled() const noexcept {
    return upcast(this)->isDepthCullingEnabled();
}

bool Material::isDoubleSided() const noexcept {
    return upcast(this)->isDoubleSided();
}

float Material::getMaskThreshold() const noexcept {
    return upcast(this)->getMaskThreshold();
}

bool Material::hasShadowMultiplier() const noexcept {
    return upcast(this)->hasShadowMultiplier();
}

bool Material::hasSpecularAntiAliasing() const noexcept {
    return upcast(this)->hasSpecularAntiAliasing();
}

float Material::getSpecularAntiAliasingVariance() const noexcept {
    return upcast(this)->getSpecularAntiAliasingVariance();
}

float Material::getSpecularAntiAliasingThreshold() const noexcept {
    return upcast(this)->getSpecularAntiAliasingThreshold();
}

size_t Material::getParameterCount() const noexcept {
    return upcast(this)->getParameterCount();
}

size_t Material::getParameters(ParameterInfo* parameters, size_t count) const noexcept {
    return upcast(this)->getParameters(parameters, count);
}

AttributeBitset Material::getRequiredAttributes() const noexcept {
    return upcast(this)->getRequiredAttributes();
}

RefractionMode Material::getRefractionMode() const noexcept {
    return upcast(this)->getRefractionMode();
}

RefractionType Material::getRefractionType() const noexcept {
    return upcast(this)->getRefractionType();
}

ReflectionMode Material::getReflectionMode() const noexcept {
    return upcast(this)->getReflectionMode();
}

bool Material::hasParameter(const char* name) const noexcept {
    return upcast(this)->hasParameter(name);
}

bool Material::isSampler(const char* name) const noexcept {
    return upcast(this)->isSampler(name);
}

MaterialInstance* Material::getDefaultInstance() noexcept {
    return upcast(this)->getDefaultInstance();
}

MaterialInstance const* Material::getDefaultInstance() const noexcept {
    return upcast(this)->getDefaultInstance();
}

} // namespace filament
