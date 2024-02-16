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
    return downcast(this)->createInstance(name);
}

const char* Material::getName() const noexcept {
    return downcast(this)->getName().c_str_safe();
}

Shading Material::getShading()  const noexcept {
    return downcast(this)->getShading();
}

Interpolation Material::getInterpolation() const noexcept {
    return downcast(this)->getInterpolation();
}

BlendingMode Material::getBlendingMode() const noexcept {
    return downcast(this)->getBlendingMode();
}

VertexDomain Material::getVertexDomain() const noexcept {
    return downcast(this)->getVertexDomain();
}

MaterialDomain Material::getMaterialDomain() const noexcept {
    return downcast(this)->getMaterialDomain();
}

CullingMode Material::getCullingMode() const noexcept {
    return downcast(this)->getCullingMode();
}

TransparencyMode Material::getTransparencyMode() const noexcept {
    return downcast(this)->getTransparencyMode();
}

bool Material::isColorWriteEnabled() const noexcept {
    return downcast(this)->isColorWriteEnabled();
}

bool Material::isDepthWriteEnabled() const noexcept {
    return downcast(this)->isDepthWriteEnabled();
}

bool Material::isDepthCullingEnabled() const noexcept {
    return downcast(this)->isDepthCullingEnabled();
}

bool Material::isDoubleSided() const noexcept {
    return downcast(this)->isDoubleSided();
}

bool Material::isAlphaToCoverageEnabled() const noexcept {
    return downcast(this)->isAlphaToCoverageEnabled();
}

float Material::getMaskThreshold() const noexcept {
    return downcast(this)->getMaskThreshold();
}

bool Material::hasShadowMultiplier() const noexcept {
    return downcast(this)->hasShadowMultiplier();
}

bool Material::hasSpecularAntiAliasing() const noexcept {
    return downcast(this)->hasSpecularAntiAliasing();
}

float Material::getSpecularAntiAliasingVariance() const noexcept {
    return downcast(this)->getSpecularAntiAliasingVariance();
}

float Material::getSpecularAntiAliasingThreshold() const noexcept {
    return downcast(this)->getSpecularAntiAliasingThreshold();
}

size_t Material::getParameterCount() const noexcept {
    return downcast(this)->getParameterCount();
}

size_t Material::getParameters(ParameterInfo* parameters, size_t count) const noexcept {
    return downcast(this)->getParameters(parameters, count);
}

AttributeBitset Material::getRequiredAttributes() const noexcept {
    return downcast(this)->getRequiredAttributes();
}

RefractionMode Material::getRefractionMode() const noexcept {
    return downcast(this)->getRefractionMode();
}

RefractionType Material::getRefractionType() const noexcept {
    return downcast(this)->getRefractionType();
}

ReflectionMode Material::getReflectionMode() const noexcept {
    return downcast(this)->getReflectionMode();
}

FeatureLevel Material::getFeatureLevel() const noexcept {
    return downcast(this)->getFeatureLevel();
}

bool Material::hasParameter(const char* name) const noexcept {
    return downcast(this)->hasParameter(name);
}

bool Material::isSampler(const char* name) const noexcept {
    return downcast(this)->isSampler(name);
}

MaterialInstance* Material::getDefaultInstance() noexcept {
    return downcast(this)->getDefaultInstance();
}

MaterialInstance const* Material::getDefaultInstance() const noexcept {
    return downcast(this)->getDefaultInstance();
}

void Material::compile(CompilerPriorityQueue priority, UserVariantFilterMask variantFilter,
        backend::CallbackHandler* handler, utils::Invocable<void(Material*)>&& callback) noexcept {
    downcast(this)->compile(priority, variantFilter, handler, std::move(callback));
}

UserVariantFilterMask Material::getSupportedVariants() const noexcept {
    return downcast(this)->getSupportedVariants();
}


} // namespace filament
