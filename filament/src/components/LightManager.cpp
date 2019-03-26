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

#include "FilamentAPI-impl.h"

#include "components/LightManager.h"

#include "details/Engine.h"

#include <math/fast.h>
#include <math/scalar.h>
#include <filament/LightManager.h>


using namespace filament::math;
using namespace utils;

namespace filament {

using namespace details;

// ------------------------------------------------------------------------------------------------

struct LightManager::BuilderDetails {
    Type mType;
    bool mCastShadows = false;
    bool mCastLight = true;
    float3 mPosition = {};
    float mFalloff = 1.0f;
    LinearColor mColor = LinearColor{ 1.0f };
    float mIntensity = 100000.0f;
    float3 mDirection = { 0.0f, -1.0f, 0.0f };
    float2 mSpotInnerOuter = { (float)M_PI, (float)M_PI };
    float mSunAngle = 0.00951f; // 0.545° in radians
    float mSunHaloSize = 10.0f;
    float mSunHaloFalloff = 80.0f;
    ShadowOptions mShadowOptions;

    explicit BuilderDetails(Type type) noexcept : mType(type) { }
    // this is only needed for the explicit instantiation below
    BuilderDetails() = default;
};

using BuilderType = LightManager;
BuilderType::Builder::Builder(Type type) noexcept: BuilderBase<LightManager::BuilderDetails>(type) {}
BuilderType::Builder::~Builder() noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder&& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder&& rhs) noexcept = default;

LightManager::Builder& LightManager::Builder::castShadows(bool enable) noexcept {
    mImpl->mCastShadows = enable;
    return *this;
}

LightManager::Builder& LightManager::Builder::shadowOptions(const ShadowOptions& options) noexcept {
    mImpl->mShadowOptions = options;
    return *this;
}

LightManager::Builder& LightManager::Builder::castLight(bool enable) noexcept {
    mImpl->mCastLight = enable;
    return *this;
}

LightManager::Builder& LightManager::Builder::position(const float3& position) noexcept {
    mImpl->mPosition = position;
    return *this;
}

LightManager::Builder& LightManager::Builder::direction(const float3& direction) noexcept {
    mImpl->mDirection = direction;
    return *this;
}

LightManager::Builder& LightManager::Builder::color(const LinearColor& color) noexcept {
    mImpl->mColor = color;
    return *this;
}

LightManager::Builder& LightManager::Builder::intensity(float intensity) noexcept {
    mImpl->mIntensity = intensity;
    return *this;
}

LightManager::Builder& LightManager::Builder::intensity(float watts, float efficiency) noexcept {
    mImpl->mIntensity = efficiency * 683.0f * watts;
    return *this;
}

LightManager::Builder& LightManager::Builder::falloff(float radius) noexcept {
    mImpl->mFalloff = radius;
    return *this;
}

LightManager::Builder& LightManager::Builder::spotLightCone(float inner, float outer) noexcept {
    mImpl->mSpotInnerOuter = { inner, outer };
    return *this;
}

LightManager::Builder& LightManager::Builder::sunAngularRadius(float sunAngle) noexcept {
    mImpl->mSunAngle = sunAngle;
    return *this;
}

LightManager::Builder& LightManager::Builder::sunHaloSize(float haloSize) noexcept {
    mImpl->mSunHaloSize = haloSize;
    return *this;
}

LightManager::Builder& LightManager::Builder::sunHaloFalloff(float haloFalloff) noexcept {
    mImpl->mSunHaloFalloff = haloFalloff;
    return *this;
}

LightManager::Builder::Result LightManager::Builder::build(Engine& engine, Entity entity) {
    upcast(engine).createLight(*this, entity);
    return Success;
}

// ------------------------------------------------------------------------------------------------

