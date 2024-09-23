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

bool LightManager::hasComponent(Entity e) const noexcept {
    return downcast(this)->hasComponent(e);
}

size_t LightManager::getComponentCount() const noexcept {
    return downcast(this)->getComponentCount();
}

bool LightManager::empty() const noexcept {
    return downcast(this)->empty();
}

utils::Entity LightManager::getEntity(LightManager::Instance i) const noexcept {
    return downcast(this)->getEntity(i);
}

utils::Entity const* LightManager::getEntities() const noexcept {
    return downcast(this)->getEntities();
}

LightManager::Instance LightManager::getInstance(Entity e) const noexcept {
    return downcast(this)->getInstance(e);
}

void LightManager::destroy(Entity e) noexcept {
    return downcast(this)->destroy(e);
}

void LightManager::setLightChannel(Instance i, unsigned int channel, bool enable) noexcept {
    downcast(this)->setLightChannel(i, channel, enable);
}

bool LightManager::getLightChannel(LightManager::Instance i, unsigned int channel) const noexcept {
    return downcast(this)->getLightChannel(i, channel);
}

void LightManager::setPosition(Instance i, const float3& position) noexcept {
    downcast(this)->setLocalPosition(i, position);
}

const float3& LightManager::getPosition(Instance i) const noexcept {
    return downcast(this)->getLocalPosition(i);
}

void LightManager::setDirection(Instance i, const float3& direction) noexcept {
    downcast(this)->setLocalDirection(i, direction);
}

const float3& LightManager::getDirection(Instance i) const noexcept {
    return downcast(this)->getLocalDirection(i);
}

void LightManager::setColor(Instance i, const LinearColor& color) noexcept {
    downcast(this)->setColor(i, color);
}

const float3& LightManager::getColor(Instance i) const noexcept {
    return downcast(this)->getColor(i);
}

void LightManager::setIntensity(Instance i, float intensity) noexcept {
    downcast(this)->setIntensity(i, intensity, FLightManager::IntensityUnit::LUMEN_LUX);
}

void LightManager::setIntensityCandela(Instance i, float intensity) noexcept {
    downcast(this)->setIntensity(i, intensity, FLightManager::IntensityUnit::CANDELA);
}

float LightManager::getIntensity(Instance i) const noexcept {
    return downcast(this)->getIntensity(i);
}

void LightManager::setFalloff(Instance i, float radius) noexcept {
    downcast(this)->setFalloff(i, radius);
}

float LightManager::getFalloff(Instance i) const noexcept {
    return downcast(this)->getFalloff(i);
}

void LightManager::setSpotLightCone(Instance i, float inner, float outer) noexcept {
    downcast(this)->setSpotLightCone(i, inner, outer);
}

float LightManager::getSpotLightOuterCone(Instance i) const noexcept {
    return downcast(this)->getSpotParams(i).outerClamped;
}

float LightManager::getSpotLightInnerCone(Instance i) const noexcept {
    return downcast(this)->getSpotLightInnerCone(i);
}

void LightManager::setSunAngularRadius(Instance i, float angularRadius) noexcept {
    downcast(this)->setSunAngularRadius(i, angularRadius);
}

float LightManager::getSunAngularRadius(Instance i) const noexcept {
    float radius = downcast(this)->getSunAngularRadius(i);
    return radius * f::RAD_TO_DEG;
}

void LightManager::setSunHaloSize(Instance i, float haloSize) noexcept {
    downcast(this)->setSunHaloSize(i, haloSize);
}

float LightManager::getSunHaloSize(Instance i) const noexcept {
    return downcast(this)->getSunHaloSize(i);
}

void LightManager::setSunHaloFalloff(Instance i, float haloFalloff) noexcept {
    downcast(this)->setSunHaloFalloff(i, haloFalloff);
}

float LightManager::getSunHaloFalloff(Instance i) const noexcept {
    return downcast(this)->getSunHaloFalloff(i);
}

void LightManager::setWidth(Instance i, float width) noexcept {
    downcast(this)->setWidth(i, width);
}

float LightManager::getWidth(Instance i) const noexcept {
    return downcast(this)->getWidth(i);
}

void LightManager::setHeight(Instance i, float height) noexcept {
    downcast(this)->setHeight(i, height);
}

float LightManager::getHeight(Instance i) const noexcept {
    return downcast(this)->getHeight(i);
}

LightManager::Type LightManager::getType(LightManager::Instance i) const noexcept {
    return downcast(this)->getType(i);
}

const LightManager::ShadowOptions& LightManager::getShadowOptions(Instance i) const noexcept {
    return downcast(this)->getShadowOptions(i);
}

void LightManager::setShadowOptions(Instance i, ShadowOptions const& options) noexcept {
    downcast(this)->setShadowOptions(i, options);
}

bool LightManager::isShadowCaster(Instance i) const noexcept {
    return downcast(this)->isShadowCaster(i);
}

void LightManager::setShadowCaster(Instance i, bool castShadows) noexcept {
    downcast(this)->setShadowCaster(i, castShadows);
}

} // namespace filament
