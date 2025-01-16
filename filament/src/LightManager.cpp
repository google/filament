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

#include "components/LightManager.h"

using namespace utils;

namespace filament {

using namespace math;

bool LightManager::hasComponent(Entity const e) const noexcept {
    return downcast(this)->hasComponent(e);
}

size_t LightManager::getComponentCount() const noexcept {
    return downcast(this)->getComponentCount();
}

bool LightManager::empty() const noexcept {
    return downcast(this)->empty();
}

Entity LightManager::getEntity(Instance const i) const noexcept {
    return downcast(this)->getEntity(i);
}

Entity const* LightManager::getEntities() const noexcept {
    return downcast(this)->getEntities();
}

LightManager::Instance LightManager::getInstance(Entity const e) const noexcept {
    return downcast(this)->getInstance(e);
}

void LightManager::destroy(Entity const e) noexcept {
    return downcast(this)->destroy(e);
}

void LightManager::setLightChannel(Instance const i, unsigned int const channel, bool const enable) noexcept {
    downcast(this)->setLightChannel(i, channel, enable);
}

bool LightManager::getLightChannel(Instance const i, unsigned int const channel) const noexcept {
    return downcast(this)->getLightChannel(i, channel);
}

void LightManager::setPosition(Instance const i, const float3& position) noexcept {
    downcast(this)->setLocalPosition(i, position);
}

const float3& LightManager::getPosition(Instance const i) const noexcept {
    return downcast(this)->getLocalPosition(i);
}

void LightManager::setDirection(Instance const i, const float3& direction) noexcept {
    downcast(this)->setLocalDirection(i, direction);
}

const float3& LightManager::getDirection(Instance const i) const noexcept {
    return downcast(this)->getLocalDirection(i);
}

void LightManager::setColor(Instance const i, const LinearColor& color) noexcept {
    downcast(this)->setColor(i, color);
}

const float3& LightManager::getColor(Instance const i) const noexcept {
    return downcast(this)->getColor(i);
}

void LightManager::setIntensity(Instance const i, float const intensity) noexcept {
    downcast(this)->setIntensity(i, intensity, FLightManager::IntensityUnit::LUMEN_LUX);
}

void LightManager::setIntensityCandela(Instance const i, float const intensity) noexcept {
    downcast(this)->setIntensity(i, intensity, FLightManager::IntensityUnit::CANDELA);
}

float LightManager::getIntensity(Instance const i) const noexcept {
    return downcast(this)->getIntensity(i);
}

void LightManager::setFalloff(Instance const i, float const radius) noexcept {
    downcast(this)->setFalloff(i, radius);
}

float LightManager::getFalloff(Instance const i) const noexcept {
    return downcast(this)->getFalloff(i);
}

void LightManager::setSpotLightCone(Instance const i, float const inner, float const outer) noexcept {
    downcast(this)->setSpotLightCone(i, inner, outer);
}

float LightManager::getSpotLightOuterCone(Instance const i) const noexcept {
    return downcast(this)->getSpotParams(i).outerClamped;
}

float LightManager::getSpotLightInnerCone(Instance const i) const noexcept {
    return downcast(this)->getSpotLightInnerCone(i);
}

void LightManager::setSunAngularRadius(Instance const i, float const angularRadius) noexcept {
    downcast(this)->setSunAngularRadius(i, angularRadius);
}

float LightManager::getSunAngularRadius(Instance const i) const noexcept {
    float radius = downcast(this)->getSunAngularRadius(i);
    return radius * f::RAD_TO_DEG;
}

void LightManager::setSunHaloSize(Instance const i, float const haloSize) noexcept {
    downcast(this)->setSunHaloSize(i, haloSize);
}

float LightManager::getSunHaloSize(Instance const i) const noexcept {
    return downcast(this)->getSunHaloSize(i);
}

void LightManager::setSunHaloFalloff(Instance const i, float const haloFalloff) noexcept {
    downcast(this)->setSunHaloFalloff(i, haloFalloff);
}

float LightManager::getSunHaloFalloff(Instance const i) const noexcept {
    return downcast(this)->getSunHaloFalloff(i);
}

LightManager::Type LightManager::getType(Instance const i) const noexcept {
    return downcast(this)->getType(i);
}

const LightManager::ShadowOptions& LightManager::getShadowOptions(Instance const i) const noexcept {
    return downcast(this)->getShadowOptions(i);
}

void LightManager::setShadowOptions(Instance const i, ShadowOptions const& options) noexcept {
    downcast(this)->setShadowOptions(i, options);
}

bool LightManager::isShadowCaster(Instance const i) const noexcept {
    return downcast(this)->isShadowCaster(i);
}

void LightManager::setShadowCaster(Instance const i, bool const castShadows) noexcept {
    downcast(this)->setShadowCaster(i, castShadows);
}

} // namespace filament