namespace details {

FLightManager::FLightManager(FEngine& engine) noexcept : mEngine(engine) {
    // DON'T use engine here in the ctor, because it's not fully constructed yet.
}

FLightManager::~FLightManager() {
    // all components should have been destroyed when we get here
    // (terminate should have been called from Engine's shutdown())
    assert(mManager.getComponentCount() == 0);
}

void FLightManager::init(FEngine& engine) noexcept {
}

void FLightManager::create(const FLightManager::Builder& builder, utils::Entity entity) {
    auto& manager = mManager;

    if (UTILS_UNLIKELY(manager.hasComponent(entity))) {
        destroy(entity);
    }
    Instance i = manager.addComponent(entity);
    assert(i);

    if (i) {
        // This needs to happen before we call the set() methods below
        // Type must be set first (some calls depend on it below)
        LightType& lightType = manager[i].lightType;
        lightType.type = builder->mType;
        lightType.shadowCaster = builder->mCastShadows;
        lightType.lightCaster = builder->mCastLight;
        lightType.shadowMapBits = uint8_t(std::min(15, std::ilogbf(builder->mShadowOptions.mapSize)));

        ShadowParams& shadowParams = manager[i].shadowParams;
        shadowParams.shadowConstantBias = clamp(builder->mShadowOptions.constantBias, 0.0f, 2.0f);
        shadowParams.shadowNormalBias = clamp(builder->mShadowOptions.normalBias, 0.0f, 3.0f);
        shadowParams.shadowFar = std::max(builder->mShadowOptions.shadowFar, 0.0f);
        shadowParams.shadowNearHint = std::max(builder->mShadowOptions.shadowNearHint, 0.0f);
        shadowParams.shadowFarHint = std::max(builder->mShadowOptions.shadowFarHint, 0.0f);

        // set default values by calling the setters
        setLocalPosition(i, builder->mPosition);
        setLocalDirection(i, builder->mDirection);
        setColor(i, builder->mColor);

        // this must be set before intensity
        setSpotLightCone(i, builder->mSpotInnerOuter.x, builder->mSpotInnerOuter.y);
        setIntensity(i, builder->mIntensity);

        setFalloff(i, builder->mCastLight ? builder->mFalloff : 0);
        setSunAngularRadius(i, builder->mSunAngle);
        setSunHaloSize(i, builder->mSunHaloSize);
        setSunHaloFalloff(i, builder->mSunHaloFalloff);
    }
}

void FLightManager::prepare(backend::DriverApi& driver) const noexcept {
}

void FLightManager::destroy(utils::Entity e) noexcept {
    Instance i = getInstance(e);
    if (i) {
        auto& manager = mManager;
        manager.removeComponent(e);
    }
}

void FLightManager::terminate() noexcept {
    auto& manager = mManager;
    if (!manager.empty()) {
#ifndef NDEBUG
        slog.d << "cleaning up " << manager.getComponentCount()
               << " leaked Light components" << io::endl;
#endif
        while (!manager.empty()) {
            Instance ci = manager.end() - 1;
            manager.removeComponent(manager.getEntity(ci));
        }
    }
}

void FLightManager::setLocalPosition(Instance i, const float3& position) noexcept {
    assert(i);
    auto& manager = mManager;
    manager[i].position = position;
}

void FLightManager::setLocalDirection(Instance i, float3 direction) noexcept {
    assert(i);
    auto& manager = mManager;
    manager[i].direction = direction;
}

void FLightManager::setColor(Instance i, const LinearColor& color) noexcept {
    if (i) {
        auto& manager = mManager;
        manager[i].color = color;
    }
}

void FLightManager::setIntensity(Instance i, float intensity) noexcept {
    auto& manager = mManager;
    if (i) {
        Type type = getLightType(i).type;
        float luminousPower = intensity;
        float luminousIntensity;
        switch (type) {
            case Type::SUN:
            case Type::DIRECTIONAL:
                // luminousPower is in lux, nothing to do.
                luminousIntensity = luminousPower;
                break;

            case Type::POINT:
                // li = lp / (4*pi)
                luminousIntensity = luminousPower * float(M_1_PI) * 0.25f;
                break;

            case Type::FOCUSED_SPOT: {
                // li = lp / (2*pi*(1-cos(outer/2)))
                SpotParams& spotParams = manager[i].spotParams;
                float2 scaleOffset = spotParams.scaleOffset;
                float cosOuter = -scaleOffset.y / scaleOffset.x;
                float cosHalfOuter = std::sqrt((1.0f + cosOuter) * 0.5f); // half-angle identities
                luminousIntensity = luminousPower / (2.0f * float(M_PI) * (1.0f - cosHalfOuter));
                spotParams.luminousPower = luminousPower;
                break;
            }
            case Type::SPOT:
                // li = lp / pi
                luminousIntensity = luminousPower * float(M_1_PI);
                break;
        }
        manager[i].intensity = luminousIntensity;
    }
}

void FLightManager::setFalloff(Instance i, float falloff) noexcept {
    auto& manager = mManager;
    if (i && !isDirectionalLight(i)) {
        float sqFalloff = falloff * falloff;
        SpotParams& spotParams = manager[i].spotParams;
        manager[i].squaredFallOffInv = sqFalloff ? (1 / sqFalloff) : 0;
        spotParams.radius = falloff;
    }
}

void FLightManager::setSpotLightCone(Instance i, float inner, float outer) noexcept {
    auto& manager = mManager;
    if (i && isSpotLight(i)) {
        // clamp the inner/outer angles to pi
        float outerClamped = std::min(std::abs(outer), float(M_PI));
        float innerClamped = std::min(std::abs(inner), float(M_PI));

        // inner must always be smaller than outer
        innerClamped = std::min(innerClamped, outerClamped);

        float cosOuter = fast::cos(outerClamped);
        float cosInner = fast::cos(innerClamped);
        float cosOuterSquared = cosOuter * cosOuter;
        float scale = 1 / std::max(1.0f / 1024.0f, cosInner - cosOuter);
        float offset = -cosOuter * scale;

        SpotParams& spotParams = manager[i].spotParams;
        spotParams.cosOuterSquared = cosOuterSquared;
        spotParams.sinInverse = 1 / std::sqrt(1 - cosOuterSquared);
        spotParams.scaleOffset = { scale, offset };

        // we need to recompute the luminous intensity
        Type type = getLightType(i).type;
        if (type == Type::FOCUSED_SPOT) {
            float luminousPower = spotParams.luminousPower;
            float cosHalfOuter = std::sqrt((1.0f + cosOuter) * 0.5f); // half-angle identities
            float luminousIntensity = luminousPower / (2.0f * float(M_PI) * (1.0f - cosHalfOuter));
            manager[i].intensity = luminousIntensity;
        }
    }
}

void FLightManager::setSunAngularRadius(Instance i, float angularRadius) noexcept {
    auto& manager = mManager;
    if (i && isSunLight(i)) {
        angularRadius = clamp(angularRadius, 0.25f, 20.0f);
        manager[i].sunAngularRadius = angularRadius * float(M_PI / 180.0);
    }
}

void FLightManager::setSunHaloSize(Instance i, float haloSize) noexcept {
    auto& manager = mManager;
    if (i && isSunLight(i)) {
        manager[i].sunHaloSize = haloSize;
    }
}

void FLightManager::setSunHaloFalloff(Instance i, float haloFalloff) noexcept {
    auto& manager = mManager;
    if (i && isSunLight(i)) {
        manager[i].sunHaloFalloff = haloFalloff;
    }
}

} // namespace details

// ------------------------------------------------------------------------------------------------
// Trampoline calling into private implementation
// ------------------------------------------------------------------------------------------------

using namespace details;

bool LightManager::hasComponent(Entity e) const noexcept {
    return upcast(this)->hasComponent(e);
}

LightManager::Instance
LightManager::getInstance(Entity e) const noexcept {
    return upcast(this)->getInstance(e);
}

void LightManager::destroy(Entity e) noexcept {
    return upcast(this)->destroy(e);
}

void LightManager::setPosition(Instance i, const float3& position) noexcept {
    return upcast(this)->setLocalPosition(i, position);
}

const float3& LightManager::getPosition(Instance i) const noexcept {
    return upcast(this)->getLocalPosition(i);
}

void LightManager::setDirection(Instance i, const float3& direction) noexcept {
    upcast(this)->setLocalDirection(i, direction);
}

const float3& LightManager::getDirection(Instance i) const noexcept {
    return upcast(this)->getLocalDirection(i);
}

void LightManager::setColor(Instance i, const LinearColor& color) noexcept {
    upcast(this)->setColor(i, color);
}

const float3& LightManager::getColor(Instance i) const noexcept {
    return upcast(this)->getColor(i);
}

void LightManager::setIntensity(Instance i, float intensity) noexcept {
    upcast(this)->setIntensity(i, intensity);
}

float LightManager::getIntensity(Instance i) const noexcept {
    return upcast(this)->getIntensity(i);
}

void LightManager::setFalloff(Instance i, float radius) noexcept {
    upcast(this)->setFalloff(i, radius);
}

float LightManager::getFalloff(Instance i) const noexcept {
    return upcast(this)->getSquaredFalloffInv(i);
}

void LightManager::setSpotLightCone(Instance i, float inner, float outer) noexcept {
    upcast(this)->setSpotLightCone(i, inner, outer);
}

void LightManager::setSunAngularRadius(Instance i, float angularRadius) noexcept {
    upcast(this)->setSunAngularRadius(i, angularRadius);
}

float LightManager::getSunAngularRadius(Instance i) const noexcept {
    float radius = upcast(this)->getSunAngularRadius(i);
    return radius * float(180.0 / M_PI);
}

void LightManager::setSunHaloSize(Instance i, float haloSize) noexcept {
    upcast(this)->setSunHaloSize(i, haloSize);
}

float LightManager::getSunHaloSize(Instance i) const noexcept {
    return upcast(this)->getSunHaloSize(i);
}

void LightManager::setSunHaloFalloff(Instance i, float haloFalloff) noexcept {
    upcast(this)->setSunHaloFalloff(i, haloFalloff);
}

float LightManager::getSunHaloFalloff(Instance i) const noexcept {
    return upcast(this)->getSunHaloFalloff(i);
}

LightManager::Type LightManager::getType(LightManager::Instance i) const noexcept {
    return upcast(this)->getType(i);
}

} // namespace filament
